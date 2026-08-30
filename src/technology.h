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
#pragma once

 /*
  * Technology related objects, variables and functions.
  */
enum TechnologyFlagBitfield {
    TFLAG_SECRETS = 0x1,
    TFLAG_IMPROVED_PROBES = 0x2,
    TFLAG_INC_COMMERCE = 0x4,
    TFLAG_REVEALS_MAP = 0x8,
    TFLAG_ALLOW_GENE_WARFARE = 0x10,
    TFLAG_INC_GENE_WARFARE_DEFENSE = 0x20,
    TFLAG_INC_ENERGY_FUNGUS = 0x40,
    TFLAG_INC_MINERALS_FUNGUS = 0x80,
    TFLAG_INC_NUTRIENT_FUNGUS = 0x100,
};

enum TechnologyId {
    TECH_BIOGEN = 0,
    TECH_INDUST = 1,
    TECH_INFNET = 2,
    TECH_PHYSIC = 3,
    TECH_PSYCH = 4,
    TECH_MOBILE = 5,
    TECH_ECOLOGY = 6,
    TECH_SUPER = 7,
    TECH_CHAOS = 8,
    TECH_E_MC2 = 9,
    TECH_FUSION = 10,
    TECH_ALLOYS = 11,
    TECH_SUBAT = 12,
    TECH_CHEMIST = 13,
    TECH_SURFACE = 14,
    TECH_METAL = 15,
    TECH_STRING = 16,
    TECH_MILALG = 17,
    TECH_MAGNETS = 18,
    TECH_MATCOMP = 19,
    TECH_UNIFIED = 20,
    TECH_GRAVITY = 21,
    TECH_POLY = 22,
    TECH_AGRAV = 23,
    TECH_DELETE_1 = 24,
    TECH_QUANTUM = 25,
    TECH_SINGMEC = 26,
    TECH_CONSING = 27,
    TECH_TEMPMEC = 28,
    TECH_PROBMEC = 29,
    TECH_ALGOR = 30,
    TECH_SOLIDS = 31,
    TECH_PLANETS = 32,
    TECH_DIGSENT = 33,
    TECH_HAL9000 = 34,
    TECH_DOCINIT = 35,
    TECH_DOCFLEX = 36,
    TECH_INTEG = 37,
    TECH_FOSSIL = 38,
    TECH_DOCAIR = 39,
    TECH_DOCSEC = 40,
    TECH_MINDMAC = 41,
    TECH_NANOMIN = 42,
    TECH_DOCLOY = 43,
    TECH_ETHCALC = 44,
    TECH_INDECON = 45,
    TECH_INDAUTO = 46,
    TECH_CENTMED = 47,
    TECH_BRAIN = 48,
    TECH_GENE = 49,
    TECH_BIOENG = 50,
    TECH_BIOMAC = 51,
    TECH_NEURAL = 52,
    TECH_CYBER = 53,
    TECH_EUDAIM = 54,
    TECH_WILLPOW = 55,
    TECH_THRESH = 56,
    TECH_MATTER = 57,
    TECH_CENTEMP = 58,
    TECH_ENVECON = 59,
    TECH_ECOENG = 60,
    TECH_PLAECON = 61,
    TECH_ECOENG2 = 62,
    TECH_CENTPSI = 63,
    TECH_ALPHCEN = 64,
    TECH_CREATE = 65,
    TECH_SPACE = 66,
    TECH_HOMOSUP = 67,
    TECH_SUPLUBE = 68,
    TECH_QUANMAC = 69,
    TECH_DELETED_2 = 70,
    TECH_NANEDIT = 71,
    TECH_OPTCOMP = 72,
    TECH_INDROB = 73,
    TECH_CENTGEN = 74,
    TECH_SENTECO = 75,
    TECH_VIRAL = 76,
    TECH_ORBITAL = 77,
    TECH_PRPSYCH = 78,
    TECH_FLDMOD = 79,
    TECH_ADAPDOC = 80,
    TECH_ADAPECO = 81,
    TECH_BIOADAP = 82,
    TECH_SENTRES = 83,
    TECH_SECMANI = 84,
    TECH_NEWMISS = 85,
    TECH_BFG9000 = 86,
    TECH_USER = 87,
    TECH_TRANT = 88,
};

struct RulesTechnology {
    uint32_t flags;
    LPSTR name;
    char id[8]; // short name up to 7 characters in length
    int padding; // unused
    int growth_value;
    int tech_value;
    int wealth_value;
    int power_value;
    int preq_tech_1;
    int preq_tech_2;
};

// THE STRIDE THE IMAGE INDEXES `Technology` (0x0094F358) BY, read off 87
// call sites by tools/derive_array_strides.py - e.g. 0x0042C181, 0x00430B7F. That
// number is a measurement of the binary; `sizeof` below is the compiler's
// arithmetic over the members declared above. The two were computed from
// different things and agree, which is what the assertion records.
static_assert(sizeof(RulesTechnology) == 0x2C,
              "RulesTechnology layout must match the original executable");


struct RulesMandate {
    LPSTR name;
    LPSTR name_caps;
};

static const int MaxTechnologyNum = 89;
static const int TechNone = -1;
static const int TechDisabled = -2;
static const int MaxMandateNum = 4;

// Real storage in the image (.bss, filled by read_tech()), defined in alpha.cpp.
extern RulesTechnology Technology[MaxTechnologyNum]; // 0x0094F358
uint8_t *const GameTechAchieved = (uint8_t *)0x009A6670; // [89]
extern RulesMandate Mandate[MaxMandateNum]; // 0x0094B4A0
extern int TechValidCount;
extern int TechCommerceCount;
extern char TechName[80];

void __cdecl say_tech(int tech_id, BOOL category_lvl);
BOOL __cdecl valid_tech_leap(int tech_id, int faction_id);
void __cdecl say_tech(LPSTR output, int tech_id, BOOL category_lvl);
// `has_tech` and `tech_recurse` are DEFINED below, both as `MEASURED inline`,
// so neither carries a bare declaration here - an `extern` one alongside an
// inline definition is C2375, "redefinition; different linkage". Two agents
// moved one function each into this header from opposite sides of a merge,
// which is how both declarations survived their own removals.
int __cdecl tech_category(int tech_id);
BOOL __cdecl tech_avail(int tech_id, int faction_id);
void __cdecl tech_effects(int faction_id);
int __cdecl tech_val(int tech_id, int faction_id, BOOL simple_calc);
int __cdecl tech_ai(int faction_id);
int __cdecl tech_rate(int faction_id);

MEASURED inline LPSTR __cdecl tech_name(int tech_id, BOOL category_lvl) {
    TechName[0] = 0;
    say_tech(TechName, tech_id, category_lvl);
    return TechName;
}

MEASURED inline int __cdecl tech_mil(int tech_id) {
    // TODO: why only this one returns 0 with other three returning 1? typo/bug?
    if (tech_id < MaxTechnologyNum) {
        return Technology[tech_id].power_value;
    }
    return 0;
}

MEASURED inline int __cdecl tech_tech(int tech_id) {
    if (tech_id < MaxTechnologyNum) {
        return Technology[tech_id].tech_value;
    }
    return 1;
}

MEASURED inline int __cdecl tech_infra(int tech_id) {
    if (tech_id < MaxTechnologyNum) {
        return Technology[tech_id].wealth_value;
    }
    return 1;
}

MEASURED inline int __cdecl tech_colonize(int tech_id) {
    if (tech_id < MaxTechnologyNum) {
        return Technology[tech_id].growth_value;
    }
    return 1;
}

MEASURED inline int __cdecl tech_recurse(int tech_id, int base_lvl) {
    if (tech_id < 0 || tech_id >= MaxTechnologyNum) {
        return base_lvl;
    }
    const int next_lvl = base_lvl + 1;
    RulesTechnology *tech = &Technology[tech_id];
    int val1 = tech_recurse(tech->preq_tech_1, next_lvl);
    int val2 = tech_recurse(tech->preq_tech_2, next_lvl);
    return (val1 > val2) ? val1 : val2;
}

MEASURED inline BOOL __cdecl tech_is_preq(int preq_tech_id, int parent_tech_id, int range) {
    if (preq_tech_id < 0 || parent_tech_id < 0) {
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

// BODY IN technology.h, as `MEASURED inline` (marker stays in
// technology.cpp): `__forceinline` in the .cpp only satisfies inlining
// within that one translation unit - callers elsewhere (e.g. facility_avail
// in base.cpp) got a real out-of-line call, where the image open-codes the
// whole preq_tech walk at every one of its 109 call sites.
MEASURED inline BOOL __cdecl has_tech(int tech_id, int faction_id) {
    if (faction_id <= 0) {
        return false;
    }
    if (tech_id == TechNone) {
        return true;
    }
    if (tech_id < 0 || tech_id >= (MaxTechnologyNum - 1)) { // excludes 'Transcendent Thought'
        return false;
    }
    RulesTechnology *tech = &Technology[tech_id];
    if (tech->preq_tech_1 < TechNone
        || (tech->preq_tech_2 < TechNone && tech->preq_tech_1 != TechNone)) {
        // "none, disable" ; valid #TECH preq_tech entry
        return false;
    }
    return ((1 << faction_id) & GameTechAchieved[tech_id]) != 0;
}

// Non-inline forwarder to `has_tech` above: best_specialist (base.cpp) needs
// a real `call 0x5b9f20` the way the image emits there, unlike most of
// has_tech's other 108 call sites, which genuinely inline the whole
// preq_tech walk. The E8 target is a relocation on both sides and is
// discounted, so this symbol's own name costs nothing - same idiom as
// general.h's bitmask_call.
BOOL __cdecl has_tech_call(int tech_id, int faction_id);
