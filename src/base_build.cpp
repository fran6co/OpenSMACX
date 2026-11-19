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
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h
#include "stdafx.h"
#include "popup.h"
#include "base.h"
#include "planwin.h"
#include "council.h"
#include "alpha.h"
#include "faction.h"
#include "game.h"
#include "log.h"
#include "map.h"
#include "text.h"
#include "technology.h"
#include "veh.h"
#include "console.h"
#include "datalink.h"
#include "netdaemon.h"
#include "xpops.h"
#include "buffer.h"
#include "font.h"
#include "font_queue.h"
#include "infowin.h"
#include "maininterface.h"
#include "mapwin.h"
#include "general.h"
#include "netmsg.h"
#include "replaywin.h"
#include "messagewin.h"
#include "tutwin.h"
#include "path.h"
#include "terraforming.h"
#include "interlude.h"
#include "worldwin.h"
#include "crt_random_inline.h"

// base_claimed and base_build were one original unit after base.cpp:
// base_build calls base_compute, is_coast, worm_mod, cost_factor and
// base_lose_minerals out of line where base.cpp expands them, the
// header-inline copies only it calls follow it (0x500120..0x5002F0), and
// has_project's copy closes base.cpp's run at 0x4F80D0. The file name
// is inferred from its content, not a historical filename.

/*
 * THE FOUR BASE-SITE CLAIM POINTS the AI records for a base it is founding.
 * BaseSiteClaimX[k] / BaseSiteClaimY[k] hold the x,y of the kth site
 * base_build() picked for the base - Y starts at 999999, meaning "none yet".
 * base_reset (0x004E5FE0) fills them with the base's state bits
 * BSTATE_UNK_1000000 / _2000000 / _4000000 / _8000000 set one at a time, and
 * base_claimed (0x004F8110) tests a tile against them.
 */
// RECOVERY_OBJECT: v1 0x0090EA44 ?BaseSiteClaimX@@3PAHA
int BaseSiteClaimX[4];  // 0x0090EA44
// RECOVERY_OBJECT: v1 0x0090EA54 ?BaseSiteClaimY@@3PAHA
int BaseSiteClaimY[4];  // 0x0090EA54

// ORIGINAL: 0x004F8110 ?base_claimed@@YAHHHHHHH@Z 0x004F8110-0x004F8195 FILE BYTE_EXACT
// LEVER: the scaffold's eight carried bindings are two four-int runs - the
//        base-site claim points base_reset writes and this function tests - so
//        they became the arrays BaseSiteClaimX[4] / BaseSiteClaimY[4] rather
//        than eight address-named scalars.
// LEVER: the six `aN` parameters are the tile and its four site-freedom flags;
//        naming them cleared the last scaffold names this body carried.
// size      133 bytes
// prototype int (__cdecl ?base_claimed@@YAHHHHHHH@Z)(int x, int y, int site1_free, int site2_free, int site3_free, int site4_free)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// Purpose: Is this tile one of the four sites the AI has claimed for a base?
//          A site counts only while its flag is still clear and only at or
//          below the recorded y - the AI claims southward from the site.
// Status:  transcription homed from src/unrecovered/004f8110.cpp

int __cdecl base_claimed(int x, int y, int site1_free, int site2_free,
                         int site3_free, int site4_free) {
    if (site1_free == 0 && x == BaseSiteClaimX[0] && y >= BaseSiteClaimY[0]) {
        return 1;
    }
    if (site2_free == 0 && x == BaseSiteClaimX[1] && y >= BaseSiteClaimY[1]) {
        return 1;
    }
    if (site3_free == 0 && x == BaseSiteClaimX[2] && y >= BaseSiteClaimY[2]) {
        return 1;
    }
    if (site4_free == 0 && x == BaseSiteClaimX[3] && y >= BaseSiteClaimY[3]) {
        return 1;
    }
    return 0;
}

// WORKAROUND(inline): dead budget probe (folds to no code). Calls ahead of
// the probe-team tech checks spend what the image spent before its late refusals:
// range at the weapon/armour and morale clamps, base_lose_minerals and base_claimed
// stay calls as in the image. Counted with base_compute, is_coast and worm_mod as
// plain calls in this unit and base.h's one-level has_fac. Since the game units
// dropped /Oi (2026-11-18) ten were too many: seven stayed out-of-line calls of
// this probe. Five: the range at 0x4FD528 is a call as in the image and one
// probe still is (four: no probe call, but that range expands); the frame
// (0x1F6C) matches at every count from 4 to 10. Branchy probes plus pure-store
// probes (bb_probe_store, cheaper than a branch) spend the amount in between: no
// probe call and the image's call multiset. 2026-11-19: three + two; after the
// 2026-11-19 integration the range at 0x4FD528 expanded again,
// and four + one (or five + none) is the window again. With the secret-project
// ternary (a_130[j] = l_84 ? 5 : -l_d8) only five + none keeps the multiset.
// After is_port became a header inline five leave one probe call and four
// expand that range again; three one-test probes and one two-test probe
// (bb_probe2) give the image's call multiset with no probe call.
static inline int bb_probe(int x) {
    if (x == 1) {
        x = 2;
    }
    return x;
}
static inline int bb_probe2(int x) {
    if (x == 1) {
        x = 2;
    }
    if (x == 3) {
        x = 4;
    }
    return x;
}

// Complete base_build was moved here from its retired seed:
// src/unrecovered/004f81a0.cpp. Its original findings remain below.
// The independent base_change seed remains retained at
// src/recovered/units/004e5a60.cpp.
// ORIGINAL: 0x004F81A0 ?base_build@@YAHHPAH00@Z 0x004F81A0-0x004FFFB8
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004f81a0.cpp", "source:src/base.cpp:base_reset", "source:src/unrecovered/004f81a0.cpp: complete original identity/span and historical seed findings preserved", "Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original32280-byte span and68-selector/36-target facility dispatcher", "build/recovery/base-build-owner-resume-oct6/review-receipt-oct6.md: bounded independent review, not exhaustive or native acceptance", "recovery/tests/test_base_build_owner.py: actual-source bounded host/damage controls", "BaseSecretProject true64-word storage; historical scalar-member bindings remain unresolved primitive-array accessor interpretations"], "reason": "Recover complete build selection through real Base, faction, map, prototype, facility and project owners, preserving original unchecked indexes, arithmetic, lifecycle and output contracts. Prior source decision retained as history: Explicitly unfinished Base-owned dependency required to link the recovered base_reset caller; full 72-arm build-selection behavior remains in its seed"}
// TRIED: mismatch #3 (call vs test) - this is the largest function in the batch (32280 bytes, a 72-entry switch reached via the jump table at 0x4FA56E documented in the brief, one arm per buildable unit/facility/project id) preceded by ~0x360 bytes of reactor/rules-bit eligibility computation and several 0x200/0x36-int array-zeroing blocks. Only the entry parameter normalization (a1<0 fold) plus set_base()/base_compute(1) are transcribed; the switch and its scoring tables are not modelled - the body returns the same -1/"no candidate" sentinel the original's accumulator starts in, rather than fabricate any of the 72 per-item eligibility rules.
int __cdecl base_build(int base_id, int *out_secondary, int *out_tertiary, int *out_score) {
    // WORKAROUND(inline): the image calls range (0x00422F00) and has_project
    // (0x004F80D0) out of line at these sites of base_build.
    int (__cdecl *const range_call)(int, int, int) = range;
    BOOL (__cdecl *const project_call)(int, int) = has_project;
    int sat_type;  // WORKAROUND(decl): base_build 0x004FED4D forms faction*0x833 + (type - 0x41) before scaling; only a local subscript keeps the -0x41 out of the displacement.
    int a_1f6c[512];
    int a_176c[512];
    int a_f6c[512];
    int a_76c[54];
    int a_694[64];
    int a_594[54];
    int a_4bc[54];
    int a_3e4[8];
    int a_3c4[54];
    int a_2ec[54];
    int a_1ec[3];
    int a_1dc[3];
    int a_130[3];
    int a_114[3];
    int l_164;
    int l_160;
    int l_15c;
    int l_158;
    int l_154;
    int l_150;
    int l_14c;
    int l_148;
    int l_144;
    int l_140;
    int l_13c;
    int l_138;
    int l_134;
    int l_124;
    int l_120;
    int l_11c;
    int l_118;
    int l_108;
    int l_104;
    int l_100;
    int l_fc;
    int l_f8;
    int l_f4;
    int l_f0;
    int l_ec;
    int l_e8;
    int l_e4;
    int l_e0;
    int l_dc;
    int l_d8;
    int l_d4;
    int l_d0;
    int l_cc;
    int l_c8;
    int l_c4;
    int l_c0;
    int l_bc;
    int l_b8;
    int l_b4;
    int l_b0;
    int l_ac;
    int l_a8;
    int l_a4;
    int l_a0;
    int l_9c;
    int l_98;
    int l_94;
    int l_90;
    int l_8c;
    int l_88;
    int l_84;
    int l_80;
    int l_7c;
    int l_78;
    int l_74;
    int l_70;
    int l_6c;
    int l_68;
    int l_64;
    int l_60;
    int l_5c;
    int l_58;
    int l_54;
    int l_50;
    int l_4c;
    int l_48;
    int l_44;
    int l_40;
    int l_3c;
    int l_38;
    int l_34;
    int l_30;
    int l_2c;
    int l_28;
    int l_24;
    int l_20;
    int l_1c;
    int l_18;
    int l_14;
    int l_10;
    int l_c;
    int l_8;
    int i, j;
    uint32_t gov;
    uint8_t b_50;
    int w, expanding, weight, value, bonus;
    // ---- chunk 0: 0x4F81A0-0x4F9C43 ----
    // Negative base ids select the base without the governor's priorities.
    if (base_id < 0) {
        l_108 = 1;
        base_id = (base_id + 1 >= 0) ? base_id + 1 : -1 - base_id;
    } else {
        l_108 = 0;
    }
    set_base(base_id);
    base_compute(1);
    l_54 = Bases[base_id].x;
    l_8c = Bases[base_id].y;
    l_8 = Bases[base_id].faction_id_current;
    l_38 = is_ocean(l_54, l_8c);
    l_10 = (&map_tiles()[(l_54 >> 1) + l_8c * MapLongitude])->region;
    l_c = PlayersData[l_8].region_base_plan[l_10];
    l_100 = best_reactor(l_8);
    l_1c = 1 << l_8;

    // AI priorities, or the governor's priorities for a human base under governor control.
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
        && ((gov = BaseCurrent->governor) & 0x80000000)) {
        if (!(gov & 0xF200000) && BaseIDCurrentSelected >= 8) {
            if (PlayersData[l_8].region_base_plan[l_10] <= 2) {
                gov |= 0x8000000;
            }
            if (PlayersData[l_8].region_base_plan[l_10] == 8) {
                gov |= 0x1000000;
            }
            if (PlayersData[l_8].region_base_plan[l_10] == 7) {
                gov |= 0x2000000;
            }
            gov |= 0x1000000 << (BaseIDCurrentSelected & 3);
        }
        l_a4 = (gov >> 24) & 1;
        l_58 = (gov >> 25) & 1;
        l_98 = (gov >> 26) & 1;
        l_60 = (gov >> 27) & 1;
        if (l_60) {
            l_7c = 1;
        } else if (l_a4) {
            l_7c = 0;
        } else if (!l_58 && !l_98) {
            l_7c = PlayersData[l_8].ai_fight;
        } else {
            l_7c = -1;
        }
        l_64 = l_a4;
        l_3c = l_58;
        l_40 = l_98;
        l_30 = l_60;
        if (l_a4 && l_58 && l_98 && l_60) {
            l_64 = l_3c = l_40 = l_30 = 0;
        }
    } else {
        l_a4 = PlayersData[l_8].ai_growth;
        l_58 = PlayersData[l_8].ai_tech;
        l_98 = PlayersData[l_8].ai_wealth;
        l_60 = PlayersData[l_8].ai_power;
        l_7c = PlayersData[l_8].ai_fight;
        l_64 = l_3c = l_40 = l_30 = 0;
    }
    l_fc = 0;
    for (i = 0; i < 128; i++) {
        if (PlayersData[l_8].region_total_bases[i]) {
            l_fc++;
        }
    }
    for (int z_a_f6c = 0; z_a_f6c < 512; z_a_f6c++) {
        a_f6c[z_a_f6c] = 0;
    }
    for (int z_a_1f6c = 0; z_a_1f6c < 512; z_a_1f6c++) {
        a_1f6c[z_a_1f6c] = 0;
    }
    for (int z_a_176c = 0; z_a_176c < 512; z_a_176c++) {
        a_176c[z_a_176c] = 0;
    }
    for (int z_a_4bc = 0; z_a_4bc < 54; z_a_4bc++) {
        a_4bc[z_a_4bc] = 0;
    }
    for (int z_a_2ec = 0; z_a_2ec < 54; z_a_2ec++) {
        a_2ec[z_a_2ec] = 0;
    }
    for (int z_a_594 = 0; z_a_594 < 54; z_a_594++) {
        a_594[z_a_594] = 0;
    }
    for (int z_a_3c4 = 0; z_a_3c4 < 54; z_a_3c4++) {
        a_3c4[z_a_3c4] = 0;
    }
    a_1dc[2] = 0;
    a_1dc[0] = 0;
    a_1dc[1] = 0;
    for (int z_a_76c = 0; z_a_76c < 54; z_a_76c++) {
        a_76c[z_a_76c] = 0;
    }
    l_14c = 0;
    l_134 = 0;
    l_11c = 0;
    l_e4 = 0;
    l_bc = 0;
    l_118 = 0;
    l_ac = 0;
    l_ec = 0;
    l_104 = 0;
    l_124 = has_fac_built(FAC_SKUNKWORKS, BaseIDCurrentSelected);
    l_a0 = 0;
    l_120 = 0;
    l_44 = 0;
    l_dc = 0;
    l_d4 = 0;
    l_158 = 0;
    l_144 = 0;
    l_b4 = 0;
    l_68 = 0;
    l_154 = 0;
    l_a8 = 0;
    l_cc = 0;
    l_140 = 0;
    if (!l_c) {
        l_70 = 1;
    } else {
        l_70 = 0;
        if (l_c == 2) {
            l_70 = 1;
        }
    }

    // Pact partner (a human one preferred).
    l_94 = -1;
    for (i = 1; i < 8; i++) {
        if (l_8 != i && (PlayersData[l_8].diplo_treaties[i] & 1)
            && (is_human(i) || l_94 < 0)) {
            l_94 = i;
        }
    }

    // Vendettas: threat, enemies in this region / elsewhere, bases seen by enemies.
    l_13c = -1;
    l_f8 = -1;
    l_f4 = 0;
    for (i = 1; i < 8; i++) {
        if (l_8 == i) {
            continue;
        }
        base_id = PlayersData[l_8].diplo_treaties[i];  // param slot reused: treaty flags
        l_9c = base_id & 0x10;
        if (l_9c || (is_human(i) && (base_id & 8) && !(base_id & 1))) {
            for (j = 0; j < GameDataStateStorage.fields_.base_current_count_; j++) {
                if (Bases[j].faction_id_current == l_8
                    && (Bases[j].faction_id_current == i || (Bases[j].visibility & (1 << i)))) {
                    l_f4 = 1;
                    break;
                }
            }
        }
        if (!l_9c) {
            continue;
        }
        if (l_c == 7) {
            l_cc = 1;
        }
        if ((l_94 >= 0 && (PlayersData[l_94].diplo_treaties[i] & 0x10))
            || (is_human(i) && (GameDataStateStorage.fields_.game_rules_ & 0x40))
            || ((base_id & 0x20) && PlayersData[i].diff_level >= 3)
            || (base_id & 0x40)
            || (PlayersData[i].ranking > PlayersData[l_8].ranking && l_7c >= 0)) {
            if (l_c == 7) {
                l_70 = 1;
            }
            if (!PlayersData[i].region_total_bases[l_10]) {
                if (l_c == 7) {
                    l_f8 = i;
                }
            } else {
                l_13c = i;
            }
        }
    }
    if (l_c == 7 && l_cc && (GameDataStateStorage.fields_.game_state_ & 0x200)) {
        l_70 = 2;
    }

    // Attack target in this region (2: this base's own square).
    l_138 = 0;
    if (PlayersData[l_8].x_target >= 0) {
        if (region_at(PlayersData[l_8].x_target, PlayersData[l_8].y_target) == l_10) {
            l_138 = 1;
        }
        if (l_54 == PlayersData[l_8].x_target && l_8c == PlayersData[l_8].y_target) {
            l_138 = 2;
        }
    }

    // Colony sites in this region not claimed by another faction.
    l_150 = 0;
    for (l_94 = 0; l_94 < 25; l_94++) {
        if (PlayersData[l_8].sites[l_94].type == 3 && PlayersData[l_8].sites[l_94].priority > 0
            && region_at(PlayersData[l_8].sites[l_94].x, PlayersData[l_8].sites[l_94].y) == l_10) {
            int owner = whose_territory(l_8, PlayersData[l_8].sites[l_94].x,
                                        PlayersData[l_8].sites[l_94].y, NULL, 0);
            if (owner < 0 || owner == l_8) {
                l_150++;
            }
        }
    }

    // Need for psych facilities.
    l_74 = 0;
    if (BaseCurrent->drone_total) {
        if (BaseCurrent->drone_total >= BaseCurrent->talent_total) {
            l_74 = 1;
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            || BaseCurrent->minerals_accumulated <= Rules.retool_exemption) {
            if ((BaseCurrent->state & 2)
                || (l_74 && BaseCurrent->specialist_total - BaseCurrent->specialist_unk_1 > 1)) {
                l_74++;
            }
        }
    }
    base_id = BaseCurrent->mineral_surplus;  // param slot reused: mineral surplus
    l_e0 = BaseCurrent->intake_2[RSC_MINERALS] + base_id * 2;
    if (out_score) {
        if (BaseCurrent->production_id_last <= -70
            && BaseCurrent->minerals_accumulated > Rules.retool_exemption) {
            l_b0 = 1;
        } else {
            l_b0 = 0;
        }
    } else {
        l_b0 = BaseCurrent->queue_production_id[0] <= -70;
    }

    // Secret projects: those of this base, and of the faction per group of eight.
    for (int z_a_3e4 = 0; z_a_3e4 < 8; z_a_3e4++) {
        a_3e4[z_a_3e4] = 0;
    }
    for (int z_a_694 = 0; z_a_694 < 64; z_a_694++) {
        a_694[z_a_694] = 0;
    }
    l_148 = 0;
    for (i = 0; i < 64; i++) {
        if (GameDataStateStorage.fields_.secret_project_.project_base[i] == BaseIDCurrentSelected) {
            l_148++;
        }
        if (has_project(i, l_8)) {
            a_3e4[i / 8]++;
        }
    }
    if (l_b0 && BaseCurrent->nutrient_surplus > 0 && base_id > 0
        && BaseCurrent->specialist_total - BaseCurrent->specialist_unk_1
               <= BaseCurrent->population_size / 4 + 1) {
        l_74 = 1;
    }

    // All bases: targets, production, facilities, distances and garrisons.
    l_15c = 24;
    l_6c = 0;
    l_164 = 24;
    l_d0 = 0;
    l_160 = 0;
    l_c4 = 0;
    l_d8 = 0;
    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        // param slot reused: base loop index
        int owner = Bases[base_id].faction_id_current;
        if (l_8 != owner) {
            if (PlayersData[l_8].base_id_atk_target == base_id
                && (PlayersData[l_8].diplo_treaties[owner] & 0x10)) {
                if (region_at(Bases[base_id].x, Bases[base_id].y) == l_10) {
                    l_104 |= 2;
                } else if (l_c == 7) {
                    l_104 |= 1;
                }
            }
            if (Bases[base_id].queue_production_id[0] == -FAC_ASCENT_TO_TRANSCENDENCE) {
                l_68++;
                if (is_human(owner)) {
                    l_154++;
                }
            }
            if (Bases[base_id].queue_production_id[0] == BaseCurrent->queue_production_id[0]) {
                l_44++;
            }
            if (Bases[base_id].queue_production_id[0] <= -70) {
                a_694[-Bases[base_id].queue_production_id[0] - 70] |= 1 << owner;
            }
            if (is_human(owner) && Bases[base_id].faction_id_former == l_8
                && region_at(Bases[base_id].x, Bases[base_id].y) == l_10
                && !(PlayersData[l_8].diplo_treaties[owner] & 1)) {
                l_a0++;
                if (l_a0 > 1 - l_7c) {
                    l_70 = 1;
                }
            }
            continue;
        }
        if (Bases[base_id].population_size > l_d8) {
            l_d8 = Bases[base_id].population_size;
        }
        if (is_ocean(Bases[base_id].x, Bases[base_id].y)) {
            l_a8++;
        }
        int dist = vector_dist(l_54, l_8c, Bases[base_id].x, Bases[base_id].y);
        // has_fac(.., 0): the image keeps each inline's materialized is_built store.
        if (has_fac(FAC_NAVAL_YARD, base_id, 0) && dist < l_15c) {
            l_15c = dist;
        }
        if ((has_fac(FAC_AEROSPACE_COMPLEX, base_id, 0)
             || has_project(SP_CLOUDBASE_ACADEMY, Bases[base_id].faction_id_current))
            && dist < l_164) {
            l_164 = dist;
        }
        if (has_fac(FAC_PSI_GATE, base_id, 0)) {
            l_b4++;
        }
        if (has_fac(FAC_PUNISHMENT_SPHERE, base_id, 0)) {
            l_140++;
        }
        if (has_fac(FAC_HEADQUARTERS, base_id, 0)
            || Bases[base_id].queue_production_id[0] == -FAC_HEADQUARTERS) {
            l_14c++;
        }
        if (base_id == BaseIDCurrentSelected) {
            continue;
        }
        if (Bases[base_id].state & 0x100000) {
            l_d0++;
        }
        if (Bases[base_id].population_size > l_144) {
            l_144 = Bases[base_id].population_size;
        }
        if (Bases[base_id].queue_production_id[0] == BaseCurrent->queue_production_id[0]) {
            l_120++;
            if (BaseCurrent->queue_production_id[0] <= -70) {
                if (BaseCurrent->mineral_surplus_final <= 0) {
                    if (Bases[base_id].mineral_surplus_final > 0) {
                        l_dc++;
                    }
                } else if (Bases[base_id].mineral_surplus_final > 0) {
                    // The image indexes Facility before the cost_factor call and
                    // divides for this base first.
                    int cost = Facility[-BaseCurrent->queue_production_id[0]].cost
                        * cost_factor(BaseCurrent->faction_id_current, 1, -1);
                    if ((cost - Bases[base_id].minerals_accumulated)
                            / Bases[base_id].mineral_surplus_final
                        < (cost - BaseCurrent->minerals_accumulated)
                              / BaseCurrent->mineral_surplus_final) {
                        l_dc++;
                    }
                }
            }
        }
        int item = Bases[base_id].queue_production_id[0];
        if (item <= -70) {
            a_694[-item - 70] |= l_1c;
        }
        // BUG(original): for facility items (item < 0) plan/triad are read from
        // VehPrototypes[item] (before the table) and has_abil(item, 0x8000) may be called.
        int plan = VehPrototypes[item].plan;
        l_20 = Chassis[VehPrototypes[item].chassis_id].triad;
        if (plan == 7
            && (VehPrototypes[item].carry_capacity < 2
                || VehPrototypes[item].carry_capacity < PlayersData[l_8].unk_70 / 0x400000)) {
            plan = 15;
        }
        if (!l_20 && has_abil(item, 0x8000)) {
            plan = 16;
        }
        if (item >= 0) {
            a_3c4[l_20 + plan * 3] += (VehPrototypes[item].reactor_id >= l_100) + 1;
            a_1f6c[item]++;
        }
        if (l_10 != region_at(Bases[base_id].x, Bases[base_id].y)) {
            continue;
        }
        if (Bases[base_id].state & 1) {
            l_6c = 1;
        }
        if (Bases[base_id].queue_production_id[0] <= -70) {
            l_d4++;
        }
        if (has_fac(FAC_COMMAND_CENTER, base_id, 0)
            || Bases[base_id].queue_production_id[0] == -FAC_COMMAND_CENTER) {
            l_11c++;
        }
        if (has_fac(FAC_BIOENHANCEMENT_CENTER, base_id, 0)
            || Bases[base_id].queue_production_id[0] == -FAC_BIOENHANCEMENT_CENTER) {
            l_134++;
        }
        if (has_fac(FAC_NAVAL_YARD, base_id, 0)
            || Bases[base_id].queue_production_id[0] == -FAC_NAVAL_YARD) {
            l_118++;
        }
        if (has_fac(FAC_AEROSPACE_COMPLEX, base_id, 0)
            || Bases[base_id].queue_production_id[0] == -FAC_AEROSPACE_COMPLEX) {
            l_ac++;
        }
        if (has_fac(FAC_SKUNKWORKS, base_id, 0)
            || Bases[base_id].queue_production_id[0] == -FAC_SKUNKWORKS) {
            l_ec++;
        }
        if (has_fac(FAC_CENTAURI_PRESERVE, base_id, 0)) {
            l_bc++;
        }
        if (has_fac(FAC_TEMPLE_OF_PLANET, base_id, 0)) {
            l_e4++;  // BUG(original)?: l_e4 is reset to 0 after this loop; the count is dead
        }
        if (Bases[base_id].queue_production_id[0] >= 0) {
            a_f6c[Bases[base_id].queue_production_id[0]]++;
            a_594[l_20 + plan * 3] += (VehPrototypes[item].reactor_id >= l_100) + 1;
            a_1dc[l_20]++;
            if (Bases[base_id].queue_production_id[0] >= 64
                && !(VehPrototypes[Bases[base_id].queue_production_id[0]].flags & 4)) {
                l_158++;
            }
        }
        // Garrison of the region's other bases (when this base has a command center
        // and the other one does not).
        if (!has_fac(FAC_COMMAND_CENTER, BaseIDCurrentSelected, 0)) {
            continue;
        }
        if (has_fac(FAC_COMMAND_CENTER, base_id, 0)) {
            continue;
        }
        int veh = stack_fix(veh_at(Bases[base_id].x, Bases[base_id].y));
        l_f0 = garrison_check(base_id);
        l_c8 = defensive_check(base_id);
        l_5c = stack_check(veh, 11, 0x10000000, 0x10000000, l_8);
        int def;
        int t = stack_check(veh, 2, 2, l_8, -1);
        if (t >= 0 && l_c8 >= 0) {
            def = min(l_c8, t);  // the image takes the result and its l_c8 arm in different registers
            if (def && l_5c) {
                def++;
            }
        } else {
            def = 0;
        }
        if (PlayersData[l_8].region_base_plan[l_10] == 2
            || PlayersData[l_8].region_base_plan[l_10] == 8) {
            def += stack_check(veh, 2, 1, l_8, -1);
        }
        if (PlayersData[l_8].best_armor_value < 2) {
            def += stack_check(veh, 2, 3, -1, -1);
        } else if (!def) {
            def = stack_check(veh, 2, 3, -1, -1) != 0;
        }
        l_94 = 0;
        for (j = 0; j < GameDataStateStorage.fields_.vehicle_current_count_; j++) {
            if (Vehs[j].faction_id == l_8
                && (VehPrototypes[Vehs[j].proto_id].plan == 2 || (Vehs[j].state & 0x10000000))
                && region_at(Vehs[j].x, Vehs[j].y) == l_10
                && base_who(Vehs[j].x, Vehs[j].y) < 0
                && (Vehs[j].home_base_id == base_id
                    || Vehs[j].home_base_id == BaseIDCurrentSelected)) {
                if (Vehs[j].home_base_id == BaseIDCurrentSelected) {
                    l_94++;
                    if (l_94 > l_c4) {
                        continue;
                    }
                    l_c4++;
                }
                def++;
            }
        }
        if (def < l_f0) {
            // The image loads the production item once, before the l_5c test.
            int prod = Bases[base_id].queue_production_id[0];
            if (!(!l_5c && l_f0 != l_c8)) {
                if (prod >= 0 && VehPrototypes[prod].plan == 2 && ++def >= l_f0) {
                    continue;
                }
                l_160++;
            } else {
                if (prod >= 0 && VehPrototypes[prod].plan <= 1 && ++def >= l_f0) {
                    continue;
                }
                l_160++;
            }
        }
    }
    if (BaseCurrent->state & 1) {
        l_6c = 2;
    }

    // The faction's vehicles per plan/triad: everywhere, homed here, in this region.
    l_68 = 0;
    l_e4 = 0;
    l_e8 = 0;
    for (l_4c = 0; l_4c < GameDataStateStorage.fields_.vehicle_current_count_; l_4c++) {
        if (Vehs[l_4c].faction_id != l_8) {
            continue;
        }
        l_68++;
        l_20 = Chassis[VehPrototypes[Vehs[l_4c].proto_id].chassis_id].triad;
        int plan = VehPrototypes[Vehs[l_4c].proto_id].plan;
        if (plan == 7
            && (veh_cargo(l_4c) < 2 || veh_cargo(l_4c) < PlayersData[l_8].unk_70 / 0x400000)) {
            plan = 15;
        }
        if (!l_20 && has_abil(Vehs[l_4c].proto_id, 0x8000)) {
            plan = 16;
        }
        if (VehPrototypes[Vehs[l_4c].proto_id].unk_1 == 0x15) {
            plan = 17;
        }
        if (Vehs[l_4c].state & 0x10000000) {
            plan = 2;
        }
        a_2ec[l_20 + plan * 3] += (VehPrototypes[Vehs[l_4c].proto_id].reactor_id >= l_100) + 1;
        if (Vehs[l_4c].home_base_id == BaseIDCurrentSelected) {
            a_76c[l_20 + plan * 3]++;
            if (VehPrototypes[Vehs[l_4c].proto_id].plan == 3
                && has_abil(Vehs[l_4c].proto_id, 0x40000)
                && l_10 == region_at(Vehs[l_4c].x, Vehs[l_4c].y)) {
                l_e4++;
            }
        }
        if (l_10 == region_at(Vehs[l_4c].x, Vehs[l_4c].y)) {
            a_176c[Vehs[l_4c].proto_id]++;
            a_4bc[l_20 + plan * 3] += (VehPrototypes[Vehs[l_4c].proto_id].reactor_id >= l_100) + 1;
            if (base_who(Vehs[l_4c].x, Vehs[l_4c].y) < 0 && plan <= 3) {
                l_e8++;
            }
        }
    }
    // Halve the statistics of plans 0-16 (plan 17 is left alone).
    for (i = 0; i < 17; i++) {
        for (j = 0; j < 3; j++) {
            a_3c4[j + i * 3] /= 2;
            a_594[j + i * 3] /= 2;
            a_2ec[j + i * 3] /= 2;
            a_4bc[j + i * 3] /= 2;
        }
    }

    // Naval goal.
    l_c4 = 0;
    for (i = 0; i < 64; i++) {
        if (PlayersData[l_8].region_total_bases[i] >= 3) {
            l_c4++;
        }
    }
    if (PlayersData[l_8].current_num_bases >= 10) {
        l_c4++;
    }
    if (PlayersData[l_8].current_num_bases >= 20) {
        l_c4++;
    }
    l_c4 += (l_a8 + 2) / 3;
    if (has_project(SP_MARITIME_CONTROL_CENTER, l_8)) {
        l_c4 = l_c4 * 2 + 1;
    }
    // The image loads the police rating before the brood pit bit: it is the right operand.
    if (has_fac_built(FAC_BROOD_PIT, BaseIDCurrentSelected) * 2
            + PlayersData[l_8].soc_effect_pending.values_[SocialEffect::POLICE] <= -3) {
        l_c4 = (l_c4 + 1) / 2;
    }
    if (has_fac_built(FAC_BROOD_PIT, BaseIDCurrentSelected) * 2
            + PlayersData[l_8].soc_effect_pending.values_[SocialEffect::POLICE] <= -5) {
        l_c4 = l_c4 / 2;
    }

    // Base radius: enemy units nearby and farmable special tiles.
    l_94 = 0;
    l_84 = 0;
    for (i = 0; i < 21; i++) {
        int x = wrap_map_x(l_54 + RadiusOffsetX[i]);
        int y = RadiusOffsetY[i] + l_8c;
        if (on_map(x, y)) {
            if (map_loc(x, y)->bit & 0x200000) {
                l_84 += ((1 << i) & BaseCurrent->resource_sqr_radius) ? 2 : 1;
            }
            int v = veh_at(x, y);
            if (v >= 0 && Vehs[v].faction_id != l_8
                && (PlayersData[l_8].diplo_treaties[Vehs[v].faction_id] & 0x30)) {
                l_94 += stack_check(v, 1, -1, -1, -1);
            }
        }
    }

    // Defenders of this base.
    l_4c = stack_fix(veh_at(BaseCurrent->x, BaseCurrent->y));
    l_f0 = garrison_check(BaseIDCurrentSelected);
    l_c8 = defensive_check(BaseIDCurrentSelected);
    l_5c = stack_check(l_4c, 11, 0x10000000, 0x10000000, l_8);
    {
        int t = stack_check(l_4c, 2, 2, l_8, -1);
        if (t >= 0 && l_c8 >= 0) {
            l_88 = (t > l_c8) ? l_c8 : t;  // one store: the image computes it in a register
            if (l_88 && l_5c) {
                l_88++;
            }
        } else {
            l_88 = 0;
        }
    }
    if ((BaseCurrent->state & 0x4000000) && PlayersData[l_8].region_base_plan[l_10] > 2) {
        l_88++;
    }
    if (PlayersData[l_8].region_base_plan[l_10] == 2
        || PlayersData[l_8].region_base_plan[l_10] == 8) {
        l_88 += stack_check(l_4c, 2, 1, l_8, -1);
    }
    if (PlayersData[l_8].best_armor_value < 2) {
        l_88 += stack_check(l_4c, 2, 3, -1, -1);
    } else if (!l_88) {
        l_88 = stack_check(l_4c, 2, 3, -1, -1) != 0;
    }
    stack_check(l_4c, 2, 7, l_8, -1);
    stack_check(l_4c, 2, 6, l_8, -1);
    if (l_1c & VehPrototypes[2].obsolete_factions) {
        l_88 -= range(stack_check(l_4c, 0, 2, l_8, -1), 0, 1);
    }
    if (PlayersData[l_8].region_base_plan[l_10] > 2 && (BaseCurrent->state & 0x8000000)) {
        l_88++;
    }

    // Need for defense (0-3).
    // One store after the chain; the image keeps the value in a register for the
    // governor test below.
    l_28 = (l_88 < 1) ? 3 : (l_88 < l_f0 - 1) ? 2 : (l_88 < l_f0);
    l_9c = GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c;
    if (l_9c && BaseCurrent->minerals_accumulated > Rules.retool_exemption
        && !(BaseCurrent->state & 8)) {
        if (!l_64 && !l_3c && !l_40) {
            if (l_30) {
                if (l_28 > 1) {
                    l_28 = 1;
                }
                goto L_4f9e89;
            }
        } else {
            if (l_30 || l_64) {
                if (l_28 > 1) {
                    l_28 = 1;
                }
            } else {
                l_28 = 0;
            }
            goto L_4f9e89;
        }
    }
    // falls through to chunk 1 at 0x4F9C43
    // ---- chunk 1: 0x4f9c43-0x4fb622 ----
L_4f9c43:
    // Normal need: lower it when the region looks safe.
    if (l_c == 8
        || (l_c == 7 && (PlayersData[l_8].current_num_bases < 4 || !l_f4))
        || (l_9c && !l_30)) {
        w = 1;  // no (possibly) hostile units or bases in the region
        for (i = 1; i < 8; i++) {
            if (l_8 != i
                && (PlayersData[i].region_total_combat_vehs[l_10]
                    || PlayersData[i].region_total_bases[l_10])) {
                if ((PlayersData[l_8].diplo_treaties[i] & 0x30)
                    || ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << i))
                        && (PlayersData[i].diplo_treaties[l_8] & 8)
                        && (!(PlayersData[l_8].diplo_treaties[i] & 2)
                            || PlayersData[i].integrity_blemishes))) {
                    w = 0;
                }
            }
        }
        if (l_28 && w
            && ((l_9c && !l_30)
                || (BaseCurrent->population_size < 3
                    && (l_a4 || PlayersData[l_8].current_num_bases < 3)
                    && !(BaseCurrent->state & 0x1000000)))) {
            l_28--;
            l_48 = BaseCurrent->y;
            l_9c = BaseCurrent->x;
            for (i = 0; i < 121; i++) {
                int x = wrap_map_x(RadiusOffsetX[i] + l_9c);
                int y = RadiusOffsetY[i] + l_48;
                if (on_map(x, y)) {
                    Map *sq = map_loc(x, y);
                    if (sq->region == l_10
                        && ((PlayersData[l_8].flags & 0x200) || (sq->visibility & l_1c))
                        && (sq->bit & 3)) {
                        int owner = sq->val2 & 0xf;
                        if (owner < 8 && owner >= 0 && owner != l_8
                            && (!owner || (PlayersData[l_8].diplo_treaties[owner] & 0x30))) {
                            l_28++;
                            break;
                        }
                    }
                }
            }
        }
    }
L_4f9e89:
    // Frontier: unexplored or foreign (unfriendly) tiles in the base radius.
    l_c0 = 0;
    l_44 = 0;
    do {
        int x = wrap_map_x(RadiusOffsetX[l_44] + BaseCurrent->x);
        int y = RadiusOffsetY[l_44] + BaseCurrent->y;
        if (on_map(x, y)) {
            Map *sq = map_loc(x, y);
            if (sq->region == l_10) {
                if (!(PlayersData[l_8].flags & 0x200) && !(sq->visibility & l_1c)) {
                    l_c0 = 1;
                    break;
                }
                if (sq->bit & 3) {
                    int owner = sq->val2 & 0xf;
                    if (owner < 8 && owner > 0 && owner != l_8) {
                        l_c0 = 1;
                        break;
                    }
                }
                int territory = whose_territory(l_8, x, y, NULL, 0);
                if (territory > 0 && territory != l_8) {
                    if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << territory)) {
                        if (!(PlayersData[l_8].diplo_treaties[territory] & 1)) {
                            l_c0 = 1;
                            break;
                        }
                    } else if (!(PlayersData[l_8].diplo_treaties[territory] & 2)) {
                        l_c0 = 1;
                        break;
                    }
                }
            }
        }
    } while (++l_44 < 49);
    // Best choices: a_114 scores, a_130 ids.
    a_114[2] = 999999;
    a_114[0] = 999999;
    a_114[1] = 999999;
    for (i = 0; i < 3; i++) {
        a_130[i] = i ? 99999 : -69;
    }
    if (PlayersData[l_8].total_mil_veh >= 512) goto L_4fd908;
    l_9c = 999999;
    l_14 = 0;
L_4fa03d:
    // Unit loop body (l_14 = prototype id); the increment/test is at 0x4fd8f6 (chunk 3).
    if (!veh_avail(l_14, l_8, BaseIDCurrentSelected)) goto L_4fd8f6;
    l_20 = Chassis[VehPrototypes[l_14].chassis_id].triad;
    l_2c = VehPrototypes[l_14].plan;
    if (!l_20 && has_abil(l_14, 0x8000)) {
        l_2c = 16;
    }
    l_78 = 0;
    l_90 = VehPrototypes[l_14].plan < 8;
    w = (VehPrototypes[l_14].cost + 2) << 8;
    // Triad switch. Case source order is sea, land, air: air is the dispatch
    // fall-through laid first, sea follows it and falls into the join at 0x4fa54b,
    // land is deferred past the plan dispatch (0x4fa5a9). The same order fits the
    // image's range inlining (sea and land expanded, air's later sites called).
    switch (l_20) {
    case 1: {
        if (l_c != 7 && !l_38
            && PlayersData[l_8].region_total_combat_vehs[l_10]
                   < PlayersData[l_8].region_total_bases[l_10] * 2 + 2) {
            goto L_4fd8f6;
        }
        if (!is_port(BaseIDCurrentSelected, 0)) {
            goto L_4fd8f6;
        }
        int coast = base_coast(BaseIDCurrentSelected);
        if (bad_reg(coast)) goto L_4fd8f6;
        if (sea_coasts(coast) < 2 && !l_38
            && (l_2c == 7 || Continents[coast].tile_count < 48)) {
            goto L_4fd8f6;
        }
        l_18 = a_2ec[l_2c * 3 + 1] + a_3c4[l_2c * 3 + 1] * 2;
        if (l_c != 7 && !l_38) {
            if (l_c == 2) l_18++;
            if (PlayersData[l_8].unk_77[l_10] & 0x400) l_18++;
            l_18 += a_1dc[1];
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && (PlayersData[l_8].flags & 0x8000000)) {
            w /= 2;
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && (Players[l_8].rule_flags & 0x100)) {
            w /= 2;
            l_fc++;  // BUG(original): persists for the following units
        }
        w = range(8 - l_fc, 4, 8) * w / 8;
        int minerals = range(BaseCurrent->mineral_surplus, 1, 999);
        w = range(veh_cost(l_14, BaseIDCurrentSelected, NULL) * cost_factor(l_8, 1, -1) * w
                         / (minerals * 15),
                     w, w * 10);
        break;
    }
    case 0:
        if (Weapon[VehPrototypes[l_14].weapon_id].mode <= 2) {
            int lim = 99;
            if (VehPrototypes[l_14].plan <= 1) {
                // lim = 1 is the deferred else arm (0x4fa631/0x4fa634) shared by both
                // of its paths; the two lim = 2 stores stay separate (0x4fa603, 0x4fa62a).
                if (!PlayersData[l_8].unk_71
                    && (l_c != 2 || !(PlayersData[l_8].unk_77[l_10] & 0x400))) {
                    if (l_c == 2) {
                        lim = 2;
                    } else if (PlayersData[l_8].unk_77[l_10] & 0x600) {
                        lim = 2;
                    }
                } else {
                    lim = 1;
                }
            }
            int extra = (l_c != 2 && !(PlayersData[l_8].unk_77[l_10] & 0x400)) ? a_594[l_2c * 3] : 0;
            l_18 = (a_f6c[l_14]
                    + range(a_176c[l_14] + a_3c4[l_2c * 3] + a_4bc[l_2c * 3], 0, lim)
                    + extra + 1) / 2;
            if (has_fac(FAC_COMMAND_CENTER, BaseIDCurrentSelected, 0)) {
                w = (l_11c + 2) * w / (l_11c + 3);
            }
            if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                && (PlayersData[l_8].flags & 0x4000000)) {
                w /= 2;  // cross-jumped into the air arm's l_38 tail (0x4fa2fe)
            }
        } else if (l_2c == 8 && l_c == 8 && !(BaseCurrent->state & 0x2000000)
                   && GameDataStateStorage.fields_.diff_level_current_ >= 3) {
            l_18 = 0;
        } else {
            l_18 = a_4bc[l_2c * 3];
            if (!(l_2c == 8 && l_c == 8 && !(BaseCurrent->state & 0x1000000)
                  && GameDataStateStorage.fields_.diff_level_current_ >= 3)) {
                l_18 += a_594[l_2c * 3] + a_f6c[l_14];
                if (l_2c == 8) {
                    l_18 += a_3c4[24];
                }
            }
        }
        break;
    case 2: {
        // WORKAROUND(inline): the image calls range (0x00422F00) out of line for the
        // three clamps after n - 2 (0x4fa280, 0x4fa2d9, 0x4fa2ec).
        int (__cdecl *const range_call)(int, int, int) = range;
        if (l_c != 7 && !l_38
            && PlayersData[l_8].region_total_combat_vehs[l_10]
                   < PlayersData[l_8].region_total_bases[l_10] * 2 + 2) {
            goto L_4fd8f6;
        }
        l_48 = a_3c4[l_2c * 3 + 2];
        l_b8 = a_2ec[l_2c * 3 + 2];
        int n = (l_48 + l_b8 != 0) + 1;
        if (l_c != 7 && l_2c != 5 && !l_38) {
            if (PlayersData[l_8].x_target >= 0) n++;
            if (l_c == 2) n++;
            if (PlayersData[l_8].unk_77[l_10] & 0x400) n++;
            n += a_1dc[2];
        }
        w = n * w;
        if (has_fac(FAC_AEROSPACE_COMPLEX, BaseIDCurrentSelected, 0)) {
            w = (l_ac + 2) * w / (l_ac + 3);
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && (PlayersData[l_8].flags & 0x10000000)) {
            w /= 2;
        }
        l_18 = ((l_2c != 4) + 1) * l_48 + range(n - 2, 0, 99) + l_b8;
        int s = range_call(8 - l_fc, 4, 8) * w / 8;
        w = range_call(veh_cost(l_14, BaseIDCurrentSelected, NULL) * cost_factor(l_8, 1, -1) * s
                           / (range_call(BaseCurrent->mineral_surplus, 1, 999) * 15),
                       s, s * 10);
        if (l_38) {
            w /= 2;  // tail shared with the triad 0 test at 0x4fa2fe
        }
        break;
    }
    }
    l_18++;
    b_50 = VehPrototypes[l_14].plan;
    // Plan switch (jump table 0x4fffb8). Its bodies for plans 9-11 lie in chunk 2 and
    // its end is 0x4fc68f (chunk 2), so `break` is spelled `goto L_4fc68f` here.
    switch (b_50) {
    case 0:  // offense
        w *= (l_c <= 2 || ((l_c == 7 || l_38) && l_cc)) ? 32 : 64;
        if (l_88 && !l_5c
            && PlayersData[l_8].region_total_offensive_vehs[l_10] < guard_check(l_8, l_10)) {
            w /= 2;
        }
        if (l_104 & 2) {
            w /= 2;
        } else if (l_104 & 1) {
            w = w * 3 / 4;
        }
        if (l_38 && !l_20) {
            w *= 2;
        }
        if (l_30 && (l_20 == 2 || !l_38)) {
            w /= 2;
            if (GameDataStateStorage.fields_.turn_current_num_ < 50) {
                w /= 2;
            }
        }
        if (l_20 == 2 && l_c != 2) {
            if ((PlayersData[l_8].unk_66 > PlayersData[l_8].unk_67
                     ? PlayersData[l_8].unk_66 : PlayersData[l_8].unk_67)
                > a_3c4[l_2c * 3 + 2] + a_2ec[l_2c * 3 + 2]) {
                w /= 2;
            }
            if ((PlayersData[l_8].unk_66 > PlayersData[l_8].unk_67
                     ? PlayersData[l_8].unk_66 : PlayersData[l_8].unk_67) * 2
                > a_3c4[l_2c * 3 + 2] + a_2ec[l_2c * 3 + 2]) {
                w /= 2;
            }
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && Chassis[VehPrototypes[l_14].chassis_id].missile
            && (PlayersData[l_8].flags_ext & 0x10)) {
            w /= 3;
        }
        goto L_4fc68f;
    case 1:  // combat
        w *= (l_c <= 2 || ((l_c == 7 || l_38) && l_cc)) ? 24 : 32;
        if (l_88 && !l_5c
            && PlayersData[l_8].region_total_offensive_vehs[l_10] < guard_check(l_8, l_10)) {
            w /= 2;
        }
        if (l_38 && !l_20) {
            w *= 2;
        }
        if (l_30 && !l_38) {
            w /= 2;
            if (GameDataStateStorage.fields_.turn_current_num_ > 50) {
                w /= 2;
            }
        }
        goto L_4fc68f;
    case 2:  // defense
        w *= 16;
        if (l_160 && !l_28 && has_fac_built(FAC_COMMAND_CENTER)) {
            w /= 2;
        }
        if (l_88 > 1 && !l_5c
            && PlayersData[l_8].region_total_offensive_vehs[l_10] < guard_check(l_8, l_10)) {
            w *= 2;
        }
        if (has_abil(l_14, 0x40000) && !stack_check(l_4c, 6, 0x40000, -1, -1)
            && PlayersData[l_8].planet_ecology >= 4) {
            w = w * 2 / 3;
            if (PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE] <= 0) {
                w = w * 2 / 3;
            }
        }
        if (has_abil(l_14, 0x100)) {
            if (BaseCurrent->state & 0x100000) {
                w /= 2;
            } else {
                if (stack_check(veh_at(BaseCurrent->x, BaseCurrent->y), 6, 0x100, -1, -1)) {
                    w *= 3;
                }
                if (l_d0) {
                    w /= 2;
                } else if (!PlayersData[l_8].unk_66 && !PlayersData[l_8].unk_68) {
                    w *= 4;
                }
            }
        }
        if (l_30 || l_3c || l_64) {
            if (GameDataStateStorage.fields_.turn_current_num_ > 25) {
                w *= 2;
            }
            if (GameDataStateStorage.fields_.turn_current_num_ > 75) {
                w *= 2;
            }
        }
        goto L_4fc68f;
    case 3: {  // reconnaissance
        // WORKAROUND(inline): the image calls range (0x00422F00) out of line at 0x4fac23.
        int (__cdecl *const range_call)(int, int, int) = range;
        l_48 = has_abil(l_14, 0x40000);
        if (!l_28 && (l_e4 || !l_48)) {
            int lim;
            if (l_64 || l_c) {
                if (l_c == 2) {
                    l_78 = 1;
                }
            } else if (a_176c[l_14] || a_f6c[l_14]) {
                l_78 = 1;
            }
            if (l_c == 7) {
                lim = 1;
            } else {
                lim = (Continents[l_10].tile_count + 199) / 200;
            }
            if (a_176c[l_14] + a_f6c[l_14] * 2 > lim && l_c != 8) {
                l_78 = 1;
            }
            if (a_176c[l_14] > PlayersData[l_8].current_num_bases
                    - range_call(GameDataStateStorage.fields_.turn_current_num_ / 50, 0,
                                 PlayersData[l_8].current_num_bases - 1)) {
                l_78 = 1;
            }
        }
        if ((l_c == 8 || !l_e8) && l_e8 < l_150 + l_64 * 2
            && (l_64 || (!l_30 && !l_40 && !l_3c))
            && l_e8 < (PlayersData[l_8].current_num_bases + 1) / ((l_64 == 0) + 1)) {
            w *= 16;
        } else if (l_48 && !l_e4) {
            w *= 32;
            if (BaseCurrent->eco_damage) {
                if (l_c != 2) {
                    w /= 2;
                    if (l_c > 2) {
                        w /= 2;
                    }
                }
            } else if (l_c > 2) {
                w /= 2;
            }
        } else {
            w = ((!l_28 && !l_64 && PlayersData[l_8].best_armor_value >= 2) ? 0x80 : 0x20) * w;
        }
        if (l_30 && PlayersData[l_8].best_armor_value < 2 && !l_38) {
            w /= 2;
        }
        goto L_4fc68f;
    }
    case 6: {  // naval superiority (laid between reconnaissance and air superiority, 0x4fad54)
        // WORKAROUND(inline): the image calls range (0x00422F00) out of line at 0x4fae88.
        int (__cdecl *const range_call)(int, int, int) = range;
        if (!naval_base(BaseIDCurrentSelected)) goto L_4fd8f6;
        w = (l_70 != 1 ? 128 : 256) * w * 56 / MapAreaSqRoot;
        if (l_c == 7) {
            w = w * 2 / 3;
        }
        if (has_fac_built(FAC_NAVAL_YARD)) {
            w = (l_118 + 2) * w / (l_118 + 3);
        }
        if (VehPrototypes[l_14].unk_1 == 21) {
            if (l_38) {
                w /= 2;
            }
            if (a_3c4[l_2c * 3 + 1] + a_2ec[l_2c * 3 + 1] >= a_2ec[22]) {
                w *= 2;
            }
            if (a_2ec[22] > (a_3c4[l_2c * 3 + 1] + a_2ec[l_2c * 3 + 1]) * 3 / 2) {
                w /= 2;
            }
            goto L_4fc68f;
        }
        if (at_goal(l_8, 6, BaseCurrent->x, BaseCurrent->y)) {
            w /= 2;
        }
        w = (range_call((a_2ec[l_20 + 18] - (PlayersData[l_8].unk_64 & 0x7ff)) * 2 + 8
                            - PlayersData[l_8].unk_65 / 2, 0, 8) + 8) * w / 16;
        if (a_2ec[19] + a_3c4[19] * 2 < l_c4) {
            w /= 2;
            if (l_c == 7) {
                w /= 2;
            }
        }
        if (a_3c4[19] + a_2ec[19] < a_3c4[22] + a_2ec[22]) {
            w /= 2;
            if (l_c == 7) {
                w /= 2;
            }
        }
        if (l_64) {
            w = w * 3 / 4;
        }
        if (PlayersData[l_8].flags & 0x80000) {
            w /= 2;
        }
        if (Players[l_8].rule_flags & 0x100) {
            w /= 2;
        }
        if (l_38) {
            w /= 2;
        }
        if (l_30 && l_38) {
            w /= 2;
        }
        goto L_4fc68f;
    }
    case 4:  // air superiority
        w *= 64;
        if (PlayersData[l_8].unk_106 * 2 >= PlayersData[l_8].unk_105) {
            l_48 = a_3c4[l_20 + l_2c * 3] + a_2ec[l_20 + l_2c * 3];
            if (l_48 > PlayersData[l_8].unk_66 * 4 / 3) {
                w *= 2;
            }
        } else if (PlayersData[l_8].unk_106 < PlayersData[l_8].unk_105 / 2) {
            w /= 2;
            if (PlayersData[l_8].unk_106 < PlayersData[l_8].unk_105 / 4) {
                w /= 2;
            }
        }
        if (PlayersData[l_8].unk_66 > a_3c4[l_20 + l_2c * 3] + a_2ec[l_20 + l_2c * 3]) {
            if (!a_2ec[l_20 + l_2c * 3]) {
                w /= 2;
            }
            w /= 2;
            if (l_c != 2) {
                if (!a_2ec[l_20 + l_2c * 3]) {
                    w /= 2;
                }
                if (PlayersData[l_8].unk_66 >= (a_3c4[l_20 + l_2c * 3] + a_2ec[l_20 + l_2c * 3]) * 2) {
                    w /= 2;
                }
                w /= 2;
            }
        }
        if (!(BaseCurrent->state & 0x100000) && l_d0) {
            w /= 2;
        }
        goto L_4fc68f;
    case 5:  // planet buster
        w *= 64;
        if (l_30) {
            w /= 2;
            if (l_40) {
                w /= 2;
            }
        }
        if (a_594[l_20 + l_2c * 3] >= 2) goto L_4fd8f6;
        if (!a_3c4[l_20 + l_2c * 3] && a_2ec[l_20 + l_2c * 3] < 2) {
            w /= 2;
        }
        l_54 = 4;  // highest orbital defense of the others
        l_44 = 0;  // a target faction halved the weight already
        for (i = 1; i < 8; i++) {
            if (i != l_8
                && (!has_treaty(l_8, i, 1)
                    || is_human(i))) {
                if (l_54 <= PlayersData[i].satellites_[3]) {
                    l_54 = PlayersData[i].satellites_[3];
                }
                if (!l_44 && (has_treaty(l_8, i, 0x30) || PlayersData[i].major_atrocities)
                    && PlayersData[l_8].planet_busters < PlayersData[i].satellites_[3] + 1) {
                    l_44 = 1;
                    w /= 2;
                }
            }
        }
        if (PlayersData[l_8].planet_busters > l_54) goto L_4fd8f6;
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)) {
            if (PlayersData[l_8].flags_ext & 0x10) {
                w /= 2;
            }
            if (PlayersData[l_8].flags & 0x1000000) {
                w /= 2;
            }
        }
        goto L_4fc68f;
    case 13:  // tectonic payload
        w *= 64;
        if (a_594[l_20 + l_2c * 3] >= 2) goto L_4fd8f6;
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) goto L_4fc68f;
        if (l_58 && !l_60) {
            w /= 2;
        }
        if (PlayersData[l_8].flags_ext & 0x10) {
            w /= 2;
        }
        goto L_4fc68f;
    case 14:  // fungal payload
        w *= 64;
        if (PlayersData[l_8].soc_effect_pending.values_[SocialEffect::PLANET] > 0) {
            w /= 2;
        }
        if (a_594[l_20 + l_2c * 3] >= 2) goto L_4fd8f6;
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) goto L_4fc68f;
        if (l_a4 && !l_60) {
            w /= 2;
        }
        if (PlayersData[l_8].flags_ext & 0x10) {
            w /= 2;
        }
        goto L_4fc68f;
    case 7: {  // transport
        if (l_20 == 1 && !transport_base(BaseIDCurrentSelected)) goto L_4fd8f6;
        w = (l_c == 7 ? 16 : (l_c != 8 ? 128 : 64)) * w;
        if (l_64 || (l_30 && l_c == 7)) {
            w /= 2;
            if (!a_3c4[l_20 + 21] && !PlayersData[l_8].proto_id_active[l_14] && l_2c == 7) {
                w /= 4;
            }
        }
        if (l_38 && l_28 < 3) {
            w /= 4;
            if (!a_2ec[l_20 + 21]) {
                w /= 4;
            }
        }
        if (has_abil(l_14, 0x80000)) {
            w = w * 3 / 4;
        }
        int lim = PlayersData[l_8].current_num_bases / 2;
        if (lim < 2) {
            lim = 2;
        }
        if (a_2ec[l_20 + 21] >= lim) {
            w *= 2;
        }
        if (l_2c == 7) {
            if (a_3c4[l_20 + 18] + a_2ec[l_20 + 18] + a_2ec[l_20 + 21] + a_3c4[l_20 + 21] < 2
                && GameDataStateStorage.fields_.turn_current_num_ < 100) {
                w /= 2;
            }
            if (!a_2ec[l_20 + 21] && !a_3c4[l_20 + 21]
                && (a_2ec[l_20 + 24] || a_3c4[l_20 + 24]) && l_c > 2) {
                w /= 2;
            }
        }
        if (BaseCurrent->unk_1) {
            w *= 2;
        }
        if (l_c == 7) {
            if (l_28 >= 2 || l_2c != 7) goto L_4fc68f;
            int regions = 0;
            for (int j = 1; j < 128; j++) {
                if (PlayersData[l_8].region_total_bases[j]) {
                    regions++;
                }
            }
            int want = (PlayersData[l_8].region_total_bases[l_10] + 3) / 4;
            if (regions > want) {
                want = regions;
            }
            if (a_2ec[l_20 + 21] - (PlayersData[l_8].proto_id_active[13] != 0) >= want) goto L_4fc68f;
            if (a_3c4[l_20 + 21]) goto L_4fc68f;
            w /= 4;
        } else {
            if (l_70 == 1) {
                w *= 2;
            }
            if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) goto L_4fc68f;
            if (a_176c[l_14] > 1 || l_c == 2 || (PlayersData[l_8].unk_77[l_10] & 0x400)) {
                l_78 = 1;
            }
        }
        goto L_4fc68f;
    }
    default:
        goto L_4fc68f;  // also plan 12 and plans > 14 (position in the switch is immaterial)
    case 8:  // colony
        // The whole block below is skipped (to 0x4fb650) for a governed human base
        // whose governor only has the 0x20000 permission among 0xDFBE00.
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && (BaseCurrent->governor & 0xdfbe00) == 0x20000) {
            goto L_4fb650;
        }
        if (!((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
              && (BaseCurrent->governor & 0x80000000))) {
            if (((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) || l_28 >= 3)
                && BaseCurrent->population_size < 2) {
                goto L_4fd8f6;
            }
            if (a_4bc[l_20 + 24] + a_594[l_20 + 24] > 1 && l_c != 8) goto L_4fd8f6;
        }
        if (BaseCurrent->nutrient_surplus < 1 && (a_2ec[l_20 + 24] || a_594[l_20 + 24])) {
            goto L_4fd8f6;
        }
        if (l_20) {
            // sea colony pods: the skip is the fall-through `jmp` at 0x4fb61d, the
            // keep path jumps over the land block to 0x4fb650.
            if (l_38
                || (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                    && (PlayersData[l_8].flags_ext & 4))
                || (!PlayersData[l_8].proto_id_active[l_14] && !a_1f6c[l_14])) {
                goto L_4fb650;
            }
            goto L_4fd8f6;
        }
        goto L_4fb622;  // land colony pods: laid next, at 0x4fb622 (chunk 2)
    // (chunk 2 continues at 0x4fb622)
    // ---- chunk 2: 0x4FB622-0x4FCE6A ----
    // (inside the plan switch of the unit loop: tail of case PLAN_COLONIZATION)
L_4fb622:
        // land colony pod
        if (l_38 && (l_a8 < PlayersData[l_8].current_num_bases || l_a8 < 3)) {
            goto L_4fd8f6;
        }
        if (l_c == 2) {
            goto L_4fd8f6;
        }
L_4fb650:
        // The image keeps `state & 0x1000000` and the dword human test (chunk 1's esi)
        // in registers across this block: common subexpressions, not variables.
        if (BaseCurrent->state & 0x1000000) {
            int t = PlayersData[l_8].current_num_bases
                - GameDataStateStorage.fields_.turn_current_num_
                    / ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & l_1c) ? 40 : 20)
                - l_60 - l_a4 - 1;
            l_18 += (t < 0) ? 0 : t;
        }
        w *= 5;
        if (BaseCurrent->population_size < 2
            && (BaseCurrent->nutrient_surplus < BaseCurrent->mineral_surplus
                || BaseCurrent->nutrient_surplus < 1)) {
            w *= 2;
        } else if (l_c > 2 && !(BaseCurrent->state & 0x1000000)) {
            if (!(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & l_1c) || l_c != 7
                || PlayersData[l_8].current_num_bases < GameDataStateStorage.fields_.base_current_count_ / 5
                || PlayersData[l_8].current_num_bases < 10) {
                if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & l_1c)
                    && (BaseCurrent->governor & 0x80000000)) {
                    if (l_64) {
                        w /= range_call(GameDataStateStorage.fields_.diff_level_current_ + 1, 2, 99);
                    } else if (l_40 || l_30) {
                        w /= 2;
                    }
                } else {
                    w /= GameDataStateStorage.fields_.diff_level_current_ + 1;
                }
            }
        }
        if (PlayersData[l_8].flags_ext & 1) {
            w /= 2;
        }
        if (PlayersData[l_8].unk_77[l_10] & 0x400) {
            w *= 2;
            l_78 = 1;
        }
        if (GameDataStateStorage.fields_.game_rules_ & 0x20000000) {
            w /= 2;
            if (l_c == 8) {
                w /= 2;
            }
        }
        if (!l_20) {
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                && GameDataStateStorage.fields_.turn_current_num_ > 100) {
                goto L_4fd8f6;
            }
            // a_4bc/a_594 [9] = land reconnaissance, [27] = land terraforming
            if (l_c == 8
                && (a_4bc[9] || a_594[9] || !PlayersData[l_8].unk_83[l_10])
                && (a_4bc[27] || a_594[27] || !has_tech(VehPrototypes[1].preq_tech, l_8))
                && !(PlayersData[l_8].unk_77[l_10] & 0x20)) {
                w /= 2;
            }
            if (!a_2ec[l_2c * 3] && !a_3c4[l_2c * 3] && BaseCurrent->nutrient_surplus >= 0
                && l_c != 2
                && (!(PlayersData[l_8].unk_77[l_10] & 0x401) || l_c == 7)) {
                w /= 2;
            }
            if (l_28 < 3 && !a_4bc[l_2c * 3] && !a_594[l_2c * 3]) {
                if (l_c != 8) {
                    if (l_c != 7) {
                        goto L_4fba4f;
                    }
                    if ((BaseCurrent->state & 0x1000000) || MapOceanCoverage >= 1) {
                        goto L_4fba68;
                    }
                }
                w /= 2;
                if (PlayersData[l_8].current_num_bases == 1) {
                    w /= 4;
                }
            }
        } else {
            if (l_38) {
                w /= 2;
                if (PlayersData[l_8].current_num_bases < 4) {
                    w /= 2;
                }
                if (PlayersData[l_8].current_num_bases < 8) {
                    w /= 2;
                }
                if (Players[l_8].rule_flags & 0x100) {
                    w /= 2;
                }
            }
            if (MapOceanCoverage > 1) {
                w /= 2;
                if (l_28 < 3 && !a_594[l_20 + l_2c * 3] && !a_4bc[l_20 + l_2c * 3]
                    && !(BaseCurrent->state & 0x1000000)) {
                    w /= 2;
                }
            }
            if (BaseCurrent->nutrient_surplus >= Rules.nutrient_req_citizen && (l_c == 7 || l_38)) {
                w /= 2;
            }
            if (!l_38 && l_c != 7
                && ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                    || !(PlayersData[l_8].flags_ext & 4))) {
                w *= 2;
            }
            if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                && (PlayersData[l_8].flags_ext & 4)) {
                w /= 2;
            }
        }
L_4fba4f:
        if (l_c != 8 || PlayersData[l_8].region_total_bases[l_10] > 1) {
L_4fba68:
            if (PlayersData[l_8].current_num_bases >= 3 && l_e0 == PlayersData[l_8].unk_28) {
                w *= 2;
            }
            if (PlayersData[l_8].current_num_bases >= 4 && BaseCurrent->population_size > l_144) {
                w *= 2;
            }
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                && PlayersData[l_8].current_num_bases
                    >= (PlayersData[l_8].diff_level + 1) * GameDataStateStorage.fields_.turn_current_num_ / 40) {
                w *= 3;
            }
        }
        if (!strcmp(Players[l_8].filename, "BELIEVE")
            && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && !(PlayersData[l_8].unk_77[l_10] & 1)) {
            w /= 2;
        }
        if (l_64) {
            w /= 2;
        }
        break;

    case PLAN_TERRAFORMING:
        w *= l_70 ? 6 : 4;
        if (l_40) {
            w /= 2;
        } else if (l_30) {
            w *= 2;
        }
        if (PlayersData[l_8].flags_ext & 2) {
            w /= 2;
        }
        if (l_20 == 1) {
            // sea formers; a_2ec/a_4bc [28] = sea terraforming
            if ((a_f6c[l_14] || a_2ec[28] > l_a8 / 2) && !l_38) {
                goto L_4fd8f6;
            }
            if (BaseCurrent->state & 0x80000) {
                goto L_4fd8f6;
            }
            if (!l_38 && (l_c <= 2 || (l_c != 7 && a_2ec[28]))) {
                goto L_4fd8f6;
            }
            if (l_cc && !l_38 && a_2ec[28]) {
                goto L_4fd8f6;
            }
            if (!l_28) {
                if (l_38 && a_176c[l_14] < l_a8) {
                    w /= 2;
                }
                if (a_176c[l_14] < l_a8 / 2) {
                    w /= 2;
                }
                if (!a_2ec[28] && !l_cc) {
                    w /= 2;
                }
            }
            if (a_4bc[28] >= l_a8) {
                goto L_4fd8f6;
            }
            if (l_38) {
                w /= 2;
            }
            break;
        }
        if (l_38) {
            goto L_4fd8f6;
        }
        l_44 = a_4bc[l_20 + 27];  // formers of this triad in the region
        if (!l_44 && !a_f6c[l_14] && GameDataStateStorage.fields_.turn_current_num_ < 25
            && !l_30 && !l_64) {
            l_28 = 0;  // BUG(original): the need persists for the following units
        } else if (l_28 >= 3) {
            goto L_4fbd33;
        }
        if (!l_44 && !a_f6c[l_14]) {
            if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                && (l_c == 8 || GameDataStateStorage.fields_.turn_current_num_ < 40)) {
                w /= 16;
            }
            if (l_c == 7) {
                w /= 2;
                goto L_4fbd3d;
            }
        }
L_4fbd33:
        if (l_c == 2) {
            goto L_4fd8f6;
        }
L_4fbd3d:
        l_b8 = PlayersData[l_8].unk_77[l_10];  // 16-bit value (stored as a dword)
        if (l_b8 & 0x400) {
            w <<= 2;
        }
        if (!l_28 && !l_30) {
            l_48 = BaseCurrent->state;
            if (!(l_48 & 0x20)
                && a_4bc[l_20 + l_2c * 3] < PlayersData[l_8].region_total_bases[l_10]) {
                if (l_c != 2 && (l_98 || l_58 || (!l_c0 && l_c > 2))) {
                    w /= 2;
                }
                if (l_c == 7 || (l_48 & 0x2000000)) {
                    if (!l_98 && !l_58 && l_c0) {
                        w /= 2;
                    } else {
                        w /= 4;
                    }
                }
            }
            if (l_c > 2) {
                if (a_4bc[l_20 + l_2c * 3] <= PlayersData[l_8].region_total_bases[l_10]) {
                    w /= 2;
                }
                if (a_4bc[l_20 + l_2c * 3] <= (PlayersData[l_8].region_total_bases[l_10] + 1) / 2
                    && (!(l_b8 & 0x20) || !l_c0)
                    && (l_98 || l_58 || !l_c0)) {
                    w /= 2;
                }
            }
        }
        if (BaseCurrent->state & 0x20) {
            w *= 3;
        }
        if (l_44 >= PlayersData[l_8].region_total_bases[l_10]) {
            goto L_4fd8f6;
        }
        if (l_44 >= PlayersData[l_8].region_total_bases[l_10] * 2 / 3 && (!l_c || l_c == 2)) {
            w = w * 3 / 2;
        }
        if (!MapCloudCover) {
            w /= 2;
        }
        break;

    case PLAN_INFO_WARFARE: {
        int target;
        int k;
        if (a_4bc[l_20 + l_2c * 3] >= PlayersData[l_8].region_total_bases[l_10]) {
            goto L_4fd8f6;
        }
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && a_3c4[l_20 + l_2c * 3] + a_2ec[l_20 + l_2c * 3] > 1) {
            goto L_4fd8f6;
        }
        l_54 = 0;
        if (l_28 < 2 && GameDataStateStorage.fields_.diff_level_current_ > 2 && l_74 < 2) {
            if (!strcmp(Players[l_8].filename, "PEACE")) {
                l_54 = 1;
            }
            if (at_climax(l_8)) {
                l_54++;
            }
        }
        if (PlayersData[l_8].flags_ext & 8) {
            l_54 += 2;
        }
        if (a_594[l_20 + l_2c * 3] > l_54) {
            goto L_4fd8f6;
        }
        target = -1;
        l_44 = 0;
        l_a0 = 0;
        l_b4 = 0;
        for (i = 1; i < 8; i++) {
            if (i == l_8) {
                continue;
            }
            l_a0 += PlayersData[l_8].diplo_mind_control[i];
            l_44 += PlayersData[l_8].stolen_data_count[i];
            if (!PlayersData[i].region_total_bases[l_10]) {
                continue;
            }
            if (project_call(SP_HUNTER_SEEKER_ALGO, i)) {
                continue;
            }
            if (!has_treaty(l_8, i, 8)) {
                continue;
            }
            l_b4++;
            if (has_treaty(l_8, i, 7)) {
                if (!is_human(i) && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)) {
                    continue;
                }
                if (!l_54) {
                    if (has_treaty(l_8, i, 0x1000000)) {
                        if (PlayersData[i].ranking < PlayersData[l_8].ranking) {
                            continue;
                        }
                        if ((PlayersData[i].tech_ranking - PlayersData[l_8].tech_ranking) / 2
                            < ((PlayersData[i].ranking != 7) ? 3 : 6)) {
                            continue;
                        }
                    } else if (PlayersData[i].ranking < PlayersData[l_8].ranking
                               && PlayersData[i].tech_ranking < PlayersData[l_8].tech_ranking) {
                        continue;
                    }
                }
            }
            target = i;
            if (is_human(i) && !IsMultiplayerNet) {
                break;
            }
        }
        if (target < 0) {
            if (((l_c == 7 && GameDataStateStorage.fields_.diff_level_current_ > 1) || l_3c)
                && !a_4bc[l_20 + l_2c * 3]) {
                target = GameDataStateStorage.fields_.faction_rankings_[7];
            }
            if (target < 0) {
                goto L_4fd8f6;
            }
        }
        if (target == l_8) {
            goto L_4fd8f6;
        }
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
            l_b8 = 0;
        } else {
            l_b8 = PlayersData[l_8].energy_reserves / range_call(PlayersData[target].energy_reserves, 100, 999999);
        }
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
            l_8c = 0;
        } else {
            l_8c = PlayersData[target].mil_strength_1
                / range_call(PlayersData[l_8].mil_strength_1, PlayersData[l_8].current_num_bases + 1, 999999);
        }
        if (GameDataStateStorage.fields_.game_rules_ & 0x40) {
            l_bc = 5;
        } else {
            l_bc = PlayersData[target].diff_level;
        }
        k = (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) ? 4 : 2;
        w = range_call(PlayersData[l_8].tech_ranking / k - PlayersData[target].tech_ranking / k
                      + PlayersData[target].soc_effect_pending.values_[SocialEffect::PROBE]
                      - l_bc - l_8c - l_b8 - l_54
                      + a_4bc[l_20 + l_2c * 3] + bit_count(has_treaty(l_8, target, 7)) + 13,
                  1, 16)
            * w * 3 / (l_58 - l_60 + 3);
        bb_probe(0);
        bb_probe(0);
        bb_probe(0);
        bb_probe2(0);
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)) {
            if (has_tech(Chassis[5].preq_tech, target) && !has_tech(Chassis[5].preq_tech, l_8)) {
                w /= 2;
            }
            if (has_tech(Chassis[8].preq_tech, target) && !has_tech(Chassis[8].preq_tech, l_8)) {
                w /= 2;
            }
            if (has_tech(Weapon[16].preq_tech, target) && !has_tech(Weapon[16].preq_tech, l_8)) {
                w /= 2;
            }
            if (has_tech(Facility[102].preq_tech, target) && !has_tech(Facility[102].preq_tech, l_8)) {
                w /= 2;
            }
            for (j = 1; j <= 3; j++) {
                if (has_tech(Reactor[j].preq_tech, target) && !has_tech(Reactor[j].preq_tech, l_8)) {
                    w /= 2;
                }
            }
            if (PlayersData[target].best_weapon_value > PlayersData[l_8].best_weapon_value
                || PlayersData[target].best_armor_value > PlayersData[l_8].best_armor_value) {
                w /= 2;
            }
        }
        if (l_c != 7 && (PlayersData[l_8].unk_77[l_10] & 0x20) && l_b4) {
            if (l_3c && !a_2ec[l_20 + l_2c * 3] && !a_3c4[l_20 + l_2c * 3]) {
                w /= 2;
            }
        } else {
            if (!has_tech(Chassis[3].preq_tech, l_8)) {
                goto L_4fd8f6;
            }
            if (a_4bc[l_20 + l_2c * 3] && l_a0 + l_44 < 3) {
                goto L_4fd8f6;
            }
            if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
                w *= 2;
            }
            if (a_3c4[l_20 + l_2c * 3] + a_2ec[l_20 + l_2c * 3]) {
                w *= 2;
            }
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && (PlayersData[l_8].flags_ext & 8)) {
            w /= 4;
        }
        if (l_70 == 1 && l_7c > 0) {
            l_18 *= 2;
        }
        break;
    }

    case PLAN_SUPPLY_CONVOY:
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
            goto L_4fd8f6;
        }
        if (l_c <= 2) {
            goto L_4fd8f6;
        }
        l_34 = 3;  // slot reused: convoys still allowed
        // One loop guard in the image; its skip edge bypasses the `!l_34` test
        // (l_34 is still 3 there).
        for (i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
            if (Vehs[i].faction_id == l_8
                && VehPrototypes[Vehs[i].proto_id].plan == b_50) {
                if (Vehs[i].home_base_id == BaseIDCurrentSelected) {
                    w *= 2;
                }
                if (Vehs[i].order != ORDER_CONVOY) {
                    goto L_4fd8f6;
                }
                if (l_34) {
                    l_34--;
                }
                w = w * 3 / 2;
            }
        }
        if (!l_34) {
            goto L_4fd8f6;
        }
        if (!l_b0 && l_c == 7 && PlayersData[l_8].ai_tech) {
            w /= 2;
        }
        w = w * 768 / BaseCurrent->population_size;
L_4fc68d:
        break;
    }
L_4fc68f:
    // Prototypes: compare with the faction's best active designs.
    if (l_14 >= 64 && !(VehPrototypes[l_14].flags & 4)) {
        l_80 = 0;
        a_1ec[2] = 0;
        a_1ec[1] = 0;
        a_1ec[0] = 0;
        l_48 = 0;
        l_44 = 0;
        l_a0 = 0;
        for (l_b4 = 0; l_b4 < 128; l_b4++) {
            int id = (l_b4 < 64) ? l_b4 : l_8 * 64 + l_b4 - 64;
            if (id == l_14) {
                continue;
            }
            if (!(VehPrototypes[id].flags & 1)) {
                continue;
            }
            if (id >= 64 && !(VehPrototypes[id].flags & 4)) {
                continue;
            }
            if (l_a0 <= weap_val(id, l_8)) {
                l_a0 = weap_val(id, l_8);
            }
            // BUG(original): arm_val takes an armor id but is passed the prototype id.
            if (l_44 <= arm_val(id, l_8)) {
                l_44 = arm_val(id, l_8);
            }
            l_48 |= VehPrototypes[id].ability_flags;
            l_b8 = a_1ec[Chassis[VehPrototypes[id].chassis_id].triad];
            a_1ec[Chassis[VehPrototypes[id].chassis_id].triad]
                = (l_b8 > speed_proto(id)) ? l_b8 : speed_proto(id);
        }
        int wv = weap_val(l_14, l_8);
        int av = arm_val(l_14, l_8);  // BUG(original): prototype id as armor id
        int sp = speed_proto(l_14);
        if (l_2c == 2) {
            if (Armor[VehPrototypes[l_14].armor_id].defense_rating != 1 && av > l_44
                && (av > l_44 + 1 || l_c == 7)) {
                w = w * 2 / 3;
                l_80 = 1;
            }
        } else if (l_2c <= 1) {
            if (wv > l_a0 + ((l_c == 7 && l_f4) ? 0 : 1)) {
                l_80 = 1;
            }
        }
        if (sp > a_1ec[Chassis[VehPrototypes[l_14].chassis_id].triad]) {
            l_80 = 1;
        }
        if (!l_124) {
            if (l_2c != 5 || PlayersData[l_8].planet_busters) {
                if (!l_80) {
                    l_18 += range_call((l_c > 2) - (l_30 || l_3c) + l_a4, 0, 2);
                }
                if (l_c == 8 || (l_c == 7 && !(BaseCurrent->state & 0x1000000))) {
                    if (l_2c != 8) {
                        if (!l_30) {
                            l_18++;
                            if (!l_f4 && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)) {
                                w *= 2;
                            }
                        }
                        if (!l_80) {
                            if (GameDataStateStorage.fields_.turn_current_num_ < 50) {
                                w *= 2;
                            }
                            if (GameDataStateStorage.fields_.turn_current_num_ < 100
                                && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)) {
                                w *= 2;
                            }
                        }
                        if (!(BaseCurrent->state & 0x1000000)) {
                            w *= 2;
                        }
                    } else {
                        w /= 2;
                    }
                }
            }
            if (l_28 >= 3) {
                w <<= 3;
            }
        }
        if (!a_1f6c[l_14]) {
            int lim = PlayersData[l_8].unk_28 * 3 / 4;
            if (PlayersData[l_8].unk_28 - 1 < lim) {
                lim = PlayersData[l_8].unk_28 - 1;
            }
            if (l_e0 >= lim && BaseCurrent->queue_production_id[0] > -70 && !l_124) {
                if (l_ec < 2) {
                    w /= 2;
                }
                if (VehPrototypes[l_14].plan == PlayersData[l_8].region_base_plan[l_10]) {
                    w /= 2;
                }
                if (VehPrototypes[l_14].plan == 1 && PlayersData[l_8].region_base_plan[l_10] <= 2) {
                    w /= 2;
                }
                if (l_80 && PlayersData[l_8].region_base_plan[l_10] == 7 && l_f4) {
                    w /= 2;
                }
                if (VehPrototypes[l_14].plan == 1 && (l_58 || l_98)) {
                    w /= 2;
                }
                if (VehPrototypes[l_14].plan == 0 && (l_60 || l_a4)) {
                    w /= 2;
                }
            }
            if (l_124) {
                w = w * 2 / 3;
            } else if (l_ec) {
                w = (l_ec + 5) * w / 5;
            }
            if (l_124 || l_ec < PlayersData[l_8].current_num_bases / 4) {
                switch (PlayersData[l_8].region_base_plan[l_10]) {
                case 0:
                case 2:
                    if (VehPrototypes[l_14].plan == PlayersData[l_8].region_base_plan[l_10]) {
                        w /= 2;
                    }
                    if (VehPrototypes[l_14].plan != 1) {
                        break;
                    }
                    // fall through
                case 8:
                    w = w * 3 / 4;
                    break;
                case 7:
                    if (VehPrototypes[l_14].plan == 7) {
                        w /= 2;
                    }
                    if (VehPrototypes[l_14].plan == 6) {
                        w = w * 3 / 4;
                    }
                    break;
                }
            }
        } else if (!l_124) {
            goto L_4fd8f6;
        }
        if (Weapon[VehPrototypes[l_14].weapon_id].mode > 2 && !l_124) {
            w *= 2;
        }
        if (l_e0 >= PlayersData[l_8].unk_28 * 2 / 3 && l_c != 8) {
            w = w * 2 / 3;
        }
        if (l_e0 < PlayersData[l_8].unk_28 / 2) {
            if (!l_124 && !(l_80 && l_2c == 2)) {
                w *= 2;
            }
        } else {
            if (!l_158 && l_c != 8) {
                w = w * 3 / 4;
            }
            if (l_3c && !l_124) {
                w /= 2;
            }
        }
    }

    // Defense needs (prio capped by the need), or the cost of supporting one more unit.
    // if (l_28) { ... } else { L_4fce99 (chunk 3) } ; join at L_4fcf6d (chunk 3)
    if (l_28) {
    if (l_2c != 2) {
        if (l_2c != 3) {
            // One test chain in the image (0x4fcc96): plan 0 needs the ability and the
            // base flag, plan 1 the guard condition, any other plan passes.
            if (l_2c == 0
                    ? (has_abil(l_14, 0x8000) && (BaseCurrent->state & 0x200000))
                    : (l_2c != 1
                       || ((l_28 == 3
                            || (l_5c + l_c8 >= l_f0
                                && PlayersData[l_8].region_total_offensive_vehs[l_10] >= guard_check(l_8, l_10)))
                           && (!l_c || l_c == 7) && l_5c + l_c8 >= l_f0))) {
                if (l_28 == 3
                    && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                        || (BaseCurrent->governor & 0x1000))) {
                    l_78 = 1;
                }
                goto L_4fcf6d;
            }
        } else {
            if (PlayersData[l_8].best_armor_value > 1 && l_28 < 3) {
                w *= 2;
            }
        }
    }
    if (!l_5c && l_c8 < l_f0 && VehPrototypes[l_14].plan > 1) {
        w *= 2;
    }
    if (has_abil(l_14, 0x40000) && stack_check(l_4c, 6, 0x40000, -1, -1) < l_c8 / 2 - 1) {
        w /= 2;
    }
    if (has_abil(l_14, 0x200) && stack_check(l_4c, 6, 0x200, -1, -1) < l_c8 / 2 - 1) {
        w /= 2;
    }
    if (l_28 == 3) {
        w *= (VehPrototypes[l_14].cost + 1) / 2;
        if ((l_2c == 2 || PlayersData[l_8].best_armor_value == 1)
            && (l_14 < 64 || (VehPrototypes[l_14].flags & 4))) {
            l_18 = 0;
            goto L_4fcf70;
        }
    }
    // falls into the rank cap at L_4fce6a/L_4fce6d (chunk 3)
    // ---- chunk 3: 0x4FCE6A-0x4FE99D ----
    // Unit loop, tail of the plan weighting (need == 3 handled by chunk 2): cap the rank.
    if (l_74 < 2 && l_18 >= 1 << (3 - l_28)) {
        l_18 = 1 << (3 - l_28);
    }
    goto L_4fcf70;
    } else {
    // Else-branch of chunk 2's `if (l_28)` (0x4fcc7b): no defense need.
    if (VehPrototypes[l_14].plan <= 9 && BaseCurrent->intake_2[RSC_MINERALS]
        && (VehPrototypes[l_14].plan != 7 || l_c != 7)) {
        int support = PlayersData[l_8].soc_effect_pending.values_[SocialEffect::SUPPORT];
        int cons = BaseCurrent->mineral_consumption;
        int free_units, extra;
        if (support >= 3) {
            free_units = BaseCurrent->population_size;
            extra = 1;
        } else if (support <= -4) {
            free_units = 0;
            extra = 2;
        } else {
            if (support <= -3) {
                free_units = 0;
            } else {
                free_units = (support == -2) ? 1 : support + 2;
            }
            extra = 1;
        }
        if (BaseCurrentForcesSupported >= free_units) {
            cons += extra;
        }
        int t = (l_98 - (l_70 || l_cc) - l_60 + 3) * cons;
        w = (t * t / 9 + BaseCurrent->intake_2[RSC_MINERALS]) * w
               / BaseCurrent->intake_2[RSC_MINERALS];
    }
    }
L_4fcf6d:
L_4fcf70:
    if (PlayersData[l_8].unk_77[l_10] & 0x4200) {
        if (VehPrototypes[l_14].plan == 0 || VehPrototypes[l_14].plan == 1
            || VehPrototypes[l_14].plan == 4 || VehPrototypes[l_14].plan == 11
            || VehPrototypes[l_14].plan == 5) {
            w /= 2;
            if (l_c == 2 && (PlayersData[l_8].unk_77[l_10] & 0x4000)) {
                w /= 2;
            }
        }
    }
    if (l_138 && (!l_c || l_f8 > 0)) {
        if (l_20) {
            if (l_138 > 1) {
                w *= 2;
            }
        } else if (VehPrototypes[l_14].plan < 2) {
            w = (4 - l_138) * w / 4;
        }
    }
    if (Weapon[VehPrototypes[l_14].weapon_id].mode <= 2
        && has_fac_built(FAC_BIOENHANCEMENT_CENTER)) {
        w = (l_134 + 2) * w / (l_134 + 3);
    }
    if (PlayersData[l_8].unk_66 && PlayersData[l_8].unk_66 > PlayersData[l_8].unk_106
        && PlayersData[l_8].unk_66 > PlayersData[l_8].unk_105) {
        if (has_abil(l_14, 0x120)) {
            w /= 2;
        } else if (has_abil(l_14, 0x8000)) {
            w *= 2;
        } else if (!l_20 && !l_2c) {
            w = w * 2 / 3;
        }
    }
    if (has_abil(l_14, 0x8000)) {
        if (!(PlayersData[l_8].flags_ext & 0x40)
            && !at_goal(l_8, 0x10, BaseCurrent->x, BaseCurrent->y)
            && !(BaseCurrent->state & 0x200000)) {
            l_18 += a_4bc[l_20 + l_2c * 3] + a_594[l_20 + l_2c * 3];
        }
        if (!stack_check(l_4c, 15, -1, -1, -1)) {
            if (at_goal(l_8, 0x10, BaseCurrent->x, BaseCurrent->y)
                || (BaseCurrent->state & 0x200000)) {
                w /= 3;
            } else if (BaseCurrent->event & 0x100) {
                w = w * 2 / 3;
            }
            if (l_c <= 2) {
                w = w * 2 / 3;
            }
        }
        if ((l_13c >= 0 || l_f8 >= 0) && (!stack_check(l_4c, 15, -1, -1, -1) || !l_28)) {
            w = w * 2 / 3;  // tail shared with the else branch (0x4fd242)
        }
    } else if (l_f8 >= 0 && !l_28) {
        if (!l_2c) {
            w /= 2;
        } else if (l_2c == 1) {
            w = w * 2 / 3;
        }
    } else if (l_13c >= 0 && !l_28) {
        if (!l_2c) {
            w = w * 2 / 3;
        } else if (l_2c == 1) {
            w = w * 4 / 5;
        }
    }
    if (has_abil(l_14, 8) && (l_38 || l_c == 7)) {
        w = w * 2 / 3;
    }
    if (has_abil(l_14, 0x10)) {
        if (l_c != 7 && l_c) {
            w = w * 2 / 3;
        } else {
            w /= 2;
            if (!PlayersData[l_8].proto_id_active[l_14] && !PlayersData[l_8].proto_id_queue[l_14]) {
                w /= 2;
            }
        }
    }
    if (Weapon[VehPrototypes[l_14].weapon_id].offense_rating) {
        // WORKAROUND(inline): the image calls range (0x422F00) out of line at all four
        // sites of this block; plain `range` is expanded at three of them here.
        int (__cdecl *const range_call)(int, int, int) = range;
        int worm = worm_mod(BaseIDCurrentSelected, l_8);
        int morale = morale_mod(BaseIDCurrentSelected, l_8,
                                Chassis[VehPrototypes[l_14].chassis_id].triad)
                     + PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE];
        if (l_2c == 2 && has_fac_built(FAC_CHILDREN_CRECHE)) {
            int m = morale - PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE] + 1;
            if (morale <= m) {
                morale = m;
            }
        }
        if (Weapon[VehPrototypes[l_14].weapon_id].offense_rating < 0 && l_14 < 64) {
            l_18 += 2 - range_call(worm_mod(BaseIDCurrentSelected, l_8), 0, 2);
            if (morale > worm) {
                l_18 += morale - worm;
            }
            w = w * 8 / range_call(worm - morale + 6, 1, 16);
            if (Players[l_8].rule_psi) {
                w = w * 100 / range_call(Players[l_8].rule_psi + 100, 50, 200);
            }
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                && morale >= 0 && worm < 3) {
                l_18++;
                w *= 2;
            }
        } else {
            if (l_20 != 2 && worm > morale + 1) {
                l_18++;
            }
            w = w * 8 / range_call(worm - morale + 8, 1, 16);
        }
    }
    {
        int armor = arm_strat(VehPrototypes[l_14].armor_id, l_8);
        int weap;
        if (planet_buster2(l_14)) {
            weap = 50;
        } else {
            weap = weap_strat(VehPrototypes[l_14].weapon_id, l_8);
        }
        if (l_28 && (l_2c == 2 || l_2c == 3 || (l_2c == 1 && (l_c == 2 || l_c == 8)))) {
            armor *= 2;
        }
        // BUG(original): l_4c may be -1 (no defenders), Vehs[-1] is read then.
        w = w
               / (range(bit_count(VehPrototypes[l_14].ability_flags) + weap + armor, 1, 999)
                  * (Chassis[VehPrototypes[Vehs[l_4c].proto_id].chassis_id].speed
                         / Rules.move_rate_roads
                     + 1));
    }
    if (l_cc && l_c == 7 && VehPrototypes[l_14].plan <= 1 && !l_28) {
        switch (l_7c) {
        case 1:
            w /= 2;
            break;
        case 0:
            w = w * 2 / 3;
            break;
        }
        if (!a_594[l_20 + l_2c * 3]) {
            w /= 2;
        }
    }
    if (l_68 >= 0x100) {
        w *= 2;
    }
    if (l_30 && Weapon[VehPrototypes[l_14].weapon_id].offense_rating && l_2c != 2) {
        w /= 2;
    }
    if (l_3c) {
        w *= 2;
    }
    if (l_40 && VehPrototypes[l_14].plan != 9) {
        w *= (l_2c != 2) ? 8 : 2;
    }
    if (l_64) {
        if (VehPrototypes[l_14].plan <= 1) {
            w *= 2;
        }
        if (VehPrototypes[l_14].plan >= 3) {
            w /= 2;
        }
    }
    {
        int lost = base_lose_minerals(BaseIDCurrentSelected, l_14);
        if (lost > 0
            && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                || (!out_score && (BaseCurrent->governor & 0x80000000)))
            && l_c != 2) {
            int cost = veh_cost(l_14, BaseIDCurrentSelected, NULL) * cost_factor(l_8, 1, -1);
            w = (lost + cost) * w / cost;
        }
    }
    if (l_6c == 1 && l_c != 7) {
        l_6c = 0;  // BUG(original): persists for the following units
    } else if (l_6c) {
        if (l_c > 2 && (VehPrototypes[l_14].plan != 2 || l_28)
            && (VehPrototypes[l_14].plan <= 3 || VehPrototypes[l_14].plan == 8
                || VehPrototypes[l_14].plan == 11)) {
            w /= l_6c + 1;
        } else if (VehPrototypes[l_14].plan == 7 && l_6c == 2) {
            w *= 2;
        }
    }
    l_18 *= w;
    if (!(!l_78)) {
        l_90 = 0;
        if ((a_130[1] < 90000 && l_2c != 9 && l_2c != 8) || !VehPrototypes[l_14].plan
            || l_2c == 4 || l_2c == 6) {
            goto L_4fd8f6;
        }
    } else {
        int s = l_18;
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
            // Governor permission the unit's plan needs (jump table 0x500018).
            int mask = 0;
            switch (VehPrototypes[l_14].plan) {
            case 0:
                mask = 0x200;
                break;
            case 1:
                mask = (BaseCurrent->governor & 0x200) ? 0x200 : 0x1000;
                break;
            case 2:
                mask = 0x1000;
                break;
            case 3:
                mask = 0x1000;
                if (!(BaseCurrent->governor & 0x1000) || VehPrototypes[l_14].chassis_id) {
                    mask = 0x400000;
                }
                break;
            case 4:
                mask = 0x2000;
                break;
            case 5:
                mask = 0x800;
                break;
            case 6:
                mask = 0x400;
                break;
            case 8:
                mask = 0x20000;
                break;
            case 9:
                mask = 0x8000;
                break;
            case 7:
            case 10:
                mask = 0x800000;
                break;
            case 11:
                mask = 0x100000;
                break;
            }
            if (l_14 >= 64 && !(VehPrototypes[l_14].flags & 4)) {
                mask |= 0x80000;
            }
            if ((BaseCurrent->governor & mask) != mask) goto L_4fd8f6;
        }
        if (out_score && base_claimed(l_14, l_18, l_64, l_3c, l_40, l_30)) {
            s = l_18 + 100000;
        }
        if (s <= a_114[0]) {
            if (s >= a_114[0] && ((a_130[0] < 0 && l_28 < 3) || w >= l_9c)) {
                l_18 = 999999;
            } else {
                l_9c = w;
                a_114[0] = s;
                a_130[0] = l_14;
            }
        }
    }
    if (l_18 <= a_114[(l_90 != 0) + 1]) {
        a_114[(l_90 != 0) + 1] = l_18;
        a_130[(l_90 != 0) + 1] = l_14;
    }
L_4fd8f6:
    l_14++;
    if (l_14 < 512) goto L_4fa03d;

    // Facility loop: for (l_20 = 1; l_20 < 70; l_20++); increment/test at 0x4ff11f (chunk 4).
L_4fd908:
    l_20 = 1;
    for (; l_20 < 70; l_20++) {
L_4fd912:
    if (!facility_avail(l_20, l_8, BaseIDCurrentSelected, 0)) goto L_4ff11f;
    l_14 = 0;
    l_90 = 0;
    l_6c = BaseCurrent->specialist_total - BaseCurrent->specialist_unk_1;
    bonus = l_40;
    value = 999;
    l_24 = 0x10000;
    l_34 = 0x100;
    // Jump table 0x500048 through the byte index table 0x5000d8. Bodies past 0x4fe99d are
    // chunk 4's. Cases end in `break` (the end of the switch is L_4fef49, chunk 4); the
    // image's shared tails are VC6 tail merges of identical `break` arms, which keep the
    // later copy (fusion/quantum lab, the `| l_40` 0-arm at 0x4fec33).
    switch (l_20) {
    default:
        break;

    case FAC_HEADQUARTERS:
        if (l_c != 2
            && PlayersData[l_8].soc_effect_pending.values_[SocialEffect::EFFICIENCY] > -4
            && (!l_14c || (l_14c == 1 && BaseCurrent->queue_production_id[0] == -1))) {
            value = 3 - BaseCurrent->population_size / 5;
        }
        break;

    case FAC_CHILDREN_CRECHE: {
        // Image: `12 - ineff` is formed (esi) before the morale divisor is computed.
        value = 12 - ((PlayersData[l_8].soc_effect_pending.values_[SocialEffect::EFFICIENCY] > -4)
                          ? BaseCurrent->energy_inefficiency : 0);
        int mor = PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE];
        value -= BaseCurrent->population_size * ((l_c <= 2) + 1)
                 / range((mor + (mor > 0 ? 1 : (mor < 0 ? -1 : 0)) + 6) / 2, 1, 999);
        if (!has_project(SP_CLONING_VATS, l_8) && BaseCurrent->mineral_surplus > 2
            && value > 4) {
            value = 4;
        }
        if (Players[l_8].rule_morale
                + PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE] < 0
            && l_c == 2) {
            value /= 2;
        }
        value = range(value, 1, 12);
        if (!has_fac_built(FAC_RECYCLING_TANKS) && BaseCurrent->population_size < 5) {
            break;
        }
        if (BaseCurrent->energy_inefficiency < BaseCurrent->population_size / 2
            && BaseCurrent->energy_inefficiency < BaseCurrent->intake_2[RSC_ENERGY] / 2) {
            break;
        }
        bonus = l_40 * 2;
        break;
    }

    case FAC_RECYCLING_TANKS:
    case FAC_AQUAFARM:
    case FAC_SUBSEA_TRUNKLINE:
    case FAC_THERMOCLINE_TRANSDUCER: {
        l_18 = l_60 - l_98 - l_58 + 4;
        if (l_20 != FAC_RECYCLING_TANKS) {
            l_18 -= (Players[l_8].rule_flags >> 7) & 2;
        }
        if (l_c == 7) {
            l_18--;
        } else if (l_c <= 2) {
            l_18++;
        }
        l_50 = BaseCurrent->state & 0x1000;
        if (l_50) {
            l_18 -= ResourceInfo[4].energy;
        }
        int tidal = BaseCurrent->state & 0x2000;
        if (tidal) {
            l_18 -= ResourceInfo[4].minerals;
        }
        if (l_38) {
            if ((BaseCurrent->state & 0x4000) && l_20 == FAC_AQUAFARM) {
                l_18 /= 2;
            }
            if (tidal && l_20 == FAC_SUBSEA_TRUNKLINE) {
                l_18 /= 2;
            }
            if (l_50 && l_20 == FAC_THERMOCLINE_TRANSDUCER) {
                l_18 /= 2;
            }
        }
        l_78 = 0;
        if (l_20 != FAC_RECYCLING_TANKS) {
            // l_6c holds the byte offset 4*index in the image (strength-reduced, 0..0x54).
            for (l_6c = 0; l_6c < 21; l_6c++) {
                int x = xrange(RadiusOffsetX[l_6c] + BaseCurrent->x);
                int y = RadiusOffsetY[l_6c] + BaseCurrent->y;
                if (on_map(x, y) && is_ocean(x, y)) {
                    if (l_20 == FAC_AQUAFARM) {
                        if (bit_at(x, y) & 0x8000) l_78++;
                    } else if (l_20 == FAC_SUBSEA_TRUNKLINE) {
                        if (bit_at(x, y) & 0x10) l_78++;
                    } else if (l_20 == FAC_THERMOCLINE_TRANSDUCER) {
                        if (bit_at(x, y) & 0x40) l_78++;
                    }
                }
            }
            if (!l_78) {
                value = 999;
                break;
            }
            l_18 -= l_78;
        }
        if (BaseCurrent->population_size < 5) {
            l_18--;
        }
        value = range(l_18, 1, 4);
        if (!(BaseCurrent->state & 0x1000000)
            && (l_c == 8
                || PlayersData[l_8].proto_id_active[0] + PlayersData[l_8].current_num_bases < 4)) {
            value *= 2;
        }
        if (!l_3c) break;
        if (!(BaseCurrent->state & 0x1000000) && l_c == 8) {
            goto L_4fef0c;  // the shared `bonus = 1` (chunk 4)
        }
        bonus = 2;
        break;
    }

    case FAC_PERIMETER_DEFENSE: {
        if (has_project(SP_CITIZENS_DEFENSE_FORCE, l_8)) break;
        l_90 = 1;
        int b = range(8 - BaseCurrent->population_size - (has_fac_built(FAC_HEADQUARTERS) ? 2 : 0),
                      1, 10);
        // Image: [ebp-0x68] is the induction pointer &PlayersData[i].region_total_bases[l_10].
        for (i = 1; i < 8; i++) {
            if (!has_treaty(l_8, i, 0x10)) {
                int t = has_treaty(l_8, i, 0x40020) | (PlayersData[i].integrity_blemishes >= 6);
                if (t) {
                    b -= PlayersData[i].region_total_bases[l_10] ? bit_count(t) : 1;
                    if (PlayersData[l_8].region_total_bases[l_10] == 1) {
                        b--;
                    }
                }
            }
        }
        if (PlayersData[l_8].planet_ecology >= 4) {
            b -= range(BaseCurrent->eco_damage / 10, 1, 4);
        }
        if (b < 1 && !l_28) {
            l_34 = 0x80;
        }
        value = range(b, 1, 10);
        bonus = (l_30 && l_c == 2 && !l_28) | l_40;  // 0-arm merges with chunk 4's 0x4fec33
        break;
    }

    case FAC_TACHYON_FIELD:
    case FAC_FLECHETTE_DEFENSE_SYS: {
        l_90 = 1;
        int b = range(10 - BaseCurrent->population_size / 2
                          - (has_fac_built(FAC_HEADQUARTERS) ? 2 : 0),
                      1, 10);
        for (i = 1; i < 8; i++) {
            if (!has_treaty(l_8, i, 0x10)) {
                int t = has_treaty(l_8, i, 0x40020);
                if (t) {
                    b -= (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << i))
                             ? bit_count(t) : 1;
                }
            }
        }
        if (PlayersData[l_8].planet_ecology >= 4 && l_20 != FAC_FLECHETTE_DEFENSE_SYS) {
            b -= range(BaseCurrent->eco_damage / 10, 1, 4);
        }
        if (b < 1 && !l_28) {
            l_34 = 0x80;
        }
        value = range(b, 1, 10);
        bonus = (l_30 && l_c == 2 && !l_28) | l_40;  // 0-arm merges with chunk 4's 0x4fec33
        break;
    }

    case FAC_ENERGY_BANK:
        value = range((6 - l_98) * 2 - BaseCurrent->intake_2[RSC_ENERGY] / 2, 1, 10);
        if (!has_fac_built(FAC_RECYCLING_TANKS) && BaseCurrent->population_size < 5) {
            break;
        }
        bonus = l_40 * 2;
        break;

    case FAC_HAB_COMPLEX:
    case FAC_HABITATION_DOME: {
        int lim = (l_20 == FAC_HAB_COMPLEX) ? Rules.pop_limit_sans_hab_complex
                                            : Rules.pop_limit_sans_hab_dome;
        lim -= Players[l_8].rule_population;
        if (has_project(SP_ASCETIC_VIRTUES, l_8)) {
            lim += 2;
        }
        if ((BaseCurrent->nutrient_surplus + 1) / range(Rules.nutrient_req_citizen, 1, 10)
                + BaseCurrent->population_size
            >= lim) {
            value = range(lim
                                  - (Rules.nutrient_req_citizen / 2 + BaseCurrent->nutrient_surplus)
                                        / range(Rules.nutrient_req_citizen, 1, 10)
                                  - BaseCurrent->population_size + 4,
                              1, 10);
            if (l_c > 2) {
                value /= 2;
            }
        }
        if (has_project(SP_CLONING_VATS, l_8)) {
            value /= 2;
        }
        bonus = l_40 | l_64;
        if (BaseCurrent->population_size < lim) break;
        if (!l_30) {
            bonus *= 2;
        }
        // Image: the call is followed by `jmp 0x4feda1`, a tail shared with chunk 4's
        // `if (has_project(...)) value /= 2;` (sky hydro lab).
        if (aah_ooga(l_8, l_8)) {
            value /= 2;
        }
        break;
    }

    case FAC_RECREATION_COMMONS:
        if (l_74 < 2) {
            value = (has_project(SP_VIRTUAL_WORLD, l_8) ? 6 : 7) - (l_74 + 2) * l_6c;
            bonus = l_40 | l_3c;
            if (PlayersData[l_8].diff_level > 3 && l_3c) {
                bonus = 2;
            }
            if (has_project(SP_CLONING_VATS, l_8)) {
                value /= 2;
            }
        } else {
            l_24 = 0;
            value = -10;
        }
        break;

    case FAC_HOLOGRAM_THEATRE:
        if (l_74 < 2) {
            value = 8 - (l_74 + 2) * l_6c;
            if (has_project(SP_CLONING_VATS, l_8)) {
                value /= 2;
            }
        } else {
            l_24 = 0;
            value = -8;
        }
        break;

    case FAC_RESEARCH_HOSPITAL:
        if (l_74 < 2) {
            value = range((5 - l_58 - stack_check(l_4c, 2, 12, l_8, -1)) * 2
                                  - BaseCurrent->intake_2[RSC_ENERGY] / 4,
                              1, 10);
            bonus = l_3c;
        } else {
            l_24 = 0;
            value = -4;
        }
        break;

    case FAC_NANOHOSPITAL:
        if (l_74 < 2) {
            value = range((6 - l_58 - stack_check(l_4c, 2, 12, l_8, -1)) * 2
                                  - BaseCurrent->intake_2[RSC_ENERGY] / 5,
                              1, 10);
            bonus = l_3c;
        } else {
            l_24 = 0;
            value = -2;
        }
        break;

    case FAC_PARADISE_GARDEN:
        if (l_74 < 2) {
            value = 8 - l_6c;
        } else {
            l_24 = 0;
            value = -6;
        }
        break;

    case FAC_NETWORK_NODE:
        value = range((5 - l_58 - stack_check(l_4c, 2, 12, l_8, -1)) * 2
                              - BaseCurrent->intake_2[RSC_ENERGY] / 3,
                          1, 10);
        if (has_project(SP_VIRTUAL_WORLD, l_8)) {
            if (l_74 < 2) {
                int v = 6 - (l_74 + 2) * l_6c;
                if (value >= v) {
                    value = v;
                }
            } else {
                l_24 = 0;
                value = -10;
            }
        }
        bonus = (has_project(SP_VIRTUAL_WORLD, l_8) ? 3 : 1) * l_3c;
        break;

    case FAC_BIOLOGY_LAB: {
        value = (5 - l_58 - l_a4) * 2;
        if (Players[l_8].rule_psi > 0) {
            value = value * 100 / range(Players[l_8].rule_psi + 100, 50, 200);
            l_14 = l_70;
        }
        int mor = PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE];
        if (mor < 0) {
            value = (mor - 1) / 2 + value / 2;
            l_14 = l_70;
        }
        if (l_3c
            || (l_30 && mor < 0 && l_c != 2 && !(PlayersData[l_8].unk_77[l_10] & 0x4000))) {
            goto L_4fef0c;  // shared `bonus = 1` (chunk 4)
        }
    }
    L_4fe410:
        // Shared `bonus = 0` tail. Every reference from the later cases (and chunk 4) is a
        // conditional goto: an unconditional one makes VC6 chain this block after it.
        bonus = 0;
        break;

    case FAC_SKUNKWORKS:
        value = (l_ec - l_60) * 2 + 6
                    - ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) ? 0 : l_7c)
                    + l_120;
        l_24 = 0x90000;
        if (has_fac_built(FAC_COMMAND_CENTER)) {
            value--;
        }
        if (has_fac_built(FAC_BIOENHANCEMENT_CENTER)) {
            value--;
        }
        // Image: the two stores follow the range call and the arm keeps its own
        // `mov esi, eax` (it is not merged into the psi gate's tail).
        value = PlayersData[l_8].unk_28 * value / range(l_e0, 1, 999);
        bonus = l_3c;
        l_14 = 1;
        break;

    case FAC_FUSION_LAB:
        value = range((6 - l_58) * 2 - BaseCurrent->intake_2[RSC_ENERGY] / 3, 1, 12);
        bonus = l_40 | l_3c;
        break;

    case FAC_QUANTUM_LAB:
        value = range((6 - l_58) * 2 - BaseCurrent->intake_2[RSC_ENERGY] / 4, 1, 12);
        bonus = l_40 | l_3c;
        break;

    case FAC_TREE_FARM:
        if (aah_ooga(l_8, l_8)) {
            l_84 *= 2;
        }
        value = range(12 - (l_84 + 2) / 4 - (BaseCurrent->eco_damage + 1) / 2
                              - BaseCurrent->intake_2[RSC_ENERGY] / 3,
                          1, 10);
        break;

    case FAC_HYBRID_FOREST:
        value = range(12 - (l_84 + 2) / 4 - (BaseCurrent->eco_damage + 2) / 3
                              - BaseCurrent->intake_2[RSC_ENERGY] / 4,
                          1, 10);
        break;

    case FAC_COVERT_OPS_CENTER:
        value = (2 - ((PlayersData[l_8].flags_ext >> 3) & 1)) * 5;
        break;

    case FAC_GEOSYNC_SURVEY_POD:
        value = 7;
        // BUG(original): adjusts l_4c (this base's stack id) instead of value.
        if (PlayersData[l_8].ai_growth) {
            l_4c++;
        }
        if (PlayersData[l_8].ai_tech) {
            l_4c--;
        }
        if (PlayersData[l_8].ai_wealth) {
            l_4c -= 3;
        }
        if (PlayersData[l_8].ai_power) {
            l_4c--;
        }
        break;

    case FAC_COMMAND_CENTER:
    case FAC_BIOENHANCEMENT_CENTER:
    case FAC_BROOD_PIT: {
        l_24 = 0x10200;
        l_90 = 1;
        l_14 = 1;
        if (l_20 == FAC_COMMAND_CENTER) {
            l_68 = l_11c;
        } else {
            l_68 = l_134;
        }
        value = l_120 + (l_58 - l_60) * 2 + 5;
        if (l_20 == FAC_BROOD_PIT) {
            value += 5;
        } else {
            value += l_68;
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)) {
            value -= l_7c * 2;
        }
        int v = (fac_maint(l_20, l_8) + 4) * value / 4;
        if (Players[l_8].rule_psi > 0) {
            v = (Players[l_8].rule_psi + 100) * v / 100;
        }
        if (l_20 == FAC_COMMAND_CENTER && !l_68 && l_c != 7
            && PlayersData[l_8].region_total_bases[l_10] >= 4) {
            v /= 2;
        }
        value = range(v, 1, 99);
        if (PlayersData[l_8].region_total_combat_vehs[l_10]
                < PlayersData[l_8].region_total_bases[l_10] * 2
            || (PlayersData[l_8].unk_77[l_10] & 0x4000)
            || (GameDataStateStorage.fields_.turn_current_num_ >= 100
                && PlayersData[l_8].region_total_bases[l_10] <= 8 && l_c != 7)) {
            goto L_4fe410;
        }
        bonus = l_30;
        break;
    }

    case FAC_NAVAL_YARD:
        if (naval_base(BaseIDCurrentSelected)) {
            l_24 = 0x10400;
            l_90 = 1;
            // BUG(original): base_id holds the base loop index of chunk 0 here (stale base).
            int c = (l_58 - l_7c - l_60 + l_a4) * 2 + 24 - Bases[base_id].population_size / 3
                    - PlayersData[l_8].unk_65 - l_15c / 2 + l_118;
            if (l_c == 7) {
                c /= 2;
            }
            value = range(c, 1, 24);
            if (Players[l_8].rule_flags & 0x100) {
                value /= 3;
            }
        }
        // Image: merged into chunk 4's identical tail at 0x4feef7 (spelled the same).
        if (!l_30 || l_c != 7) {
            goto L_4fe410;
        }
        bonus = 1;
        break;

    case FAC_AEROSPACE_COMPLEX: {
        l_24 = 0x10800;
        l_90 = 1;
        l_14 = 1;
        // BUG(original): base_id holds the base loop index of chunk 0 here (stale base).
        int c = (l_58 - PlayersData[l_8].unk_66 - l_7c + l_a4) * 2
                - Bases[base_id].population_size / 3 - PlayersData[l_8].unk_68
                + (8 - l_60) * 3 - l_164 / 3 + l_ac;
        if (l_c == 7) {
            c /= 2;
        }
        if (BaseCurrent->state & 0x100000) {
            c /= 2;
        }
        if (l_100 < 2) {
            c *= 2;
        }
        value = range(c, 1, 24);
        // Image: `test` here, then `jmp 0x4fef06` (chunk 4's jne L_4fe410 / bonus = 1).
        if (!l_30 || l_c == 2 || (PlayersData[l_8].unk_77[l_10] & 0x4000)) {
            goto L_4fe410;
        }
        bonus = 1;
        break;
    }

    case FAC_PRESSURE_DOME:
        if (alt_at(BaseCurrent->x, BaseCurrent->y) != 3) break;
        if (alt_detail_at(BaseCurrent->x, BaseCurrent->y) - MapSeaLevel
            < (int)AltNatural[3] + GameDataStateStorage.fields_.climate_sea_level_delta_) {
            value = -16;
            break;
        }
        // Falls through into chunk 4 (0x4fe99d), the rest of this case.
    // ---- chunk 4: 0x4FE99D-0x4FFFB8 ----
    // We are inside the facility loop (for l_20 = 1..69) and inside the
    // switch (l_20) whose dispatch is at 0x4fd97c (chunk 3). `value` is esi
    // (999 by default), `bonus` is ebx (l_40 by default), both set by chunk 3.

        // case FAC_PRESSURE_DOME (26), continued from chunk 3 (0x4fe949):
        // the base is below the current sea level.
L_4fe99d:
        if (PlayersData[l_8].flags & 0x80000) {
            value = 0;
            break;
        }
        for (i = 0; i < 64; i++) {
            if (base_project(i) == BaseIDCurrentSelected) {
                // This base holds a secret project.
                if (alt_detail_at(BaseCurrent->x, BaseCurrent->y) - MapSeaLevel
                    < (int)(WorldBuilder.sea_level_rises
                            + GameDataStateStorage.fields_.climate_sea_level_delta_
                            + AltNatural[3])) {
                    value = range(8 - BaseCurrent->population_size / 3,
                        (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) ? 4 : 2,
                        8);
                } else {
                    value = range(20 - BaseCurrent->population_size / 2,
                        (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) ? 6 : 4,
                        12);
                }
                goto L_4fef49;
            }
        }
        break;

    case FAC_SUBSPACE_GENERATOR:
        if (Players[l_8].rule_flags & RFLAG_ALIEN) {
            if (generators(l_8, NULL) < Rules.subspace_generators_needed) {
                // BUG(original): base_id is the stale base-loop index (Bases[base count]),
                // not the current base.
                if (Bases[base_id].population_size >= Rules.size_base_subspace_gen) {
                    value = 1;
                } else if (Bases[base_id].population_size >= Rules.size_base_subspace_gen * 4 / 5) {
                    value = 6;
                }
            }
        }
        break;

    case FAC_GENEJACK_FACTORY:
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
            break;
        }
        value = range((BaseCurrent->drone_total - BaseCurrent->talent_total
                       - has_fac_built(FAC_BROOD_PIT)) * 2 + 16
                      - BaseCurrent->intake_1[RSC_MINERALS]
                      - PlayersData[l_8].soc_effect_pending.values_[SocialEffect::POLICE],
                      1, 16);
        if (!l_30 || l_c <= 2) {
            goto L_4fe410;
        }
        bonus = 1;
        break;

    case FAC_PUNISHMENT_SPHERE:
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
            break;
        }
        if (PlayersData[l_8].soc_effect_pending.values_[SocialEffect::POLICE]
                + has_fac_built(FAC_BROOD_PIT) * 2 <= 0
            && BaseCurrent->drone_total >= 2
            && l_c <= 2
            && PlayersData[l_8].soc_effect_pending.values_[SocialEffect::ECONOMY] > 1
            && (BaseCurrent->assimilation_turns_left || !l_140)) {
            value = range((8 - BaseCurrent->drone_total) * 2 - BaseCurrent->population_size / 2,
                          1, 16);
        }
        bonus = l_30;
        break;

    case FAC_ROBOTIC_ASSEMBLY_PLANT:
        value = range(10 - BaseCurrent->intake_1[RSC_MINERALS], 1, 10);
        // The image materializes the condition (ebx = 1 / ebx = 0, then `or ebx, l_40`);
        // its 0-arm (0x4fec33) is the merged tail of the perimeter/tachyon cases too.
        bonus = (l_30 && l_c > 2) | l_40;
        break;

    // The image places case 21's body before case 20's; the two are identical.
    case FAC_QUANTUM_CONVERTER:
        value = range(10 - BaseCurrent->intake_1[RSC_MINERALS] / 2, 1, 10);
        break;

    case FAC_NANOREPLICATOR:
        value = range(10 - BaseCurrent->intake_1[RSC_MINERALS] / 2, 1, 10);
        break;

    case FAC_CENTAURI_PRESERVE:
    case FAC_TEMPLE_OF_PLANET:
        value = 11 - BaseCurrent->eco_damage;
        if (Players[l_8].rule_psi > 0) {
            value = value * 100 / range(Players[l_8].rule_psi + 100, 50, 200);
            l_14 = l_70;
        }
        if (PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE] < 0) {
            value = (PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE] - 1) / 2
                    + value / 2;
            l_14 = l_70;
            bonus = (l_30 && !(PlayersData[l_8].unk_77[l_10] & 0x4000)) | l_40;
        }
        value = range(value, 1, 12);
        break;

    case FAC_SKY_HYDRO_LAB:
    case FAC_NESSUS_MINING_STATION:
    case FAC_ORBITAL_POWER_TRANS:
        sat_type = l_20 - FAC_SKY_HYDRO_LAB;
        if (PlayersData[l_8].satellites_[sat_type]
            < BaseCurrent->population_size) {
            value = range((PlayersData[l_8].satellites_[sat_type]
                           + (4 - l_98) * 2 + l_60) * 2 - BaseCurrent->population_size,
                          2, 16);
            if (l_20 == FAC_SKY_HYDRO_LAB && has_project(SP_CLONING_VATS, l_8)) {
                value = value / 2;
            }
        }
        break;

    case FAC_ORBITAL_DEFENSE_POD:
        {
            int best = 1;  // ebx: largest enemy planet-buster / ODP threat
            l_24 = 0x12000;
            l_90 = 1;
            for (l_5c = 1; l_5c < 8; l_5c++) {
                if (l_5c != l_8) {
                    if (!has_treaty(l_8, l_5c, 1)) {
                        if (best <= (PlayersData[l_5c].major_atrocities
                                         ? PlayersData[l_5c].planet_busters
                                         : (PlayersData[l_5c].planet_busters + 1) / 2)) {
                            best = PlayersData[l_5c].major_atrocities
                                       ? PlayersData[l_5c].planet_busters
                                       : (PlayersData[l_5c].planet_busters + 1) / 2;
                        }
                        if (has_treaty(l_8, l_5c, 0x30)
                            || (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_
                                & (1 << l_5c))) {
                            if (best <= PlayersData[l_5c].satellites_[3]) {
                                best = PlayersData[l_5c].satellites_[3];
                            }
                        }
                    }
                }
            }
            if (PlayersData[l_8].satellites_[3] < best + 2) {
                value = range((PlayersData[l_8].satellites_[3] - best) * 2 + 12
                              - BaseCurrent->population_size, 1, 12);
                if (!PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].planet_busters
                    && !PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].major_atrocities
                    && PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].satellites_[3]
                           <= PlayersData[l_8].satellites_[3]) {
                    value *= 2;
                }
            }
        }
        l_14 = 1;
        // Shared tail; chunk 3's FAC_NAVAL_YARD case jumps here.
L_4feef7:
        if (!l_30 || l_c != 7) {
            goto L_4fe410;  // conditional, so L_4fe410 stays in the biology lab case
        }
L_4fef0c:
        bonus = 1;
        break;

    case FAC_PSI_GATE:
        value = range((climactic_battle() ? 2 : 4) - l_154 - at_climax(l_8), 0, 4);
        break;
    }   // end of switch (l_20)
    // Cross-jump merge targets in the middle of a statement (see notes): a goto
    // to one of these must already have completed its own statement.
L_4feda1:
L_4fef3e:
L_4fef3f:
L_4fef47:
L_4fef49:
    {
        int lost = base_lose_minerals(BaseIDCurrentSelected, -l_20);
        if (lost > 0
            && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                || ((BaseCurrent->governor & 0x80000000) && !out_score))) {
            int cost = cost_factor(l_8, 1, -1) * Facility[l_20].cost;
            if (cost) {
                l_34 = (lost + cost) * l_34 / cost;
            }
        }
    }
    l_18 = value * l_34 * 32 / ((l_98 - l_60 + 3) * (bonus + 1));
    if ((l_14 != 0) != (l_70 != 0) || (l_70 && l_28)) {
        l_18 *= 2;
    }
    if (l_c == 2 || (PlayersData[l_8].unk_77[l_10] & 0x4000)) {
        l_18 *= 2;
    }
    if (l_c <= 2 && !l_90) {
        l_18 *= 2;
    }
    if (l_f8 >= 0 && !l_90
        && (l_7c > 0
            || ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << l_f8))
                && (l_7c >= 0 || has_treaty(l_8, l_f8, 0x20))))) {
        l_18 *= 2;
    }
    for (i = 0; i < 2; i++) {
        int s = l_18;
        int slot;
        if (i == 0) {
            if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
                if (!(BaseCurrent->governor & 0x10000)) {
                    continue;
                }
                if ((BaseCurrent->governor & l_24) != l_24) {
                    s += 100000;
                }
                if (out_score && base_claimed(-l_20, s, l_64, l_3c, l_40, l_30)) {
                    s += 100000;
                }
            }
            slot = 0;
        } else {
            slot = l_90 ? 2 : 1;
        }
        if (s < a_114[slot]) {
            a_114[slot] = s;
            a_130[slot] = -l_20;
        }
    }
L_4ff11f:
    ;
    }   // end of the facility loop (for l_20 = 1; l_20 < 70; l_20++), opened in chunk 3

    // Secret projects.
    if (l_108) {
        goto L_4ffea4;
    }
    if ((!l_b0
         || (PlayersData[l_8].region_base_plan[l_10] <= 2
             && l_d4 >= (PlayersData[l_8].region_base_plan[l_10] != 2) + 1))
        && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            || (BaseCurrent->governor & 0xDFBE00) != 0x40000)) {
        if (PlayersData[l_8].region_base_plan[l_10] == 2
            || BaseCurrent->population_size < 3
            || l_28 >= 2
            || (l_28 && l_88 <= 1
                && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c))
            || l_e0 < BaseCurrent->population_size * 2
            || BaseCurrent->drone_total > BaseCurrent->talent_total
            || (BaseCurrent->state & 2)
            || (BaseCurrent->drone_total
                && BaseCurrent->drone_total >= BaseCurrent->talent_total
                && BaseCurrent->specialist_total > BaseCurrent->specialist_unk_1
                && BaseCurrent->population_size < 8)
            || (BaseCurrent->population_size <= (l_d8 + 1) / 2
                && l_e0 <= (PlayersData[l_8].unk_28 + 1) / 2)) {
            goto L_4ffea4;
        }
        if ((l_c == 7 ? PlayersData[l_8].current_num_bases
                      : PlayersData[l_8].region_total_bases[l_10]) - l_d4 * 3
            < (l_c == 8) - l_3c - l_58 + l_60 + 3) {
            goto L_4ffea4;
        }
    }
    // base_id (dead parameter slot) is the project index here.
    for (base_id = 0; base_id < 64; base_id++) {
        int cap;
        if (!facility_avail(base_id + FacilitySPStart, l_8, BaseIDCurrentSelected, 0)) {
            continue;
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && GameDataStateStorage.fields_.diff_level_current_ < 3
            && Facility[base_id + FacilitySPStart].preq_tech >= 0
            && !(GameDataStateStorage.fields_.game_tech_achieved_[Facility[base_id + FacilitySPStart].preq_tech]
                 & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
            continue;
        }
        l_14 = 0;
        l_84 = a_694[base_id] & l_1c;
        l_48 = a_3e4[base_id / 8];
        l_50 = l_d4 * 2 + 8
               - Facility[base_id + FacilitySPStart].sp_ai_fight * l_7c
               - Facility[base_id + FacilitySPStart].sp_ai_power * l_60
               - Facility[base_id + FacilitySPStart].sp_ai_wealth * l_98
               - Facility[base_id + FacilitySPStart].sp_ai_tech * l_58
               - Facility[base_id + FacilitySPStart].sp_ai_growth * l_a4
               - (PlayersData[l_8].ranking >= 5)
               - BaseCurrent->population_size;
        l_18 = l_148 / 2 + l_48 + (l_50 - GameDataStateStorage.fields_.turn_current_num_ / 40) + l_94;
        weight = 0x100;
        l_108 = l_40;
        if (l_3c) {
            l_108 = 2;
        }
        if (base_id == SP_PLANETARY_TRANS_SYS || base_id == SP_HUMAN_GENOME_PROJ) {
            if (l_c == 8) {
                weight = 0xAA;
            }
        }
        if (base_id == SP_HUMAN_GENOME_PROJ) {
            if (Players[l_8].rule_talent
                || PlayersData[l_8].soc_effect_pending.values_[SocialEffect::TALENT] > 0) {
                weight *= 2;
            }
        }
        if (base_id == SP_XENOEMPATYH_DOME || base_id == SP_PHOLUS_MUTAGEN
            || base_id == SP_DREAM_TWISTER || base_id == SP_NEURAL_AMPLIFIER) {
            weight = (Players[l_8].rule_morale
                      + PlayersData[l_8].soc_effect_pending.values_[SocialEffect::MORALE] + 8)
                     * weight / 8;
        }
        if (base_id == SP_HUMAN_GENOME_PROJ || base_id == SP_VIRTUAL_WORLD
            || base_id == SP_LONGEVITY_VACCINE || base_id == SP_CLINICAL_IMMORTALITY
            || base_id == SP_TELEPATHIC_MATRIX) {
            if (Players[l_8].rule_talent) {
                weight = weight * 3 / 2;
            }
            if (Players[l_8].rule_drone) {
                weight /= 2;
            }
        }
        if (base_id == SP_CITIZENS_DEFENSE_FORCE) {
            l_14 = l_70;
            for (i = 0; i < Players[l_8].faction_bonus_count; i++) {
                if ((Players[l_8].faction_bonus_id[i] == 2 && Players[l_8].faction_bonus_val1[i] == 4)
                    || (Players[l_8].faction_bonus_id[i] == 12 && Players[l_8].faction_bonus_val1[i] == 4)) {
                    break;
                }
            }
            if (i < Players[l_8].faction_bonus_count) {
                continue;
            }
        }
        if (base_id == SP_COMMAND_NEXUS || base_id == SP_CITIZENS_DEFENSE_FORCE) {
            if (PlayersData[l_8].unk_77[l_10] & 0x40) {
                weight /= 2;
            } else if (PlayersData[l_8].unk_77[l_10] & 0x20) {
                weight = weight * 2 / 3;
            }
        }
        if ((base_id == SP_MARITIME_CONTROL_CENTER || base_id == SP_WEATHER_PARADIGM) && l_c == 7) {
            weight /= 2;
        }
        if (base_id == SP_HUNTER_SEEKER_ALGO) {
            if (!PlayersData[l_8].ai_power) {
                weight /= 2;
            }
            if (PlayersData[l_8].ai_tech) {
                weight /= 2;
            }
            if (PlayersData[l_8].soc_effect_base.values_[SocialEffect::PROBE] < 0) {
                weight /= 2;
            }
            if (PlayersData[l_8].soc_effect_base.values_[SocialEffect::PROBE] > 0) {
                weight *= 2;
            }
        }
        if (base_id == SP_NETHACK_TERMINUS) {
            if (PlayersData[l_8].flags_ext & 8) {
                weight /= 2;
            }
        }
        if ((base_id == SP_PLANETARY_ENERGY_GRID || base_id == SP_MANIFOLD_HARMONICS)
            && PlayersData[l_8].ai_wealth) {
            weight /= 2;
        }
        if (base_id == SP_CLOUDBASE_ACADEMY) {
            if (PlayersData[l_8].unk_77[l_10] & 0x40) {
                weight /= 2;
            } else if (PlayersData[l_8].unk_77[l_10] & 0x20) {
                weight = weight * 2 / 3;
            }
        }
        if (base_id == SP_PLANETARY_DATALINKS) {
            // Includes the own faction.
            for (i = 1; i < 8; i++) {
                if (PlayersData[i].tech_ranking >= PlayersData[l_8].tech_ranking) {
                    weight = weight * 7 / 8;
                }
            }
        }
        if (base_id == SP_EMPATH_GUILD) {
            for (i = 1; i < 8; i++) {
                if (l_8 != i) {
                    weight = (has_treaty(l_8, i, 8) ? 9 : 7) * weight / 8;
                }
            }
            if (GameDataStateStorage.fields_.governor_faction_ == l_8) {
                weight /= 2;
            }
            if (PlayersData[l_8].ranking == 7) {
                weight /= 2;
            }
            for (i = 0; i < Players[l_8].faction_bonus_count; i++) {
                if (Players[l_8].faction_bonus_id[i] == 11) {
                    weight /= 2;
                    break;
                }
            }
        }
        if (!l_b0 && base_id < SP_VOICE_OF_PLANET
            && (l_c == 2 || (PlayersData[l_8].unk_77[l_10] & 0x4000))) {
            weight *= 2;
        }
        if (base_id == SP_VOICE_OF_PLANET && !l_dc
            && (GameDataStateStorage.fields_.game_rules_ & 0x800)) {
            weight /= 2;
            if (Facility[base_id + FacilitySPStart].preq_tech >= 0) {
                for (l_5c = 1; l_5c < 8; l_5c++) {
                    if (l_5c != l_8 && PlayersData[l_5c].ranking > PlayersData[l_8].ranking) {
                        if (!(!has_tech(Facility[base_id + FacilitySPStart].preq_tech, l_5c))) {
                            weight *= 2;
                        } else {
                            weight /= 2;
                        }
                    }
                }
            }
        }
        if (!l_48 && !l_d4) {
            weight /= 4;
        }
        if (base_id == SP_ASCENT_TO_TRANSCENDENCE && !l_dc
            && l_e0 >= PlayersData[l_8].unk_28 / 2) {
            weight /= 4;
        }
        clear();
        says(Players[l_8].filename);
        CharUpperA(StringTemp);
        if (!strcmp(StringTemp, "CARETAKE") && base_id == SP_ASCENT_TO_TRANSCENDENCE) {
            l_18 = 99;
            weight = 0x400;
        }
        l_18 = range(l_18, 1, 99);
        l_ac = l_18 * weight * 32;
        l_50 = -(base_id + FacilitySPStart);
        {
            int lost = base_lose_minerals(BaseIDCurrentSelected, l_50);
            if (lost > 0
                && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                    || (!out_score && (BaseCurrent->governor & 0x80000000)))) {
                int cost = cost_factor(l_8, 1, -1) * Facility[base_id + FacilitySPStart].cost;
                weight = (lost + cost) * weight / cost;  // no zero test here
            }
        }
        weight = (PlayersData[l_8].unk_28 + 8) * l_18 * weight * 32
                 / ((l_58 - l_60 + 3) * (l_e0 + 8));
        if (l_14 != l_70) {
            int cost = cost_factor(l_8, 1, -1) * Facility[base_id + FacilitySPStart].cost + 1;
            weight += (cost - BaseCurrent->minerals_accumulated) * weight / cost;
        }
        weight = weight * (l_84 ? 2 : l_dc + 1) * 1024 / (PlayersData[l_8].energy_reserves + 1024);
        weight = weight * 8
                 / (range(BaseCurrent->mineral_surplus
                          / (GameDataStateStorage.fields_.turn_current_num_ / 100 + 3), 0, 8) + 8);
        if (l_108) {
            weight /= l_108 + 1;
        }
        if (has_fac_built(FAC_HEADQUARTERS) && !l_84) {
            weight = weight * 3 / 4;
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
            && BaseCurrent->queue_production_id[0] == l_50) {
            l_ac /= 4;
            if (BaseCurrent->minerals_accumulated
                >= cost_factor(l_8, 1, -1) * Facility[base_id + FacilitySPStart].cost) {
                weight /= 2;
                if (l_28 < 3) {
                    l_28 = 0;
                }
            }
        }
        // Other own bases already building this project.
        for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
            if (Bases[i].faction_id_current == l_8
                && i != BaseIDCurrentSelected
                && Bases[i].queue_production_id[0] == -(base_id + FacilitySPStart)) {
                if (region_at(Bases[i].x, Bases[i].y) != l_10) {
                    goto L_4ffe94;
                }
                if (!l_84 || l_b0) {
                    weight *= 2;
                }
            }
        }
        if (weight < 0) {
            continue;
        }
        cap = 9999;
        if (!has_fac_built(FAC_HAB_COMPLEX)
            && has_tech(Facility[FAC_HAB_COMPLEX].preq_tech, l_8)) {
            cap = Rules.pop_limit_sans_hab_complex;
        } else if (!has_fac_built(FAC_HABITATION_DOME)
                   && has_tech(Facility[FAC_HABITATION_DOME].preq_tech, l_8)) {
            cap = Rules.pop_limit_sans_hab_dome;
        }
        if (BaseCurrent->population_size >= cap - Players[l_8].rule_population && !l_b0) {
            continue;
        }
        if (l_b0) {
            if (PlayersData[l_8].region_base_plan[l_10] != 2 || !l_28) {
                if (PlayersData[l_8].region_base_plan[l_10] <= 2) {
                    if (!l_28) {
                        weight /= 2;
                    }
                } else {
                    weight /= 3;
                    if (l_28 < 3 && l_74 < 2) {
                        weight /= 2;
                        if (!(BaseCurrent->state & 8)
                            && GameDataStateStorage.fields_.secret_project_.project_base[base_id] == -1
                            && a_130[0] > -FacilitySPStart) {
                            a_114[0] = 9999999;
                            l_74 = 0;
                            l_28 = 0;
                        }
                    }
                }
            }
        }
        if (l_84
            && (!l_d4 || is_ocean(BaseCurrent->x, BaseCurrent->y) || a_3c4[30] + a_2ec[30] > 2)) {
            continue;
        }
        l_d8 = base_id + FacilitySPStart;
        if (!has_fac_built(FAC_RECREATION_COMMONS)) {
            if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                || (BaseCurrent->governor & 0x10000)) {
                if (!has_tech(Facility[FAC_RECREATION_COMMONS].preq_tech, l_8)) {
                    continue;
                }
                l_d8 = FAC_RECREATION_COMMONS;
            }
        }
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c) {
            if (l_84) {
                continue;
            }
        } else if (l_84 && !veh_avail(5, l_8, BaseIDCurrentSelected)) {
            l_84 = 0;
        }
        if (a_130[0] <= -FacilitySPStart && !l_84) {
            if (l_ac < l_9c) {
                l_9c = l_ac;
                for (j = 0; j < 2; j++) {
                    int s = weight;
                    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                        || (BaseCurrent->governor & 0x40000)) {
                        if (out_score
                            && base_claimed(-(base_id + FacilitySPStart), weight, l_64, l_3c, l_40, l_30)) {
                            s = weight + 100000;
                        }
                        a_130[j] = -l_d8;
                        if (weight < a_114[j]) {
                            a_114[j] = s;
                        }
                    }
                }
            }
        } else {
            for (j = 0; j < 2; j++) {
                int s = weight;
                if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                    || (BaseCurrent->governor & 0x40000)) {
                    if (out_score
                        && base_claimed(-(base_id + FacilitySPStart), weight, l_64, l_3c, l_40, l_30)) {
                        s = weight + 100000;
                    }
                    if (s < a_114[j]) {
                        a_114[j] = s;
                        a_130[j] = l_84 ? 5 : -l_d8;
                        l_9c = l_ac;
                    }
                }
            }
        }
L_4ffe94:
        ;
    }
L_4ffea4:
    if (out_secondary) {
        *out_secondary = a_130[1];
        parse_num(1, a_114[1]);
    }
    if (out_tertiary) {
        *out_tertiary = a_130[2];
        parse_num(0, a_114[2]);
    }
    // No offensive choice: fall back to the cheapest available defensive unit when
    // the base tile is empty.
    if (a_130[0] < 0 || !Weapon[VehPrototypes[a_130[0]].weapon_id].offense_rating) {
        if (veh_who(BaseCurrent->x, BaseCurrent->y) < 0 && l_74 < 2
            && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & l_1c)
                || (BaseCurrent->governor & 0x1000))) {
            int best = 99999;
            for (i = 0; i < MaxVehProtoNum; i++) {
                if (veh_avail(i, l_8, BaseIDCurrentSelected)
                    && (VehPrototypes[i].plan == 2 || VehPrototypes[i].plan == 3
                        || VehPrototypes[i].plan == 1)
                    && (VehPrototypes[i].flags & 4)
                    && VehPrototypes[i].cost < best) {
                    best = VehPrototypes[i].cost;
                    a_130[0] = i;
                }
            }
        }
    }
    base_id = a_130[0];
    if (out_score) {
        *out_score = a_114[0];
    }
    return base_id;
}

/*
Purpose: Checks whether the facility (non-SP) has been build in the currently selected base.
// ORIGINAL: 0x00500290 ?has_fac_built@@YA_NH@Z 0x00500290-0x005002DE BYTE_EXACT
// symbol    ?has_fac_built@@YAHH@Z
// body      src/base_build.cpp
// size      78 bytes
// prototype bool (__cdecl ?has_fac_built@@YA_NH@Z)(int facilityID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
// LEVER: body in base.h hoists `int base_id = BaseIDCurrentSelected;` above the
//        ternary guard so the global is read unconditionally, before the
//        `facility_id >= FacilityRepStart` early-out - the image does the
//        same (its `mov esi,[0x689370]` precedes the `cmp/jl`). The tool's
//        agreeing-instruction count is pinned at 2/34 either way because
//        `measure` can only isolate this address by forcing /Ob0, which also
//        disables inlining the has_fac_built(facility_id,base_id) callee and
//        so never reaches the image's real bitmask-call shape; but similarity
//        at that flag set moved 0.571 -> 0.612, confirming the reorder is
//        real. Two divergences remain, not fixable from this call site: the
//        image's early return clears full `eax` (int width) where a `bool`
//        return clears only `al`, and the callee's bitmask() is a real call
//        in the image but constant-folds here - both are the documented
//        systemic gaps (return-width note above; bitmask folding TRIED
//        on has_fac base.cpp:104).
Return Value: Does current base have facility? true/false
Status: Complete
*/
// WORKAROUND(noinline): base_build is this unit's only caller and the image
// calls this body out of line at all 16 sites (its copy at 0x00500290 follows
// base_build with the other header-inline copies only base_build calls). The
// original was a header inline whose sites base_build's /Ob2 budget refused;
// defined plainly here, /Ob2 expands it. auto_inline(off) gives the same object
// as the former local function-pointer stand-in and leaves the sites spelled
// as plain calls.
#pragma auto_inline(off)
int __cdecl has_fac_built(int facility_id) {
    int base_id = BaseIDCurrentSelected;
    if (facility_id >= FacilityRepStart) {
        return false;
    }
    int mask;
    bitmask(facility_id, &facility_id, &mask);
    return (Bases[base_id].facilities_built[facility_id] & mask) != 0;
}
#pragma auto_inline(on)

/*
Purpose: Get the current status of the specified project.
// ORIGINAL: 0x005002E0 ?base_project@@YAHH@Z 0x005002E0-0x005002EF BYTE_EXACT
// body      src/base.h
// size      15 bytes
// prototype int (__cdecl ?base_project@@YAHH@Z)(int projectID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Base id, if not built (-1) or destroyed (-2)
Status: Complete
*/
// BODY IN base.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
