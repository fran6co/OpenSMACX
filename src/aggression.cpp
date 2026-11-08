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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050BA50 ?intervention@@YAXHH@Z 0x0050BA50-0x0050BCB6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050ba50.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl intervention(int target_faction, int actor_faction) {
    for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if (candidate == target_faction || candidate == actor_faction) continue;

        // LockEnableMask byte +1 (native 0x009A64E9) is the faction-alive mask.
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << candidate)) == 0) continue;
        if ((PlayersData[candidate].diplo_treaties[target_faction] & 1u) == 0) continue;
        if ((PlayersData[candidate].diplo_treaties[actor_faction] & 0x11u) != 0) continue;

        GenderDefault = Players[candidate].noun_gender;
        PluralityDefault = Players[candidate].is_noun_plural;
        parse_says(0, Players[candidate].noun_faction, -1, -1);

        GenderDefault = Players[target_faction].noun_gender;
        PluralityDefault = Players[target_faction].is_noun_plural;
        parse_says(1, Players[target_faction].noun_faction, -1, -1);

        GenderDefault = Players[candidate].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(2, Players[candidate].title_leader, -1, -1);

        GenderDefault = Players[candidate].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(3, Players[candidate].name_leader, -1, -1);

        GenderDefault = Players[actor_faction].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(4, Players[actor_faction].title_leader, -1, -1);

        GenderDefault = Players[actor_faction].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(5, Players[actor_faction].name_leader, -1, -1);

        GenderDefault = Players[actor_faction].noun_gender;
        PluralityDefault = Players[actor_faction].is_noun_plural;
        parse_says(6, Players[actor_faction].noun_faction, -1, -1);

        if (((1 << actor_faction) & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0 ||
            (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1 << target_faction)) != 0) {
            if (((1 << actor_faction) & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0) {
                PlayersData[actor_faction].diplo_spoke[candidate] = GameDataStateStorage.fields_.turn_current_num_;
            }
            treaty_on(candidate, actor_faction, 0x80000018);
            set_treaty(candidate, actor_faction, 0x40, TRUE);
        }

        if (actor_faction == ConsoleGlobal.faction_id_) {
            X_pop("AIDSTHEM", 0);
        } else if (target_faction == ConsoleGlobal.faction_id_) {
            X_pop("AIDSUS", 0);
        }
    }
}

// ORIGINAL: 0x0050BCC0 ?double_cross@@YAXHHH@Z 0x0050BCC0-0x0050C2D7
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0050bcc0.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
// TRIED: exact register-cached reuse of the 2099*a1/2099*a2 index products across the whole tail (rebuilt recomputes idxE/idxF locally at each site instead); structure (message section + three-index diplomacy-effects section, both branches of the cVarFlags&1 split) transcribed field-by-field from the disasm with parse_says/set_agenda/set_treaty/pact_ends/treaty_on/ cause_friction/great_satan/intervention argument order matched from each call's own push sequence.
void __cdecl double_cross(int faction_id, int partner_id, int victim_id) {
    if (IsMultiplayerNet) {
        if (partner_id == ConsoleGlobal.faction_id_ && victim_id > 0) {
            parse_set(Players[victim_id].is_leader_female, 0);
            parse_says(0, Players[victim_id].title_leader, -1, -1);
            parse_set(Players[victim_id].is_leader_female, 0);
            parse_says(1, Players[victim_id].name_leader, -1, -1);
            parse_set(Players[victim_id].noun_gender, Players[victim_id].is_noun_plural);
            parse_says(2, Players[victim_id].noun_faction, -1, -1);
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(3, Players[faction_id].title_leader, -1, -1);
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(4, Players[faction_id].name_leader, -1, -1);
            parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
            parse_says(5, Players[faction_id].noun_faction, -1, -1);
            NetMsgPrimary.pop("INCITED", 0x1388, 0, nullptr);
        } else if (partner_id == ConsoleGlobal.faction_id_ && is_human(faction_id)) {
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
            parse_says(2, Players[faction_id].noun_faction, -1, -1);
            NetMsgPrimary.pop("VENDETTAWARNING", 0x1388, 0, nullptr);
        }
    }
    PlayersData[partner_id].loan_balance[faction_id] = 0;
    uint32_t treaty_flags = PlayersData[faction_id].diplo_treaties[partner_id];
    int treaty_active;
    if ((treaty_flags & 0x40000) || PlayersData[partner_id].major_atrocities) {
        treaty_active = 1;
    } else {
        treaty_active = 0;
    }
    int friction_flag = 0;
    if (victim_id >= 0) {
        uint32_t victim_treaty = PlayersData[faction_id].diplo_treaties[victim_id];
        if (victim_treaty & 1) {
            if (!(treaty_flags & 1)) {
                friction_flag = 1;
            }
        } else if (victim_treaty & 2) {
            if (!(treaty_flags & 2)) {
                friction_flag = 1;
            }
        } else if (victim_treaty & 4) {
            if (!(treaty_flags & 4)) {
                friction_flag = 1;
            }
        }
        PlayersData[faction_id].diplo_unk1[victim_id]++;
    }
    if (PlayersData[faction_id].diplo_treaties[partner_id] & 1) {
        if (!(PlayersData[faction_id].diplo_agenda[partner_id] & 0x20)) {
            int reputation = PlayersData[faction_id].global_reputation - 1;
            PlayersData[faction_id].global_reputation = reputation < 0 ? 0 : reputation;
            if (PlayersData[faction_id].diff_level && !treaty_active && !friction_flag) {
                PlayersData[faction_id].integrity_blemishes++;
                PlayersData[partner_id].diplo_betrayed[faction_id]++;
            }
            if (victim_id < 0) {
                if (!treaty_active) {
                    PlayersData[faction_id].integrity_blemishes++;
                }
                PlayersData[faction_id].diplo_wrongs[partner_id]++;
                PlayersData[partner_id].diplo_betrayed[faction_id]++;
            } else {
                cause_friction(victim_id, faction_id, -4);
            }
            if (is_human(faction_id)) {
                if (PlayersData[partner_id].diplo_agenda[faction_id] & 0x400) {
                    set_agenda(partner_id, faction_id, 0x800, 1);
                }
                if (PlayersData[partner_id].diplo_treaties[faction_id] & 0x20) {
                    set_agenda(partner_id, faction_id, 0x400, 1);
                }
                set_treaty(partner_id, faction_id, 0x20, 1);
            }
        }
        pact_ends(faction_id, partner_id);
        treaty_on(faction_id, partner_id, 0x10);
        return;
    }
    if (!(PlayersData[faction_id].diplo_agenda[partner_id] & 0x20)) {
        if (PlayersData[faction_id].diplo_treaties[partner_id] & 6) {
            if (!(PlayersData[faction_id].diplo_treaties[partner_id] & 0x10000)) {
                int reputation = PlayersData[faction_id].global_reputation - 1;
                PlayersData[faction_id].global_reputation = reputation < 0 ? 0 : reputation;
                if ((great_satan(faction_id, 0)
                        || ((int)PlayersData[faction_id].ranking >= 6
                            && (int)PlayersData[partner_id].ranking <= 4))
                    && victim_id < 0 && !treaty_active) {
                    PlayersData[faction_id].integrity_blemishes++;
                    PlayersData[partner_id].diplo_betrayed[faction_id]++;
                }
            }
            if (PlayersData[faction_id].diff_level && !treaty_active && !friction_flag) {
                PlayersData[faction_id].integrity_blemishes++;
                PlayersData[partner_id].diplo_betrayed[faction_id]++;
            }
            if (victim_id < 0) {
                PlayersData[faction_id].diplo_wrongs[partner_id]++;
            }
            if (is_human(faction_id)) {
                if (PlayersData[partner_id].diplo_agenda[faction_id] & 0x400) {
                    set_agenda(partner_id, faction_id, 0x800, 1);
                }
                if (PlayersData[partner_id].diplo_treaties[faction_id] & 0x20) {
                    set_agenda(partner_id, faction_id, 0x400, 1);
                }
                set_treaty(partner_id, faction_id, 0x20, 1);
            }
            if (victim_id >= 0) {
                cause_friction(victim_id, faction_id, -2);
            }
        } else if (victim_id >= 0) {
            if (is_human(faction_id)) {
                if (PlayersData[partner_id].diplo_agenda[faction_id] & 0x400) {
                    set_agenda(partner_id, faction_id, 0x800, 1);
                }
                if (PlayersData[partner_id].diplo_treaties[faction_id] & 0x20) {
                    set_agenda(partner_id, faction_id, 0x400, 1);
                }
                set_treaty(partner_id, faction_id, 0x20, 1);
            }
            if (victim_id >= 0) {
                cause_friction(victim_id, faction_id, -2);
            }
        }
    }
    treaty_on(faction_id, partner_id, 0x10);
    if (!(PlayersData[faction_id].diplo_agenda[partner_id] & 0x20)) {
        if (is_human(faction_id) != is_human(partner_id)) {
            set_treaty(faction_id, partner_id, 0x40, 1);
            PlayersData[faction_id].diplo_spoke[partner_id] = GameDataStateStorage.fields_.turn_current_num_;
        }
        intervention(partner_id, faction_id);
    }
}

// Aggression tests the target faction's treaty with the acting faction, not
// the reverse entry. double_cross remains an unfinished dependency; this
// caller preserves its original call and the unconditional treaty update.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050C2E0 ?act_of_aggression@@YAHHH@Z 0x0050C2E0-0x0050C332 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050c2e0.cpp retained signature/extents and complete seed; no historical claim copied", "Coordinator full pinned original instruction decode 0050C2E0..0050C332, typed source owner fields and corrected original call/branch evidence", "recovery/tests/test_airdrop_network.py actual-body directional treaty and network reentrancy controls; not linked/runtime/game acceptance"], "reason": "Test target-faction treaty entry for acting faction, conditionally invoke double_cross then unconditionally treaty_on. Existing double_cross remains unfinished; original-direction lookup and service order retained."}
int __cdecl act_of_aggression(int faction_id, int target_faction_id) {
    int result = 0;
    if ((PlayersData[target_faction_id].diplo_treaties[faction_id] & 7U) != 0) {
        double_cross(faction_id, target_faction_id, -1);
        result = 1;
    }
    treaty_on(faction_id, target_faction_id, 0x8010);
    return result;
}

// ORIGINAL: 0x0050C340 ?steal_tech@@YAHHHH@Z 0x0050C340-0x0050C4AF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native instructions at 0x0050C340..0x0050C4AF: local acquisition, multiplayer return, notification and base-reset branches.","source:src/faction.h Player/PlayerData and canonical technology/network APIs; source:src/base.h Base reset API."],"reason":"Recover the complete technology acquisition path in the faction owner using real player, technology, network and base-reset APIs; retain seed lessons and original span without claiming a current byte match."}
// TRIED: writing the tech-flag bit test with an intermediate `and` result (rather than folding into the `if`'s condition) adds an extra `mov`; a ternary for the message-name select lowers to `neg/sbb/and` instead of the original's `mov`/`test`/`je` overwrite. Landed body: 0.976 mnemonic similarity, first divergence at #17 (one extra `mov`, one `lea` vs `add` in the tech-preference-table address computation), not chased further.
int __cdecl steal_tech(int faction_id, int faction_id_with,
                       int acquisition_mode) {
    if (faction_id == 0 || faction_id_with == 0) {
        return 0;
    }

    if (IsMultiplayerNet != 0 &&
        (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id)) != 0 &&
        faction_id != ConsoleGlobal.faction_id_) {
        return 0;
    }

    char *const topic = acquisition_mode != 0
        ? const_cast<char *>
// Technology selection and the stolen-technology popup use these distinct script topics.
// Bounds cover the literal payload, not surrounding allocation or lifetime.
// RECOVERY_LITERAL: v1 0x0068A334 0x0068A33E ??_C@_09HGGG@STEALTECH?$AA@ 0x0050C340
            ("STEALTECH")
        : const_cast<char *>
// RECOVERY_LITERAL: v1 0x0068A340 0x0068A34C ??_C@_0M@BIGF@ACQUIRETECH?$AA@ 0x0050C340
            ("ACQUIRETECH");
    int cost = tech_pick(faction_id, 0, faction_id_with, topic);
    if (cost < 0) {
        cost = 9999;
    }

    if (IsMultiplayerNet != 0 &&
        faction_id == ConsoleGlobal.faction_id_) {
        message_data(0x244C, 0, faction_id, cost, faction_id_with, 0);
        return cost == 9999;
    }

    if (faction_id_with == ConsoleGlobal.faction_id_ && cost != 9999) {
        GenderDefault = Players[faction_id].noun_gender;
        PluralityDefault = Players[faction_id].is_noun_plural;
        parse_says(0, Players[faction_id].noun_faction, -1, -1);
        StringTemp[0] = '\0';
        say_tech(StringTemp, cost, 0);
        parse_says(1, StringTemp, -1, -1);
// RECOVERY_LITERAL: v1 0x0068A34C 0x0068A356 ??_C@_09JAMO@STOLETECH?$AA@ 0x0050C340
        NetMsgPrimary.pop("STOLETECH", 5000, 0, NULL);
    }

    tech_achieved(faction_id, cost, faction_id_with, 0);

    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id)) == 0 && cost != 9999) {
        bases_reset(-1, faction_id, 0);
    }
    return cost == 9999;
}

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

// Base capture owns faction transfer, project notices and territorial updates.
// ORIGINAL: 0x0050C510 ?capture_base@@YAXHHH@Z 0x0050C510-0x0050DD8F
// RECOVERY_BINDING: v1 {"evidence": ["git:2fc3037a00903d280c784474ddfabc9116bba83e:src/unrecovered/0050c510.cpp: complete original identity, span and seed transcription", "src/base.h: canonical capture_base(base_id, faction_id, mode) caller contract", "Facility[70..133] owns project names; SecretProject/base_project owns the separate project base ids", "recovery/tasks/capture-base-owner-oct5.json", "recovery/tests/test_capture_base_source.py"], "reason": "Recover complete base capture through existing Base, PlayerData, Facility, SecretProject, Map, Vehicle, Console and network owners; preserve original callback ordering, arithmetic and error paths. Historical span and findings remain obligations, not a matching or runtime claim."}
// TRIED: full struct-based transcription (Base@0x97D040, PlayerData@0x96C9E0, Player@0x946A50, Map@0x94A30C, Veh@0x952828, RulesFacility@0x9A4B68, all cross-checked against base.h/faction.h/map.h/veh.h offsets and static_asserts) diverges at instruction #3 (prologue reg-save/stack layout order) - not yet matched at the frame-shape level.
void __cdecl capture_base(int base_id, int faction_id, int mode) {
    unsigned char bVar11;
    char cVar2, cVar4;
    int iVar5, iVar8, iVar10, iVar15, iVar17;
    unsigned int uVar13, uVar14;
    int uVar7, uVar20, uVar21;
    char *pcVar18;
    char *pcVar19;
    char *hqName;
    char *hqDirectionName;
    char local_174[256];
    char local_74[32];
    unsigned int local_5c;
    int local_58;
    int local_54;
    int local_50;
    unsigned int local_4c;
    int local_48;
    unsigned int local_44;
    int local_40;
    unsigned int local_3c;
    int local_38;
    int local_34;
    int local_30;
    unsigned int local_2c;
    int local_28;
    int local_1c;
    unsigned int local_18;
    int local_14;
    unsigned int local_10;
    unsigned int local_8;
    int wipeOut;
    int j, k, otherIdx, techIdx, fc, dirOff, i, f;
    int found;
    int idxTmp, maskTmp;
    int cnt3, popFactor, techVal;
    int doSkip;
    unsigned int owner2;
    int tileIdx, protoBase;
    Map *tile;
    int bestBase;

    log_say("Capture Base", Bases[base_id].name_string, base_id, faction_id, 0);
    local_8 = (unsigned int)(unsigned char)Bases[base_id].faction_id_current;
    local_1c = (int)Bases[base_id].y;
    local_3c = (unsigned int)(Bases[base_id].faction_id_former == faction_id);
    local_28 = (int)Bases[base_id].x;
    local_44 = (unsigned int)MapTiles[(MapLongitude) * local_1c + (local_28 >> 1)].region;
    local_4c = (unsigned int)faction_id * 2099u;
    cVar4 = Bases[base_id].population_size;
    local_48 = (int)((local_4c + local_8) * 4u);
    PlayersData[faction_id].diplo_unk4[local_8] = PlayersData[faction_id].diplo_unk4[local_8] + cVar4 * 2;
    PlayersData[faction_id].diplo_unk3[local_8] = PlayersData[faction_id].diplo_unk3[local_8] + cVar4 * 2;
    strcpy(local_74, Bases[base_id].name_string);
    treaty_off(faction_id, (int)local_8, 0x40);
    bVar11 = (unsigned char)faction_id;
    local_5c = 1u << (bVar11 & 0x1f);
    if (((local_5c & GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff) != 0) ||
        ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & (1u << (local_8 & 0x1f))) != 0)) {
        treaty_on(faction_id, (int)local_8, 0x2000);
    }

    uVar14 = local_8;
    for (uVar13 = 1; (int)uVar13 < 8; uVar13++) {
        if (uVar13 != (unsigned int)faction_id && uVar13 != uVar14 &&
            (PlayersData[uVar14].diplo_treaties[uVar13] & 1) != 0 &&
            (PlayersData[uVar13].diplo_treaties[faction_id] & 0x10) == 0) {
            treaty_on((int)uVar14, (int)uVar13, 0x2000);
            uVar14 = local_8;
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_) {
        iVar5 = Bases[base_id].queue_production_id[0];
        if (iVar5 < -0x45) {
            if (iVar5 < 0) iVar5 = -iVar5;
            if (base_project(iVar5 - FAC_HUMAN_GENOME_PROJ) == -1) {
                found = 0;
                if (GameDataStateStorage.fields_.base_current_count_ > 0) {
                    for (otherIdx = 0; otherIdx < GameDataStateStorage.fields_.base_current_count_; otherIdx++) {
                        if (otherIdx != base_id &&
                            Bases[otherIdx].faction_id_current == uVar14 &&
                            Bases[otherIdx].queue_production_id[0] == Bases[base_id].queue_production_id[0]) {
                            local_18 = (unsigned int)Bases[otherIdx].queue_production_id[0];
                            parse_says(0, Players[uVar14].adj_name_faction, -1, -1);
                            parse_say(1, Facility[(int)local_18].name, -1, -1);
                            popp(ScriptPopupFile, "SURVIVEPROJECT", 0, "secproj_sm.pcx", 0);
                            uVar14 = local_8;
                            found = 1;
                            break;
                        }
                    }
                }
                if (!found) {
                    PluralityDefault = 0;
                    GenderDefault = Players[uVar14].is_leader_female;
                    parse_says(0, Players[uVar14].title_leader, -1, -1);
                    GenderDefault = Players[uVar14].is_leader_female;
                    PluralityDefault = 0;
                    parse_says(1, Players[uVar14].name_leader, -1, -1);
                    GenderDefault = Players[uVar14].noun_gender;
                    PluralityDefault = Players[uVar14].is_noun_plural;
                    parse_says(2, Players[uVar14].noun_faction, -1, -1);
                    iVar5 = Bases[base_id].queue_production_id[0];
                    if (iVar5 < 0) iVar5 = -iVar5;
                    parse_say(3, Facility[iVar5].name, -1, -1);
                    popp(ScriptPopupFile, "HALTPROJECT", 0, "secproj_sm.pcx", 0);
                    iVar5 = Bases[base_id].queue_production_id[0];
                    if (iVar5 < 0) iVar5 = -iVar5;
                    bitmask(iVar5 - 0x46, &local_54, &local_58);
                    PlayersData[local_8].secret_project_intel[local_54] &= ~(unsigned int)local_58;
                    uVar14 = local_8;
                }
            }
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_ || uVar14 == (unsigned int)ConsoleGlobal.faction_id_) {
        for (int project_record_id = 0; project_record_id < 64; ++project_record_id) {
            if (base_project(project_record_id) == base_id) {
                parse_say(0, Facility[70 + project_record_id].name, -1, -1);
                if (faction_id == ConsoleGlobal.faction_id_) {
                    pcVar19 = "secproj_sm.pcx";
                    pcVar18 = "SEIZEPROJECT";
                } else {
                    pcVar19 = "secproj_sm.pcx";
                    pcVar18 = "LOSEPROJECT";
                }
                popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
            }
        }
    }

    if (PlayersData[local_8].energy_reserves < 1) {
        local_40 = 0;
    } else {
        local_40 = (Bases[base_id].population_size * PlayersData[local_8].energy_reserves) /
                   (PlayersData[local_8].pop_total + 1);
    }

    bitmask(1, &idxTmp, &maskTmp);
    local_10 = (unsigned int)((Bases[base_id].facilities_built[idxTmp] & maskTmp) != 0);
    if (local_10 != 0 && 999 < PlayersData[local_8].energy_reserves) {
        local_34 = 9999;
        local_38 = -1;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            for (local_14 = 0; local_14 < GameDataStateStorage.fields_.base_current_count_; local_14++) {
                if (local_14 != base_id && Bases[local_14].faction_id_current == (unsigned char)local_8) {
                    int aBase, bBase, hi, lo;
                    local_30 = Bases[local_14].y;
                    uVar14 = (unsigned int)(unsigned short)Bases[local_14].x;
                    local_50 = 1;
                    local_2c = (unsigned int)MapTiles[(MapLongitude) * local_30 + ((int)uVar14 >> 1)].region;
                    local_10 = uVar14;
                    if (GameDataStateStorage.fields_.base_current_count_ > 0) {
                        for (j = 0; j < GameDataStateStorage.fields_.base_current_count_; j++) {
                            if (j != local_14 && j != base_id &&
                                Bases[j].faction_id_current == (unsigned char)local_8) {
                                iVar15 = abs((int)uVar14 - (int)Bases[j].x);
                                if ((MapIsFlat & 1) == 0 && MapLongitude < iVar15) {
                                    iVar15 = MapLongitudeBounds - iVar15;
                                }
                                iVar8 = abs(local_30 - (int)Bases[j].y);
                                aBase = abs(iVar15);
                                bBase = abs(iVar8);
                                hi = (aBase > bBase) ? aBase : bBase;
                                lo = (aBase < bBase) ? aBase : bBase;
                                iVar15 = hi - (((aBase + bBase >> 1) - lo) + 1 >> 1);
                                tile = &MapTiles[(MapLongitude) * (int)Bases[j].y + ((int)Bases[j].x >> 1)];
                                if ((unsigned int)tile->region != local_2c) {
                                    iVar15 = iVar15 * 2;
                                }
                                local_50 = local_50 + iVar15;
                                uVar14 = local_10;
                            }
                        }
                    }
                    iVar8 = (local_50 << 8) /
                            (int)((unsigned int)PlayersData[local_8].region_total_bases[local_2c] +
                                  (int)Bases[local_14].population_size * 2);
                    if (iVar8 < local_34) {
                        local_38 = local_14;
                        local_34 = iVar8;
                    }
                }
            }
            if (0 < local_38) {
                parse_says(0, local_74, -1, -1);
                parse_says(1, Bases[local_38].name_string, -1, -1);
                if (IsMultiplayerNet != 0 || local_8 != (unsigned int)ConsoleGlobal.faction_id_ ||
                    X_pop("ESCAPE", 0) != 0) {
                    uVar14 = local_8;
                    PlayersData[local_8].energy_reserves -= 1000;
                    set_fac(1, local_38, 1);
                    set_fac(1, base_id, 0);
                    if (uVar14 == (unsigned int)ConsoleGlobal.faction_id_) {
                        if (IsMultiplayerNet != 0) {
                            pcVar18 = "ESCAPED2";
                            NetMsgPrimary.pop(pcVar18, 5000, 0, 0);
                        }
                    } else {
                        GenderDefault = Players[uVar14].noun_gender;
                        PluralityDefault = Players[uVar14].is_noun_plural;
                        parse_says(2, Players[uVar14].noun_faction, -1, -1);
                        pcVar18 = "ESCAPED";
                        NetMsgPrimary.pop(pcVar18, 5000, 0, 0);
                    }
                    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << (uVar14 & 0x1f))) == 0) {
                        base_reset(local_38, 0);
                    }
                }
            }
        }
    }

    bitmask(1, &idxTmp, &maskTmp);
    uVar14 = local_8;
    local_10 = (unsigned int)((Bases[base_id].facilities_built[idxTmp] & maskTmp) != 0);
    if (local_10 != 0 && GameDataStateStorage.fields_.turn_current_num_ < PlayersData[local_8].corner_market_turn &&
        0 < PlayersData[local_8].corner_market_active) {
        PlayersData[local_8].energy_reserves += PlayersData[local_8].corner_market_active;
        uVar13 = (unsigned int)ConsoleGlobal.faction_id_;
        local_40 = local_40 + PlayersData[local_8].corner_market_active / 2;
        PlayersData[local_8].corner_market_active = 0;
        PlayersData[local_8].corner_market_turn = 0;
        if (local_8 == uVar13) {
            popp(ScriptPopupFile, "CORNERFOILED", 0, "econwin_sm.pcx", 0);
        } else {
            parse_says(0, Players[local_8].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "CORNERTHEMFOILED", 0, "econwin_sm.pcx", 0);
        }
    }

    PlayersData[faction_id].energy_reserves += local_40;
    {
        int newVal = PlayersData[uVar14].energy_reserves - local_40;
        if (newVal < 0) newVal = 0;
        else if (9999999 < newVal) newVal = 9999999;
        PlayersData[uVar14].energy_reserves = newVal;
    }

    wipeOut = 0;
    if (mode == 0 && local_3c == 0 &&
        (Bases[base_id].state & 0x40000) == 0 &&
        is_objective(base_id) == 0 &&
        (Bases[base_id].population_size > 1 ||
         (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & Bases[base_id].visibility) != 0 ||
         IsMultiplayerNet != 0)) {
        cVar4 = Bases[base_id].population_size - 1;
        Bases[base_id].population_size = cVar4;
        wipeOut = (cVar4 == 0);
    }

    if (wipeOut) {
        base_kill(base_id);
        draw_map(1);
        base_id = -1;
    } else if (base_id >= 0) {
        Bases[base_id].faction_id_current = bVar11;
        for (f = 1; f < 8; f++) {
            if (PlayersData[f].base_id_atk_target == base_id) PlayersData[f].base_id_atk_target = -1;
        }
        Bases[base_id].nerve_staple_turns_left = 0;
        if (Bases[base_id].faction_id_former == faction_id) {
            Bases[base_id].assimilation_turns_left = 0;
        } else {
            Bases[base_id].faction_id_former = (unsigned char)uVar14;
            Bases[base_id].assimilation_turns_left = 0x32;
        }
        PlayersData[faction_id].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
        PlayersData[uVar14].current_num_bases -= 1;
        PlayersData[faction_id].current_num_bases += 1;
        replay_base(1, local_28, local_1c, faction_id);
        Bases[base_id].minerals_accumulated = 0;
        Bases[base_id].unk_6 = 0;
        if ((int)Bases[base_id].queue_size >= 0) {
            int *item = &Bases[base_id].queue_production_id[0];
            int cnt = (int)Bases[base_id].queue_size + 1;
            do {
                if (*item >= 0) {
                    PlayersData[uVar14].proto_id_queue[*item] -= 1;
                }
                item++;
                cnt--;
            } while (cnt != 0);
        }
        Bases[base_id].queue_size = 0;
        Bases[base_id].production_id_last = -69;
        Bases[base_id].queue_production_id[0] = -69;

        local_34 = 0;
        local_38 = -1;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            for (j = 0; j < GameDataStateStorage.fields_.base_current_count_; j++) {
                if (j != base_id && Bases[j].faction_id_current == (unsigned char)faction_id) {
                    int val = (int)Bases[j].population_size;
                    bitmask(1, &idxTmp, &maskTmp);
                    if ((Bases[j].facilities_built[idxTmp] & maskTmp) != 0) val = val * 2;
                    if (local_34 < val) { local_38 = j; local_34 = val; }
                }
            }
            if (local_38 >= 0) {
                Bases[base_id].governor = Bases[local_38].governor;
            }
        }

        Bases[base_id].state &= 0xffdef7b1u;

        if (mode == 0 && local_3c == 0) {
            for (k = 1; k < 0x46; k++) {
                if (k != 0x1a && (rand() % 3) == 0) {
                    set_fac(k, base_id, 0);
                }
            }
        }
        set_fac(1, base_id, 0);
        if (local_3c == 0) {
            set_fac(6, base_id, 0);
            set_fac(3, base_id, 0);
        }

        {
            int mapW = MapLongitudeBounds;
            for (dirOff = 0; dirOff < 0x54; dirOff += 4) {
                int dx = RadiusOffsetX[dirOff / 4];
                int tx = dx + local_28;
                int dy, ty;
                if ((MapIsFlat & 1) == 0) {
                    if (tx < 0) tx += mapW;
                    else if (mapW <= tx) tx -= mapW;
                }
                dy = RadiusOffsetY[dirOff / 4];
                ty = dy + local_1c;
                if (ty >= 0 && ty < MapLatitudeBounds && tx >= 0 && tx < mapW) {
                    using_set(tx, ty, faction_id);
                    tile = &MapTiles[(MapLongitude) * ty + (tx >> 1)];
                    tile->visibility = (unsigned char)(tile->visibility | (1u << (bVar11 & 0x1f)));
                    synch_bit(tx, ty, faction_id);
                    iVar5 = veh_at(tx, ty);
                    mapW = MapLongitudeBounds;
                    if (iVar5 >= 0 && Vehs[iVar5].faction_id == (unsigned char)faction_id) {
                        while (iVar5 >= 0) {
                            Vehs[iVar5].state &= 0xfffbffffu;
                            iVar5 = Vehs[iVar5].next_veh_id_stack;
                            mapW = MapLongitudeBounds;
                        }
                    }
                }
            }
        }

        spot_base(base_id, (int)local_8);
        reset_territory();
        for (i = GameDataStateStorage.fields_.vehicle_current_count_ - 1; i >= 0; i--) {
            if (i < GameDataStateStorage.fields_.vehicle_current_count_ && Vehs[i].faction_id == (unsigned char)local_8 &&
                Vehs[i].home_base_id == base_id) {
                int vy, dx8, dy8, killIt;
                Bases[base_id].faction_id_current = 0xff;
                iVar5 = base_find(local_28, local_1c, (int)local_8);
                Bases[base_id].faction_id_current = bVar11;
                killIt = 1;
                if (iVar5 >= 0) {
                    vy = Vehs[i].y;
                    dx8 = abs(local_28 - Vehs[i].x);
                    if ((MapIsFlat & 1) == 0 && MapLongitude < dx8) {
                        dx8 = MapLongitudeBounds - dx8;
                    }
                    dy8 = abs(local_1c - vy);
                    if (((dx8 + dy8) & 0xfffffffe) != 0) {
                        Vehs[i].home_base_id = (short)iVar5;
                        killIt = 0;
                    }
                }
                if (killIt) {
                    kill(i);
                }
            }
        }

        for (fc = 0; fc < 2; fc++) {
            int fid = (fc == 0) ? faction_id : (int)local_8;
            int cnt2 = Players[fid].faction_bonus_count;
            for (k = 0; k < cnt2; k++) {
                if (Players[fid].faction_bonus_id[k] == 2 ||
                    (Players[fid].faction_bonus_id[k] == 0xc &&
                     has_tech(Facility[Players[fid].faction_bonus_val1[k]].preq_tech, faction_id) != 0)) {
                    int fv = Players[fid].faction_bonus_val1[k];
                    if (fv < 1) fv = 1;
                    else if (fv > 0x45) fv = 0x45;
                    set_fac(fv, BaseIDCurrentSelected, 1);
                }
            }
        }
        base_first(base_id);
    }

    iVar5 = local_1c;
    iVar15 = local_28;
    owner_set(local_28, local_1c, faction_id);
    draw_map(1);
    UnkBitfield1 |= 2;
    WorldClimateWorldWin.redraw();

    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(0, Players[faction_id].noun_faction, -1, -1);
    parse_says(1, local_74, -1, -1);
    GenderDefault = Players[local_8].noun_gender;
    PluralityDefault = Players[local_8].is_noun_plural;
    parse_says(2, Players[local_8].noun_faction, -1, -1);
    parse_num(0, local_40);
    local_174[0] = 0;
    pcVar18 = "LIBERATE";
    if (local_3c == 0) pcVar18 = "SEIZE";
    strcat(local_174, pcVar18);
    strcat(local_174, "BASE");

    local_10 = 1u << (local_8 & 0x1f);
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff & local_10) != 0 &&
        (local_5c & GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff) == 0 &&
        base_id >= 0 &&
        (Bases[base_id].state & 0x80) == 0 && local_3c == 0) {
        Bases[base_id].state |= 0x10000;
    }

    if (faction_id == ConsoleGlobal.faction_id_ || local_8 == (unsigned int)ConsoleGlobal.faction_id_) {
        iVar10 = ConsoleGlobal.focus(iVar15, iVar5, ConsoleGlobal.faction_id_);
        if (iVar10 == 0) {
            draw_tile_fixup(iVar15, iVar5, 1, 2);
        }
        ConsoleGlobal.update_data(0);
        pcVar18 = "liberate_sm.pcx";
        if (faction_id != ConsoleGlobal.faction_id_) pcVar18 = "seized_sm.pcx";
        popp(ScriptPopupFile, local_174, 0, pcVar18, 0);
        ambience(0xd5);
    } else {
        draw_tile_fixup(iVar15, iVar5, 1, 2);
        if ((PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction_id] & 1) == 0 &&
            ((PlayersData[faction_id].diplo_treaties[ConsoleGlobal.faction_id_] & 0x80) == 0 ||
             (PlayersData[faction_id].diplo_treaties[local_8] & 0x40) == 0)) {
            if ((PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[local_8] & 1) == 0 &&
                ((PlayersData[local_8].diplo_treaties[ConsoleGlobal.faction_id_] & 0x80) == 0 ||
                 (PlayersData[local_8].diplo_treaties[faction_id] & 0x40) == 0)) {
                int notSpying = 0;
                iVar15 = spying(faction_id);
                if (iVar15 == 0) {
                    iVar15 = spying((int)local_8);
                    if (iVar15 == 0) notSpying = 1;
                }
                if (!notSpying) {
                    strcat(local_174, "4");
                    pcVar18 = "seized_sm.pcx";
                    popp(ScriptPopupFile, local_174, 0, pcVar18, 0);
                }
            } else {
                strcat(local_174, "3");
                pcVar18 = "seized_sm.pcx";
                popp(ScriptPopupFile, local_174, 0, pcVar18, 0);
            }
        } else {
            strcat(local_174, "2");
            pcVar18 = "liberate_sm.pcx";
            popp(ScriptPopupFile, local_174, 0, pcVar18, 0);
        }
    }

    if (((Players[local_8].rule_flags ^ Players[faction_id].rule_flags) & 0x80) != 0 && base_id >= 0 &&
        Bases[base_id].population_size >= 2) {
        PluralityDefault = Players[faction_id].is_noun_plural;
        GenderDefault = Players[faction_id].noun_gender;
        uVar14 = (unsigned int)(Bases[base_id].population_size / 2);
        local_18 = uVar14;
        parse_says(0, Players[faction_id].noun_faction, -1, -1);
        parse_says(1, local_74, -1, -1);
        parse_num(0, (int)uVar14);
        Bases[base_id].population_size = 1;
        draw_tile((int)Bases[base_id].x, (int)Bases[base_id].y, 2);

        if ((Players[faction_id].rule_flags & 0x80) == 0) {
            if (faction_id == ConsoleGlobal.faction_id_) {
                if (uVar14 == 1) {
                    pcVar19 = "alienref_sm.pcx"; pcVar18 = "HOMELESSONE";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                } else if (uVar14 != 0) {
                    pcVar19 = "alienref_sm.pcx"; pcVar18 = "HOMELESS";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                }
                interlude(0x19, Bases[base_id].name_string, 1, 0);
            }
            if (local_8 == (unsigned int)ConsoleGlobal.faction_id_) {
                if (uVar14 == 1) {
                    pcVar19 = "alienref_sm.pcx"; pcVar18 = "HOMELESSONE";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                } else if (uVar14 != 0) {
                    pcVar19 = "alienref_sm.pcx"; pcVar18 = "HOMELESS";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                }
            }
            mode = 1;
            iVar5 = MapLongitudeBounds;
        } else {
            if (faction_id == ConsoleGlobal.faction_id_) {
                if (uVar14 == 1) {
                    pcVar19 = "humanref_sm.pcx"; pcVar18 = "HOMELESSONE";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                } else if (uVar14 != 0) {
                    pcVar19 = "humanref_sm.pcx"; pcVar18 = "HOMELESS";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                }
                interlude(0x25, Bases[base_id].name_string, 1, 0);
            }
            if (local_8 == (unsigned int)ConsoleGlobal.faction_id_) {
                if (uVar14 == 1) {
                    pcVar19 = "humanref_sm.pcx"; pcVar18 = "HOMELESSONE";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                } else if (uVar14 != 0) {
                    pcVar19 = "humanref_sm.pcx"; pcVar18 = "HOMELESS";
                    popp(ScriptPopupFile, pcVar18, 0, pcVar19, 0);
                }
                interlude(0x18, Bases[base_id].name_string, 1, 0);
            }
            mode = 1;
            iVar5 = MapLongitudeBounds;
        }

        do {
            iVar10 = mode;
            if (mode < 9) {
                uVar14 = (unsigned int)rand();
                uVar14 = uVar14 & 0x80000007u;
                if ((int)uVar14 < 0) {
                    uVar14 = (uVar14 - 1) | 0xfffffff8u;
                    uVar14 = uVar14 + 1;
                }
                iVar10 = (int)uVar14 + 1;
                iVar5 = MapLongitudeBounds;
            }
            iVar15 = RadiusOffsetX[iVar10] + (int)Bases[base_id].x;
            if ((MapIsFlat & 1) == 0) {
                if (iVar15 < 0) iVar15 += iVar5;
                else if (iVar5 <= iVar15) iVar15 -= iVar5;
            }
            iVar10 = RadiusOffsetY[iVar10] + (int)Bases[base_id].y;
            if (-1 < iVar10 && iVar10 < MapLatitudeBounds && -1 < iVar15 && iVar15 < iVar5) {
                tileIdx = (MapLongitude) * iVar10 + (iVar15 >> 1);
                tile = &MapTiles[tileIdx];
                doSkip = 0;
                if ((tile->bit & 3) != 0 && (unsigned int)(tile->val2 & 0xf) < 8) {
                    if ((tile->bit & 3) == 0 || (owner2 = (unsigned int)(tile->val2 & 0xf), owner2 > 7)) {
                        owner2 = 0xffffffff;
                    }
                    iVar5 = MapLongitudeBounds;
                    if (owner2 != local_8) {
                        doSkip = 1;
                    }
                }
                if (!doSkip) {
                    protoBase = ((tile->climate & 0xe0) < 0x60) ? 0x15 : 0;
                    veh_init(protoBase, (int)local_8, iVar15, iVar10);
                    draw_tile(iVar15, iVar10, 2);
                    local_18 = local_18 - 1;
                    iVar5 = MapLongitudeBounds;
                    if (local_18 == 0) break;
                }
            }
            mode = mode + 1;
        } while (mode < 0x15);
    }

    uVar14 = local_8;
    if ((GameDataStateStorage.fields_.game_rules_ & 0x4000) == 0 && (Players[faction_id].rule_flags & 0x10) == 0) {
        cnt3 = 0;
        for (techIdx = 0; techIdx < 0x59; techIdx++) {
            if (has_tech(techIdx, (int)local_8) != 0 && has_tech(techIdx, faction_id) == 0) {
                cnt3 = cnt3 + 1;
            }
        }
        if (base_id < 0) popFactor = 1;
        else popFactor = (int)Bases[base_id].population_size + 1;
        techVal = (PlayersData[faction_id].tech_ranking + 3) * cnt3 * popFactor;
        PlayersData[faction_id].tech_accumulated += (techVal + ((techVal >> 0x1f) & 7)) >> 3;
    } else {
        steal_tech(faction_id, (int)local_8, 1);
        if (base_id >= 0 && GameDataStateStorage.fields_.secret_project_.universal_translator() == base_id) {
            steal_tech(faction_id, (int)uVar14, 1);
        }
    }

    if (((unsigned char)local_10 & (unsigned char)GameDataStateStorage.fields_.lock_masks_.enable_mask_) == 0) {
        bases_reset((int)local_8, (int)local_44, 0);
    }
    mon_conquer_base(faction_id, local_74);

    if (faction_id == ConsoleGlobal.faction_id_ && base_id >= 0 &&
        (Bases[base_id].state & 0x40000) != 0 &&
        (GameDataStateStorage.fields_.game_preferences_ & 0x40000) == 0 && IsMultiplayerNet == 0) {
        parse_says(6, Players[ConsoleGlobal.faction_id_].assistant_city, -1, -1);
        hqName = Bases[base_id].name_string;
        parse_says(5, hqName, -1, -1);
        parse_says(7, Players[local_8].adj_name_faction, -1, -1);
        cVar4 = 0;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            bestBase = -1;
            for (iVar15 = 0; iVar15 < GameDataStateStorage.fields_.base_current_count_; iVar15++) {
                if (Bases[iVar15].faction_id_current == faction_id && iVar15 != base_id) {
                    bitmask(1, &idxTmp, &maskTmp);
                    if ((Bases[iVar15].facilities_built[idxTmp] & maskTmp) != 0) {
                        parse_says(2, Bases[iVar15].name_string, -1, -1);
                        bestBase = iVar15;
                        break;
                    }
                    cVar2 = Bases[iVar15].population_size;
                    if (cVar4 < cVar2) {
                        parse_says(2, Bases[iVar15].name_string, -1, -1);
                        cVar4 = cVar2;
                    }
                }
            }
            (void)bestBase;
        }
        iVar15 = local_28;
        iVar5 = base_find(local_28, local_1c, (int)local_8);
        if (iVar5 < 0) {
            hqDirectionName = label_get(403);
        } else {
            iVar10 = (int)Bases[iVar5].x;
            iVar8 = iVar10 - iVar15;
            if (iVar8 < 0) iVar8 = iVar15 - iVar10;
            iVar17 = (int)Bases[iVar5].y;
            iVar5 = iVar17 - local_1c;
            if (iVar5 < 0) iVar5 = local_1c - iVar17;
            if ((iVar8 * 3) / 2 < iVar5) {
                if (iVar17 < local_1c) hqDirectionName = label_get(403);
                else hqDirectionName = label_get(405);
            } else {
                iVar15 = iVar15 - iVar10;
                if (MapLongitudeBounds / 2 < iVar15) iVar15 -= MapLongitudeBounds;
                if (iVar15 < -(MapLongitudeBounds / 2)) iVar15 += MapLongitudeBounds;
                if (iVar15 < 1) hqDirectionName = label_get(404);
                else hqDirectionName = label_get(406);
            }
        }
        uVar21 = -1; uVar20 = -1;
        parse_says(4, hqDirectionName, uVar20, uVar21);
        strcpy(hqName, Players[ConsoleGlobal.faction_id_].assistant_city);
        make_base_unique(base_id);
        interlude(7, 0, 1, 1);
        draw_map(1);
        text_close();
    }

    eliminate_player((int)local_8, faction_id);
    if (base_id >= 0) {
        set_base(base_id);
        base_compute(1);
        if (faction_id == ConsoleGlobal.faction_id_) {
            iVar15 = ConsoleGlobal.ready_search(-1);
            if (iVar15 < 0) {
                GameDataStateStorage.fields_.game_state_ |= 4;
                return;
            }
            TutWinBaseWindow.zoom(base_id, 0);
        }
    }
    return;
}
