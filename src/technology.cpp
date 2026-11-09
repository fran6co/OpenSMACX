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
#include "time.h"
#include "datalink.h"
#include "alpha.h"
#include "base.h"
#include "game.h"
#include "general.h"
#include "crt_random_inline.h"
#include "map.h"
#include "random.h"
#include "strings.h"
#include "veh.h"
#include "faction.h"
#include "console.h"
#include "terraforming.h"
#include "netdaemon.h"
#include "netmsg.h"
#include "xpops.h"
#include "worldwin.h"
#include "popup.h"
#include "newtechwin.h"
#include "designwin.h"
#include "fx.h"
#include "interlude.h"
#include "picktech.h"
#include "spritebox.h"
#include "log.h"
#include "buffer.h"
#include "graphicwin.h"
#include "sprite.h"
#include "mapwin.h"
#include "stringstruct.h"
#include "wave.h"

extern Sprite g_TECH_ICON_SPRITES[MaxTechnologyNum];
void __cdecl design_new_veh(int faction_id, int proto_id);
void __cdecl help_tech_info(Popup *popup, int tech_id, int faction_id);
void __cdecl social_select(int faction_id);
void __cdecl set_dirty();
extern char *TutorialPopupCaption;
extern int FirstProposedPrototype;
// Original pointer cell 0x00691B18 (stringstruct.cpp's definition) selects the
// WIZARD popup text used by tech_wizard_unk1.
extern char *TechWizardPopupFile;

// 0x00945E80, read off the image's own tech_name body: `push 0x945e80` /
// `mov byte ptr [0x945e80], 0` / `mov eax, 0x945e80` at
// 0x005B9EFB/0x005B9F00/0x005B9F0F - the buffer tech_name zeroes, passes to
// say_tech and returns. v1, not v2: the writes through say_tech are bounded
// only by this declaration, so no initial-data extent is claimed.
// RECOVERY_OBJECT: v1 0x00945E80 ?TechName@@3PADA
char TechName[80];

// The general voice-over wave lives in this unit: its .bss cell 0x00945ED0
// follows TechName (0x00945E80 + 0x50) and precedes TechAchievedGate
// (0x00945F40, the next dword after its 0x6C bytes), and its CRT initializer pair
// opens the unit's code at 0x005B9C10, ahead of say_tech.
// ORIGINAL: 0x005B9C10 _$E66 0x005B9C10-0x005B9C26 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x0068265C selects init 0x005B9C10, which registers exit callback 0x005B9C30.", "Original init span 0x005B9C10-0x005B9C26 and exit span 0x005B9C30-0x005B9C3A are independently disassembled; both identify receiver 0x00945ED0 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E66 via .CRT$XCU, relocating to g_WAVE_GENERAL, Wave construction, paired local fini _$E64 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_WAVE_GENERAL. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E17
// ORIGINAL: 0x005B9C30 _$E64 0x005B9C30-0x005B9C3A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x0068265C selects init 0x005B9C10, which registers exit callback 0x005B9C30.", "Original init span 0x005B9C10-0x005B9C26 and exit span 0x005B9C30-0x005B9C3A are independently disassembled; both identify receiver 0x00945ED0 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E66 via .CRT$XCU, relocating to g_WAVE_GENERAL, Wave construction, paired local fini _$E64 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_WAVE_GENERAL. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E15
// RECOVERY_OBJECT: v1 0x00945ED0 ?g_WAVE_GENERAL@@3VWave@@A
Wave g_WAVE_GENERAL;  // 0x00945ED0


// Native tech_achieved5BB496 and tech_advance5BE673 read this shared
// dword; the adjacent945F44 AI batch latch and945F48 phase are distinct.
// RECOVERY_OBJECT: v2 0x00945F40 0x00945F44 ?TechAchievedGate@@3HA 0x005BB000
int TechAchievedGate = 0;
// The original 0x00945F44 zero-filled latch is raised by time_warp's AI research batch; tech_advance and base_change read it. Keep it distinct from TechAchievedGate (0x00945F40) and the upkeep phase at 0x00945F48.
// RECOVERY_OBJECT: v1 0x00945F44 ?AiResearchBatchActive@@3HA
int AiResearchBatchActive = 0;

// Per-technology faction-ownership bitsets, populated by the rules/game data
// loaders and changed as scenario-editor ownership is edited.
// The editor traverses 89 byte cells at 9A6670; this is the array itself,
// not a pointer containing an address in the old executable.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

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
// RESOLVED (was UNEXPLORED): the `tech_id < -1` branch was never a fabricated
//   constant. `mov eax,[0x9b90f8]; mov eax,[eax+0x4d8]; push eax; call
//   Strings::get` IS `label_get(310)` - `Labels.strings_ptr` (the object at
//   0x009B90F8) indexed by 310, i.e. 0x4d8/4. Same for label_get(25) = [+0x64]
//   and label_get(306) = [+0x4c8], and label_get(487) = [+0x79c] and
//   label_get(185) = [+0x2e4] further down. The body was already faithful; the
//   `strcat_s(output, 80, ...)` wrappers around those calls were not.
// LEVER: plain `strcat`, not `strcat_s(output, 80, ...)` - every one of the
//   image's 19 string appends here is the 2-arg import at 0x00645470
//   (`push <src>; push <output>; call strcat; add esp, 8`). The bounded
//   spelling changes the import, the stack cleanup and the merged-tail shape.
// LEVER: `" ("` is TWO appends - the image calls strcat with 0x00682820 (" ")
//   and then 0x00682E9C ("(") at 0x005B9D10/0x005B9D1B, and again in the
//   Language arm at 0x005B9E42/0x005B9E4D. No " (" literal exists in the image
//   to fold them into. Landed with the two fixes above; 12/221, 0.735 similar.
// TRIED: guard clauses with an explicit `return;` per branch, to split the
//   three epilogues the image carries at 0x005B9C78/0x005B9CA3/0x005B9CD4 -
//   measured identical, 12/221, so reverted to the if/else-if chain (the image
//   shares its last two tails at 0x005B9ED5/0x005B9EEA, which reads as a chain).
// TRIED: the remaining 190-vs-221 gap is one thing and it is not reachable from
//   this file - the image INLINES tech_category here (its four-compare chain at
//   0x005B9D26-0x005B9D5B) where this tree keeps the out-of-line call, and
//   holding the extra live value costs us the `push ebx` in the prologue that
//   the image defers to a push/pop pair inside the category branch
//   (0x005B9D66/0x005B9DCB). Moving tech_category to technology.h as
//   `MEASURED inline` would earn the inline, but risks the six other call
//   sites and the BYTE_EXACT body it already has; left for a pass that owns
//   the whole tech family.
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
        strcat(output, label_get(310)); // "Not Available"
    } else if (tech_id < 0) {
        strcat(output, label_get(25)); // "NONE"
    } else if (tech_id == 9999) {
        strcat(output, label_get(306)); // "World Map"
    } else if (tech_id < MaxTechnologyNum) {
        strcat(output, StringTable.get(Technology[tech_id].name));
        if (category_lvl) {
            // " " and "(" are TWO separate appends - the image calls strcat
            // with 0x00682820 (" ") and then 0x00682E9C ("("), and there is no
            // " (" literal anywhere in it to fold the pair into.
            strcat(output, " ");
            strcat(output, "(");
            strcat(output, label_get(629 + tech_category(tech_id))); // 'E#', 'D#', 'B#', 'C#'
            _itoa(tech_recurse(tech_id, 0), num_buf, 10);
            strcat(output, num_buf);
            strcat(output, ")");
        }
    } else if (tech_id < 97) {
        int faction_id = tech_id - MaxTechnologyNum;
        if (!Language) {
            strcat(output, Players[faction_id].adj_name_faction);
            strcat(output, " ");
            strcat(output, label_get(487)); // 'Comm Frequency'
        } else {
            strcat(output, label_get(487)); // 'Comm Frequency'
            strcat(output, " ");
            strcat(output, "(");
            GenderDefault = Players[faction_id].noun_gender;
            PluralityDefault = Players[faction_id].is_noun_plural;
            strcat(output, Players[faction_id].noun_faction);
            strcat(output, ")");
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
LPSTR __cdecl tech_name(int tech_id, BOOL category_lvl) {
    TechName[0] = 0;
    say_tech(TechName, tech_id, category_lvl);
    return TechName;
}

/*
Purpose: Check whether faction has a particular tech or not.
// ORIGINAL: 0x005B9F20 ?has_tech@@YA_NHH@Z 0x005B9F20-0x005B9F89 BYTE_EXACT
// body      src/technology.cpp
// LEVER: (2026-09-03, BYTE_EXACT 46/46) the pointer lever below was HALF right
//   and the last 14 instructions were the other half: the two preq reads must
//   be SUBSCRIPTS off the array - `Technology[tech_id].preq_tech_1` into a
//   local, then `Technology[tech_id].preq_tech_2` read straight from memory -
//   so the compiler folds the ARRAY BASE into the displacements
//   (`mov esi, [eax + 0x94f37c]`, `cmp dword ptr [eax + 0x94f380], -1`). A
//   `RulesTechnology *tech = &Technology[tech_id]` local materialises the
//   address and puts the base inside the lea chain instead, which costs four
//   instructions at the first read and the shared tail after it. Full note on
//   the body in technology.h, where it lives.
// TRIED: `RulesTechnology *tech = &Technology[tech_id];` then `tech->preq_tech_1`/`tech->preq_tech_2`
//   forces the compiler to keep BOTH tech_id and faction_id live in callee-saved registers
//   (matching the image's edi/esi) instead of re-deriving the field address per access;
//   two direct `Technology[tech_id].field` reads in one condition compile to a completely
//   different (and much longer) address recomputation. Moved 8/46 -> 32/46 agreeing. The
//   missing half: the reads were still THROUGH A POINTER, which is what blocked the fold.
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
// An ordinary definition in its technology TU (0x005B9C10-0x005BEC3D). Every
// one of the image's 109 out-of-line calls comes from another unit, and the
// in-unit consumers (facility_avail, veh_avail, terrain_avail, tech_avail,
// tech_effects, tech_val, tech_research) expand it under /Ob2 without any
// inline marking. The header keeps a plain declaration.

BOOL __cdecl has_tech(int tech_id, int faction_id) {
    if (faction_id <= 0) {
        return false;
    }
    if (tech_id == TechNone) {
        return true;
    }
    if (tech_id < 0 || tech_id >= (MaxTechnologyNum - 1)) { // excludes 'Transcendent Thought'
        return false;
    }
    // These are SUBSCRIPTS and not a `RulesTechnology *tech = &Technology[tech_id]`
    // local: the image computes `44 * tech_id` once into a register and folds the
    // array base into both displacements - `[eax + 0x94f37c]`, `[eax + 0x94f380]` -
    // which only happens when the field is named off the array. A pointer local
    // materialises the address and the base ends up inside the lea chain instead.
    // `preq_1` is read once and held (the image keeps it in esi across the third
    // test); preq_tech_2 is read from memory a second time.
    int preq_1 = Technology[tech_id].preq_tech_1;
    if (preq_1 < TechNone) {
        // "none, disable" ; valid #TECH preq_tech entry
        return false;
    }
    if (Technology[tech_id].preq_tech_2 < TechNone && preq_1 != TechNone) {
        return false;
    }
    return ((1 << faction_id) & GameDataStateStorage.fields_.game_tech_achieved_[tech_id]) != 0;
}

/*
Purpose: Determine technology level for tech_id.
// Historical pointer/subscript trials below used a fixed-address table pointer.
// The source-owned array permits direct prerequisite indexing while keeping
// the second field read after the first recursive call.
// ORIGINAL: 0x005B9F90 ?tech_recurse@@YAHHH@Z 0x005B9F90-0x005B9FE0 BYTE_EXACT
// body      src/technology.cpp
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
// Ordinary definition in its technology TU: the in-unit callers expand it one
// recursion level under /Ob2 exactly as the former `MEASURED inline` header
// body did, and other units (picktech, reportif, goody_box) call it out of
// line as the image does.
int __cdecl tech_recurse(int tech_id, int base_lvl) {
    if (tech_id < 0 || tech_id >= MaxTechnologyNum) {
        return base_lvl;
    }
    const int next_lvl = base_lvl + 1;
    int val1 = tech_recurse(Technology[tech_id].preq_tech_1, next_lvl);
    int val2 = tech_recurse(Technology[tech_id].preq_tech_2, next_lvl);
    return (val1 > val2) ? val1 : val2;
}
// TRIED: (2026-09-03, after the has_tech fold discovery) `int preq_1 =
//   Technology[tech_id].preq_tech_1; int preq_2 = ...` locals in place of the
//   pointer - 22/36, WORSE than the pointer's 25/36, because both loads then
//   happen before the first recursive call where the image reads preq_tech_2
//   out of [esi + 0x94f380] only after it returns. The image's form wants the
//   reads at the call sites AND the array base folded into the displacements;
//   this compiler will not do both at once here. Left at the pointer form.



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
// Original say_tech open-codes these signed comparisons at 0x005B9D26,
// while six other caller bodies make direct calls to this entry (for
// example labs_write at 0x004A844D). Keep an ordinary external definition,
// visible to the same-TU optimizer without exposing its body in the header.
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
Purpose: Determine if the facility is redundant due to a Secret Project counting as that facility.
// ORIGINAL: 0x005BA030 ?redundant@@YAHHH@Z 0x005BA030-0x005BA0DD BYTE_EXACT
// size      173 bytes
// prototype int (__cdecl ?redundant@@YAHHH@Z)(int facilityID, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: a `switch` assigning ONE shared `project_id` local, checked once at
//        a single `return has_project(project_id, faction_id)`, keeps
//        project_id a RUNTIME value - VC6 cannot fold `base_project()`'s
//        address per case, and 7 cases in a numeric range compile to a jump
//        table besides, where the image cascades explicit compares
//        (FAC_NAVAL_YARD checked first and alone, with its own duplicated
//        tail, before the rest). Returning the has_project() call directly
//        from each arm makes project_id a COMPILE-TIME literal at every
//        call site, so each one folds to the fixed global the image reads
//        (0x9a653c, 0x9a6528, ...), and if/else instead of switch avoids
//        the jump table. Moved 11/61 (0.478) -> 16/61 (0.950).
// TRIED: pulling the image's shared normalizing tail out (`test eax,eax / je
//        / mov eax,1 / xor eax,eax` at 0x5ba0ce, jumped at by arm 0 and
//        fallen into by arms 1-6) with three spellings - `return
//        has_project(...) ? TRUE : FALSE;` per arm; a `BOOL built`
//        accumulator assigned down one if/else-if chain with `return built ?
//        TRUE : FALSE;` after; `BOOL built` per arm + `return built;`. ALL
//        THREE lose VC6's inline expansion of has_project (a real `call` per
//        arm, 49-119 instructions against the image's 61). Only the bare
//        `return has_project(...)` keeps the body written out seven times,
//        and that form returns the inlined `sete` straight away, so the
//        image's re-normalize-through-a-branch tail never appears. Also
//        measured: a `static __forceinline` twin of has_project is IGNORED
//        by VC6 at every one of those call-site shapes (2, 3 and 7 arms)
//        while the identical `static __forceinline` has_fac_built_call IS
//        honoured in is_objective - the twin's own body calls a `MEASURED
//        inline` (dllexport) function, which the other does not.
// MATCH: `if (has_project(...)) return TRUE;` per arm, falling through to
//        `return false`, produces the shared test/je/mov eax,1 tail.
Return Value: Is facility redundant? true/false
Status: Complete
*/
BOOL __cdecl redundant(int facility_id, int faction_id) {
    if (facility_id == FAC_NAVAL_YARD) {
        if (has_project(SP_MARITIME_CONTROL_CENTER, faction_id)) {
            return TRUE;
        }
    }
    if (facility_id == FAC_PERIMETER_DEFENSE) {
        if (has_project(SP_CITIZENS_DEFENSE_FORCE, faction_id)) {
            return TRUE;
        }
    }
    if (facility_id == FAC_COMMAND_CENTER) {
        if (has_project(SP_COMMAND_NEXUS, faction_id)) {
            return TRUE;
        }
    }
    if (facility_id == FAC_BIOENHANCEMENT_CENTER) {
        if (has_project(SP_CYBORG_FACTORY, faction_id)) {
            return TRUE;
        }
    }
    if (facility_id == FAC_QUANTUM_CONVERTER) {
        if (has_project(SP_SINGULARITY_INDUCTOR, faction_id)) {
            return TRUE;
        }
    }
    if (facility_id == FAC_AEROSPACE_COMPLEX) {
        if (has_project(SP_CLOUDBASE_ACADEMY, faction_id)) {
            return TRUE;
        }
    }
    if (facility_id == FAC_ENERGY_BANK) {
        if (has_project(SP_PLANETARY_ENERGY_GRID, faction_id)) {
            return TRUE;
        }
    }
    return false;
}

/*
Purpose: Determine if the provided faction can build a specific facility or Secret Project in the
         specified base. Checks are included to prevent SMACX specific facilities from being built
         in SMAC mode.
// ORIGINAL: 0x005BA0E0 ?facility_avail@@YAHHHHH@Z 0x005BA0E0-0x005BA90A
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x5BA123-0x5BA138 early Ascent102 branch checks Voice slot and victory bit0x800 before filename normalization", "pinned:0x5BA1E2-0x5BA23B clears StringTemp9B86A0, strcat Player.filename+4, CharUpperA import66931C, strcmp CARETAKE literal6916E4; rejects IDs101/102", "pinned:0x5BA749-0x5BA7A5 Sphere23 checks Garden12 built/queued; complete span5BA0E0-5BA90A has no reciprocal Garden gate"], "reason": "Preserve original asymmetric Garden/Sphere availability, early Ascent project/rule return and shared filename normalization before late Caretaker rejection. No symmetric restriction or Caretaker Ascent fix is introduced. Full native matching and runtime behavior remain obligations."}
// size      2090 bytes
// prototype int (__cdecl ?facility_avail@@YAHHHHH@Z)(int facilityID, int factionID, int baseID, int queueCount)
// callers   5   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00421670 0x004E49D0 0x004F80D0 0x0050BA00 0x00645470 0x00645660
// indirect  0x005BA213
// LEVER: has_tech's body moved from technology.cpp to the end of
//        technology.h, and ascending's from base.cpp to the end of base.h,
//        both as `MEASURED inline` - both already had their own bodies
//        (`__forceinline` for has_tech, a plain out-of-line BYTE_EXACT
//        function for ascending), but neither satisfied inlining for a
//        caller in a DIFFERENT translation unit, and the image open-codes
//        both of them here (has_tech's whole preq_tech walk, and
//        ascending's single base_project() != SP_Unbuilt compare). Best
//        similarity 0.137, agreeing instructions 20->25 of 742. The ten
//        `call 0x50ba00` (bitmask) sites the image keeps are the documented
//        systemic has_fac_built(literal,...) folding, TRIED on has_fac
//        base.cpp:104, and are not fixable from this site.
Return Value: Is facility or Secret Project available to faction, base and game mode? true/false
Status: Complete
*/
BOOL __cdecl facility_avail(int facility_id, int faction_id, int base_id,
                            int queue_count) {
    // initial checks
    if (!facility_id || (facility_id == FAC_SKUNKWORKS && GameDataStateStorage.fields_.diff_level_current_ <= DLVL_SPECIALIST)
        || (facility_id >= FAC_HUMAN_GENOME_PROJ && GameDataStateStorage.fields_.game_rules_ & RULES_SCN_NO_BUILDING_SP)) {
        return false; // Skunkworks removed if there are no prototype costs
    }
    if (facility_id == FAC_ASCENT_TO_TRANSCENDENCE) { // at top since anyone can build it
        // This owner tests the Voice slot directly in the original early branch.
        return base_project(SP_VOICE_OF_PLANET) != SP_Unbuilt
            && GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_TRANSCENDENCE;
    }
    if (!has_tech(Facility[facility_id].preq_tech, faction_id)) { // Check tech for facility + SP
        return false;
    }
    // Secret Project availability
    if (!ExpansionEnabled && (facility_id == FAC_MANIFOLD_HARMONICS
        || facility_id == FAC_NETHACK_TERMINUS || facility_id == FAC_CLOUDBASE_ACADEMY
        || facility_id == FAC_PLANETARY_ENERGY_GRID)) {
        return false;
    }
    // The original normalizes the filename in shared scratch storage here;
    // the early Ascent return above bypasses this later Caretaker check.
    StringTemp[0] = 0;
    strcat(StringTemp, Players[faction_id].filename);
    CharUpperA(StringTemp);
    if (!strcmp(StringTemp, "CARETAKE")
        && (facility_id == FAC_VOICE_OF_PLANET
            || facility_id == FAC_ASCENT_TO_TRANSCENDENCE)) {
        return false;
    }
    if (facility_id >= FAC_HUMAN_GENOME_PROJ) {
        return base_project(facility_id - FAC_HUMAN_GENOME_PROJ) == SP_Unbuilt;
    }
    // Facility availability
    if (base_id < 0) {
        return true;
    }
    if (has_fac(facility_id, base_id, queue_count)) {
        return false; // already built or in queue
    }
    if (redundant(facility_id, faction_id)) {
        return false; // has SP that counts as facility
    }
    switch (facility_id) { // consolidated into switch to improve readability and performance
      case FAC_RECYCLING_TANKS:
        return !has_fac(FAC_PRESSURE_DOME, base_id, queue_count); // count as Recycling Tank, skip
      case FAC_TACHYON_FIELD:
        return has_fac(FAC_PERIMETER_DEFENSE, base_id, queue_count)
            || has_project(SP_CITIZENS_DEFENSE_FORCE, faction_id); // Cumulative
      case FAC_SKUNKWORKS:
        return !(Players[faction_id].rule_flags & RFLAG_FREEPROTO); // no prototype costs? skip
      case FAC_HOLOGRAM_THEATRE:
        return has_fac(FAC_RECREATION_COMMONS, base_id, queue_count) // not documented in manual
            && !has_project(SP_VIRTUAL_WORLD, faction_id); // Network Nodes replaces Theater
      case FAC_HYBRID_FOREST:
        return has_fac(FAC_TREE_FARM, base_id, queue_count); // Cumulative
      case FAC_QUANTUM_LAB:
        return has_fac(FAC_FUSION_LAB, base_id, queue_count); // Cumulative
      case FAC_NANOHOSPITAL:
        return has_fac(FAC_RESEARCH_HOSPITAL, base_id, queue_count); // Cumulative
      case FAC_PUNISHMENT_SPHERE:
        return !has_fac(FAC_PARADISE_GARDEN, base_id, queue_count); // antithetical
      case FAC_NANOREPLICATOR:
        return has_fac(FAC_ROBOTIC_ASSEMBLY_PLANT, base_id, queue_count) // Cumulative
            || has_fac(FAC_GENEJACK_FACTORY, base_id, queue_count);
      case FAC_HABITATION_DOME:
        return has_fac(FAC_HAB_COMPLEX, base_id, queue_count); // must have Complex
      case FAC_TEMPLE_OF_PLANET:
        return has_fac(FAC_CENTAURI_PRESERVE, base_id, queue_count); // must have Preserve
      case FAC_QUANTUM_CONVERTER:
        return has_fac(FAC_ROBOTIC_ASSEMBLY_PLANT, base_id, queue_count); // Cumulative
      case FAC_NAVAL_YARD:
        return is_coast(Bases[base_id].x, Bases[base_id].y, false); // needs ocean
      case FAC_AQUAFARM:
      case FAC_SUBSEA_TRUNKLINE:
      case FAC_THERMOCLINE_TRANSDUCER:
        return ExpansionEnabled && is_coast(Bases[base_id].x, Bases[base_id].y, false);
      case FAC_COVERT_OPS_CENTER:
      case FAC_BROOD_PIT:
      case FAC_FLECHETTE_DEFENSE_SYS:
        return ExpansionEnabled;
      case FAC_GEOSYNC_SURVEY_POD: // SMACX only & must have Aerospace Complex
        return ExpansionEnabled && (has_fac(FAC_AEROSPACE_COMPLEX, base_id, queue_count)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)
            || has_project(SP_SPACE_ELEVATOR, faction_id));
      case FAC_SKY_HYDRO_LAB:
      case FAC_NESSUS_MINING_STATION:
      case FAC_ORBITAL_POWER_TRANS:
      case FAC_ORBITAL_DEFENSE_POD:  // must have Aerospace Complex
        return has_fac(FAC_AEROSPACE_COMPLEX, base_id, queue_count)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)
            || has_project(SP_SPACE_ELEVATOR, faction_id);
      case FAC_SUBSPACE_GENERATOR: // Progenitor factions only
        return is_alien_faction(faction_id);
      default:
        break;
    }
    return true;
}

/*
Purpose: Check to see whether provided faction and base can build a specific prototype. Checks are
         included to prevent SMACX specific units from being built in SMAC mode.
// ORIGINAL: 0x005BA910 ?veh_avail@@YAHHHH@Z 0x005BA910-0x005BAB39 BYTE_EXACT
// LEVER: WRONG CALLEE - is_port(base_id, false) hand-inlined to
//   is_coast(Bases[base_id].x, Bases[base_id].y, false); the image never calls is_port.
//   Split every `||`-combined early return (PROTO_ACTIVE/obsolete_factions, and the
//   whole 18-way !ExpansionEnabled SMAC-mode block) into sequential `if (...) return
//   false;` statements - the image gives each condition its own duplicated epilogue
//   rather than sharing one merged branch. The ability_flags checks are bitwise
//   (`abil_flag & ABL_X`), not equality - the image emits `test`, not `cmp`. Together
//   0.538 -> 0.996 similar; TRIED: the last ~6-instruction gap is register
//   allocation inside has_tech()/is_coast()'s own inlined bodies (shared MEASURED
//   inline helpers) - did not chase further to avoid touching their other callers.
// size      553 bytes
// prototype int (__cdecl ?veh_avail@@YAHHHH@Z)(int protoID, int factionID, int baseID)
// callers   7   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E49D0
Return Value: Is unit available to faction/base? true/false
Status: Complete
*/
BOOL __cdecl veh_avail(int proto_id, int faction_id, int base_id) {
    // The image gives PROTO_ACTIVE and obsolete_factions separate early returns, each
    // with its own epilogue, rather than one merged `||` - reproduced as sequential ifs.
    if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)) {
        return false;
    }
    if (VehPrototypes[proto_id].obsolete_factions & (1 << faction_id)) {
        return false;
    }
    if (proto_id < MaxVehProtoFactionNum) {
        if (!has_tech(VehPrototypes[proto_id].preq_tech, faction_id)) {
            return false;
        }
    }
    if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION
        && GameDataStateStorage.fields_.game_rules_ & RULES_SCN_NO_COLONY_PODS) {
        return false;
    }
    // is_port(), hand-inlined - the image calls is_coast() directly here, not is_port().
    if (base_id >= 0 && get_proto_triad(proto_id) == TRIAD_SEA
        && !is_coast(Bases[base_id].x, Bases[base_id].y, false)) {
        return false;
    }
    // The image splits this whole SMAC-mode block into eighteen separate early
    // returns, each with its own epilogue, rather than one merged `||` chain -
    // reproduced as sequential ifs. The ability_flags checks are bitwise (`test`
    // against a single flag bit), not equality - `abil_flag == ABL_X` would only
    // fire for a unit with exactly that one ability and nothing else.
    if (!ExpansionEnabled) {
        if (VehPrototypes[proto_id].armor_id > ARM_PSI_DEFENSE) {
            return false;
        }
        uint8_t weap_id = VehPrototypes[proto_id].weapon_id;
        if (weap_id == WPN_RESONANCE_LASER) {
            return false;
        }
        if (weap_id == WPN_RESONANCE_BOLT) {
            return false;
        }
        if (weap_id == WPN_STRING_DISRUPTOR) {
            return false;
        }
        if (weap_id == WPN_TECTONIC_PAYLOAD) {
            return false;
        }
        if (weap_id == WPN_FUNGAL_PAYLOAD) {
            return false;
        }
        uint32_t abil_flag = VehPrototypes[proto_id].ability_flags;
        if (abil_flag & ABL_SOPORIFIC_GAS) {
            return false;
        }
        if (abil_flag & ABL_DISSOCIATIVE_WAVE) {
            return false;
        }
        if (abil_flag & ABL_MARINE_DETACHMENT) {
            return false;
        }
        if (abil_flag & ABL_FUEL_NANOCELLS) {
            return false;
        }
        if (abil_flag & ABL_ALGO_ENHANCEMENT) {
            return false;
        }
        if (proto_id == BSC_SEALURK) {
            return false;
        }
        if (proto_id == BSC_SPORE_LAUNCHER) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK1) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK2) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK3) {
            return false;
        }
        if (proto_id == BSC_FUNGAL_TOWER) {
            return false;
        }
        if (proto_id == BSC_UNITY_MINING_LASER) {
            return false;
        }
    }
    if (proto_id < MaxVehProtoFactionNum) {
        return true;
    }
    return (proto_id / MaxVehProtoFactionNum) == faction_id;
}

/*
Purpose: Check to see whether the specified faction can construct a specific terrain enhancement.
// ORIGINAL: 0x005BAB40 ?terrain_avail@@YAHHHH@Z 0x005BAB40-0x005BAC13 BYTE_EXACT
// size      211 bytes
// prototype int (__cdecl ?terrain_avail@@YAHHHH@Z)(int terraformID, BOOL isSea, int factionID)
// callers   5   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: the base-address split for `&Terraforming[terraform_id]
//        .preq_tech + is_sea` - the image computes `(is_sea + id*8)` as an
//        INDEX first, then a separate `[index*4 + 0x691880]` load; this
//        tree's `lea` folds the base INTO the index before the implied
//        `*4` (0x1a4620*4 == 0x691880, so it is the same address, just
//        computed with the constant added before the scale rather than
//        after). Tried the equivalent `reinterpret_cast<int*>(...)
//        [is_sea]` array-subscript spelling - VC6 canonicalises both to
//        the identical folded `lea`. Not a source-shape question found
//        this pass; best 3/86.
Return Value: Is terrain enhancement available to faction? true/false
Status: Complete
*/
BOOL __cdecl terrain_avail(int terraform_id, BOOL is_sea, int faction_id) {
    int preq_tech = Terraforming[terraform_id].prerequisite_tech[is_sea];
    if (preq_tech < TechNone) {
        return false;
    }
    if ((terraform_id == TERRA_RAISE_LAND || terraform_id == TERRA_LOWER_LAND)
        && GameDataStateStorage.fields_.game_rules_ & RULES_SCN_NO_TERRAFORMING) {
        return false;
    }
    if (terraform_id >= TERRA_CONDENSER && terraform_id <= TERRA_LEVEL_TERRAIN) {
        const BOOL weather_paradigm = has_project(SP_WEATHER_PARADIGM, faction_id);
        if (weather_paradigm) {
            return true;
        }
    }
    return has_tech(preq_tech, faction_id);
}

/*
Purpose: Check to see whether provided faction can research a specific technology. Checks are
         included to prevent SMACX specific Veh from being built in SMAC mode.
// ORIGINAL: 0x005BAC20 ?tech_avail@@YAHHH@Z 0x005BAC20-0x005BADBF BYTE_EXACT
// LEVER: (2026-09-03, 78/130 -> BYTE_EXACT 130/130) the pointer lever this
//   marker credits below was the wrong half of the same discovery has_tech
//   needed: the preq reads here are SUBSCRIPTS read IN ORDER - preq_tech_1
//   into a local, tested, and only then preq_tech_2 - so the compiler keeps
//   `44 * tech_id` in a register with the array base folded into both
//   displacements (`mov eax, [edx + 0x94f37c]`, `mov edi, [edx + 0x94f380]`).
//   The pointer spelling materialised the address, and reading both preqs up
//   front put the second load before the first compare, where the image has it
//   after. Everything else in this body already agreed.
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
// The image calls tech_avail out of line from tech_ai.
#pragma auto_inline(off)
BOOL __cdecl tech_avail(int tech_id, int faction_id) {
    if (has_tech(tech_id, faction_id) || tech_id >= MaxTechnologyNum || (!ExpansionEnabled
        && (tech_id == TECH_PRPSYCH || tech_id == TECH_FLDMOD || tech_id == TECH_ADAPDOC
            || tech_id == TECH_ADAPECO || tech_id == TECH_BIOADAP || tech_id == TECH_SENTRES
            || tech_id == TECH_SECMANI || tech_id == TECH_NEWMISS || tech_id == TECH_BFG9000))) {
        return false;
    }
    // Subscripts, and read IN ORDER: preq_tech_1 into a local, tested, and only
    // then preq_tech_2. The image keeps `44 * tech_id` in a register with the
    // array base folded into both displacements ([edx + 0x94f37c] / [edx +
    // 0x94f380]); a `RulesTechnology *tech` local materialises the address
    // instead, and reading both preqs up front puts the second load before the
    // first compare, where the image has it after.
    int preq_tech_1 = Technology[tech_id].preq_tech_1;
    if (preq_tech_1 < TechNone) {
        return false; // prerequisite tech set to disabled (-2)
    }
    int preq_tech_2 = Technology[tech_id].preq_tech_2;
    if (preq_tech_2 < TechNone) {
        return false; // prerequisite tech set to disabled (-2)
    }
    return (has_tech(preq_tech_1, faction_id) && has_tech(preq_tech_2, faction_id));
}
#pragma auto_inline(on)

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005BADC0 ?tech_secret@@YAXHH@Z 0x005BADC0-0x005BAE54 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005badc0.cpp", "image:terranx_original.exe:0x005BADC0", "Pinned5BADDE parser0 then5BADFF parser1; literal6916F0 THESECRET, loaded Technology+4 name pointer and Player+304 adjective; live ConsoleGlobal local faction read follows caption append.", "Coordinator-owned core-pump-secret-owner-20261001 scope; preserved original seeds and historical obligations, no matching/runtime claim from binding"], "reason": "Technology announcement uses loader-owned Technology.name and Player faction adjective; preserves parser order and native local/remote caption suffix."}
void __cdecl tech_secret(int tech_id, int faction_id) {
    // RulesTechnology::name is the loader-owned string address, not an
    // ordinal.
    parse_say(0, Technology[tech_id].name, -1, -1);
    parse_says(1, Players[faction_id].adj_name_faction, -1, -1);
    StringTemp[0] = '\0';
    // The independent tech_achieved announcement at 5BC490 appends the
    // same caption prefix from 6916F0; retain that shared literal identity.
    // RECOVERY_LITERAL: v1 0x006916F0 0x006916FA ??_C@_09BKAL@THESECRET?$AA@ 0x005BB000
    strcat(StringTemp, "THESECRET");
    say_num(faction_id != ConsoleGlobal.faction_id_);
    X_pop(StringTemp, 0);
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
//   moved closer to the image's 133 (149 -> 122). Left in.
// TRIED: plateau 17/133 under /Oy-; loop register allocation differs from the image -
//        the same open plateau as has_tech/tech_recurse (MEASURED-inline family ceiling)
// TRIED: (2026-09-03) 13/133 under /Oy- after has_tech went BYTE_EXACT - the
//        image's inlined has_tech COPY here is a different compilation than
//        has_tech's own body: it indexes with `imul ecx, ecx, 0x2c`
//        (0x005BAEC6) where has_tech's own body uses the lea/shl chain, and it
//        interleaves the two stride computations (65*faction for PlayersData,
//        359*faction for Players) with the 0xCC-padded param slot [ebp+8]
//        reused to spill one of them (0x005BAE87). A shared inline body cannot
//        be both forms at once, so this is a ceiling, not a defect; the
//        has_tech edit was still worth it (it is what made has_tech and
//        tech_avail exact).
Return Value: n/a
Status: Complete
*/
void __cdecl tech_effects(int faction_id) {
    PlayersData[faction_id].tech_commerce_bonus = Players[faction_id].rule_commerce;
    memset(PlayersData[faction_id].tech_fungus_, 0, sizeof PlayersData[faction_id].tech_fungus_);
    for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
        if (has_tech(tech_id, faction_id)) {
            uint32_t flags = Technology[tech_id].flags;
            if (flags & TFLAG_INC_ENERGY_FUNGUS) {
                PlayersData[faction_id].tech_fungus_[RSC_ENERGY]++;
            }
            if (flags & TFLAG_INC_MINERALS_FUNGUS) {
                PlayersData[faction_id].tech_fungus_[RSC_MINERALS]++;
            }
            if (flags & TFLAG_INC_NUTRIENT_FUNGUS) {
                PlayersData[faction_id].tech_fungus_[RSC_NUTRIENTS]++;
            }
            if (flags & TFLAG_INC_COMMERCE) {
                PlayersData[faction_id].tech_commerce_bonus++;
            }
        }
    }
    int bonus_count = Players[faction_id].faction_bonus_count;
    for (int i = 0; i < bonus_count; i++) {
        if (Players[faction_id].faction_bonus_id[i] == RULE_FUNGNUTRIENT) {
            PlayersData[faction_id].tech_fungus_[RSC_NUTRIENTS] += Players[faction_id].faction_bonus_val1[i];
        }
        if (Players[faction_id].faction_bonus_id[i] == RULE_FUNGMINERALS) {
            PlayersData[faction_id].tech_fungus_[RSC_MINERALS] += Players[faction_id].faction_bonus_val1[i];
        }
        if (Players[faction_id].faction_bonus_id[i] == RULE_FUNGENERGY) {
            PlayersData[faction_id].tech_fungus_[RSC_ENERGY] += Players[faction_id].faction_bonus_val1[i];
        }
    }
    // if values are below zero, cap at zero
    int *tech_fungus = PlayersData[faction_id].tech_fungus_;
    for (int n = 0; n < 4; n++) {
        const int value = tech_fungus[n];
        tech_fungus[n] = value > 0 ? value : 0;
    }
    if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::ECONOMY] > 2) {
        PlayersData[faction_id].tech_commerce_bonus++;
        if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::ECONOMY] > 3) {
            PlayersData[faction_id].tech_commerce_bonus++;
            if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::ECONOMY] > 4) {
                PlayersData[faction_id].tech_commerce_bonus++;
            }
        }
    }
}


// The native propagation paths exclude technology88 and disabled rule
// entries. Ownership is checked separately and reloaded after callbacks.
static bool technology_shareable(int tech_id) {
    if (tech_id < 0 || tech_id >= 0x58)
        return false;
    const RulesTechnology &technology = Technology[tech_id];
    return technology.preq_tech_1 >= -1 &&
           (technology.preq_tech_2 >= -1 || technology.preq_tech_1 == -1);
}

// The four integer roles are faction, technology, other faction and freebie.
// Native phase ordering includes ordinary ownership, map/prototype transfers,
// scoped popup lifetimes, facilities/abilities, social notices and interludes.
// ORIGINAL: 0x005BB000 ?tech_achieved@@YAXHHHH@Z 0x005BB000-0x005BCB55;0x00662710-0x00662A1E
// RECOVERY_BINDING: v1 {"evidence": ["Retired draft git:de105f04:src/unrecovered/005bb000.cpp retains original identity and primary/EH spans; pinned native branches establish the actual argument roles, reactor upgrades, per-model repeated tutorial, recipient visibility/intel and both recursive technology propagation paths.", "recovery/tests/test_tech_achieved_behavior.py exercises actual owner source and rejects inverted reactor/view/treaty predicates, truncated tutorial traversal, swapped recursion arguments and wrong intel ownership. Task38/40 authorize shared945F40 gate and spy-base scalar without changing the original baseline.", "Pinned 5BB461 XOR ECX,ECX and 5BB465 SETE CL form a zero/one snapshot; 5BB468 stores it as a dword at EBP-0x30, and 5BC43C reloads/tests that full word after achievement callbacks. The source keeps the snapshot as int, without moving its early read."], "reason": "Recover the full technology acquisition lifecycle in its real owner using source-owned player/map/base/prototype storage and real scoped Popup, SpriteBox, DesignWin and NewTech interactions; retain all independent callee and historical seed obligations."}
void __cdecl tech_achieved(int faction_id, int tech_id, int faction_id_with,
                           int give_freebie) {
    Popup popup;

    if (tech_id < 0) {
        return;
    }

    const char *leader_name = Players[faction_id].name_leader;
    log_say("Gaining a TECH", leader_name, faction_id, tech_id,
            faction_id_with);

    const uint32_t faction_mask = 1u << (faction_id & 31);
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_mask) != 0) {
        ambience(0xC9);
    }

    // The large pseudo-technology path shares current map/site knowledge,
    // rather than recording an ordinary research discovery.
    if (tech_id >= 0x270F) {
        PlayersData[faction_id].traded_maps |= 1u << faction_id_with;
        PlayersData[faction_id_with].traded_maps |=
            PlayersData[faction_id].traded_maps;

        for (int tile_id = 0; tile_id < MapArea; ++tile_id) {
            Map &tile = map_tiles()[tile_id];
            if ((tile.visibility & (1u << faction_id_with)) == 0) continue;
            tile.visibility |= static_cast<uint8_t>(faction_mask);
            if (faction_id != 0 && faction_id_with != 0) {
                const uint32_t from =
                    tile.bit_visible[faction_id_with - 1] & tile.bit;
                tile.bit_visible[faction_id - 1] |= from;
            }
        }

        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            Base &base = Bases[base_id];
            if ((base.visibility & (1u << faction_id_with)) != 0 ||
                base.faction_id_current == faction_id_with) {
                spot_base(base_id, faction_id);
            }
        }

        if (faction_id == ConsoleGlobal.faction_id_) {
            draw_map(1);
            WorldClimateWorldWin.field_239BC_ |= 4;
            WorldClimateWorldWin.GraphicWin::redraw();
        }
        return;
    }

    if (tech_id >= 0x59 && tech_id < 0x61) {
        treaty_on(faction_id, tech_id - 0x59, 8);
        return;
    }

    if (tech_id >= 0x61) {
        const int prototype_index = tech_id - 0x61;
        const VehPrototype &prototype = VehPrototypes[prototype_index];
        const int proposed = propose_proto(
            faction_id, prototype.chassis_id, prototype.weapon_id,
            prototype.armor_id, static_cast<int>(prototype.ability_flags),
            prototype.reactor_id, -2,
            const_cast<char *>(prototype.veh_name));
        if (proposed >= 0) {
            if (IsMultiplayerNet != 0 &&
                (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_mask) != 0) {
                VehPrototypes[proposed].obsolete_factions |= 1;
            } else {
                prune_protos(faction_id, proposed, 0);
                upgrade_any_prototypes(faction_id);
            }
        }
        return;
    }

    PlayerData &player = PlayersData[faction_id];
    if (player.tech_id_researching == tech_id) {
        player.tech_id_researching = -1;
    }

    const int was_new = GameDataStateStorage.fields_.game_tech_achieved_[tech_id] == 0;
    GameDataStateStorage.fields_.game_tech_achieved_[tech_id] |= static_cast<uint8_t>(faction_mask);
    ++player.tech_achieved;
    if (GameDataStateStorage.fields_.turn_current_num_ == 0 || TechAchievedGate != 0) {
        return;
    }
    ++player.tech_ranking;
    if (player.unk_26 != 0) --player.unk_26;
    ++player.tech_ranking;
    if (player.unk_26 != 0) --player.unk_26;
    if (tech_id == 0x58) ++player.theory_of_everything;
    // Reactor unlocks are checked against the three non-initial reactor
    // levels (native starts at Reactor[1].preq_tech and advances by one
    // 0x0C-byte RulesReactor record). Each matching tier offers eligible
    // active prototypes to the genuine proposal/pruning path.
    int reactor_unlock_level = 0;
    int reactor_level = 2;
    for (int reactor_id = 1; reactor_id < MaxReactorNum;
         ++reactor_id, ++reactor_level) {
        if (Reactor[reactor_id].preq_tech != tech_id) continue;
        const int first_proto = faction_id * MaxVehProtoFactionNum;
        for (int proto_id = first_proto;
             proto_id < first_proto + MaxVehProtoFactionNum; ++proto_id) {
            const VehPrototype &prototype = VehPrototypes[proto_id];
            if ((prototype.flags & PROTO_ACTIVE) == 0 ||
                (prototype.obsolete_factions & faction_mask) != 0 ||
                prototype.reactor_id >= reactor_level) {
                continue;
            }
            // The dialog's native tier is the reactor level whose eligible
            // prototype reached the proposal call, not a count of matching
            // prerequisite records.
            reactor_unlock_level = reactor_level;
            const int16_t flags_word = static_cast<int16_t>(prototype.flags);
            const int proposal_class =
                ~(static_cast<int>(flags_word) >> 2) | 0xFFFFFFFE;
            const int proposed = propose_proto(
                faction_id, prototype.chassis_id, prototype.weapon_id,
                prototype.armor_id, static_cast<int>(prototype.ability_flags),
                reactor_level, proposal_class, nullptr);
            if (proposed < 0) continue;
            if (IsMultiplayerNet != 0 &&
                (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_mask) != 0) {
                VehPrototypes[proposed].obsolete_factions |= 1;
            } else {
                prune_protos(faction_id, proposed, 0);
                upgrade_any_prototypes(faction_id);
            }
        }
    }
    if (reactor_unlock_level != 0 && faction_id == ConsoleGlobal.faction_id_ &&
        IsMultiplayerNet == 0) {
        parse_say(0, Reactor[reactor_unlock_level - 1].name, -1, -1);
        X_pop("REACTORUPGRADE", 0);
    }

    FirstProposedPrototype = -1;
    if (IsMultiplayerNet == 0) {
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_mask) != 0 &&
            (GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_AUTO_DESIGN_VEH) != 0) {
            const int first_proto = faction_id * MaxVehProtoFactionNum;
            for (int proto_id = first_proto;
                 proto_id < first_proto + MaxVehProtoFactionNum; ++proto_id) {
                VehPrototypes[proto_id].flags &= ~0xC0;
            }
            consider_designs(faction_id);
            if (faction_id == ConsoleGlobal.faction_id_) {
                for (int proto_id = 0; proto_id < MaxVehProtoFactionNum;
                     ++proto_id) {
                    const VehPrototype &prototype = VehPrototypes[proto_id];
                    if ((prototype.flags & PROTO_ACTIVE) != 0 &&
                        prototype.preq_tech == tech_id &&
                        FirstProposedPrototype < 0) {
                        FirstProposedPrototype = proto_id;
                    }
                }
            }
        }
    } else {
        player.flags |= 2;
    }

    // Pinned 0x005BB71A clears this pair after prototype selection and
    // before the technology presentation path.
    TechnologyHelpEntryName = reinterpret_cast<LPSTR>(static_cast<intptr_t>(-1));
    TechnologyHelpEntryKind = -1;

    // The presentation payload is prepared for both human and AI recipients.
    // These offsets are the typed Player fields: the native record's name
    // starts at 0x946A84, while its noun/gender/plural fields are in the same
    // 0x59C-byte Player record.
    StringTemp[0] = 0;
    say_tech(StringTemp, tech_id, FALSE);
    parse_says(0, StringTemp, -1, -1);

    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(1, Players[faction_id].noun_faction + 4, -1, -1);

    if (faction_id_with > 0) {
        GenderDefault = Players[faction_id_with].noun_gender;
        PluralityDefault = Players[faction_id_with].is_noun_plural;
        parse_says(2, Players[faction_id_with].noun_faction + 4, -1, -1);
    } else if (faction_id_with == -1) {
        parse_say(2, Facility[FacilitySPStart + FAC_PLANETARY_DATALINKS].name,
                  -1, -1);
    } else if (faction_id_with < -1) {
        // Native reads the string-table token at Labels.strings_ptr + 0x35C
        // and resolves it through StringTable::get before parsing.
        parse_says(2, label_get(0x35C / sizeof(int)), -1, -1);
    }

    if (faction_id == ConsoleGlobal.faction_id_) {
        popup.init(0, 0);
        popup.set_width(0x1F4);
        StringTemp[0] = 0;
        strcat(StringTemp, "TECH");
        char tech_id_text[12];
        _itoa(tech_id, tech_id_text, 10);
        strcat(StringTemp, tech_id_text);
        popup.start(const_cast<char *>("BLURBSX"), StringTemp, -1,
                    nullptr, 0x80, nullptr);
        popup.string(const_cast<char *>("^"));
        popup.string(const_cast<char *>("^------"));
        popup.string(const_cast<char *>("^"));
    }

    StringTemp[0] = 0;
    strcat(StringTemp, faction_id == ConsoleGlobal.faction_id_ ? "WE" : "THEY");
    strcat(StringTemp, faction_id_with != 0 ? "ACQUIRE" : "DEVELOP");
    if (give_freebie != 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, "WEGETSHARETECH");
    }

    {
        SpriteBox sprite_box;
        sprite_box.sprite(nullptr,
                          reinterpret_cast<char *>(
                              &g_TECH_ICON_SPRITES[tech_id]),
                          0);
    }
    popup.start(PopupStartCaption, StringTemp, -1, nullptr, 0);

    if (faction_id == ConsoleGlobal.faction_id_) {
        help_tech_info(&popup, tech_id, faction_id);
    }

    g_FX.play(0x33);
    if (faction_id == ConsoleGlobal.faction_id_) {
        if (IsMultiplayerNet != 0 &&
            (DiplomacyActive != 0 || CommlinkPending != 0 ||
             TurnRedrawPending != 0)) {
            parse_say(0, Technology[tech_id].name, -1, -1);
            NetMsgPrimary.pop("TECHOBTAINED", 0x1388, 0, nullptr);
        } else {
            NewTechWindow.exec(tech_id, faction_id_with);
        }
    } else if (IsMultiplayerNet == 0) {
        popup.exec(0, nullptr);
    }

    // Faction-granted free facilities become available as soon as their own
    // prerequisite technology is discovered. Native walks the live bonus
    // count, then awards the facility to each base currently owned by this
    // faction before processing the free-ability and project notices.
    give_freebie = 0;
    int bonus;
    for (bonus = 0; bonus < Players[faction_id].faction_bonus_count;
         ++bonus) {
        if (Players[faction_id].faction_bonus_id[bonus] != RULE_FREEFAC) {
            continue;
        }
        const int facility_id = Players[faction_id].faction_bonus_val1[bonus];
        if (Facility[facility_id].preq_tech != tech_id) continue;
        give_freebie = 1;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current == faction_id) {
                set_fac(facility_id, base_id, TRUE);
            }
        }
        if (faction_id == ConsoleGlobal.faction_id_) {
            parse_say(0, Technology[tech_id].name, -1, -1);
            parse_say(1, Facility[facility_id].name, -1, -1);
            NetMsgPrimary.pop("FREEFACTECH", -5000, 0, nullptr);
        }
    }
    if (give_freebie != 0) {
        NetDaemonNet.synch(0x14, 0, 0, 0, nullptr, 1, 0x2101);
    }

    for (bonus = 0; bonus < Players[faction_id].faction_bonus_count;
         ++bonus) {
        if (Players[faction_id].faction_bonus_id[bonus] != RULE_FREEABIL) {
            continue;
        }
        const int ability_id = Players[faction_id].faction_bonus_val1[bonus];
        if (Ability[ability_id].preq_tech != tech_id ||
            faction_id != ConsoleGlobal.faction_id_) {
            continue;
        }
        parse_say(0, Technology[tech_id].name, -1, -1);
        parse_say(1, Ability[ability_id].name, -1, -1);
        NetMsgPrimary.pop("FREEABILTECH", -5000, 0, nullptr);
    }

    if ((Technology[tech_id].flags & TFLAG_SECRETS) != 0) {
        mon_secrets_of_tech(faction_id);
    }

    if (faction_id == ConsoleGlobal.faction_id_ &&
        tech_id == Terraforming[TERRA_FOREST].prerequisite_tech[0] && IsMultiplayerNet == 0) {
        parse_say(0, Technology[tech_id].name, -1, -1);
        // Original TREETIME argument at 0x005BBC5E; independent literal storage.
        // RECOVERY_LITERAL: v1 0x0069178C 0x00691795 ??_C@_08BPDN@TREETIME?$AA@ 0x005BB000
        X_pop(TutorialPopupCaption, "TREETIME", nullptr);
    }

    if (faction_id == ConsoleGlobal.faction_id_ &&
        FirstProposedPrototype >= 0 && IsMultiplayerNet == 0) {
        int design_choice = 0;
        int proto_id;
        for (proto_id = 0; proto_id < MaxVehProtoFactionNum;
             ++proto_id) {
            const VehPrototype &prototype = VehPrototypes[proto_id];
            if ((prototype.flags & PROTO_ACTIVE) == 0 ||
                prototype.preq_tech != tech_id ||
                Weapon[prototype.armor_id].offense_rating < 0 ||
                // The native compares the prerequisite-field pointer with
                // 0x009ABBA6, which is VehPrototypes[15].preq_tech.
                proto_id == 15) {
                continue;
            }
            if (design_choice == 0) {
                design_choice = X_pop("ASKSEEDESIGN", nullptr) == 0 ? -1 : 1;
            } else if (design_choice > 0) {
                design_new_veh(-faction_id, proto_id);
            }
        }

        const int first_proto = faction_id * MaxVehProtoFactionNum;
        for (proto_id = first_proto;
             proto_id < first_proto + MaxVehProtoFactionNum; ++proto_id) {
            VehPrototype &prototype = VehPrototypes[proto_id];
            if ((prototype.flags & 0x80) != 0) {
                if ((static_cast<uint8_t>(prototype.flags) & faction_mask) == 0) {
                    if (design_choice == 0) {
                        design_choice =
                            X_pop("ASKSEEDESIGN", nullptr) == 0 ? -1 : 1;
                    }
                    if (design_choice > 0) {
                        design_new_veh(-faction_id, proto_id);
                    }
                }
                prototype.flags &= static_cast<uint16_t>(~0x80);
            }
        }

        {
            DesignWin design_window;
            design_window.shut_that_badboy_down();
        }

        for (proto_id = first_proto;
             proto_id < first_proto + MaxVehProtoFactionNum; ++proto_id) {
            const VehPrototype &prototype = VehPrototypes[proto_id];
            if ((static_cast<uint8_t>(prototype.flags) & faction_mask) == 0 &&
                (prototype.flags & 0x100) != 0 &&
                (IsMultiplayerNet == 0 ||
                 (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_mask) == 0)) {
                upgrade_prototypes(faction_id, proto_id);
            }
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_ &&
        PlayersData[faction_id].diff_level > 1 && IsMultiplayerNet == 0) {
        for (int category = 0; category < MaxSocialCatNum; ++category) {
            for (int social_model = 0; social_model < 4; ++social_model) {
                if (SocialCategories[category].preq_tech[social_model] != tech_id ||
                    society_avail(category, social_model, faction_id) == 0)
                    continue;
                int selection;
                do {
                    parse_say(0, Technology[tech_id].name, -1, -1);
                    parse_say(1,
                        SocialCategories[category].name[social_model], -1, -1);
                    StringTemp[0] = 0;
                    strcat(StringTemp, "SOCIETY");
                    selection = X_pop(
                        TutorialPopupCaption, StringTemp, nullptr);
                    if (selection != 0) social_select(faction_id);
                } while (selection == 2);
            }
        }
    }

    if ((Technology[tech_id].flags & TFLAG_REVEALS_MAP) != 0 &&
        (PlayersData[faction_id].flags & 0x200) == 0) {
        Map *tile = map_tiles();
        for (int tile_id = 0; tile_id < MapArea; ++tile_id, ++tile) {
            tile->visibility |= static_cast<uint8_t>(faction_mask);
            if (faction_id != 0) {
                tile->bit_visible[faction_id - 1] = tile->bit;
            }
        }

        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            Bases[base_id].visibility |= static_cast<uint8_t>(faction_mask);
            Bases[base_id].faction_pop_size_intel[faction_id] =
                Bases[base_id].population_size;
        }
        PlayersData[faction_id].flags |= 0x200;

        if (GameDataStateStorage.fields_.turn_current_num_ != 0 &&
            faction_id == ConsoleGlobal.faction_id_) {
            parse_say(0, Technology[tech_id].name, -1, -1);
            popp(ScriptPopupFile, "SEEMAP2", 0, "native_sm.pcx", nullptr);
            draw_map(1);
            set_dirty();
            WorldClimateWorldWin.GraphicWin::redraw();
        }
    }

    // Recompute the recipient's technology-derived production and commerce
    // after the direct free-facility/free-ability notifications above.
    tech_effects(faction_id);

    if (faction_id == ConsoleGlobal.faction_id_) {
        if (tech_id == Facility[9].preq_tech) {
            interlude(1, nullptr, 1, 0);
        }
        if (tech_id == 0x2F) {
            interlude(2, nullptr, 1, 0);
        }

        if (GameDataStateStorage.fields_.secret_project_.voice_of_planet() != -1 && faction_id > 0 &&
            Technology[34].preq_tech_1 >= -1 &&
            (Technology[34].preq_tech_2 >= -1 ||
             Technology[34].preq_tech_1 == -1) &&
            (GameDataStateStorage.fields_.game_tech_achieved_[34] & faction_mask) != 0 &&
            Technology[57].preq_tech_1 >= -1 &&
            (Technology[57].preq_tech_2 >= -1 ||
             Technology[57].preq_tech_1 == -1) &&
            (GameDataStateStorage.fields_.game_tech_achieved_[57] & faction_mask) != 0) {
            interlude(0xF, nullptr, 1, 0);
            interlude(0x10, nullptr, 0, 0);
        }

        if (tech_id == 4 && (Players[faction_id].rule_flags & 0x80) != 0) {
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (other != faction_id &&
                    (Players[other].rule_flags & 0x80) == 0 &&
                    (PlayersData[faction_id].diplo_treaties[other] & 8) != 0) {
                    interlude(0x24, nullptr, 1, 0);
                }
            }
        }
    }

    const int chained_base = GameDataStateStorage.fields_.secret_project_.planetary_datalinks();
    if (chained_base >= 0) {
        const int chained_faction = Bases[chained_base].faction_id_current;
        if (chained_faction > 0 && faction_id > 0 && technology_shareable(tech_id) &&
            (GameDataStateStorage.fields_.game_tech_achieved_[tech_id] & faction_mask) != 0 &&
            (GameDataStateStorage.fields_.game_tech_achieved_[tech_id] & (1u << chained_faction)) == 0) {
            int known_factions = 0;
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (technology_shareable(tech_id) &&
                    (GameDataStateStorage.fields_.game_tech_achieved_[tech_id] & (1u << other)) != 0)
                    ++known_factions;
            }
            if (known_factions >= 3)
                tech_achieved(chained_faction, tech_id, -1, 0);
        }
    }

    if (faction_id > 0 && technology_shareable(tech_id) &&
        (GameDataStateStorage.fields_.game_tech_achieved_[tech_id] & faction_mask) != 0) {
        for (int receiver = 1; receiver < MaxPlayerNum; ++receiver) {
            if (Players[receiver].rule_sharetech <= 0 ||
                (technology_shareable(tech_id) &&
                 (GameDataStateStorage.fields_.game_tech_achieved_[tech_id] & (1u << receiver)) != 0))
                continue;
            int known_factions = 0;
            for (int donor = 1; donor < MaxPlayerNum; ++donor) {
                if (!technology_shareable(tech_id) ||
                    (GameDataStateStorage.fields_.game_tech_achieved_[tech_id] & (1u << donor)) == 0)
                    continue;
                if (ExpansionEnabled == 0 ||
                    (Players[receiver].rule_flags & RFLAG_TECHSHARE) == 0 ||
                    (PlayersData[receiver].diplo_treaties[donor] &
                     (DTREATY_HAVE_INFILTRATOR | DTREATY_PACT)) != 0 ||
                    (GameDataStateStorage.fields_.secret_project_.empath_guild() >= 0 &&
                     Bases[GameDataStateStorage.fields_.secret_project_.empath_guild()].faction_id_current == receiver) ||
                    (receiver == GameDataStateStorage.fields_.governor_faction_ &&
                     (Players[donor].rule_flags & RFLAG_ALIEN) == 0))
                    ++known_factions;
            }
            if (known_factions >= Players[receiver].rule_sharetech)
                tech_achieved(receiver, tech_id, -2, 1);
        }
    }

    if (GameDataStateStorage.fields_.turn_current_num_ != 0 && was_new &&
        (Technology[tech_id].flags & TFLAG_SECRETS) != 0) {
        player.flags |= 0x40;
        parse_say(0, Technology[tech_id].name, -1, -1);
        parse_says(1, Players[faction_id].adj_name_faction + 4, -1, -1);
        StringTemp[0] = 0;
        strcat(StringTemp, "THESECRET");
        char recipient_text[12];
        _itoa(faction_id != ConsoleGlobal.faction_id_ ? 1 : 0,
              recipient_text, 10);
        strcat(StringTemp, recipient_text);
        X_pop(StringTemp, nullptr);
        log_say("Learning a SECRET", leader_name, faction_id, tech_id,
                GameDataStateStorage.fields_.game_tech_achieved_[tech_id]);
    }

    desktop_update();
}

/*
Purpose: Determine if preqTechID is a prerequisite of parentTechID within descending range.
// ORIGINAL: 0x005BCB60 ?tech_is_preq@@YAHHHH@Z 0x005BCB60-0x005BCBDB BYTE_EXACT
// body      src/technology.cpp
// LEVER: guard-split, in technology.h where the body lives - the first two tests are
//   TWO separate early returns, each with its own `xor eax,eax / pop ebx / pop ebp / ret`
//   epilogue, not one `if (preq_tech_id < 0 || parent_tech_id < 0)`. The chained spelling
//   shares one epilogue, which puts `mov eax,[ebp+0xc]` where the image has the first
//   guard's `xor eax,eax` and everything after it diverges (5/63). Split: 63/63 BYTE_EXACT.
//   The earlier TRIED line below blamed the `&Technology[...]` pointer for the same
//   divergence - the pointer was incidental; both spellings chained the guards.
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
// Ordinary definition in its technology TU: the in-unit callers expand it one
// recursion level under /Ob2 exactly as the former `MEASURED inline` header
// body did, and other units (picktech, reportif, goody_box) call it out of
// line as the image does.
BOOL __cdecl tech_is_preq(int preq_tech_id, int parent_tech_id, int range) {
    // EACH of these four guards is its OWN early return with its own epilogue -
    // the image never shares the `return false` block between two tests, so the
    // first two must not be chained with `||` (see technology.cpp's LEVER list).
    if (preq_tech_id < 0) {
        return false;
    }
    if (parent_tech_id < 0) {
        return false;
    }
    if (preq_tech_id == parent_tech_id) {
        return true;
    }
    if (!range) {
        return false;
    }
    return tech_is_preq(preq_tech_id, Technology[parent_tech_id].preq_tech_1, range - 1)
        || tech_is_preq(preq_tech_id, Technology[parent_tech_id].preq_tech_2, range - 1);
}


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
// TRIED: (2026-09-03) 31/1361 under /O2 /Oy- after tech_is_preq went BYTE_EXACT
//   (was 22/1361) - its 20 inlined pairs now compile from an exact body. The
//   remaining wall is the frame: the image allocates 0x20 of locals and loads
//   tech_id into edx BEFORE the callee-saved pushes (0x005BCBE3-0x005BCBE9),
//   this tree allocates 0x38 and pushes ebx first, so every displacement after
//   that is off by one slot. 4133 bytes of body behind that; not reachable in
//   a source-form pass.
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
            if (i != faction_id
                && (PlayersData[faction_id].diplo_treaties[i] & DTREATY_VENDETTA) != 0) {
                vendetta_count++;
            }
        }
        uint32_t factor_ai = 1;
        if (!simple_calc) {
            factor_ai = (GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH) ? 4 : 2;
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
        int base_count = PlayersData[faction_id].current_num_bases;
        if ((!power || !ai_power && !vendetta_count) && (!tech || !ai_tech)
            && (!wealth || !ai_wealth) && (!growth || !ai_growth && base_count >= 4)) {
            value_ret = (value_ret + 1) / 2;
        }
        BOOL is_human_player = is_human(faction_id);
        if (!is_human_player && !has_tech(tech_id, faction_id) && simple_calc) {
            // LEVER: the image's two adjacent bit_count() calls (0x005BCEC7,
            // 0x005BCEDD, 22 bytes apart) are this test and this expression
            // each evaluating bit_count() separately, not a cached local.
            if (bit_count(GameDataStateStorage.fields_.game_tech_achieved_[tech_id]) > 1) {
                value_ret += 2 - 2 * bit_count(GameDataStateStorage.fields_.game_tech_achieved_[tech_id]); // increase priority if more factions have tech
            }
            int search_lvl = 1;
            for (int i = 0; i < MaxTechnologyNum; i++) {
                if (has_tech(i, faction_id)) {
                    int current_lvl = tech_recurse(i, 0);
                    if (search_lvl < current_lvl) {
                        search_lvl = tech_recurse(i, 0);
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
                    int pwr_base = PlayersData[faction_id].region_total_bases[region] * power;
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
                                && PlayersData[faction_id].region_total_bases[region]) {
                                const uint32_t treaties = PlayersData[faction_id].diplo_treaties[i];
                                if ((treaties & DTREATY_COMMLINK) != 0
                                    && ((treaties & (DTREATY_PACT | DTREATY_TREATY)) == 0
                                        || (treaties & DTREATY_WANT_REVENGE) != 0)) {
                                    value_ret += (pwr_base / (base_count * (is_human_player + 1)));
                                }
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
        if (PlayersData[faction_id].soc_effect_base.values_[SocialEffect::PLANET] > 0 && ai_growth) {
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
        if (PlayersData[faction_id].soc_effect_base.values_[SocialEffect::PROBE] <= 0) {
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
        if (Technology[tech_id].flags & TFLAG_SECRETS && !GameDataStateStorage.fields_.game_tech_achieved_[tech_id]
            && !(GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH)) {
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
                && !(GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH)) {
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
                && GameDataStateStorage.fields_.turn_current_num_ > 250) {
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
                    && !GameDataStateStorage.fields_.game_tech_achieved_[i] && tech_is_preq(tech_id, i, 1)) {
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
        int factor = 1;
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
//   automatically (i is also used for tech_avail/tech_val/the Formers compare/the return value).
// TRIED: plateau 10/124 after the tech_recurse inline; the loop's induction variable stays an
//        int index where the image steps a pointer by 0x2C - strength reduction did not fall out
// TRIED: (2026-09-03, re-read after the has_tech/tech_avail landings) the image's
//        strength-reduced pointer lives in a STACK SLOT - `[ebp - 4] = 0x94f380`
//        at 0x005BDC2F, the address of `Technology[0].preq_tech_2`, re-tested
//        against both array bounds (0x005BDCB0/0x005BDCB7) as the inlined
//        tech_recurse's `tech_id < 0 || tech_id >= MaxTechnologyNum`. Same four
//        locals, but the image spills search/is_human/the pointer while this
//        tree keeps them in registers. Nothing in the body's spelling chooses
//        which; measured once more at 10/124 after the header edits, unchanged.
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
    int search = -999;
    int tech_id = -1;
    for (int i = 0; i < MaxTechnologyNum; i++) {
        if (tech_avail(i, faction_id)) {
            int tech_value = tech_val(i, faction_id, false);
            int compare;
            if (GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH) {
                if (is_human(faction_id) && (PlayersData[faction_id].ai_growth
                    || PlayersData[faction_id].ai_wealth)
                    && i == VehPrototypes[BSC_FORMERS].preq_tech) {
                    return i; // Direct human player research toward gaining Formers
                }
                int preq = tech_recurse(i, 0);
                tech_value = (tech_value << 8) / preq;
            }
            compare = is_human(faction_id) ? random(0, tech_value + 1) // unsure why humans get random()
                : bounded_crt_random(tech_value + 1); // vs AI rnd() ?
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
// body      src/technology.cpp
// size      30 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: power_value or 0 if tech_id is greater than max
Status: Complete
*/
// HISTORICAL PLACEMENT NOTE (superseded): this body was in technology.h as
// `MEASURED inline`. The image writes it out at some call sites and calls it
// at others, and a .cpp definition is only ever one of those. The marker stays
// here because that is where the catalogue reads it.
// LEVER: branch polarity - guard clause 'if (tech_id < MaxTechnologyNum) return
//        Technology[tech_id].power_value; return 0;' instead of the
//        '(tech_id >= MaxTechnologyNum) ? 0 : ...' ternary. The ternary put the
//        constant-return block first; the guard clause falls through to the
//        work and tails the constant return, matching the image's jge-past-work
//        layout.
int __cdecl tech_mil(int tech_id) {
    // TODO: why only this one returns 0 with other three returning 1? typo/bug?
    if (tech_id < MaxTechnologyNum) {
        return Technology[tech_id].power_value;
    }
    return 0;
}


/*
Purpose: Get tech_value from technology struct for tech id.
// ORIGINAL: 0x005BDD90 ?tech_tech@@YAHH@Z 0x005BDD90-0x005BDDB1 BYTE_EXACT
// body      src/technology.cpp
// size      33 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: tech_value or 1 if tech_id is greater than max
Status: Complete
*/
// HISTORICAL PLACEMENT NOTE (superseded): this body was in technology.h as
// `MEASURED inline`. The image writes it out at some call sites and calls it
// at others, and a .cpp definition is only ever one of those. The marker stays
// here because that is where the catalogue reads it.
// LEVER: branch polarity - same guard-clause rewrite as tech_mil.
int __cdecl tech_tech(int tech_id) {
    if (tech_id < MaxTechnologyNum) {
        return Technology[tech_id].tech_value;
    }
    return 1;
}


/*
Purpose: Get wealth_value from technology struct for tech id.
// ORIGINAL: 0x005BDDC0 ?tech_infra@@YAHH@Z 0x005BDDC0-0x005BDDE1 BYTE_EXACT
// body      src/technology.cpp
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
// HISTORICAL PLACEMENT NOTE (superseded): this body was in technology.h as
// `MEASURED inline`. The image writes it out at some call sites and calls it
// at others, and a .cpp definition is only ever one of those. The marker stays
// here because that is where the catalogue reads it.
int __cdecl tech_infra(int tech_id) {
    if (tech_id < MaxTechnologyNum) {
        return Technology[tech_id].wealth_value;
    }
    return 1;
}


/*
Purpose: Get growth_value from technology struct for tech id.
// ORIGINAL: 0x005BDDF0 ?tech_colonize@@YAHH@Z 0x005BDDF0-0x005BDE11 BYTE_EXACT
// body      src/technology.cpp
// size      33 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: growth_value or 1 if tech_id is greater than max
Status: Complete
*/
// HISTORICAL PLACEMENT NOTE (superseded): this body was in technology.h as
// `MEASURED inline`. The image writes it out at some call sites and calls it
// at others, and a .cpp definition is only ever one of those. The marker stays
// here because that is where the catalogue reads it.
// LEVER: branch polarity - same guard-clause rewrite as tech_mil.
int __cdecl tech_colonize(int tech_id) {
    if (tech_id < MaxTechnologyNum) {
        return Technology[tech_id].growth_value;
    }
    return 1;
}

// The seed scaffold says int 0; the negative edge only restores
// registers and the normal edge leaves Popup::exec immediately before
// RET. Keep that interpretation as seed history,
// but do not promise a return value without a caller/result consumer.
// ORIGINAL: 0x005BDE20 tech_wizard_unk1 0x005BDE20-0x005BE0F8 FILE
// RECOVERY_BINDING: v1 {"evidence":["task:tech-wizard-owner-oct5","Original 0x005BDE20-0x005BE0F8, 728-byte FILE body; complete original seed header/TRIED/call inventory retained beside this definition.","Pinned raw flow 0x5BDE20..0x5BE0F7: two negative guards; original popup stack arguments; technology lookup and icon calls; normal tail calls second Popup::exec before register/frame restore and RET.","Technology table 0x94F358 stride 0x2C; lookup fields growth +0x14, tech +0x18, wealth +0x1C. Native strict ranking: result1 when growth<wealth && wealth>tech; result0 when growth>wealth && growth>tech; else result2, after sum/equal-ID result9 path.","Popup receiver member evidence: caller_sprite_ +0x2144; Dialogs +0x21D0; SpriteBox +0x2240; SpriteList entry_count +0x2274.","Native pointer cell 0x691B18 selects WIZARD text at 0x691B94; neighboring TUTOR caption uses a distinct cell/text owner.","Seed catalog lists zero direct callers; mapped caller search does not establish absence of indirect callers. Five-argument cdecl stack shape and typed Popup* source parameters are provisional; no return-value contract is asserted."],"reason":"Recover the complete helper through existing typed source owners, preserving the pinned branches, calls, side-effect order and original historical evidence. The source home is a technology-helper grouping, not a claimed historical translation-unit boundary. No new match, direct-caller, or game-acceptance claim."}
// symbol    _tech_wizard_unk1
// TRIED: `void` return instead of `int 0`; splitting the two negative-parameter guards into separate `if`s instead of `||` - neither moved the divergence, which sits at #4 (`mov ebx,[ebp+0x10]` vs a hoisted `xor ebx,ebx`), already before the first call; likely register allocation pressure from the many later 0-argument pushes, not the guard shape. Control flow, field offsets (field_2144_, dialogs_+0x2274/+0x2240) and the lookupA/2+lookupB+lookupMid tie-break were hand-traced from the raw disasm, not Ghidra's decompile.
// size      728 bytes
// prototype
// callers   0   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00406380 0x005B9C40 0x00602600 0x006108E0 0x00625810 0x00625EC0 0x00645470 0x0064FC88
extern "C" void __cdecl tech_wizard_unk1(Popup *first_popup,
                                          Popup *second_popup,
                                          int first_tech, int second_tech,
                                          int force_roll) {
    if (first_tech < 0 || second_tech < 0) {
        return;
    }

    char number[0x50];

    StringTemp[0] = '\0';
    say_tech(StringTemp, first_tech, 0);
    parse_says(0, StringTemp, -1, -1);

    StringTemp[0] = '\0';
    say_tech(StringTemp, second_tech, 0);
    parse_says(1, StringTemp, -1, -1);

    int lookup_a = 1;
    int lookup_mid = 1;
    int lookup_b = 1;
    if (second_tech < MaxTechnologyNum) {
        lookup_a = Technology[second_tech].tech_value;
        lookup_mid = Technology[second_tech].wealth_value;
        lookup_b = Technology[second_tech].growth_value;
    }

    first_popup->start(TechWizardPopupFile, "TECHMIL", -1, 0, 0x100, 0);

    if (first_tech < MaxTechnologyNum) {
        Sprite *const icon = &g_TECH_ICON_SPRITES[first_tech];
        if (first_popup->dialogs().SpriteBox::list_.entry_count_ == 0)
            first_popup->set_caller_picture(icon);
        else
            first_popup->set_caller_picture(0);
        first_popup->dialogs().sprite(icon, 0, 0);
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, "MIL");
    const int roll = force_roll != 0 ? 3 : static_cast<int>(random(0, 3));
    _itoa(roll, number, 10);
    strcat(StringTemp, number);
    first_popup->start(TechWizardPopupFile, StringTemp, -1, 0, 0x80, 0);
    first_popup->exec(0, 0);

    second_popup->start(TechWizardPopupFile, "TECHINF", -1, 0, 0x100, 0);

    if (second_tech < MaxTechnologyNum) {
        Sprite *const icon = &g_TECH_ICON_SPRITES[second_tech];
        if (second_popup->dialogs().SpriteBox::list_.entry_count_ == 0)
            second_popup->set_caller_picture(icon);
        else
            second_popup->set_caller_picture(0);
        second_popup->dialogs().sprite(icon, 0, 0);
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, "INF");

    int result;
    if (lookup_a / 2 + lookup_b + lookup_mid < 1 ||
        second_tech == first_tech) {
        result = 9;
    } else if (force_roll == 0) {
        if (lookup_b < lookup_mid && lookup_a < lookup_mid) {
            result = 1;
        } else if (lookup_mid < lookup_b && lookup_a < lookup_b) {
            result = 0;
        } else {
            result = 2;
        }
    } else {
        result = 3;
    }

    _itoa(result, number, 10);
    strcat(StringTemp, number);
    second_popup->start(TechWizardPopupFile, StringTemp, -1, 0, 0x80, 0);
    second_popup->exec(0, 0);
}

/*
Purpose: Determine whether a faction wants the specified prototype based on the faction's current
         prototype designs.
// ORIGINAL: 0x005BE100 ?wants_prototype@@YAHHH@Z 0x005BE100-0x005BE29D
// TRIED: nested ternary chains (`cond ? a : cond2 ? b : c`) used as a single `if (...)` condition compiled to SETcc-then-test where the image branches directly - splitting each into if/else-if with explicit `continue` (the loop's early-out target is one shared label in the image) recovered the direct jumps. Also split the combined `if (!A || !B) return false;` guard at the top into two separate ifs - the image has two full, separate epilogues, one per check, not a combined OR. And walking a `VehPrototype *proto_cmp` pointer (++proto_cmp each iteration) instead of indexing `VehPrototypes[proto_id_cmp]` per field matches the image's single incrementing esi and its negative-offset field reads (esi-1 for chassis_id, esi+5 for plan) - though the image anchors that pointer at the weapon_id field specifically (esi+0), where this tree's pointer anchors at the struct's own front, a difference not chased further. Moved 0.617 -> 0.865 similar (/O2 /Gy /GR- /Oy- /GX).
// size      413 bytes
// prototype int (__cdecl ?wants_prototype@@YAHHH@Z)(int protoID, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D480
Return Value: Does faction want prototype? true/false
Status: Complete
*/
BOOL __cdecl wants_prototype(int proto_id, int faction_id) {
    if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)) {
        return false;
    }
    if (!(VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE)) {
        return false;
    }
    for (int i = 0; i < MaxVehProtoFactionNum; i++) {
        int proto_id_cmp = faction_id * MaxVehProtoFactionNum + i;
        if (!(VehPrototypes[proto_id_cmp].flags & PROTO_ACTIVE)) {
            continue;
        }
        if (!(VehPrototypes[proto_id_cmp].flags & PROTO_TYPED_COMPLETE)) {
            continue;
        }
        if (VehPrototypes[proto_id].plan != VehPrototypes[proto_id_cmp].plan) {
            continue;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].triad
            != Chassis[VehPrototypes[proto_id_cmp].chassis_id].triad) {
            continue;
        }
        if (Weapon[VehPrototypes[proto_id_cmp].weapon_id].mode > WPN_MODE_MISSILE) {
            if (Weapon[VehPrototypes[proto_id_cmp].weapon_id].mode
                != Weapon[VehPrototypes[proto_id].weapon_id].mode) {
                continue;
            }
        } else if (Weapon[VehPrototypes[proto_id_cmp].weapon_id].offense_rating < 0) {
            if (Weapon[VehPrototypes[proto_id_cmp].weapon_id].offense_rating
                != get_proto_offense_rating(proto_id)) {
                continue;
            }
        } else if (Weapon[VehPrototypes[proto_id_cmp].weapon_id].offense_rating
                   < get_proto_offense_rating(proto_id)) {
            continue;
        }
        if (Armor[VehPrototypes[proto_id_cmp].armor_id].defense_rating <= 0) {
            if (VehPrototypes[proto_id_cmp].armor_id != VehPrototypes[proto_id].armor_id) {
                continue;
            }
        } else {
            // The image calls armor_val for the CANDIDATE first (its index is
            // still in edx) and reloads proto_id from the frame for the second
            // call. A bare `armor_val(proto_id) > armor_val(proto_id_cmp)` makes
            // VC6 keep proto_id in edx for the whole function: 28/134 -> 79/134.
            int armor_cmp = armor_val(proto_id_cmp, faction_id);
            if (armor_cmp < armor_val(proto_id, faction_id)) {
                continue;
            }
        }
        if (Chassis[VehPrototypes[proto_id_cmp].chassis_id].speed >= Chassis[VehPrototypes[proto_id].chassis_id].speed) {
            return false;
        }
    }
    return true;
}

// tech_wizard_unk2, 0x005BE2A0. The tech wizard's facility sprite painter:
// convert the page's structure size into the isometric x/y offsets, publish
// the drawing scale pair, then draw the facility sprite with its
// transparency byte.
/*
Purpose: Draw one tech-wizard facility sprite at the isometric position its
         record's footprint implies.
// ORIGINAL: 0x005BE2A0 tech_wizard_unk2 0x005BE2A0-0x005BE378 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5BE2A0..5BE378; retired seed git:ee30a69d:src/unrecovered/005be2a0.cpp records the same identity and spans with its register-save TRIED note kept below.", "The scale pair is sprite.h's own SpriteScaleNumerator/Denominator (0x00696D18/0x00696D1C), the size/divisor pair is mapwin.h's MapWinCaptionScale/MapWinCaptionDivisor (0x00691E6C/0x00691E70), and the facility tables are sprite.cpp's g_TECH_ICON_SPRITES (0x00759E28, whose 0x2C stride is the seed's 0x11-int entry) with the transparency bytes at 0x00759E30 - all read exactly as the seed measures them.", "The sprite draw is sprite.h's own Sprite::draw(Buffer *, int, int, int)."], "reason": "Coverage transcription of the facility sprite painter with the record-table arithmetic kept at the seed's measured strides."}
// symbol    _tech_wizard_unk2
// TRIED (seed): 6-param __cdecl void, if/else matching Ghidra's arithmetic
//         structure reaches 0.84 mnemonic similarity; the original saves
//         ebx/esi before the al&1 test while the scaffold defers it.
Status: Coverage transcription - unclaimed body
*/
// The scale pair and size pair are the tree's own bindings: sprite.h's
// SpriteScaleNumerator/Denominator (0x00696D18/0x696D1C) and mapwin.h's
// MapWinCaptionScale/MapWinCaptionDivisor (0x00691E6C/0x00691E70). The
// facility tables are sprite.cpp's g_TECH_ICON_SPRITES (0x00759E28, 0x2c
// stride); each Sprite owns its transparency byte at +8 (first 0x00759E30).

// The pinned first stack word is adjusted by the GraphicWin-to-Buffer base
// offset when non-null. Caller provenance is incomplete: this type reflects
// the observed +0x444 base view, not proof of the original source class.
extern "C" void __cdecl tech_wizard_unk2(GraphicWin *canvas, int index, int odd,
                                         int base_x, int base_y, int base_bottom) {
    if (index < 0x59) {
        const int size = MapWinCaptionScale;
        const int divisor = MapWinCaptionDivisor;
        int x_offset;
        if ((odd & 1) != 0) {
            if (size > 0) {
                x_offset = (size * 0xD9) / divisor;
            } else {
                x_offset = 0x6C;
            }
        } else {
            x_offset = 0;
        }
        const int x = base_x + x_offset;

        int y_offset;
        if (size > 0) {
            y_offset = (size * 0xD7) / divisor;
        } else {
            y_offset = 0x6B;
        }
        const int dy = base_bottom - y_offset;
        const int y = base_y + (dy / 2);

        if (size > 0) {
            SpriteScaleNumerator = size;
            SpriteScaleDenominator = divisor;
        } else {
            SpriteScaleNumerator = 8;
            SpriteScaleDenominator = 0x10;
        }

        Buffer *const target = (canvas != 0)
            ? static_cast<Buffer *>(canvas)
            : 0;

        // Pinned 5BE34F reads array base759E28 + element stride2C + member8.
        const unsigned char transparency =
            g_TECH_ICON_SPRITES[index].transparent_index();
        g_TECH_ICON_SPRITES[index].draw(target, transparency, x, y);

        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
    }
}


// The research slot is PlayersData[faction].tech_id_researching, not a
// separate absolute table. A remote human's title/name are announced once
// while the network loop waits for their choice. tech_pick owns its real local
// window in picktech.cpp; NetMsg::pop retains its separate recovery obligations.
// ORIGINAL: 0x005BE380 ?tech_selection@@YAHH@Z 0x005BE380-0x005BE52A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["retired seed git:84bf6850:src/unrecovered/005be380.cpp records the two network wait paths and pop-up order", "PlayersData[faction].tech_id_researching is the seed's 0x0096CDAC+faction*0x20CC slot; Player title/name and general.h's default noun globals cover its 0x00946A50-based records", "ConsoleGlobal.faction_id_ owns the 0x00939284 local-faction slot", "Pinned5BE38A uses a full-width announcement latch;5BE39F/5BE4E2 shift directly without explicit count masks. Network/human guards and research-slot reads occur separately before the first wait and after tech_pick; preserve those live reads across callbacks."], "reason": "Home the typed faction research selection and network wait in the technology owner, retaining the picker and popup owner's separate unfinished obligations."}
int __cdecl tech_selection(int faction) {
    int announced = false;
    if (IsMultiplayerNet != 0) {
        const unsigned int faction_bit = 1u << faction;
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) != 0
            && faction != ConsoleGlobal.faction_id_) {
            while (ExitTurnLoop == 0 && PlayersData[faction].tech_id_researching < 0) {
                NetDaemonNet.net_tasks();
                if (!announced) {
                    announced = true;
                    PluralityDefault = 0;
                    GenderDefault = Players[faction].is_leader_female;
                    parse_says(0, Players[faction].title_leader, -1, -1);
                    GenderDefault = Players[faction].is_leader_female;
                    PluralityDefault = 0;
                    parse_says(1, Players[faction].name_leader, -1, -1);
                    NetMsgPrimary.pop("PICKINGTECH", 0, 0, 0);
                }
            }
            NetMsgPrimary.close();
            return PlayersData[faction].tech_id_researching;
        }
    }
    // RECOVERY_LITERAL: v1 0x0069180C 0x00691818 ??_C@_0M@JCB@PICKINGTECH?$AA@ 0x005BE380
    const int result = tech_pick(faction, 0, -1, 0);
    PlayersData[faction].tech_id_researching = result;
    if (IsMultiplayerNet != 0) {
        const unsigned int faction_bit = 1u << faction;
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) != 0) {
            synch_researching(faction);
            int stop = ExitTurnLoop;
            PlayersData[faction].tech_id_researching = -1;
            while (stop == 0 && PlayersData[faction].tech_id_researching < 0) {
                NetDaemonNet.net_tasks();
                stop = ExitTurnLoop;
            }
        }
    }
    return result;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_LITERAL: v1 0x00691828 0x00691845 ??_C@_0BN@EAGD@Saving?5Earned?5Tech?5for?5Later?$AA@ 0x005BE530
// RECOVERY_LITERAL: v1 0x00691818 0x00691827 ??_C@_0P@HDIK@Earning?5a?5Tech?$AA@ 0x005BE530
/*
Purpose: Finish one faction's research turn: log the tech being researched,
         or when the faction finishes (no PBEM/human gate applies) commit
         the researched tech, reset the faction's bases through bases_reset
         with the selected base preserved, re-select the next tech when the
         gates allow, and count the faction's earned-tech bookkeeping.

// ORIGINAL: 0x005BE530 ?tech_advance@@YAHH@Z 0x005BE530-0x005BE6A5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image full span and eight mapped caller owners", "0x00946A84 + faction*0x59C is Players[faction].name_leader; 0x00939284 is ConsoleGlobal.faction_id_", "0x00691818 and 0x00691828 are the two original research log strings", "PlayersData, GameDataStateStorage and technology callbacks are existing typed owners", "0x00945F44 is the AI batch latch distinct from TechAchievedGate at 0x00945F40 and upkeep phase 0x00945F48"], "reason": "Home original research advancement flow in technology.cpp, retaining arithmetic, branch order, callbacks and historical exactness obligation."}
// TRIED: masking the shift count with `& 0x1f` explicitly (hardware already masks CL); dropping the mask reached mismatch #76 (was #44 with the mask present).
// size      373 bytes
// prototype int (__cdecl ?tech_advance@@YAHH@Z)(int factionID)
// callers   8   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E39D0 0x004E6300 0x004EC3B0 0x005BB000 0x005BE380 0x00626250
// notes     Seed provenance (TRIED): masking the shift count with `& 0x1f`
//           explicitly loses - the hardware already masks CL; dropping the
//           mask reached mismatch #76 (was #44 with the mask present). The
//           0x20CC-stride slots are PlayersData fields: 0x0096CDAC is
//           tech_id_researching (0x3CC) and 0x0096CDB4 is earned_techs_saved
//           (0x3D4).
Return Value: the selection the faction advanced on, or the negative
              re-selection result
Status: Measured BYTE_EXACT here in place (wave 7 record-then-retire); the
        seed git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005be530.cpp is retired and its claim lives on
        the marker above.
*/
int __cdecl tech_advance(int faction) {
    int *researching = &PlayersData[faction].tech_id_researching;
    int *earned = &PlayersData[faction].earned_techs_saved;
    char *tech_name = Players[faction].name_leader;

    log_say("Earning a Tech", tech_name, *researching, 0, 0);

    int selection;
    if (IsMultiplayerNet == 0 || TurnUpkeepPhase != 1) {
        if (IsMultiplayerPBEM == 0 ||
            (((1 << faction) &
              GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) == 0) ||
            faction == ConsoleGlobal.faction_id_) {
            selection = *researching;
            if (selection < 0) {
                selection = tech_selection(faction);
                if (selection < 0) {
                    return selection;
                }
            }
            *researching = -1;
            tech_achieved(faction, selection, 0, 0);
            if (((1 << faction) &
                 GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) == 0) {
                int saved_base = BaseIDCurrentSelected;
                bases_reset(-1, faction, 0);
                if (GameDataStateStorage.fields_.turn_current_num_ != 0 && saved_base >= 0 &&
                    saved_base < GameDataStateStorage.fields_.base_current_count_) {
                    set_base(BaseIDCurrentSelected);
                    base_compute(0);
                }
            }
            if ((IsMultiplayerNet != 0 || faction == ConsoleGlobal.faction_id_) &&
                TechAchievedGate == 0 && AiResearchBatchActive == 0 &&
                *researching < 0) {
                *researching = tech_selection(faction);
            }
            return selection;
        }
    } else {
        log_say("Saving Earned Tech for Later", tech_name, faction, 0, 0);
    }
    ++*earned;
    return -1;
}



// Research effect direction (-1, 0, 1); the image tests the loaded value twice.
static inline int tech_rate_sign(int value) {
    return value > 0 ? 1 : (value < 0 ? -1 : 0);
}

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
//   is signed - `100 * discovery_rate / Rules.tech_discovery_rate_pct_std`
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
// TRIED: (2026-09-03, re-read) the instruction COUNT now matches exactly, 234
//   against 234, and 134 agree under /O2 /Oy-. What is left is register NAMING
//   and one spilled slot, not shape: the image holds `rule_factor` in edi and
//   its >>3 divisor in the same register (`shr edi, 3; idiv edi`,
//   0x005BE7EC-0x005BE7EF) keeping esi for the >>5 copy, where this tree has
//   the two swapped; and the [ebp-4] slot-reuse noted above. Both spellings of
//   the source express the same arithmetic, so there is nothing left to
//   rewrite here.
// LEVER: (0.81 -> 0.887) is_human() re-evaluated per use with diff_lvl as an
//   if/else (the image tests it three times), the research effect folded through
//   a sign helper inline in the multiplier, `diff_factor += range(...)` reused as
//   the final factor, and the second multiply as its own statement. Left: the
//   final factor lives in esi where the image keeps it in ecx.
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
    if (!Rules.tech_discovery_rate_pct_std) {
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
    int diff_factor = is_human(faction_id) ? PlayersData[faction_id].diff_level : GameDataStateStorage.fields_.diff_level_current_;
    diff_factor += (diff_factor < DLVL_LIBRARIAN);
    int diff_lvl;
    if (is_human(faction_id)) {
        diff_lvl = DLVL_LIBRARIAN;
    } else {
        diff_lvl = GameDataStateStorage.fields_.diff_level_current_;
    }
    diff_factor = is_human(faction_id) ? diff_factor * 4 + 8 : 29 - diff_factor * 3;
    diff_factor = range(diff_factor, 12 - player_factor, player_factor + 12);
    int tech_stagnation = GameDataStateStorage.fields_.game_rules_ & RULES_TECH_STAGNATION;
    uint32_t rule_factor = tech_stagnation | 0x40; // 64 or 96
    diff_factor += range(
        player_factor - (GameDataStateStorage.fields_.turn_current_num_ / static_cast<int>(rule_factor >> 3)),
        0, (diff_factor * static_cast<int>(rule_factor >> 5)) >> 1);
    int discovery_rate = diff_factor
        - range((top_factor - diff_lvl - player_factor + 7) / (8 - diff_lvl),
            0, diff_lvl * diff_factor / 10 + 1);
    discovery_rate *= range(player_factor
        - tech_rate_sign(PlayersData[faction_id].soc_effect_base.values_[SocialEffect::RESEARCH]), 1, 99999);
    if (Rules.tech_discovery_rate_pct_std != 100) {
        discovery_rate = 100 * discovery_rate
            / Rules.tech_discovery_rate_pct_std;
    }
    if (Players[faction_id].rule_techcost != 100) {
        discovery_rate = discovery_rate * Players[faction_id].rule_techcost / 100;
    }
    int cost = (discovery_rate * MapAreaSqRoot) / 56;
    if (tech_stagnation) {
        cost += cost / 2; // Slower Rate of Research Discoveries
    }
    return range(cost, 1, 99999999);
}

// ORIGINAL: 0x005BE940 ?tech_research@@YAXHH@Z 0x005BE940-0x005BEB70 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:e82971b277b557e89a52143c7c51170d6d3b0bd4:src/recovered/units/005be940.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "recovery/tests/test_base_colony_economy.py", "Pinned original 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original extent reviewed against native instructions and measured VC6 owner layouts.", "Original5BE940..5BEB70 uses Facility33/102 prerequisites, low-byte LockEnableMask, tech_cost=-1 before recomputation, and shared945F48 phase distinct from9B2070 UI phase."], "reason": "Recover complete research accumulation, climactic prerequisite decisions, advances and selection through real faction/technology storage and shared TurnUpkeepPhase. Preserve native byte-width human mask, callback-owned selection and wrap. Seed retained; no new byte or runtime claim."}
// symbol    ?tech_research@@YAXHH@Z
// LEVER: same TU as has_tech (0x005BE940 inside the 0x005B9F20-0x005BEB70
//        block), which the image inlines twice here; technology.cpp cannot
//        see it without valid_tech_leap (another TU) inlining it too.
void __cdecl tech_research(int faction, int points) {
    if (points <= 0) {
        return;
    }

    if (!is_human(faction) && climactic_battle()) {
        if (!has_tech(Facility[FAC_PSI_GATE].preq_tech, faction)
            || !has_tech(Facility[FAC_ASCENT_TO_TRANSCENDENCE].preq_tech, faction)) {
            points *= 2;
        }
    }

    PlayersData[faction].tech_accumulated += points;
    if (IsMultiplayerNet == 0 && PlayersData[faction].tech_id_researching < 0) {
        goto after_advance;
    }

    {
        const int research_rate = tech_rate(faction);
        if (PlayersData[faction].tech_id_researching >= 0x59) {
            PlayersData[faction].tech_accumulated += points;
        }
        if (PlayersData[faction].tech_accumulated >= research_rate) {
            log_say("Researched Advance", Players[faction].name_leader,
                    faction, 0, 0);
            const int advance = tech_advance(faction);
            if (advance >= 0) {
                mon_tech_discovered(faction, advance);
            }
            PlayersData[faction].tech_accumulated = 0;
            PlayersData[faction].tech_cost = -1;
            PlayersData[faction].tech_cost = tech_rate(faction);
        }

        if ((PlayersData[faction].flags & 0x40u) != 0) {
            log_say("Secret Advance", Players[faction].name_leader,
                    faction, 0, 0);
            PlayersData[faction].flags &= ~0x40u;
            const int advance = tech_advance(faction);
            if (advance >= 0) {
                mon_tech_discovered(faction, advance);
            }
        }
    }

after_advance:
    if (PlayersData[faction].tech_accumulated >= 0 && PlayersData[faction].tech_id_researching < 0
        && TurnUpkeepPhase != 1
        && (!is_human(faction)
            || faction == ConsoleGlobal.faction_id_
            || IsMultiplayerPBEM == 0)) {
        PlayersData[faction].tech_id_researching = tech_selection(faction);
    }
}

// Native 5BEB8F..95 constructs the complete local PickTech; 5BEBB7 calls pick.
// The 484B60 close and reverse Font/base cleanup at 5BEBDF..5BEC27 are the
// caller-inlined complete destructor. Do not add a second explicit close.
// EH action662A1E calls complete PickTech::~PickTech (488690).
// ORIGINAL: 0x005BEB70 ?tech_pick@@YAHHHHPAD@Z 0x005BEB70-0x005BEC3D;0x00662A1E-0x00662A64 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:5BEB70-5BEC3D and662A1E-662A64; retired seed provenance git:ba96f00d:src/unrecovered/005beb70.cpp; historical FILE/TRIED lessons retained.", "image:5BEB8F constructor484AC0, 5BEBB7 pick486510, 5BEBDF close484B60 and reverse Fonts/base cleanup are the inlined complete destructor; EH state0 action662A1E calls destructor488690.", "Historical symbol tech_pick PAD is mutable char*; both current product callers pass null. Authoritative declaration removes stale const overloads without changing arguments."], "reason": "Real automatic PickTech owns complete construction, modal pick and cleanup; close belongs to its destructor, not an extra caller operation."}
// symbol    ?tech_pick@@YAHHHHPAD@Z
// LEVER: technology TU (after tech_research); ~PickTech is inline in
//        picktech.h so it inlines here, while close stays out of line.
int __cdecl tech_pick(int faction, int mode, int count, char *section) {
    PickTech picker;
    return picker.pick(faction, mode, count, section);
}