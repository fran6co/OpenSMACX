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
#include "interface_palette.h"

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
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0050bcc0.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
// TRIED: exact register-cached reuse of the 2099*a1/2099*a2 index products across the whole tail (rebuilt recomputes idxE/idxF locally at each site instead); structure (message section + three-index diplomacy-effects section, both branches of the cVarFlags&1 split) transcribed field-by-field from the disasm with parse_says/set_agenda/set_treaty/pact_ends/treaty_on/ cause_friction/great_satan/intervention argument order matched from each call's own push sequence.
void __cdecl double_cross(int faction_id, int partner_id, int victim_id) {
    if (IsMultiplayerNet) {
        if (partner_id == ConsoleGlobal.faction_id_ && victim_id > 0) {
            parse_says(0, get_title(victim_id), -1, -1);
            parse_says(1, get_name(victim_id), -1, -1);
            parse_says(2, get_noun(victim_id), -1, -1);
            parse_says(3, get_title(faction_id), -1, -1);
            parse_says(4, get_name(faction_id), -1, -1);
            parse_says(5, get_noun(faction_id), -1, -1);
            NetMsgPrimary.pop("INCITED", 0x1388, 0, nullptr);
        } else if (partner_id == ConsoleGlobal.faction_id_ && is_human(faction_id)) {
            parse_says(0, get_title(faction_id), -1, -1);
            parse_says(1, get_name(faction_id), -1, -1);
            parse_says(2, get_noun(faction_id), -1, -1);
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
            int &reputation = PlayersData[faction_id].global_reputation;
            reputation = reputation - 1 < 0 ? 0 : reputation - 1;
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
                int &reputation = PlayersData[faction_id].global_reputation;
                reputation = reputation - 1 < 0 ? 0 : reputation - 1;
                if ((great_satan(faction_id, 0)
                        || (PlayersData[faction_id].ranking >= 6
                            && PlayersData[partner_id].ranking <= 4))
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
        }
        if (victim_id >= 0) {
            cause_friction(victim_id, faction_id, -2);
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
// RECOVERY_LITERAL: v1 0x0068A334 0x0068A33E $SG62913 0x0050C340
            ("STEALTECH")
        : const_cast<char *>
// RECOVERY_LITERAL: v1 0x0068A340 0x0068A34C $SG62914 0x0050C340
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
// RECOVERY_LITERAL: v1 0x0068A34C 0x0068A356 $SG62919 0x0050C340
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
// RECOVERY_BINDING: v1 {"evidence": ["git:cc76bd4a7bb6ad460724dd032c15b58553d31c8f:src/unrecovered/0050c510.cpp: complete original identity, span and seed transcription", "src/base.h: canonical capture_base(base_id, faction_id, mode) caller contract", "Facility[70..133] owns project names; SecretProject/base_project owns the separate project base ids", "recovery/tasks/capture-base-owner-oct5.json", "recovery/tests/test_capture_base_source.py"], "reason": "Recover complete base capture through existing Base, PlayerData, Facility, SecretProject, Map, Vehicle, Console and network owners; preserve original callback ordering, arithmetic and error paths. Historical span and findings remain obligations, not a matching or runtime claim."}
// TRIED: full struct-based transcription (Base@0x97D040, PlayerData@0x96C9E0, Player@0x946A50, Map@0x94A30C, Veh@0x952828, RulesFacility@0x9A4B68, all cross-checked against base.h/faction.h/map.h/veh.h offsets and static_asserts) diverges at instruction #3 (prologue reg-save/stack layout order) - not yet matched at the frame-shape level.
void __cdecl capture_base(int base_id, int faction_id, int mode) {
    char label[256];
    char base_name[32];
    int best_base;
    int best_score;

    log_say("Capture Base", Bases[base_id].name_string, base_id, faction_id, 0);
    int faction_id_def = Bases[base_id].faction_id_current;
    BOOL liberated = Bases[base_id].faction_id_former == faction_id;
    int base_x = Bases[base_id].x;
    int base_y = Bases[base_id].y;
    int region = region_at(base_x, base_y);
    PlayersData[faction_id].diplo_unk4[faction_id_def] += Bases[base_id].population_size * 2;
    PlayersData[faction_id].diplo_unk3[faction_id_def] += Bases[base_id].population_size * 2;
    strcpy(base_name, Bases[base_id].name_string);
    treaty_off(faction_id, faction_id_def, DTREATY_UNK_40);
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id)) || (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id_def))) {
        treaty_on(faction_id, faction_id_def, DTREATY_WANT_TO_TALK);
    }
    for (int player_id = 1; player_id < MaxPlayerNum; player_id++) {
        if (player_id != faction_id && player_id != faction_id_def
            && has_treaty(faction_id_def, player_id, DTREATY_PACT)
            && !has_treaty(player_id, faction_id, DTREATY_VENDETTA)) {
            treaty_on(faction_id_def, player_id, DTREATY_WANT_TO_TALK);
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_) {
        int item_id = Bases[base_id].queue_production_id[0];
        if (item_id <= -FAC_HUMAN_GENOME_PROJ) {
            if (item_id < 0) {
                item_id = -item_id;
            }
            if (base_project(item_id - FAC_HUMAN_GENOME_PROJ) == -1) {
                int survivor;
                for (survivor = 0; survivor < GameDataStateStorage.fields_.base_current_count_; survivor++) {
                    if (survivor != base_id
                        && Bases[survivor].faction_id_current == faction_id_def
                        && Bases[survivor].queue_production_id[0] == Bases[base_id].queue_production_id[0]) {
                        best_base = Bases[survivor].queue_production_id[0];
                        parse_says(0, get_adjective(faction_id_def), -1, -1);
                        parse_say(1, Facility[best_base].name, -1, -1);
                        popp(ScriptPopupFile, "SURVIVEPROJECT", 0, "secproj_sm.pcx", 0);
                        break;
                    }
                }
                if (survivor >= GameDataStateStorage.fields_.base_current_count_) {
                    parse_says(0, get_title(faction_id_def), -1, -1);
                    parse_says(1, get_name(faction_id_def), -1, -1);
                    parse_says(2, get_noun(faction_id_def), -1, -1);
                    int halted_id = Bases[base_id].queue_production_id[0];
                    if (halted_id < 0) {
                        halted_id = -halted_id;
                    }
                    parse_say(3, Facility[halted_id].name, -1, -1);
                    popp(ScriptPopupFile, "HALTPROJECT", 0, "secproj_sm.pcx", 0);
                    halted_id = Bases[base_id].queue_production_id[0];
                    if (halted_id < 0) {
                        halted_id = -halted_id;
                    }
                    int intel_offset;
                    int intel_mask;
                    bitmask(halted_id - FAC_HUMAN_GENOME_PROJ, &intel_offset, &intel_mask);
                    PlayersData[faction_id_def].secret_project_intel[intel_offset] &= ~intel_mask;
                }
            }
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_ || faction_id_def == ConsoleGlobal.faction_id_) {
        for (int project_id = 0; project_id < MaxSecretProjectNum; project_id++) {
            if (base_project(project_id) == base_id) {
                parse_say(0, Facility[FAC_HUMAN_GENOME_PROJ + project_id].name, -1, -1);
                if (faction_id == ConsoleGlobal.faction_id_) {
                    popp(ScriptPopupFile, "SEIZEPROJECT", 0, "secproj_sm.pcx", 0);
                } else {
                    popp(ScriptPopupFile, "LOSEPROJECT", 0, "secproj_sm.pcx", 0);
                }
            }
        }
    }

    int energy_stolen = steal_energy(base_id);

    if (has_fac(FAC_HEADQUARTERS, base_id, 0)
        && PlayersData[faction_id_def].energy_reserves >= 1000) {
        best_score = 9999;
        best_base = -1;
        for (int candidate = 0; candidate < GameDataStateStorage.fields_.base_current_count_; candidate++) {
            if (candidate != base_id && Bases[candidate].faction_id_current == faction_id_def) {
                int other_x = Bases[candidate].x;
                int other_y = Bases[candidate].y;
                int dist_sum = 1;
                int other_region = region_at(other_x, other_y);
                for (int neighbor = 0; neighbor < GameDataStateStorage.fields_.base_current_count_; neighbor++) {
                    if (neighbor != candidate && neighbor != base_id
                        && Bases[neighbor].faction_id_current == faction_id_def) {
                        int dist = vector_dist(other_x, other_y, Bases[neighbor].x, Bases[neighbor].y);
                        if (region_at(Bases[neighbor].x, Bases[neighbor].y) != other_region) {
                            dist *= 2;
                        }
                        dist_sum += dist;
                    }
                }
                int score = (dist_sum << 8)
                    / (PlayersData[faction_id_def].region_total_bases[other_region]
                       + Bases[candidate].population_size * 2);
                if (score < best_score) {
                    best_score = score;
                    best_base = candidate;
                }
            }
        }
        if (best_base > 0) {  // base 0 is never chosen (0x0050CB95)
            parse_says(0, base_name, -1, -1);
            parse_says(1, Bases[best_base].name_string, -1, -1);
            if (IsMultiplayerNet || faction_id_def != ConsoleGlobal.faction_id_
                || X_pop("ESCAPE", 0)) {
                PlayersData[faction_id_def].energy_reserves -= 1000;
                set_fac(FAC_HEADQUARTERS, best_base, 1);
                set_fac(FAC_HEADQUARTERS, base_id, 0);
                if (faction_id_def != ConsoleGlobal.faction_id_) {
                    parse_says(2, get_noun(faction_id_def), -1, -1);
                    NetMsgPrimary.pop("ESCAPED", 5000, 0, 0);
                } else if (IsMultiplayerNet) {
                    NetMsgPrimary.pop("ESCAPED2", 5000, 0, 0);
                }
                if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id_def))) {
                    base_reset(best_base, 0);
                }
            }
        }
    }

    if (has_fac(FAC_HEADQUARTERS, base_id, 0)
        && PlayersData[faction_id_def].corner_market_turn > GameDataStateStorage.fields_.turn_current_num_
        && PlayersData[faction_id_def].corner_market_active > 0) {
        PlayersData[faction_id_def].energy_reserves += PlayersData[faction_id_def].corner_market_active;
        energy_stolen += PlayersData[faction_id_def].corner_market_active / 2;
        PlayersData[faction_id_def].corner_market_active = 0;
        PlayersData[faction_id_def].corner_market_turn = 0;
        if (faction_id_def == ConsoleGlobal.faction_id_) {
            popp(ScriptPopupFile, "CORNERFOILED", 0, "econwin_sm.pcx", 0);
        } else {
            parse_says(0, get_adjective(faction_id_def), -1, -1);
            popp(ScriptPopupFile, "CORNERTHEMFOILED", 0, "econwin_sm.pcx", 0);
        }
    }

    PlayersData[faction_id].energy_reserves += energy_stolen;
    PlayersData[faction_id_def].energy_reserves =
        range(PlayersData[faction_id_def].energy_reserves - energy_stolen, 0, 9999999);

    if (!mode && !liberated
        && !(Bases[base_id].state & BSTATE_ASSISTANT_KILLER_HOME)
        && !is_objective(base_id)
        && (Bases[base_id].population_size > 1
            || (Bases[base_id].visibility & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
            || IsMultiplayerNet)) {
        if (!--Bases[base_id].population_size) {
            base_kill(base_id);
            draw_map(1);
            base_id = -1;
        }
    }

    if (base_id >= 0) {
        Bases[base_id].faction_id_current = faction_id;
        for (int planner = 1; planner < MaxPlayerNum; planner++) {
            if (PlayersData[planner].base_id_atk_target == base_id) {
                PlayersData[planner].base_id_atk_target = -1;
            }
        }
        Bases[base_id].nerve_staple_turns_left = 0;
        if (Bases[base_id].faction_id_former != faction_id) {
            Bases[base_id].faction_id_former = faction_id_def;
            Bases[base_id].assimilation_turns_left = 50;
        } else {
            Bases[base_id].assimilation_turns_left = 0;
        }
        PlayersData[faction_id].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
        PlayersData[faction_id_def].current_num_bases--;
        PlayersData[faction_id].current_num_bases++;
        replay_base(1, base_x, base_y, faction_id);
        Bases[base_id].minerals_accumulated = 0;
        Bases[base_id].minerals_accumulated_2 = 0;
        for (int queue_index = 0; queue_index <= Bases[base_id].queue_size; queue_index++) {
            if (Bases[base_id].queue_production_id[queue_index] >= 0) {
                PlayersData[faction_id_def].proto_id_queue[Bases[base_id].queue_production_id[queue_index]]--;
            }
        }
        Bases[base_id].queue_size = 0;
        Bases[base_id].production_id_last = -FAC_STOCKPILE_ENERGY;
        Bases[base_id].queue_production_id[0] = -FAC_STOCKPILE_ENERGY;

        best_score = 0;
        best_base = -1;
        for (int donor = 0; donor < GameDataStateStorage.fields_.base_current_count_; donor++) {
            if (donor != base_id && Bases[donor].faction_id_current == faction_id) {
                int donor_score = Bases[donor].population_size;
                if (has_fac(FAC_HEADQUARTERS, donor, 0)) {
                    donor_score *= 2;
                }
                if (donor_score > best_score) {
                    best_score = donor_score;
                    best_base = donor;
                }
            }
        }
        if (best_base >= 0) {
            Bases[base_id].governor = Bases[best_base].governor;
        }

        Bases[base_id].state &= 0xFFDEF7B1;
        if (!mode && !liberated) {
            for (int facility_id = FAC_HEADQUARTERS; facility_id < FAC_HUMAN_GENOME_PROJ; facility_id++) {
                if (facility_id != FAC_PRESSURE_DOME && !rnd(3, "CAPTURE")) {
                    set_fac(facility_id, base_id, 0);
                }
            }
        }
        set_fac(FAC_HEADQUARTERS, base_id, 0);
        if (!liberated) {
            set_fac(FAC_RECREATION_COMMONS, base_id, 0);
            set_fac(FAC_RECYCLING_TANKS, base_id, 0);
        }

        for (int radius_index = 0; radius_index < 21; radius_index++) {
            int tile_x = wrap_map_x(base_x + RadiusOffsetX[radius_index]);
            int tile_y = base_y + RadiusOffsetY[radius_index];
            if (on_map(tile_x, tile_y)) {
                using_set(tile_x, tile_y, faction_id);
                map_loc(tile_x, tile_y)->visibility |= 1 << faction_id;
                synch_bit(tile_x, tile_y, faction_id);
                int veh_id = veh_at(tile_x, tile_y);
                if (veh_id >= 0 && Vehs[veh_id].faction_id == faction_id) {
                    do {
                        Vehs[veh_id].state &= ~0x40000;
                        veh_id = Vehs[veh_id].next_veh_id_stack;
                    } while (veh_id >= 0);
                }
            }
        }

        spot_base(base_id, faction_id_def);
        reset_territory();
        for (int veh_index = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_index >= 0; veh_index--) {
            if (veh_index < GameDataStateStorage.fields_.vehicle_current_count_
                && Vehs[veh_index].faction_id == faction_id_def
                && Vehs[veh_index].home_base_id == base_id) {
                Bases[base_id].faction_id_current = 0xFF;
                int home_id = base_find(base_x, base_y, faction_id_def);
                Bases[base_id].faction_id_current = faction_id;
                if (home_id >= 0
                    && territory_dist(base_x, base_y, Vehs[veh_index].x, Vehs[veh_index].y)) {
                    Vehs[veh_index].home_base_id = home_id;
                } else {
                    kill(veh_index);
                }
            }
        }

        for (best_base = 0; best_base < Players[faction_id].faction_bonus_count; best_base++) {
            if (Players[faction_id].faction_bonus_id[best_base] == RULE_FACILITY
                || (Players[faction_id].faction_bonus_id[best_base] == RULE_FREEFAC
                    && has_tech(Facility[Players[faction_id].faction_bonus_val1[best_base]].preq_tech, faction_id))) {
                set_fac(range(Players[faction_id].faction_bonus_val1[best_base],
                              FAC_HEADQUARTERS, FAC_STOCKPILE_ENERGY),
                        BaseIDCurrentSelected, 1);
            }
        }
        for (best_base = 0; best_base < Players[faction_id_def].faction_bonus_count; best_base++) {
            if (Players[faction_id_def].faction_bonus_id[best_base] == RULE_FACILITY
                || (Players[faction_id_def].faction_bonus_id[best_base] == RULE_FREEFAC
                    && has_tech(Facility[Players[faction_id_def].faction_bonus_val1[best_base]].preq_tech, faction_id))) {
                set_fac(range(Players[faction_id_def].faction_bonus_val1[best_base],
                              FAC_HEADQUARTERS, FAC_STOCKPILE_ENERGY),
                        BaseIDCurrentSelected, 1);
            }
        }
        base_first(base_id);
    }

    owner_set(base_x, base_y, faction_id);
    draw_map(1);
    WorldClimateWorldWin.field_239BC_ |= 2;
    WorldClimateWorldWin.redraw();
    parse_says(0, get_noun(faction_id), -1, -1);
    parse_says(1, base_name, -1, -1);
    parse_says(2, get_noun(faction_id_def), -1, -1);
    parse_num(0, energy_stolen);
    label[0] = '\0';
    strcat(label, liberated ? "LIBERATE" : "SEIZE");
    strcat(label, "BASE");
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id_def)) && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id)) && base_id >= 0
        && !(Bases[base_id].state & BSTATE_UNK_80) && !liberated) {
        Bases[base_id].state |= BSTATE_UNK_10000;
    }

    if (faction_id == ConsoleGlobal.faction_id_ || faction_id_def == ConsoleGlobal.faction_id_) {
        if (!ConsoleGlobal.focus(base_x, base_y, ConsoleGlobal.faction_id_)) {
            draw_tile_fixup(base_x, base_y, 1, 2);
        }
        ConsoleGlobal.update_data(0);
        popp(ScriptPopupFile, label, 0,
             faction_id == ConsoleGlobal.faction_id_ ? "liberate_sm.pcx" : "seized_sm.pcx", 0);
        ambience(0xD5);
    } else {
        draw_tile_fixup(base_x, base_y, 1, 2);
        if (has_treaty(ConsoleGlobal.faction_id_, faction_id, DTREATY_PACT)
            || (has_treaty(faction_id, ConsoleGlobal.faction_id_, DTREATY_UNK_80)
                && has_treaty(faction_id, faction_id_def, DTREATY_UNK_40))) {
            strcat(label, "2");
            popp(ScriptPopupFile, label, 0, "liberate_sm.pcx", 0);
        } else if (has_treaty(ConsoleGlobal.faction_id_, faction_id_def, DTREATY_PACT)
                   || (has_treaty(faction_id_def, ConsoleGlobal.faction_id_, DTREATY_UNK_80)
                       && has_treaty(faction_id_def, faction_id, DTREATY_UNK_40))) {
            strcat(label, "3");
            popp(ScriptPopupFile, label, 0, "seized_sm.pcx", 0);
        } else if (spying(faction_id) || spying(faction_id_def)) {
            strcat(label, "4");
            popp(ScriptPopupFile, label, 0, "seized_sm.pcx", 0);
        }
    }

    if (((Players[faction_id].rule_flags ^ Players[faction_id_def].rule_flags) & RFLAG_ALIEN)
        && base_id >= 0 && Bases[base_id].population_size > 1) {
        int refugees = Bases[base_id].population_size / 2;
        parse_says(0, get_noun(faction_id), -1, -1);
        parse_says(1, base_name, -1, -1);
        parse_num(0, refugees);
        Bases[base_id].population_size = 1;
        draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            if (faction_id == ConsoleGlobal.faction_id_) {
                if (refugees == 1) {
                    popp(ScriptPopupFile, "HOMELESSONE", 0, "humanref_sm.pcx", 0);
                } else if (refugees) {
                    popp(ScriptPopupFile, "HOMELESS", 0, "humanref_sm.pcx", 0);
                }
                interlude(0x25, Bases[base_id].name_string, 1, 0);
            }
            if (faction_id_def == ConsoleGlobal.faction_id_) {
                if (refugees == 1) {
                    popp(ScriptPopupFile, "HOMELESSONE", 0, "humanref_sm.pcx", 0);
                } else if (refugees) {
                    popp(ScriptPopupFile, "HOMELESS", 0, "humanref_sm.pcx", 0);
                }
                interlude(0x18, Bases[base_id].name_string, 1, 0);
            }
        } else {
            if (faction_id == ConsoleGlobal.faction_id_) {
                if (refugees == 1) {
                    popp(ScriptPopupFile, "HOMELESSONE", 0, "alienref_sm.pcx", 0);
                } else if (refugees) {
                    popp(ScriptPopupFile, "HOMELESS", 0, "alienref_sm.pcx", 0);
                }
                interlude(0x19, Bases[base_id].name_string, 1, 0);
            }
            if (faction_id_def == ConsoleGlobal.faction_id_) {
                if (refugees == 1) {
                    popp(ScriptPopupFile, "HOMELESSONE", 0, "alienref_sm.pcx", 0);
                } else if (refugees) {
                    popp(ScriptPopupFile, "HOMELESS", 0, "alienref_sm.pcx", 0);
                }
            }
        }
        for (int spot = 1; spot < 21; spot++) {
            int offset_index = spot < 9 ? rand() % 8 + 1 : spot;
            int tile_x = wrap_map_x(Bases[base_id].x + RadiusOffsetX[offset_index]);
            int tile_y = Bases[base_id].y + RadiusOffsetY[offset_index];
            if (on_map(tile_x, tile_y)) {
                if (anything_at(tile_x, tile_y) < 0 || anything_at(tile_x, tile_y) == faction_id_def) {
                    if (is_ocean(tile_x, tile_y)) {
                        veh_init(BSC_SEA_ESCAPE_POD, faction_id_def, tile_x, tile_y);
                        draw_tile(tile_x, tile_y, 2);
                    } else {
                        veh_init(BSC_COLONY_POD, faction_id_def, tile_x, tile_y);
                        draw_tile(tile_x, tile_y, 2);
                    }
                    if (!--refugees) {
                        break;
                    }
                }
            }
        }
    }

    if (!(GameDataStateStorage.fields_.game_rules_ & RULES_SPOILS_OF_WAR)
        && !(Players[faction_id].rule_flags & RFLAG_TECHSTEAL)) {
        int tech_count = 0;
        for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
            if (has_tech(tech_id, faction_id_def) && !has_tech(tech_id, faction_id)) {
                tech_count++;
            }
        }
        int multiplier = base_id >= 0 ? Bases[base_id].population_size + 1 : 1;
        PlayersData[faction_id].tech_accumulated +=
            (PlayersData[faction_id].tech_ranking + 3) * tech_count * multiplier / 8;
    } else {
        steal_tech(faction_id, faction_id_def, 1);
        if (base_id >= 0
            && GameDataStateStorage.fields_.secret_project_.universal_translator() == base_id) {
            steal_tech(faction_id, faction_id_def, 1);
        }
    }

    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id_def))) {
        bases_reset(faction_id_def, region, 0);
    }
    mon_conquer_base(faction_id, base_name);

    if (faction_id == ConsoleGlobal.faction_id_ && base_id >= 0
        && (Bases[base_id].state & BSTATE_ASSISTANT_KILLER_HOME)
        && !(GameDataStateStorage.fields_.game_preferences_ & PREF_AV_INTERLUDES_DISABLED)
        && !IsMultiplayerNet) {
        parse_says(6, Players[ConsoleGlobal.faction_id_].assistant_city, -1, -1);
        char *hq_name = Bases[base_id].name_string;
        parse_says(5, hq_name, -1, -1);
        parse_says(7, get_adjective(faction_id_def), -1, -1);
        int best_pop = 0;
        for (int hq_candidate = 0; hq_candidate < GameDataStateStorage.fields_.base_current_count_; hq_candidate++) {
            if (Bases[hq_candidate].faction_id_current == faction_id && hq_candidate != base_id) {
                if (has_fac(FAC_HEADQUARTERS, hq_candidate, 0)) {
                    parse_says(2, Bases[hq_candidate].name_string, -1, -1);
                    break;
                }
                int candidate_pop = Bases[hq_candidate].population_size;
                if (candidate_pop > best_pop) {
                    parse_says(2, Bases[hq_candidate].name_string, -1, -1);
                    best_pop = candidate_pop;
                }
            }
        }
        int near_id = base_find(base_x, base_y, faction_id_def);
        if (near_id >= 0) {
            int near_x = Bases[near_id].x;
            int dx = near_x - base_x;
            if (dx < 0) {
                dx = base_x - near_x;
            }
            int near_y = Bases[near_id].y;
            int dy = near_y - base_y;
            if (dy < 0) {
                dy = base_y - near_y;
            }
            if (dx * 3 / 2 >= dy) {
                int wrap_dx = base_x - near_x;
                if (wrap_dx > MapLongitudeBounds / 2) {
                    wrap_dx -= MapLongitudeBounds;
                }
                if (wrap_dx < -(MapLongitudeBounds / 2)) {
                    wrap_dx += MapLongitudeBounds;
                }
                if (wrap_dx > 0) {
                    parse_says(4, StringTable.get(Labels.strings_ptr[406]), -1, -1);
                } else {
                    parse_says(4, StringTable.get(Labels.strings_ptr[404]), -1, -1);
                }
            } else if (base_y > near_y) {
                parse_says(4, StringTable.get(Labels.strings_ptr[403]), -1, -1);
            } else {
                parse_says(4, StringTable.get(Labels.strings_ptr[405]), -1, -1);
            }
        } else {
            parse_says(4, label_get(403), -1, -1);
        }
        strcpy(hq_name, Players[ConsoleGlobal.faction_id_].assistant_city);
        make_base_unique(base_id);
        interlude(7, 0, 1, 1);
        draw_map(1);
        text_close();
    }

    eliminate_player(faction_id_def, faction_id);
    if (base_id >= 0) {
        set_base(base_id);
        base_compute(1);
        if (faction_id == ConsoleGlobal.faction_id_) {
            if (ConsoleGlobal.ready_search(-1) < 0) {
                GameDataStateStorage.fields_.game_state_ |= 4;
                return;
            }
            TutWinBaseWindow.zoom(base_id, 0);
        }
    }
}

// Pump redraws before non-input work at least once. The original captures the
// WINMM timeGetTime import once and compares unsigned elapsed time, including
// wraparound and negative arguments interpreted as unsigned durations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050DD90 ?clock_wait@@YAXH@Z 0x0050DD90-0x0050DDBA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/0050dd90.cpp retained exact seed and original extent; preserved without copying historical claim", "Pinned PE import669368 resolves WINMM.dll timeGetTime; full disassembly50DD90..50DDBA captures import once, calls5FCBB0 then5FCA30 and uses unsigned jb elapsed compare"], "reason": "Home cooperative unsigned elapsed-time wait to timer integration; preserve import capture, redraw/non-input ordering, mandatory first iteration and modular DWORD subtraction, without fixed import address."}
void __cdecl clock_wait(int milliseconds) {
    DWORD (WINAPI *const read_clock)() = timeGetTime;
    const DWORD started = read_clock();
    do {
        do_all_draws();
        do_non_input();
    } while (read_clock() - started < static_cast<DWORD>(milliseconds));
}

/*
Purpose: Check whether there is a path between two regions. It seems to only take into account land
         destinations. TODO: Revisit in the future when Continent/Path is complete.
// ORIGINAL: 0x0050DDC0 ?sea_coast@@YAHHH@Z 0x0050DDC0-0x0050DE00 BYTE_EXACT
// LEVER: two spellings, both visible in the image's own body. (1) the
//            offset out-param is the PARAMETER ITSELF - `bitmask_call(...,
//            &region_src, &mask)`, then `sea_coasts[region_src]` reads it
//            back - not a separate `int offset` local. The image's
//            `lea ecx, [ebp + 0xc]` is `&region_src`, and its second read of
//            [ebp+0xc] at 0x0050DDB is the offset coming back out of
//            bitmask. (2) the call is `bitmask_call`, the real E8 - at /Ob2
//            the header `inline bitmask` expands at this site and the whole
//            body collapses to 24 instructions with no call at all.
// size      64 bytes
// prototype int (__cdecl ?sea_coast@@YAHHH@Z)(int regionDst, int regionSrc)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready
// calls     0x0050BA00
Return Value: Is there a path? true/false
Status: Complete
*/
BOOL __cdecl sea_coast(int region_dst, int region_src) {
    int mask;
    bitmask(region_src & RegionBounds, &region_src, &mask);
    return (Continents[region_dst].sea_coasts[region_src] & mask) != 0;
}

/*
Purpose: Count the number of paths from the source region. It seems to only take into account land 
         source and destination ranges. TODO: Revisit in the future when Continent/Path is complete.
// ORIGINAL: 0x0050DE00 ?sea_coasts@@YAHH@Z 0x0050DE00-0x0050DE49 BYTE_EXACT
// size      73 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Sea coasts valid path count
Status: Complete
*/
int __cdecl sea_coasts(int region_src) {
    uint32_t sea_coast_count = 0;
    for (int i = 1; i < RegionBounds; i++) {
        // sea_coast() (0x0050DDC0, BYTE_EXACT as its own out-of-line
        // function) is hand-inlined here: the image writes its body out at
        // this call site (bitmask() is the only call it keeps), rather than
        // calling 0x0050DDC0.
        int offset;
        int mask;
        bitmask(region_src & RegionBounds, &offset, &mask);
        if ((Continents[i].sea_coasts[offset] & mask) != 0) {
            sea_coast_count++;
        }
    }
    return sea_coast_count;
}

/*
Purpose: Check to see whether base is within a one tile radius of a sea tile with specified region.
         If you pass a land region (<63) as the 2nd parameter, it is possible to get collision
         behavior due to region bounding. TODO: Revisit in the future to see whether to remove them.
// ORIGINAL: 0x0050DE50 ?base_on_sea@@YAHHH@Z 0x0050DE50-0x0050DF28 BYTE_EXACT
// TRIED: --all-flags's winner (23/81, the default set) already beats every other set;
//            first divergence is the prologue's two globals (MapIsFlat at 0x94988c,
//            MapLongitudeBounds at 0x949870) loading into ebx/edi in the opposite order
//            from the image - same register-allocation swap seen on base_coast
//            (0x0050DF30) and the rest of this xrange/on_map-loop family
//            (bonus_at, goody_at already carry the same note).
// size      216 bytes
// prototype int (__cdecl ?base_on_sea@@YAHHH@Z)(int baseID, int region)
// callers   10   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is base connected to specified sea region? true/false
Status: Complete
*/
BOOL __cdecl base_on_sea(int base_id, int region_sea) {
    region_sea &= RegionBounds;
    if (region_sea >= RegionBounds) { // change to equals since already bounded?
        return false; // skips poles (land or ocean)
    }
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    for (uint32_t i = 0; i < 8; i++) {
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)
            && (region_at(x_radius, y_radius) & RegionBounds) == region_sea) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Determine the ocean region for coastal bases. There is an issue if a base is connected to
         more than one ocean region, it will only return the last checked region based on the base
         radius clockwise order. This could cause the AI to make incorrect calculations for what to
         prioritize building at the base. An example could be a base that is connected to a small
         water body as well as the much larger ocean. Depending on where these bodies of water are
         positioned, the AI might make assumptions not to prioritize building naval units. Also,
         the Continents compare logic isn't used by anything. This might be the root cause of
         outlined bug. TODO: Revisit in the future once more is known about Continent structure.
// ORIGINAL: 0x0050DF30 ?base_coast@@YAHH@Z 0x0050DF30-0x0050E021 BYTE_EXACT
// TRIED: 42/81 plateau, same xrange/on_map-loop family as base_on_sea
//            (0x0050DE50). First divergence is inside the inlined `xrange`:
//            the image loads MapLongitudeBounds ONCE and reuses that
//            register for both the `+=` and `-=` wrap arms, this tree
//            reloads it from memory for the `-=` arm, shifting every
//            address after it by 4 bytes and cascading the mismatch
//            through the rest of the loop body (which otherwise agrees,
//            per `listing_diff`: only 6 runs differ across 81
//            instructions). Tried hand-inlining `xrange`/`on_map` instead
//            of calling them - no change, confirming this is `xrange`'s own
//            shared register-allocation ceiling, not something local to
//            this call site.
// size      241 bytes
// prototype int (__cdecl ?base_coast@@YAHH@Z)(int baseID)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Ocean region or -1 if landlocked
Status: Complete
*/
int __cdecl base_coast(int base_id) {
    int region = -1;
    int val = 0;
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    for (int i = 0; i < 8; i++) { // is_coast()
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            region = region_at(x_radius, y_radius);
            int compare = Continents[region].tile_count;
            if (region >= 127) {
                compare = 1;
            }
            if (compare >= val) {
                val = compare; // value isn't used?
            }
            i += (2 - (i & 1)); // skips adjacent tiles
        }
    }
    return region;
}

/*
Purpose: Check to see if a port base shares a common body of water with destination coastal region.
// ORIGINAL: 0x0050E030 ?port_to_coast@@YAHHH@Z 0x0050E030-0x0050E15A BYTE_EXACT
// size      298 bytes
// prototype int (__cdecl ?port_to_coast@@YAHHH@Z)(int baseID, int region)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Is port and coastal region accessible by water to each other? true/false
Status: Complete
*/
// TRIED: best 77/103, 0.922 similar (best flag set /O2 /Gy /GR- /Oy-
//            /GX), MISMATCH at every flag set. The remaining gaps are all
//            `region_at`'s inlined `map_loc()->region` read: the image
//            schedules `xor ecx, ecx` (or the byte-index add) before loading
//            the map_tiles() pointer, this tree always loads the pointer
//            first - same register-scheduling plateau as alt_at/alt_detail_at
//            (0x00500150/0x00500180) and abstract_at (0x00591210), inherited
//            here because `region_at` is called four times over the body.
//            Not chased further.
BOOL __cdecl port_to_coast(int base_id, int region) {
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    if (region == region_at(x, y)) {
        return true;
    }
    for (int i = 0; i < 8; i++) { // is_coast()
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            // sea_coast() (0x0050DDC0, BYTE_EXACT as its own out-of-line
            // function) is hand-inlined here: the image writes its body out
            // at this call site (bitmask() is the only call it keeps),
            // rather than calling 0x0050DDC0.
            int offset;
            int mask;
            bitmask(region_at(x_radius, y_radius) & RegionBounds, &offset, &mask);
            if ((Continents[region].sea_coasts[offset] & mask) != 0) {
                return true;
            }
            i += (2 - (i & 1)); // skips adjacent tiles
        }
    }
    return false;
}

static __forceinline BOOL __cdecl port_to_port_body(int base_id_src, int base_id_dst) {
    int last_region = -1;
    int x = Bases[base_id_src].x;
    int y = Bases[base_id_src].y;
    uint32_t i = 0;
    for (;;) { // is_coast()
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            int region_src = region_at(x_radius, y_radius);
            if (region_src != last_region) { // reduce redundant checks especially sea bases
                last_region = region_src;
                // Inlined base_on_sea(base_id_dst, region_src): the image
                // writes this out here rather than calling it.
                int region_sea = region_src & RegionBounds;
                if (region_sea < RegionBounds) {
                    int x_dst = Bases[base_id_dst].x;
                    int y_dst = Bases[base_id_dst].y;
                    for (uint32_t j = 0; j < 8; j++) {
                        int x_radius_dst = wrap_map_x(x_dst + VehMoveDeltaX[j]);
                        int y_radius_dst = y_dst + VehMoveDeltaY[j];
                        if (on_map(x_radius_dst, y_radius_dst) && is_ocean(x_radius_dst, y_radius_dst)
                            && (region_at(x_radius_dst, y_radius_dst) & RegionBounds) == region_sea) {
                            return true;
                        }
                    }
                }
            }
        }
        if (++i >= 8) {
            return false;
        }
    }
}

/*
Purpose: Check to see if two port bases share a common body of water determined by region.
// ORIGINAL: 0x0050E160 ?port_to_port@@YAHHH@Z 0x0050E160-0x0050E306 BYTE_EXACT
// LEVER: call_diff showed this tree calling base_on_sea where the image makes
//        no calls at all - the image inlines base_on_sea's whole body into
//        the loop instead. Manually inlining it (guard clause on
//        `region_sea >= RegionBounds`, then the destination-base radius
//        loop) took this from 5/141 to 12/141 instructions agreeing
//        (0.587 -> 0.785 similarity); still MISMATCH, the remaining gap
//        looks like register/stack-slot allocation (image reserves
//        `sub esp, 0x18`, this tree `0x10`) rather than another structural
//        difference.
// TRIED: the same inline rewritten as early-return guard clauses
//            (`if (!on_map(...)) continue;` etc.) instead of nested `if` -
//            scored identically (12/141), so the remaining gap is not
//            branch polarity.
// size      422 bytes
// prototype int (__cdecl ?port_to_port@@YAHHH@Z)(int baseIDSrc, int baseIDDst)
// callers   7   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER (2026-08-21): moved to map.h as `MEASURED inline`. naval_base (0x0050E3C0) and
//        convoy (0x0050E5C0) call_diff MORE by exactly one call each on port_to_port -
//        the image inlines this whole 422-byte body at both, while still keeping a
//        real call at get_there and veh.cpp's valid_patrol (both already call_diff-clean
//        and stayed that way after this move).
Return Value: Are both ports accessible by water to each other? true/false
Status: Complete
*/
BOOL __cdecl port_to_port(int base_id_src, int base_id_dst) {
    return port_to_port_body(base_id_src, base_id_dst);
}

/*
Purpose: Determine if a base has access to ports or more than one coastal region. This helps
         prioritize whether naval transports should be built.
// ORIGINAL: 0x0050E310 ?transport_base@@YAHH@Z 0x0050E310-0x0050E3BD BYTE_EXACT
// LEVER: catalogue lists `sea_coasts` as callers=1, and this wasn't it - the
//        image hand-inlines `sea_coasts`'s whole loop at this call site
//        (same shape as `sea_coasts`'s own comment about `sea_coast()`)
//        instead of calling 0x0050DE00. Went 2/67 -> 42/67 (0.889 similar)
//        inlining that loop instead of calling `sea_coasts(region)`.
// TRIED: the remaining gap is is_ocean's own known plateau - the image
//            loads map_tiles() (0x94a30c) earlier, before the Bases[].x/y
//            reads, than this tree schedules it. Same root cause as
//            is_ocean's standalone note, not something local to this site.
// size      173 bytes
// prototype int (__cdecl ?transport_base@@YAHH@Z)(int baseID)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00 0x0050DF30
Return Value: Should base build naval transports? true/false
Status: Complete
*/
BOOL __cdecl transport_base(int base_id) {
    int region = base_coast(base_id);
    if (region < 0) {
        return false;
    }
    if (is_ocean(Bases[base_id].x, Bases[base_id].y)) {
        return true;
    }
    // `sea_coasts` (0x0050DE00, BYTE_EXACT as its own out-of-line function)
    // is hand-inlined here: the image writes its whole loop body out at this
    // call site (bitmask() is the only call it keeps), rather than calling
    // 0x0050DE00.
    int sea_coast_count = 0;
    for (int i = 1; i < RegionBounds; i++) {
        int offset;
        int mask;
        bitmask(region & RegionBounds, &offset, &mask);
        if ((Continents[i].sea_coasts[offset] & mask) != 0) {
            sea_coast_count++;
        }
    }
    return sea_coast_count > 1;
}

/*
Purpose: Determine if there are other faction's ports in the vicinity of the specified base.
// ORIGINAL: 0x0050E3C0 ?naval_base@@YAHH@Z 0x0050E3C0-0x0050E5BE
// TRIED: call count already matches the image (1 call, to base_coast;
//            port_to_port makes 0 calls here, same as the image - `call_diff`
//            agrees). The plateau (11/164, 0.894 similar, best of every flag
//            set) is inherited from `port_to_port`'s own already-documented
//            MISMATCH ceiling (12/141, register/stack-slot allocation, not
//            branch shape) - here it is inlined a SECOND time, inside the
//            `BaseCurrentCount` loop, compounding the same divergence.
// size      510 bytes
// prototype int (__cdecl ?naval_base@@YAHH@Z)(int baseID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050DF30
Return Value: Does base have a strategic naval importance? true/false
Status: Complete
*/
BOOL __cdecl naval_base(int base_id) {
    if (base_coast(base_id) < 0) {
        return false;
    }
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current != Bases[base_id].faction_id_current
            && port_to_port(base_id, i)) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Determine if specified unit can set up a convoy route with specified base.
// ORIGINAL: 0x0050E5C0 ?convoy@@YAHHH@Z 0x0050E5C0-0x0050E81C BYTE_EXACT
// TRIED: call count already matches the image (0 calls - `port_to_port`
//            fully inlines, same as the image; `call_diff` agrees). Best of
//            every flag set is the default (18/200, 0.514 similar) - same
//            inherited `port_to_port` inlining plateau as `naval_base`,
//            not something specific to this call site's own control flow.
// size      604 bytes
// prototype int (__cdecl ?convoy@@YAHHH@Z)(int vehID, int baseID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is a convoy route possible? true/false
Status: Complete
*/
BOOL __cdecl convoy(int veh_id, int base_id) {
    int home_base_id = Vehs[veh_id].home_base_id;
    if (home_base_id < 0 || home_base_id == base_id) {
        return false;
    }
    uint8_t triad = get_triad(veh_id);
    if (triad == TRIAD_AIR) {
        return true;
    }
    int region_base = region_at(Bases[base_id].x, Bases[base_id].y);
    if (region_base == region_at(Bases[home_base_id].x, Bases[home_base_id].y)
        && ((region_base >= 64) == (get_triad(veh_id) == TRIAD_SEA))) {
        return true; // same region, by land or sea
    }
    if (triad) {
        return port_to_port(base_id, home_base_id); // sea
    }
    return false;
}

// Homed from src/map_color.cpp (2026-09-02): its leaf-test isolation
// rationale died with the leaf suite, the same retirement that freed
// basepop_font.cpp. The mandate row belongs to TextColourTable[162..168].
/*
Purpose: Return the display colour for a mandate selector. Selectors 0, 1, and 2
         map to the first three table entries; any other value returns the
         fourth. The original decrements the selector twice against zero, so the
         branch structure is preserved for exactness.
// ORIGINAL: 0x0050E820 ?mandate_color@@YAHH@Z 0x0050E820-0x0050E84D BYTE_EXACT
// LEVER: a `switch`, not a chain of `if`s. The image's dispatch is `sub eax, 0 / je` then `dec eax / je` twice - VC6's small-switch lowering, which keeps the selector live in eax and lays the arms out in reverse after the chain. Three separate `if (mandate == N)` compare `[ebp+8]` against an immediate at every arm and scored 5 of 21; the switch scores 21 of 21.
// size      45 bytes
// prototype int (__cdecl ?mandate_color@@YAHH@Z)(int category)
// callers   9   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Colour dword from the mandate colour table
Status: Complete
*/
int __cdecl mandate_color(int mandate) {
    switch (mandate) {
    case 0:
        return TextColourTable[162];
    case 1:
        return TextColourTable[164];
    case 2:
        return TextColourTable[166];
    default:
        return TextColourTable[168];
    }
}
