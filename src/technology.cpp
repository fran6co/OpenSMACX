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
#include "technology.h"
#include "alpha.h"
#include "base.h"
#include "game.h"
#include "general.h"
#include "map.h"
#include "random.h"
#include "strings.h"
#include "veh.h"

int TechValidCount;  // 0x00949730
int TechCommerceCount;  // 0x00949734
char TechName[80];

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
// ORIGINAL: 0x0057CE50 ?valid_tech_leap@@YAHHH@Z 0x0057CE50-0x0057CF9E
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
BOOL __cdecl valid_tech_leap(int tech_id, int faction_id) {
    // EACH `&&` IS TWO SEPARATE EARLY RETURNS, not one combined test: the
    // image tests every has_tech() call on its own `je`, and re-reads
    // Technology[tech_id].preq_tech_1/2 from the array rather than keeping
    // an earlier read live across a has_tech()/weapon_budget() call.
    if (Technology[tech_id].preq_tech_1 >= 0) {
        if (!has_tech_call(Technology[Technology[tech_id].preq_tech_1].preq_tech_1, faction_id)) {
            return false; // doesn't have 1st prerequisite's 1st prerequisite
        }
        if (!has_tech_call(Technology[Technology[tech_id].preq_tech_1].preq_tech_2, faction_id)) {
            return false; // doesn't have 1st prerequisite's 2nd prerequisite
        }
    }
    if (Technology[tech_id].preq_tech_2 >= 0) {
        if (!has_tech_call(Technology[Technology[tech_id].preq_tech_2].preq_tech_1, faction_id)) {
            return false; // doesn't have 2nd prerequisite's 1st prerequisite
        }
        if (!has_tech_call(Technology[Technology[tech_id].preq_tech_2].preq_tech_2, faction_id)) {
            return false; // doesn't have 2nd prerequisite's 2nd prerequisite
        }
    }
    if (Technology[tech_id].preq_tech_1 <= TechDisabled) {
        return false; // disabled
    }
    if (Technology[tech_id].preq_tech_2 <= TechDisabled) {
        return false; // disabled
    }
    RulesReactor *reactor = Reactor;
    for (int i = 0; i < MaxReactorNum; i++, reactor++) {
        if (tech_id == reactor->preq_tech) {
            return false; // leap not possible for reactor tech
        }
    }
    RulesWeapon *weapon = Weapon;
    for (i = 0; i < MaxWeaponNum; i++, weapon++) {
        if (weapon->preq_tech == tech_id) { // may end early if 2 weapons have the same preq
            if (PlayersData[faction_id].ranking > 2) { // lowest two ranking factions
                return false;
            }
            // this line is an odd comparison (offensive rating <= best weapon id + 2)
            // however it might be to prevent leaps for later weapon tech
            if (weapon->offense_rating > weapon_budget(faction_id, 99, false) + 2) {
                return false;
            }
            return true;
        }
    }
    return true;
}

/*
Purpose: Craft an output string related to a specific technology. For techIDs outside the standard
         range, craft a string related to world map, comm links or prototypes.
// ORIGINAL: 0x005B9C40 ?say_tech@@YAXPADHH@Z 0x005B9C40-0x005B9EF0
// TRIED: call_diff shows this tree FEWER by 1 call (28 vs the image's 29) at every
//   flag set; the image's own `calls` list has no target for tech_category, so it
//   inlines `tech_category(tech_id)` at this one call site even though tech_category
//   keeps its own out-of-line, BYTE_EXACT body elsewhere (same "6 callers, 0 call
//   targets" shape as has_tech/tech_recurse). Measured two ways to reproduce that:
//   reordering tech_category's definition ahead of say_tech in this file (so it is
//   visible to /O2's same-TU inliner) is a no-op, still called under every flag set;
//   marking it `__forceinline` DOES fold it into say_tech under /O2, but the
//   measured-best flag set for byte similarity is /O2 /Oi- (not the /Ob0 set
//   call_diff's own gap-search picks), and there tech_category still doesn't inline -
//   `measure` regressed from 12/221 to 7/221 because the differently-shaped body
//   scores worse elsewhere. Reverted; tech_category is not `MEASURED inline`'able
//   from within this file without a net loss.
// UNEXPLORED: left untouched - MISMATCH at instruction 2, compiled body is 133 instructions
//   against the image's 221, a size gap much bigger than a register-allocation difference
//   accounts for. The `tech_id < -1` branch here is suspect: the image reads a label id out of
//   a runtime struct (`mov eax,[0x9b90f8]; mov eax,[eax+0x4d8]`, then an apparent __thiscall
//   through ecx=0x9b90d8 into 0x6169a0) rather than pushing a literal constant the way
//   `label_get(310)` compiles to here. Worth checking whether `label_get`'s argument in that
//   branch should be a field read (e.g. off a Rules-like global) instead of the literal 310.
// LEVER: the sprintf_s(&output[strlen(output)], 80, ...) calls replaced with the image's
//   strcat/_itoa idiom (a `char num_buf[80];` local, one strcat per literal/string piece, and
//   `_itoa(value, num_buf, 10); strcat(output, num_buf);` for the numeric field), and
//   tech_recurse given a `MEASURED inline` body (see its own marker) so its call site here
//   expands one level. call_diff now agrees on the call count (9 calls, matching the image);
//   moved 7/221 -> 12/221 agreeing. Remaining divergence starts at instruction 3 with register
//   allocation (esi vs eax + push order), not chased further this pass.
// size      688 bytes
// prototype void (__cdecl ?say_tech@@YAXPADHH@Z)(int8* output, int techID, int categoryLvl)
// callers   30   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F90 0x006169A0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_tech(LPSTR output, int tech_id, BOOL category_lvl) {
    char num_buf[80];
    if (tech_id < -1) {
        strcat_s(output, 80, label_get(310)); // "Not Available"
    } else if (tech_id < 0) {
        strcat_s(output, 80, label_get(25)); // "NONE"
    } else if (tech_id == 9999) {
        strcat_s(output, 80, label_get(306)); // "World Map"
    } else if (tech_id < MaxTechnologyNum) {
        strcat_s(output, 80, StringTable->get((int)Technology[tech_id].name));
        if (category_lvl) {
            strcat(output, " (");
            strcat(output, label_get(629 + tech_category(tech_id))); // 'E#', 'D#', 'B#', 'C#'
            _itoa(tech_recurse(tech_id, 0), num_buf, 10);
            strcat(output, num_buf);
            strcat(output, ")");
        }
    } else if (tech_id < 97) {
        if (Language) {
            strcat(output, label_get(487)); // 'Comm Frequency'
            strcat(output, " (");
            strcat(output, get_noun(tech_id - MaxTechnologyNum));
            strcat(output, ")");
        } else {
            strcat(output, Players[tech_id - MaxTechnologyNum].adj_name_faction);
            strcat(output, " ");
            strcat(output, label_get(487)); // 'Comm Frequency'
        }
    } else {
        strcat(output, VehPrototypes[tech_id - 97].veh_name);
        strcat(output, " ");
        strcat(output, label_get(185)); // 'Prototype'
    }
}

/*
Purpose: Get tech string for tech_id and store it into TechName buffer.
// ORIGINAL: 0x005B9EF0 ?tech_name@@YAPADHH@Z 0x005B9EF0-0x005B9F16 BYTE_EXACT
// size      38 bytes
// prototype int8* (__cdecl ?tech_name@@YAPADHH@Z)(int techID, BOOL categoryLvl)
// callers   9   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9C40
Return Value: Pointer to TechName
Status: Complete
*/
// BODY IN technology.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Check whether faction has a particular tech or not.
// ORIGINAL: 0x005B9F20 ?has_tech@@YA_NHH@Z 0x005B9F20-0x005B9F89
// LEVER: `RulesTechnology *tech = &Technology[tech_id];` then `tech->preq_tech_1`/`tech->preq_tech_2`
//   forces the compiler to keep BOTH tech_id and faction_id live in callee-saved registers
//   (matching the image's edi/esi) instead of re-deriving the field address per access;
//   two direct `Technology[tech_id].field` reads in one condition compile to a completely
//   different (and much longer) address recomputation. Moved 8/46 -> 32/46 agreeing.
// TRIED: `== TechDisabled` for the preq checks compiles to `cmp/je -2`; the image is
//   `cmp/jl -1` (i.e. `< TechNone`), which is what preq_tech_1/preq_tech_2 use now.
// symbol    ?has_tech@@YAHHH@Z
// size      105 bytes
// prototype bool (__cdecl ?has_tech@@YA_NHH@Z)(int techID, int factionID)
// callers   109   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Faction has tech? true/false
Status: Complete
*/
// BODY IN technology.h, as `MEASURED inline`: `__forceinline` here only
// satisfied inlining within this one translation unit - callers in OTHER
// units (facility_avail in base.cpp) got a real out-of-line call where the
// image open-codes the whole preq_tech walk at all 109 call sites.

// has_tech_call's DEFINITION LIVES IN leaf_recoveries.cpp, not here, and the
// distance is the whole point. `/O2` implies `/Ob2`, which inlines any
// suitable function and not merely `inline`-marked ones - so a forwarder
// defined in THIS file gets folded straight back into every caller in this
// file, and valid_tech_leap's four sites emitted has_tech's preq walk in place
// where the image emits `call 0x5b9f20`. That is why best_specialist
// (base.cpp) worked with the same idiom: it is CROSS-TU, and VC6 cannot inline
// across translation units. Moving the definition out is what makes the
// forwarder actually forward.

/*
Purpose: Determine technology level for tech_id.
// ORIGINAL: 0x005B9F90 ?tech_recurse@@YAHHH@Z 0x005B9F90-0x005B9FE0
// LEVER: hoist-next-lvl `const int next_lvl = base_lvl + 1;` ahead of the tech
//   pointer, passed to both recursive calls instead of writing `base_lvl + 1`
//   twice. The image reads base_lvl at 0x005B9F9F, before the index arithmetic
//   finishes, and builds edi once; writing the `+ 1` at each call site defers
//   that read past the lea chain. 22/36 -> 25/36, and the instruction count
//   now matches the image exactly at 36.
// TRIED: scaled-index-in-register the last divergence is addressing form,
//   not control flow: the image keeps `esi = 44*tech_id` (`shl esi, 2`) and
//   folds the array base into BOTH displacements - `[esi+0x94F37C]`,
//   `[esi+0x94F380]` - where this tree materialises the whole pointer,
//   `lea edi, [eax*4+0x94F358]` then `[edi+0x28]`. `osmx semantic` refuses on
//   exactly that, "instruction 13: shl against lea". Two subscripts
//   (`Technology[tech_id].preq_tech_N`) do NOT recover it: VC6 then keeps
//   tech_id itself in a register and rebuilds the whole lea chain after the
//   first call, 4/36 with the hoist and 4/36 without it. The array base is a
//   `RulesTechnology *const` in technology.h, so the fold is already an
//   immediate; nothing in the body's spelling chooses which half stays live.
// LEVER: same as has_tech - `RulesTechnology *tech = &Technology[tech_id];` before the two
//   recursive calls, instead of two `Technology[tech_id].preq_tech_N` subscripts. 4/36 -> 22/36.
// LEVER: `MEASURED inline` in technology.h - the image expands a recursive inline exactly one
//   level and leaves the inner recursive calls as real calls to the out-of-line body, which is
//   why say_tech/tech_val/tech_ai each show a paired tech_recurse call at their tech_recurse
//   call sites instead of one. Moving the body out of this .cpp reproduces that at every caller
//   without costing tech_recurse its own out-of-line copy (the image still has one, at this
//   address).
// size      80 bytes
// prototype int (__cdecl ?tech_recurse@@YAHHH@Z)(int techID, int ret)
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F90
Return Value: Level
Status: Complete
*/
// BODY IN technology.h, as `MEASURED inline`: see the LEVER note above.

/*
Purpose: Determine what category is dominate for tech_id. If there is a tie, the order of precedence
         is as follows: growth > tech > wealth > power.
// ORIGINAL: 0x005B9FE0 ?tech_category@@YAHH@Z 0x005B9FE0-0x005BA02C BYTE_EXACT
// size      76 bytes
// prototype int (__cdecl ?tech_category@@YAHH@Z)(int techID)
// callers   6   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Tech category id: growth (0), tech (1), wealth (2) or power (3).
Status: Complete
*/
int __cdecl tech_category(int tech_id) {
    uint32_t category = 0;
    int compare = Technology[tech_id].growth_value;
    if (Technology[tech_id].tech_value > compare) {
        category = 1;
        compare = Technology[tech_id].tech_value;
    }
    if (Technology[tech_id].wealth_value > compare) {
        category = 2;
        compare = Technology[tech_id].wealth_value;
    }
    return (Technology[tech_id].power_value > compare) ? 3 : category;
}

/*
Purpose: Check to see whether provided faction can research a specific technology. Checks are
         included to prevent SMACX specific Veh from being built in SMAC mode.
// ORIGINAL: 0x005BAC20 ?tech_avail@@YAHHH@Z 0x005BAC20-0x005BADBF
// LEVER: has_tech marked `__forceinline` (it is defined earlier in the file, so every caller
//   below it CAN inline it; the image inlines it here and in tech_effects/tech_val, but calls it
//   for real in valid_tech_leap, which is defined ABOVE has_tech and so cannot see its body -
//   matches the image's own "calls (none)" vs "calls ... 0x005B9F20" split exactly).
//   Plus the same `RulesTechnology *tech = &Technology[tech_id];` pointer lever as has_tech,
//   applied to this function's own preq_tech_1/preq_tech_2 reads. 6/130 -> 78/130 agreeing.
// size      415 bytes
// prototype int (__cdecl ?tech_avail@@YAHHH@Z)(int techID, int factionID)
// callers   6   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Is tech available to faction? true/false
Status: Complete
*/
BOOL __cdecl tech_avail(int tech_id, int faction_id) {
    if (has_tech(tech_id, faction_id) || tech_id >= MaxTechnologyNum || (!ExpansionEnabled
        && (tech_id == TECH_PRPSYCH || tech_id == TECH_FLDMOD || tech_id == TECH_ADAPDOC
            || tech_id == TECH_ADAPECO || tech_id == TECH_BIOADAP || tech_id == TECH_SENTRES
            || tech_id == TECH_SECMANI || tech_id == TECH_NEWMISS || tech_id == TECH_BFG9000))) {
        return false;
    }
    RulesTechnology *tech = &Technology[tech_id];
    int preq_tech_1 = tech->preq_tech_1;
    int preq_tech_2 = tech->preq_tech_2;
    if (preq_tech_1 < TechNone || preq_tech_2 < TechNone) {
        return false; // if either prerequisite tech is set to disabled (-2)
    }
    return (has_tech(preq_tech_1, faction_id) && has_tech(preq_tech_2, faction_id));
}

/*
Purpose: Calculate faction's tech related bonuses for commerce and resource production in fungus.
// ORIGINAL: 0x005BAE60 ?tech_effects@@YAXH@Z 0x005BAE60-0x005BAFFB
// LEVER: `memset(&PlayersData[faction_id].tech_fungus_nutrient, 0, 4 * sizeof(int))` for the
//   four contiguous `tech_fungus_*` zero-inits reproduces the image's single-base-pointer
//   4x `mov [reg+N], 0` (0/4/8/0xc); four separate `= 0;` statements each re-derive the field
//   address instead of sharing one. has_tech forceinline (see tech_avail) also applies here,
//   since this function's own tech loop calls has_tech; net effect on THIS function's raw
//   agreeing count was roughly flat (register allocation inside the loop still differs from
//   the image - same open plateau as has_tech/tech_recurse) but it is the semantically correct
//   shape (the image inlines has_tech here too - "calls (none)").
// size      411 bytes
// prototype void (__cdecl ?tech_effects@@YAXH@Z)(int factionID)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: faction_bonus loop - hoisted `int bonus_count = Players[faction_id].faction_bonus_count;`
//   out of the loop condition and read `faction_bonus_val1[i]` once at the top instead of in each
//   of the three arms, matching the image's single stepped pointer. Clamp loop - a counted
//   `for (int n = 0; n < 4; n++)` over `(&tech_fungus_nutrient)[n]` instead of the pointer-bounded
//   `<=` walk, matching the image's `mov esi,4 ... dec esi; jne` branchless clamp. Under the
//   default-picked flag set (no /Oy-) the raw agreeing count is unchanged at 3/133 - that flag set
//   never keeps the frame pointer regardless of loop shape, so instruction 0 always diverges there.
//   Under /Oy- (which does keep it) this moved 12-14/133 -> 17/133; compiled instruction count also
//   moved closer to the image's 133 (149 -> 122). Left in; not chased further.
Return Value: n/a
Status: Complete
*/
void __cdecl tech_effects(int faction_id) {
    PlayersData[faction_id].tech_commerce_bonus = Players[faction_id].rule_commerce;
    memset(&PlayersData[faction_id].tech_fungus_nutrient, 0, 4 * sizeof(int));
    for (uint32_t tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
        if (has_tech(tech_id, faction_id)) {
            uint32_t flags = Technology[tech_id].flags;
            if (flags & TFLAG_INC_ENERGY_FUNGUS) {
                PlayersData[faction_id].tech_fungus_energy++;
            }
            if (flags & TFLAG_INC_MINERALS_FUNGUS) {
                PlayersData[faction_id].tech_fungus_mineral++;
            }
            if (flags & TFLAG_INC_NUTRIENT_FUNGUS) {
                PlayersData[faction_id].tech_fungus_nutrient++;
            }
            if (flags & TFLAG_INC_COMMERCE) {
                PlayersData[faction_id].tech_commerce_bonus++;
            }
        }
    }
    int bonus_count = Players[faction_id].faction_bonus_count;
    for (int i = 0; i < bonus_count; i++) {
        int bonus_val = Players[faction_id].faction_bonus_val1[i];
        if (Players[faction_id].faction_bonus_id[i] == RULE_FUNGNUTRIENT) {
            PlayersData[faction_id].tech_fungus_nutrient += bonus_val;
        } else if (Players[faction_id].faction_bonus_id[i] == RULE_FUNGMINERALS) {
            PlayersData[faction_id].tech_fungus_mineral += bonus_val;
        } else if (Players[faction_id].faction_bonus_id[i] == RULE_FUNGENERGY) {
            PlayersData[faction_id].tech_fungus_energy += bonus_val;
        }
    }
    // if values are below zero, cap at zero
    int *tech_fungus = &PlayersData[faction_id].tech_fungus_nutrient;
    for (int n = 0; n < 4; n++) {
        if (tech_fungus[n] < 0) {
            tech_fungus[n] = 0;
        }
    }
    if (PlayersData[faction_id].soc_effect_pending.economy > 2) {
        PlayersData[faction_id].tech_commerce_bonus++;
        if (PlayersData[faction_id].soc_effect_pending.economy > 3) {
            PlayersData[faction_id].tech_commerce_bonus++;
            if (PlayersData[faction_id].soc_effect_pending.economy > 4) {
                PlayersData[faction_id].tech_commerce_bonus++;
            }
        }
    }
}

/*
Purpose: Determine if preqTechID is a prerequisite of parentTechID within descending range.
// ORIGINAL: 0x005BCB60 ?tech_is_preq@@YAHHHH@Z 0x005BCB60-0x005BCBDB
// TRIED: the `RulesTechnology *tech = &Technology[parent_tech_id];` pointer lever that
//   helped has_tech/tech_recurse/tech_avail REGRESSES this one (9/63 -> 5/63): it merges the
//   two early `return false;` guards (preq_tech_id<0, parent_tech_id<0) into one shared
//   epilogue the image does not use. Left as plain `Technology[parent_tech_id].preq_tech_N`.
//   Still MISMATCH at 9/63; unexplored beyond that.
// LEVER: `MEASURED inline` in technology.h - the image expands tech_is_preq one recursion
//   level at each call site (20 tight pairs of calls to this address in tech_val alone); moving
//   the body out of this .cpp reproduces that without costing tech_is_preq its own out-of-line
//   copy, which the image still emits here.
// size      123 bytes
// prototype int (__cdecl ?tech_is_preq@@YAHHHH@Z)(int preqTechID, int parentTechID, unsigned int range)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BCB60
Return Value: Is preqTechID prerequisite of parentTechID? true/false
Status: Complete
*/
// BODY IN technology.h, as `MEASURED inline`: see the LEVER note above.

/*
Purpose: Determine how valuable the specified techID is to a faction. This id either corresponds to
         a technology (0-88), another faction (89-96) or a prototype (97-608). The 3rd parameter
         determines whether a simplistic or extended calculation is required for a technology id.
// ORIGINAL: 0x005BCBE0 ?tech_val@@YAHHHH@Z 0x005BCBE0-0x005BDC05
// LEVER: has_tech `__forceinline` (see tech_avail) applies here too - the image's own "calls"
//   list has no 0x005B9F20, so it inlines has_tech at every one of this function's several
//   call sites. Moved 21/1361 -> 26/1361 agreeing; still deep MISMATCH, unexplored past that -
//   4133 bytes is far past what a source-form search covers in one pass.
// TRIED: applying the `RulesTechnology *tech_ptr = &Technology[tech_id];` pointer lever to
//   the growth/power/wealth/tech reads (the same shape that helped has_tech/tech_avail)
//   REGRESSES this function (26/1361 -> 16/1361); left as four separate `Technology[tech_id].*`
//   reads.
// LEVER: tech_is_preq and tech_recurse moved to `MEASURED inline` in technology.h (their own
//   markers carry the detail) - the image expands each one recursion level at every call site
//   (20 tech_is_preq pairs, 3 tech_recurse pairs), which this source's plain calls did not
//   reproduce. call_diff now agrees on all 40 tech_is_preq calls; bit_count/climactic_battle/
//   wants_to_attack/tech_recurse still short by 3 calls total, not chased further this pass -
//   the divergence starts at instruction 0 (image keeps ebp frame, the picked flag set does not)
//   and cascades. Moved 26/1361 -> 29/1361 agreeing.
// size      4133 bytes
// prototype int (__cdecl ?tech_val@@YAHHHH@Z)(int techID, int factionID, BOOL simpleCalc)
// callers   14   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA30 0x00539E40 0x0055BC80 0x005B9F90 0x005BCB60
Return Value: Value of techID to the specified faction
Status: Complete
*/
int __cdecl tech_val(int tech_id, int faction_id, BOOL simple_calc) {
    if (tech_id == 9999) {
        return 2;
    }
    int value_ret;
    if (tech_id < MaxTechnologyNum) {
        uint32_t vendetta_count = 0;
        for (int i = 1; i < MaxPlayerNum; i++) {
            if (i != faction_id && has_treaty(faction_id, i, DTREATY_VENDETTA)) {
                vendetta_count++;
            }
        }
        uint32_t factor_ai = 1;
        if (!simple_calc) {
            factor_ai = (GameRules & RULES_BLIND_RESEARCH) ? 4 : 2;
        }
        BOOL ai_growth = PlayersData[faction_id].ai_growth;
        BOOL ai_power = PlayersData[faction_id].ai_power;
        BOOL ai_wealth = PlayersData[faction_id].ai_wealth;
        BOOL ai_tech = PlayersData[faction_id].ai_tech;
        int growth = Technology[tech_id].growth_value;
        int power = Technology[tech_id].power_value;
        int wealth = Technology[tech_id].wealth_value;
        int tech = Technology[tech_id].tech_value;
        value_ret = growth * (factor_ai * ai_growth + 1) + wealth * (factor_ai * ai_wealth + 1)
            + tech * (factor_ai * ai_tech + 1) + power * (factor_ai * ai_power + 1);
        uint32_t base_count = PlayersData[faction_id].current_num_bases;
        if ((!power || !ai_power && !vendetta_count) && (!tech || !ai_tech)
            && (!wealth || !ai_wealth) && (!growth || !ai_growth && base_count >= 4)) {
            value_ret = (value_ret + 1) / 2;
        }
        BOOL is_human_player = is_human(faction_id);
        if (!is_human_player && !has_tech(tech_id, faction_id) && simple_calc) {
            // LEVER: the image's two adjacent bit_count() calls (0x005BCEC7,
            // 0x005BCEDD, 22 bytes apart) are this test and this expression
            // each evaluating bit_count() separately, not a cached local.
            if (bit_count(GameTechAchieved[tech_id]) > 1) {
                value_ret += 2 - 2 * bit_count(GameTechAchieved[tech_id]); // increase priority if more factions have tech
            }
            int search_lvl = 1;
            for (int i = 0; i < MaxTechnologyNum; i++) {
                if (has_tech(i, faction_id)) {
                    int current_lvl = tech_recurse(i, 0);
                    if (search_lvl < current_lvl) {
                        search_lvl = current_lvl;
                    }
                }
            }
            int tech_id_lvl = tech_recurse(tech_id, 0);
            if (tech_id_lvl < search_lvl) {
                value_ret = value_ret * (tech_id_lvl + 1) / (search_lvl + 1);
            }
            if (value_ret < 1) {
                value_ret = 1;
            }
        }
        if (simple_calc) {
            return value_ret;
        }
        if (base_count) {
            for (int region = 1; region < MaxRegionLandNum; region++) {
                if (!bad_reg(region)) {
                    uint32_t pwr_base = PlayersData[faction_id].region_total_bases[region] * power;
                    uint8_t plan = PlayersData[faction_id].region_base_plan[region];
                    if (plan == PLAN_NAVAL_TRANSPORT && vendetta_count && !is_human_player) {
                        value_ret += (pwr_base / base_count);
                    } else if (plan == PLAN_DEFENSIVE) {
                        value_ret += (pwr_base * 4) / (base_count * (is_human_player + 1));
                    } else if (plan == PLAN_OFFENSIVE) {
                        value_ret += (pwr_base * ((PlayersData[faction_id].best_weapon_value
                            >= PlayersData[faction_id].enemy_best_weapon_value) ? 2 : 4))
                            / (base_count * (is_human_player + 1));
                    } else {
                        for (int i = 1; i < MaxPlayerNum; i++) {
                            if (i != faction_id && PlayersData[i].region_total_bases[region]
                                && PlayersData[faction_id].region_total_bases[region]
                                && has_treaty(faction_id, i, DTREATY_COMMLINK)
                                && (!has_treaty(faction_id, i, DTREATY_PACT | DTREATY_TREATY)
                                    || has_treaty(faction_id, i, DTREATY_WANT_REVENGE))) {
                                value_ret += (pwr_base / (base_count * (is_human_player + 1)));
                            }
                        }
                    }
                }
            }
        }
        if (has_tech(tech_id, faction_id)) {
            return value_ret; // should this be moved further up?
        }
        if (climactic_battle()
            && tech_is_preq(tech_id, Facility[FAC_ASCENT_TO_TRANSCENDENCE].preq_tech, 2)) {
            value_ret *= 4;
        }
        if (PlayersData[faction_id].soc_effect_base.planet > 0 && ai_growth) {
            if (tech_is_preq(tech_id, TECH_CENTMED, 9999)) {
                value_ret *= 3;
            }
            if (tech_is_preq(tech_id, TECH_PLAECON, 9999)) {
                value_ret *= 2;
            }
            if (tech_is_preq(tech_id, TECH_ALPHCEN, 3)) {
                value_ret *= 2;
            }
        }
        if (PlayersData[faction_id].soc_effect_base.probe <= 0) {
            if (tech_is_preq(tech_id, Facility[FAC_HUNTER_SEEKER_ALGO].preq_tech, ai_tech + 2)) {
                if (!ai_power) {
                    value_ret *= 2;
                }
                if (ai_tech) {
                    value_ret *= 2;
                }
            }
        }
        if (ai_growth && tech_is_preq(tech_id, TECH_DOCINIT, 2)) {
            value_ret *= 2;
        }
        if ((ai_wealth || !MapCloudCover) && tech_is_preq(tech_id, TECH_ENVECON, 9999)) {
            value_ret *= 2;
        }
        if (Technology[tech_id].flags & TFLAG_SECRETS && !GameTechAchieved[tech_id]
            && !(GameRules & RULES_BLIND_RESEARCH)) {
            value_ret *= (ai_power + 1) * 2;
        }
        if (Players[faction_id].rule_psi > 0) {
            if (tech_is_preq(tech_id, Facility[FAC_DREAM_TWISTER].preq_tech, 9999)) {
                value_ret *= 2;
            }
        } else {
            int preq_tech_fusion = Reactor[RECT_FUSION - 1].preq_tech;
            if ((int)tech_id == preq_tech_fusion) {
                value_ret *= 2;
            }
            if ((int)tech_id == Reactor[RECT_QUANTUM - 1].preq_tech) {
                value_ret *= 2;
            }
            if (tech_is_preq(tech_id, preq_tech_fusion, 9999)) {
                value_ret++;
            }
            if (tech_is_preq(tech_id, preq_tech_fusion, 1)
                && !(GameRules & RULES_BLIND_RESEARCH)) {
                value_ret *= 2;
            }
        }
        int eco_dmg_unk = PlayersData[faction_id].unk_49 / range(base_count, 1, 9999);
        if (eco_dmg_unk > ai_power 
            && (tech_is_preq(tech_id, Facility[FAC_HYBRID_FOREST].preq_tech, 9999)
            || tech_is_preq(tech_id, Facility[FAC_TREE_FARM].preq_tech, 9999)
            || tech_is_preq(tech_id, Facility[FAC_CENTAURI_PRESERVE].preq_tech, 9999)
            || tech_is_preq(tech_id, Facility[FAC_TEMPLE_OF_PLANET].preq_tech, 9999))) {
            value_ret += eco_dmg_unk;
        }
        if (Players[faction_id].rule_population > 0) {
            if (tech_is_preq(tech_id, Facility[FAC_HAB_COMPLEX].preq_tech, 9999)) {
                value_ret *= 2;
            } else if (tech_is_preq(tech_id, Facility[FAC_HABITATION_DOME].preq_tech, 9999)
                && TurnCurrentNum > 250) {
                value_ret = (value_ret * 3) / 2;
            }
        }
        if (ai_power) {
            for (int i = 0; i < MaxWeaponNum; i++) {
                if (Weapon[i].offense_rating) {
                    int weap_preq_tech = Weapon[i].preq_tech;
                    if ((int)tech_id == weap_preq_tech) {
                        value_ret *= (is_human_player + 3);
                    } else if (tech_is_preq(tech_id, weap_preq_tech, 1)) {
                        value_ret *= (is_human_player + 2);
                    }
                }
            }
        }
        if (ai_tech || !ai_power) {
            for (int i = 0; i < MaxTechnologyNum; i++) {
                if (!has_tech(i, faction_id) && Technology[i].flags & TFLAG_SECRETS
                    && !GameTechAchieved[i] && tech_is_preq(tech_id, i, 1)) {
                    value_ret *= 3;
                }
            }
        }
        if (tech_is_preq(tech_id, VehPrototypes[BSC_FORMERS].preq_tech, 9999)
            && !has_tech(VehPrototypes[BSC_FORMERS].preq_tech, faction_id)) {
            value_ret *= 2;
            if (is_human_player) {
                value_ret *= 2;
            }
        }
        if (tech_is_preq(tech_id, Chassis[CHSI_FOIL].preq_tech, 9999)
            && !has_tech(Chassis[CHSI_FOIL].preq_tech, faction_id)) {
            BOOL search = false;
            for (int region = 1; region < MaxRegionLandNum; region++) {
                if (PlayersData[faction_id].region_total_bases[region]) {
                    for (int i = 1; i < MaxPlayerNum; i++) {
                        if (faction_id != i && !PlayersData[i].region_total_bases[region]) {
                            search = true;
                            break;
                        }
                    }
                    if (search && PlayersData[faction_id].unk_79[region] 
                        >= Continents[region].tile_count) {
                        value_ret *= 3;
                        if (is_human_player) {
                            value_ret *= 2;
                        }
                        break;
                    }
                }
            }
            if (search) {
                value_ret = (value_ret * 2) + 4;
            }
        }
    } else if (tech_id < 97) { // factions
        uint32_t factor = 1;
        uint32_t faction_id_2 = tech_id - MaxTechnologyNum;
        if (!wants_to_attack(faction_id, faction_id_2, 0)) {
            factor = 2;
        }
        if (!wants_to_attack(faction_id_2, faction_id, 0)) {
            factor++;
        }
        value_ret = factor * (factor / (PlayersData[faction_id].ai_fight + 2));
    } else {  // prototypes
        uint32_t proto_id = tech_id - 97;
        value_ret = range(get_proto_offense_rating(proto_id), 1, 2)
            + range(get_proto_defense_rating(proto_id), 1, 2)
            + range(Chassis[VehPrototypes[proto_id].chassis_id].speed, 1, 2)
            + VehPrototypes[proto_id].reactor_id - 2;
    }
    return value_ret;
}

/*
Purpose: Determine a tech the specified faction should research.
// ORIGINAL: 0x005BDC10 ?tech_ai@@YAHH@Z 0x005BDC10-0x005BDD64
// LEVER: tech_recurse moved to `MEASURED inline` in technology.h (see its own marker) - the
//   image expands `tech_recurse(i, 0)` one level inside this function's main loop, which is
//   what turns the loop's integer index into the image's pointer stepping by 0x2C. Moved
//   4/124 -> 10/124 agreeing; the induction-variable strength reduction itself did not fall out
//   automatically (i is also used for tech_avail/tech_val/the Formers compare/the return value),
//   not chased further this pass.
// size      340 bytes
// prototype int (__cdecl ?tech_ai@@YAHH@Z)(int factionID)
// callers   3   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F90 0x005BAC20 0x005BCBE0 0x00625810 0x0064601D
Return Value: techID or -1
Status: Complete
*/
int __cdecl tech_ai(int faction_id) {
    int tech_id = -1;
    int search = -999;
    BOOL is_human_player = is_human(faction_id);
    for (int i = 0; i < MaxTechnologyNum; i++) {
        if (tech_avail(i, faction_id)) {
            int tech_value = tech_val(i, faction_id, false);
            int compare;
            if (GameRules & RULES_BLIND_RESEARCH) {
                if (is_human_player && (PlayersData[faction_id].ai_growth
                    || PlayersData[faction_id].ai_wealth)
                    && i == VehPrototypes[BSC_FORMERS].preq_tech) {
                    return i; // Direct human player research toward gaining Formers
                }
                int preq = tech_recurse(i, 0);
                tech_value = preq ? (tech_value << 8) / preq : 0;
            }
            compare = is_human_player ? random(0, tech_value + 1) // unsure why humans get random()
                : rnd(tech_value + 1, NULL); // vs AI rnd() ?
            if (compare > search) {
                search = compare;
                tech_id = i;
            }
        }
    }
    return tech_id;
}

/*
Purpose: Get power_value from technology struct for tech id.
// ORIGINAL: 0x005BDD70 ?tech_mil@@YAHH@Z 0x005BDD70-0x005BDD8E BYTE_EXACT
// size      30 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: power_value or 0 if tech_id is greater than max
Status: Complete
*/
// BODY IN technology.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
// LEVER: branch polarity - guard clause 'if (tech_id < MaxTechnologyNum) return
//        Technology[tech_id].power_value; return 0;' instead of the
//        '(tech_id >= MaxTechnologyNum) ? 0 : ...' ternary. The ternary put the
//        constant-return block first; the guard clause falls through to the
//        work and tails the constant return, matching the image's jge-past-work
//        layout.


/*
Purpose: Get tech_value from technology struct for tech id.
// ORIGINAL: 0x005BDD90 ?tech_tech@@YAHH@Z 0x005BDD90-0x005BDDB1 BYTE_EXACT
// size      33 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: tech_value or 1 if tech_id is greater than max
Status: Complete
*/
// BODY IN technology.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
// LEVER: branch polarity - same guard-clause rewrite as tech_mil.


/*
Purpose: Get wealth_value from technology struct for tech id.
// ORIGINAL: 0x005BDDC0 ?tech_infra@@YAHH@Z 0x005BDDC0-0x005BDDE1 BYTE_EXACT
// LEVER: same guard-clause rewrite as tech_tech/tech_colonize - the ternary
// spelling (`(tech_id >= Max) ? 1 : *(&Technology[tech_id].wealth_value)`)
// was a different program, not just a different shape.
// size      33 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: wealth_value or 1 if tech_id is greater than max
Status: Complete
*/
// BODY IN technology.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Get growth_value from technology struct for tech id.
// ORIGINAL: 0x005BDDF0 ?tech_colonize@@YAHH@Z 0x005BDDF0-0x005BDE11 BYTE_EXACT
// size      33 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: growth_value or 1 if tech_id is greater than max
Status: Complete
*/
// BODY IN technology.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
// LEVER: branch polarity - same guard-clause rewrite as tech_mil.


/*
Purpose: Calculate how much researching a tech will cost the specified faction.
// ORIGINAL: 0x005BE6B0 ?tech_rate@@YAHH@Z 0x005BE6B0-0x005BE931
// LEVER: unsigned-shift-signed-divide `rule_factor` is UNSIGNED - the image
//   shifts it `shr edi, 3` and `shr esi, 5` at 0x005BE7EC/0x005BE7F1 - while the
//   division it feeds stays signed, `cdq; idiv edi`. So the source is
//   `uint32_t rule_factor` with `(int)` casts at the two use sites, not an `int`
//   throughout: as `int` VC6 emits `sar` for both shifts.
// LEVER: signed-divide-through-a-uint32_t-operand three expressions divided or
//   multiplied through a `uint32_t` global and came out unsigned where the image
//   is signed - `100 * discovery_rate / Rules->tech_discovery_rate_pct_std`
//   (`div` against the image's `cdq; idiv`) and `discovery_rate * MapAreaSqRoot
//   / 56` (`mul`/`shr` against `imul`/`sar`). Casting each divisor and factor to
//   `int` at the use site fixes both without changing the shared declarations.
// LEVER: top-factor-compares-ge the max loop is `if (compare >= top_factor)`,
//   not `>`: the image's `cmp ebx, esi; jl` skips the assignment only when
//   compare is STRICTLY below, so the equal case still stores. Same value,
//   opposite condition byte.
// TRIED: this pass did NOT reach byte-exactness. The three levers above took
//   best similarity 0.384 -> 0.721 at `/c /O2 /Gy /GR- /Oy- /GX` and the compiled
//   body from 230 to 232 instructions against the image's 234, with 71 of 234
//   agreeing; the differing runs fell from 20 to 15. What is left, measured
//   2026-08-22:
//   * `is_human` reads `FactionsStatus[0]` as a `uint8_t`, which compiles
//     `xor ecx, ecx; mov cl, byte ptr [0x9A64E8]`, where the image loads the
//     WORD and masks - `mov ecx, [0x9A64E8]; and ecx, 0xFF`. Writing that mask
//     out locally in this body does NOT reproduce it either: VC6 reassociates
//     `(1 << faction_id) & (word & 0xFF)` into `and eax, edx; and eax, 0xFF`,
//     and it costs five agreeing instructions (71 -> 66) while flattering the
//     similarity to 0.773. The fix belongs in `is_human` itself, in faction.h,
//     and that declaration is shared with every other caller.
//   * this tree's frame is `sub esp, 0x14` against the image's `sub esp, 0x10`.
//     The image REUSES [ebp-4] - the `range(..., 2, 9999)` result for
//     player_factor at 0x005BE717, then `diff_lvl = 3` at 0x005BE794 once the
//     first is dead - where this tree gives the two separate slots and every
//     later displacement is off by one slot.
//   * `range(research, -1, 1)` collapses in the image to the SIGN of research -
//     `test; jle; mov eax, 1` / `xor eax, eax; test; setge al; dec eax` - and it
//     is computed inline at its single use, immediately before `sub ebx, eax`.
//     Spelling that ternary out reproduces the sign idiom but VC6 then hoists it
//     to the top of the expression and spills it, which is WORSE overall:
//     0.721 -> 0.459 similarity for one extra agreeing instruction. Measured
//     and reverted.
// LEVER: signedness - player_factor, top_factor, compare, diff_factor, diff_lvl,
//   tech_stagnation, rule_factor, fin_factor, discovery_rate and cost changed from uint32_t to
//   int so the image's signed sar/cdq/idiv/jl/setl forms come out instead of shr/div/jbe. Under
//   /c /O2 /Gy /GR- /Oy- /GX (the flag set that keeps the frame pointer) this moved the raw
//   agreeing count from ~17/234 to 71/234; osmx's own similarity-picked flag set still lands on
//   a small mismatch (it never picks /Oy- for this function), so `measure`'s default output does
//   not show the size of this win. tools/signedness.py now reports 0 remaining pure-signedness
//   claims on this body. Still deep MISMATCH beyond that; not chased further.
// size      641 bytes
// prototype int (__cdecl ?tech_rate@@YAHH@Z)(int factionID)
// callers   6   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: tech rate/cost
Status: Complete
*/
int __cdecl tech_rate(int faction_id) {
    if (PlayersData[faction_id].tech_cost >= 0) {
        return PlayersData[faction_id].tech_cost; // already set
    }
    if (!Rules->tech_discovery_rate_pct_std) {
        return 999999999; // max cost
    }
    int player_factor = range(PlayersData[faction_id].earned_techs_saved * 2
        - PlayersData[faction_id].unk_26 + PlayersData[faction_id].tech_ranking, 2, 9999);
    int top_factor = 0;
    for (uint32_t i = 1; i < MaxPlayerNum; i++) {
        int compare = PlayersData[i].earned_techs_saved * 2 + PlayersData[i].tech_ranking;
        if (compare >= top_factor) {
            top_factor = compare;
        }
    }
    player_factor /= 2;
    top_factor /= 2;
    BOOL is_human_player = is_human(faction_id);
    int diff_factor = is_human_player ? PlayersData[faction_id].diff_level : DiffLevelCurrent;
    diff_factor += (diff_factor < DLVL_LIBRARIAN);
    int diff_lvl = !is_human_player ? DiffLevelCurrent : DLVL_LIBRARIAN;
    diff_factor = is_human_player ? diff_factor * 4 + 8 : 29 - diff_factor * 3;
    diff_factor = range(diff_factor, 12 - player_factor, player_factor + 12);
    int tech_stagnation = GameRules & RULES_TECH_STAGNATION;
    uint32_t rule_factor = tech_stagnation | 0x40; // 64 or 96
    int fin_factor = range(
        player_factor - (TurnCurrentNum / static_cast<int>(rule_factor >> 3)),
        0, (diff_factor * static_cast<int>(rule_factor >> 5)) >> 1) + diff_factor;
    int resch_base = range(PlayersData[faction_id].soc_effect_base.research, -1, 1);
    int discovery_rate = (fin_factor
        - range((top_factor - diff_lvl - player_factor + 7) / (8 - diff_lvl),
            0, diff_lvl * fin_factor / 10 + 1))
        * range(player_factor - resch_base, 1, 99999);
    if (Rules->tech_discovery_rate_pct_std != 100) {
        discovery_rate = 100 * discovery_rate
            / static_cast<int>(Rules->tech_discovery_rate_pct_std);
    }
    if (Players[faction_id].rule_techcost != 100) {
        discovery_rate = discovery_rate * Players[faction_id].rule_techcost / 100;
    }
    int cost = (discovery_rate * static_cast<int>(MapAreaSqRoot)) / 56;
    if (tech_stagnation) {
        cost += cost / 2; // Slower Rate of Research Discoveries
    }
    return range(cost, 1, 99999999);
}
