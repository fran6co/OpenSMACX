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
#define BASEPOP_FRIEND_PROBE  // WORKAROUND(decl): see basepop.h
#define POPUP_WRAPPERS_INLINE  // stand-in: see the BasePop wrappers in basepop.h

#include "probe.h"
#include "base.h"
#include "game.h"
#include "veh.h"
#include "strings.h"
#include "map.h"
#include "faction_inline.h"  // has_treaty, expanded in corner_market

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];
extern uint32_t LockEnableMask;  // 0x009A64E8, defined in lock.cpp

BOOL ProbeHasAlgoEnhancement;  // 0x00945B30
uint32_t ProbeTargetFactionID;  // 0x00945B34
BOOL ProbeTargetHasHSA;  // 0x00945B38

/*
Purpose: Calculate how vulnerable the coordinates are for the specified faction based on how far
         away this tile is from the faction's Headquarters.
// ORIGINAL: 0x0059E980 ?vulnerable@@YAHHHH@Z 0x0059E980-0x0059EA73 BYTE_EXACT
// size      243 bytes
// prototype int (__cdecl ?vulnerable@@YAHHHH@Z)(int factionID, int xCoord, int yCoord)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00 0x00644F3A
// LEVER: moved here from base.cpp - the image places it directly before
//        mind_control in this TU (0x0059E980..0x0059EA73).
Return Value: Radial distance between coordinates and faction's HQ or 12 if no HQ/bases
Status: Complete
*/
int __cdecl vulnerable(int faction_id, int x, int y) {
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id && has_fac(FAC_HEADQUARTERS, i, 0)) {
            return vector_dist(x, y, Bases[i].x, Bases[i].y);
        }
    }
    return 12; // no bases or no HQ
}

// WORKAROUND(inline): /Ob2 budget stand-in for mind_control. The image keeps
// has_fac(FAC_HEADQUARTERS) (0x00421670) as a call inside its inlined copy of
// vulnerable (0x0059EADB) while expanding the three depth-1 has_fac tests;
// with plain `cost /= 2` spellings this unit expands that depth-2 has_fac as
// well. Routing the first five halvings through this helper spends the
// missing budget (call multiset now matches; 0.914 -> 0.9385). The original's
// spelling of the spend is unknown.
static inline int mc_half(int value) {
    return value / 2;
}

/*
Purpose: Calculate the cost for the faction to be able to mind control the specified base. The 3rd
         parameter determines if this cost is for cornering the market (true) or via probe (false).
// ORIGINAL: 0x0059EA80 ?mind_control@@YAHHHH@Z 0x0059EA80-0x0059EE42
// Historical LEVER: a local forwarder once forced an out-of-line
//   `vector_dist` call here while the helper remained inline in map.h. The
//   canonical definition now lives in map.cpp and this caller uses it directly.
//   MEASURED: the direct-call multiset went from 13
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
// NOTE: with vector_dist now a map.h inline, calling vulnerable() reproduces
//        its inlined copy (0.666 -> 0.708). Remaining gap: the image keeps
//        that copy's has_fac as a real call; this tree inlines it.
Return Value: Mind control cost
Status: Complete
*/
#pragma optimize("y", off)
int __cdecl mind_control(int base_id, int faction_id, BOOL is_corner_market) {
    uint32_t target_faction_id = Bases[base_id].faction_id_current;
    int target_x = Bases[base_id].x;
    int target_y = Bases[base_id].y;
    int cost = vulnerable(target_faction_id, target_x, target_y);
    if (cost <= 0) {
        if (is_corner_market) {
            cost = 1;
        } else {
            return -1;
        }
    }
    if (has_fac(FAC_GENEJACK_FACTORY, base_id, 0)) {
        cost *= 2;
    }
    if (has_fac(FAC_CHILDREN_CRECHE, base_id, 0)) {
        cost = mc_half(cost); // Reduces base's vulnerability to enemy mind control
    }
    if (has_fac(FAC_PUNISHMENT_SPHERE, base_id, 0)) {
        cost = mc_half(cost);
    }
    if (Bases[base_id].nerve_staple_turns_left) {
        cost = mc_half(cost);
    }
    int veh_id = stack_fix(veh_at(target_x, target_y));
    cost = ((stack_check(veh_id, 2, PLAN_COMBAT, -1, -1)
        + stack_check(veh_id, 2, PLAN_OFFENSIVE, -1, -1))
        * (stack_check(veh_id, 6, ABL_POLY_ENCRYPTION, -1, -1) + 1)
        + (int)PlayersData[faction_id].mind_control_total / 4 + Bases[base_id].population_size)
        * ((PlayersData[target_faction_id].corner_market_active
            + PlayersData[target_faction_id].energy_reserves + 1200) / (cost + 4));
    int human_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
    if (!(human_mask & (1 << faction_id)) && (human_mask & (1 << target_faction_id))) {
        int diff = PlayersData[target_faction_id].diff_level;
        if (diff > DLVL_LIBRARIAN) {
            cost = (cost * 3) / diff;
        }
    }
    if (is_corner_market) {
        if (PlayersData[faction_id].diplo_treaties[target_faction_id] & DTREATY_PACT) {
            cost = mc_half(cost);
        }
        if (PlayersData[faction_id].diplo_treaties[target_faction_id] & DTREATY_TREATY) {
            cost = mc_half(cost);
        }
        int tech_comm_target = PlayersData[target_faction_id].tech_commerce_bonus;
        tech_comm_target *= tech_comm_target;
        int tech_comm_probe = PlayersData[faction_id].tech_commerce_bonus;
        tech_comm_probe *= tech_comm_probe;
        cost = (cost * (tech_comm_target + 1)) / (tech_comm_probe + 1);
    } else if (PlayersData[faction_id].diplo_treaties[target_faction_id] & DTREATY_PACT) {
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
#pragma optimize("", on)

/*
Purpose: Calculate the cost for the faction to corner the Global Energy Market (Economic Victory).
// ORIGINAL: 0x0059EE50 ?corner_market@@YAHH@Z 0x0059EE50-0x0059EEDD BYTE_EXACT
// size      141 bytes
// prototype int (__cdecl ?corner_market@@YAHH@Z)(int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0059EA80
Return Value: Cost to corner the Global Energy Market
Status: Complete
*/
int __cdecl corner_market(int faction_id) {
    int cost = 0;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        uint32_t target_faction_id = Bases[i].faction_id_current;
        if (target_faction_id != faction_id) {
            if (!has_treaty(target_faction_id, faction_id, DTREATY_PACT)
                || !has_treaty(target_faction_id, faction_id, DTREATY_HAVE_SURRENDERED)) {
                cost += mind_control(i, faction_id, true);
            }
        }
    }
    return (cost < 1000) ? 1000 : cost;
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
#pragma optimize("y", off)
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
        int prb_defense = ((base_id + 1) != 0)
            ? (has_fac(FAC_COVERT_OPS_CENTER, base_id, 0) ? 2 : 0) : 0;
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

        int prb_defense_2 = ((base_id + 1) != 0)
            ? (has_fac(FAC_COVERT_OPS_CENTER, base_id, 0) ? 2 : 0) : 0;
        prb_defense_2 = range(PlayersData[ProbeTargetFactionID].soc_effect_active.values_[SocialEffect::PROBE]
            + prb_defense_2, -2, 0);
        int loss_rate = ((diff_modifier + 1) * 100) / (morale - prb_defense_2);
        if (ProbeHasAlgoEnhancement && !ProbeTargetHasHSA) {
            loss_rate /= 2;
        }
        int survival_rate = 100 - loss_rate;
        if (ProbeTargetHasHSA) {
            // BUG IN THE SHIPPED GAME: this hits success_rate a second time instead
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
#pragma optimize("", on)

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
#include "faction_inline.h"  // has_treaty, expanded in corner_market
#include "random.h"
#include "veh.h"
#include <cstdlib>
#include <cstring>
#include "tutwin.h"
#include "crt_random_inline.h"
// WORKAROUND(decl): probe's four `timeGetTime() & 1` coin flips call the
// WINMM thunk 0x005A58D0 in the image, not the import slot; a plain
// (non-dllimport) redeclaration routes calls through the thunk. The two
// random_reseed(timeGetTime()) sites call the import slot in the image and
// now go through the thunk too: VC6 binds every call to the latest
// declaration, so one unit cannot reproduce both.
extern "C" DWORD WINAPI timeGetTime(void);

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0059F120 0x005A5838 0x005A5850 0x005A1795
// RECOVERY_ASSOCIATED: v1 switch_selector 0x0059F120 0x005A5850 0x005A5860 0x005A178F
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0059F120 0x005A5860 0x005A5878 0x005A23A7
// The image tests the human/alive mask bytes in place at every site.
// WORKAROUND(inline): the first five human tests spell faction.h's is_human
// (same bytes); its expansions are the /Ob2 spend that leaves the FRAME
// list's get_name a call (0x005A3047), as in the image. Every site as
// is_human over-spends (two more ~Scroll stages refused, 0.958).
#define HUMAN_BIT(f) (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << (f)))
#define ALIVE_BIT(f) (GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << (f)))

// WORKAROUND(inline): file-local twin of the faction.h getter get_noun
// (0x0050B930). The image calls it out of line where probe's /Ob2 budget is
// exhausted and expands it (raw parse_set + field) where it is not; the
// expansions are budget spend.
static inline char *probe_noun(int faction_id);

// ORIGINAL: 0x0059F120 ?probe@@YAHHHHH@Z 0x0059F120-0x005A5837;0x006613D0-0x00661B3D
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original instructions establish distinct actor, target-base and fallback-vehicle roles, genuine PlayerData/Map/Base/Veh owners and one automatic Popup with class-owned teardown.", "The actor lock/state edge skips strategy scoring and energy checks before its shared mode/selector tail. Action-six raw UI results and normalized remote bit tests have distinct threshold stores; false edges retain the original path-local value, with no eager initialization.", "Bounded actual-source controls and native compilation are not full CFG, VC6 unwind-state, linked-preservation or game-runtime proof. Ordered comparison/reload fidelity and historical matching remain restoration obligations.", "Pinned5A292F..5A2961 operands96CD3C equal PlayersData96C9E0+35C, the signed tech_ranking word; trade_source actually begins36C, so its former+8 alias selected the wrong374 word. Preserve target rank/2 > actor rank/2+4 with truncation toward zero and the existing facility/action/pact short-circuit guards."], "reason": "Complete typed Probe action/UI owner in its existing translation unit, retaining the four-argument ABI and both original main/unwind spans."}
int __cdecl probe(int veh_id, int base_id, int veh_id_tgt, int mode) {
    Popup popup;
    int local_58 = 0;
    int local_4c = 0;
    int is_alien = 0;
    int local_64 = -1;
    int faction = Vehs[veh_id].faction_id;
    int faction_tgt = base_id >= 0 ? Bases[base_id].faction_id_current
                                   : Vehs[veh_id_tgt].faction_id;
    ProbeTargetFactionID = faction_tgt;
    if (Players[faction].rule_flags & RFLAG_ALIEN) {
        is_alien = 1;
    }
    int diff_modifier;
    int sabotage_id;
    int pcx_width;
    int pcx_height;
    char prod_name[256];
    int has_flag10_tech;
    int flag20_count;
    int flag20_known;
    int tech_id;
    int action;
    int home_base_id;
    if (is_ocean(Vehs[veh_id].x, Vehs[veh_id].y)
        && (base_id >= 0 ? !is_ocean(Bases[base_id].x, Bases[base_id].y)
                         : !is_ocean(Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y))
        && get_triad(veh_id) != TRIAD_SEA
        && !has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)) {
        if (faction == ConsoleGlobal.faction_id_ && mode && is_human(faction)
            && (NetDaemonNet.action_mode_ & 1)) {
            int ability = abil_index(ABL_AMPHIBIOUS);
            parse_say(0, Ability[ability].name, -1, -1);
            NetMsgPrimary.pop("PROBEABILAMPHIBIOUS", 5000, 0, 0);
        }
        return 0;
    }
    if ((base_id < 0 && is_ocean(Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y)
         || base_id >= 0 && is_ocean(Bases[base_id].x, Bases[base_id].y))
        && !is_ocean(Vehs[veh_id].x, Vehs[veh_id].y)
        && !has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)) {
        if (faction == ConsoleGlobal.faction_id_ && mode && is_human(faction)
            && (NetDaemonNet.action_mode_ & 1)) {
            int ability = abil_index(ABL_AMPHIBIOUS);
            parse_say(0, Ability[ability].name, -1, -1);
            NetMsgPrimary.pop("PROBEABILAMPHIBIOUS2", 5000, 0, 0);
        }
        return 0;
    }

    ProbeTargetHasHSA = 0;
    ProbeHasAlgoEnhancement = 0;
    if (has_abil(Vehs[veh_id].proto_id, ABL_ALGO_ENHANCEMENT)) {
        ProbeHasAlgoEnhancement = 1;
        if (has_project(SP_HUNTER_SEEKER_ALGO, faction_tgt)) {
            if (!mode && is_human(faction)) {
                return 0;
            }
            if ((NetDaemonNet.action_mode_ & 2) && mode) {
                ProbeTargetHasHSA = 1;
            } else {
                return 1;
            }
        }
    } else if (has_project(SP_HUNTER_SEEKER_ALGO, faction_tgt)) {
        if (!mode && is_human(faction)) {
            return 0;
        }
        if ((NetDaemonNet.action_mode_ & 2) && mode) {
            if (faction == ConsoleGlobal.faction_id_) {
                parse_says(0, Players[faction_tgt].adj_name_faction, -1, -1);
                NetMsgPrimary.pop("HUNTERSEEKER", "huntsk_sm.pcx");
            } else if (faction_tgt == ConsoleGlobal.faction_id_) {
                parse_says(0, Players[faction].adj_name_faction, -1, -1);
                NetMsgPrimary.pop("HUNTERSEEKER2", "huntsk_sm.pcx");
            }
            kill(veh_id);
            return 1;
        }
        return 1;
    }

    has_flag10_tech = 0;
    flag20_count = 0;
    flag20_known = 0;
    for (tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
        if ((Technology[tech_id].flags & 0x10) && has_tech(tech_id, faction)) {
            has_flag10_tech = 1;
        }
        if (Technology[tech_id].flags & 0x20) {
            flag20_count++;
            if (has_tech(tech_id, faction_tgt)) {
                flag20_known++;
            }
        }
    }

    if (base_id >= 0) {
        if (faction == ConsoleGlobal.faction_id_ && is_human(faction)
            && (NetDaemonNet.action_mode_ & 1)) {
            parse_says(1, Players[faction_tgt].adj_name_faction, -1, -1);
            popup.start("PROBE", 0x40);
            if (!Buffer::get_pcx_dimensions(
                    is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx",
                    &pcx_width, &pcx_height)
                && !popup.sprite_.init(
                    is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx",
                    pcx_width, pcx_height)) {
                popup.caller_sprite_ = &popup.sprite_;
            }
            if (X_text_open(ScriptPopupFile, "PROBEMENU")) {
                return 0;
            }
            text_get();
            if (!(PlayersData[faction].diplo_treaties[faction_tgt] & 0x1000)) {
                popup.item(TextBufferGetPtr, 0);
            }
            text_get();
            if (Rules.tgl_probe_can_steal_tech) {
                popup.item(TextBufferGetPtr, 1);
            }
            text_get();
            popup.item(TextBufferGetPtr, 2);
            text_get();
            popup.item(TextBufferGetPtr, 3);
            text_get();
            popup.item(TextBufferGetPtr, 4);
            text_get();
            if (has_fac(FAC_HEADQUARTERS, base_id, 0)) {
                popup.item(TextBufferGetPtr, 5);
            }
            text_get();
            if (!has_fac(FAC_HEADQUARTERS, base_id, 0)) {
                popup.item(TextBufferGetPtr, 6);
            }
            text_get();
            if (has_flag10_tech) {
                popup.item(TextBufferGetPtr, 7);
            }
            if (ExpansionEnabled && !IsMultiplayerNet) {
                text_get();
                int found = 0;
                if (has_fac(FAC_HEADQUARTERS, base_id, 0)) {
                    // Image reads seven records from 15: the last one,
                    // records_[21], is past the declared 21-record array
                    // (FactionMonuments+0x4A8, inside opaque_tail_prefix_).
                    for (int i = 15; i < 22; i++) {
                        if (FactionMonuments[faction_tgt].records_[i].achieved_) {
                            found = 1;
                        }
                    }
                    if (found) {
                        popup.item(TextBufferGetPtr, 8);
                    }
                }
            }
            action = popup.exec();
            if (action < 0) {
                return 0;
            }
            Vehs[veh_id].probe_action = (uint8_t)action;
        } else {
            if (!HUMAN_BIT(faction)) {
                goto L_5a2571;
            }
            action = Vehs[veh_id].probe_action & 7;
        }
    } else {
        action = -1;
    }

L_59fb64:
    if (!action && (NetDaemonNet.action_mode_ & 2)) {
        if (!(PlayersData[faction].diplo_treaties[faction_tgt] & 0x1000)) {
            set_treaty(faction, faction_tgt, 0x1000, 1);
            if (faction == ConsoleGlobal.faction_id_) {
                report_intel(faction_tgt);
            }
        }
        if (faction_tgt == ConsoleGlobal.faction_id_) {
            parse_says(0, get_title(faction), -1, -1);
            parse_says(1, get_name(faction), -1, -1);
            parse_says(2, probe_noun(faction), -1, -1);
            NetMsgPrimary.pop("DETECTINFILTRATE",
                              is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
        }
    }
    if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
        && (NetDaemonNet.action_mode_ & 1)
        && (PlayersData[faction].diplo_treaties[faction_tgt] & 7)
        && (!(PlayersData[faction_tgt].diplo_treaties[faction] & 1)
            || !(PlayersData[faction_tgt].diplo_treaties[faction] & 0x2000000))
        && action > 0) {
        parse_says(0, get_title(faction_tgt), -1, -1);
        parse_says(1, get_name(faction_tgt), -1, -1);
        if (!popp(ScriptPopupFile, "RISKEXCUSE", 0,
                  is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", 0)) {
            return 0;
        }
    }

    diff_modifier = 0;
    if (action == 1) {
        if (Bases[base_id].state & 0x40) {
            diff_modifier = 1;
            if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
                && (NetDaemonNet.action_mode_ & 1)) {
                parse_says(0, Bases[base_id].name_string, -1, -1);
                success_rates(1, morale_veh(veh_id, 1, 0), -1, base_id);
                if (!success_rates(2, morale_veh(veh_id, 1, 0), 1, base_id)) {
                    NetMsgPrimary.pop("ADVDECIPHER1", 0);
                    return 0;
                }
                if (!popp(ScriptPopupFile, "ADVDECIPHER", 0,
                          is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", 0)) {
                    return 0;
                }
            }
        }
        int tech_start;
        if (IsMultiplayerNet && HUMAN_BIT(faction)) {
            random_reseed(MapRandSeed
                          + GameDataStateStorage.fields_.turn_current_num_
                          + faction);
            tech_start = random(0, MaxTechnologyNum);
            random_reseed(timeGetTime());
        } else {
            tech_start = rand() % MaxTechnologyNum;
        }
        int i;
        for (i = 0; i < MaxTechnologyNum; i++) {
            tech_id = (tech_start + tech_id) % MaxTechnologyNum;
            if (has_tech(tech_id, faction_tgt) && !has_tech(tech_id, faction)) {
                break;
            }
        }
        if (i >= MaxTechnologyNum) {
            tech_id = 9999;
        }
        if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)) {
            success_rates(1, morale_veh(veh_id, 1, 0), diff_modifier, base_id);
            if (success_rates(2, morale_veh(veh_id, 1, 0), diff_modifier + 1,
                              base_id)) {
                int choice;
                if (!(NetDaemonNet.action_mode_ & 2)) {
                    choice = popp(ScriptPopupFile, "DECIPHER", 0x40,
                                  is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx",
                                  0);
                    if (choice < 0) {
                        return 0;
                    }
                    Vehs[veh_id].probe_action |= choice << 3;
                } else {
                    choice = (Vehs[veh_id].probe_action >> 3) & 1;
                }
                if (choice == 1) {
                    diff_modifier++;
                    tech_id = -1;
                    goto L_5a2dcd;
                }
            }
        }
        if (tech_id == 9999 && (PlayersData[faction].flags & 0x200)) {
            if (!HUMAN_BIT(faction)) {
                action = (PlayersData[faction].diplo_treaties[faction_tgt] & 0x30)
                    ? 2 : 0;
                goto L_59fb64;
            }
            if (faction == ConsoleGlobal.faction_id_
                && (NetDaemonNet.action_mode_ & 2)) {
                NetMsgPrimary.pop("STOLENOTHING", 0);
                return 0;
            }
        }
        goto L_5a2dcd;
    } else if (action == 3) {
        if (Bases[base_id].state & 0x400000) {
            diff_modifier = 1;
            if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
                && (NetDaemonNet.action_mode_ & 1)) {
                parse_says(0, Bases[base_id].name_string, -1, -1);
                success_rates(1, morale_veh(veh_id, 1, 0), -1, base_id);
                if (!success_rates(2, morale_veh(veh_id, 1, 0), 1, base_id)) {
                    NetMsgPrimary.pop("ADVENERGY1", 0);
                    return 0;
                }
                if (!popp(ScriptPopupFile, "ADVENERGY", 0,
                          is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", 0)) {
                    return 0;
                }
            }
        }
        goto L_5a2dcd;
    } else if (action == 2) {

    // action == 2 (5a051b)
    sabotage_id = 0;
    prod_name[0] = '\0';
    {
        int cost;
        if (Bases[base_id].queue_production_id[0] >= 0) {
            strcat(prod_name,
                   VehPrototypes[Bases[base_id].queue_production_id[0]].veh_name);
            cost = VehPrototypes[Bases[base_id].queue_production_id[0]].cost;
        } else {
            strcat(prod_name, StringTable.get(
                Facility[-Bases[base_id].queue_production_id[0]].name));
            cost = Facility[-Bases[base_id].queue_production_id[0]].cost;
        }
        int rnd_amount;
        if (IsMultiplayerNet && HUMAN_BIT(faction)) {
            random_reseed(MapRandSeed
                          + GameDataStateStorage.fields_.turn_current_num_
                          + faction);
            rnd_amount = random(0, morale_veh(veh_id, 1, 0) / 2 + 2);
            random_reseed(timeGetTime());
        } else {
            rnd_amount = bounded_crt_random(morale_veh(veh_id, 1, 0) / 2 + 2);
        }
        if (Bases[base_id].minerals_accumulated
                < cost_factor(faction, 1, -1) * cost
            || rnd_amount) {
            int ofs = rand() % 69;
            for (int i = 1; i < 70; i++) {
                int fac = (i + ofs) % 69 + 1;
                if (fac == 1) {
                    if (stack_check(veh_at(Bases[base_id].x, Bases[base_id].y),
                                    2, 5, -1, -1)) {
                        sabotage_id = 0x62;
                        break;
                    }
                } else if ((fac != FAC_PRESSURE_DOME
                            || !is_ocean(Bases[base_id].x, Bases[base_id].y))
                           && has_fac(fac, base_id, 0)) {
                    sabotage_id = fac;
                    break;
                }
            }
            if (!sabotage_id && !HUMAN_BIT(faction)) {
                action = Bases[base_id].population_size > 3 ? 4 : 0;
                goto L_59fb64;
            }
        }
    }

    // L_5a074b
    if (HUMAN_BIT(faction)) {
    success_rates(1, morale_veh(veh_id, 1, 0), 0, base_id);
    {
        int choice;
        if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
            && (NetDaemonNet.action_mode_ & 1)) {
            if (success_rates(2, morale_veh(veh_id, 1, 0), 1, base_id)) {
                choice = popp(ScriptPopupFile, "ADVVIRUS", 0,
                              is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", 0);
            } else {
                choice = 0;
            }
            Vehs[veh_id].probe_action |= choice << 3;
        } else {
            choice = (Vehs[veh_id].probe_action >> 3) & 1;
        }
        if (!choice) {
            goto L_5a2dcd;
        }
    }
    diff_modifier = 1;
    if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
        && (NetDaemonNet.action_mode_ & 1)) {
        popup.start("VIRUS");
        if (stack_check(veh_at(Bases[base_id].x, Bases[base_id].y),
                        2, 5, -1, -1)) {
            popup.item(label_get(0x404), 0x62);
        }
        for (int fac = 1; fac < 70; fac++) {
            if (fac != 1
                && (fac != FAC_PRESSURE_DOME
                    || !is_ocean(Bases[base_id].x, Bases[base_id].y))
                && has_fac(fac, base_id, 0)) {
                popup.item(StringTable.get(Facility[fac].name), fac);
            }
        }
        StringTemp[0] = '\0';
        strcat(StringTemp, prod_name);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, SocialWinOpenParenText);
        strcat(StringTemp, label_get(0xF9));
        strcat(StringTemp, SocialWinCloseParenText);
        popup.item(StringTemp, 0);
        parse_it(ScriptPopupFile, "ABORTSTRING");
        popup.item(StringTemp, 0x63);
        sabotage_id = popup.exec();
        if (sabotage_id < 0) {
            return 0;
        }
        Vehs[veh_id].probe_sabotage_id = (uint8_t)sabotage_id;
    } else {
        // WORKAROUND(type): movsx at 5a0af5 - the field reads as signed char.
        sabotage_id = (int8_t)Vehs[veh_id].probe_sabotage_id;
    }
    if (sabotage_id == 0x63) {
        goto L_5a2dcd;
    }
    if (has_fac(FAC_HEADQUARTERS, base_id, 0)) {
        if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
            && (NetDaemonNet.action_mode_ & 1)) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_says(1, Players[faction_tgt].adj_name_faction, -1, -1);
            success_rates(2, morale_veh(veh_id, 1, 0), 2, base_id);
            if (!popp(ScriptPopupFile, "HQVIRUS", 0,
                      is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", 0)) {
                return 0;
            }
        }
    } else if (sabotage_id == FAC_PERIMETER_DEFENSE || sabotage_id == FAC_TACHYON_FIELD) {
    if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
        && (NetDaemonNet.action_mode_ & 1)) {
        parse_says(0, Bases[base_id].name_string, -1, -1);
        parse_say(1, Facility[sabotage_id].name, -1, -1);
        success_rates(2, morale_veh(veh_id, 1, 0), 2, base_id);
        if (!popp(ScriptPopupFile, "MILVIRUS", 0,
                  is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", 0)) {
            return 0;
        }
    }
    } else {
        goto L_5a2dcd;
    }
    // ---- 5a0d96: last statement of the HUMAN_BIT(faction) arm of action 2
    diff_modifier = 2;
    goto L_5a2dcd;
    } else {
    if (morale_veh(veh_id, 1, 0) >= 5) {
        if (stack_check(veh_at(Bases[base_id].x, Bases[base_id].y), 2, 5, -1, -1)) {
            sabotage_id = 0x62;
            diff_modifier = 1;
        } else if (has_fac(FAC_TACHYON_FIELD, base_id, 0)) {
            sabotage_id = FAC_TACHYON_FIELD;
            diff_modifier = 1;
        } else if (has_fac(FAC_PERIMETER_DEFENSE, base_id, 0)) {
            sabotage_id = FAC_PERIMETER_DEFENSE;
            diff_modifier = 1;
        } else if (has_fac(FAC_CHILDREN_CRECHE, base_id, 0)) {
            sabotage_id = FAC_CHILDREN_CRECHE;
            diff_modifier = 1;
        } else if (has_fac(FAC_COMMAND_CENTER, base_id, 0)) {
            sabotage_id = FAC_COMMAND_CENTER;
            diff_modifier = 1;
        }
    }
    goto L_5a2dcd;
    }
    // ---- 5a0ee6: else of `action == 2`
    } else if (action == 5) {
        diff_modifier = 1;
        if (!success_rates(1, morale_veh(veh_id, 1, 0), 1, base_id)) {
            if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
                && (NetDaemonNet.action_mode_ & 1)) {
                NetMsgPrimary.pop("NOASSASSIN", nullptr);
            }
            return 0;
        }
        if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
            && (NetDaemonNet.action_mode_ & 1)
            && !popp(ScriptPopupFile, "ASSASSIN", 0,
                     is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", nullptr)) {
            return 0;
        }
        goto L_5a2dcd;
    } else if (action == 8) {
        diff_modifier = 1;
        if (!success_rates(1, morale_veh(veh_id, 1, 0), 1, base_id)) {
            if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)) {
                NetMsgPrimary.pop("NOFREEWILLY", nullptr);
            }
            return 0;
        }
        if (faction == ConsoleGlobal.faction_id_ && HUMAN_BIT(faction)
            && (NetDaemonNet.action_mode_ & 1)
            && !popp(ScriptPopupFile, "FREEWILLY", 0,
                     is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", nullptr)) {
            return 0;
        }
        goto L_5a2dcd;
    } else if (action == -1) {
        // 5a1252: target is a vehicle stack
        if (stack_check(veh_id_tgt, 1, -1, -1, -1) > 1) {
            return 0;
        }
        if (Chassis[VehPrototypes[Vehs[veh_id_tgt].proto_id].chassis_id].triad == TRIAD_AIR
            && !has_abil(Vehs[veh_id].proto_id, ABL_AIR_SUPERIORITY)) {
            if (faction == ConsoleGlobal.faction_id_ && mode && HUMAN_BIT(faction)
                && (NetDaemonNet.action_mode_ & 1)) {
                int ability = abil_index(ABL_AIR_SUPERIORITY);
                parse_say(0, Ability[ability].name, -1, -1);
                NetMsgPrimary.pop("PROBEABILSUPERIORITY", 5000, 0, nullptr);
            }
            return 0;
        }
        if (!faction_tgt
            || (Vehs[veh_id_tgt].proto_id < 0x40
                && (Weapon[VehPrototypes[Vehs[veh_id_tgt].proto_id].weapon_id].offense_rating < 0
                    || Vehs[veh_id_tgt].proto_id == 0x0F))) {
            // 5a201d
            if (faction == ConsoleGlobal.faction_id_ && mode && HUMAN_BIT(faction)
                && (NetDaemonNet.action_mode_ & 1)) {
                NetMsgPrimary.pop("ALIENPROBE", nullptr);
            }
            return 0;
        } else {
            // 5a1460..5a16f4: inlined vulnerable() (x/y param temps in
            // [ebp-0x34]/[ebp-0x60]; has_fac and 2-arg vector_dist stay calls)
            int dist = vulnerable(faction_tgt, Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y);
            if (Vehs[veh_id_tgt].home_base_id >= 0
                && has_fac(FAC_PUNISHMENT_SPHERE, Vehs[veh_id_tgt].home_base_id, 0)) {
                dist /= 2;
            }
            int cost = (PlayersData[faction_tgt].energy_reserves + 800) / (dist + 2)
                * VehPrototypes[Vehs[veh_id_tgt].proto_id].cost;
            int human_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
            if (!(human_mask & (1 << faction)) && (human_mask & (1 << faction_tgt))
                && PlayersData[faction_tgt].diff_level > 3) {
                cost = cost * 3 / PlayersData[faction_tgt].diff_level;
            }
            if (Players[faction_tgt].rule_flags & RFLAG_MINDCONTROL) {
                if (faction == ConsoleGlobal.faction_id_ && (human_mask & (1 << faction)) && mode
                    && (NetDaemonNet.action_mode_ & 1)) {
                    parse_says(0, Players[faction_tgt].adj_name_faction, -1, -1);
                    NetMsgPrimary.pop("MINDIMMUNITY2", nullptr);
                }
                return 0;
            }
            for (int i = 0; i < Players[faction].faction_bonus_count; i++) {
                if (Players[faction].faction_bonus_id[i] == RULE_PROBECOST) {
                    cost = cost * Players[faction].faction_bonus_val1[i] / 100;
                }
            }
            if (has_project(SP_NETHACK_TERMINUS, faction)) {
                cost = cost * 3 / 4;
            }
            // 5a1795: byte table at 0x5A5850, selector + 6
            switch (PlayersData[faction_tgt].soc_effect_active.values_[SocialEffect::PROBE]) {
            case -6:
            case -5:
            case -4:
            case -3:
            case -2:
                cost /= 2;
                break;
            case -1:
                cost -= cost / 4; // image: neg + add shared with case 1 (UNRESOLVED 1)
                break;
            case 1:
                cost += cost / 2;
                break;
            case 2:
                cost *= 2;
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                if (faction == ConsoleGlobal.faction_id_ && (human_mask & (1 << faction)) && mode
                    && (NetDaemonNet.action_mode_ & 1)) {
                    parse_says(0, Players[faction_tgt].adj_name_faction, -1, -1);
                    NetMsgPrimary.pop("IMMUNITY", nullptr);
                }
                return 0;
            }
            if (has_abil(Vehs[veh_id_tgt].proto_id, ABL_POLY_ENCRYPTION)) {
                cost *= 2;
            }
            if (VehPrototypes[Vehs[veh_id_tgt].proto_id].plan != PLAN_COLONIZATION
                && VehPrototypes[Vehs[veh_id_tgt].proto_id].plan != PLAN_TERRAFORMING) {
                cost /= 2;
            }
            parse_says(0, VehPrototypes[Vehs[veh_id_tgt].proto_id].veh_name, -1, -1);
            human_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
            if (!(human_mask & (1 << faction)) || (Vehs[veh_id].state & 0x200)) {
                int score = (PlayersData[faction].ai_power - PlayersData[faction].ai_wealth + 2)
                    * cost * 4;
                if (human_mask & (1 << faction_tgt)) {
                    score /= 2;
                }
                int weap = weap_strat(VehPrototypes[Vehs[veh_id_tgt].proto_id].weapon_id, faction);
                int arm = arm_strat(VehPrototypes[Vehs[veh_id_tgt].proto_id].armor_id, faction);
                int speed = Chassis[VehPrototypes[Vehs[veh_id_tgt].proto_id].chassis_id].speed;
                int region = region_at(Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y);
                if (PlayersData[faction].region_total_bases[region]) {
                    if (PlayersData[faction_tgt].region_force_rating[region]
                        > PlayersData[faction].region_force_rating[region] / 2
                            + PlayersData[faction].region_total_combat_vehs[region]) {
                        score /= 2;
                    }
                }
                if (PlayersData[faction].region_force_rating[region]
                    > PlayersData[faction_tgt].region_total_combat_vehs[region]) {
                    score *= 2;
                }
                if (weap > PlayersData[faction].best_weapon_value) {
                    score /= 2;
                }
                if (arm > PlayersData[faction].best_armor_value) {
                    score /= 2;
                }
                if (speed > PlayersData[faction].best_land_speed) {
                    score = score * 2 / 3;
                }
                if (PlayersData[faction].diplo_treaties[faction_tgt] & DTREATY_TREATY) {
                    score *= 2;
                }
                if (!(PlayersData[faction].diplo_treaties[faction_tgt] & DTREATY_WANT_REVENGE)
                    && !(GameDataStateStorage.fields_.game_state_ & 0x200)
                    && !at_climax(faction)
                    && (PlayersData[faction].diplo_treaties[faction_tgt] & DTREATY_UNK_1000000)
                    && (PlayersData[faction].diplo_treaties[faction_tgt]
                        & (DTREATY_TREATY | DTREATY_TRUCE))
                    && whose_territory(faction, Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y,
                                       nullptr, 0) != faction) {
                    return 0;
                }
                if (weap < PlayersData[faction].best_weapon_value
                    && arm < PlayersData[faction].best_armor_value) {
                    score *= 2;
                }
                if (weap > PlayersData[faction].best_weapon_value) {
                    score /= 2;
                }
                Map *tile = map_loc(Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y);
                if ((tile->val3 & 0xC0) >= 0x80
                    || ((tile->bit & BIT_FUNGUS) && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF)
                    || (tile->bit & BIT_BUNKER)) {
                    score /= 2;
                }
                if (VehPrototypes[Vehs[veh_id_tgt].proto_id].plan <= PLAN_COMBAT) {
                    score /= 2;
                }
                if (VehPrototypes[Vehs[veh_id_tgt].proto_id].plan >= PLAN_DEFENSIVE) {
                    score *= 3;
                }
                if (PlayersData[faction].energy_reserves < score) {
                    return 0;
                }
                if (PlayersData[faction].energy_reserves < cost) {
                    return 0;
                }
            }
            if (!mode) {
                return 1; // 5a1da9 falls into the 5a1e28 epilogue
            }
            if (HUMAN_BIT(faction)) {
                int choice;
                if (faction == ConsoleGlobal.faction_id_
                    && (NetDaemonNet.action_mode_ & 1)) {
                    parse_num(0, cost);
                    parse_num(1, PlayersData[faction].energy_reserves);
                    success_rates(1, morale_veh(veh_id, 1, 0), 0, -1);
                    success_rates(2, morale_veh(veh_id, 1, 0), 1, -1);
                    popup.start("SUBVERT");
                    if (PlayersData[faction].energy_reserves >= cost) {
                        popup.start("SUBVERTMENU", 0x80);
                    }
                    choice = popup.exec();
                    if (!choice) {
                        return 0;
                    }
                    Vehs[veh_id].probe_action |= (choice + 0x1F) << 3;
                } else {
                    choice = (Vehs[veh_id].probe_action & 8) ? 2 : 1;
                }
                diff_modifier = choice - 1;
            }
            if (NetDaemonNet.action_mode_ & 2) {
                PlayersData[faction].energy_reserves -= cost;
                if (faction == ConsoleGlobal.faction_id_) {
                    ConsoleGlobal.update_data(0); // image cross-jumps to 5a2dc1
                }
            }
            goto L_5a2dcd;
        }
    } else if (action == 6) {
        // No initializers: chunk A enters this block at L_5a2571.
        int cost;       // edi; chunk C (5a2ab3..) reads it
        int probe;
        int human_mask;
        int region;
        int found;
        int found_mon;
        cost = mind_control(base_id, faction, 0);
        if (cost < 0) {
            if (!HUMAN_BIT(faction)) {
                action = 1;
                goto L_59fb64;
            }
            return 0;
        }
        if (Players[faction_tgt].rule_flags & RFLAG_MINDCONTROL) {
            if (!HUMAN_BIT(faction)) {
                goto L_5a2571; // image: 5a2574
            }
            if (faction == ConsoleGlobal.faction_id_
                && (NetDaemonNet.action_mode_ & 1)) {
                parse_says(0, Players[faction_tgt].adj_name_faction, -1, -1);
                NetMsgPrimary.pop("MINDIMMUNITY", nullptr);
            }
            return 0;
        }
        { for (int i = 0; i < Players[faction].faction_bonus_count; i++) {
            if (Players[faction].faction_bonus_id[i] == RULE_PROBECOST) {
                cost = cost * Players[faction].faction_bonus_val1[i] / 100;
            }
        } }
        if (has_project(SP_NETHACK_TERMINUS, faction)) {
            cost = cost * 3 / 4;
        }
        probe = PlayersData[faction_tgt].soc_effect_active.values_[SocialEffect::PROBE];
        if (has_fac(FAC_GENEJACK_FACTORY, base_id, 0)) {
            probe--;
        }
        if (has_fac(FAC_COVERT_OPS_CENTER, base_id, 0)) {
            probe += 2;
        }
        // 5a23a7: jump table at 0x5A5860, selector + 2
        switch (probe) {
        case -2:
            cost /= 2;
            break;
        case -1:
            cost -= cost / 4; // image: neg + add shared with case 1 (UNRESOLVED 1)
            break;
        case 1:
            cost += cost / 2;
            break;
        case 2:
            cost *= 2;
            break;
        case 3:
            if (!HUMAN_BIT(faction)) {
                action = 1;
                goto L_59fb64;
            }
            if (faction == ConsoleGlobal.faction_id_
                && (NetDaemonNet.action_mode_ & 1)) {
                parse_says(0, Players[faction_tgt].adj_name_faction, -1, -1);
                NetMsgPrimary.pop("IMMUNITY", nullptr);
            }
            return 0;
        }
        human_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
        if (!(human_mask & (1 << faction))) {
            if (human_mask & (1 << faction_tgt)) {
                if (PlayersData[faction].energy_reserves
                    < cost + range(PlayersData[faction].energy_reserves / 10, 0, 50) + 2) {
                    goto L_5a2571; // image: 5a2574
                }
            } else if (PlayersData[faction].energy_reserves
                       < (PlayersData[faction].ai_power - PlayersData[faction].ai_wealth + 3)
                           * cost * 2 / 3) {
                // ---- 5a2571: AI action choice; also entered from chunk A (59fb46)
L_5a2571:
                action = (PlayersData[faction].diplo_treaties[faction_tgt] & 0x30) ? 2 : 0;
                region = region_at(Bases[base_id].x, Bases[base_id].y);
                if ((PlayersData[faction].diplo_treaties[faction_tgt] & 0x30)
                    && PlayersData[faction].region_base_plan[region]
                    && Bases[base_id].talent_total == Bases[base_id].drone_total
                    && !(rand() % 2)
                    && Bases[base_id].population_size > 3) {
                    action = 4;
                }
                if (has_flag10_tech) {
                    if (Bases[base_id].population_size >= 6
                        && ((PlayersData[faction].diplo_treaties[faction_tgt] & DTREATY_ATROCITY_VICTIM)
                            || (GameDataStateStorage.fields_.council_resolution_count_[8]
                                    > GameDataStateStorage.fields_.council_resolution_count_[9]
                                && (PlayersData[faction].diplo_treaties[faction_tgt]
                                    & DTREATY_WANT_REVENGE)))
                        && PlayersData[faction].region_base_plan[region] == 2) {
                        action = 7;
                    }
                    if (Bases[base_id].population_size >= 4
                        && (PlayersData[faction].flags & 0x1000000)
                        && (!(PlayersData[faction].diplo_treaties[faction_tgt] & 7)
                            || (PlayersData[faction].diplo_treaties[faction_tgt]
                                & DTREATY_WANT_REVENGE))) {
                        action = 7;
                    }
                }
                found = 0;
                if (PlayersData[faction].diplo_treaties[faction_tgt] & 0x30) {
                    for (int i = 0; i < 21; i++) {
                        int x = wrap_map_x(Bases[base_id].x + RadiusOffsetX[i]);
                        int y = Bases[base_id].y + RadiusOffsetY[i];
                        if (on_map(x, y) && !is_ocean(x, y)) {
                            int veh_id_near = stack_fix(veh_at(x, y));
                            if (veh_id_near >= 0 && Vehs[veh_id_near].faction_id == faction
                                && stack_check(veh_id_near, 4, faction, -1, -1)) {
                                found = 1;
                                action = 2;
                                break;
                            }
                        }
                    }
                }
                if (Rules.tgl_probe_can_steal_tech) {
                    if (!(Bases[base_id].state & 0x40)
                        || morale_veh(veh_id, 1, 0) >= 4
                        || (!action && (PlayersData[faction].diplo_treaties[faction_tgt]
                                        & DTREATY_HAVE_INFILTRATOR))) {
                        if (!found && PlayersData[faction_tgt].region_base_plan[region] != 2) {
                            action = 1;
                        }
                    }
                }
                if (has_flag10_tech
                    && Bases[base_id].population_size >= 4
                    && aah_ooga(faction, faction) == faction_tgt
                    && climactic_battle()
                    && (PlayersData[faction].ai_fight > 0
                        || (!PlayersData[faction].ai_fight
                            && (PlayersData[faction].diplo_treaties[faction_tgt]
                                & DTREATY_WANT_REVENGE)))
                    && (PlayersData[faction].diplo_treaties[faction_tgt] & 0xC30)) {
                    action = 7;
                }
                if (local_4c) {
                    goto L_59fb64;
                }
                local_4c = has_fac(FAC_HEADQUARTERS, base_id, 0);
                if (local_4c) {
                    if (action != 1
                        && PlayersData[faction_tgt].tech_accumulated
                            >= PlayersData[faction_tgt].tech_cost / 2
                        && ((PlayersData[faction].diplo_treaties[faction_tgt] & 0x30)
                            || PlayersData[faction_tgt].tech_ranking / 2
                                > PlayersData[faction].tech_ranking / 2 + 4)) {
                        action = 5;
                    }
                    if (ExpansionEnabled && !IsMultiplayerNet) {
                        found_mon = 0;
                        local_4c = has_fac(FAC_HEADQUARTERS, base_id, 0);
                        if (local_4c) {
                            // records_[15 + 6] runs one past the declared array (image does)
                            for (int i = 0; i < 7; i++) {
                                if (FactionMonuments[faction_tgt].records_[i + 15].achieved_
                                    && !(PlayersData[faction].diplo_treaties[i + 1] & DTREATY_VENDETTA)) {
                                    found_mon = 1;
                                    if (local_64 == -1
                                        || (PlayersData[faction].diplo_treaties[i + 1] & DTREATY_PACT)
                                        || ((PlayersData[faction].diplo_treaties[i + 1] & DTREATY_TREATY)
                                            && !(PlayersData[faction].diplo_treaties[local_64 + 1]
                                                 & DTREATY_PACT))) {
                                        local_64 = i + 1;
                                    }
                                }
                            }
                            if (found_mon) {
                                action = 8;
                                local_4c = 1;
                                goto L_59fb64;
                            }
                        }
                    }
                    goto L_5a2aa4;
                }
                // ---- chunk C continues here at 5a2a58, still inside this
                // else-if block (which ends at 5a2aa4: local_4c = 1; goto L_59fb64)
    // ---- 5a2a58: else-arm of chunk B's `if (local_4c)` (5a28ee) ----
    if ((PlayersData[faction].diplo_treaties[faction_tgt] & 0x30) != 0
        && (PlayersData[faction_tgt].diff_level > 2 || !HUMAN_BIT(faction_tgt))
        && PlayersData[faction_tgt].soc_effect_active.values_[SocialEffect::PROBE] < 3) {
        action = 6;
    }
L_5a2aa4:
    local_4c = 1;
    goto L_59fb64;
    }
    }

    // ---- 5a2ab0: thought control ----
    if (HUMAN_BIT(faction)) {
        int tc_level;
        if (faction == ConsoleGlobal.faction_id_
            && (NetDaemonNet.action_mode_ & 1)) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_num(0, cost);
            parse_num(1, PlayersData[faction].energy_reserves);
            success_rates(1, morale_veh(veh_id, 1, 0), 0, -1);
            success_rates(2, morale_veh(veh_id, 1, 0), 1, -1);
            if (PlayersData[faction].energy_reserves < cost) {
                wave_it(9);
                NetMsgPrimary.pop("THOUGHTCONTROL", NULL);
                return 0;
            }
            wave_it(0x20);
            popup.start("THOUGHTCONTROL");
            popup.start("THOUGHTMENU", 0x80);
            int choice = popup.exec();
            if (choice <= 0) {
                return 0;
            }
            // image: add dl,0x1f; shl dl,3 (byte-folded -1); if this does not
            // reproduce, spell it (choice + 0x1F) << 3
            Vehs[veh_id].probe_action |= (choice - 1) << 3;
            tc_level = choice;
        } else {
            tc_level = (Vehs[veh_id].probe_action & 8) ? 2 : 1;
        }
        if (tc_level > 1) {
            diff_modifier = 1;
        }
    }
    if (NetDaemonNet.action_mode_ & 2) {
        PlayersData[faction].energy_reserves -= cost;
        PlayersData[faction].mind_control_total += 4;
        PlayersData[faction_tgt].diplo_mind_control[faction] += 4;
        if (faction == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.update_data(0);
        }
    }

    }

    // ---- 5a2dcd: shared resolution ----
L_5a2dcd:
    mode = 0;
    if ((action < 6 && action > 0) || action == 8) {
        if (!HUMAN_BIT(faction)) {
            if (morale_veh(veh_id, 1, 0) >= 5 && !diff_modifier && !HUMAN_BIT(faction_tgt)) {
                for (mode = 1; mode < MaxPlayerNum; mode++) {
                    if (mode != faction && mode != faction_tgt
                        && HUMAN_BIT(mode) && ALIVE_BIT(mode)
                        && mode == GameDataStateStorage.fields_.ranking_faction_id_unk1_
                        && !(PlayersData[faction_tgt].diplo_treaties[mode] & 0x10)
                        && (PlayersData[faction_tgt].diplo_treaties[mode] & 8)) {
                        break;
                    }
                }
                // natural loop exit goes straight to mode = 0 (5a2e97);
                // the break path tests < 8 then shares the tail at 5a30f6
                if (mode >= MaxPlayerNum) {
                    mode = 0;
                }
            }
        } else if (!HUMAN_BIT(faction_tgt)) {
            if (faction == ConsoleGlobal.faction_id_
                && (NetDaemonNet.action_mode_ & 1)) {
                popup.start("FRAME");
                success_rates(2, morale_veh(veh_id, 1, 0), diff_modifier, base_id);
                if (success_rates(3, morale_veh(veh_id, 1, 0), diff_modifier + 1, base_id)
                    && !X_text_open(ScriptPopupFile, "FRAMEMENU")) {
                    text_get();
                    parse_string(TextBufferGetPtr, StringTemp);
                    popup.item(StringTemp, 0);
                    text_get();
                    mode = 0;
                    for (int i = 1; i < MaxPlayerNum; i++) {
                        if (i != faction && i != faction_tgt
                            && (PlayersData[faction].diplo_treaties[i] & 8)
                            && ALIVE_BIT(i)) {
                            mode++;
                            parse_set(Players[i].is_leader_female, 0);
                            parse_says(0, Players[i].title_leader, -1, -1);
                            parse_says(1, get_name(i), -1, -1);
                            clear();
                            parse_string(TextBufferGetPtr, StringTemp);
                            popup.item(StringTemp, i);
                        }
                    }
                    if (mode) {
                        mode = popup.exec();
                    } else {
                        mode = 0;
                    }
                }
                Vehs[veh_id].probe_action |= mode << 5;
            } else {
                mode = Vehs[veh_id].probe_action >> 5;
            }
        }
        if (mode) {
            diff_modifier++;
        }
    }
    if (!(NetDaemonNet.action_mode_ & 2)) {
        return 1;
    }
    {
        int bonus;
        if (action != -1) {
            bonus = has_fac(0x22, base_id, 0) ? 2 : 0;
        } else {
            bonus = 0;
        }
        if (morale_veh(veh_id, 1, 0) < 1) {
            local_4c = 1;
        } else {
            local_4c = morale_veh(veh_id, 1, 0);
        }
        if ((int)rnd(local_4c / 2
                - range(PlayersData[faction_tgt].soc_effect_active.values_[SocialEffect::PROBE]
                        + bonus, -2, 0) + 1, "PROBEBUST") < diff_modifier) {
            if (!ProbeHasAlgoEnhancement || ProbeTargetHasHSA || (timeGetTime() & 1)) {
                mode = -mode;
                if (faction == ConsoleGlobal.faction_id_) {
                    if (Players[faction].rule_flags & RFLAG_ALIEN) {
                        NetMsgPrimary.pop("BUSTED", "al_cap_sm.pcx");
                    } else {
                        NetMsgPrimary.pop("BUSTED", "capture_sm.pcx");
                    }
                }
                if (faction_tgt == ConsoleGlobal.faction_id_) {
                    local_58 = action;
                }
                kill(veh_id);
                goto L_5a4b15;
            }
        } else if (ProbeTargetHasHSA && (timeGetTime() & 1)) {
            mode = -mode;
            if (faction == ConsoleGlobal.faction_id_) {
                if (Players[faction].rule_flags & RFLAG_ALIEN) {
                    NetMsgPrimary.pop("BUSTED", "al_cap_sm.pcx");
                } else {
                    NetMsgPrimary.pop("BUSTED", "capture_sm.pcx");
                }
            }
            if (faction_tgt == ConsoleGlobal.faction_id_) {
                local_58 = action;
            }
            kill(veh_id);
            goto L_5a4b15;
        }
    }

    // ---- 5a329d: action dispatch (chunk D supplies the final -1 arm) ----
    if (action == 1) {
        PlayersData[faction_tgt].stolen_data_count[faction]++;
        if (tech_id < 0) {
            steal_tech(faction, faction_tgt, 1);
        } else {
            if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
                parse_says(0, get_adjective(faction_tgt), -1, -1);
                parse_says(2, get_adjective(faction), -1, -1);
                clear();
                if (tech_id == 9999) {
                    says("STOLEMAP");
                } else {
                    say_tech(tech_id, 0);
                    parse_says(1, StringTemp, -1, -1);
                    clear();
                    says("DECIPHERED");
                }
                says(faction == ConsoleGlobal.faction_id_ ? "0" : "1");
                NetMsgPrimary.pop(StringTemp, is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
            }
            tech_achieved(faction, tech_id, faction_tgt, 0);
        }
        Bases[base_id].state |= 0x40;
    } else if (action == 2) {
        if (sabotage_id == 99) {
            NetMsgPrimary.pop("VIRUSABORT", NULL);
            return 1;
        }
        clear();
        if (sabotage_id == 0) {
            Bases[base_id].minerals_accumulated_2 = 0;
            Bases[base_id].minerals_accumulated = 0;
            parse_says(0, prod_name, -1, -1);
            says("PRODVIRUS");
            parse_says(1, Bases[base_id].name_string, -1, -1);
            parse_says(2, get_adjective(faction), -1, -1);
            says(faction == ConsoleGlobal.faction_id_ ? "0" : "1");
            if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop(StringTemp, is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
            }
        } else if (sabotage_id == 0x62) {
            int veh_id_hit = veh_at(Bases[base_id].x, Bases[base_id].y);
            if (veh_id_hit < 0) {
                return 1;
            }
            do {
                if (VehPrototypes[Vehs[veh_id_hit].proto_id].plan == 5) {
                    break;
                }
                veh_id_hit = veh_next_stack_id(veh_id_hit);
            } while (veh_id_hit >= 0);
            // exhausted loop jumps straight to 5a36c9 (5a350e); break path
            // straight to the body at 5a3513
            if (veh_id_hit >= 0) {
                if (Vehs[veh_id_hit].faction_id == ConsoleGlobal.faction_id_) {
                    parse_says(0, VehPrototypes[Vehs[veh_id_hit].proto_id].veh_name, -1, -1);
                    parse_says(1, get_adjective(faction), -1, -1);
                    NetMsgPrimary.pop("BUSTMYPLANET",
                                      is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
                } else if (faction == ConsoleGlobal.faction_id_) {
                    parse_says(0, VehPrototypes[Vehs[veh_id_hit].proto_id].veh_name, -1, -1);
                    parse_says(2, get_adjective(Vehs[veh_id_hit].faction_id), -1, -1);
                    NetMsgPrimary.pop("BUSTYOURPLANET",
                                      is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
                }
                kill(veh_id_hit);
                if (veh_id > veh_id_hit) {
                    veh_id--;
                }
            }
        } else {
            set_fac(sabotage_id, base_id, 0);
            parse_say(0, Facility[sabotage_id].name, -1, -1);
            says("FACVIRUS");
            parse_says(1, Bases[base_id].name_string, -1, -1);
            parse_says(2, get_adjective(faction), -1, -1);
            says(faction == ConsoleGlobal.faction_id_ ? "0" : "1");
            if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop(StringTemp, is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
            }
        }
        TutWinBaseWindow.check_base(base_id);
        draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
    } else if (action == 3) {
        int energy = steal_energy(base_id);
        int share = morale_veh(veh_id, 1, 0) * energy / 6;
        energy = range(share + (int)rnd(energy - share, "PROBEENERGY") / 2, 0,
                       PlayersData[faction_tgt].energy_reserves);
        if (energy == 1) {
            energy = 0;
        }
        PlayersData[faction_tgt].energy_reserves -= energy;
        PlayersData[faction].energy_reserves += energy;
        if (faction == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.update_data(0);
            parse_num(0, energy);
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_says(2, get_adjective(faction_tgt), -1, -1);
            NetMsgPrimary.pop(energy ? "GOTENERGY" : "GOTNOENERGY",
                              is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
        } else if (faction_tgt == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.update_data(0);
            parse_num(0, energy);
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_says(2, get_adjective(faction), -1, -1);
            NetMsgPrimary.pop(energy ? "TOOKENERGY" : "TOOKNOENERGY",
                              is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
        }
    } else if (action == 4) {
        // image: max(assim, pop*10/4) computed first (pushed as the 3rd arg)
        int limit = Bases[base_id].population_size * 10 / 4;
        if (Bases[base_id].assimilation_turns_left > limit) {
            limit = Bases[base_id].assimilation_turns_left;
        }
        Bases[base_id].assimilation_turns_left =
            range(Bases[base_id].assimilation_turns_left + 10, 0, limit);
        set_base(base_id);
        base_compute(1);
        parse_says(0, Bases[base_id].name_string, -1, -1);
        if (BaseCurrent->drone_total > BaseCurrent->talent_total) {
            BaseCurrent->state |= 2;
            draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
            if (Players[faction].rule_flags & RFLAG_ALIEN) {
                if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
                    NetMsgPrimary.pop("DRONEINCITE0", "ALdrone_sm.pcx");
                }
            } else {
                if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
                    NetMsgPrimary.pop("DRONEINCITE0", "drone_sm.pcx");
                }
            }
        } else {
            if (Players[faction].rule_flags & RFLAG_ALIEN) {
                if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
                    NetMsgPrimary.pop("DRONEINCITE1", "ALdrone_sm.pcx");
                }
            } else {
                if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
                    NetMsgPrimary.pop("DRONEINCITE1", "drone_sm.pcx");
                }
            }
        }
    } else if (action == 5) {
        int stolen = rnd(PlayersData[faction_tgt].tech_accumulated, "PROBEASSASSIN");
        PlayersData[faction_tgt].tech_accumulated -= stolen;
        if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
            parse_num(0, stolen);
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_says(1, get_adjective(faction_tgt), -1, -1);
            NetMsgPrimary.pop("ASSASSINATED", is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
        }
    } else if (action == 8) {
        if (HUMAN_BIT(faction) && faction == ConsoleGlobal.faction_id_) {
            parse_says(1, get_adjective(faction_tgt), -1, -1);
            popup.start("FREEWHO", 0x40);
            if (!Buffer::get_pcx_dimensions(is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx",
                                            &pcx_width, &pcx_height)
                && !popup.sprite_.init(
                       is_alien ? (char *)"al_infil_sm.pcx" : (char *)"infil_sm.pcx",
                       pcx_width, pcx_height)) {
                popup.caller_sprite_ = &popup.sprite_;
            }
            for (int k = 15; k < 21; k++) {
                if (FactionMonuments[faction_tgt].records_[k].achieved_) {
                    clear();
                    says(get_title(FactionMonuments[faction_tgt].records_[k].word_8_));
                    space();
                    says(get_name(FactionMonuments[faction_tgt].records_[k].word_8_));
                    space();
                    say_label(0xB4);
                    space();
                    says(get_noun(FactionMonuments[faction_tgt].records_[k].word_8_));
                    popup.item(StringTemp, FactionMonuments[faction_tgt].records_[k].word_8_);
                }
            }
            local_64 = popup.exec();
            if (local_64 < 0) {
                return 0;
            }
        }
        if (local_64 != -1) {
            home_base_id = Vehs[veh_id].home_base_id;
            if (home_base_id < 0) {
                veh_kill(veh_id);
            }
            GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ |= (uint8_t)(1 << local_64);
            setup_player(local_64, faction_tgt, 1);
            PlayersData[faction_tgt].unk_108--;
            for (int k = 15; k < 21; k++) {
                if (FactionMonuments[faction_tgt].records_[k].achieved_
                    && FactionMonuments[faction_tgt].records_[k].word_8_ == local_64) {
                    FactionMonuments[faction_tgt].records_[k].achieved_ = 0;
                }
            }
            if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_) {
                parse_says(0, get_title(local_64), -1, -1);
                parse_says(1, get_name(local_64), -1, -1);
                parse_says(2, get_noun(local_64), -1, -1);
                NetMsgPrimary.pop("WILLYFREE", "fac_escape_sm.pcx");
            }
            if (!(GameDataStateStorage.fields_.game_rules_ & 0x2000000)) {
                for (tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
                    if (has_tech(tech_id, local_64) && !has_tech(tech_id, faction)) {
                        net_tech(faction, tech_id, local_64, 0);
                    }
                }
            }
            ConsoleInfoWin.redraw_phase_ = 0;
            ConsoleGlobal.update_data(0);
            PlayersData[faction].diplo_friction[local_64] = 0;
            net_treaty_off(local_64, faction, 0x20, 1);
            net_treaty_on(faction, local_64, 0x0800000A, 0);
            net_set_treaty(local_64, faction, 0x02000000, 1, 0);
            net_treaty_on(local_64, faction, 0x01000001, 0);
            PlayersData[local_64].diplo_patience[faction] = 0;
            PlayersData[faction].diplo_spoke[local_64] = 0;
            diplomacy_check(faction, local_64, 0);
            PlayersData[faction].global_reputation++;
            PlayersData[local_64].global_reputation++;
        }
    } else if (action == 7) {
        atrocity(faction, faction_tgt, 1, 0);
        if (flag20_count < 1) {
            flag20_count++;
        }
        if (PlayersData[faction_tgt].flags & 0x400) {
            flag20_known++;
        }
        int known = range(flag20_known, 1, flag20_count);   // edx only
        PlayersData[faction_tgt].flags |= 0x400;
        int pop_loss = (flag20_count - known) * Bases[base_id].population_size / flag20_count;
        if (Bases[base_id].state & 0x20000) {
            pop_loss /= 2;
        }
        if (has_fac(0x11, base_id, 0)) {
            pop_loss /= 2;
        }
        if (has_fac(0x12, base_id, 0)) {
            pop_loss /= 2;
        }
        if (++pop_loss >= Bases[base_id].population_size) {
            pop_loss = Bases[base_id].population_size - 1;
        }
        for (int veh_id_hit = veh_at(Bases[base_id].x, Bases[base_id].y); veh_id_hit >= 0;
             veh_id_hit = veh_next_stack_id(veh_id_hit)) {
            // [ebp-0x44] reused as the damage amount; rnd() result spilled to [ebp-0x54]
            flag20_count = rnd((veh_health(veh_id_hit) + 1) / 2, NULL)
                           + veh_health(veh_id_hit) / 2;
            if (has_fac(0x11, base_id, 0)) {
                flag20_count /= 2;
            }
            if (has_fac(0x12, base_id, 0)) {
                flag20_count /= 2;
            }
            if (Bases[base_id].state & 0x20000) {
                flag20_count /= 2;
            }
            Vehs[veh_id_hit].dmg_incurred += flag20_count;
        }
        Bases[base_id].state |= 0x20000;
        if (pop_loss) {
            Bases[base_id].population_size -= pop_loss;
        }
        Bases[base_id].nutrients_accumulated = 0;
        spot_base(base_id, faction);
        draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
        if (faction == ConsoleGlobal.faction_id_ || faction_tgt == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & 0x80)) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_says(1, get_noun(faction), -1, -1);
            clear();
            says("GENETICWARFARE");
            if (faction == ConsoleGlobal.faction_id_) {
                says("0");
            } else {
                says("1");
            }
            popp(StringTemp, is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", NULL);
        }
    } else if (action == -1) {  // L_5a4101
        PlayersData[faction_tgt].proto_id_active[Vehs[veh_id_tgt].proto_id]--;
        if (Vehs[veh_id_tgt].proto_id > 0x40) {
            int proto_id = Vehs[veh_id_tgt].proto_id;
            proto_id = propose_proto(faction, VehPrototypes[proto_id].chassis_id,
                VehPrototypes[proto_id].weapon_id, VehPrototypes[proto_id].armor_id,
                VehPrototypes[proto_id].ability_flags,
                VehPrototypes[proto_id].reactor_id, -2,
                VehPrototypes[proto_id].veh_name);
            if (proto_id >= 0) {
                Vehs[veh_id_tgt].proto_id = (int16_t)proto_id;
            }
        }
        Vehs[veh_id_tgt].faction_id = (uint8_t)faction;
        PlayersData[faction].proto_id_active[Vehs[veh_id_tgt].proto_id]++;
        PlayersData[faction].mind_control_total++;
        PlayersData[faction_tgt].diplo_mind_control[faction]++;
        owner_set(Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y, faction);
        Vehs[veh_id_tgt].home_base_id = (int16_t)base_find(Vehs[veh_id_tgt].x,
            Vehs[veh_id_tgt].y, faction);
        Vehs[veh_id_tgt].moves_expended = 0;
        Vehs[veh_id_tgt].order = ORDER_NONE;
        Vehs[veh_id_tgt].state &= 0xFCFFBDFF;
        spot_stack(veh_id_tgt, faction_tgt);
        draw_tile(Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y, 2);
        if (faction_tgt == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.focus(Vehs[veh_id_tgt].x, Vehs[veh_id_tgt].y, faction);
            parse_says(1, get_noun(faction), -1, -1);
            NetMsgPrimary.pop("SUBVERTED",
                is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
        }
        if (diff_modifier) {
            return 1;
        }
        goto L_5a4b28;
    }
    {
        int prb_defense;
        int reloc_base;
        // 5a36ee..5a370d (C's address range) and 5a42ca/5a42cc are this ternary.
        prb_defense = ((action + 1) != 0)
            ? (has_fac(FAC_COVERT_OPS_CENTER, base_id, 0) ? 2 : 0) : 0;
        if ((int)rnd(morale_veh(veh_id, 1, 0)
                - range(PlayersData[faction_tgt].soc_effect_active.values_[SocialEffect::PROBE]
                    + prb_defense, -2, 0),
                "PROBESURVIVE") > diff_modifier) {
            reloc_base = base_find(Vehs[veh_id].x, Vehs[veh_id].y, faction);
            if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA
                && reloc_base >= 0 && !is_port(reloc_base, 0)) {
                for (reloc_base = 0;
                     reloc_base < GameDataStateStorage.fields_.base_current_count_;
                     reloc_base++) {
                    if (Bases[reloc_base].faction_id_current == faction
                        && is_port(reloc_base, 0)
                        && (!is_ocean(Vehs[veh_id].x, Vehs[veh_id].y)
                            || base_on_sea(reloc_base,
                                   region_at(Vehs[veh_id].x, Vehs[veh_id].y)))) {
                        break;
                    }
                }
                if (reloc_base >= GameDataStateStorage.fields_.base_current_count_) {
                    reloc_base = -1;
                }
                if (ProbeTargetHasHSA && (timeGetTime() & 1)) {
                    reloc_base = -1;
                    mode = -mode;
                }
            }
        } else if (!ProbeTargetHasHSA && ProbeHasAlgoEnhancement
                   && (timeGetTime() & 1)) {
            reloc_base = base_find(Vehs[veh_id].x, Vehs[veh_id].y, faction);
            if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA
                && reloc_base >= 0 && !is_port(reloc_base, 0)) {
                for (reloc_base = 0;
                     reloc_base < GameDataStateStorage.fields_.base_current_count_;
                     reloc_base++) {
                    if (Bases[reloc_base].faction_id_current == faction
                        && is_port(reloc_base, 0)
                        && (!is_ocean(Vehs[veh_id].x, Vehs[veh_id].y)
                            || base_on_sea(reloc_base,
                                   region_at(Vehs[veh_id].x, Vehs[veh_id].y)))) {
                        break;
                    }
                }
                if (reloc_base >= GameDataStateStorage.fields_.base_current_count_) {
                    reloc_base = -1;
                }
            }
        } else {
            reloc_base = -1;
            mode = -mode;
        }
        if (reloc_base < 0 && action == 8
            && VehPrototypes[Vehs[veh_id].proto_id].weapon_id != WPN_PROBE_TEAM) {
            reloc_base = home_base_id;
        }
        if (reloc_base >= 0) {
            veh_put(veh_id, Bases[reloc_base].x, Bases[reloc_base].y);
            Vehs[veh_id].terraforming_turns = 0;
            draw_tiles(Bases[base_id].x, Bases[base_id].y, 2);
            draw_tile(Bases[reloc_base].x, Bases[reloc_base].y, 2);
            parse_says(1, Bases[reloc_base].name_string, -1, -1);
            int morale_before = morale_veh(veh_id, 1, 0);
            Vehs[veh_id].morale = (uint8_t)range(Vehs[veh_id].morale + 1, 0, 6);
            veh_skip(veh_id);
            if (faction == ConsoleGlobal.faction_id_) {
                parse_says(0, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
                parse_say(2, Morale[morale_veh(veh_id, 1, 0)].name, -1, -1);
                clear();
                says("PROBESURVIVAL");
                if (action == 8) {
                    says("X");
                } else {
                    say_num(morale_before != morale_veh(veh_id, 1, 0)
                        && morale_veh(veh_id, 1, 0) >= 1);
                }
                if (faction == ConsoleGlobal.faction_id_) {
                    NetMsgPrimary.pop(StringTemp,
                        is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
                }
            }
        } else {
            veh_kill(veh_id);
        }
        if (action == 6) {
            if (faction == ConsoleGlobal.faction_id_
                || faction_tgt == ConsoleGlobal.faction_id_
                || spying(faction) || spying(faction_tgt)
                || (GameDataStateStorage.fields_.game_state_ & 0x80)) {
                if ((GameDataStateStorage.fields_.game_state_ & 0x80)
                    || Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
                    || (Bases[base_id].visibility & (1 << ConsoleGlobal.faction_id_))) {
                    ConsoleGlobal.focus(Bases[base_id].x, Bases[base_id].y,
                        Bases[base_id].faction_id_current);
                }
                parse_says(0, get_title(faction), -1, -1);
                parse_says(1, get_name(faction), -1, -1);
                parse_says(2, get_noun(faction), -1, -1);
                parse_says(3, Bases[base_id].name_string, -1, -1);
                parse_says(4, get_title(faction_tgt), -1, -1);
                parse_says(5, get_name(faction_tgt), -1, -1);
                clear();
                says("MINDCONTROL");
                says(faction == ConsoleGlobal.faction_id_ ? "0"
                    : faction_tgt == ConsoleGlobal.faction_id_ ? "1" : "2");
                popp(StringTemp, is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx", 0);
            }
            for (int veh_id_loop = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
                 veh_id_loop >= 0; veh_id_loop--) {
                if (veh_id_loop < GameDataStateStorage.fields_.vehicle_current_count_
                    && Vehs[veh_id_loop].faction_id == faction_tgt) {
                    int dist = vector_dist(Vehs[veh_id_loop].x, Vehs[veh_id_loop].y,
                        Bases[base_id].x, Bases[base_id].y);
                    if (dist <= 1
                        && (!dist || base_who(Vehs[veh_id_loop].x, Vehs[veh_id_loop].y) < 0)) {
                        PlayersData[faction_tgt].proto_id_active[Vehs[veh_id_loop].proto_id]--;
                        if (Vehs[veh_id_loop].proto_id > 0x40) {
                            int new_proto = propose_proto(faction,
                                VehPrototypes[Vehs[veh_id_loop].proto_id].chassis_id,
                                VehPrototypes[Vehs[veh_id_loop].proto_id].weapon_id,
                                VehPrototypes[Vehs[veh_id_loop].proto_id].armor_id,
                                VehPrototypes[Vehs[veh_id_loop].proto_id].ability_flags,
                                VehPrototypes[Vehs[veh_id_loop].proto_id].reactor_id, -2,
                                VehPrototypes[Vehs[veh_id_loop].proto_id].veh_name);
                            if (new_proto >= 0) {
                                Vehs[veh_id_loop].proto_id = (int16_t)new_proto;
                            }
                        }
                        Vehs[veh_id_loop].faction_id = (uint8_t)faction;
                        Vehs[veh_id_loop].home_base_id = (int16_t)base_id;
                        Vehs[veh_id_loop].moves_expended = 0;
                        PlayersData[faction].proto_id_active[Vehs[veh_id_loop].proto_id]++;
                        if (Chassis[VehPrototypes[Vehs[veh_id_loop].proto_id].chassis_id].range) {
                            Vehs[veh_id_loop].terraforming_turns = 0;
                        }
                        if (Vehs[veh_id_loop].order != ORDER_HOLD) {
                            Vehs[veh_id_loop].order = ORDER_NONE;
                        }
                        owner_set(Vehs[veh_id_loop].x, Vehs[veh_id_loop].y, faction);
                        Vehs[veh_id_loop].visibility |= (uint8_t)(1 << faction_tgt);
                    }
                }
            }
            if (!diff_modifier) {
                act_of_aggression(faction, faction_tgt);
            }
            capture_base(base_id, faction, 1);
        }
        if (faction == ConsoleGlobal.faction_id_ && reloc_base < 0) {
            if (Players[faction].rule_flags & RFLAG_ALIEN) {
                NetMsgPrimary.pop(action == 6 ? "PROBESUCCUMB" : "PROBECAUGHT",
                    "al_cap_sm.pcx");
            } else {
                NetMsgPrimary.pop(action == 6 ? "PROBESUCCUMB" : "PROBECAUGHT",
                    "capture_sm.pcx");
            }
        }
    }
L_5a4b15:
    if (action == 0) {
        return 1;
    }
L_5a4b28:
    if (!has_treaty(faction, faction_tgt, 7)) {
        return 1;
    }
    if (has_treaty(faction_tgt, faction, 1)
        && has_treaty(faction_tgt, faction, 0x2000000)) {
        return 1;
    }
    if (action == 6 && diff_modifier) {
        set_treaty(faction_tgt, faction, 0x420, 1);
        return 1;
    }
    if (mode < 0 && !HUMAN_BIT(-mode)) {
        mode = -mode;
        if (faction == ConsoleGlobal.faction_id_) {
            parse_says(0, get_title(mode), -1, -1);
            parse_says(1, get_name(mode), -1, -1);
            if (Players[faction].rule_flags & RFLAG_ALIEN) {
                NetMsgPrimary.pop("FRAMEFLOP", "al_cap_sm.pcx");
            } else {
                NetMsgPrimary.pop("FRAMEFLOP", "capture_sm.pcx");
            }
        }
        treaty_on(mode, faction, 0x10);
        mode = -mode;
    }
    if (mode < 0 && HUMAN_BIT(-mode)) {
        mode = -mode;
        parse_says(0, get_title(faction), -1, -1);
        parse_says(1, get_name(faction), -1, -1);
        parse_says(2, get_pact_hood(faction, mode), -1, -1);
        parse_says(3, get_noun(faction), -1, -1);
        parse_says(4, get_his_her(faction, 0), -1, -1);
        parse_says(5, get_him_her(faction, 0), -1, -1);
        parse_says(6, get_title(faction_tgt), -1, -1);
        parse_says(7, get_name(faction_tgt), -1, -1);
        parse_says(8, get_noun(faction_tgt), -1, -1);
        if (IsMultiplayerNet) {
            agenda_on(mode, faction, 0x20);
            if (mode == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop("HAVEFRAMEEXCUSE", 0);
            }
            if (faction == ConsoleGlobal.faction_id_) {
                parse_says(0, get_title(mode), -1, -1);
                parse_says(1, get_name(mode), -1, -1);
                parse_says(2, get_noun(mode), -1, -1);
                if (Players[faction].rule_flags & RFLAG_ALIEN) {
                    NetMsgPrimary.pop("HASFRAMEEXCUSE", "al_cap_sm.pcx");
                } else {
                    NetMsgPrimary.pop("HASFRAMEEXCUSE", "capture_sm.pcx");
                }
            }
        } else if (has_treaty(mode, faction, 1)) {
            if ((Players[faction].rule_flags & RFLAG_ALIEN)
                    ? popp("PACTFRAMEEXCUSE", "al_cap_sm.pcx", 0)
                    : popp("PACTFRAMEEXCUSE", "capture_sm.pcx", 0)) {
                pact_ends(faction, mode);
                PlayersData[mode].diplo_spoke[faction] =
                    GameDataStateStorage.fields_.turn_current_num_;
            }
        } else {
            if ((Players[faction].rule_flags & RFLAG_ALIEN)
                    ? popp("FRAMEEXCUSE", "al_cap_sm.pcx", 0)
                    : popp("FRAMEEXCUSE", "capture_sm.pcx", 0)) {
                treaty_on(faction, mode, 0x10);
                PlayersData[mode].diplo_spoke[faction] =
                    GameDataStateStorage.fields_.turn_current_num_;
            }
        }
        mode = -mode;
    }
    if (HUMAN_BIT(faction_tgt)) {
        parse_says(0, get_title(faction), -1, -1);
        parse_says(1, get_name(faction), -1, -1);
        parse_says(2, get_pact_hood(faction, faction_tgt), -1, -1);
        parse_says(3, get_noun(faction), -1, -1);
        parse_says(4, get_his_her(faction, 0), -1, -1);
        parse_says(5, get_him_her(faction, 0), -1, -1);
        if (IsMultiplayerNet) {
            agenda_on(faction_tgt, faction, 0x20);
            if (action >= 6) {
                PlayersData[faction].integrity_blemishes++;
            }
            if (faction_tgt == ConsoleGlobal.faction_id_) {
                if (local_58) {
                    clear();
                    says("BUSTED");
                    say_num(local_58);
                    popup.start(StringTemp);
                    if (Players[faction].rule_flags & RFLAG_ALIEN) {
                        if (!Buffer::get_pcx_dimensions("al_cap_sm.pcx",
                                &pcx_width, &pcx_height)
                            && !popup.sprite_.init((char *)"al_cap_sm.pcx",
                                pcx_width, pcx_height)) {
                            popup.caller_sprite_ = &popup.sprite_;
                        }
                    } else {
                        if (!Buffer::get_pcx_dimensions("capture_sm.pcx",
                                &pcx_width, &pcx_height)
                            && !popup.sprite_.init((char *)"capture_sm.pcx",
                                pcx_width, pcx_height)) {
                            popup.caller_sprite_ = &popup.sprite_;
                        }
                    }
                }
                popup.start("NETEXCUSE", local_58 ? 0x80 : 0);
                popup.exec();
            }
            if (faction == ConsoleGlobal.faction_id_) {
                parse_says(0, get_title(faction_tgt), -1, -1);
                parse_says(1, get_name(faction_tgt), -1, -1);
                parse_says(2, get_noun(faction_tgt), -1, -1);
                NetMsgPrimary.pop("HASNETEXCUSE", 0);
            }
        } else if (has_treaty(faction_tgt, faction, 1)) {
            if (local_58) {
                clear();
                says("BUSTED");
                say_num(local_58);
                popup.start(StringTemp);
                if (Players[faction].rule_flags & RFLAG_ALIEN) {
                    if (!Buffer::get_pcx_dimensions("al_cap_sm.pcx",
                            &pcx_width, &pcx_height)
                        && !popup.sprite_.init((char *)"al_cap_sm.pcx",
                            pcx_width, pcx_height)) {
                        popup.caller_sprite_ = &popup.sprite_;
                    }
                } else {
                    if (!Buffer::get_pcx_dimensions("capture_sm.pcx",
                            &pcx_width, &pcx_height)
                        && !popup.sprite_.init((char *)"capture_sm.pcx",
                            pcx_width, pcx_height)) {
                        popup.caller_sprite_ = &popup.sprite_;
                    }
                }
            }
            popup.start("PACTEXCUSE", local_58 ? 0x80 : 0);
            if (popup.exec()) {
                pact_ends(faction, faction_tgt);
                PlayersData[faction_tgt].diplo_spoke[faction] =
                    GameDataStateStorage.fields_.turn_current_num_;
            }
        } else {
            if (local_58) {
                clear();
                says("BUSTED");
                say_num(local_58);
                popup.start(StringTemp);
                if (Players[faction].rule_flags & RFLAG_ALIEN) {
                    if (!Buffer::get_pcx_dimensions("al_cap_sm.pcx",
                            &pcx_width, &pcx_height)
                        && !popup.sprite_.init((char *)"al_cap_sm.pcx",
                            pcx_width, pcx_height)) {
                        popup.caller_sprite_ = &popup.sprite_;
                    }
                } else {
                    if (!Buffer::get_pcx_dimensions("capture_sm.pcx",
                            &pcx_width, &pcx_height)
                        && !popup.sprite_.init((char *)"capture_sm.pcx",
                            pcx_width, pcx_height)) {
                        popup.caller_sprite_ = &popup.sprite_;
                    }
                }
            }
            popup.start("EXCUSE", local_58 ? 0x80 : 0);
            if (popup.exec()) {
                if (action < 6) {
                    treaty_on(faction, faction_tgt, 0x10);
                } else {
                    double_cross(faction, faction_tgt, -1);
                }
                PlayersData[faction_tgt].diplo_spoke[faction] =
                    GameDataStateStorage.fields_.turn_current_num_;
            }
        }
        return 1;
    }
    if (mode > 0) {
        treaty_on(faction_tgt, mode, 0x58);
        set_treaty(faction_tgt, mode, 0x20, 1);
        if (mode == ConsoleGlobal.faction_id_) {
            parse_says(0, get_title(faction_tgt), -1, -1);
            parse_says(1, get_name(faction_tgt), -1, -1);
            parse_says(2, get_noun(faction_tgt), -1, -1);
            NetMsgPrimary.pop("FRAMEJOB",
                is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
        }
        if (faction == ConsoleGlobal.faction_id_) {
            parse_says(0, get_title(faction_tgt), -1, -1);
            parse_says(1, get_name(faction_tgt), -1, -1);
            parse_says(2, get_noun(faction_tgt), -1, -1);
            parse_says(3, get_title(mode), -1, -1);
            parse_says(4, get_name(mode), -1, -1);
            parse_says(5, get_noun(mode), -1, -1);
            NetMsgPrimary.pop("FRAMED",
                is_alien ? "al_infil_sm.pcx" : "infil_sm.pcx");
        }
        return 1;  // 5a5509 (C's L_5a5501 / L_5a5509 tails land here)
    }
    diplomacy_check(faction, faction_tgt, 0);
    diplomacy_caption(faction, faction_tgt);
    parse_says(0, get_title(faction), -1, -1);
    parse_says(1, get_name(faction), -1, -1);
    parse_says(2, get_pact_hood(faction, faction_tgt), -1, -1);
    if (has_treaty(faction, faction_tgt, 1)) {
        if (action == 1
            && PlayersData[faction_tgt].tech_ranking > PlayersData[faction].tech_ranking
            && PlayersData[faction].ranking < 6 && !DiploScratchFA3C) {
            cause_friction(faction_tgt, faction, rnd(4, "PROBEFRICT"));
            if (faction == ConsoleGlobal.faction_id_) {
                clear();
                says("KEPTPACT");
                say_num(1);
                x_pops_minimal(StringTemp, FactionDiplomacyPortraits[faction_tgt], 0);
            }
        } else if (action < 6
                   && PlayersData[faction].ranking < PlayersData[faction_tgt].ranking
                   && PlayersData[faction].ranking < 5) {
            if (faction == ConsoleGlobal.faction_id_) {
                x_pops_minimal("EXCUSEDPACT", FactionDiplomacyPortraits[faction_tgt], 0);
            }
        } else {
            if (faction == ConsoleGlobal.faction_id_) {
                x_pops_minimal("USEDEXCUSE", FactionDiplomacyPortraits[faction_tgt], 0);
            }
            if (action < 6) {
                pact_ends(faction, faction_tgt);
            } else {
                double_cross(faction, faction_tgt, -1);
                treaty_on(faction, faction_tgt, 0x10);
            }
        }
    } else if (!has_treaty(faction_tgt, faction, 0x20) && action < 6
               && (((action == 1 || action == 5)
                    && PlayersData[faction_tgt].tech_ranking > PlayersData[faction].tech_ranking
                    && PlayersData[faction].ranking < 6 && !DiploScratchFA3C
                    && rnd(8, 0))
                   || ((action == 2 || action == 4)
                       && ((DiploScratchFA98 && !DiploScratchFA3C)
                           || (PlayersData[faction_tgt].ranking > PlayersData[faction].ranking
                               && !DiploScratchFA88 && rnd(8, 0)))))) {
        cause_friction(faction_tgt, faction, rnd(4, 0));
        if (faction == ConsoleGlobal.faction_id_) {
            clear();
            says("KEPT");
            say_num(action);
            x_pops_minimal(StringTemp, FactionDiplomacyPortraits[faction_tgt], 0);
        }
    } else {
        if (faction == ConsoleGlobal.faction_id_) {
            x_pops_minimal("USEDEXCUSE", FactionDiplomacyPortraits[faction_tgt], 0);
        }
        if (action < 6) {
            treaty_on(faction, faction_tgt, 0x10);
        } else {
            double_cross(faction, faction_tgt, -1);
        }
    }
    return 1;  // 5a5813/5a5816/5a5821: the function's final return
}

static inline char *probe_noun(int faction_id) {
    parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
    return Players[faction_id].noun_faction;
}
