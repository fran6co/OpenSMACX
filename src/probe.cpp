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
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
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
    if (!((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF) & (1 << faction_id)) && ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF) & (1 << target_faction_id))) {
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
        prb_defense = range(PlayersData[ProbeTargetFactionID].soc_effect_active.values_[SocialEffect::PROBE]
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
        prb_defense_2 = range(PlayersData[ProbeTargetFactionID].soc_effect_active.values_[SocialEffect::PROBE]
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

#include "popup.h"
#include "dialogs.h"
#include "netdaemon.h"
#include "console.h"
#include "council.h"
#include "technology.h"
#include "faction.h"
#include "reportif.h"
#include "infowin.h"
#include "buffer.h"
#include "sprite.h"

#include "netmsg.h"
#include "xpops.h"
#include "general.h"
#include "alpha.h"
#include "text.h"
#include "game.h"
#include "base.h"
#include "map.h"
#include "random.h"
#include "veh.h"
#include <cstdlib>
#include <cstring>


// The native signed base ID at 0x9A659C is the existing
// SecretProject::nethack_terminus member (probe_kuna_owner_crosswalk.md).
// Typed transcription of the table walk at native 0x0059F77C..0x0059F7C9.
// Original bit/count locals retain descriptive names where gameplay meanings
// remain unproved. These helpers are source decomposition, not new image identities.
struct ProbeTechnologyScan {
    int known_flag_10;
    int flag_20_entries;
    int known_flag_20_entries;
};

static ProbeTechnologyScan probe_scan_technology_flags(int actor_faction_id,
                                                       int target_faction_id) {
    ProbeTechnologyScan result = {0, 0, 0};
    for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
        const uint32_t flags = Technology[tech_id].flags;
        if ((flags & 0x10u) != 0 &&
            has_tech_call(tech_id, actor_faction_id)) {
            result.known_flag_10 = 1;
        }
        if ((flags & 0x20u) != 0) {
            ++result.flag_20_entries;
            if (has_tech_call(tech_id, target_faction_id)) {
                ++result.known_flag_20_entries;
            }
        }
    }
    return result;
}

// Typed transcription of the seven-DWORD native scan at 0x0059FA29..0x0059FA54
// (and the same shape at 0x005A29ED..0x005A2A3C). This is only a local region
// fragment, not a complete probe() body or identity. Native starts at
// records_[15].achieved_, advances 0x38 six times through record 20, then
// reads the following DWORD at +0x4A8. In MonumentFaction that final word is
// bytes 12..15 of the deliberately opaque tail prefix; it is not latest_type_
// at +0x498 and is not a valid records_[21] member.
static bool probe_native_has_project_marker(const MonumentFaction &faction) {
    for (int record = 15; record <= 20; ++record) {
        if (faction.records_[record].achieved_ != 0) return true;
    }

    int opaque_tail_word;
    memcpy(&opaque_tail_word, faction.opaque_tail_prefix_ + 12,
           sizeof opaque_tail_word);
    return opaque_tail_word != 0;
}

static bool probe_native_project_marker_at(const MonumentFaction &faction,
                                            int scan_index) {
    if (scan_index < 6)
        return faction.records_[15 + scan_index].achieved_ != 0;
    int opaque_tail_word;
    memcpy(&opaque_tail_word, faction.opaque_tail_prefix_ + 12,
           sizeof opaque_tail_word);
    return opaque_tail_word != 0;
}

static bool probe_native_select_project_marker(const PlayerData &target_data,
                                                const MonumentFaction &faction,
                                                int &selection) {
    bool marker_seen = false;
    for (int scan_index = 0; scan_index < 7; ++scan_index) {
        if (!probe_native_project_marker_at(faction, scan_index)) continue;
        const uint32_t relation =
            target_data.diplo_treaties[scan_index + 1];
        if ((relation & 0x10u) != 0) continue;
        marker_seen = true;
        if (selection == -1 || (relation & 1u) != 0 ||
            ((relation & 2u) != 0 &&
             (target_data.diplo_treaties[selection + 1] & 1u) == 0)) {
            selection = scan_index + 1;
        }
    }
    return marker_seen;
}

// Native probe entry capture at 0x59F150..0x59F1D3, using source-owned rows.
// The actor-alien bit is cached in the EBP-0x24 local and later drives the
// image variants used by the result/UI helpers.
static void probe_native_capture_entry(int actor_vehicle_id, int base_id,
                                       int fallback_vehicle_id,
                                       int &actor_faction_id,
                                       int &target_faction_id,
                                       BOOL &actor_is_alien) {
    actor_faction_id = Vehs[actor_vehicle_id].faction_id;
    target_faction_id = base_id >= 0
        ? Bases[base_id].faction_id_current
        : Vehs[fallback_vehicle_id].faction_id;
    ProbeTargetFactionID = static_cast<uint32_t>(target_faction_id);
    actor_is_alien =
        (Players[actor_faction_id].rule_flags & RFLAG_ALIEN) != 0;
}

// Native entry decision through 0x59F2AF. Return values preserve the observed
// CFG exits: 0 reaches the 0x59F363 continuation, 1 reaches the subsequent
// local/lock UI gate at 0x59F2B5, and 2 takes the has_abil(8) branch to
// 0x59F357. The UI gate and its cleanup remain in the enclosing function.
static int probe_native_entry_route(int actor_vehicle_id, int base_id,
                                    int fallback_vehicle_id) {
    const Veh &actor_vehicle = Vehs[actor_vehicle_id];
    if ((map_loc(actor_vehicle.x, actor_vehicle.y)->climate & 0xE0) >= 0x60) {
        return 0;
    }

    const int target_x = base_id >= 0 ? Bases[base_id].x
                                      : Vehs[fallback_vehicle_id].x;
    const int target_y = base_id >= 0 ? Bases[base_id].y
                                      : Vehs[fallback_vehicle_id].y;
    if ((map_loc(target_x, target_y)->climate & 0xE0) < 0x60) return 0;

    const int prototype_id = actor_vehicle.proto_id;
    if (Chassis[VehPrototypes[prototype_id].chassis_id].triad == 1) return 0;
    if (has_abil(prototype_id, 8)) return 2;
    return 1;
}

// Native 0x59F2B5..0x59F352 dialog branch. At the native NetDaemon base
// 0x93CD90, 0x93E904 is offset 0x1B74: lock_state_[1].leading_words_[3]
// under the existing 0x3C-entry owner. Its meaning remains opaque (the
// constructor clears it and process_message reads it); preserve the low-bit
// test without renaming it. A true result means the
// outer probe takes its Popup/embedded-Scroll cleanup path at 0x59F31D and
// continues at 0x5A2CF4; this helper intentionally does not destroy borrowed
// function-scope objects.
static BOOL probe_native_entry_local_warning(int actor_faction_id,
                                             int argument_four) {
    if (actor_faction_id != ConsoleGlobal.faction_id_ || !argument_four) {
        return 0;
    }
    const uint32_t actor_shift =
        static_cast<uint32_t>(actor_faction_id) & 31u;
    const uint8_t actor_bit = static_cast<uint8_t>(1u << actor_shift);
    if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & actor_bit) == 0) return 0;
    if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) == 0) return 0;

    parse_say(0, static_cast<int>(reinterpret_cast<uintptr_t>(
        Ability[abil_index(8)].name)), -1, -1);
    NetMsgPrimary.pop("PROBEABILAMPHIBIOUS", 5000, 0, 0);
    return 1;
}

// Native 0x5A2F13..0x5A30DA choice producer. The start/rate/text gates precede
// the Dialogs list; the first localized row has ID zero, followed by eligible
// faction rows. The direct faction loop excludes actor/target, checks the
// directed actor-row/other-column treaty bit, then the alive byte.
static int probe_native_result_one_choices(Popup &popup, int actor_faction_id,
                                           int target_faction_id,
                                           int active_vehicle_id,
                                           int target_base_id,
                                           int native_threshold) {
    // Native starts this popup before calculating both rates. The first
    // success_rates return is intentionally ignored; the second controls
    // whether text/list setup continues.
    popup.start(PopupStartCaption, "FRAME", -1, 0, 0, 0);
    const int first_morale = morale_veh(active_vehicle_id, 1, 0);
    success_rates(2, first_morale, native_threshold, target_base_id);
    const int second_morale = morale_veh(active_vehicle_id, 1, 0);
    if (success_rates(3, second_morale, native_threshold + 1,
                      target_base_id) == 0) {
        const uint8_t selected = 0;
        Vehs[active_vehicle_id].probe_action = static_cast<uint8_t>(
            Vehs[active_vehicle_id].probe_action |
            static_cast<uint8_t>(selected << 5));
        return 0;
    }

    if (X_text_open(ScriptPopupFile, "FRAMEMENU") != 0) {
        const uint8_t selected = 0;
        Vehs[active_vehicle_id].probe_action = static_cast<uint8_t>(
            Vehs[active_vehicle_id].probe_action |
            static_cast<uint8_t>(selected << 5));
        return 0;
    }

    text_get();
    parse_string(TextBufferGetPtr, StringTemp);
    popup.dialogs().item(StringTemp, 0);

    int choices = 0;
    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        if (faction_id == actor_faction_id ||
            faction_id == target_faction_id) {
            continue;
        }
        if ((PlayersData[actor_faction_id].diplo_treaties[faction_id] & 8u) == 0) {
            continue;
        }
        const uint8_t alive = reinterpret_cast<const uint8_t *>(
            &GameDataStateStorage.fields_.lock_masks_.enable_mask_)[1];
        const uint8_t faction_bit = static_cast<uint8_t>(1u << faction_id);
        if ((alive & faction_bit) == 0) continue;

        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        parse_says(1, get_name(faction_id), -1, -1);
        clear();
        text_get();
        parse_string(TextBufferGetPtr, StringTemp);
        popup.dialogs().item(StringTemp, faction_id);
        ++choices;
    }

    int selected_faction_id = 0;
    if (choices != 0) selected_faction_id = popup.exec();
    Vehs[active_vehicle_id].probe_action = static_cast<uint8_t>(
        Vehs[active_vehicle_id].probe_action |
        static_cast<uint8_t>(static_cast<uint8_t>(selected_faction_id) << 5));
    return selected_faction_id;
}

// Native 0x5A32A9..0x5A33CA completion. A negative local tech value takes the
// technology-steal owner; nonnegative values optionally report to the local
// console before tech_achieved. Both branches then set the selected Base state
// bit. This retains the distinct native call paths without inventing names for
// the enclosing probe locals.
static void probe_native_result_one_complete(int actor_faction_id,
                                              int target_faction_id,
                                              int target_base_id,
                                              int tech_id_or_negative,
                                              BOOL actor_is_alien) {
    ++PlayersData[target_faction_id].stolen_data_count[actor_faction_id];
    if (tech_id_or_negative < 0) {
        steal_tech(actor_faction_id, target_faction_id, 1);
    } else {
        const int current_faction = ConsoleGlobal.faction_id_;
        if (actor_faction_id == current_faction ||
            target_faction_id == current_faction) {
            parse_says(0, get_adjective(target_faction_id), -1, -1);
            parse_says(2, get_adjective(actor_faction_id), -1, -1);
            clear();
            if (tech_id_or_negative == 0x270F) {
                says("STOLEMAP");
            } else {
                say_tech(tech_id_or_negative, 0);
                parse_says(1, StringTemp, -1, -1);
                clear();
                says("DECIPHERED");
            }
            says(actor_faction_id == current_faction ? "0" : "1");
            NetMsgPrimary.pop(StringTemp,
                              actor_is_alien ? "690708" : "690718");
        }
        tech_achieved(actor_faction_id, tech_id_or_negative,
                      target_faction_id, 0);
    }
    Bases[target_base_id].state |= 0x40u;
}

// Typed fragment of probe's native selector-3 branch. The preceding native
// call sites supply `stealable_energy` from steal_energy(EDI), then morale from
// morale_veh(arg1, 1, 0); their register-to-ID producer is tracked separately.
static int probe_energy_signed32(uint32_t bits) {
    int value;
    memcpy(&value, &bits, sizeof value);
    return value;
}

static int probe_apply_energy_transfer_native(int actor_faction_id,
                                              int target_faction_id,
                                              int stealable_energy,
                                              int morale) {
    const uint32_t product_bits =
        static_cast<uint32_t>(stealable_energy) *
        static_cast<uint32_t>(morale);
    const int product = probe_energy_signed32(product_bits);
    const int quotient = product / 3;
    const int remainder = product - quotient * 3;

    // The actual rnd implementation never reads this second parameter. The
    // pinned call supplies an rdata string; zero is behavior-equivalent here
    // while avoiding an old-image pointer in the typed fragment.
    const int random_half = static_cast<int>(rnd(remainder, 0) / 2u);
    int amount = range(quotient + random_half, 0,
                       PlayersData[target_faction_id].energy_reserves);
    if (amount == 1) amount = 0;

    PlayersData[target_faction_id].energy_reserves = probe_energy_signed32(
        static_cast<uint32_t>(PlayersData[target_faction_id].energy_reserves) -
        static_cast<uint32_t>(amount));
    PlayersData[actor_faction_id].energy_reserves = probe_energy_signed32(
        static_cast<uint32_t>(PlayersData[actor_faction_id].energy_reserves) +
        static_cast<uint32_t>(amount));
    return amount;
}

// Native selector-8 cleanup at 0x5A3CB3..0x5A3CE3. Keep the fields opaque:
// native compares the first DWORD in each of six records and clears the DWORD
// at +8 only when it was nonzero. That is MonumentRecord::year_ / word_8_
// under the existing data owner, not a guessed monument-status operation.
static void probe_native_clear_monument_choice_word(
        MonumentFaction &faction, int selected_first_word) {
    for (int record = 0; record < 6; ++record) {
        MonumentRecord &entry = faction.records_[record];
        if (entry.word_8_ != 0 && entry.year_ == selected_first_word) {
            entry.word_8_ = 0;
        }
    }
}

// Provisional project owner for the single native store at 0x007AD330. The
// pinned code writes this DWORD to zero in the selector-8 release path. No
// original symbol, enclosing object, capacity or lifetime is established;
// retain the unknown identity rather than assigning it to a nearby global.
uint32_t ProbeNativeUnknown7AD330 = 0;

static void probe_native_set_monument_selection_lock_bit(int selected_faction_id) {
    // Native SHL is performed on DL, so the count is modulo 32 and the
    // one-bit result is truncated to one byte before writing [0x9A64E9].
    const uint32_t shift_count = (uint32_t)selected_faction_id & 31u;
    const uint8_t selected_bit = (uint8_t)(1u << shift_count);
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ |= (uint32_t)selected_bit << 8;
}

// Typed Popup/Dialogs list region at 0x5A3B23..0x5A3C50. Popup lifetime is
// owned by the enclosing probe candidate. The apparent frame-local Sprite and
// its pointer are actually Popup-owned members: the native addresses are
// Popup+0x2118 and Popup+0x20E4, matching BasePop::sprite_ and
// caller_sprite_. Use the existing owned-picture accessors; don't add a second
// Sprite or constructor/destructor that the image doesn't call.
static int probe_native_monument_choice_list(Popup &popup,
                                              MonumentFaction &faction,
                                              BOOL actor_is_alien) {
    popup.start("FREEWHO", 0x40);
    char *const image = actor_is_alien
        ? (char *)"al_infil_sm.pcx" : (char *)"infil_sm.pcx";
    int picture_width = 0;
    int picture_height = 0;
    if (Buffer::get_pcx_dimensions(image, &picture_width, &picture_height) == 0 &&
        popup.owned_picture().init(image, picture_width, picture_height) == 0) {
        popup.select_owned_picture();
    }

    for (int record = 0; record < 6; ++record) {
        MonumentRecord &entry = faction.records_[record];
        if (entry.word_8_ == 0) continue;

        StringTemp[0] = '\0';
        says(get_title(entry.year_));
        space();
        says(get_name(entry.year_));
        say_label(0xB4);
        space();
        says(get_adjective(entry.year_));
        popup.item(StringTemp, entry.year_);
    }

    const int selection = popup.exec();
    // The enclosing native probe exits immediately for a negative selection;
    // its lexical Popup destructor performs the teardown, including close().
    // This borrowed-reference region must not destroy/close the same object.
    return selection;
}

// Continuation immediately after the positive Popup::exec result at
// 0x5A3C50..0x5A3E6A. Keeps every matrix direction explicit; the other side of
// the chooser (local Sprite + start/list construction) is handled by the
// Popup-reference routine above.
static int probe_native_commit_monument_choice(Popup &popup,
                                                int actor_faction_id,
                                                int target_faction_id,
                                                int probe_vehicle_id,
                                                int initial_selection,
                                                uint32_t native_alive_bits,
                                                BOOL actor_is_alien) {
    int selected_faction_id = initial_selection;
    const uint32_t actor_bit = (uint32_t)1u << actor_faction_id;
    if ((native_alive_bits & actor_bit) != 0 &&
        actor_faction_id == ConsoleGlobal.faction_id_) {
        parse_says(1, get_adjective(selected_faction_id), -1, -1);
        selected_faction_id = probe_native_monument_choice_list(
            popup, FactionMonuments[target_faction_id],
            actor_is_alien);
    }
    if (selected_faction_id < 0) return selected_faction_id;

    if (Vehs[probe_vehicle_id].home_base_id < 0) {
        veh_kill(probe_vehicle_id);
    }
    probe_native_set_monument_selection_lock_bit(selected_faction_id);
    setup_player(selected_faction_id, target_faction_id, 1);
    --PlayersData[target_faction_id].unk_108;
    probe_native_clear_monument_choice_word(
        FactionMonuments[target_faction_id], selected_faction_id);

    if (actor_faction_id == ConsoleGlobal.faction_id_ ||
        selected_faction_id == ConsoleGlobal.faction_id_) {
        parse_says(0, get_title(selected_faction_id), -1, -1);
        parse_says(1, get_name(selected_faction_id), -1, -1);
        parse_says(2, get_adjective(selected_faction_id), -1, -1);
        NetMsgPrimary.pop("fac_escape_sm.pcx", "WILLYFREE");
    }

    if (!(GameDataStateStorage.fields_.game_rules_ & 0x02000000)) {
        for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
            if (has_tech(tech_id, selected_faction_id) &&
                !has_tech(tech_id, actor_faction_id)) {
                // This is the native call's positional register order: actor,
                // tech, selected faction, wait. Keep it even though the
                // current formal parameter names on net_tech obscure it.
                net_tech(actor_faction_id, tech_id, selected_faction_id, 0);
            }
        }
    }

    ProbeNativeUnknown7AD330 = 0;
    ConsoleGlobal.update_data(0);
    PlayersData[actor_faction_id].diplo_friction[selected_faction_id] = 0;
    net_treaty_off(selected_faction_id, actor_faction_id, 0x20, 1);
    net_treaty_on(actor_faction_id, selected_faction_id, 0x0800000A, 0);
    net_set_treaty(selected_faction_id, actor_faction_id,
                   0x02000000, 1, 0);
    net_treaty_on(selected_faction_id, actor_faction_id, 0x01000001, 0);
    PlayersData[selected_faction_id].diplo_patience[actor_faction_id] = 0;
    PlayersData[actor_faction_id].diplo_spoke[selected_faction_id] = 0;
    diplomacy_check(actor_faction_id, selected_faction_id, 0);
    ++PlayersData[actor_faction_id].global_reputation;
    ++PlayersData[selected_faction_id].global_reputation;
    return selected_faction_id;
}

// Selector 2 native action at 0x5A33CF..0x5A3712. This is a connected typed
// result-region candidate, not the enclosing probe() body: the lexical Popup,
// SEH states, shared notice edge for 0x63, other selector arms and action-loop
// locals still belong to the full owner closure.
#include "basewin.h"
#include "tutwin.h"

// Producer of the native stack text buffer passed to parse_says slot 0 later
// in selector 2. Native clears the local first, then concatenates a unit
// prototype name for nonnegative production IDs or the translated facility
// name for negative facility IDs.
static void probe_native_selector2_prepare_local_text(int base_id,
                                                       char *local_text) {
    local_text[0] = 0;
    const int production_id = Bases[base_id].queue_production_id[0];
    if (production_id >= 0) {
        strcat(local_text, VehPrototypes[production_id].veh_name);
    } else {
        const int facility_id = -production_id;
        const int name_ref = static_cast<int>(reinterpret_cast<uintptr_t>(
            Facility[facility_id].name));
        strcat(local_text, StringTable.get(name_ref));
    }
}

// Keep the two mineral stores separate and in native order. Selection 0 then
// rejoins the same BaseWin/redraw/facility-gate tail as the facility path.
static void probe_native_selector2_clear_base_minerals(int base_id) {
    Bases[base_id].minerals_accumulated_2 = 0;
    Bases[base_id].minerals_accumulated = 0;
}

// Native selection 0x62 searches vehicles on the selected base tile. The
// prototype-plan retry calls the real source owner veh_next_stack_id (whose
// original identity is 0x50B8F0), not an executable-address stand-in.
static int probe_native_selector2_find_plan_five_vehicle(int base_id) {
    const Base &base = Bases[base_id];
    int vehicle_id = veh_at(base.x, base.y);
    if (vehicle_id < 0) return -2; // native jumps to the other notice exit
    while (vehicle_id >= 0) {
        const int prototype_id = Vehs[vehicle_id].proto_id;
        if (VehPrototypes[prototype_id].plan == 5) return vehicle_id;
        vehicle_id = veh_next_stack_id(vehicle_id);
    }
    return -1;
}

// The common native tail is reached after selector-2's selection-specific
// work. Preserve BaseWin::check_base before draw_tile and the gated has_fac
// return value (0 or 2) rather than turning the latter into a boolean.
static int probe_native_selector2_common_tail(int base_id,
                                               BOOL local_followup_gate) {
    TutWinBaseWindow.check_base(base_id);
    draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
    if (!local_followup_gate) return 0;
    return has_fac(0x22, base_id, 0) ? 2 : 0;
}

// Native parser/network notices from the selection-specific portion. The
// prepared local input is passed explicitly because its caller owns the stack
// buffer; its verified production/facility producer is the helper above. Do
// not substitute StringTemp or change the local-buffer alias.
static void probe_native_selector2_notice(int selection, int base_id,
                                           int vehicle_id,
                                           int actor_faction_id,
                                           int target_faction_id,
                                           BOOL actor_is_alien,
                                           const char *prepared_local_text) {
    const char *const image = actor_is_alien
        ? "al_infil_sm.pcx" : "infil_sm.pcx";
    const int current_faction = ConsoleGlobal.faction_id_;

    if (selection == 0) {
        parse_says(0, prepared_local_text, -1, -1);
        says("PRODVIRUS");
        parse_says(1, Bases[base_id].name_string + 1, -1, -1);
        parse_says(2, get_adjective(actor_faction_id), -1, -1);
        says(actor_faction_id == current_faction ? "0" : "1");
        if (actor_faction_id == current_faction ||
            target_faction_id == current_faction) {
            NetMsgPrimary.pop(StringTemp, image);
        }
        return;
    }

    if (selection == 0x62) {
        const int victim_faction_id = Vehs[vehicle_id].faction_id;
        const VehPrototype &prototype = VehPrototypes[Vehs[vehicle_id].proto_id];
        if (victim_faction_id == current_faction) {
            parse_says(0, prototype.veh_name, -1, -1);
            parse_says(1, get_adjective(actor_faction_id), -1, -1);
            NetMsgPrimary.pop("BUSTMYPLANET", image);
        } else if (actor_faction_id == current_faction) {
            parse_says(0, prototype.veh_name, -1, -1);
            parse_says(2, get_adjective(victim_faction_id), -1, -1);
            NetMsgPrimary.pop("BUSTYOURPLANET", image);
        }
        return;
    }

    parse_say(0, static_cast<int>(reinterpret_cast<uintptr_t>(
        Facility[selection].name)), -1, -1);
    says("FACVIRUS");
    parse_says(1, Bases[base_id].name_string + 1, -1, -1);
    parse_says(2, get_adjective(actor_faction_id), -1, -1);
    says(actor_faction_id == current_faction ? "0" : "1");
    if (actor_faction_id == current_faction ||
        target_faction_id == current_faction) {
        NetMsgPrimary.pop(StringTemp, image);
    }
}

// Known native storage effects for selector 2. The selection 0x63 edge leaves
// this result region for the enclosing shared VIRUSABORT notice path; callers
// must handle it there. Generic selections set_fac(selection, base, 0), while
// 0x62 returns the selected plan-five vehicle and updates the native search
// cursor only when the candidate is greater than that cursor.
static int probe_native_selector2_storage_action(int selection, int base_id,
                                                  int actor_faction_id,
                                                  int target_faction_id,
                                                  BOOL actor_is_alien,
                                                  const char *prepared_local_text,
                                                  int &vehicle_search_cursor,
                                                  BOOL local_followup_gate) {
    // Native clears StringTemp before it compares the saved selector at
    // 0x5A33D8, so every selector-2 edge starts from this same buffer state.
    clear();
    if (selection == 0x63) {
        NetMsgPrimary.pop("VIRUSABORT", 0);
        return -1;
    }

    if (selection == 0) {
        probe_native_selector2_clear_base_minerals(base_id);
    } else if (selection == 0x62) {
        const int vehicle_id = probe_native_selector2_find_plan_five_vehicle(base_id);
        if (vehicle_id == -2) return -2;
        if (vehicle_id >= 0) {
            probe_native_selector2_notice(selection, base_id, vehicle_id,
                actor_faction_id, target_faction_id, actor_is_alien,
                prepared_local_text);
            kill(vehicle_id);
            if (vehicle_search_cursor > vehicle_id) --vehicle_search_cursor;
        }
    } else {
        set_fac(selection, base_id, 0);
    }

    if (selection != 0x62) {
        probe_native_selector2_notice(selection, base_id, -1,
            actor_faction_id, target_faction_id, actor_is_alien,
            prepared_local_text);
    }
    return probe_native_selector2_common_tail(base_id, local_followup_gate);
}

// Native action selector -1, 0x5A4101..0x5A42C5. The byte table is the
// already-owned PlayerData::proto_id_active at +0x858 / 0x96D238, indexed by
// the EBP-0x20 target-row byte offset and the vehicle's current proto id.
// `native_exit_flag` is the still-opaque EBP-0x30 value deciding whether the
// native body returns at the common epilogue or continues at 0x5A4B28.
static BOOL probe_native_reassign_prototype_action(
        int actor_faction_id, int target_faction_id, int probe_vehicle_id,
        BOOL native_exit_flag, BOOL actor_is_alien) {
    Veh &vehicle = Vehs[probe_vehicle_id];
    const int old_prototype_id = vehicle.proto_id;
    uint8_t &target_active_count =
        PlayersData[target_faction_id].proto_id_active[old_prototype_id];
    target_active_count = static_cast<uint8_t>(target_active_count - 1u);

    if (old_prototype_id > 0x40) {
        VehPrototype &old_prototype = VehPrototypes[old_prototype_id];
        const int replacement = propose_proto(
            actor_faction_id, old_prototype.chassis_id,
            old_prototype.weapon_id, old_prototype.armor_id,
            old_prototype.ability_flags, old_prototype.reactor_id, -2,
            old_prototype.veh_name);
        if (replacement >= 0) vehicle.proto_id = static_cast<int16_t>(replacement);
    }

    vehicle.faction_id = static_cast<uint8_t>(actor_faction_id);
    uint8_t &actor_active_count = PlayersData[actor_faction_id]
        .proto_id_active[vehicle.proto_id];
    actor_active_count = static_cast<uint8_t>(actor_active_count + 1u);
    ++PlayersData[actor_faction_id].mind_control_total;
    ++PlayersData[target_faction_id].diplo_mind_control[actor_faction_id];

    const int x = vehicle.x;
    const int y = vehicle.y;
    owner_set(x, y, actor_faction_id);
    vehicle.home_base_id = static_cast<int16_t>(base_find(x, y, actor_faction_id));
    vehicle.moves_expended = 0;
    vehicle.order = 0;
    vehicle.state &= 0xFCFFBDFFu;
    spot_stack(probe_vehicle_id, target_faction_id);
    draw_tile(x, y, 2);

    if (target_faction_id == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.focus(x, y, actor_faction_id);
        parse_says(1, get_noun(actor_faction_id), -1, -1);
        NetMsgPrimary.pop(actor_is_alien
            ? "al_infil_sm.pcx" : "infil_sm.pcx", "SUBVERTED");
    }
    return native_exit_flag;
}

// Typed fragments lifted into the probe source draft from the verified
// 0x5A2321..0x5A23C8 native path. These do not constitute the probe body.
static int probe_policy_signed32(uint32_t bits) {
    int value;
    memcpy(&value, &bits, sizeof value);
    return value;
}

// The entry block selects the faction once from the explicit base argument,
// or from the fallback vehicle when that argument is negative.
static void probe_native_select_target_faction(int base_id,
                                               int fallback_vehicle_id) {
    if (base_id >= 0) {
        ProbeTargetFactionID = Bases[base_id].faction_id_current;
    } else {
        ProbeTargetFactionID = Vehs[fallback_vehicle_id].faction_id;
    }
}

// At 0x59F3D0..0x59F403 the native body reads the real Hunter-Seeker owner
// base from SecretProject, branches separately on a negative base ID, then
// compares that base's current faction with the target. Preserve all three
// states: -1 = unbuilt path, 0 = built but not target-owned, 1 = target-owned.
static int probe_native_hsa_project_target_relation(int target_faction_id) {
    const int owner_base_id = GameDataStateStorage.fields_.secret_project_.hunter_seeker_algorithm();
    if (owner_base_id < 0) return -1;
    return Bases[owner_base_id].faction_id_current == target_faction_id ? 1 : 0;
}

// Result encoding at 0x5A2D19; the following test at 0x5A2D33 reads bit 3
// back and produces native local state 1 or 2.
static void probe_native_store_vehicle_result(Veh &vehicle, int result) {
    const uint8_t encoded = static_cast<uint8_t>(
        static_cast<uint8_t>(static_cast<uint8_t>(result) + 0x1Fu) << 3);
    vehicle.probe_sabotage_id = static_cast<uint8_t>(
        vehicle.probe_sabotage_id | encoded);
}

static int probe_native_vehicle_result_state(const Veh &vehicle) {
    return (vehicle.probe_sabotage_id & 0x08) ? 2 : 1;
}

static int probe_native_base_facility_local_2c(int base_id,
                                                int local_2c) {
    const Base &base = Bases[base_id];
    if (base.facilities_built[6] & 0x40) --local_2c;
    if (base.facilities_built[8] & 0x04) local_2c += 2;
    return local_2c;
}

// 0x5A2D71..0x5A2DB1 under NetDaemon lock-state bit 1: actor faction is EBX,
// selected target is ESI. Preserve native row/column direction and 32-bit
// arithmetic for the energy cost and two mind-control counters.
static void probe_record_mind_control_accounting_native(int actor_faction_id,
                                                        int target_faction_id,
                                                        int energy_cost) {
    PlayerData &actor = PlayersData[actor_faction_id];
    PlayerData &target = PlayersData[target_faction_id];
    actor.energy_reserves = probe_policy_signed32(
        static_cast<uint32_t>(actor.energy_reserves) -
        static_cast<uint32_t>(energy_cost));
    actor.mind_control_total += 4u;
    target.diplo_mind_control[actor_faction_id] += 4u;
}

// Typed caller closure for selector 3 at 0x5A3712. Native calls
// steal_energy(target_base_id), then morale_veh(probe_vehicle_id,1,0), before
// the arithmetic in probe_apply_energy_transfer_native.
static int probe_native_energy_transfer_action(int actor_faction_id,
                                               int target_faction_id,
                                               int target_base_id,
                                               int probe_vehicle_id) {
    const int stealable_energy = steal_energy(target_base_id);
    const int morale = morale_veh(probe_vehicle_id, 1, 0);
    return probe_apply_energy_transfer_native(actor_faction_id,
                                               target_faction_id,
                                               stealable_energy, morale);
}

// Prefix adjustment made by the 0x5A1795 selector table. Selectors that reach
// 0x5A17B9 continue into subsequent native policy calculations; this helper
// intentionally models only their unchanged ESI value at that edge.
static int probe_native_policy_switch_prefix(int selector, int value) {
    switch (selector) {
    case 0: case 1: case 2: case 3: case 4: case 7:
        return value / 2; // native signed divide-by-two toward zero
    case 5: {
        const int64_t wide = value;
        const int64_t ceil_quarter = wide >= 0
            ? (wide + 3) / 4 : wide / 4;
        return static_cast<int>(wide - ceil_quarter);
    }
    case 8:
        return static_cast<int>(static_cast<uint32_t>(value) * 2u);
    default:
        return value;
    }
}

static int probe_native_policy_facility_adjusted_value(int base_id,
                                                       int local_2c,
                                                       int edi_value) {
    const int adjusted_local =
        probe_native_base_facility_local_2c(base_id, local_2c);
    return probe_native_policy_switch_prefix(adjusted_local + 2, edi_value);
}

// Selector 4 at 0x5A38C5..0x5A3A36. This is kept as a source-typed action
// region: the surrounding function's popup/SEH lifetime is still being
// reconstructed, so this is not a declaration of complete probe coverage.
// The source names below are established owners (Base, BaseCurrent, Player,
// ConsoleGlobal, range, set_base, base_compute, draw_tile and NetMsgPrimary).
static void probe_native_assimilation_action(int base_id,
                                             int actor_faction_id,
                                             int target_faction_id) {
    Base &base = Bases[base_id];
    const uint32_t wrapped_product =
        static_cast<uint32_t>(static_cast<int32_t>(base.population_size)) * 10u;
    const int population_limit = probe_policy_signed32(wrapped_product) / 4;
    const int old_assimilation_turns = base.assimilation_turns_left;
    const int candidate = population_limit > old_assimilation_turns
        ? population_limit : old_assimilation_turns;
    base.assimilation_turns_left = static_cast<uint8_t>(
        range(candidate, 0, old_assimilation_turns + 10));

    set_base(base_id);
    base_compute(1);
    const BOOL has_drone_riot = BaseCurrent->drone_total > BaseCurrent->talent_total;
    if (has_drone_riot) BaseCurrent->state |= 2u;
    draw_tile(base.x, base.y, 2);

    const BOOL actor_is_alien =
        (Players[actor_faction_id].rule_flags & RFLAG_ALIEN) != 0;
    if (actor_faction_id != ConsoleGlobal.faction_id_ &&
        target_faction_id != ConsoleGlobal.faction_id_) return;

    const char *const image = actor_is_alien
        ? "ALdrone_sm.pcx" : "drone_sm.pcx";
    const char *const message = has_drone_riot
        ? "DRONEINCITE0"
        : (actor_is_alien ? "DRONEINCITE1" : "PROBEASSIMILATE");
    NetMsgPrimary.pop(image, message);
}

// Selector 5 at 0x5A3A3B..0x5A3ADC. The research row is the incoming ESI
// faction (the same selected-target row used to form the local PlayerData
// pointer); the action actor remains EBX. Native reloads the research DWORD
// after rnd and writes it back before checking who receives the notice.
static int probe_native_tech_theft_action(int research_faction_id,
                                          int actor_faction_id,
                                          int target_faction_id,
                                          int target_base_id,
                                          BOOL actor_is_alien) {
    PlayerData &research_owner = PlayersData[research_faction_id];
    const int stolen = static_cast<int>(
        rnd(research_owner.tech_accumulated, (char *)"PROBEASSASSIN"));
    research_owner.tech_accumulated = probe_policy_signed32(
        static_cast<uint32_t>(research_owner.tech_accumulated) -
        static_cast<uint32_t>(stolen));

    if (actor_faction_id != ConsoleGlobal.faction_id_ &&
        target_faction_id != ConsoleGlobal.faction_id_) return stolen;

    parse_num(0, stolen);
    parse_says(0, Bases[target_base_id].name_string, -1, -1);
    parse_says(1, get_adjective(research_faction_id), -1, -1);
    parse_says(2, get_adjective(actor_faction_id), -1, -1);
    const char *const image = actor_is_alien
        ? "al_infil_sm.pcx" : "infil_sm.pcx";
    NetMsgPrimary.pop(image, "ASSASSINATED");
    return stolen;
}

static void probe_native_atrocity_followup(int actor_faction_id,
                                           int target_faction_id,
                                           int target_base_id,
                                           int local_44,
                                           int local_5c,
                                           BOOL actor_is_alien) {
    atrocity(actor_faction_id, target_faction_id, 1, 0);
    if (local_44 < 1) ++local_44;

    PlayerData &target_data = PlayersData[target_faction_id];
    if (target_data.flags & 0x400u) ++local_5c;
    const int adjusted_count = range(local_5c, 1, local_44);
    target_data.flags |= 0x400u;

    Base &base = Bases[target_base_id];
    uint32_t product_bits = static_cast<uint32_t>(local_44 - adjusted_count) *
        static_cast<uint32_t>(static_cast<int32_t>(base.population_size));
    int population_change = probe_policy_signed32(product_bits) / local_44;
    if (base.state & 0x20000u) population_change /= 2;
    if (has_fac(0x11, target_base_id, 0)) population_change /= 2;
    if (has_fac(0x12, target_base_id, 0)) population_change /= 2;
    ++population_change;
    if (population_change >= base.population_size) {
        population_change = base.population_size - 1;
    }

    const int x = base.x;
    const int y = base.y;
    int vehicle_id = veh_at(x, y);
    if (vehicle_id >= 0) {
        do {
            const int health_for_random = veh_health(vehicle_id);
            const int random_morale = static_cast<int>(rnd(
                (health_for_random + 1) / 2, NULL));
            const int health_for_half = veh_health(vehicle_id);
            int morale_change = random_morale + health_for_half / 2;
            if (has_fac(0x11, target_base_id, 0)) morale_change /= 2;
            if (has_fac(0x12, target_base_id, 0)) morale_change /= 2;
            if (base.state & 0x20000u) morale_change /= 2;
            Vehs[vehicle_id].morale = static_cast<uint8_t>(
                Vehs[vehicle_id].morale + static_cast<uint8_t>(morale_change));
            vehicle_id = veh_next_stack_id(vehicle_id);
        } while (vehicle_id >= 0);
    }

    base.state |= 0x20000u;
    if (population_change != 0) {
        uint8_t population_bits;
        memcpy(&population_bits, &base.population_size,
               sizeof population_bits);
        population_bits = static_cast<uint8_t>(
            population_bits - static_cast<uint8_t>(population_change));
        memcpy(&base.population_size, &population_bits,
               sizeof population_bits);
    }

    uint16_t unk_x_bits;
    memcpy(&unk_x_bits, &base.unk_x, sizeof unk_x_bits);
    const uint8_t next_unk_x_low = static_cast<uint8_t>(
        static_cast<uint8_t>(unk_x_bits) + static_cast<uint8_t>(local_44));
    unk_x_bits = static_cast<uint16_t>((unk_x_bits & 0xFF00u) |
                                       next_unk_x_low);
    memcpy(&base.unk_x, &unk_x_bits, sizeof unk_x_bits);
    base.nutrients_accumulated = 0;
    spot_base(target_base_id, actor_faction_id);
    draw_tile(base.x, base.y, 2);

    if (actor_faction_id != ConsoleGlobal.faction_id_ &&
        target_faction_id != ConsoleGlobal.faction_id_ &&
        !(GameDataStateStorage.fields_.game_state_ & 0x80)) return;

    parse_says(0, base.name_string, -1, -1);
    parse_says(1, get_adjective(actor_faction_id), -1, -1);
    StringTemp[0] = '\0';
    says("GENETICWARFARE");
    says(actor_faction_id == ConsoleGlobal.faction_id_ ? "0" : "1");
    X_pop(StringTemp, actor_is_alien
        ? "al_infil_sm.pcx" : "infil_sm.pcx", 0);
}


// Native 0x5A26F2..0x5A27CA traverses the first 21 entries of the existing
// paired RadiusOffsetX/RadiusOffsetY owners in order, stepping four bytes
// until offset 0x54. Keep this distinct from the nine-entry VehMoveDeltaX/Y.

// Native relocation selector 0x5A4331 / 0x5A4494. Sea chassis retain the
// original nearest-base test, then the actor-owned port scan and sea-region
// predicate. The scan's actor faction and original vehicle coordinates are
// passed as source-owned values, not reconstructed through address arithmetic.
static int probe_native_relocation_base(int x, int y, int actor_faction_id,
                                        int chassis_id,
                                        bool &native_port_scan) {
    native_port_scan = false;
    int relocation_base = base_find(x, y, actor_faction_id);
    if (Chassis[chassis_id].triad != TRIAD_SEA || relocation_base < 0 ||
        is_port(relocation_base, 0))
        return relocation_base;

    native_port_scan = true;
    for (int candidate_base = 0; candidate_base < GameDataStateStorage.fields_.base_current_count_;
         ++candidate_base) {
        if (Bases[candidate_base].faction_id_current != actor_faction_id)
            continue;
        if (!is_port(candidate_base, 0)) continue;
        // Keep native call order and repeated reads: each owned port checks
        // ocean at the probe's captured tile, then obtains that tile's region
        // before calling base_on_sea. Do not hoist either operation out of
        // the loop; native may observe their side effects more than once.
        if (!is_ocean(x, y)) continue;
        const int sea_region = region_at(x, y);
        if (base_on_sea(candidate_base, sea_region)) return candidate_base;
    }
    return -1;
}

// Native fallback selector reads one byte at
// PlayerData[actor] + 0x196C + Map[base_tile].region. The byte run straddles
// the tail of the existing PlayerData::unk_83[128] and the beginning of
// region_base_plan[128]; spell both in-range pieces through their genuine
// members while leaving the byte's gameplay meaning opaque.
static uint8_t probe_native_region_selector_byte(int faction_id,
                                                  int region_id) {
    if (region_id < 16)
        return PlayersData[faction_id].unk_83[112 + region_id];
    return PlayersData[faction_id].region_base_plan[region_id - 16];
}

// Native compares unaligned DWORDs at PlayerData+0x35C. This falls inside the
// existing opaque tech_trade_source byte run (+0x354); preserve the exact
// four-byte alias without assigning it an unsupported gameplay meaning.
static int probe_native_trade_source_word(const PlayerData &data) {
    int value;
    memcpy(&value, data.tech_trade_source + 8, sizeof value);
    return value;
}

// Early native selector -1 scan: the loop exits on the first target-owned
// facility-1 base. Both an empty scan and exhaustion without a match reach the
// shared label_5a14bf default of 12.
static int probe_native_negative_selector_base_distance(
        int fallback_vehicle_id, int target_faction_id) {
    int base_distance = 0x0C;
    for (int base_cursor = 0; base_cursor < GameDataStateStorage.fields_.base_current_count_; ++base_cursor) {
        if (Bases[base_cursor].faction_id_current == target_faction_id &&
            has_fac(1, base_cursor, 0)) {
            const int dx = x_dist(
                static_cast<int>(Vehs[fallback_vehicle_id].x),
                static_cast<int>(Bases[base_cursor].x));
            const int dy = abs(
                static_cast<int>(Vehs[fallback_vehicle_id].y) -
                static_cast<int>(Bases[base_cursor].y));
            base_distance = vector_dist(dx, dy);
            break;
        }
    }
    // Native tests the opaque Veh+0x2E word against FAC_23 after the scan,
    // halving the distance (not the computed amount) when that base is built.
    const int native_facility_index =
        Vehs[fallback_vehicle_id].prev_veh_id_stack;
    if (native_facility_index >= 0 &&
        has_fac(0x17, native_facility_index, 0))
        base_distance /= 2;
    return base_distance;
}

static int probe_native_negative_selector_amount(
        int target_faction_id, int fallback_proto_id, int base_distance) {
    // Preserve the native +0x320 reserve bias before signed division.
    const int available_energy = static_cast<int32_t>(
        static_cast<uint32_t>(
            PlayersData[target_faction_id].energy_reserves) + 0x320u);
    const int quotient = available_energy / (base_distance + 2);
    const uint32_t product = static_cast<uint32_t>(quotient) *
        static_cast<uint32_t>(VehPrototypes[fallback_proto_id].cost);
    return static_cast<int32_t>(product);
}

// Native 0x5A14E5 applies this reserve ratio to the computed amount before
// project protection, faction bonuses, and the policy jump table.
static int probe_native_negative_selector_target_ratio(int amount,
        int actor_faction_id, int target_faction_id,
        BOOL actor_is_human, BOOL target_is_human) {
    const int target_energy =
        PlayersData[target_faction_id].energy_reserves;
    if (!actor_is_human && target_is_human && target_energy >= 4) {
        const int32_t tripled = static_cast<int32_t>(
            static_cast<uint32_t>(amount) * 3u);
        amount = tripled / target_energy;
    }
    return amount;
}

static BOOL probe_native_negative_selector_eligible(
        int target_faction_id, int fallback_proto_id) {
    return target_faction_id != 0 &&
        (fallback_proto_id >= 0x40 ||
         (Weapon[VehPrototypes[fallback_proto_id].weapon_id]
                  .offense_rating >= 0 && fallback_proto_id != 0x0F));
}

static BOOL probe_native_negative_selector_protected(int target_faction_id) {
    return (Players[target_faction_id].rule_flags & 0x800) != 0;
}

static BOOL probe_native_negative_selector_vehicle_guard(
        int fallback_vehicle_id) {
    return stack_check(fallback_vehicle_id, 1, -1, -1, -1) >= 2;
}

static void probe_native_negative_selector_accounting(
        int actor_faction_id, int target_faction_id, int amount) {
    PlayersData[actor_faction_id].energy_reserves = static_cast<int32_t>(
        static_cast<uint32_t>(
            PlayersData[actor_faction_id].energy_reserves) -
        static_cast<uint32_t>(amount));
    PlayersData[actor_faction_id].soc_effect_base.values_[SocialEffect::TALENT] = static_cast<int32_t>(
        static_cast<uint32_t>(
            PlayersData[actor_faction_id].soc_effect_base.values_[SocialEffect::TALENT]) + 4u);
    PlayersData[target_faction_id]
        .diplo_mind_control[actor_faction_id] += 4;
}

static BOOL probe_native_negative_selector_target_can_afford(
        int target_faction_id, int amount) {
    const int target_energy =
        PlayersData[target_faction_id].energy_reserves;
    const uint32_t difference_bits =
        static_cast<uint32_t>(PlayersData[target_faction_id].unk_29[1]) -
        static_cast<uint32_t>(PlayersData[target_faction_id].unk_29[0]) + 3u;
    const int difference = static_cast<int32_t>(difference_bits);
    const uint32_t demand_bits = static_cast<uint32_t>(difference) *
        static_cast<uint32_t>(amount) * 2u;
    const int demand = static_cast<int32_t>(demand_bits) / 3;
    return demand <= target_energy;
}

// Preserve the original ordered amount modifiers: each actor faction bonus,
// then the Nethack Terminus base's 3/4 reduction. Native bypasses these amount
// modifiers for a protected target; the earlier distance change stays.
static int probe_native_nethack_terminus_adjust_amount(
        int amount, int actor_faction_id) {
    const int nethack_terminus_base = GameDataStateStorage.fields_.secret_project_.nethack_terminus();
    if (nethack_terminus_base >= 0 &&
        Bases[nethack_terminus_base].faction_id_current == actor_faction_id) {
        const int32_t tripled = static_cast<int32_t>(
            static_cast<uint32_t>(amount) * 3u);
        amount = (tripled + ((tripled >> 31) & 3)) >> 2;
    }
    return amount;
}

static int probe_native_negative_selector_adjust_amount(
        int amount, int actor_faction_id) {

    for (int bonus = 0;
         bonus < Players[actor_faction_id].faction_bonus_count; ++bonus) {
        if (Players[actor_faction_id].faction_bonus_id[bonus] == 0x10) {
            const uint32_t product = static_cast<uint32_t>(amount) *
                static_cast<uint32_t>(
                    Players[actor_faction_id].faction_bonus_val1[bonus]);
            amount = static_cast<int32_t>(product) / 100;
        }
    }

    return probe_native_nethack_terminus_adjust_amount(amount,
                                                        actor_faction_id);
}

// Native 0x5A23A7 amount table. The adjusted target probe value plus two
// indexes the switch; policy three's notification/re-entry is caller-owned.
static int probe_native_social_amount_prefix(int amount, int policy) {
    const uint32_t selector = static_cast<uint32_t>(policy) + 2u;
    if (selector > 5u) return amount;
    switch (policy) {
    case -2:
        return amount / 2;
    case -1: {
        const int quarter = (amount + ((amount >> 31) & 3)) >> 2;
        return amount - quarter;
    }
    case 1:
        return static_cast<int32_t>(static_cast<uint32_t>(amount) +
            static_cast<uint32_t>(amount / 2));
    case 2:
        return static_cast<int32_t>(static_cast<uint32_t>(amount) * 2u);
    case 0:
    case 3:
        return amount;
    }
    return amount;
}

// Native 0x5A183E..0x5A1D9E strategy score core, expressed through the
// existing typed PlayerData, VehPrototype, Chassis and Map owners. The
// selector at [EBP-0x20]+6 maps to the target's active probe social value plus
// six. This helper covers the selector's amount cases and the identified typed
// fields; surrounding entry/error/UI joins remain in the caller. The native
// The target-human ratio mutates the amount before protection and modifiers;
// it is not part of this score helper.
static int probe_native_wrapped_product(int value, int factor) {
    return static_cast<int32_t>(static_cast<uint32_t>(value) *
                               static_cast<uint32_t>(factor));
}

// Native 0x5A1785..0x5A1811 runs before the main strategy comparisons. Keep
// its policy-table prefix, ability check, prototype-plan exception and parser
// side effect in their original order; the caller then applies the human-actor
// vehicle-state early exit before entering the remaining score calculation.
static int probe_native_strategy_adjusted_amount(int amount,
        int strategy_vehicle_id, int target_policy_value) {
    const VehPrototype &prototype =
        VehPrototypes[Vehs[strategy_vehicle_id].proto_id];
    const uint32_t selector =
        static_cast<uint32_t>(target_policy_value) + 6u;
    if (selector <= 0x0Fu) {
        if (selector <= 4u || selector == 7u) {
            amount /= 2;
        } else if (selector == 5u) {
            amount -= amount / 4;
        } else if (selector == 6u) {
            if (has_abil(Vehs[strategy_vehicle_id].proto_id, 0x1000))
                amount = probe_native_wrapped_product(amount, 2);
            if (prototype.plan != 8 && prototype.plan != 9)
                amount /= 2;
        } else if (selector == 8u) {
            amount = probe_native_wrapped_product(amount, 2);
        }
    }
    parse_says(0, prototype.veh_name, -1, -1);
    return amount;
}

static BOOL probe_native_strategy_human_vehicle_early_exit(
        uint32_t lock_mask, int actor_faction_id, const Veh &vehicle) {
    const uint32_t actor_bit =
        1u << (static_cast<uint32_t>(actor_faction_id) & 31u);
    return (lock_mask & actor_bit) != 0 && (vehicle.state & 0x200u) != 0;
}

static int probe_native_strategy_score_core(int &native_amount,
        int actor_faction_id, int strategy_vehicle_id, int target_faction_id,
        BOOL &territory_mismatch) {
    territory_mismatch = 0;
    const VehPrototype &prototype =
        VehPrototypes[Vehs[strategy_vehicle_id].proto_id];
    int adjusted_score = native_amount;
    const int strategic_scale = static_cast<int32_t>(
        static_cast<uint32_t>(PlayersData[actor_faction_id].unk_29[1]) -
        static_cast<uint32_t>(PlayersData[actor_faction_id].unk_29[0]) + 2u);
    int score = static_cast<int32_t>(static_cast<uint32_t>(adjusted_score) *
                                     static_cast<uint32_t>(strategic_scale) *
                                     4u);
    native_amount = adjusted_score;

    const int weapon_strategy =
        weap_strat(prototype.weapon_id, actor_faction_id);
    const int armor_strategy =
        arm_strat(prototype.armor_id, actor_faction_id);
    const int region = map_loc(Vehs[strategy_vehicle_id].x,
                               Vehs[strategy_vehicle_id].y)->region;
    const int weapon_baseline =
        PlayersData[actor_faction_id].best_weapon_value;
    const int armor_baseline =
        PlayersData[actor_faction_id].best_armor_value;
    if (PlayersData[actor_faction_id].region_total_bases[region] != 0) {
        const int actor_force =
            PlayersData[actor_faction_id].region_force_rating[region];
        const int target_force =
            PlayersData[target_faction_id].region_force_rating[region];
        const int actor_combat =
            PlayersData[actor_faction_id].region_total_combat_vehs[region];
        if (target_force > actor_force / 2 + actor_combat) score /= 2;
        if (PlayersData[target_faction_id].region_total_combat_vehs[region] <
            actor_force)
            score = probe_native_wrapped_product(score, 2);
    }
    if (weapon_baseline < weapon_strategy) score /= 2;
    if (armor_baseline < armor_strategy) score /= 2;
    if (PlayersData[actor_faction_id].best_land_speed <
        Chassis[prototype.chassis_id].speed)
        score = static_cast<int32_t>(static_cast<uint32_t>(score) * 2u) / 3;

    const uint32_t pair_flags =
        PlayersData[actor_faction_id].diplo_treaties[target_faction_id];
    if (pair_flags & 2u)
        score = probe_native_wrapped_product(score, 2);
    if (!(pair_flags & 0x20u) && !(GameDataStateStorage.fields_.game_state_ & 0x200u) &&
        !is_alien_faction(actor_faction_id) && (pair_flags & 0x1000000u) &&
        (pair_flags & 6u)) {
        if (whose_territory(actor_faction_id,
                Vehs[strategy_vehicle_id].x,
                Vehs[strategy_vehicle_id].y,
                nullptr, 0) != actor_faction_id)
            territory_mismatch = 1;
    }
    if (weapon_strategy < weapon_baseline &&
        armor_strategy < armor_baseline)
        score = probe_native_wrapped_product(score, 2);
    if (weapon_baseline < weapon_strategy) score /= 2;

    const Map *tile = map_loc(Vehs[strategy_vehicle_id].x,
                              Vehs[strategy_vehicle_id].y);
    if ((tile->val3 & 0xC0) >= 0x80 ||
        ((tile->bit2 & 0x20u) && (tile->bit & 0xE0u) >= 0x40u) ||
        (tile->bit2 & 0x800u))
        score /= 2;
    // Native 0x5A1C70 reads the selected VehPrototype byte at +0x2A:
    // plans 0/1 halve this score and plans 2..255 triple it. This is distinct
    // from the chassis speed used earlier in the strategy comparisons.
    if (prototype.plan < 2)
        score /= 2;
    else
        score = static_cast<int32_t>(static_cast<uint32_t>(score) * 3u);
    return score;
}

// Native 0x5A1E73..0x5A1ECD parser/rate sequence used by the local
// SUBVERT chooser. Keep the two morale_veh calls distinct and ordered.
static void probe_native_subvert_preflight(int actor_vehicle_id,
        int target_base_id, int amount, int actor_energy) {
    parse_num(0, amount);
    parse_num(1, actor_energy);
    const int first_morale = morale_veh(actor_vehicle_id, 1, 0);
    success_rates(1, first_morale, -1, target_base_id);
    const int second_morale = morale_veh(actor_vehicle_id, 1, 0);
    success_rates(2, second_morale, 1, target_base_id);
}

// The function-scope Popup remains the owner. This preserves the native
// message/start sequence and the no-argument BasePop::exec overload.
static int probe_native_subvert_menu(Popup &popup, int amount,
                                    int actor_faction_id) {
    wave_it(0x20);
    popup.start(PopupStartCaption, "SUBVERT", -1, nullptr, 0, nullptr);
    if (amount <= PlayersData[actor_faction_id].energy_reserves)
        popup.start(PopupStartCaption, "SUBVERTMENU", -1,
                    nullptr, 0x80, nullptr);
    return popup.exec();
}

// Native 0x5A2EB2 and 0x5A36EE chance gates have the same ordered reads:
// optional target-base FAC_22, a morale probe used only for the >=1 test, a
// second morale read on that branch (otherwise literal one), then the live
// target probe value and its range clamp before rnd. Keep this as a real typed
// call sequence; the two gates execute separately around action accounting.
static BOOL probe_native_success_roll_passes(int action_result,
        int actor_vehicle_id, int target_base_id, int target_faction_id,
        int roll_threshold, BOOL threshold_inclusive) {
    int facility_bonus = 0;
    if (action_result != -1 && has_fac(0x22, target_base_id, 0) != 0)
        facility_bonus = 2;
    int morale_for_roll = 1;
    if (morale_veh(actor_vehicle_id, 1, 0) >= 1)
        morale_for_roll = morale_veh(actor_vehicle_id, 1, 0);
    const int target_probe = range(
        PlayersData[target_faction_id].soc_effect_active.values_[SocialEffect::PROBE] +
            facility_bonus,
        -2, 0);
    const int roll = rnd((morale_for_roll / 2 - target_probe) + 1, nullptr);
    return threshold_inclusive ? roll_threshold <= roll :
                                 roll_threshold < roll;
}

// Native 0x5A2DCD's first selector test routes only action values 1..5 and 8
// through the faction-choice producer. Values 6/7 and nonpositive values
// bypass it; selector 8 is deliberately not folded into the 1..5 range.
static BOOL probe_native_selector_uses_faction_choice(int action_result) {
    return !((action_result >= 6 || action_result <= 0) &&
             action_result != 8);
}

// Native 0x5A2E11..0x5A30F6 selects RankingFactionIDUnk1 automatically for
// a sufficiently veteran nonhuman actor when the target is not human. Preserve
// the one live morale call, candidate order, directed target-row policy test,
// and byte-mask membership gates.
static int probe_native_automatic_faction_choice(int actor_vehicle_id,
        int actor_faction_id, int target_faction_id,
        int native_probe_roll_threshold, uint8_t native_human_bits,
        uint8_t native_alive_bits) {
    if (morale_veh(actor_vehicle_id, 1, 0) < 5 ||
        native_probe_roll_threshold != 0)
        return 0;
    const uint8_t target_bit = static_cast<uint8_t>(1u <<
        (static_cast<uint32_t>(target_faction_id) & 31u));
    if ((native_human_bits & target_bit) != 0) return 0;

    for (int candidate_faction_id = 1;
         candidate_faction_id < MaxPlayerNum; ++candidate_faction_id) {
        if (candidate_faction_id == actor_faction_id ||
            candidate_faction_id == target_faction_id)
            continue;
        const uint8_t candidate_bit = static_cast<uint8_t>(1u <<
            (static_cast<uint32_t>(candidate_faction_id) & 31u));
        if ((native_human_bits & candidate_bit) == 0 ||
            (native_alive_bits & candidate_bit) == 0 ||
            candidate_faction_id != GameDataStateStorage.fields_.ranking_faction_id_unk1_)
            continue;
        const uint32_t target_policy =
            PlayersData[target_faction_id]
                .diplo_treaties[candidate_faction_id];
        if ((target_policy & 0x10u) == 0 && (target_policy & 8u) != 0)
            return candidate_faction_id;
    }
    return 0;
}

// ORIGINAL: 0x0059F120 ?probe@@YAHHHHH@Z 0x0059F120-0x005A5837;0x006613D0-0x00661B3D
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original instructions establish distinct actor, target-base and fallback-vehicle roles, genuine PlayerData/Map/Base/Veh owners and one automatic Popup with class-owned teardown.", "The actor lock/state edge skips strategy scoring and energy checks before its shared mode/selector tail. Action-six raw UI results and normalized remote bit tests have distinct threshold stores; false edges retain the original path-local value, with no eager initialization.", "Bounded actual-source controls and native compilation are not full CFG, VC6 unwind-state, linked-preservation or game-runtime proof. Ordered comparison/reload fidelity and historical matching remain restoration obligations."], "reason": "Complete typed Probe action/UI owner in its existing translation unit, retaining the four-argument ABI and both original main/unwind spans."}
int __cdecl probe(int actor_vehicle_id, int target_base_id,
                  int fallback_vehicle_id, int mode) {
    // The native stack receiver at EBP-0x53FC is the one function-scope Popup.
    // C++ owns its Popup -> Scroll/FlatButton/GraphicWin -> BasePop lifetime.
    Popup popup;

    int action_result = -1;
    int local_result = 0;
    // One persistent function-local gate, native EBP-0x30. The image has no
    // prologue initialization; every reachable read is preceded by one of its
    // path-local native stores, so do not replace them with an eager default.
    // It is distinct from the selected action, saved Veh byte, and local_result.
    int native_probe_roll_threshold;
    // The selector's project/fallback marker survives until its common
    // 0x59FB64 rejoin. KUNA carries this as v36; the physical [EBP-0x4C]
    // slot is reused by earlier coordinate temporaries, so this names the
    // semantic live range rather than claiming a whole-function stack slot.
    bool native_fallback_marker = false;
    bool native_nearby_stack_found = false;
    // KUNA v53 is initialized to -1 at entry and then carried across the
    // selector's project-record loop/rejoins; do not reset it per scan.
    int native_project_selection = -1;
    int local_context = 0;
    int native_flag20_entry_count = 0;
    int native_known_flag20_count = 0;
    bool native_known_flag10 = false;
    int local_project_cursor = -1;
    int local_technology_cursor = 0;
    int local_menu_result = -1;
    // Native [EBP+0x14] initially carries `mode`, then 0x5A2DCD clears that
    // same slot before any faction chooser. Keep the later relationship
    // choice distinct from the still-live source argument `mode`.
    int native_relationship_choice = 0;
    int control_chance;
    // The native stack buffer is not initialized at entry. Its sole producer
    // clears byte zero before appending, and all reads are dominated by that
    // producer; keep that path-local initialization rather than zeroing the
    // whole array on unrelated Probe paths.
    char selector2_prepared_text[256];
    int actor_faction_id;
    int target_faction_id;
    BOOL actor_is_alien;
    bool actor_has_hsa_ability = false;
    bool native_hsa_owner_mismatch = false;
    uint8_t native_human_bits = 0;
    uint8_t native_alive_bits = 0;
    uint8_t native_actor_bit = 0;
    uint8_t native_target_bit = 0;
    int captured_actor_vehicle_id = actor_vehicle_id;
    int captured_target_base_id = target_base_id;
    int captured_fallback_vehicle_id = fallback_vehicle_id;
    const int actor_proto_id = Vehs[actor_vehicle_id].proto_id;
    const int actor_chassis_id = VehPrototypes[actor_proto_id].chassis_id;
    const uint8_t actor_triad = Chassis[actor_chassis_id].triad;
    const int actor_x = Vehs[actor_vehicle_id].x;
    const int actor_y = Vehs[actor_vehicle_id].y;
    const int initial_actor_home_base_id =
        Vehs[actor_vehicle_id].home_base_id;

    probe_native_capture_entry(actor_vehicle_id, target_base_id,
                               fallback_vehicle_id, actor_faction_id,
                               target_faction_id, actor_is_alien);
    const uint32_t actor_bit =
        1u << (static_cast<uint32_t>(actor_faction_id) & 31u);
    const int entry_route = probe_native_entry_route(
        actor_vehicle_id, target_base_id, fallback_vehicle_id);
    if (entry_route == 1) {
        probe_native_entry_local_warning(actor_faction_id, mode);
        goto label_5a2cf4;
    }

    ProbeTargetHasHSA = 0;
    ProbeHasAlgoEnhancement = 0;
    actor_has_hsa_ability = has_abil(actor_proto_id, 0x10000000) != 0;
    if (actor_has_hsa_ability) {
        ProbeHasAlgoEnhancement = 1;
        const int hsa_owner_base = GameDataStateStorage.fields_.secret_project_.hunter_seeker_algorithm();
        native_hsa_owner_mismatch = hsa_owner_base < 0 ||
            Bases[hsa_owner_base].faction_id_current != target_faction_id;
        if (native_hsa_owner_mismatch) {
            ProbeTargetHasHSA = 0;
        } else {
            ProbeTargetHasHSA = 1;
        }
    }

label_59f769:
    {
    // Native reaches 0x59F769 only for an HSA actor whose project owner does
    // not match the selected target. Its counters otherwise remain zero, but
    // the later action/result flow still runs. Keep that conditional scan and
    // its actor-vs-target membership direction distinct from the common CFG.
    {
        ProbeTechnologyScan scan = {0};
        if (native_hsa_owner_mismatch)
            scan = probe_scan_technology_flags(actor_faction_id,
                                               target_faction_id);
        local_result = scan.known_flag_10;
        native_known_flag10 = scan.known_flag_10 != 0;
        native_flag20_entry_count = scan.flag_20_entries;
        native_known_flag20_count = scan.known_flag_20_entries;
        local_context = native_flag20_entry_count;
        local_project_cursor = native_known_flag20_count;
    }

    // Human-controlled actors with a nonnegative base argument enter the
    // native action-choice popup only when the exact network permission
    // condition below holds. The opposite paths retain the native selector
    // value and join label_59fb67 without manufacturing a menu result.
    if (target_base_id >= 0) {
        const uint32_t actor_bit =
            1u << (static_cast<uint32_t>(actor_faction_id) & 31u);
        const bool local_human_network_choice =
            actor_faction_id == ConsoleGlobal.faction_id_ &&
            (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0 &&
            (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0;
        if (!local_human_network_choice) {
            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) == 0)
                goto label_5a2574;
            action_result = Vehs[actor_vehicle_id].probe_action & 7;
            goto label_59fb67;
        }

        // Native parses the selected faction's noun before Popup::start; the
        // parser result controls the two portrait variants just as in the seed.
        const int popup_start_result = parse_says(
            0, Players[target_faction_id].noun_faction, -1, -1);
        popup.start(PopupStartCaption, "PROBE", -1, nullptr, 0x40, nullptr);
        Sprite portrait_sprite;
        int portrait_width = 0;
        int portrait_height = 0;
        const char *portrait_name = popup_start_result == 0
            ? "al_infil_sm.pcx" : "infil_sm.pcx";
        if (Buffer::get_pcx_dimensions(portrait_name, &portrait_width,
                                       &portrait_height) == 0) {
            if (popup_start_result == 0)
                portrait_name = "al_infil_sm.pcx";
            else
                portrait_name = "infil_sm.pcx";
            portrait_sprite.init(const_cast<char *>(portrait_name),
                                 portrait_width, portrait_height);
        }
        if (X_text_open(ScriptPopupFile, "PROBEMENU") != 0) {
            goto label_5a2cf4;
        }

        // Preserve the original sequence of text_get/item calls and sparse
        // option inclusion. `text_get` mutates TextBuffer and each item call
        // consumes that same buffer before the next read.
        if ((PlayersData[actor_faction_id].diplo_treaties[target_faction_id]
             & 0x1000u) == 0) {
            text_get();
            popup.dialogs().item(TextBufferGetPtr, 0);
        }
        text_get();
        if (Rules.tgl_probe_can_steal_tech != 0)
            popup.dialogs().item(TextBufferGetPtr, 1);
        text_get();
        popup.dialogs().item(TextBufferGetPtr, 2);
        text_get();
        popup.dialogs().item(TextBufferGetPtr, 3);
        text_get();
        popup.dialogs().item(TextBufferGetPtr, 4);
        text_get();
        int facility_byte = 0;
        int facility_mask = 0;
        bitmask(1, &facility_byte, &facility_mask);
        if ((Bases[target_base_id].facilities_built[facility_byte] &
             facility_mask) != 0)
            popup.dialogs().item(TextBufferGetPtr, 5);
        text_get();
        bitmask(1, &facility_byte, &facility_mask);
        if ((Bases[target_base_id].facilities_built[facility_byte] &
             facility_mask) == 0)
            popup.dialogs().item(TextBufferGetPtr, 6);
        text_get();
        if (local_result != 0)
            popup.dialogs().item(TextBufferGetPtr, 7);
        if (ExpansionEnabled && !IsMultiplayerNet) {
            text_get();
            bitmask(1, &facility_byte, &facility_mask);
            if ((Bases[target_base_id].facilities_built[facility_byte] &
                 facility_mask) != 0 &&
                probe_native_has_project_marker(
                    FactionMonuments[target_faction_id]))
                popup.dialogs().item(TextBufferGetPtr, 8);
        }
        local_menu_result = popup.exec(0, nullptr);
        if (local_menu_result < 0) {
            goto label_5a2cf4;
        }
        action_result = local_menu_result;
        Vehs[actor_vehicle_id].probe_action =
            static_cast<uint8_t>(local_menu_result);
    } else {
        action_result = -1;
    }
    // Native reaches 0x59FB64 after a completed selector-popup choice (or
    // the no-base -1 seed). The automatic faction/default selector below is
    // an out-of-line predecessor at 0x5A2574, never the popup's fallthrough.
    goto label_59fb64;

label_5a2574:
    {
        // Native 0x5A2574..0x5A2AAF: directed relationship defaults, base
        // state and target-region fallbacks, local stack search, then the
        // facility/monument policy choice. Preserve the order of reads and
        // repeated helper calls in this branch.
        const uint32_t native_pair_treaty =
            PlayersData[actor_faction_id].diplo_treaties[target_faction_id];
        action_result = (native_pair_treaty & 0x30u) != 0 ? 2 : 0;

        const int target_base_region = region_at(
            Bases[target_base_id].x, Bases[target_base_id].y);
        const uint8_t actor_region_selector =
            probe_native_region_selector_byte(actor_faction_id,
                                               target_base_region);
        if ((native_pair_treaty & 0x30u) != 0 &&
            actor_region_selector != 0 &&
            Bases[target_base_id].talent_total ==
                Bases[target_base_id].drone_total &&
            (rand() & 1) == 0 &&
            morale_veh(actor_vehicle_id, 1, 0) <= 3) {
            action_result = 4;
        }

        if (native_known_flag10) {
            if (Bases[target_base_id].population_size >= 6 &&
                (((native_pair_treaty & 0x40000u) != 0) ||
                 ((GameDataStateStorage.fields_.council_resolution_count_[8] > GameDataStateStorage.fields_.council_resolution_count_[9]) &&
                  (native_pair_treaty & 0x20u) != 0)) &&
                probe_native_region_selector_byte(actor_faction_id,
                                                    target_base_region) == 2) {
                action_result = 7;
            }
            if (Bases[target_base_id].population_size >= 4 &&
                (PlayersData[actor_faction_id].flags & 0x1000000u) != 0 &&
                ((native_pair_treaty & 7u) == 0 ||
                 (native_pair_treaty & 0x20u) != 0)) {
                action_result = 7;
            }
        }

        native_nearby_stack_found = false;
        if ((native_pair_treaty & 0x30u) != 0) {
            const int base_x = Bases[target_base_id].x;
            const int base_y = Bases[target_base_id].y;
            for (int coordinate_index = 0; coordinate_index < 21;
                 ++coordinate_index) {
                int search_x = base_x +
                    RadiusOffsetX[coordinate_index];
                if ((MapIsFlat & 1) == 0) {
                    if (search_x < 0) search_x += MapLongitudeBounds;
                    else if (search_x >= MapLongitudeBounds)
                        search_x -= MapLongitudeBounds;
                }
                const int search_y = base_y +
                    RadiusOffsetY[coordinate_index];
                if (search_y < 0 || search_y >= MapLatitudeBounds ||
                    search_x < 0 || search_x >= MapLongitudeBounds)
                    continue;
                const Map *const search_tile = map_loc(search_x, search_y);
                if ((search_tile->climate & 0xE0) < 0x60) continue;
                const int nearby_vehicle = veh_at(search_x, search_y);
                const int nearby_stack = stack_fix(nearby_vehicle);
                if (nearby_stack < 0 ||
                    Vehs[nearby_stack].faction_id != actor_faction_id)
                    continue;
                if (stack_check(nearby_stack, 4, actor_faction_id,
                                -1, -1) != 0) {
                    native_nearby_stack_found = true;
                    action_result = 2;
                    break;
                }
            }
        }

        if (Rules.tgl_probe_can_steal_tech != 0) {
            if ((Bases[target_base_id].state & 0x40u) != 0 &&
                morale_veh(actor_vehicle_id, 1, 0) <= 3 &&
                (action_result != 0 ||
                 (native_pair_treaty & 0x1000u) == 0)) {
                // Native jumps to label_5a2851, preserving the action.
            } else if (!native_nearby_stack_found &&
                       probe_native_region_selector_byte(
                           target_faction_id, target_base_region) != 2) {
                action_result = 1;
            }
        }

        if (native_known_flag10) {
            if (Bases[target_base_id].population_size >= 4 &&
                aah_ooga(actor_faction_id, actor_faction_id) ==
                    target_faction_id && climactic_battle() &&
                (PlayersData[actor_faction_id].y_target != 0 ||
                 (native_pair_treaty & 0x20u) != 0) &&
                (native_pair_treaty & 0xC30u) != 0) {
                action_result = 7;
            }
        }

        // Native tests the separate carried EBP-0x4C value before obtaining
        // the first bitmask. On a clear value it computes the marker from
        // target-base facilities, then may select action 5.
        if (!native_fallback_marker) {
            int facility_byte = 0;
            int facility_mask = 0;
            bitmask(1, &facility_byte, &facility_mask);
            native_fallback_marker =
                (Bases[target_base_id].facilities_built[facility_byte] &
                 facility_mask) != 0;
            const PlayerData &target_data = PlayersData[target_faction_id];
            if (native_fallback_marker && action_result != 1 &&
                target_data.ai_growth >= target_data.ai_wealth / 2 &&
                ((native_pair_treaty & 0x30u) != 0 ||
                 probe_native_trade_source_word(target_data) / 2 >
                    probe_native_trade_source_word(
                        PlayersData[actor_faction_id]) / 2 + 4)) {
                action_result = 5;
            }

            if (native_fallback_marker) {
                if (!ExpansionEnabled || IsMultiplayerNet) {
                    native_fallback_marker = true;
                    goto label_59fb64;
                }

                int monument_facility_byte = 0;
                int monument_facility_mask = 0;
                bitmask(1, &monument_facility_byte, &monument_facility_mask);
                if ((Bases[target_base_id].facilities_built[
                         monument_facility_byte] & monument_facility_mask) != 0) {
                    const MonumentFaction &monuments =
                        FactionMonuments[target_faction_id];
                    const bool native_project_seen_marker =
                        probe_native_select_project_marker(
                            target_data, monuments, native_project_selection);
                    if (native_project_seen_marker) {
                        action_result = 8;
                        native_fallback_marker = true;
                        goto label_59fb64;
                    }
                }
            }
        }

        if (!native_fallback_marker &&
            (native_pair_treaty & 0x30u) != 0 &&
            PlayersData[target_faction_id].diff_level <= 2 &&
            (factions_status_byte(0) &
             (1u << (static_cast<uint32_t>(target_faction_id) & 31u))) == 0 &&
            PlayersData[target_faction_id].soc_effect_temp.values_[SocialEffect::PROBE] < 3) {
            action_result = 6;
        }
        native_fallback_marker = true;
    }
    goto label_59fb64;

label_59fb64:
    // Native reloads the selector target into EDI at 0x59FB64. C++ retains
    // the already captured target faction as target_faction_id.
label_59fb67:
    while (true) {
        // label_59fb67, native 0x59FB67..0x59FC7D: result-zero network
        // completion publishes the directed 0x1000 treaty bit and, for the
        // local actor, refreshes the target's intelligence report.
        if (action_result == 0 &&
            (NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0) {
            if ((PlayersData[actor_faction_id]
                     .diplo_treaties[target_faction_id] & 0x1000u) == 0) {
                set_treaty(actor_faction_id, target_faction_id, 0x1000, 1);
                if (actor_faction_id == ConsoleGlobal.faction_id_)
                    report_intel(target_faction_id);
            }
            if (target_faction_id == ConsoleGlobal.faction_id_) {
                PluralityDefault = 0;
                GenderDefault = Players[actor_faction_id].is_leader_female;
                parse_says(0, Players[actor_faction_id].title_leader,
                           -1, -1);
                GenderDefault = Players[actor_faction_id].is_leader_female;
                PluralityDefault = 0;
                parse_says(1, Players[actor_faction_id].name_leader,
                           -1, 1);
                GenderDefault = Players[actor_faction_id].noun_gender;
                PluralityDefault = Players[actor_faction_id].is_noun_plural;
                const int noun_parse_result = parse_says(
                    2, Players[actor_faction_id].noun_faction, -1, 2);
                NetMsgPrimary.pop("DETECTINFILTRATE", -5000, 0,
                    noun_parse_result ? "al_infil_sm.pcx" : "infil_sm.pcx");
            }
        }

        // Native retaliation notice: the treaty row and reverse-row mask are
        // deliberately separate reads, followed by the `action_result >= 1`
        // gate. Keep parser slot order and the popp failure edge.
        const uint32_t actor_to_target_treaty =
            PlayersData[actor_faction_id].diplo_treaties[target_faction_id];
        const uint32_t target_to_actor_treaty =
            PlayersData[target_faction_id].diplo_treaties[actor_faction_id];
        if (actor_faction_id == ConsoleGlobal.faction_id_ &&
            (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
             static_cast<uint8_t>(1u << (actor_faction_id & 31))) != 0 &&
            (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0 &&
            (actor_to_target_treaty & 7u) != 0 &&
            ((target_to_actor_treaty & 1u) == 0 ||
             (target_to_actor_treaty & 0x02000000u) == 0) &&
            action_result >= 1) {
            PluralityDefault = 0;
            GenderDefault = Players[target_faction_id].is_leader_female;
            parse_says(0, Players[target_faction_id].title_leader,
                       -1, -1);
            GenderDefault = Players[target_faction_id].is_leader_female;
            PluralityDefault = 0;
            const int target_name_parse_result = parse_says(
                1, Players[target_faction_id].name_leader, -1, 1);
            GenderDefault = Players[target_faction_id].is_leader_female;
            PluralityDefault = 0;
            const char *risk_image = target_name_parse_result
                ? "infil_sm.pcx" : "al_infil_sm.pcx";
            if (popp(ScriptPopupFile, "RISKEXCUSE", 0,
                     risk_image, nullptr) == 0) {
                goto label_5a2ce9;
            }
        }

        if (action_result != 1) goto label_5a0917_done;
        {
        // Native initializes EBP-0x30 at 0x59FE0F, then writes one at
        // 0x59FE3D only when this Base::state bit is set.
        native_probe_roll_threshold =
            (Bases[target_base_id].state & 0x40u) != 0 ? 1 : 0;
        if ((Bases[target_base_id].state & 0x40u) != 0 &&
            actor_faction_id == ConsoleGlobal.faction_id_ &&
            (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
             static_cast<uint8_t>(1u << (actor_faction_id & 31))) != 0 &&
            (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0) {
            parse_says(0, Bases[target_base_id].name_string, -1, -1);
            const int first_morale = morale_veh(actor_vehicle_id, 1, 0);
            success_rates(1, first_morale, -1, target_base_id);

            const int second_morale = morale_veh(actor_vehicle_id, 1, 0);
            if (success_rates(2, second_morale, 1, target_base_id) == 0) {
                NetMsgPrimary.pop("ADVDECIPHER1", -5000, 0, nullptr);
                goto label_5a2ce9;
            }
            const char *decipher_image = second_morale
                ? "al_infil_sm.pcx" : "infil_sm.pcx";
            if (popp(ScriptPopupFile, "ADVDECIPHER", 0,
                     decipher_image, nullptr) == 0) {
                goto label_5a2ce9;
            }
        }

        // Native chooses a starting offset, then walks Technology in the
        // repeated (start + previous-cursor) modulo-89 order. Do not replace
        // this with a linear first-match search: the order is observable.
        int technology_start;
        if (IsMultiplayerNet != 0 && (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0) {
            random_reseed(MapRandSeed + GameDataStateStorage.fields_.turn_current_num_ + actor_faction_id);
            technology_start = static_cast<int>(random(0, 0x59));
            random_reseed(timeGetTime());
        } else {
            technology_start = rand() % 0x59;
        }

        int selected_technology = 0x59;
        bool found_stealable_technology = false;
        do {
            selected_technology =
                (technology_start + selected_technology) % 0x59;
            if (has_tech(selected_technology, target_faction_id)) {
                if (!has_tech(selected_technology, actor_faction_id)) {
                    found_stealable_technology = true;
                    break;
                }
            }
        } while (selected_technology + 1 < 0x59);
        if (!found_stealable_technology) {
            selected_technology = 0x270F;
        }
        local_technology_cursor = selected_technology;

label_5a00b2:
        if (actor_faction_id == ConsoleGlobal.faction_id_ &&
            (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0) {
            const int first_theft_morale =
                morale_veh(actor_vehicle_id, 1, 0);
            success_rates(1, first_theft_morale,
                          selected_technology, target_base_id);

            const int second_theft_morale =
                morale_veh(actor_vehicle_id, 1, 0);
            if (success_rates(2, second_theft_morale,
                              selected_technology + 1,
                              target_base_id) != 0) {
                int decoded_choice;
                if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0) {
                    decoded_choice =
                        (Vehs[actor_vehicle_id].probe_action & 8u) >> 3;
                } else {
                    const char *success_image = second_theft_morale
                        ? "al_infil_sm.pcx" : "infil_sm.pcx";
                    decoded_choice = popp(ScriptPopupFile, "DECIPHER", 0,
                                           success_image, nullptr);
                    if (decoded_choice < 0) {
                        goto label_5a2ce9;
                    }
                    Vehs[actor_vehicle_id].probe_action =
                        static_cast<uint8_t>(
                            Vehs[actor_vehicle_id].probe_action |
                            (decoded_choice << 3));
                }
                if (decoded_choice == 1) {
                    // EDI is reloaded from EBP-0x30 and incremented before
                    // native 0x5A01FE stores it back on this continuation.
                    ++native_probe_roll_threshold;
                    goto label_5a2dcd;
                }
            }
        }

        if (selected_technology == 0x270F &&
            (PlayersData[actor_faction_id].flags & 0x200u) != 0) {
            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0) {
                if (actor_faction_id != ConsoleGlobal.faction_id_ ||
                    (NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) == 0)
                    goto label_5a2dcd;
                NetMsgPrimary.pop("STOLENOTHING", -5000, 0, nullptr);
                goto label_5a2ce9;
            }
            action_result =
                (PlayersData[actor_faction_id]
                     .diplo_treaties[target_faction_id] & 0x30u) != 0
                ? 2 : 0;
        }

        if (action_result == 3) {
            // Native 0x5A034F writes one after the selected Base passes the
            // original +0x400000 state test; subsequent authorization and
            // report failures do not undo this function-local state.
            if ((Bases[target_base_id].state & 0x00400000u) != 0)
                native_probe_roll_threshold = 1;
            if ((Bases[target_base_id].state & 0x00400000u) == 0 ||
                actor_faction_id != ConsoleGlobal.faction_id_ ||
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) == 0 ||
                (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) == 0) {
                goto label_5a2dcd;
            }
            parse_says(0, Bases[target_base_id].name_string, -1, -1);
            const int first_energy_morale =
                morale_veh(actor_vehicle_id, 1, 0);
            success_rates(1, first_energy_morale, -1, target_base_id);
            const int second_energy_morale =
                morale_veh(actor_vehicle_id, 1, 0);
            if (success_rates(2, second_energy_morale, 1,
                              target_base_id) == 0) {
                NetMsgPrimary.pop("ADVENERGY1", -5000, 0, nullptr);
                goto label_5a2ce9;
            }
            const char *energy_image = second_energy_morale
                ? "al_infil_sm.pcx" : "infil_sm.pcx";
            if (popp(ScriptPopupFile, "ADVENERGY", 0,
                     energy_image, nullptr) == 0) {
                goto label_5a2ce9;
            }
            goto label_5a2dcd;
        }

        if (action_result == 2) {
            probe_native_selector2_prepare_local_text(
                target_base_id, selector2_prepared_text);

            int random_amount;
            if (IsMultiplayerNet != 0 &&
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0) {
                random_reseed(MapRandSeed + GameDataStateStorage.fields_.turn_current_num_ + actor_faction_id);
                const int morale = morale_veh(actor_vehicle_id, 1, 0);
                random_amount = static_cast<int>(
                    random(0, morale / 2 + 2));
                random_reseed(timeGetTime());
            } else {
                const int morale = morale_veh(actor_vehicle_id, 1, 0) / 2;
                random_amount = (morale != -1 && morale + 1 >= 0)
                    ? rand() % (morale + 2) : 0;
            }

            if (Bases[target_base_id].specialist_unk_1 <
                    cost_factor(actor_faction_id, 1, -1) * -1 ||
                random_amount != 0) {
                int candidate_action = 1;
                const int random_offset = rand() % 0x45;
                int candidate_attempts = 0;
                do {
                    const int candidate =
                        ((candidate_action + random_offset) % 0x45) + 1;
                    ++candidate_attempts;
                    if (candidate != 1) {
                        if ((candidate != 0x1A ||
                             (map_loc(Bases[target_base_id].x,
                                      Bases[target_base_id].y)->climate & 0xE0)
                                 >= 0x60) &&
                            candidate <= 0x40) {
                            int facility_byte = 0;
                            int facility_mask = 0;
                            bitmask(candidate, &facility_byte, &facility_mask);
                            if ((Bases[target_base_id]
                                     .facilities_built[facility_byte] &
                                 facility_mask) != 0) {
                                local_result = candidate;
                                if (candidate != 0)
                                    goto label_5a074b;
                                break;
                            }
                        }
                    } else {
                        const int first_vehicle = veh_at(
                            Bases[target_base_id].x, Bases[target_base_id].y);
                        if (first_vehicle >= 0 &&
                            stack_check(first_vehicle, 2, 5, -1, -1) != 0) {
                            local_result = 0x62;
                            goto label_5a074b;
                        }
                    }
                    ++candidate_action;
                } while (candidate_attempts < 0x46);
                if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) == 0) {
                    action_result =
                        Bases[target_base_id].faction_id_current <= 3 ? 4 : 0;
                    goto label_59fb67;
                }
            }

label_5a074b:
            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) == 0) {
                if (morale_veh(actor_vehicle_id, 1, 0) >= 5) {
                    const int first_vehicle = veh_at(
                        Bases[target_base_id].x, Bases[target_base_id].y);
                    if (first_vehicle >= 0 &&
                        stack_check(first_vehicle, 2, 5, -1, -1) != 0) {
                        local_result = 0x62;
                    } else {
                        int facility_byte = 0;
                        int facility_mask = 0;
                        const int choices[] = {5, 4, 2, 0x1B};
                        for (int choice_index = 0; choice_index < 4;
                             ++choice_index) {
                            const int choice = choices[choice_index];
                            bitmask(choice, &facility_byte, &facility_mask);
                            if ((Bases[target_base_id]
                                     .facilities_built[facility_byte] &
                                 facility_mask) != 0) {
                                local_result = choice;
                                break;
                            }
                        }
                    }
                }
                // Native's five fallback-facility joins store the selected
                // facility cursor (EBP-0x30 <- EDI) before the shared result
                // dispatch, including the no-stack/no-facility value.
                native_probe_roll_threshold = local_result;
                goto label_5a2dcd;
            }

            const int first_build_morale =
                morale_veh(actor_vehicle_id, 1, 0);
            success_rates(1, first_build_morale, -1, target_base_id);
            if (actor_faction_id != ConsoleGlobal.faction_id_) {
label_5a0852:
                local_result =
                    (Vehs[actor_vehicle_id].probe_action & 8u) >> 3;
            } else if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0 &&
                       (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0) {
                const int second_build_morale =
                    morale_veh(actor_vehicle_id, 1, 0);
                if (success_rates(2, second_build_morale, 1,
                                  target_base_id) != 0) {
                    const char *build_image = second_build_morale
                        ? "al_infil_sm.pcx" : "infil_sm.pcx";
                    local_result = popp(ScriptPopupFile, "ADVVIRUS", 0,
                                        build_image, nullptr);
                    native_probe_roll_threshold = local_result;
                } else {
                    local_result = 0;
                }
                Vehs[actor_vehicle_id].probe_action =
                    static_cast<uint8_t>(
                        Vehs[actor_vehicle_id].probe_action |
                        (local_result << 3));
            } else {
                goto label_5a0852;
            }
            if (local_result == 0) goto label_5a2dcd;
            // Native sets EBP-0x30 to one at 0x5A0877 when the saved
            // selector's bit-3 continuation reaches this successful branch.
            native_probe_roll_threshold = 1;
            if (local_result != 0 &&
                actor_faction_id == ConsoleGlobal.faction_id_ &&
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0 &&
                (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0) {
                popup.start(PopupStartCaption, "VIRUS", -1, nullptr, 0,
                            nullptr);
                const int first_vehicle = veh_at(
                    Bases[target_base_id].x, Bases[target_base_id].y);
                if (first_vehicle >= 0 &&
                    stack_check(first_vehicle, 2, 5, -1, -1) != 0) {
                    popup.dialogs().item(StringTable.get(0x62), 0x62);
                }
                local_context = 1;
                goto label_5a0917;
            }
            local_project_cursor =
                static_cast<int8_t>(Vehs[actor_vehicle_id].probe_sabotage_id);
            native_probe_roll_threshold = local_project_cursor;
            goto label_5a0aff;
        }
        goto label_5a0917_done;

label_5a0917:
        {
            // Native facility-choice popup. The facility menu uses the
            // existing Base::facilities_built byte array; 0x1A is included
            // only on the native high-altitude terrain path, and id 0x63 is
            // always appended after the translated production-name option.
            for (int choice = 1; choice < 0x46; ++choice) {
                if (choice == 1) continue;
                if (choice == 0x1A &&
                    (map_loc(Bases[target_base_id].x,
                             Bases[target_base_id].y)->climate & 0xE0) < 0x60)
                    continue;
                if (choice >= 0x41) continue;

                int facility_byte = 0;
                int facility_mask = 0;
                bitmask(choice, &facility_byte, &facility_mask);
                if ((Bases[target_base_id].facilities_built[facility_byte] &
                     facility_mask) != 0) {
                    popup.dialogs().item(StringTable.get(choice), choice);
                }
            }

            StringTemp[0] = '\0';
            strcat(StringTemp, selector2_prepared_text);
            strcat(StringTemp, " ");
            strcat(StringTemp, "(");
            strcat(StringTemp, StringTable.get(0xF9));
            strcat(StringTemp, ")");
            popup.dialogs().item(StringTemp, 0);
            parse_it("ABORTSTRING", "SCRIPT");
            popup.dialogs().item(StringTemp, 0x63);

            local_project_cursor = popup.exec(0, nullptr);
            if (local_project_cursor < 0) {
                goto label_5a2ce9;
            }
            Vehs[actor_vehicle_id].probe_sabotage_id =
                static_cast<uint8_t>(local_project_cursor);
            native_probe_roll_threshold = local_project_cursor;
            goto label_5a0aff;
        }

label_5a0aff:
        {
            // Native caches the selector byte and tests 0x63 before any
            // facility lookups. The existing typed selector owner preserves
            // the distinct abort/empty-stack exits and its notification,
            // storage, BaseWin check and redraw order.
            if (local_project_cursor == 0x63) {
                NetMsgPrimary.pop("VIRUSABORT", 0);
                goto label_5a5816;
            }

            int facility_byte = 0;
            int facility_mask = 0;
            bitmask(1, &facility_byte, &facility_mask);
            const bool facility_option_available =
                (Bases[target_base_id].facilities_built[facility_byte] &
                 facility_mask) != 0;
            const uint32_t actor_bit =
                1u << (static_cast<uint32_t>(actor_faction_id) & 31u);
            const bool report_facility_probe =
                actor_faction_id == ConsoleGlobal.faction_id_ &&
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0 &&
                (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0;
            if (!facility_option_available &&
                local_project_cursor != 4 && local_project_cursor != 5) {
                goto label_5a2dcd;
            }
            if (report_facility_probe) {
                parse_says(0, Bases[target_base_id].name_string, -1, -1);
                if (facility_option_available) {
                    parse_says(1,
                        Players[target_faction_id].adj_name_faction, -1, 1);
                    const int facility_morale =
                        morale_veh(actor_vehicle_id, 1, 0);
                    const int facility_success = success_rates(
                        2, facility_morale, 1, target_base_id);
                    const char *const facility_image = facility_success
                        ? "al_infil_sm.pcx" : "infil_sm.pcx";
                    if (popp(ScriptPopupFile, "HQVIRUS", 0,
                             facility_image, nullptr) == 0) {
                        goto label_5a2ce9;
                    }
                } else {
                    parse_say(1, static_cast<int>(reinterpret_cast<uintptr_t>(
                        Facility[local_project_cursor].name)), -1, -1);
                    const int facility_morale =
                        morale_veh(actor_vehicle_id, 1, 0);
                    const int facility_success = success_rates(
                        2, facility_morale, 1, target_base_id);
                    const char *const facility_image = facility_success
                        ? "al_infil_sm.pcx" : "infil_sm.pcx";
                    if (popp(ScriptPopupFile, "MILVIRUS", 0,
                             facility_image, nullptr) == 0) {
                        goto label_5a2ce9;
                    }
                    // Native's successful 0x5A0D16 MILVIRUS confirmation
                    // writes two to this same EBP-0x30 local at 0x5A0D96.
                    native_probe_roll_threshold = 2;
                }
            }

            local_result = probe_native_selector2_storage_action(
                local_project_cursor, target_base_id, actor_faction_id,
                target_faction_id, actor_is_alien,
                selector2_prepared_text, actor_vehicle_id, 1);
            if (local_result == -1 || local_result == -2)
                goto label_5a5816;
            goto label_5a36ee;
        }
        }
label_5a0917_done:

        if (action_result == 5 || action_result == 8) {
            const int success_morale =
                morale_veh(actor_vehicle_id, 1, 0);
            const int assault_success =
                success_rates(1, success_morale, 1, target_base_id);
            const bool actor_can_report =
                actor_faction_id == ConsoleGlobal.faction_id_ &&
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0 &&
                (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0;
            if (assault_success != 0) {
                if (!actor_can_report) goto label_5a2dcd;
                const char *const image = success_morale
                    ? "al_infil_sm.pcx" : "infil_sm.pcx";
                const char *const label =
                    action_result == 5 ? "ASSASSIN" : "FREEWILLY";
                if (popp(ScriptPopupFile, label, 0, image, nullptr) != 0)
                    goto label_5a2dcd;
            } else if (actor_can_report) {
                NetMsgPrimary.pop(action_result == 5
                    ? "NOASSASSIN" : "NOFREEWILLY", -5000, 0, nullptr);
            }
            goto label_5a2ce9;
        }

        if (action_result == -1) {
            if (probe_native_negative_selector_vehicle_guard(
                    fallback_vehicle_id)) {
                goto label_5a2ce9;
            }
            const int fallback_proto_id =
                Vehs[fallback_vehicle_id].proto_id;
            const int fallback_chassis_id =
                VehPrototypes[fallback_proto_id].chassis_id;
            if (Chassis[fallback_chassis_id].triad == 2 &&
                !has_abil(actor_proto_id, 0x20)) {
                if (actor_faction_id == ConsoleGlobal.faction_id_ && mode != 0 &&
                    (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0 &&
                    (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0) {
                    parse_say(0, static_cast<int>(reinterpret_cast<intptr_t>(
                        Ability[abil_index(0x20)].name)), -1, -1);
                    NetMsgPrimary.pop("PROBEABILSUPERIORITY", 5000, 0,
                                      nullptr);
                }
                goto label_5a2ce9;
            }

            // Native 0x5A142B..0x5A20CF is the early negative-selector path.
            // It is distinct from the later 0x5A4101 reassignment tail. The
            // base cursor is the first matching target-faction base. A miss
            // retains BaseCurrentCount; only an empty table gives 12.
            if (probe_native_negative_selector_eligible(
                    target_faction_id, fallback_proto_id)) {
                const int base_distance =
                    probe_native_negative_selector_base_distance(
                        fallback_vehicle_id, target_faction_id);
                int amount = probe_native_negative_selector_amount(
                    target_faction_id, fallback_proto_id, base_distance);
                amount = probe_native_negative_selector_target_ratio(
                    amount, actor_faction_id, target_faction_id,
                    (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0,
                    (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u <<
                        (static_cast<uint32_t>(target_faction_id) & 31u))) != 0);

                // Project protection is tested after the preceding amount
                // work. The native protected route may notify the local
                // actor, but always takes the ordinary zero-return cleanup.
                if (probe_native_negative_selector_protected(
                    target_faction_id)) {
                    if (actor_faction_id == ConsoleGlobal.faction_id_ &&
                        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0 && mode != 0 &&
                        (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0) {
                        parse_says(0,
                            Players[target_faction_id].noun_faction,
                            -1, -1);
                        NetMsgPrimary.pop("MINDIMMUNITY2", -5000, 0,
                                          nullptr);
                    }
                    goto label_5a2ce9;
                }

                // Native applies ordered type-0x10 actor bonuses and the
                // Nethack Terminus amount adjustment only on the unprotected
                // branch. The distance's Veh+0x2E lookup already ran above.
                amount = probe_native_negative_selector_adjust_amount(
                    amount, actor_faction_id);
                const int native_score_policy =
                    PlayersData[target_faction_id].soc_effect_active.values_[SocialEffect::PROBE];
                BOOL native_territory_mismatch = 0;
                amount = probe_native_strategy_adjusted_amount(
                    amount, fallback_vehicle_id, native_score_policy);
                // Native parses the probe unit name, then takes the local
                // human-actor/vehicle-state early edge before any strategic
                // table calls. Veh::state bit 9 is the tested DH bit from
                // the dword at the vehicle's +4 state field.
                // The conjunction tested at native 0x5A1821..0x5A1838
                // branches to 0x5A1D9E, not to Popup cleanup. That edge skips
                // the strategy-score core and its two actor-energy compares,
                // then shares the mode/selector continuation below.
                const bool skip_native_strategy_score =
                    probe_native_strategy_human_vehicle_early_exit(
                        GameDataStateStorage.fields_.lock_masks_.enable_mask_, actor_faction_id,
                        Vehs[actor_vehicle_id]);
                if (!skip_native_strategy_score) {
                    const int native_strategy_score =
                        probe_native_strategy_score_core(
                            amount, actor_faction_id, fallback_vehicle_id,
                            target_faction_id, native_territory_mismatch);
                    // Native compares both values against one actor-row
                    // energy read at 0x5A1C9A; EAX remains live for the second
                    // compare at 0x5A1D20. The initial amount uses target
                    // reserves, but this later UI/return gate does not.
                    const int actor_energy_at_strategy_join =
                        PlayersData[actor_faction_id].energy_reserves;
                    if (native_territory_mismatch || native_strategy_score >
                            actor_energy_at_strategy_join ||
                        amount > actor_energy_at_strategy_join)
                        goto label_5a2ce9;
                }

                // Native's 0x5A1D9E no-chooser edge takes the early
                // BasePop return continuation at 0x5A1E28. Preserve it as a
                // separate source label from the later common Popup return;
                // the single automatic Popup still owns the actual typed
                // close/Scroll/BasePop teardown on either return.
                if (mode == 0) goto label_5a1e28;

                // The remaining native selector path selects the action from
                // the saved probe byte for a remote actor. Local human
                // execution performs both success-rate calls before checking
                // the actor's energy, and changes energy only after the
                // selected action and second lock-state bit allow accounting.
                if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0) {
                    int chosen_policy =
                        actor_faction_id == ConsoleGlobal.faction_id_ &&
                        (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0
                        ? -1
                        : ((Vehs[actor_vehicle_id].probe_action & 8u)
                            != 0 ? 2 : 1);
                    if (chosen_policy < 0) {
                        probe_native_subvert_preflight(
                            actor_vehicle_id, target_base_id,
                            amount,
                            PlayersData[actor_faction_id].energy_reserves);
                        chosen_policy = probe_native_subvert_menu(
                            popup, amount, actor_faction_id);
                        if (chosen_policy <= 0) goto label_5a2ce9;
                        Vehs[actor_vehicle_id].probe_action =
                            static_cast<uint8_t>(
                                Vehs[actor_vehicle_id].probe_action |
                                ((chosen_policy + 0x1F) << 3));
                    }
                    // Native stores the selected value minus one into its
                    // persistent EBP-0x30 local; it does not reload bit 3 as
                    // a boolean. For the remote path chosen_policy already
                    // came from that saved byte, so the same store applies.
                    native_probe_roll_threshold = chosen_policy - 1;
                } else {
                    const int actor_energy =
                        PlayersData[actor_faction_id].energy_reserves;
                    const uint32_t target_bit = 1u <<
                        (static_cast<uint32_t>(target_faction_id) & 31u);
                    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & target_bit) != 0) {
                        int reserve_margin = actor_energy / 10;
                        if (reserve_margin < 0) reserve_margin = 0;
                        if (reserve_margin > 0x32) reserve_margin = 0x32;
                        const int reserve_demand = static_cast<int32_t>(
                            static_cast<uint32_t>(amount) +
                            static_cast<uint32_t>(reserve_margin) + 2u);
                        if (reserve_demand > actor_energy)
                            goto label_5a2574;
                    } else {
                        if (!probe_native_negative_selector_target_can_afford(
                                target_faction_id, amount))
                    goto label_5a2574;
                    }
                }

                if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) == 0)
                    goto label_5a2dcd;
                probe_native_negative_selector_accounting(
                    actor_faction_id, target_faction_id, amount);
                goto label_5a2012;
            }
            if (actor_faction_id == ConsoleGlobal.faction_id_ && mode != 0 &&
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0 &&
                (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0)
                NetMsgPrimary.pop("ALIENPROBE", -5000, 0, nullptr);
            goto label_5a2ce9;
        }

        if (action_result == 6) {
            control_chance =
                mind_control(target_base_id, actor_faction_id, 1);
            if (control_chance < 0) {
                if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0) {
                    goto label_5a2ce9;
                }
                action_result = 1;
                goto label_59fb67;
            }

            if ((Players[target_faction_id].rule_flags & 0x800) != 0) {
                if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) == 0)
                    goto label_5a2574;
                if (actor_faction_id == ConsoleGlobal.faction_id_ &&
                    (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0) {
                    parse_says(0, Players[target_faction_id].noun_faction,
                               -1, -1);
                    NetMsgPrimary.pop("MINDIMMUNITY", -5000, 0, nullptr);
                }
                goto label_5a2ce9;
            }

            for (int bonus = 0;
                 bonus < Players[actor_faction_id].faction_bonus_count;
                 ++bonus) {
                if (Players[actor_faction_id].faction_bonus_id[bonus] == 0x10) {
                    control_chance = probe_native_wrapped_product(
                        control_chance,
                        Players[actor_faction_id].faction_bonus_val1[bonus]) /
                        100;
                }
            }
            control_chance = probe_native_nethack_terminus_adjust_amount(
                control_chance, actor_faction_id);
            const int cloudbase_academy_base = GameDataStateStorage.fields_.secret_project_.cloudbase_academy();
            if (cloudbase_academy_base >= 0 &&
                Bases[cloudbase_academy_base].faction_id_current ==
                    actor_faction_id) {
                const int tripled_control_chance =
                    probe_native_wrapped_product(control_chance, 3);
                control_chance = (tripled_control_chance +
                    ((tripled_control_chance >> 31) & 3)) >> 2;
            }
            // The target policy is the active probe field (+0x298), separate
            // from the temporary social-effect talent member.
            int social_modifier =
                PlayersData[target_faction_id].soc_effect_active.values_[SocialEffect::PROBE];
            if (has_fac(0x16, target_base_id, 0)) --social_modifier;
            if (has_fac(0x22, target_base_id, 0)) social_modifier += 2;
            if (social_modifier + 2 < 6) {
                if (social_modifier == 3) {
                    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) == 0)
                        goto label_5a2455;
                    if (actor_faction_id == ConsoleGlobal.faction_id_ &&
                        (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) != 0) {
                        parse_says(0, Players[target_faction_id].noun_faction,
                                   -1, -1);
                        NetMsgPrimary.pop("IMMUNITY", -5000, 0, nullptr);
                    }
                    goto label_5a2ce9;
                }
                control_chance = probe_native_social_amount_prefix(
                    control_chance, social_modifier);
            }

            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & actor_bit) != 0) {
label_5a2ab3:
                if (actor_faction_id != ConsoleGlobal.faction_id_ ||
                    (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u) == 0) {
                    local_result =
                        (Vehs[actor_vehicle_id].probe_action & 8u) != 0
                        ? 2 : 1;
                    // Native 0x5A2D33 reloads the saved byte and normalizes
                    // bit 3 to EAX 2/1 before the shared >1 test at 0x5A2D4C.
                    // A clear bit leaves the path-local threshold untouched.
                    if (local_result > 1)
                        native_probe_roll_threshold = 1;
                } else {
                    parse_says(0, Bases[target_base_id].name_string, -1, -1);
                    parse_num(1, control_chance);
                    parse_num(2,
                        PlayersData[target_faction_id].energy_reserves);
                    const int first_control_morale =
                        morale_veh(actor_vehicle_id, 1, 0);
                    success_rates(1, first_control_morale, -1,
                                  target_base_id);
                    const int second_control_morale =
                        morale_veh(actor_vehicle_id, 1, 0);
                    success_rates(2, second_control_morale, 1,
                                  target_base_id);
                    if (PlayersData[actor_faction_id].energy_reserves <
                        control_chance) {
                        wave_it(9);
                        NetMsgPrimary.pop("THOUGHTCONTROL", -5000, 0, nullptr);
                        goto label_5a2ce9;
                    }
                    wave_it(0x20);
                    popup.start(PopupStartCaption, "THOUGHTCONTROL", -1,
                                nullptr, 0, nullptr);
                    popup.start(PopupStartCaption, "THOUGHTMENU", -1,
                                nullptr, 0x80, nullptr);
                    const int thought_choice = popup.exec(0, nullptr);
                    if (thought_choice <= 0) {
                        goto label_5a2ce9;
                    }
                    Vehs[actor_vehicle_id].probe_action =
                        static_cast<uint8_t>(
                        Vehs[actor_vehicle_id].probe_action |
                            ((thought_choice + 0x1F) << 3));
                    if (thought_choice >= 2) local_result = 1;
                    // BasePop::exec's raw signed EAX survives the selector
                    // byte update to 0x5A2D4C. Native writes one only when
                    // the raw result is greater than one; result one leaves
                    // the prior path-local threshold unchanged.
                    if (thought_choice > 1)
                        native_probe_roll_threshold = 1;
                }
            }
            if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) == 0)
                goto label_5a2dcd;
            PlayersData[actor_faction_id].energy_reserves = static_cast<int32_t>(
                static_cast<uint32_t>(
                    PlayersData[actor_faction_id].energy_reserves) -
                static_cast<uint32_t>(control_chance));
            PlayersData[actor_faction_id].mind_control_total += 4;
            PlayersData[target_faction_id]
                .diplo_mind_control[actor_faction_id] += 4;
            goto label_5a2012;
        }

label_5a2012:
        // Native debits before entering 0x5A2012; the shared join only
        // refreshes local-console data for the local actor.
        if (actor_faction_id == ConsoleGlobal.faction_id_)
            ConsoleGlobal.update_data(0);
        goto label_5a2dcd;

label_5a2455:
        action_result = 1;
        goto label_59fb67;

label_5a2dcd:
        {
        // Connected beginning of the native shared result/accounting region
        // (0x5A2DCD onward). The source keeps its byte-oriented faction-state
        // reads: byte zero is the human bitfield and byte one is the alive
        // bitfield in the existing LockEnableMask owner.
        local_context = 0;
        // The native writes zero to its reused fourth-argument local at the
        // dispatcher entry; no-chooser selectors must not retain input mode.
        native_relationship_choice = 0;
        native_human_bits =
            reinterpret_cast<const uint8_t *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_)[0];
        native_alive_bits =
            reinterpret_cast<const uint8_t *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_)[1];
        native_actor_bit = static_cast<uint8_t>(1u <<
            (static_cast<uint32_t>(actor_faction_id) & 31u));
        native_target_bit = static_cast<uint8_t>(1u <<
            (static_cast<uint32_t>(target_faction_id) & 31u));

        // At 0x5A2DCD only selectors 1..5 and 8 enter the chooser envelope.
        // Native routes only selectors 1..5 and 8 through this producer, and
        // only for human actors. AI actors bypass it. A human actor probing
        // a nonhuman target reuses the packed high selector byte; two human
        // factions enter the local Dialogs/BasePop path only for the current
        // actor and a live network action.
        if (probe_native_selector_uses_faction_choice(action_result) &&
            (native_human_bits & native_actor_bit) != 0) {
            if ((native_human_bits & native_target_bit) == 0) {
                const int chosen_faction =
                    (Vehs[actor_vehicle_id].probe_action >> 5) & 7;
                local_context = chosen_faction;
                native_relationship_choice = chosen_faction;
                if (chosen_faction != 0)
                    ++native_probe_roll_threshold;
            } else if (actor_faction_id == ConsoleGlobal.faction_id_ &&
                       (NetDaemonNet.lock_state_[1].leading_words_[3] & 1u)
                           != 0) {
                const int chosen_faction = probe_native_result_one_choices(
                    popup, actor_faction_id, target_faction_id,
                    actor_vehicle_id, target_base_id,
                    native_probe_roll_threshold);
                // Keep the raw BasePop::exec result, including -1; the
                // original shifts its low byte into probe_action then uses
                // this signed value again at 0x5A4B15.
                local_context = chosen_faction;
                native_relationship_choice = chosen_faction;
                if (chosen_faction != 0)
                    ++native_probe_roll_threshold;
            }
        } else if (probe_native_selector_uses_faction_choice(action_result)) {
            // The complementary native path is not an AI bypass: it may
            // carry the global ranked human faction into the later directed
            // relationship response without opening a Popup.
            const int chosen_faction = probe_native_automatic_faction_choice(
                actor_vehicle_id, actor_faction_id, target_faction_id,
                native_probe_roll_threshold, native_human_bits,
                native_alive_bits);
            local_context = chosen_faction;
            native_relationship_choice = chosen_faction;
            if (chosen_faction != 0)
                ++native_probe_roll_threshold;
        }

        // Native 0x5A2EB2 encloses all selector-specific successful
        // accounting in lock-state bit 1 and runs this first probability gate
        // before those mutations. A failed roll goes directly to the shared
        // relationship response, not through an action helper.
        if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) == 0)
            goto label_5a4b15;
        if (!probe_native_success_roll_passes(
                action_result, actor_vehicle_id, target_base_id,
                target_faction_id, native_probe_roll_threshold, 1))
            goto label_5a4b15;

        // Native 0x5A2EB2 begins the accounting dispatch. Action one owns the
        // directed stolen-data counter and technology transfer/report path;
        // subsequent selectors continue in the following candidate region.
        if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0 &&
            action_result == 1) {
            probe_native_result_one_complete(
                actor_faction_id, target_faction_id, target_base_id,
                local_technology_cursor, actor_is_alien);
            // The following native label is 0x5A36EE; this source candidate
            // keeps its next selector accounting as an explicit continuation.
            goto label_5a36ee;
        }

        if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0 &&
            action_result == 3) {
            local_result = probe_native_energy_transfer_action(
                actor_faction_id, target_faction_id, target_base_id,
                actor_vehicle_id);
            goto label_5a36ee;
        }

        if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0 &&
            action_result == 4) {
            native_probe_roll_threshold = 1;
            probe_native_assimilation_action(target_base_id,
                                              actor_faction_id,
                                              target_faction_id);
            goto label_5a36ee;
        }

        if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0 &&
            action_result == 5) {
            local_result = probe_native_tech_theft_action(
                target_faction_id, actor_faction_id, target_faction_id,
                target_base_id, actor_is_alien);
            native_probe_roll_threshold = local_technology_cursor;
            goto label_5a36ee;
        }

        if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0 &&
            action_result == 7) {
            probe_native_atrocity_followup(
                actor_faction_id, target_faction_id, target_base_id,
                native_flag20_entry_count, native_known_flag20_count,
                actor_is_alien);
            goto label_5a36ee;
        }

        if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0 &&
            action_result == 8) {
            // The native selector-8 seed comes from the high three bits of
            // the vehicle's saved probe action. Its chooser/commit helper
            // owns the original Popup lifetime interaction, six-record list,
            // selected-player setup and treaty/accounting side effects.
            const int initial_monument_selection =
                (Vehs[actor_vehicle_id].probe_action >> 5) & 7;
            local_result = probe_native_commit_monument_choice(
                popup, actor_faction_id, target_faction_id,
                actor_vehicle_id, initial_monument_selection,
                native_alive_bits, actor_is_alien);
            if (local_result < 0) {
                goto label_5a2ce9;
            }
            local_context = local_result;
            native_relationship_choice = local_result;
            goto label_5a36ee;
        }

        if ((NetDaemonNet.lock_state_[1].leading_words_[3] & 2u) != 0 &&
            action_result == -1) {
            // Native 0x5A4101 reassigns the third-argument vehicle after the
            // shared first roll gate. The persistent EBP-0x30 value chooses
            // between the ordinary Popup return and the direct 0x5A4B28
            // continuation; that continuation deliberately bypasses the
            // second roll/relocation region and the 0x5A4B15 zero-result test.
            if (probe_native_reassign_prototype_action(
                    actor_faction_id, target_faction_id,
                    fallback_vehicle_id, native_probe_roll_threshold,
                    actor_is_alien))
                goto label_5a5816;
            goto label_5a4b28;
        }

        // Unhandled selectors retain the native fallthrough to the shared
        // post-selector gate. Action-specific joins above arrive here with
        // their mutations already applied.
label_5a36ee:
        {
        // Native's shared post-selector gate keeps its original order:
        // optional HSA/facility adjustment, target probe-society clamp,
        // actor morale read, then rnd compared against the persistent
        // EBP-0x30 value. This intentionally runs after selectors that jump
        // here, including their earlier state changes and messages.
        const bool native_probe_roll_succeeded =
            probe_native_success_roll_passes(
                action_result, actor_vehicle_id, target_base_id,
                target_faction_id, native_probe_roll_threshold, 0);

        int relocation_base = -1;
        bool native_sea_port_scan = false;
        if (native_probe_roll_succeeded) {
            relocation_base = probe_native_relocation_base(
                actor_x, actor_y, actor_faction_id, actor_chassis_id,
                native_sea_port_scan);
            // On the initial successful-roll sea/port-search path, an HSA
            // target plus odd tick forces the common failed-destination edge.
            if (ProbeTargetHasHSA != 0 && native_sea_port_scan &&
                (timeGetTime() & 1u) != 0)
                relocation_base = -1;
        } else if (ProbeTargetHasHSA == 0 &&
                   ProbeHasAlgoEnhancement != 0 &&
                   (timeGetTime() & 1u) != 0) {
            // Native's failed-roll retry is restricted to the non-HSA actor
            // with the algorithm enhancement and the odd-tick branch.
            relocation_base = probe_native_relocation_base(
                actor_x, actor_y, actor_faction_id, actor_chassis_id,
                native_sea_port_scan);
        }

        if (relocation_base < 0 && action_result == 8 &&
            VehPrototypes[Vehs[actor_vehicle_id].proto_id].weapon_id !=
                WPN_PROBE_TEAM)
            relocation_base = initial_actor_home_base_id;

        if (relocation_base < 0) {
            // The native -1 continuation kills the active probe vehicle
            // unless the selector-8/non-probe-weapon home-base exception
            // supplied a destination above.
            veh_kill(actor_vehicle_id);
        } else {
            {
                veh_put(actor_vehicle_id, Bases[relocation_base].x,
                        Bases[relocation_base].y);
                // Native clears the existing terraforming-turn byte after
                // placing the vehicle; it then redraws the old stack and the
                // destination independently.
                Vehs[actor_vehicle_id].terraforming_turns = 0;
                draw_tiles(actor_x, actor_y, 2);
                draw_tile(Bases[relocation_base].x,
                          Bases[relocation_base].y, 2);
                parse_says(1, Bases[relocation_base].name_string, -1, -1);
                // Native saves this pre-increment morale in EAX-local state,
                // mutates the vehicle, and only then performs the local
                // notification parser calls. Do not move the parser read
                // ahead of the native state change: morale is live input.
                const int morale_before_relocation_bonus =
                    morale_veh(actor_vehicle_id, 1, 0);
                Vehs[actor_vehicle_id].morale = static_cast<uint8_t>(range(
                    static_cast<int>(Vehs[actor_vehicle_id].morale) + 1,
                    0, 6));
                veh_skip(actor_vehicle_id);
                if (actor_faction_id == ConsoleGlobal.faction_id_) {
                    parse_says(0,
                        VehPrototypes[Vehs[actor_vehicle_id].proto_id].veh_name,
                        -1, -1);
                    const int relocation_morale =
                        morale_veh(actor_vehicle_id, 1, 0);
                    const int native_relocation_parse_result = parse_say(
                        2, static_cast<int>(reinterpret_cast<intptr_t>(
                            Morale[relocation_morale].name)), -1, -1);
                    clear();
                    says("PROBESURVIVAL");
                    if (action_result != 8) {
                        if (morale_before_relocation_bonus != relocation_morale &&
                            relocation_morale >= 1)
                            say_num(1);
                        else
                            say_num(0);
                    } else {
                        says("0");
                    }
                    NetMsgPrimary.pop(
                        native_relocation_parse_result != 0
                            ? "al_infil_sm.pcx" : "infil_sm.pcx",
                        StringTemp);
                }
            }
        }

        // Selector 6 follows the shared relocation test before the native
        // 0x5A4B15 relationship return-vs-continue join.
        if (action_result == 6) {
            // Selector 6 reaches this block only after the native relocation
            // path has completed; its separate spying/GameState eligibility
            // is retained from the original branches.
            const uint32_t local_faction_bit =
                1u << (static_cast<uint32_t>(ConsoleGlobal.faction_id_) & 31u);
            const bool selector6_has_local_context =
                actor_faction_id == ConsoleGlobal.faction_id_ ||
                target_faction_id == ConsoleGlobal.faction_id_ ||
                spying(actor_faction_id) != 0 ||
                spying(target_faction_id) != 0 ||
                (GameDataStateStorage.fields_.game_state_ & 0x80u) != 0;
            if (selector6_has_local_context) {
                if ((GameDataStateStorage.fields_.game_state_ & 0x80u) != 0 ||
                    Bases[target_base_id].faction_id_current ==
                        ConsoleGlobal.faction_id_ ||
                    (Bases[target_base_id].visibility & local_faction_bit) != 0) {
                    ConsoleGlobal.focus(Bases[target_base_id].x,
                                        Bases[target_base_id].y,
                                        Bases[target_base_id].faction_id_current);
                }

                parse_says(0, get_title(actor_faction_id), -1, -1);
                parse_says(1, get_name(actor_faction_id), -1, -1);
                char *const actor_noun = get_noun(actor_faction_id);
                parse_says(2, actor_noun, -1, -1);
                parse_says(3, Bases[target_base_id].name_string, -1, -1);
                parse_says(4, get_title(target_faction_id), -1, -1);
                parse_says(5, get_name(target_faction_id), -1, -1);
                clear();
                says("MINDCONTROL");
                says(actor_faction_id == ConsoleGlobal.faction_id_
                    ? "0" : target_faction_id == ConsoleGlobal.faction_id_
                        ? "1" : "2");
                X_pop(StringTemp, actor_noun != nullptr
                    ? "al_cap_sm.pcx" : "capture_sm.pcx", nullptr);

                for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
                     vehicle_id >= 0; --vehicle_id) {
                    Veh &vehicle = Vehs[vehicle_id];
                    if (vehicle.faction_id != target_faction_id) continue;
                    const int distance = vector_dist(
                        vehicle.x, vehicle.y, Bases[target_base_id].x,
                        Bases[target_base_id].y);
                    if (distance > 1) continue;
                    if (distance != 0 &&
                        base_who(vehicle.x, vehicle.y) >= 0)
                        continue;

                    const int old_prototype_id = vehicle.proto_id;
                    uint8_t &target_count = PlayersData[target_faction_id]
                        .proto_id_active[old_prototype_id];
                    target_count = static_cast<uint8_t>(target_count - 1u);
                    if (old_prototype_id > 0x40) {
                        VehPrototype &old_prototype =
                            VehPrototypes[old_prototype_id];
                        const int replacement = propose_proto(
                            actor_faction_id, old_prototype.chassis_id,
                            old_prototype.weapon_id, old_prototype.armor_id,
                            old_prototype.ability_flags,
                            old_prototype.reactor_id, -2,
                            old_prototype.veh_name);
                        if (replacement >= 0)
                            vehicle.proto_id = static_cast<int16_t>(replacement);
                    }

                    const int current_prototype_id = vehicle.proto_id;
                    uint8_t &actor_count = PlayersData[actor_faction_id]
                        .proto_id_active[current_prototype_id];
                    const int x = vehicle.x;
                    const int y = vehicle.y;
                    vehicle.faction_id = static_cast<uint8_t>(actor_faction_id);
                    vehicle.home_base_id = static_cast<int16_t>(target_base_id);
                    vehicle.moves_expended = 0;
                    ++actor_count;
                    if (Chassis[VehPrototypes[current_prototype_id].chassis_id]
                            .range != 0)
                        vehicle.terraforming_turns = 0;
                    if (vehicle.order != ORDER_HOLD)
                        vehicle.order = ORDER_NONE;
                    owner_set(x, y, actor_faction_id);
                    vehicle.visibility = static_cast<uint8_t>(
                        vehicle.visibility | (1u << target_faction_id));
                }

                if (native_probe_roll_threshold == 0)
                    act_of_aggression(actor_faction_id, target_faction_id);
                capture_base(target_base_id, actor_faction_id, 1);
                if (actor_faction_id == ConsoleGlobal.faction_id_ &&
                    relocation_base < 0) {
                    NetMsgPrimary.pop(
                        actor_is_alien ? "al_cap_sm.pcx" : "capture_sm.pcx",
                        action_result == 6 ? "PROBESUCCUMB" : "PROBECAUGHT");
                }
            }
        }

        }
label_5a4b15:
        // Native label 0x5A4B15 branches directly to the Popup-owning common
        // exit when the result is zero. For a nonzero result it first checks
        // the actor-row selector-7 bit and the two directed target-row bits;
        // retain the original directions and short-circuit call order.
        if (action_result == 0) goto label_5a5816;
label_5a4b28:
        if (has_treaty(actor_faction_id, target_faction_id, 7) &&
            has_treaty(target_faction_id, actor_faction_id, 1) &&
            has_treaty(target_faction_id, actor_faction_id, 0x02000000)) {
            goto label_5a5816;
        }

        // Native keeps [EBP+0x14] signed and distinct from the selector byte
        // in Veh. Negative values enter the target-indexed response path.
        const bool native_skip_relationship_choice =
            action_result == 6 && native_probe_roll_threshold != 0;
        if (native_skip_relationship_choice) {
            set_treaty(target_faction_id, actor_faction_id, 0x420, 1);
            goto label_5a5816;
        }
        const int selected_response_faction = native_relationship_choice <= -1
            ? probe_policy_signed32(
                0u - static_cast<uint32_t>(native_relationship_choice))
            : 0;
        const uint32_t selected_response_bit =
            1u << (static_cast<uint32_t>(selected_response_faction) & 31u);

        if (native_relationship_choice <= -1) {
            // Native 0x5A4B9A..0x5A4C34 performs the negative-choice vendetta
            // branch only when the selected faction bit is clear in the
            // existing lock/human mask. Keep the byte mask and directed
            // treaty writer and local FRAMEFLOP parser/message path as seen.
            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & selected_response_bit) == 0) {
                if (actor_faction_id == ConsoleGlobal.faction_id_) {
                    parse_says(0,
                        get_title(selected_response_faction), -1, -1);
                    parse_says(1,
                        get_name(selected_response_faction), -1, -1);
                    NetMsgPrimary.pop(actor_is_alien
                        ? "al_cap_sm.pcx" : "capture_sm.pcx",
                        "FRAMEFLOP");
                }
                treaty_on(selected_response_faction, actor_faction_id, 0x10);
                // The native store at 0x5A4C39 negates its signed local only
                // on this nonhuman response edge; the human-mask path keeps
                // the negative value for its following branch.
                native_relationship_choice = selected_response_faction;
            }
        }
        if (native_relationship_choice <= -1 &&
            (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & selected_response_bit) != 0) {
                // The human-partner response consumes parser slots in this
                // exact native order before either multiplayer agenda update
                // or the local treaty response. Preserve repeated reads and
                // the actor/target direction; these are not normalized into
                // one faction-name helper because parser defaults are live.
                parse_says(0, get_title(actor_faction_id), -1, -1);
                parse_says(1, get_name(actor_faction_id), -1, -1);
                parse_says(2, get_pact_hood(selected_response_faction,
                                            actor_faction_id), -1, -1);
                parse_says(3, get_noun(actor_faction_id), -1, -1);
                parse_says(4, get_his_her(actor_faction_id, 0), -1, -1);
                parse_says(5, get_him_her(actor_faction_id, 0), -1, -1);
                parse_says(6, get_title(target_faction_id), -1, -1);
                parse_says(7, get_name(target_faction_id), -1, -1);
                parse_says(8, get_noun(target_faction_id), -1, -1);

                if (IsMultiplayerNet != 0) {
                    agenda_on(selected_response_faction, actor_faction_id,
                              0x20);
                    if (-ConsoleGlobal.faction_id_ ==
                        native_relationship_choice) {
                        NetMsgPrimary.pop(nullptr, "HAVEFRAMEEXCUSE");
                    }
                    if (actor_faction_id == ConsoleGlobal.faction_id_) {
                        parse_says(0,
                            get_title(selected_response_faction), -1, -1);
                        parse_says(1,
                            get_name(selected_response_faction), -1, -1);
                        parse_says(2,
                            get_noun(selected_response_faction), -1, -1);
                        NetMsgPrimary.pop(actor_is_alien
                            ? "al_cap_sm.pcx" : "capture_sm.pcx",
                            "HASFRAMEEXCUSE");
                    }
                } else if (has_treaty(selected_response_faction,
                                     actor_faction_id, 1)) {
                    const char *const image = actor_is_alien
                        ? "al_cap_sm.pcx" : "capture_sm.pcx";
                    if (X_pop(const_cast<char *>(image),
                              "PACTFRAMEEXCUSE", nullptr) != 0) {
                        pact_ends(selected_response_faction,
                                  actor_faction_id);
                        PlayersData[selected_response_faction]
                            .diplo_spoke[actor_faction_id] = GameDataStateStorage.fields_.turn_current_num_;
                    }
                } else {
                    const char *const image = actor_is_alien
                        ? "al_cap_sm.pcx" : "capture_sm.pcx";
                    if (X_pop(const_cast<char *>(image),
                              "FRAMEEXCUSE", nullptr) != 0) {
                        treaty_on(actor_faction_id,
                                  selected_response_faction, 0x10);
                        PlayersData[selected_response_faction]
                            .diplo_spoke[actor_faction_id] = GameDataStateStorage.fields_.turn_current_num_;
                    }
                }
        }
        if (native_relationship_choice >= 1) {
            // Native 0x5A53C7 writes the selected-faction relationship in
            // two distinct directions: treaty_on(selected,target,0x58), then
            // set_treaty(target,selected,1,0x20). Keep both mutations as
            // their original owners rather than combining masks or rows.
            treaty_on(native_relationship_choice, target_faction_id, 0x58);
            set_treaty(target_faction_id, native_relationship_choice, 1,
                       0x20);
            if (native_relationship_choice == ConsoleGlobal.faction_id_) {
                parse_says(0, get_title(target_faction_id), -1, -1);
                parse_says(1, get_name(target_faction_id), -1, -1);
                char *const target_noun = get_noun(target_faction_id);
                parse_says(2, target_noun, -1, -1);
                NetMsgPrimary.pop(target_noun != nullptr
                    ? "al_infil_sm.pcx" : "infil_sm.pcx", "FRAMEJOB");
            }
            if (actor_faction_id == ConsoleGlobal.faction_id_) {
                parse_says(0, get_title(target_faction_id), -1, -1);
                parse_says(1, get_name(target_faction_id), -1, -1);
                parse_says(2, get_noun(target_faction_id), -1, -1);
                parse_says(3, get_title(native_relationship_choice), -1, -1);
                parse_says(4, get_name(native_relationship_choice), -1, -1);
                char *const selected_noun =
                    get_noun(native_relationship_choice);
                parse_says(5, selected_noun, -1, -1);
                NetMsgPrimary.pop(selected_noun != nullptr
                    ? "al_infil_sm.pcx" : "infil_sm.pcx", "FRAMED");
            }
        } else {
            // Native 0x5A5515..0x5A5810 handles the ordinary nonpositive
            // relationship response after the selected-faction branches.
            // Its diplomacy refresh/caption and parser slots precede the
            // treaty/ranking policy tests.
            diplomacy_check(actor_faction_id, target_faction_id, 0);
            diplomacy_caption(actor_faction_id, target_faction_id);
            parse_says(0, get_title(actor_faction_id), -1, -1);
            parse_says(1, get_name(actor_faction_id), -1, -1);
            parse_says(2, get_pact_hood(actor_faction_id,
                                        target_faction_id), -1, -1);

            const int actor_rank =
                static_cast<int>(PlayersData[actor_faction_id].ranking);
            const int target_rank =
                static_cast<int>(PlayersData[target_faction_id].ranking);
            const int actor_tech_rank =
                PlayersData[actor_faction_id].tech_ranking;
            const int target_tech_rank =
                PlayersData[target_faction_id].tech_ranking;

            if (has_treaty(target_faction_id, actor_faction_id, 1)) {
                if (action_result == 1 &&
                    actor_tech_rank < target_tech_rank && actor_rank <= 5 &&
                    DiploScratchFA3C == 0) {
                    const int friction = static_cast<int>(rnd(4, nullptr));
                    cause_friction(target_faction_id, actor_faction_id,
                                   friction);
                    if (actor_faction_id == ConsoleGlobal.faction_id_) {
                        clear();
                        says("KEPTPACT");
                        say_num(1);
                        x_pops_minimal(StringTemp,
                            FactionDiplomacyPortraits[target_faction_id],
                            nullptr);
                    }
                    goto label_5a5816;
                }

                if (actor_rank < target_rank && actor_rank <= 4) {
                    if (actor_faction_id == ConsoleGlobal.faction_id_)
                        x_pops_minimal("EXCUSEDPACT",
                            FactionDiplomacyPortraits[target_faction_id],
                            nullptr);
                    goto label_5a5816;
                }

                if (actor_faction_id == ConsoleGlobal.faction_id_)
                    x_pops_minimal("USEDEXCUSE",
                        FactionDiplomacyPortraits[target_faction_id], nullptr);

                if (action_result >= 6) {
                    double_cross(target_faction_id, actor_faction_id, -1);
                    treaty_on(target_faction_id, actor_faction_id, 0x10);
                } else {
                    pact_ends(target_faction_id, actor_faction_id);
                }
            } else {
                bool cause_native_friction = false;
                if (!has_treaty(target_faction_id, actor_faction_id, 0x20) &&
                    action_result <= 5) {
                    if ((action_result == 1 || action_result == 5) &&
                        actor_tech_rank < target_tech_rank &&
                        actor_rank <= 5 && DiploScratchFA3C == 0) {
                        cause_native_friction = rnd(8, nullptr) != 0;
                    } else if ((action_result == 2 || action_result == 4) &&
                               DiploScratchFA98 != 0 &&
                               DiploScratchFA3C == 0) {
                        cause_native_friction = true;
                    } else if ((action_result == 2 || action_result == 4) &&
                               actor_rank < target_rank &&
                               DiploScratchFA88 == 0 &&
                               rnd(8, nullptr) != 0) {
                        cause_native_friction = true;
                    }
                }
                if (cause_native_friction) {
                    const int friction = static_cast<int>(rnd(4, nullptr));
                    cause_friction(target_faction_id, actor_faction_id,
                                   friction);
                    if (actor_faction_id == ConsoleGlobal.faction_id_) {
                        clear();
                        says("KEPT");
                        say_num(action_result);
                        x_pops_minimal(StringTemp,
                            FactionDiplomacyPortraits[target_faction_id],
                            nullptr);
                    }
                    goto label_5a5816;
                }

                if (actor_faction_id == ConsoleGlobal.faction_id_)
                    x_pops_minimal("USEDEXCUSE",
                        FactionDiplomacyPortraits[target_faction_id], nullptr);

                if (action_result >= 6)
                    double_cross(target_faction_id, actor_faction_id, -1);
                else
                    treaty_on(target_faction_id, actor_faction_id, 0x10);
            }
        }

        // All relationship responses converge on the same normal Popup exit;
        // the native zero-result paths remain separate at label_5a2ce9.
        }
    }
}

label_5a5816:
    // Native destroys this embedded Popup and returns one. Automatic C++
    // scope exit performs cleanup through the existing Popup/BasePop/
    // GraphicWin ownership chain.
    goto label_5a5821;

label_5a1e28:
    // Native's early negative-selector path reaches its separate BasePop
    // cleanup continuation and also returns one. C++ scope exit here uses
    // the same genuine Popup -> Scroll -> BasePop owners without reproducing
    // the compiler's staged EH/vptr bookkeeping.
    return 1;

label_5a5821:
        return 1;

label_5a2ce9:
        // Native zero-result exits close Popup, then tear down its embedded
        // Scroll/FlatButtons/GraphicWin and BasePop in staged EH states.
        // Returning leaves that same close/member/base sequence to the real
        // automatic Popup destructor and compiler-owned subobject teardown.
        goto label_5a2cf4;

label_5a2cf4:
        return 0;
}
