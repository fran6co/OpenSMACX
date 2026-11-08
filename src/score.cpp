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
#include "worldwin.h"
#include "general.h"
#include "crt_random_inline.h"
#include "time.h"
#include "win.h"
#include "faction.h"
#include "tutwin.h"
#include "alpha.h"
#include "base.h"
#include "council.h"
#include "path.h"
#include "councwin.h"
#include "popup.h"
#include "dialogs.h"
#include "text.h"
#include "game.h"
#include "map.h"
#include "probe.h"
#include "technology.h"
#include "veh.h"
#include "netmsg.h"  // NetMsgPrimary, which the diplomacy bodies pop through
#include "netdaemon.h"  // NetDaemonNet, message_data - the net_* writers
#include "log.h"  // log_say, which the net_* writers announce each move with
#include "console.h"  // ConsoleGlobal, which net_energy refreshes
#include "tutorial_script.h"
#include "socialwin.h"
#include "mapwin.h"
#include "multiwin.h"
#include "planwin.h"
#include "alphamovie.h"
#include "strings.h"
#include "xpops.h"
#include "diplopop.h"
#include "diplowin.h"
#include "infowin.h"
#include "reportif.h"
#include "random.h"
#include "replaywin.h"
#include "datalink.h"
#include "faction_inline.h"  // has_treaty/has_agenda and the shared diplomacy helpers
#include "buffer.h"
#include "font.h"
#include "font_queue.h"
#include "maininterface.h"
#include "messagewin.h"
#include "terraforming.h"
#include "interlude.h"

// Score/ranking state: allocated in this TU's .bss, beside ReplayWin's.
// RECOVERY_OBJECT: v4 0x00945DD8 ?FactionRankingsUnk@@3PAIA target ?FactionRankingsUnk@@3PAHA
// The producer computes signed ranking scores and stores -1 while selecting
// winners; numeric consumers and pinned great_satan SETGE use signed ints.
// Game/network serialization copies this 0x20-byte array unchanged.
int FactionRankingsUnk[MaxPlayerNum];

// ORIGINAL: 0x005ABFF0 ?get_rating@@YAHHH@Z 0x005ABFF0-0x005AC051 FILE BYTE_EXACT
// LEVER: the archive's `g_faction_table[a1 * 2099]` is PlayersData's
//        diff_level - the image loads `[edx*4 + 0x96c9e8]` with edx =
//        faction * 0x833, which is PlayerData + 0x8 - and its `g_009a649c`
//        byte test is game.h's GameRules bit 0x40, RULES_INTENSE_RIVALRY.
//        Both named; nothing new declared.
// size      97 bytes
// prototype int (__cdecl ?get_rating@@YAHHH@Z)(int factionID, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// Purpose: A faction's score weight: its difficulty level plus the triangular
//          series the higher levels add, one more when RULES_INTENSE_RIVALRY
//          is on, scaled by arg2 over 0xa0.
// Status:  transcription homed from src/unrecovered/005abff0.cpp

int __cdecl get_rating(int faction_id, int arg2) {
    int value = PlayersData[faction_id].diff_level;
    int rating = value;
    for (int i = 3; i <= value; i++) {
        rating += (i - 1) / 2;
    }
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY) != 0) {
        rating++;
    }
    return (rating + 6) * arg2 / 0xa0;
}

/*
Purpose: Determine whether the specified base is considered an objective.
// ORIGINAL: 0x005AC060 ?is_objective@@YAHH@Z 0x005AC060-0x005AC10B BYTE_EXACT
// TRIED: the image still boolifies the `facilities_built[offset] & mask` test with a `neg/sbb/neg` 0-or-1 materialization into the old base_id stack slot before the final `je`, where this tree emits a plain `test`; hoisting `has_fac_built_call(...)` into its own named `BOOL` local before the `if` did not reproduce it (same 43/64, 0.927) - VC6 still proves the value is only used for one branch and drops the materialization.
// TRIED: `GameRules & X || Bases[base_id].event & Y` into two separate `if` statements (matching the image's two separate short-circuit branches instead of one `||`) moved 0.586 -> 0.762 similar. CORRECTION to the note this replaces: `calls agrees at exactly 1 (0x0050BA00, bitmask)` was wrong - the one call this tree made was to has_fac (out-of-line), not bitmask; the image inlines has_fac itself (its `cmp eax,0x41/jge` IS has_fac's own FacilityRepStart guard) and keeps ONLY the bitmask call inside that. Open-coding has_fac(ScnVictFacilityObj, base_id, 0) as its own `< FacilityRepStart` guard + has_fac_built() at the site removed the out-of-line has_fac call and moved best similarity 0.762 -> 0.783. `ScnVictFacilityObj` is a runtime facility id (read from a global, not a compile-time constant), so the folded `MEASURED inline bitmask` cannot constant-fold it away - routing it through `has_fac_built_call` (the real `bitmask()` call, same lever as cost_factor/pop_goal/num_objectives) reaches the call the image makes; call_diff agreed at 0/1 before, 1/1 after. Casting the `(int)ScnVictFacilityObj < FacilityRepStart` guard fixed a jge/jae signed-vs-unsigned split (ScnVictFacilityObj is uint32_t, compared unsigned without the cast). Moved 2/64 -> 43/64 agreeing, 0.586 -> 0.927 similar.
// TRIED: rewriting base_project()'s loop as an explicit pointer walk (`for (int *p = &SecretProject.human_genome_project; p < ... + MaxSecretProjectNum; p++)`) - the image's `cmp eax,0x9a6614/jl` reads like one, but the explicit walk makes VC6 test the pointer UNSIGNED (`jb`, 42/64) while the INDEX loop keeps the signed `jl` through strength reduction and test replacement (43/64, agreeing on `jl` exactly). Open-coding the bitmask call at block scope (int mask; int offset; bitmask_call(...); Bases[..].facilities_built[offset] & mask) instead of via `has_fac_built_call` also loses the whole shape - `push ecx` becomes `sub esp,0xc` and the image's `neg/sbb/neg` boolification never appears. Residual (listing_diff, 2 runs): the image materializes the `& mask` test 0-or-1 into base_id's own dead argument slot ([ebp+8]) with offset at [ebp-4]; this tree's inline has_fac_built assigns offset->[ebp-4] and mask->[ebp+8], the mirror image, and no source form tried at this call site flips it.
// size      171 bytes
// prototype int (__cdecl ?is_objective@@YAHH@Z)(int baseID)
// callers   14   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Is base an objective? true/false
Status: Complete
*/
BOOL __cdecl is_objective(int base_id) {
    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_ALL_BASE_COUNT_OBJ) {
        return true;
    }
    if (Bases[base_id].event & BEVENT_OBJECTIVE) {
        return true;
    }
    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_SP_COUNT_OBJ) {
        const int *projects = GameDataStateStorage.fields_.secret_project_.project_base;
        for (int i = 0; i < MaxSecretProjectNum; i++) {
            if (projects[i] == base_id) {
                return true;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ) {
        // Open-coded has_fac(ScnVictFacilityObj, base_id, 0): with
        // queue_count == 0 it reduces to has_fac's own early-out guard
        // followed by has_fac_built(), and the image inlines exactly that
        // (its `cmp eax, 0x41 / jge` IS has_fac's `facility_id >=
        // FacilityRepStart` guard) rather than calling has_fac out of line.
        if ((int)GameDataStateStorage.fields_.scenario_victory_facility_ < FacilityRepStart) {
            // Retain the Base before the helper replaces the consumed base_id
            // argument with its byte offset, then its normalized result.
            Base *base = &Bases[base_id];
            int mask;
            bitmask(GameDataStateStorage.fields_.scenario_victory_facility_,
                    &base_id, &mask);
            base_id = (base->facilities_built[base_id] & mask) != 0;
            if (base_id) {
                return true;
            }
        }
    }
    return false;
}

// is_objective() as the image inlines it here (value_of_base and
// garrison_check keep the out-of-line call).
static inline BOOL num_objectives_is_objective(int base_id) {
    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_ALL_BASE_COUNT_OBJ) {
        return true;
    }
    if (Bases[base_id].event & BEVENT_OBJECTIVE) {
        return true;
    }
    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_SP_COUNT_OBJ) {
        const int *projects = GameDataStateStorage.fields_.secret_project_.project_base;
        for (int i = 0; i < MaxSecretProjectNum; i++) {
            if (projects[i] == base_id) {
                return true;
            }
        }
    }
    if ((GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ)
        && has_fac(GameDataStateStorage.fields_.scenario_victory_facility_, base_id, 0)) {
        return true;
    }
    return false;
}

/*
Purpose: Count how far the specified faction has got towards the scenario's
         objectives.
// ORIGINAL: 0x005AC110 ?num_objectives@@YAHHH@Z 0x005AC110-0x005AC599
// size      1161 bytes
// prototype int (__cdecl ?num_objectives@@YAHHH@Z)(int factionID, int)
// callers   9   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050BA00 0x0050BA30 0x005AC110 0x005B9F20
// TRIED: routing has_fac_built(FAC_HEADQUARTERS, base_id) and/or
//            has_fac_built(ScnVictFacilityObj, base_id) through the new
//            `has_fac_built_call` helper (real bitmask() call, same lever
//            that won on pop_goal) measured WORSE here in all three
//            combinations tried (both sites, HQ only, ScnVictFacilityObj
//            only): best similarity across flag sets dropped from 0.201
//            (baseline, /c /O2 /Gy /GR- /Oy- /GX, 14/367 agreeing) to at
//            best 0.147. This function's size (1161 bytes, ~20 live locals
//            across two loops) likely means the extra out-of-line calls
//            shift register allocation enough to lose more than the call
//            shape gains. Left calling has_fac_built (folded) at both sites.
Return Value: Objective count, or zero for an eliminated faction
Status: Complete

Eleven separate scenario victory conditions add into one running total, each
gated by its own bit in GameRules or GameState, and a twelfth term - the count
the faction has already banked - is where the total starts. Nothing here is
cumulative across calls: the caller reads the return value.

The original inlines is_objective() (0x005AC060) at the objective-base test.
That is written as the call it was: the inlined sequence is instruction for
instruction the recovered body, down to has_fac()'s `>= FacilityRepStart`
guard appearing as `cmp eax, 0x41` and its zero queue_count folding the call to
has_fac_built(). The same holds for is_alive() at the entry and
has_fac_built(FAC_HEADQUARTERS) at the second unit test.

`unk_101` and `theory_of_everything` are read here under names that do not
describe what they hold; the offsets - 8324 and 872 into PlayerData - are what
the original reads, and neither field has a writer this recovery located.
*/
int __cdecl num_objectives(int faction_id, BOOL count_pact_factions) {
    if (!is_alive(faction_id)) {
        return 0;
    }
    int total = PlayersData[faction_id].unk_101;
    // Objective units that have reached a base that qualifies.
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        Veh *veh = &Vehs[veh_id];
        if (veh->faction_id != faction_id || !(veh->flags & VFLAG_IS_OBJECTIVE)) {
            continue;
        }
        if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_OBJ_BASE) {
            int base_id = base_at(veh->x, veh->y);
            if (base_id >= 0 && num_objectives_is_objective(base_id)) {
                int owner = Bases[base_id].faction_id_current;
                if (owner == faction_id
                    || ((PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)
                        && (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE))) {
                    total++; // a pact partner's base counts under 'One For All'
                }
            }
        } else if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE) {
            int base_id = base_at(veh->x, veh->y);
            if (base_id >= 0 && has_fac(FAC_HEADQUARTERS, base_id, 0)) {
                int owner = Bases[base_id].faction_id_current;
                if (owner == faction_id
                    || ((PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)
                        && (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE))) {
                    total++;
                }
            }
        } else {
            total++; // neither condition is set: the unit itself counts
        }
    }
    // Population, and bases that are objectives in their own right.
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current != faction_id) {
            continue;
        }
        if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_POPULATION_COUNT_OBJ) {
            total += Bases[base_id].population_size;
        }
        if (!(GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_ALL_BASE_COUNT_OBJ)
            && !(Bases[base_id].event & BEVENT_OBJECTIVE)) {
            continue;
        }
        total++;
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_TECH_COUNT_OBJ) {
        for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
            if (has_tech(tech_id, faction_id)) {
                total++;
            }
        }
        total += PlayersData[faction_id].theory_of_everything;
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_CREDITS_COUNT_OBJ) {
        total += PlayersData[faction_id].energy_reserves;
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            if (Bases[base_id].faction_id_current == faction_id
                && has_fac(GameDataStateStorage.fields_.scenario_victory_facility_, base_id, 0)) {
                total++;
            }
        }
    }
    /*
    * Both map walks step one Map along per tile visited rather than indexing,
    * because a row holds only every other x and the rows pack together; the
    * pointer is not reset between rows.
    */
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ) {
        Map *tile = map_tiles();
        for (int y = 0; y < MapLatitudeBounds; y++) {
            for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
                if (tile->territory == faction_id) {
                    total += bit_count(tile->bit & 0x81E4885C);
                }
                tile++;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCN_VICT_TERRITORY_COUNT_OBJ) {
        Map *tile = map_tiles();
        for (int y = 0; y < MapLatitudeBounds; y++) {
            for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
                if (tile->territory == faction_id) {
                    total++;
                }
                tile++;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_SP_COUNT_OBJ) {
        for (int i = 0; i < MaxSecretProjectNum; i++) {
            int base_id = GameDataStateStorage.fields_.secret_project_.project_base[i];
            if (base_id >= 0 && Bases[base_id].faction_id_current == faction_id) {
                total++;
            }
        }
    }
    // One recursion deep only: the pact partners are asked with the flag off.
    if (count_pact_factions && (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE)) {
        for (int other_id = 1; other_id < MaxPlayerNum; other_id++) {
            if (faction_id != other_id
                && (PlayersData[faction_id].diplo_treaties[other_id] & DTREATY_PACT)) {
                total += num_objectives(other_id, false);
            }
        }
    }
    return total;
}

// ORIGINAL: 0x005AC5A0 ?most_objectives@@YAHPAHPAH@Z 0x005AC5A0-0x005AC62F FILE BYTE_EXACT
// LEVER: the scaffold's two carried bindings are both modelled names already
//        in the tree - 0x0096C9F8 is faction.h's PlayersData[0].diplo_treaties
//        (the array front 0x0096C9E0 + 0x18) and 0x009A649C is game.h's
//        GameRules - so the raw `[best_idx + off]` indexing over the 0x833
//        dword stride becomes PlayersData[faction_id].diplo_treaties[best].
// symbol    ?most_objectives@@YAHPAH0@Z
// LEVER: the image spells the same two `int *` parameters out in full
//        (@YAHPAHPAH@Z); VC6 abbreviates the repeat, so this tree's own symbol
//        is the `0` form. Same type, same address, shorter mangled name.
// size      143 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005AC110
// Purpose: Score the cooperative-victory winner: the faction with the most
//          current objectives. When RULES_VICTORY_COOPERATIVE is set the
//          second out-parameter also counts how many factions are tied with
//          the leader AND hold a pact (DTREATY_PACT) with it; the caller
//          awards the shared win when that count reaches the tie count.
// Status:  transcription homed from src/unrecovered/005ac5a0.cpp

int __cdecl most_objectives(int *faction_id_out, int *pact_count_out) {
    int best_count = 0;
    int best_faction = 0;
    if (pact_count_out != 0) {
        *pact_count_out = 0;
    }
    int faction_id = 1;
    do {
        int count = num_objectives(faction_id, GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE);
        if (count >= best_count) {
            if (pact_count_out != 0) {
                if (count > best_count) {
                    *pact_count_out = 0;
                } else if ((PlayersData[faction_id].diplo_treaties[best_faction] & DTREATY_PACT) == 0
                           || (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE) == 0) {
                    ++*pact_count_out;
                }
            }
            best_count = count;
            best_faction = faction_id;
        }
        ++faction_id;
    } while (faction_id < MaxPlayerNum);
    if (faction_id_out != 0) {
        *faction_id_out = best_faction;
    }
    return best_count;
}

/*
Purpose: Check if specified faction is currently building Ascent to Transcendence. This code isn't
         used by original game. There was also a bug where it compares to a non-negative queue id.
// ORIGINAL: 0x005AC630 ?transcending@@YAHH@Z 0x005AC630-0x005AC67A BYTE_EXACT
// size      74 bytes
// prototype int (__cdecl ?transcending@@YAHH@Z)(int factionID)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is faction transcending? true/false
Status: Complete
*/
BOOL __cdecl transcending(int faction_id) {
    if (!ascending(faction_id)) {
        return false;
    }
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        // NOT NEGATED, and that is the image rather than a slip here. The
        // bytes at 0x005AC65C are `83 38 66` - `cmp dword ptr [eax], 0x66` -
        // a POSITIVE 102, where every other site in the tree writes
        // `-FAC_ASCENT_TO_TRANSCENDENCE` for a queued facility.
        //
        // The original disagrees with itself: `at_climax` (0x00539EF0) tests
        // the same facility with `cmp dword ptr [eax + 0x10], -0x66`, the
        // negated form. One of the two shipped wrong, and by the convention
        // the rest of the game follows - facilities queued as negative ids -
        // it is this one. Reproduced as shipped, the same way
        // `Probe::success_rate` reproduces its own halving bug: this is a
        // matching decompilation, not a fix.
        if (Bases[i].faction_id_current == faction_id
            && Bases[i].queue_production_id[0] == FAC_ASCENT_TO_TRANSCENDENCE) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Check if Voice of Planet has been built that starts the Ascent to Transcendence sequence.
// ORIGINAL: 0x005AC680 ?ascending@@YAHH@Z 0x005AC680-0x005AC68F BYTE_EXACT
// body      src/score.cpp
// size      15 bytes
// prototype int (__cdecl ?ascending@@YAHH@Z)(int factionID)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Has Voice of Planet been built? true/false
Status: Complete
*/
BOOL __cdecl ascending(int UNUSED(faction_id) faction_id) {
    return base_project(SP_VOICE_OF_PLANET) != SP_Unbuilt;
}

// has_project() over the plain project_base array: the image walks a pointer.
static inline BOOL rankings_has_project(int project_id, int faction_id) {
    int base_id = GameDataStateStorage.fields_.secret_project_.project_base[project_id];
    if (base_id < 0) {
        return false;
    }
    return Bases[base_id].faction_id_current == faction_id;
}

/*
Purpose: Rescore every faction's power, and optionally publish the ranking order that scoring
         implies plus the betrayals the new order provokes.
// ORIGINAL: 0x005AC690 ?rankings@@YAXH@Z 0x005AC690-0x005ACBD6
// TRIED: `call_diff` already agrees (0 disagree) across all seven
// callees. MISMATCH is 10/425 at this pass; first divergence is in the
// prologue's register/constant scheduling (image pushes ebx/esi/edi as
// plain callee-saves before touching any parameter, this tree's O2 loads
// `edx=1` and `ecx=0x9ac598` first) rather than a call-shape or semantic
// gap. Not chased at the instruction level this pass - 1350 image bytes
// across likely the largest body in this batch, well outside its budget.
//        TRIED (2026-09-03): `has_tech_call` in the growth-value loop - the
//        image CALLS 0x005B9F20 twice (0x005AC779 / 0x005AC823) and
//        technology.h's MEASURED inline was open-coded, 39 instructions'
//        worth (compiled 441 vs the image's 425). 5/425 -> 8/425 agreeing
//        and compiled now 422. Still MISMATCH; the rest is the prologue
//        scheduling above.
// size      1350 bytes
// prototype
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00539C00 0x00539E40 0x0053A100 0x0055BB30 0x005AC110 0x005B9F20 0x0064601D
Return Value: n/a
Status: Complete

Two jobs behind one flag. The scoring pass always runs; `apply_ranks` adds the publication - the
1 to 7 ordering in PlayerData::ranking, the FactionRankings index, the highest and lowest live
factions, and a per-faction roll to turn on the leader. turn_upkeep passes 1.

SCORING, per faction 1 through 7. Faction zero is native life: its ranking is cleared and it is
never scored. A faction not alive scores zero. A scenario with an objective victory in play and
no per-objective point value scores ten per objective and stops there. Otherwise the score is

  4 * (pop_total + theory_of_everything)
  + for each technology held, its growth + tech + wealth + power values
  + 10 per secret project standing in one of its bases
  + num_objectives * ObjectiveAchievePts
  + for each prototype, a value built from cost * how many are active

and the prototype term is the interesting one. The count is skipped above 250, which leaves
251-255 as sentinels rather than counts. A weapon with no offensive rating is worth a quarter of
cost * count, a negative rating half, and a rating of 99 or more the whole of it; anything else
is scaled by rating / the best weapon the factions have between them. That best is computed once
before the faction loop by scanning the 448 faction-owned prototype slots - 64 through 511 - for
the largest offense rating below 99 on a prototype that is both PROTO_ACTIVE and
PROTO_TYPED_COMPLETE, and it starts at 1 so the division can never be by zero.

The score is also archived: for the first 1000 turns its low 16 bits go into
FactionRankingHistory, indexed faction + turn * 8.

PUBLICATION. Ranks are handed out highest score first, 7 down to 1, and the winner's score is
struck out with -1 so the next pass finds the next one. A pass where nothing beats -1 awards the
rank to faction 0, which is how a dead or all-negative field terminates. FactionRankings is then
the inverse map, rank to faction, filled only for living factions; RankingFactionIDUnk1 and
RankingFactionIDUnk2 are the highest- and lowest-ranked HUMAN factions. The two FactionsStatus
bytes are read separately throughout - [1] is the alive bitfield and [0] the human one - and the
human byte is read in both directions: the ranked-human search wants the bit set, and the
betrayal roll skips any faction that has it.

Then the target: the top-ranked faction if great_satan says so, overridden during a climactic
battle by the last live faction on difficulty 4 or above (or any live faction under intense
rivalry). No target clears STATE_UNK_200 and returns. A target sets it, counts how many of the
seven hold a vendetta against it, and rolls once per faction: reputation, plus 4 each for a
climactic battle, for PFLAG_UNK_20 in flags_ext and for wanting revenge, all multiplied by the
target's difficulty level - or by 5 flat under intense rivalry - against
rnd((vendetta_count - ai_fight + 2) * 256). Winning the roll sets DTREATY_SHALL_BETRAY.

Verification note: the sweep against recovery-gameplay-tests kills 100 of 109 valid mutants. The
nine survivors are all dead code or equivalences, not untested behaviour:

  - The technology loop tests `tech_id < MaxTechnologyNum` inside a loop whose own bound is
    MaxTechnologyNum, so the `: 4` arm at 0x005AC78D cannot be taken. That accounts for two
    survivors, the guard and its constant. It is transcribed rather than folded away because it
    is what the original does.
  - Widening that loop's own bound reaches tech 89, which has_tech refuses on its own bound of
    MaxTechnologyNum - 1, so the extra pass scores nothing.
  - `offense >= best_offense` against `>` differs only when they are equal, where the assignment
    is a no-op.
  - `offense < 0` against `< 1` or `<= 0` differ only at zero, which the preceding `!offense` has
    already taken.
  - The rank-to-faction index loop starting at 0 writes faction zero's id into a slot the clear
    two lines above has just zeroed, and any live faction sharing that rank overwrites it
    afterwards, so the extra pass cannot be seen. Widening its bound - and the climactic
    override's - to faction 8 reaches is_alive(8) and is_human(8), which read bit 8 of a
    one-byte status field and are therefore always false.

The betrayal filter carries a third piece of dead code that the sweep does not reach: it requires
DTREATY_TREATY or DTREATY_TRUCE, then rejects DTREATY_PACT, DTREATY_SHALL_BETRAY and
DTREATY_UNK_800 together (`test eax, 0xC01` at 0x005ACAC0), so by the time DTREATY_UNK_4000000 is
tested at 0x005ACACB the DTREATY_UNK_800 half of its condition (`test ah, 8` at 0x005ACAD2) is
already known false and the arm always continues. Separately, DTREATY_SHALL_BETRAY's own
rejection is the one filter no fixture can observe: the only effect a betrayal has is to OR that
same bit in, so a faction already carrying it reads identically either way.
*/
void __cdecl rankings(int apply_ranks) {
    int best_offense = 1;
    for (int proto_id = MaxVehProtoFactionNum; proto_id < MaxVehProtoNum; proto_id++) {
        if (VehPrototypes[proto_id].flags & PROTO_ACTIVE
            && VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE
            && get_proto_offense_rating(proto_id) >= best_offense
            && get_proto_offense_rating(proto_id) < 99) {
            best_offense = get_proto_offense_rating(proto_id);
        }
    }
    for (int faction_id = 0; faction_id < MaxPlayerNum; faction_id++) {
        if (apply_ranks) {
            PlayersData[faction_id].ranking = 0;
        }
        if (!faction_id) {
            continue;
        }
        if (!is_alive(faction_id)) {
            FactionRankingsUnk[faction_id] = 0;
        } else if ((ScenarioSave.objective_req_victory < 9000 || (int)ScenarioSave.objectives_sudden_death_victory < 9000)
            && !ScenarioSave.objective_achieve_points) {
            FactionRankingsUnk[faction_id] = num_objectives(faction_id, false) * 10;
        } else {
            FactionRankingsUnk[faction_id] = (PlayersData[faction_id].theory_of_everything
                + PlayersData[faction_id].pop_total) * 4;
            for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
                // has_tech_call: the image CALLS 0x005B9F20 twice from here.
                if (has_tech(tech_id, faction_id)) {
                    if (tech_id >= MaxTechnologyNum) {
                        FactionRankingsUnk[faction_id] = FactionRankingsUnk[faction_id] + 4;
                    } else {
                        FactionRankingsUnk[faction_id] = FactionRankingsUnk[faction_id]
                            + (Technology[tech_id].growth_value
                            + Technology[tech_id].tech_value + Technology[tech_id].wealth_value
                            + Technology[tech_id].power_value);
                    }
                }
            }
            for (int i = 0; i < MaxSecretProjectNum; i++) {
                if (rankings_has_project(i, faction_id)) {
                    FactionRankingsUnk[faction_id] += 10;
                }
            }
            FactionRankingsUnk[faction_id] += num_objectives(faction_id, false) * (int)ScenarioSave.objective_achieve_points;
            for (int proto_id = 0; proto_id < MaxVehProtoNum; proto_id++) {
                if (proto_id < MaxVehProtoFactionNum
                    && !has_tech(VehPrototypes[proto_id].preq_tech, faction_id)) {
                    continue;
                }
                if (PlayersData[faction_id].proto_id_active[proto_id] > 250) {
                    continue;
                }
                int value = VehPrototypes[proto_id].cost * PlayersData[faction_id].proto_id_active[proto_id];
                if (value) {
                    int offense = Weapon[VehPrototypes[proto_id].weapon_id].offense_rating;
                    if (!offense) {
                        FactionRankingsUnk[faction_id] += value / 4;
                    } else if (offense < 0) {
                        FactionRankingsUnk[faction_id] += value / 2;
                    } else if (offense >= 99) {
                        FactionRankingsUnk[faction_id] += value;
                    } else {
                        FactionRankingsUnk[faction_id] += (offense / best_offense) * value;
                    }
                }
            }
        }
        if (GameDataStateStorage.fields_.turn_current_num_ < MaxRankingHistoryTurns) {
            FactionRankingHistory[faction_id + GameDataStateStorage.fields_.turn_current_num_ * MaxPlayerNum]
                = (uint16_t)FactionRankingsUnk[faction_id];
        }
    }
    if (!apply_ranks) {
        return;
    }
    int rank;
    for (rank = MaxPlayerNum - 1; rank > 0; rank--) {
        int best_score = -1;
        int best_faction = 0;
        for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
            if (FactionRankingsUnk[faction_id] > best_score) {
                best_faction = faction_id;
                best_score = FactionRankingsUnk[faction_id];
            }
        }
        FactionRankingsUnk[best_faction] = -1;
        PlayersData[best_faction].ranking = (uint32_t)rank;
    }
    memset(GameDataStateStorage.fields_.faction_rankings_, 0, sizeof(GameDataStateStorage.fields_.faction_rankings_));
    for (faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
        if ((1 << faction_id) & GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_) {
            GameDataStateStorage.fields_.faction_rankings_[PlayersData[faction_id].ranking] = (uint32_t)faction_id;
        }
    }
    GameDataStateStorage.fields_.ranking_faction_id_unk1_ = 0;
    GameDataStateStorage.fields_.ranking_faction_id_unk2_ = 0;
    int humans = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
    for (rank = MaxPlayerNum - 1; rank >= 0; rank--) {
        if (humans & (1 << GameDataStateStorage.fields_.faction_rankings_[rank])) {
            GameDataStateStorage.fields_.ranking_faction_id_unk1_ = GameDataStateStorage.fields_.faction_rankings_[rank];
            break;
        }
    }
    for (rank = 0; rank < MaxPlayerNum; rank++) {
        if (humans & (1 << GameDataStateStorage.fields_.faction_rankings_[rank])) {
            GameDataStateStorage.fields_.ranking_faction_id_unk2_ = GameDataStateStorage.fields_.faction_rankings_[rank];
            break;
        }
    }
    int target_id = great_satan(GameDataStateStorage.fields_.faction_rankings_[MaxPlayerNum - 1], false)
        ? (int)GameDataStateStorage.fields_.faction_rankings_[MaxPlayerNum - 1] : 0;
    if (climactic_battle()) {
        humans = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
        for (faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
            if (humans & (1 << faction_id)
                && (PlayersData[faction_id].diff_level >= 4
                    || GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)) {
                target_id = faction_id;
            }
        }
    }
    if (target_id) {
        GameDataStateStorage.fields_.game_state_ |= STATE_UNK_200;
        int vendetta_count = 0;
        for (faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
            if (PlayersData[faction_id].diplo_treaties[target_id] & DTREATY_VENDETTA) {
                vendetta_count++;
            }
        }
        for (faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
            if (is_human(faction_id)) {
                continue;
            }
            uint32_t treaty = PlayersData[faction_id].diplo_treaties[target_id];
            if (!(treaty & (DTREATY_PACT | DTREATY_TREATY | DTREATY_TRUCE))) {
                continue;
            }
            if (treaty & (DTREATY_PACT | DTREATY_SHALL_BETRAY | DTREATY_UNK_800)) {
                continue;
            }
            if (treaty & DTREATY_UNK_4000000 && !(treaty & DTREATY_UNK_800)) {
                continue;
            }
            int weight;
            if (GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY) {
                weight = 5;
            } else {
                weight = PlayersData[target_id].diff_level;
            }
            int revenge = PlayersData[target_id].diplo_treaties[faction_id] & DTREATY_WANT_REVENGE;
            int urge = (reputation(target_id, faction_id)
                + (climactic_battle() ? 4 : 0)
                + ((PlayersData[faction_id].flags_ext & PFLAGEXT_SHAMELESS_BETRAY_HUMANS) ? 4 : 0)
                + (revenge ? 4 : 0))
                * weight;
            if ((int)bounded_crt_random((vendetta_count - PlayersData[faction_id].ai_fight + 2) * 256) < urge) {
                set_treaty(faction_id, target_id, DTREATY_SHALL_BETRAY, true);
            }
        }
    } else {
        GameDataStateStorage.fields_.game_state_ &= ~STATE_UNK_200;
    }
}

// Original score calculation clears all eight weights, then publishes the
// aggregate after dividing the transcendence bonus among faction populations.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.


// ORIGINAL: 0x005ACBE0 ?compute_score@@YAXHPAHPAHH@Z 0x005ACBE0-0x005AD3AD
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/005acbe0.cpp", "source:src/faction.h", "image:complete original instructions and independent storage/lifecycle evidence"], "reason": "Recover all original faction-score branches, recursive updates and publication in the base/objective domain through genuine Base, PlayerData and Rules fields; original signed population, word wrapping, bonus order and malformed-state paths remain explicit."}
// symbol    ?compute_score@@YAXHPAH0H@Z
// LEVER: plain int arithmetic; owner != faction branch first; set_base kept
//        out of line (auto_inline off); has_project for both project scans;
//        commerce `+= cond ? receipt : 0`; *total read once at the end.
//        Remaining gap: pointer-induction register choice in the commerce
//        and transcendence loops, index (not pointer) project walk.
// has_project with the project table indexed as a plain array: the image
// walks it with an absolute pointer, which the operator[] spelling loses.
static inline BOOL compute_score_has_project(int project_id, int faction_id) {
    int base_id = GameDataStateStorage.fields_.secret_project_.project_base[project_id];
    if (base_id < 0) {
        return false;
    }
    return Bases[base_id].faction_id_current == faction_id;
}

void __cdecl compute_score(int faction, int *total, int *items, int recursive) {
    int other_items[9];
    int other_total;
    if ((GameDataStateStorage.fields_.game_state_ & 8) && !recursive) return;
    if (!total) total = &PlayersData[faction].unk_109;
    if (!items) items = PlayersData[faction].score_items_;
    *total = 0;
    if (!recursive) PlayersData[faction].unk_107 = 0;
    for (int item = 0; item < 9; ++item) items[item] = 0;

    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        const int owner = Bases[base].faction_id_current;
        if (owner != faction) {
            if (!(PlayersData[faction].flags & 0x60000)) {
                const uint32_t treaty = PlayersData[owner].diplo_treaties[faction];
                if (treaty & 0x2000000) {
                    if (treaty & 1) items[0] += Bases[base].population_size;
                } else if ((treaty & 1) && (GameDataStateStorage.fields_.game_rules_ & 0x1000)) {
                    items[0] += Bases[base].population_size / 2;
                }
            } else if ((PlayersData[faction].diplo_treaties[owner] & 1)
                       && (GameDataStateStorage.fields_.game_rules_ & 0x1000)) {
                items[0] += Bases[base].population_size;
            } else {
                items[0] += Bases[base].population_size / 2;
            }
        } else {
            items[0] += Bases[base].population_size;
            set_base(base);
            base_compute(0);
            for (int other = 0; other < 7; ++other) {
                // Original tests raw rule flags, not ExpansionEnabled.
                if (!(Players[faction].rule_flags & 0x80) && !(Players[other + 1].rule_flags & 0x80)) {
                    items[1] += (!PlayersData[faction].sanction_turns
                                 && !PlayersData[other + 1].sanction_turns)
                        ? BaseCommerceReceipts[other] : 0;
                }
            }
        }
    }
    for (int technology = 0; technology < 89; ++technology)
        if (has_tech(technology, faction)) items[2]++;
    items[3] = PlayersData[faction].theory_of_everything * 10;
    int objective_points = static_cast<int>(ScenarioSave.objective_achieve_points);
    if (objective_points < 1) objective_points = 1;
    items[6] = objective_points * num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
    for (int project = 0; project < 64; ++project) {
        if (compute_score_has_project(project, faction)) items[4] += 25;
    }

    GameDataStateStorage.fields_.transcendence_score_bonus_ = 0;
    for (int slot = 0; slot < 8; ++slot) GameDataStateStorage.fields_.transcendence_score_weights_[slot] = 0;
    const int transcendence_base = GameDataStateStorage.fields_.secret_project_.ascent_to_transcendence();
    if (transcendence_base >= 0) {
        GameDataStateStorage.fields_.transcendence_score_bonus_ = 300;
        const int scaled = (1000 - GameDataStateStorage.fields_.turn_current_num_) * 2;
        if (scaled >= 300) GameDataStateStorage.fields_.transcendence_score_bonus_ = scaled;
        int aggregate = 0;
        int own_weight = 0;
        int leader = -1;
        for (int candidate = 1; candidate < 8; ++candidate) {
            if (has_project(SP_ASCENT_TO_TRANSCENDENCE, candidate)) {
                leader = candidate;
                break;
            }
        }
        if (leader >= 0) {
            aggregate = PlayersData[leader].pop_total * 2;
            GameDataStateStorage.fields_.transcendence_score_weights_[leader] = aggregate;
            if (leader == faction) own_weight = aggregate;
        }
        for (int other = 1; other < 8; ++other) {
            // Invalid/no leader leaves index -1, as in the original. Do not
            // hide that malformed-state access or the possible zero divisor.
            if (leader != other && (PlayersData[other].diplo_treaties[leader] & 1)) {
                aggregate += PlayersData[other].pop_total;
                GameDataStateStorage.fields_.transcendence_score_weights_[other] = PlayersData[other].pop_total;
                if (other == faction) own_weight += PlayersData[other].pop_total;
            }
        }
        items[6] = (GameDataStateStorage.fields_.transcendence_score_bonus_ * own_weight) / aggregate;
        // Original publishes aggregate only AFTER the signed division.
        GameDataStateStorage.fields_.transcendence_score_weights_[0] = aggregate;
    }

    // Clamp written on the expression itself (sub; shl; sets; dec; and).
    items[7] = ((500 - GameDataStateStorage.fields_.turn_current_num_) * 2 < 0)
        ? 0 : (500 - GameDataStateStorage.fields_.turn_current_num_) * 2;
    if (!(GameDataStateStorage.fields_.game_rules_ & 2)) items[7] = 0;
    if ((GameDataStateStorage.fields_.game_state_ & 0x2000) && (GameDataStateStorage.fields_.game_rules_ & 2) && items[7]) {
        int matching_population = 0;
        int total_population = 0;
        for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
            const int population = Bases[base].population_size;
            total_population += population;
            const int owner = Bases[base].faction_id_current;
            if (owner == faction || ((PlayersData[owner].diplo_treaties[faction] & 1)
                && (PlayersData[owner].diplo_treaties[faction] & 0x2000000)))
                matching_population += population;
        }
        if (total_population)
            items[7] = (PlayersData[faction].score_items_[7] * matching_population) / total_population;
    } else {
        items[7] = 0;
        const uint32_t flags = PlayersData[faction].flags;
        if (flags & 0x20000) {
            int bonus = (600 - GameDataStateStorage.fields_.turn_current_num_) * 2;
            if (bonus < 200) bonus = 200;
            items[7] = bonus;
        } else if (flags & 0x40000) {
            int bonus = (600 - GameDataStateStorage.fields_.turn_current_num_) * 2;
            if (bonus < 200) bonus = 200;
            items[7] = bonus;
        } else if (flags & 0x100000) {
            int bonus = (1000 - GameDataStateStorage.fields_.turn_current_num_) * 2;
            if (bonus < 300) bonus = 300;
            items[7] = bonus;
        } else {
            for (int other = 1; other < 8; ++other) {
                if (PlayersData[other].flags & 0x20000) {
                    if ((PlayersData[faction].diplo_treaties[other] & 1) && (GameDataStateStorage.fields_.game_rules_ & 0x1000)) {
                        int bonus = 600 - GameDataStateStorage.fields_.turn_current_num_;
                        if (bonus < 100) bonus = 100;
                        items[7] = bonus;
                    }
                } else if (PlayersData[other].flags & 0x40000) {
                    if ((PlayersData[faction].diplo_treaties[other] & 1) && (GameDataStateStorage.fields_.game_rules_ & 0x1000)) {
                        int bonus = 600 - GameDataStateStorage.fields_.turn_current_num_;
                        if (bonus < 100) bonus = 100;
                        items[7] = bonus;
                    }
                }
            }
        }
    }
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << faction))) {
        items[7] = 0;
        items[8] = 0;
    }

    int required = ScenarioSave.objective_req_victory;
    if ((required < 9000 || static_cast<int>(ScenarioSave.objectives_sudden_death_victory) < 9000)
        && !(GameDataStateStorage.fields_.game_state_ & 0x80000)) {
        // Intentionally clears the faction's GLOBAL score cache even when
        // caller supplied a separate local items array in recursive mode.
        for (int item = 0; item < 9; ++item)
            if (item != 6) PlayersData[faction].score_items_[item] = 0;
    }
    if (!PlayersData[faction].score_items_[7]) {
        if (!required) {
            int best = most_objectives(0, 0);
            if (best < 1) best = 1;
            else best = most_objectives(0, 0); // Original calls twice here.
            if (num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000) >= best)
                items[7] = static_cast<int>(ScenarioSave.victory_achieve_bonus_points);
        } else {
            const int mine = num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
            required = ScenarioSave.objective_req_victory; // Original re-reads after call.
            if (mine >= required) items[7] = static_cast<int>(ScenarioSave.victory_achieve_bonus_points);
        }
    }
    for (int total_item = 0; total_item < 9; ++total_item)
        *total += items[total_item];
    if (MapNativeLifeForms != 1 && !(GameDataStateStorage.fields_.game_state_ & 0x10)) {
        const int difference = (static_cast<int>(MapNativeLifeForms) - 1) * *total / 4;
        items[5] = difference;
        *total += difference;
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x80000) {
        if (recursive) return;
        int best = 0;
        for (int other = 1; other < 8; ++other) {
            compute_score(other, &other_total, other_items, 1);
            if (other_total > best) best = other_total;
        }
        if (*total >= best) {
            int bonus = static_cast<int>(ScenarioSave.victory_achieve_bonus_points);
            if (!bonus) bonus = 1000;
            items[7] = bonus; // Original does NOT add this late bonus to total.
        }
    } else if (recursive) return;
    const int score = *total;
    PlayersData[faction].unk_107 = score;
    if ((GameDataStateStorage.fields_.game_rules_ & 0x400) && !(GameDataStateStorage.fields_.game_state_ & 0x20))
        PlayersData[faction].unk_107 = score * 2;
}
