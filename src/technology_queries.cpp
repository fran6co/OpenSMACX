#include "stdafx.h"
#include "technology.h"

#include "time.h"
#include "datalink.h"
#include "alpha.h"
#include "base.h"
#include "game.h"
#include "general.h"
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
#include "spritebox.h"
#include "log.h"

/*
Purpose: Check whether faction has a particular tech or not.
// ORIGINAL: 0x005B9F20 ?has_tech@@YA_NHH@Z 0x005B9F20-0x005B9F89 BYTE_EXACT
// body      src/technology_queries.cpp
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
// BODY IN technology_queries.cpp. The original header-inline placement and
// wrapper experiments are retained in the historical notes above; the
// canonical definition now lives with the six query consumers whose mapped
// spans expand its body, while cross-TU callers use the ordinary declaration.

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
// ORIGINAL: 0x005BAB40 ?terrain_avail@@YAHHHH@Z 0x005BAB40-0x005BAC13
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
            if (i != faction_id && has_treaty(faction_id, i, DTREATY_VENDETTA)) {
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
