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

// Diplomacy script keys and resource paths formerly borrowed from fixed image
// string addresses. Keep one writable source-owned object for each distinct
// original address, including the several break_sm.pcx copies.
// RECOVERY_OBJECT: v2 0x0068D444 0x0068D456 ?DiploLabelSendingTREATYON@@3PADA 0x005391C0
char DiploLabelSendingTREATYON[] = "Sending TREATY ON"; // 0x0068D444
// RECOVERY_OBJECT: v2 0x0068D458 0x0068D46B ?DiploLabelSendingTREATYOFF@@3PADA 0x00539230
char DiploLabelSendingTREATYOFF[] = "Sending TREATY OFF"; // 0x0068D458
// RECOVERY_OBJECT: v2 0x0068D46C 0x0068D47F ?DiploLabelSendingSETTREATY@@3PADA 0x005392A0
char DiploLabelSendingSETTREATY[] = "Sending SET TREATY"; // 0x0068D46C
// RECOVERY_OBJECT: v2 0x0068D480 0x0068D492 ?DiploLabelSendingAGENDAON@@3PADA 0x00539310
char DiploLabelSendingAGENDAON[] = "Sending AGENDA ON"; // 0x0068D480
// RECOVERY_OBJECT: v2 0x0068D494 0x0068D4A7 ?DiploLabelSendingAGENDAOFF@@3PADA 0x00539380
char DiploLabelSendingAGENDAOFF[] = "Sending AGENDA OFF"; // 0x0068D494
// RECOVERY_OBJECT: v2 0x0068D4A8 0x0068D4BB ?DiploLabelSendingSETAGENDA@@3PADA 0x005393F0
char DiploLabelSendingSETAGENDA[] = "Sending SET AGENDA"; // 0x0068D4A8
// RECOVERY_OBJECT: v2 0x0068D4BC 0x0068D4CB ?DiploLabelSendingENERGY@@3PADA 0x00539460
char DiploLabelSendingENERGY[] = "Sending ENERGY"; // 0x0068D4BC
// RECOVERY_OBJECT: v2 0x0068D4CC 0x0068D4D9 ?DiploLabelSendingLOAN@@3PADA 0x00539510
char DiploLabelSendingLOAN[] = "Sending LOAN"; // 0x0068D4CC
// RECOVERY_OBJECT: v2 0x0068D4DC 0x0068D4EF ?DiploLabelSendingTRADEMAPS@@3PADA 0x00539580
char DiploLabelSendingTRADEMAPS[] = "Sending TRADE MAPS"; // 0x0068D4DC
// RECOVERY_OBJECT: v2 0x0068D4F0 0x0068D4FD ?DiploLabelSendingTECH@@3PADA 0x005395F0
char DiploLabelSendingTECH[] = "Sending TECH"; // 0x0068D4F0
// RECOVERY_OBJECT: v2 0x0068D500 0x0068D512 ?DiploLabelSendingPACTENDS@@3PADA 0x00539660
char DiploLabelSendingPACTENDS[] = "Sending PACT ENDS"; // 0x0068D500
// RECOVERY_OBJECT: v2 0x0068D514 0x0068D527 ?DiploLabelSendingWITHDRAWAL@@3PADA 0x005396D0
char DiploLabelSendingWITHDRAWAL[] = "Sending WITHDRAWAL"; // 0x0068D514
// RECOVERY_OBJECT: v2 0x0068D528 0x0068D53A ?DiploLabelSendingCEDEBASE@@3PADA 0x00539740
char DiploLabelSendingCEDEBASE[] = "Sending CEDE BASE"; // 0x0068D528
// RECOVERY_OBJECT: v2 0x0068D53C 0x0068D551 ?DiploLabelSendingDOUBLECROSS@@3PADA 0x005397B0
char DiploLabelSendingDOUBLECROSS[] = "Sending DOUBLE CROSS"; // 0x0068D53C
// RECOVERY_OBJECT: v2 0x0068D554 0x0068D56C ?DiploLabelGettingDIPLOMATICLOCK@@3PADA 0x0053E5C0
char DiploLabelGettingDIPLOMATICLOCK[] = "Getting DIPLOMATIC LOCK"; // 0x0068D554
// RECOVERY_OBJECT: v2 0x0068D56C 0x0068D577 ?DiploLabelNEEDGLOBAL@@3PADA 0x0053E5C0
char DiploLabelNEEDGLOBAL[] = "NEEDGLOBAL"; // 0x0068D56C
// RECOVERY_OBJECT: v2 0x0068D578 0x0068D592 ?DiploLabelReleasingDIPLOMATICLOCK@@3PADA 0x005398C0
char DiploLabelReleasingDIPLOMATICLOCK[] = "Releasing DIPLOMATIC LOCK"; // 0x0068D578

char *__cdecl get_him_her(int faction_id, int sentence_form);
void __cdecl interlude(int id, char *filename, int arg3, int arg4);
int __cdecl base_init(int faction, int x, int y);
void __cdecl base_kill(int base);
void __cdecl kill(int vehicle);
void __cdecl clear_bunglist(int faction);
int __cdecl veh_init(int prototype, int faction, int x, int y);
void __cdecl veh_kill(int vehicle);
int __cdecl veh_find(int x, int y, int faction, int exclude);
extern int DiploScratchFA98;  // Diplomacy producer storage defined below.

// ORIGINAL: 0x0053EB50 ?buy_council_vote@@YAXHHHH@Z 0x0053EB50-0x00540193;0x0065E75A-0x0065EB33
// RECOVERY_BINDING: v1 {"evidence": ["Pinned53EB50..540193 and65E75A..65EB33; git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0053eb50.cpp", "Native EDI=OTHER and EBX=FACTION; peer vote query precedes OTHER query; technology valuation rereads turn after every call; promised gift and treaty state reload after modal/parser callbacks. Popup teardown is ordinary lexical destruction, without duplicate close.", "RulesTechnology.name is the heap string address returned by Strings::put, not an ordinal; parse_say takes that address through its native32-bit integer ABI. Dialogue labels verified against bounded original strings and owned as C++ literals.", "recovery/tasks/interface-contract-report-closure.json; recovery/tests/test_council_vote_purchase.py actual-source corruption controls"], "reason": "Recover complete council vote purchasing in its genuine faction owner, including the custom modal negotiation missing from the retained seed. Use lexical Popup and source-owned labels, actual player/council/network storage and calls; preserve original spans, retained seed and all historical obligations. This is not matching or game acceptance."}
// TRIED: plain `Popup popup;` local (compiler-generated dtor order diverges from original's manual close()+~Scroll()/~BasePop() split-unwind pattern); tail dialog/ListBox/EditGroup/SpriteBox/ CheckBox construction (menuResult<1 branch) is a raw vtable- poke sequence into untyped stack buffers (aiStack_3220 etc.) that was not reproduced - only the surrounding call order was.
void __cdecl buy_council_vote(int faction, int other, int proposal, int choice) {
    Popup popup;

    PopupDialogFactionID = other;
    diplomacy_check(faction, other, 0);
    diplomacy_caption(faction, other);
    tech_analysis(faction, other);

    if (ExpansionEnabled) {
        if (is_alien_faction(faction) || is_alien_faction(other)) {
            return;
        }
    }

    uint32_t &treaty = PlayersData[other].diplo_treaties[faction];
    if (treaty & 0x40030) {
        if (TurnRedrawPending != 0) {
            PluralityDefault = 0;
            GenderDefault = Players[other].is_leader_female;
            parse_says(0, Players[other].name_leader, -1, -1);
            GenderDefault = Players[other].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[other].title_leader, -1, -1);
            X_pop("BUYNOTALK", pop_wait);
        }
        return;
    }

    int votes = wants_prop(other, proposal, choice < 1 ? faction : choice);
    if (proposal != 1 && DiploScratchFA98 != 0) votes = diplo_add(votes, 1);
    if (choice == -2) votes = diplo_sub(0, votes);
    int margin = votes;
    if (proposal == 0) {
        for (int player = 1; player < MaxPlayerNum; ++player) {
            if (player != choice && eligible(player) != 0) {
                const int candidate = diplo_sub(votes, wants_prop(other, 0, player));
                if (candidate <= margin) {
                    // Native reevaluates the winning/tied candidate instead
                    // of reusing the comparison call's result.
                    margin = diplo_sub(votes, wants_prop(other, 0, player));
                }
            }
        }
    }

    int &recorded_vote = PlayersData[other].unk_29[proposal];
    int &gift_faction = PlayersData[other].unk_30[proposal];
    if (recorded_vote != 0 &&
        (PlayersData[other].diplo_treaties[gift_faction] & 0x30) == 0) {
        PluralityDefault = 0;
        GenderDefault = Players[gift_faction].is_leader_female;
        parse_says(0, Players[gift_faction].name_leader, -1, -1);
        GenderDefault = Players[gift_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[gift_faction].title_leader, -1, -1);
        X_pops("BUYPROMISED", 0x100000,
                     FactionDiplomacyPortraits[other], pop_wait);
        return;
    }

    if (PlayersData[other].soc_effect_base.values_[SocialEffect::ECONOMY] > 0 && proposal > 1 &&
        (treaty & 0x20) == 0 && margin < -2) {
        margin = -2;
    }

    if (TurnRedrawPending != 0 &&
        choice == static_cast<int>(CouncilWindow.leader_label_kind_[other])) {
        PluralityDefault = 0;
        GenderDefault = Players[faction].is_leader_female;
        parse_says(0, Players[faction].name_leader, -1, -1);
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction].title_leader, -1, -1);
        X_pop("BUYALREADY", pop_wait);
        if (IsMultiplayerNet != 0 &&
            (PlayersData[faction].diplo_treaties[other] & 2) != 0 &&
            (treaty & 0x20) == 0) {
            recorded_vote = choice;
            gift_faction = proposal == 0 ? choice : faction;
            synch_leader(other);
        }
        return;
    }

    const bool fixed_vote = (treaty & 1) != 0 &&
                            (treaty & 0x2000000) != 0;
    int count = 0;
    if (fixed_vote) {
        votes = 0;
        if (proposal == 0) {
            for (int player = 1; player < MaxPlayerNum; ++player) {
                if (player != other) {
                    const int player_votes = council_votes(player);
                    const int other_votes = council_votes(other);
                    if (player_votes > other_votes) ++count;
                }
            }
        }
    } else {
        if (proposal == 0 && choice == other) {
            if (margin < 0) {
                X_pops("BUYNO", 0x100000,
                             FactionDiplomacyPortraits[other], pop_wait);
            }
            return;
        }

        if (margin < -3) {
            X_pops("BUYNO", 0x100000,
                         FactionDiplomacyPortraits[other], pop_wait);
            return;
        }

        if (margin == -3) {
            if (proposal != 0) {
                choice = -3;
                votes = 3;
            } else {
                votes = 4;
                for (int player = 1; player < MaxPlayerNum; ++player) {
                    if (player != other) {
                        const int player_votes = council_votes(player);
                        const int other_votes = council_votes(other);
                        if (player_votes > other_votes) ++count;
                    }
                }
            }
        } else if (margin == -2) {
            votes = 3;
            if (proposal == 0) {
                for (int player = 1; player < MaxPlayerNum; ++player) {
                    if (player != other) {
                        const int player_votes = council_votes(player);
                        const int other_votes = council_votes(other);
                        if (player_votes > other_votes) ++count;
                    }
                }
            }
        } else {
            votes = margin == -1 ? 2 : 1;
            if (proposal == 0) {
                for (int player = 1; player < MaxPlayerNum; ++player) {
                    if (player != other) {
                        const int player_votes = council_votes(player);
                        const int other_votes = council_votes(other);
                        if (player_votes > other_votes) ++count;
                    }
                }
            }
        }
    }

    int spend_self = diplo_sub(PlayersData[faction].unk_13,
                               static_cast<int>(PlayersData[faction].maint_cost_total));
    const int minimum_self = PlayersData[faction].current_num_bases;
    if (spend_self < minimum_self) {
        spend_self = minimum_self;
    }
    const int minimum_other = PlayersData[other].current_num_bases;
    int spend_other = diplo_sub(PlayersData[other].unk_13,
                                static_cast<int>(PlayersData[other].maint_cost_total));
    if (spend_other < minimum_other) spend_other = minimum_other;

    votes = diplo_add(PlayersData[faction].energy_reserves / 5,
            diplo_mul(diplo_mul(diplo_add(spend_other, spend_self), votes), 2));
    if (DiploScratchFA98 != 0) votes /= 2;
    if (minimum_other <= DiploScratchFA98) votes /= 2;
    if (other == GameDataStateStorage.fields_.governor_faction_) votes = diplo_mul(votes, 2);
    if (TurnRedrawPending == 0) {
        const int three_votes = diplo_mul(votes, 3);
        votes = diplo_add(three_votes, ((three_votes >> 31) & 3)) >> 2;
    }
    if (proposal == 0 && count == 0 &&
        static_cast<int>(CouncilWindow.leader_label_kind_[other]) == other)
        votes = diplo_mul(votes, 2);

    if (DiploScratchFAA8 >= 0) {
            const int three_votes = diplo_mul(votes, 3);
            const int need = diplo_add(three_votes, ((three_votes >> 31) & 3)) >> 2;
            int have = tech_val(DiploScratchFAA8, other, 1);
            int turn_rounded = diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7);
            int scale = diplo_add(turn_rounded, ((turn_rounded >> 31) & 7)) >> 3;
            have = diplo_mul(have, scale);
            if (have < need && DiploScratchFA18 >= 0) {
                int value = tech_val(DiploScratchFA18, other, 1);
                turn_rounded = diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7);
                scale = diplo_add(turn_rounded, ((turn_rounded >> 31) & 7)) >> 3;
                have = diplo_add(have, diplo_mul(value, scale));
            } else {
                DiploScratchFA28 = -1;
                DiploScratchFA1C = -1;
                DiploScratchFA18 = -1;
            }
            if (have < need && DiploScratchFA1C >= 0) {
                int value = tech_val(DiploScratchFA1C, other, 1);
                turn_rounded = diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7);
                scale = diplo_add(turn_rounded, ((turn_rounded >> 31) & 7)) >> 3;
                have = diplo_add(have, diplo_mul(value, scale));
            } else {
                DiploScratchFA28 = -1;
                DiploScratchFA1C = -1;
            }
            if (have < need && DiploScratchFA28 >= 0) {
                int value = tech_val(DiploScratchFA28, other, 1);
                turn_rounded = diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7);
                scale = diplo_add(turn_rounded, ((turn_rounded >> 31) & 7)) >> 3;
                have = diplo_add(have, diplo_mul(value, scale));
            } else {
                DiploScratchFA28 = -1;
            }
            if (have < need) {
                DiploScratchFA28 = DiploScratchFA1C =
                    DiploScratchFA18 = DiploScratchFAA8 = -1;
            }
    }

    StringTemp[0] = 0;
        strcat(StringTemp, "BUYVOTE");
        if (votes == 0) {
            char digits[32];
            _itoa(0, digits, 10);
            strcat(StringTemp, digits);
            PluralityDefault = 0;
            GenderDefault = Players[faction].is_leader_female;
            parse_says(0, Players[faction].name_leader, -1, -1);
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[faction].title_leader, -1, -1);
            X_pops(StringTemp, 0x100000,
                         FactionDiplomacyPortraits[other], pop_wait);
            goto commit_vote;
        } else {
            const char *headline = 0;
            if (proposal == 0) headline = "GOV";
            else if (choice == -3) headline = "ABSTAIN";
            else if (choice == -2) headline = "NAY";
            else if (choice == -1) headline = "YEA";
            if (headline != 0) {
                strcat(StringTemp, headline);
            }
            // Original (0x53F569): the leader reads run for every headline,
            // so choice < 0 indexes before the Players table.
            PluralityDefault = 0;
            GenderDefault = Players[faction].is_leader_female;
            parse_says(2, Players[faction].name_leader, -1, -1);
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(3, Players[faction].title_leader, -1, -1);
            PluralityDefault = 0;
            GenderDefault = Players[choice].is_leader_female;
            parse_says(6, Players[choice].name_leader, -1, -1);
            GenderDefault = Players[choice].is_leader_female;
            PluralityDefault = 0;
            parse_says(8, Players[choice].title_leader, -1, -1);
            parse_num(0, votes);
            if (DiploScratchFAA8 >= 0) {
                parse_say(0, Technology[DiploScratchFAA8].name, -1, -1);
                if (DiploScratchFA18 < 0) {
                    strcat(StringTemp, "1");
                } else {
                    parse_say(1, Technology[DiploScratchFA18].name, -1, -1);
                    if (DiploScratchFA1C < 0) {
                        strcat(StringTemp, "2");
                    } else {
                        parse_say(4, Technology[DiploScratchFA1C].name, -1, -1);
                        if (DiploScratchFA28 >= 0) {
                            parse_say(5, Technology[DiploScratchFA28].name, -1, -1);
                            strcat(StringTemp, "4");
                        } else {
                            strcat(StringTemp, "3");
                        }
                    }
                }
            }
            if (proposal == 0 && choice > 0 && choice != faction) {
                strcat(StringTemp, "A");
            }
            popup.start(PopupStartCaption, StringTemp, -1, nullptr,
                        0x100000, nullptr);
            popup.set_caller_picture(popup.picture_enabled()
                ? nullptr : FactionDiplomacyPortraits[other]);
            popup.dialogs().sprite(
                FactionDiplomacyPortraits[other], nullptr, 0);
            if (X_text_open(ScriptPopupFile, "BUYVOTEMENU") != 0) {
                return;
            }

            text_get();
            parse_string(TextBufferGetPtr, StringTemp);
            popup.dialogs().item(StringTemp, 0);

            text_get();
            parse_string(TextBufferGetPtr, StringTemp);
            if (PlayersData[faction].energy_reserves >= votes)
                popup.dialogs().item(StringTemp, 1);

            text_get();
            parse_string(TextBufferGetPtr, StringTemp);
            if (DiploScratchFAA8 >= 0 && DiploScratchFA18 < 0)
                popup.dialogs().item(StringTemp, 2);

            text_get();
            parse_string(TextBufferGetPtr, StringTemp);
            if (DiploScratchFAA8 >= 0 && DiploScratchFA18 >= 0 &&
                DiploScratchFA1C < 0)
                popup.dialogs().item(StringTemp, 3);

            text_get();
            parse_string(TextBufferGetPtr, StringTemp);
            if (DiploScratchFAA8 >= 0 && DiploScratchFA18 >= 0 &&
                DiploScratchFA1C >= 0 && DiploScratchFA28 < 0)
                popup.dialogs().item(StringTemp, 4);

            text_get();
            parse_string(TextBufferGetPtr, StringTemp);
            if (DiploScratchFAA8 >= 0 && DiploScratchFA18 >= 0 &&
                DiploScratchFA1C >= 0 && DiploScratchFA28 >= 0)
                popup.dialogs().item(StringTemp, 5);

            const int menu_result = popup.exec(0, pop_wait);
            if (menu_result <= 0) {
                return;
            }

            if (menu_result == 1) {
                const int delta = diplo_sub(0, votes);
                if (IsMultiplayerNet == 0) {
                    if (faction != 0)
                        PlayersData[faction].energy_reserves = diplo_add(
                            PlayersData[faction].energy_reserves, delta);
                    if (other != 0)
                        PlayersData[other].energy_reserves = diplo_add(
                            PlayersData[other].energy_reserves, votes);
                    ConsoleGlobal.update_data(1);
                } else {
                    log_say(DiploLabelSendingENERGY, faction, other, delta);
                    message_data(0x2447, 0, faction, delta, other, votes);
                    NetDaemonNet.await_diplo(0x447);
                }
            } else {
                for (int selected = 1; selected < menu_result && selected <= 4;
                     ++selected) {
                    const int tech_id = selected == 1 ? DiploScratchFAA8
                        : selected == 2 ? DiploScratchFA18
                        : selected == 3 ? DiploScratchFA1C : DiploScratchFA28;
                    if (IsMultiplayerNet == 0) {
                        tech_achieved(other, tech_id, faction, 0);
                    } else {
                        log_say(DiploLabelSendingTECH, other, tech_id, faction);
                        message_data(0x244b, 0, other, tech_id, faction, 0);
                        NetDaemonNet.await_diplo(0x44b);
                    }
                }
            }
            DiploScratchFA28 = DiploScratchFA1C =
                DiploScratchFA18 = DiploScratchFAA8 = -1;
        }
        goto commit_vote;

commit_vote:
    recorded_vote = choice;
    gift_faction = proposal == 0 ? choice : faction;
    if (IsMultiplayerNet != 0) {
        synch_leader(other);
        if (TurnRedrawPending != 0)
            message_data(0x2601, 0, other, choice, 0, 0);
    } else if (TurnRedrawPending != 0) {
        CouncilWindow.set_vote(other, choice);
    }
}

extern uint32_t LockEnableMask;  // 0x009A64E8, defined in lock.cpp

// ORIGINAL: 0x00559060 ?get_name@@YAPADH@Z 0x00559060-0x00559092 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c86818d859992f5766b6584955cdba0f7694627d:src/recovered/00559060.cpp", "source:src/faction.h"], "reason": "The pinned entry returns Player::name_leader after selecting leader gender and singular grammar; Player fields have the original stride."}
char *__cdecl get_name(int faction_id) {
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    return Players[faction_id].name_leader;
}

// ORIGINAL: 0x005590A0 ?get_title@@YAPADH@Z 0x005590A0-0x005590D2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c86818d859992f5766b6584955cdba0f7694627d:src/recovered/005590a0.cpp", "source:src/faction.h"], "reason": "The pinned entry returns Player::title_leader after selecting leader gender and singular grammar; the Player type establishes this field and the original record stride."}
char *__cdecl get_title(int faction_id) {
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    return Players[faction_id].title_leader;
}

// ORIGINAL: 0x005590E0 ?get_adjective_leader@@YAPADH@Z 0x005590E0-0x005590FA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c86818d859992f5766b6584955cdba0f7694627d:src/recovered/005590e0.cpp", "source:src/faction.h"], "reason": "The pinned entry returns Player::adj_leader from the typed Player row; the Player type establishes this field and the original record stride."}
char *__cdecl get_adjective_leader(int faction_id) {
    return Players[faction_id].adj_leader;
}

// ORIGINAL: 0x00559100 ?get_adjective_insult_leader@@YAPADH@Z 0x00559100-0x0055911A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c86818d859992f5766b6584955cdba0f7694627d:src/recovered/00559100.cpp", "source:src/faction.h"], "reason": "The pinned entry returns Player::adj_insult_leader from the typed Player row; the Player type establishes this field and the original record stride."}
char *__cdecl get_adjective_insult_leader(int faction_id) {
    return Players[faction_id].adj_insult_leader;
}

// ORIGINAL: 0x00559120 ?get_adjective_faction@@YAPADH@Z 0x00559120-0x0055913A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c86818d859992f5766b6584955cdba0f7694627d:src/recovered/00559120.cpp", "source:src/faction.h"], "reason": "The pinned entry returns Player::adj_faction from the typed Player row; the Player type establishes this field and the original record stride."}
char *__cdecl get_adjective_faction(int faction_id) {
    return Players[faction_id].adj_faction;
}

// ORIGINAL: 0x00559140 ?get_adjective_insult_faction@@YAPADH@Z 0x00559140-0x0055915A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c86818d859992f5766b6584955cdba0f7694627d:src/recovered/00559140.cpp", "source:src/faction.h"], "reason": "The pinned entry returns Player::adj_insult_faction from the typed Player row; the Player type establishes this field and the original record stride."}
char *__cdecl get_adjective_insult_faction(int faction_id) {
    return Players[faction_id].adj_insult_faction;
}

// ORIGINAL: 0x00559160 ?get_insult_leader@@YAPADH@Z 0x00559160-0x00559192 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c86818d859992f5766b6584955cdba0f7694627d:src/recovered/00559160.cpp", "source:src/faction.h"], "reason": "The pinned entry returns Player::insult_leader after selecting leader gender and singular grammar; the Player type establishes this field and the original record stride."}
char *__cdecl get_insult_leader(int faction_id) {
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    return Players[faction_id].insult_leader;
}

// Construction455F10 and destruction456010 enumerate the same37 Sprite
// members, including cleanup on partial construction. Real member lifetime
// supplies those operations and unwind actions exactly once.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Superseded seed deleted; seed paths below are historical Git provenance.


// CRT slot6820B4 selects44CC50: construct eight elements at78E978 with
// stride65C, then register44CC80 to destroy that same vector at exit.
// This independently bounds78E978..791C58; the last two members of each
// element occupy604..65C. Content loading and linked startup ordering
// remain unfinished; the compiler owns the new vector's lifetime.

// new_base4CFD66/4CFE77 reads a writable eight-entry portrait pointer table.
// Its initial targets are the diplomacy Sprites at FactionArt+4D0 with
// the independently established65C object stride. Keep the pointer slots:
// initial targets alone do not justify erasing this mutable indirection.
// RECOVERY_OBJECT: v2 0x006846D8 0x006846F8 ?FactionDiplomacyPortraits@@3PAPAVSprite@@A 0x004CF7C0
Sprite *FactionDiplomacyPortraits[MaxPlayerNum] = {
    &FactionsArt[0].diplomacy, &FactionsArt[1].diplomacy,
    &FactionsArt[2].diplomacy, &FactionsArt[3].diplomacy,
    &FactionsArt[4].diplomacy, &FactionsArt[5].diplomacy,
    &FactionsArt[6].diplomacy, &FactionsArt[7].diplomacy
};

// Both original producers453A20/455EB0 fill192 four-byte pointers to the
// first24 members of those eight objects, independently bounding
//76E590..76E890. Initial pointer storage is zero.

// 0x009A650C
// 0x009A6510
// Extent: compiled sizeof 0x4; next mapped object at 0x0093FA78.
// RECOVERY_OBJECT: v2 0x0093FA74 0x0093FA78 ?DiploFriction@@3HA 0x00427F4F
int DiploFriction;  // 0x0093FA74 // not always bounded, should it be 0-20?
// RECOVERY_OBJECT: v1 0x0093FABC ?DiploFrictionFactionIDWith@@3IA
uint32_t DiploFrictionFactionIDWith;  // 0x0093FABC
// Extent: compiled sizeof 0x4; next mapped object at 0x0093FAC4.
// RECOVERY_OBJECT: v2 0x0093FAC0 0x0093FAC4 ?DiploFrictionFactionID@@3IA 0x00456D16
uint32_t DiploFrictionFactionID;  // 0x0093FAC0
// Both are outputs of scan_prototypes() and both hold a prototype id or -1.
// buy_tech (0x005401A0) reads the first, lazily recomputing it with
// scan_prototypes when it is still negative, and turns it into a trade item
// code by adding 0x61. mention_prototypes (0x0053A230) reads the second, which
// is the prototype the speaking faction brags about.
int BestProtoForTrade;  // 0x0093F804
int BestProtoToMention;  // 0x0093FA40
// introduce (0x0053D360) stores 1 here; diplomacy_ends (0x0053E470)
// stores 0. InfoWin::state_valid tests the dword at 0x00458E25/0x00458E95.
// Those producers are still seeds; recovering this storage does not recover
// their lifecycle. The original four-byte storage is zero-initialized.
// RECOVERY_OBJECT: v2 0x0093FAB4 0x0093FAB8 ?DiplomacyActive@@3HA 0x0053E470
int DiplomacyActive;

// new_base4CFCBA publishes8 before diplomacy_ends;53E484 clears this
// independent DWORD. Both its initial storage and the cleared value are0.
// Its broader protocol meaning and remaining producers still need recovery.
// RECOVERY_OBJECT: v2 0x0093FAC8 0x0093FACC ?DiplomacyEndState@@3HA 0x004CF7C0
int DiplomacyEndState;
// Independent zero-initialized state cleared by diplomacy_ends at 53E47F.
// Its producer protocol is not yet recovered; do not alias DiplomacyActive.
// RECOVERY_OBJECT: v2 0x0093FAB8 0x0093FABC ?DiplomacyAuxiliaryState@@3HA 0x0053E470
int DiplomacyAuxiliaryState;

// wants_to_speak writes and returns the contact response in this
// shared dword; diplomacy_ends clears it before stopping the popup.
// The original initial value is zero; callback updates must remain visible.
// RECOVERY_OBJECT: v2 0x0093FACC 0x0093FAD0 ?DiplomacyContactResult@@3HA 0x0053E470
int DiplomacyContactResult;

// wants_to_speak bounds these coordinates before focusing ConsoleGlobal.
// Teardown resets Y and then X after closing the network channel.
// RECOVERY_OBJECT: v2 0x0068D43C 0x0068D440 ?DiploFocusX@@3HA 0x0053E470
int DiploFocusX = -1;
// RECOVERY_OBJECT: v2 0x0068D440 0x0068D444 ?DiploFocusY@@3HA 0x0053E470
int DiploFocusY = -1;
// The faction whose voice the next popup speaks in. Popup::start (0x00406380)
// and popp(0x0048C0A0) are the only readers: each bounds it to 1..7 and, when
// Players[it].rule_flags has RFLAG_ALIEN, reads the dialogue out of
// alienuscript.txt instead of the ordinary script. Thirty-odd diplomacy entry
// points publish their counterpart faction here before opening a popup, which
// is why suggest_plan writes it unconditionally rather than only on the paths
// that go on to show one.
// Original 0x0053CB09/0x0053E4CD publish the faction id consumed at 0x005BFA11/0x005BFC89; start only.
// RECOVERY_OBJECT: v1 0x0093F7CC ?PopupDialogFactionID@@3HA
int PopupDialogFactionID;  // 0x0093F7CC

/*
Purpose: Determine if the specified faction is a Progenitor alien faction (Caretakers / Usurpers).
Original Offset: n/a
Return Value: Is faction a Progenitor? true/false
Status: Complete
*/
// BODY IN faction.h: six bodies call it where the image calls nothing.



/*
Purpose: Check whether the primary faction has at least one of the specified treaties (bitfield)
         with the secondary faction.
// ORIGINAL: 0x005002F0 ?has_treaty@@YAHHHH@Z 0x005002F0-0x00500319 BYTE_EXACT
// body      src/faction_inline.h
// size      41 bytes
// prototype int (__cdecl ?has_treaty@@YAHHHH@Z)(int factionID, int factionIDWith, int treaty)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Treaty status between the two factions, generally treated as a boolean
Status: Complete
*/


/*
Purpose: Get the faction's name adjective.
// ORIGINAL: 0x0050B910 ?get_adjective@@YAHH@Z 0x0050B910-0x0050B92A BYTE_EXACT
// body      src/faction.cpp
// symbol    ?get_adjective@@YAPADH@Z
// size      26 bytes
// prototype int (__cdecl ?get_adjective@@YAHH@Z)(int factionID)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Faction name adjective
Status: Complete
*/
// Historical placement rationale: the image writes this helper out at some
// call sites and calls it at others. Keep its original call/inlined behavior
// available to the compiler through this ordinary faction.cpp definition.
MEASURED LPSTR __cdecl get_adjective(int faction_id) {
    return Players[faction_id].adj_name_faction;
}


/*
Purpose: Get the faction's noun.
// ORIGINAL: 0x0050B930 ?get_noun@@YAHH@Z 0x0050B930-0x0050B964 BYTE_EXACT
// body      src/faction.cpp
// LEVER: calling `parse_set(...)` grouped the two field reads together (matching
//        the image's read/read/store/store order) but gave the temps the wrong
//        registers (edx for gender, ecx for plurality). Assigning `GenderDefault`
//        and `PluralityDefault` directly, without going through parse_set, keeps
//        the image's register choice (ecx for gender, edx for plurality) - moved
//        10/15 -> 12/15. Remaining divergence: the image computes the
//        `noun_faction` return pointer (`lea eax,[eax+0x946d34]`) only after BOTH
//        stores; this tree's O2 hoists that address computation right after the
//        first read and reaches the second field off the new eax
//        (`mov edx,[eax+0x1c]`) instead of off the original one - scheduling only.
// symbol    ?get_noun@@YAPADH@Z
// size      52 bytes
// prototype int (__cdecl ?get_noun@@YAHH@Z)(int factionID)
// callers   5   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Faction noun
Status: Complete
*/
// Historical source-placement note: the ordinary API now lives here; name_base
// spells its independently observed inline field/grammar operation directly.
// BODY IN faction.h, as `MEASURED inline`: name_base (base.cpp) is the caller
// that needs it folded in place, matching the image.
// enemies_war/enemies_treaty call it out of line in the image.
#pragma auto_inline(off)
MEASURED LPSTR __cdecl get_noun(int faction_id) {
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    return Players[faction_id].noun_faction;
}
#pragma auto_inline(on)

/*
Purpose: Determine whether automatic contact is enabled for net or PBEM games.
// ORIGINAL: 0x00539160 ?auto_contact@@YAHXZ 0x00539160-0x00539193 BYTE_EXACT
// size      51 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is always contact enabled? true/false
Status: Complete
*/
BOOL __cdecl auto_contact() {
    return (IsMultiplayerNet && Rules.tgl_human_always_contact_net) ? true
        : IsMultiplayerPBEM && Rules.tgl_humans_always_contact_pbem;
}

/*
Purpose: Determine if the overall dominant human faction is a minor threat based on base count.
// ORIGINAL: 0x00539B70 ?great_beelzebub@@YAHHH@Z 0x00539B70-0x00539BF2 BYTE_EXACT
// LEVER: bases_threat was uint32_t; the image's `cmp edx,4; jge` and
//        `cmp [current_num_bases],edx; jle` are both SIGNED compares. Made
//        bases_threat `int` and cast `(int)current_num_bases` at its one use
//        here (left the shared PlayerData field uint32_t - too many other
//        callers to retype safely) - moved 35/43 -> 37/43 agreeing.
// size      130 bytes
// prototype BOOL (__cdecl ?great_beelzebub@@YAHHH@Z)(int factionID, int tgl)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is faction minor threat? true/false
Status: Complete
*/
BOOL __cdecl great_beelzebub(int faction_id, BOOL is_aggressive) {
    if (is_human(faction_id) && GameDataStateStorage.fields_.faction_rankings_[7] == faction_id) {
        int bases_threat = (GameDataStateStorage.fields_.turn_current_num_ + 25) / 50;
        if (bases_threat < 4) {
            bases_threat = 4;
        }
        if (PlayersData[faction_id].current_num_bases > bases_threat
            && (PlayersData[faction_id].diff_level > DLVL_SPECIALIST
                || GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY || is_aggressive)) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Determine if the specified faction is considered a threat based on the game state and
         ranking.
// ORIGINAL: 0x00539C00 ?great_satan@@YAHHH@Z 0x00539C00-0x00539D3B BYTE_EXACT
// LEVER: the open-coded great_beelzebub prefix is /Ob2 inlining of the
//        preceding same-TU definition; calling it is byte-exact. Earlier
//        hand-inlined spellings (see Git history) stalled at register choice.
// size      315 bytes
// prototype
// callers   20   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00539D40 0x00539E40
Return Value: Is the specified faction a threat? true/false
Status: Complete
*/
BOOL __cdecl great_satan(int faction_id, BOOL is_aggressive) {
    // great_beelzebub precedes this in the TU and /Ob2 inlines it.
    if (!great_beelzebub(faction_id, is_aggressive)) {
        return false;
    }
    BOOL has_intense_riv = (GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY);
    if (GameDataStateStorage.fields_.turn_current_num_ <= ((has_intense_riv ? 0
        : (DLVL_TRANSCEND - PlayersData[faction_id].diff_level) * 50) + 100)) {
        return false;
    }
    if (climactic_battle() && aah_ooga(faction_id, -1) == faction_id) {
        return true;
    }
    int diff_factor;
    int factor;
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY) != 0) {
        factor = 4;
        diff_factor = DLVL_TRANSCEND;
    } else if (PlayersData[faction_id].diff_level >= DLVL_LIBRARIAN
        || GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_CONQUEST || ScenarioSave.objective_req_victory <= 1000) {
        factor = 2;
        diff_factor = DLVL_LIBRARIAN;
    } else {
        factor = 1;
        diff_factor = DLVL_TALENT;
    }
    return (factor * FactionRankingsUnk[GameDataStateStorage.fields_.faction_rankings_[7]]
        >= diff_factor * FactionRankingsUnk[GameDataStateStorage.fields_.faction_rankings_[6]]);
}

/*
Purpose: Check whether the specified faction is nearing the diplomatic victory requirements to be
         able to call a Supreme Leader vote. Optional 2nd parameter (0/-1 to disable) that specifies
         a faction to skip if they have a pact with faction from the 1st parameter.
// ORIGINAL: 0x00539D40 ?aah_ooga@@YAHHH@Z 0x00539D40-0x00539E3E BYTE_EXACT
// LEVER: the image calls council_votes(player) UNCONDITIONALLY at the top of
//        each second-loop iteration and stores its result into the recycled
//        `faction_id` stack slot, before testing `player != pact_faction_id`
//        - the source must hoist `int votes = council_votes(player);` out of
//        the `&&` chain, not call it inline inside the condition. Also fixed
//        `pact_faction_id <= 0` -> `!pact_faction_id` (image tests only
//        equality: `test esi,esi; je`, never a signed range check) and made
//        the loop counters/`votes_total` plain `int` to match the image's
//        signed `jl` compares (was `uint32_t`, gave `jb`/`jge`->`jae`).
//        Moved 25/94 -> 34/94 agreeing.
// LEVER (2026-09-03): `has_tech_call`, not `has_tech`, at all three sites.
//   technology.h declares has_tech `MEASURED inline`, so this tree's compile
//   open-coded the whole preq walk three times and the body measured 0/94 -
//   the image CALLS 0x005B9F20 at 0x00539DC2/0x00539DE3/0x00539DF7, which
//   makes this a best_specialist-shaped caller. technology.h already carries
//   the non-inline forwarder for exactly that; switching the three calls took
//   34/94 -> 94/94 BYTE_EXACT, and the "register allocation / strength
//   reduction" the earlier note blamed was the inlined walk, not registers.
//   Measured 20/94 at the default flags and 49/94 under /Ob0 BEFORE the
//   switch, which is what identified the inline as the defect.
// symbol    ?aah_ooga@@YAHHH@Z
// size      254 bytes
// prototype
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0052AD30 0x005B9F20
Return Value: Faction id if nearing diplomatic victory or zero
Status: Complete
*/
int __cdecl aah_ooga(int faction_id, int pact_faction_id) {
    if (!(GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_DIPLOMATIC)) {
        return 0; // Diplomatic Victory not allowed
    }
    int votes_total = 0;
    for (int i = 1; i < MaxPlayerNum; i++) {
        votes_total += council_votes(i);
    }
    int faction_id_ret = 0;
    for (int player = 1; player < MaxPlayerNum; player++) {
        int votes = council_votes(player);
        if (player != pact_faction_id
            && (!pact_faction_id || !has_treaty(player, pact_faction_id, DTREATY_PACT)
                || !(GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE))) {
            int proposal_preq = Proposal[PROP_UNITE_SUPREME_LEADER].preq_tech;
            // has_tech_call, not has_tech: the image CALLS 0x005B9F20 at all
            // three of these sites (0x00539DC2/0x00539DE3/0x00539DF7) rather
            // than open-coding the preq walk, so this is a best_specialist
            // caller, not one of the 108 genuine inline sites.
            if ((has_tech(proposal_preq, faction_id)
                || (proposal_preq >= 0
                    && (has_tech(Technology[proposal_preq].preq_tech_1, faction_id)
                    || has_tech(Technology[proposal_preq].preq_tech_2, faction_id))))
                && votes * 2 >= votes_total && (!faction_id_ret || player == faction_id)) {
                faction_id_ret = player;
            }
        }
    }
    return faction_id_ret;
}

/*
Purpose: Check if the human controlled player is nearing the endgame.
// ORIGINAL: 0x00539E40 ?climactic_battle@@YAHXZ 0x00539E40-0x00539EE4 BYTE_EXACT
// LEVER (2026-09-03): two things closed it, 10/56 -> 56/56 BYTE_EXACT.
//   1. `has_tech_call` at both sites, for the same reason aah_ooga needs it -
//      the image CALLS 0x005B9F20 here and technology.h's MEASURED inline was
//      being open-coded three instructions' worth per site.
//   2. the SECOND loop counts in a signed `int` (named `player`, since VC6's
//      for-scope leaks forbid a second `i`): the image's `cmp esi,8; jl` is a
//      signed compare, and this tree's `uint32_t i` emitted `jb` - the one
//      instruction left after the call fix. The FIRST loop stays `uint32_t`;
//      the image walks the PlayersData pointer there (`add eax,0x20cc; cmp
//      eax,0x97f0bc; jl`), so nothing in this tree's loop counter changes it.
// PREVIOUS: the image never calls ascending(0) - it is a real 4-caller function
//        elsewhere, but here it inlines to `base_project(SP_VOICE_OF_PLANET)
//        != SP_Unbuilt` (base_project is already MEASURED inline in base.h).
//        Swapping the call for that expression moved 14/56 -> 16/56 agreeing.
//        Remaining divergence starts at instruction 0: the image loads
//        (LockEnableMask & 0xFF)/TurnCurrentNum and walks PlayersData[1].. by a raw
//        incrementing pointer through the first loop; this tree's compile
//        still indexes by `i` and schedules differently from instruction 0.
//        Plateaus here after the ascending() fix; not chased further.
// size      164 bytes
// prototype
// callers   7   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00539D40 0x005B9F20
Return Value: Is human player nearing endgame? true/false
Status: Complete
*/
BOOL __cdecl climactic_battle() {
    for (uint32_t i = 1; i < MaxPlayerNum; i++) {
        if (is_human(i) && PlayersData[i].corner_market_turn > GameDataStateStorage.fields_.turn_current_num_) {
            return true; // Human controlled player initiated Corner Global Energy Market
        }
    }
    if (aah_ooga(0, -1)) { // nearing Supreme Leader, these parameters will always return false
        return true; // TODO: Revisit in future once more end game code is complete. This may have
                     //       been effectively disabled as a design decision rather than a bug.
    }
    if (base_project(SP_VOICE_OF_PLANET) != SP_Unbuilt) {
        // A signed counter this time: the image's `cmp esi,8; jl` on the
        // second loop is signed where the first loop walks the PlayersData
        // pointer, and VC6's for-scope leaks mean the two loops cannot both
        // own an `i`.
        for (int player = 1; player < MaxPlayerNum; player++) {
            // has_tech_call: the image CALLS 0x005B9F20 here (see the calls
            // line) rather than open-coding the preq walk.
            if (is_human(player) && (has_tech(Facility[FAC_PSI_GATE].preq_tech, player)
                || has_tech(Facility[FAC_VOICE_OF_PLANET].preq_tech, player))) {
                return true; // Human controlled player has tech to build PSI Gates or VoP
            }
        }
    }
    return false;
}

/*
Purpose: Determine if the specified AI faction is at the game climax based on certain conditions.
// ORIGINAL: 0x00539EF0 ?at_climax@@YAHH@Z 0x00539EF0-0x0053A022 BYTE_EXACT
// LEVER: the first loop's corner_market_turn/has_treaty check read
//        `PlayersData[faction_id].corner_market_turn`, a loop-invariant that
//        made the image's per-i pointer walk (add ecx,0x20cc each iteration,
//        matching climactic_battle's sibling loop) unreachable from this
//        source. Fixed to `PlayersData[i].corner_market_turn`, matching
//        climactic_battle's `is_human(i) && PlayersData[i].corner_market_turn`
//        shape - similarity rose 0.888->0.957 (bytes 241/306 vs relocations).
//        TRIED: hoisting `!(GameRules & RULES_VICTORY_COOPERATIVE)` into
//        a local before the loop, to match the image's `mov edx,[GameRules]`
//        once before the loop reused via `test dh,0x10` inside it - tied at
//        5/105 agreeing, no change. Remaining divergence is a near-total
//        register swap (image keeps faction_id in edi/loop-i in esi, this
//        tree the reverse) cascading from instruction 4 - register
//        allocation only, not chased per the no-register-only rule.
//        TRIED (2026-09-03): `ascending_call` for both ascending sites - the
//        image CALLS 0x005AC680 twice here (call_diff said this tree made 2
//        calls where the image makes 4) and base.h's MEASURED inline was
//        folding them. Call count now agrees; 17/105 unchanged, and the
//        cascade's true head is the RECEIVER-SPILL fingerprint the brief
//        documents: the image pushes edi in its PROLOGUE and then reads
//        faction_id into it (`push ebx/push esi/push edi/mov edi,[ebp+8]`)
//        while this tree defers `push edi` past the four early-return guards
//        and loads faction_id into esi, so every later esi/edi assignment is
//        mirrored. Not chased further - same wall as the Palette bodies.
// size      306 bytes
// prototype
// callers   6   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00539D40 0x00539E40 0x005AC680
Return Value: Is AI faction at climax? true/false
Status: Complete
*/
BOOL __cdecl at_climax(int faction_id) {
    if (is_human(faction_id) || GameDataStateStorage.fields_.game_state_ & STATE_UNK_1 || GameDataStateStorage.fields_.diff_level_current_ == DLVL_CITIZEN
        || !climactic_battle()) {
        return false;
    }
    if (aah_ooga(faction_id, faction_id)) {
        return true;
    }
    for (int i = 1; i < MaxPlayerNum; i++) {
        if (i != faction_id && PlayersData[i].corner_market_turn > GameDataStateStorage.fields_.turn_current_num_
            && (!has_treaty(faction_id, i, DTREATY_PACT)
                || !(GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE))) {
            return true;
        }
    }
    int trans_most_min_them = 0;
    int trans_most_min_us = 0;
    for (int player = 0; player < GameDataStateStorage.fields_.base_current_count_; player++) {
        if (Bases[player].queue_production_id[0] == -FAC_ASCENT_TO_TRANSCENDENCE) {
            if (Bases[player].faction_id_current == faction_id) {
                if (trans_most_min_us <= Bases[player].minerals_accumulated) {
                    trans_most_min_us = Bases[player].minerals_accumulated;
                }
            } else if (trans_most_min_them <= Bases[player].minerals_accumulated) {
                trans_most_min_them = Bases[player].minerals_accumulated;
            }
        }
    }
    for (i = 1; i < MaxPlayerNum; i++) {
        if (i != faction_id) {
            // Both original calls remain observable across translation units.
            if (ascending(i) && !ascending(faction_id)) {
                return true;
            }
        }
    }
    return trans_most_min_them && trans_most_min_them > trans_most_min_us;
}

/*
Purpose: Add friction between the two specified factions.
// ORIGINAL: 0x0053A030 ?cause_friction@@YAXHHH@Z 0x0053A030-0x0053A08D BYTE_EXACT
// LEVER: the FLAT VIEW, not the row pointer. diplo_friction sits sixteen
//   dwords past the treaty word, so `int index = faction_id_with
//   + faction_id * SpyingStatusStride` and `SpyingStatusTable[index + 16]` at
//   both uses - the same +offset spelling treaty_off uses for the agenda
//   view - lets VC6 keep ONE unscaled index register that the load and the
//   store both reach through `[reg*4 + 0x96ca38]`. 21/33 -> 33/33 BYTE_EXACT;
//   the "register scheduling" the earlier note blamed was the row pointer
//   forcing the *4 into the pointer instead of into the accesses.
// PREVIOUS: `diplo_friction` bound to the ROW (`PlayersData[faction_id].diplo_friction`,
//   the `uint32_t[8]` itself, no `[faction_id_with]` yet) rather than to the single
//   `&...diplo_friction[faction_id_with]` ELEMENT - indexing with `[faction_id_with]`
//   at each of the two uses, instead of dereferencing a pointer already advanced past
//   the row. The earlier TRIED below only tried the ELEMENT pointer (`*diplo_friction`)
//   and a plain double-index with no pointer at all; the ROW pointer is the one the image
//   actually keeps live (`diplo_friction[8]` is itself 2D-indexed by a runtime
//   `faction_id`, and only that outer index is worth hoisting - the inner one stays a
//   subscript). Moved 13/33 -> 21/33 agreeing. Remaining divergence starts at
//   instruction 5 with push/register-assignment order (esi/edi scheduling), not
//   chased further this pass.
// size      93 bytes
// prototype void (__cdecl ?cause_friction@@YAXHHH@Z)(int factionID, int factionIDWith, int friction)
// callers   8   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Only communicate (which expands nothing) calls it out of line; the other
// diplomacy functions expand it.
void __cdecl cause_friction(int faction_id, int faction_id_with, int friction) {
    // diplo_friction is the same 2099-stride view sixteen dwords past the
    // treaty word - SpyingStatusTable[index + 16], exactly the +8 spelling
    // treaty_off uses for the agenda view. One index local reaches both the
    // read and the write, which is what the image keeps in a single register.
    PlayersData[faction_id].diplo_friction[faction_id_with] =
        range(PlayersData[faction_id].diplo_friction[faction_id_with] + friction, 0, 20);
    if (DiploFrictionFactionID == faction_id && DiploFrictionFactionIDWith == faction_id_with) {
        DiploFriction += friction; // not bounded?
    }
}

/*
Purpose: Normalize the diplomatic friction value into a mood offset.
// ORIGINAL: 0x0053A090 ?get_mood@@YAHH@Z 0x0053A090-0x0053A0F5 BYTE_EXACT
// symbol    ?get_mood@@YAHH@Z
// size      101 bytes
// prototype int (__cdecl ?get_mood@@YAHH@Z)(int friction)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Mood (0-8)
Status: Complete
*/
int __cdecl get_mood(int friction) {
    if (friction <= 0) {
        return MOOD_MAGNANIMOUS;
    }
    if (friction <= 2) {
        return MOOD_SOLICITOUS;
    }
    if (friction <= 4) {
        return MOOD_COOPERATIVE;
    }
    if (friction <= 8) {
        return MOOD_NONCOMMITTAL;
    }
    if (friction <= 12) {
        return MOOD_AMBIVALENT;
    }
    if (friction <= 15) {
        return MOOD_OBSTINATE;
    }
    if (friction <= 17) {
        return MOOD_QUARRELSOME;
    }
    return friction > 19 ? MOOD_SEETHING : MOOD_BELLIGERENT;
}

/*
Purpose: Calculate the negative reputation the specified faction has with another.
// ORIGINAL: 0x0053A100 ?reputation@@YAHHH@Z 0x0053A100-0x0053A14D BYTE_EXACT
// size      77 bytes
// prototype
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bad reputation
Status: Complete
*/
// Every in-TU caller (rankings, enemy_diplomacy) calls it out of line.
#pragma auto_inline(off)
int __cdecl reputation(int faction_id, int faction_id_with) {
    return range(PlayersData[faction_id].integrity_blemishes
        - PlayersData[faction_id].diplo_unk1[faction_id_with], 0, 99);
}
#pragma auto_inline(on)

/*
Purpose: Calculate the amount of patience the specified faction has with another.
// ORIGINAL: 0x0053A150 ?get_patience@@YAHHH@Z 0x0053A150-0x0053A1B8 BYTE_EXACT
// size      104 bytes
// prototype int (__cdecl ?get_patience@@YAHHH@Z)(int factionIDWith, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Patience
Status: Complete
*/
int __cdecl get_patience(int faction_id_with, int faction_id) {
    if (has_treaty(faction_id, faction_id_with, DTREATY_VENDETTA)) {
        return 1;
    }
    if (has_treaty(faction_id, faction_id_with, DTREATY_PACT)) {
        return has_treaty(faction_id, faction_id_with, DTREATY_HAVE_SURRENDERED) ? 500 : 6;
    }
    return (has_treaty(faction_id, faction_id_with, DTREATY_TREATY) != 0)
        - ((DiploFriction + 3) / 8) + 3;
}

/*
Purpose: Calculate the amount of goodwill a loan will generate. This is used to reduce friction.
// ORIGINAL: 0x0053A1C0 ?energy_value@@YAHH@Z 0x0053A1C0-0x0053A228 BYTE_EXACT
// size      104 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Goodwill (friction reduction amount)
Status: Complete
*/
int __cdecl energy_value(int loan_principal) {
    int goodwill = 0;
    int divisor = 2;
    for (int weight = 10, energy = loan_principal / 5; energy > 0;
         energy -= weight, weight = 20) {
        goodwill += ((weight >= 0) ? ((energy > weight) ? weight : energy) : 0)
                    / divisor++;
    }
    return (goodwill + 4) / 5;
}

/*
Purpose: Pick out the two prototypes of faction_id_with that are worth putting in front of
         faction_id: the strongest one it owns, and the strongest one still worth bragging about.
// ORIGINAL: 0x0053A4A0 ?scan_prototypes@@YAXHH@Z 0x0053A4A0-0x0053A774 BYTE_EXACT
// TRIED: `call_diff` already agrees (0 disagree) - `weap_strat`/
// `arm_strat` are called the same number of times the image calls them.
// MISMATCH is 10/218 at this pass, all register allocation/scheduling
// inside the nested-loop body (218 image instructions against a
// semantically-verified 222 here); not chased at the instruction level this
// pass - the size (724 image bytes, two nested loops over 64 prototype
// slots) puts a full register-by-register match outside this batch's
// budget. The body itself is already heavily verified (see the mutation-
// testing note below, in the Purpose block) so the gap is codegen shape,
// not a wrong transcription.
// size      724 bytes
// prototype
// callers   3   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D270 0x0057D2E0
Return Value: n/a
Status: Complete

Both answers are published as prototype ids in BestProtoForTrade and BestProtoToMention, and both
start at -1 so a caller can tell "nothing qualifies" from "prototype zero". buy_tech reads the
first only after checking it is negative, so this function is the producer for a cache rather than
a query.

The scan runs over faction_id_with's own 64 prototype slots and scores each live, non-obsolete one
at

  3 * Weapon[weapon].offense_rating
  + 4 * (((7 - plan) << 14) + Armor[armor].defense_rating)
  + Chassis[chassis].speed

The plan term dominates: shifted 14 and then multiplied by 4 it is worth 65536 a step, so the
ordering is really by plan first - PLAN_OFFENSIVE beating everything and PLAN_FUNGAL_MISSILE
scoring below zero - with weapon, armour and speed only breaking ties inside a plan.

The other half of the work is asking whether faction_id already owns something that makes the
prototype unremarkable. Every one of ITS 64 slots is compared, and a rival counts only if it is
live, not obsolete to faction_id, has the same plan and the same triad. Such a rival cancels the
prototype if it has a better reactor, or a better chassis speed, or - at no worse speed - is at
least as good on both weapon and armour and strictly better on one. The comparison uses
weap_strat and arm_strat rather than the raw ratings, so faction_id's own weapon preferences
decide it. That search stops at the first rival that cancels, and a cancelled prototype is not
discarded: its score is divided by 16 and it still competes for BestProtoForTrade.

Verification note: arm_strat is called on the WEAPON id, not the armour id, on both sides of the
comparison. That is what the original does - the two calls at 0x0053A64E and 0x0053A65E read
offset 0x25 of the prototype, the same field the weap_strat pair one instruction earlier reads,
not the armour at 0x26 - and it is reproduced rather than corrected. It is not harmless: it makes
the armour half of the domination test a second reading of the weapon, so a rival that is better
armoured but no better armed does not cancel anything. Both calls read the same wrong field, so
the test stays self-consistent and cannot be told apart from a correct one by a prototype pair
whose weapon ids are equal.

The mention answer is gated further. A prototype that already carries PROTO_UNK_20 is never
mentioned to a pact partner, and outside a pact PROTO_UNK_10 disqualifies it as well - which is
the "do not say the same thing twice" bookkeeping mention_prototypes writes. Its plan must be
PLAN_NAVAL_SUPERIORITY or lower and must not be PLAN_RECONNAISANCE. Finally a cancelled prototype
scores zero here, and therefore cannot win, UNLESS the two factions are in a vendetta and it is a
PLAN_DEFENSIVE unit, in which case it keeps a sixteenth of its score - the one case where showing
off a defender that is not your best still has a point.

Verification note: the sweep against recovery-gameplay-tests kills 40 of 41 valid mutants twice
over, and the survivor is an equivalence rather than a coverage hole. Raising
best_mention_value's initial 0 to 1 changes nothing that any fixture could reach, because no
mentionable prototype can score exactly 1: the gate admits only plans 0, 1, 2, 4, 5 and 6, whose
plan term alone is at least 4 * (1 << 14) = 65536, and the weapon, armour and speed corrections
are bounded by their int8/uint8 field ranges at 3*127 + 4*127 + 255 = 1144 above and -896 below.
The one path that shrinks a mention score - the vendetta rescue - applies only to PLAN_DEFENSIVE,
whose 327680 stays above 20424 after the shift. Every reachable mention score is therefore either
at most 0, in which case both the 0 and the 1 refuse it, or at least 20424, in which case both
accept it.

Two more things the fixtures deliberately do not try to pin. The inner loop stops at the first
rival that cancels, which cannot be observed at all: weap_strat and arm_strat have no side
effects, and setting an already-clear flag clear again is the only thing the remaining iterations
could do. And the plan term's shift width is invisible for the same reason the whole score is:
nothing publishes it. Only two prototype ids leave this function, so any change that scales every
score by one factor - which is what moving that shift does - reorders nothing, and the non-plan
terms cannot bridge even half of the smallest plan step it produces until the width falls below
eight.
*/
void __cdecl scan_prototypes(int faction_id, int faction_id_with) {
    BestProtoForTrade = -1;
    BestProtoToMention = -1;
    int best_trade_value = 0;
    int best_mention_value = 0;
    for (int i = 0; i < MaxVehProtoFactionNum; i++) {
        int proto_id = faction_id_with * MaxVehProtoFactionNum + i;
        if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE) || !(VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE)) {
            continue;
        }
        if (VehPrototypes[proto_id].obsolete_factions & (1 << faction_id_with)) {
            continue;
        }
        int unmatched = 1;
        int value = 3 * Weapon[VehPrototypes[proto_id].weapon_id].offense_rating
            + 4 * (((7 - VehPrototypes[proto_id].plan) << 14) + Armor[VehPrototypes[proto_id].armor_id].defense_rating)
            + Chassis[VehPrototypes[proto_id].chassis_id].speed;
        for (int j = 0; unmatched && j < MaxVehProtoFactionNum; j++) {
            int rival_id = faction_id * MaxVehProtoFactionNum + j;
            if (!(VehPrototypes[rival_id].flags & PROTO_ACTIVE) || !(VehPrototypes[rival_id].flags & PROTO_TYPED_COMPLETE)) {
                continue;
            }
            if (VehPrototypes[rival_id].obsolete_factions & (1 << faction_id)) {
                continue;
            }
            if (VehPrototypes[rival_id].plan != VehPrototypes[proto_id].plan) {
                continue;
            }
            if (Chassis[VehPrototypes[rival_id].chassis_id].triad != Chassis[VehPrototypes[proto_id].chassis_id].triad) {
                continue;
            }
            if (VehPrototypes[rival_id].reactor_id > VehPrototypes[proto_id].reactor_id) {
                unmatched = 0;
            }
            if (Chassis[VehPrototypes[rival_id].chassis_id].speed > Chassis[VehPrototypes[proto_id].chassis_id].speed) {
                unmatched = 0;
            }
            // Both arm_strat calls read the weapon id; see the note above.
            int rival_weapon = weap_strat(VehPrototypes[rival_id].weapon_id, faction_id);
            int proto_weapon = weap_strat(VehPrototypes[proto_id].weapon_id, faction_id);
            int rival_armor = arm_strat(VehPrototypes[rival_id].weapon_id, faction_id);
            int proto_armor = arm_strat(VehPrototypes[proto_id].weapon_id, faction_id);
            if (Chassis[VehPrototypes[rival_id].chassis_id].speed >= Chassis[VehPrototypes[proto_id].chassis_id].speed
                && rival_weapon >= proto_weapon && rival_armor >= proto_armor
                && (rival_weapon > proto_weapon || rival_armor > proto_armor)) {
                unmatched = 0;
            }
        }
        int trade_value = value;
        if (!unmatched) {
            trade_value >>= 4;
        }
        if (trade_value > best_trade_value) {
            best_trade_value = trade_value;
            BestProtoForTrade = proto_id;
        }
        uint32_t treaty = PlayersData[faction_id].diplo_treaties[faction_id_with];
        if (treaty & DTREATY_PACT) {
            if (VehPrototypes[proto_id].flags & PROTO_UNK_20) {
                continue;
            }
        } else if (VehPrototypes[proto_id].flags & (PROTO_UNK_10 | PROTO_UNK_20)) {
            continue;
        }
        if (VehPrototypes[proto_id].plan > PLAN_NAVAL_SUPERIORITY || VehPrototypes[proto_id].plan == PLAN_RECONNAISANCE) {
            continue;
        }
        if (!unmatched) {
            value = ((treaty & DTREATY_VENDETTA) && VehPrototypes[proto_id].plan == PLAN_DEFENSIVE)
                ? (value >> 4) : 0;
        }
        if (value > best_mention_value) {
            best_mention_value = value;
            BestProtoToMention = proto_id;
        }
    }
}


/*
Purpose: Check whether the primary faction has at least one of the specified agendas (bitfield)
         with the secondary faction.
// ORIGINAL: 0x005591E0 ?has_agenda@@YAHHHH@Z 0x005591E0-0x00559209 BYTE_EXACT
// body src/faction_inline.h
// size      41 bytes
// prototype int (__cdecl ?has_agenda@@YAHHHH@Z)(int factionID, int factionIDWith, int agenda)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Agenda status between the two factions, generally treated as a boolean
Status: Complete
*/


/*
Purpose: Add the specific goal to the faction's goals for the specified tile. Optional base param.
// ORIGINAL: 0x00579A30 ?add_goal@@YAXHHHHHH@Z 0x00579A30-0x00579B64 BYTE_EXACT
// LEVER: declaring `priority_search`/`goal_id` before the first loop (matching
//        their eventual use right after it, not right before the second loop)
//        moved 5/104 -> 11/104 agreeing. TRIED: rewriting the `Goal&`
//        aliases as direct PlayersData[faction_id].goals[i].field accesses
//        made it worse (1/104) - the reference form is closer to the image.
//        Remaining gap: the image needs only ebx/esi/edi as callee-saved
//        registers and reuses the (dead, post-use) faction_id parameter slot
//        at [ebp+8] as a spill for priority_search; this tree's compile still
//        pushes an extra ecx for a fourth save. Not chased further.
//        TRIED: `PlayerData *p = &PlayersData[faction_id];` before both
//        loops - ties at 11/104, no change either way.
//        TRIED: a `Goal *goals` pointer hoisted out of EITHER loop (or
//        both) and WALKED with `++` in the for-loop increment instead of
//        `Goal &goals = ...[i]` - the lever that took at_site to BYTE_EXACT
//        and del_site 61/95 -> 68/95. Here it does not help: walking loop 1
//        alone drops to 10/104, walking loop 2 alone or both ties the
//        committed 11/104. This function's ceiling is the register-pressure
//        gap described above, not the address-computation form.
// size      308 bytes
// prototype void (__cdecl ?add_goal@@YAXHHHHHH@Z)(int factionID, int type, int priority, int xCoord, int yCoord, int baseID)
// callers   12   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status:  Complete
*/
void __cdecl add_goal(int faction_id, int type, int priority, int x, int y, int base_id) {
    if (!on_map(x, y)) {
        return;
    }
    for (int i = 0; i < MaxGoalsNum; i++) {
        if (PlayersData[faction_id].goals[i].x == x && PlayersData[faction_id].goals[i].y == y
            && PlayersData[faction_id].goals[i].type == type) {
            PlayersData[faction_id].goals[i].priority = static_cast<int16_t>(
                PlayersData[faction_id].goals[i].priority > priority
                    ? PlayersData[faction_id].goals[i].priority : priority);
            return;
        }
    }
    int priority_search = 0;
    int goal_id = -1;
    for (i = 0; i < MaxGoalsNum; i++) {
        Goal &goals = PlayersData[faction_id].goals[i];
        const int16_t type_cmp = goals.type;
        if (type_cmp < 0 || goals.priority < priority) {
            int cmp = type_cmp < 0 ? 1000 : 0;
            if (!cmp) {
                cmp = goals.priority > 0 ? 20 - goals.priority : goals.priority + 100;
            }
            if (cmp > priority_search) {
                priority_search = cmp;
                goal_id = i;
            }
        }
    }
    if (goal_id >= 0) {
        PlayersData[faction_id].goals[goal_id].x = x;
        PlayersData[faction_id].goals[goal_id].y = y;
        PlayersData[faction_id].goals[goal_id].type = (int16_t)type;
        PlayersData[faction_id].goals[goal_id].priority = (int16_t)priority;
        PlayersData[faction_id].goals[goal_id].base_id = base_id;
    }
}

/*
Purpose: Add the specific site to the faction's site goals for the specified tile.
// ORIGINAL: 0x00579B70 ?add_site@@YAXHHHHH@Z 0x00579B70-0x00579CB5 BYTE_EXACT
// TRIED: hoisting priority_search/site_id before the first loop (the lever
//            that helped add_goal) made this WORSE (7/116, two spilled locals
//            instead of one); reverted. Separate loop counters (i for the
//            first loop, j for the second) instead of reusing `i` made no
//            difference. Image reserves exactly one stack slot (`push ecx` ->
//            [ebp-4], site_id) and keeps priority_search live only in a
//            register across the second loop; this tree's compile reserves
//            two (`sub esp,8`). Plateaus at 17/116 agreeing.
//            `PlayerData *p = &PlayersData[faction_id];` before both loops
//            also worse (16/116).
// TRIED: a `Goal *sites` pointer hoisted out of loop 1, loop 2, or both,
//            WALKED with `++` in the for-loop increment instead of
//            `Goal &sites = ...[i]` - the lever that took at_site to
//            BYTE_EXACT and del_site 61/95 -> 68/95. Here every combination
//            ties or loses (17/116 unchanged, or 16/116 walking loop 1).
//            Same register-pressure ceiling as add_goal, not an addressing
//            form this function is short on.
// size      325 bytes
// prototype void (__cdecl ?add_site@@YAXHHHHH@Z)(int factionID, int type, int priority, int xCoord, int yCoord)
// callers   4   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00538F30 0x00579A30
Return Value: n/a
Status: Complete
*/
void __cdecl add_site(int faction_id, int type, int priority, int x, int y) {
    if ((x ^ y) & 1 && GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) {
        danger("Bad SITE", "", x, y, type);
    }
    for (int i = 0; i < MaxSitesNum; i++) {
        if (PlayersData[faction_id].sites[i].x == x && PlayersData[faction_id].sites[i].y == y
            && PlayersData[faction_id].sites[i].type == type) {
            PlayersData[faction_id].sites[i].priority = static_cast<int16_t>(
                PlayersData[faction_id].sites[i].priority > priority
                    ? PlayersData[faction_id].sites[i].priority : priority);
            return;
        }
    }
    int priority_search = 0;
    int site_id = -1;
    for (i = 0; i < MaxSitesNum; i++) {
        Goal &sites = PlayersData[faction_id].sites[i];
        const int16_t type_cmp = sites.type;
        if (type_cmp < 0 || sites.priority < priority) {
            int cmp = type_cmp < 0 ? 1000 : 0;
            if (!cmp) {
                cmp = 20 - sites.priority;
            }
            if (cmp > priority_search) {
                priority_search = cmp;
                site_id = i;
            }
        }
    }
    if (site_id >= 0) {
        PlayersData[faction_id].sites[site_id].x = x;
        PlayersData[faction_id].sites[site_id].y = y;
        PlayersData[faction_id].sites[site_id].type = (int16_t)type;
        PlayersData[faction_id].sites[site_id].priority = (int16_t)priority;
        add_goal(faction_id, PlayersData[faction_id].sites[site_id].type,
                 PlayersData[faction_id].sites[site_id].priority,
                 PlayersData[faction_id].sites[site_id].x,
                 PlayersData[faction_id].sites[site_id].y, -1);
    }
}

/*
Purpose: Check if a goal exists at the tile for the specified faction and type.
// ORIGINAL: 0x00579CC0 ?at_goal@@YAHHHHH@Z 0x00579CC0-0x00579D16 BYTE_EXACT
// LEVER: WALK the pointer, do not index it. `for (int i = 0; i < MaxGoalsNum; i++, goals++)` with `goals->x` reaches 40 of 40 - BYTE_EXACT. The hoisted-but-INDEXED form (`goals[i]`) plateaus at 17/40, and the TRIED this replaces called that remaining gap "a different but equivalent strength reduction". It was not equivalent and it was not a gap: it was this lever, unapplied.
// NOTE: found by the adversarial verifier on at_site (0x00579D20) 2026-08-22,
//        which cited this body as precedent for the walked form while this
//        body was still indexed - so the citation named a lever that recovers
//        its own precedent outright. Checking a claim's PROSE, not just its
//        tier, is what surfaced a free BYTE_EXACT.
// size      86 bytes
// prototype int (__cdecl ?at_goal@@YAHHHHH@Z)(int factionID, int type, int xCoord, int yCoord)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Does the specific goal exist for the faction at tile? true/false
Status: Complete
*/
BOOL __cdecl at_goal(int faction_id, int type, int x, int y) {
    Goal *goals = PlayersData[faction_id].goals;
    for (int i = 0; i < MaxGoalsNum; i++, goals++) {
        if (goals->x == x && goals->y == y && goals->type == type) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Check if a site exists at the tile for the specified faction and type.
// ORIGINAL: 0x00579D20 ?at_site@@YAHHHHH@Z 0x00579D20-0x00579D76 BYTE_EXACT
// LEVER: a `Goal *sites` pointer hoisted out of the loop, WALKED with `sites++` in the for-loop's increment (NOT "matching at_goal's own lever" - at_goal was still INDEXED when this was written, and this same form recovered it outright; see 0x00579CC0) - the earlier TRIED note below tried the pointer only INDEXED (`sites[i]`), which keeps the same 17/40-raw/worse-similarity shape as the `Goal&` reference form; incrementing the pointer itself instead of re-scaling `i` each iteration is what the image does. BYTE_EXACT 40/40.
// size      86 bytes
// prototype int (__cdecl ?at_site@@YAHHHHH@Z)(int factionID, int type, int xCoord, int yCoord)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Does specific site exist for faction at tile? true/false
Status: Complete
*/
BOOL __cdecl at_site(int faction_id, int type, int x, int y) {
    Goal *sites = PlayersData[faction_id].sites;
    for (int i = 0; i < MaxSitesNum; i++, sites++) {
        if (sites->x == x && sites->y == y && sites->type == type) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Wipe all goals for the specified faction then recreate any active site related goals.
// ORIGINAL: 0x00579D80 ?wipe_goals@@YAXH@Z 0x00579D80-0x00579DFC BYTE_EXACT
// size      124 bytes
// prototype void (__cdecl ?wipe_goals@@YAXH@Z)(int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00579A30
Return Value: n/a
Status: Complete
*/
void __cdecl wipe_goals(int faction_id) {
    for (int i = 0; i < MaxGoalsNum; i++) {
        Goal &goals = PlayersData[faction_id].goals[i];
        int16_t priority = goals.priority;
        if (priority < 0) {
            goals.type = AI_GOAL_UNUSED;
        } else {
            goals.priority = -priority;
        }
    }
    for (i = 0; i < MaxSitesNum; i++) {
        Goal &sites = PlayersData[faction_id].sites[i];
        int16_t type = sites.type;
        if (type >= 0) {
            add_goal(faction_id, type, sites.priority, sites.x, sites.y, -1);
        }
    }
}

/*
Purpose: Initialize all goals for the specified faction.
// ORIGINAL: 0x00579E00 ?init_goals@@YAXH@Z 0x00579E00-0x00579E66 BYTE_EXACT
// LEVER: a `Goal &` local anchored the loop pointer at the wrong field;
//        direct PlayersData[...].goals[i] indexing anchors at priority.
// size      102 bytes
// prototype void (__cdecl ?init_goals@@YAXH@Z)(int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl init_goals(int faction_id) {
    for (int i = 0; i < MaxGoalsNum; i++) {
        // y BEFORE x: `store_order 0x00579E00` shows the image writing @+8
        // before @+4 in both loops.
        PlayersData[faction_id].goals[i].type = -1;
        PlayersData[faction_id].goals[i].priority = 0;
        PlayersData[faction_id].goals[i].y = 0;
        PlayersData[faction_id].goals[i].x = 0;
        PlayersData[faction_id].goals[i].base_id = 0;
    }
    for (i = 0; i < MaxSitesNum; i++) {
        PlayersData[faction_id].sites[i].type = -1;
        PlayersData[faction_id].sites[i].priority = 0;
        PlayersData[faction_id].sites[i].y = 0;
        PlayersData[faction_id].sites[i].x = 0;
        PlayersData[faction_id].sites[i].base_id = 0;
    }
}


// The 4-arg vector_dist body, inlined the same way (pact_withdraw).
static inline int del_site_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b) {
    return vector_dist(x_dist(x_point_a, x_point_b), abs(y_point_a - y_point_b));
}

/*
Purpose: Delete sites of the specified type within proximity of the tile along with related goals.
// ORIGINAL: 0x00579E70 ?del_site@@YAXHHHHH@Z 0x00579E70-0x00579F73 BYTE_EXACT
// LEVER: the image never calls vector_dist - `calls 0x00644F3A` is abs() alone,
//        called 4 times. vector_dist(x,y,sites.x,sites.y) is fully open-coded
//        as x_dist(x,sites.x) [inline, one abs inside] then abs(y-sites.y),
//        then the 2-arg vector_dist body re-abs's both (redundant but what the
//        image does) and runs the largest/smallest formula. Writing that out
//        instead of calling vector_dist() moved 9/95 -> 36/95 agreeing.
// LEVER: both `Goal&` aliases (outer `sites`, inner `goal_compare`) replaced
//        with a `Goal *` pointer HOISTED out of its loop and WALKED with the
//        for-loop's own `++` (same lever that took at_site to BYTE_EXACT) -
//        the earlier TRIED below only tried a plain array INDEX before
//        the reference, not an incrementing pointer; the increment is what
//        resolves the `lea` vs `add`/offset-by-4 choice it names. 61/95 ->
//        68/95 agreeing. TRIED: walking the inner `goal_compare` loop
//        alone, keeping the outer `sites` as a `Goal&` - ties at 61/95, no
//        change; the outer loop's own address form is what has to move.
// TRIED: testing sites.type==type via a plain array index before
//        taking the Goal& reference - no change (see the LEVER above for
//        what the remaining `lea` vs `add`/offset-by-4 choice actually was).
//        TRIED: `PlayerData *p = &PlayersData[faction_id];` in place of
//        the two PlayersData[faction_id] subscripts - dropped to 33/95.
// size      259 bytes
// prototype void (__cdecl ?del_site@@YAXHHHHH@Z)(int factionID, int type, int xCoord, int yCoord, int proximity)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
Return Value: n/a
Status: Complete
*/
void __cdecl del_site(int faction_id, int type, int x, int y, int proximity) {
    Goal *sites = PlayersData[faction_id].sites;
    for (int i = 0; i < MaxSitesNum; i++, sites++) {
        if (sites->type == type) {
            int site_y = sites->y;
            int dx = x_dist(x, sites->x);
            int dy = abs(y - site_y);
            if (vector_dist(dx, dy) <= proximity) {
                sites->type = AI_GOAL_UNUSED;
                sites->priority = 0;
                Goal *goal_compare = PlayersData[faction_id].goals;
                for (int j = 0; j < MaxGoalsNum; j++, goal_compare++) {
                    if (goal_compare->x == sites->x && goal_compare->y == sites->y &&
                        goal_compare->type == type) {
                        goal_compare->type = AI_GOAL_UNUSED;
                    }
                }
            }
        }
    }
}

/*
Purpose: Validate whether each faction meets the requirements to have the Map revealed.
// ORIGINAL: 0x005A96D0 ?see_map_check@@YAXXZ 0x005A96D0-0x005A9753 BYTE_EXACT
// LEVER: the image runs BOTH loops unconditionally, every time - no early `break` out of the 4-satellite loop (it always checks all four, re-OR-ing the same bit), and no `if (!PFLAG_MAP_REVEALED)` guard skipping the tech loop once the satellites already set it. Removing both "optimisations" (which changed nothing observable, only the instruction count) reached BYTE_EXACT.
// size      131 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
// LEVER (2026-09-03): `has_tech_call` at the tech loop. 0 of 51 agreeing,
//   66 instructions against the image's 51, because technology.h's MEASURED
//   inline was open-coded at the one `has_tech` site in the body. With the
//   forwarder the whole body dropped to the image's own 51 AND the winning
//   flag set flipped to `/O2 /Oy- /GX`, which is what supplies the image's
//   `push ebp / mov ebp, esp` frame - the /O2 build could never match the
//   prologue, so the bloat was masking the right set, not just the right
//   size. 51 of 51, BYTE_EXACT.
Return Value: n/a
Status: Complete
*/
void __cdecl see_map_check() {
    for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
        PlayersData[faction_id].flags &= ~PFLAG_MAP_REVEALED;
        uint32_t *satellites = PlayersData[faction_id].satellites_;
        for (int i = 0; i < 4; i++, satellites++) {
            if (*satellites) {
                PlayersData[faction_id].flags |= PFLAG_MAP_REVEALED;
            }
        }
        for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
            // has_tech_call, not has_tech: the image CALLS 0x005B9F20 once
            // from here (0x005A9718) and technology.h's MEASURED inline was
            // open-coded into this body.
            if (Technology[tech_id].flags & TFLAG_REVEALS_MAP
                && has_tech(tech_id, faction_id)) {
                PlayersData[faction_id].flags |= PFLAG_MAP_REVEALED;
            }
        }
    }
}


void __cdecl enemy_capabilities_t(uint32_t faction_id) {
    // * PSI could potentially be best weapon?
    // * PSI should always be last Weapon
    // * faction order will affect initial run through
    // > potential fix would be to calculate all factions at once before enemy best compares

    //BOOL has_worms = veh_avail(BSC_MIND_WORMS, faction_id, -1);
    //PlayersData[faction_id].bestPsiAtkVal = has_worms ? weap_strat(WPN_PSI_ATTACK, faction_id) : 0;
    PlayersData[faction_id].best_weapon_value = 1;
    for (int i = 0; i < MaxWeaponNum; i++) {
        if (has_tech(Weapon[i].preq_tech, faction_id) && Weapon[i].offense_rating < 99) {
            int weap_val = weap_strat(i, faction_id);
            //if (Weapon[i].offense_rating < 0 && weap_val > PlayersData[faction_id].bestPsiAtkVal) {
            //    PlayersData[faction_id].bestPsiAtkVal = weap_val;
            //}
            if (Weapon[i].offense_rating >= 0 && weap_val
                > PlayersData[faction_id].best_weapon_value) {
                PlayersData[faction_id].best_weapon_value = weap_val;
            }
        }
    }
    //PlayersData[faction_id].bestPsiDefVal = has_worms ? arm_strat(ARM_PSI_DEFENSE, faction_id) : 0;
    PlayersData[faction_id].best_armor_value = 1;
    for (i = 0; i < MaxArmorNum; i++) {
        if (has_tech(Armor[i].preq_tech, faction_id)) {
            int arm_val = arm_strat(i, faction_id);
            //if (Armor[i].defense_rating < 0 && arm_val > PlayersData[faction_id].bestPsiDefVal) {
            //    PlayersData[faction_id].bestPsiDefVal = arm_val;
            //}
            if (Armor[i].defense_rating >= 0
                && arm_val > PlayersData[faction_id].best_armor_value) {
                PlayersData[faction_id].best_armor_value = arm_val;
            }
        }
    }
    PlayersData[faction_id].best_land_speed = 1;
    for (i = 0; i < MaxChassisNum; i++) {
        if (has_tech(Chassis[i].preq_tech, faction_id) && Chassis[i].triad == TRIAD_LAND) {
            if (Chassis[i].speed > PlayersData[faction_id].best_land_speed) {
                PlayersData[faction_id].best_land_speed = Chassis[i].speed;
            }
        }
    }
    PlayersData[faction_id].enemy_best_weapon_value = 0;
    PlayersData[faction_id].enemy_best_armor_value = 0;
    PlayersData[faction_id].enemy_best_land_speed = 0;
    //PlayersData[faction_id].enemyBestPsiAtkVal = 0;
    //PlayersData[faction_id].enemyBestPsiDefVal = 0;
    for (i = 0; i < 4 && !PlayersData[faction_id].enemy_best_weapon_value; i++) {
        // 1st pass: vendetta, no treaty, has commlink
        // 2nd pass: no treaty, has commlink
        // 3rd pass: has commlink
        // 4th pass: any non-pact faction
        for (uint32_t j = 1, treaties; j < MaxPlayerNum; j++) {
            if (j != faction_id
                && (treaties = PlayersData[i].diplo_treaties[j], !(treaties & DTREATY_PACT))
                && ((!i && treaties & DTREATY_VENDETTA && !(treaties & DTREATY_TREATY)
                    && treaties & DTREATY_COMMLINK)
                    || (i == 1 && !(treaties & DTREATY_TREATY) && treaties & DTREATY_COMMLINK)
                    || (i == 2 && treaties & DTREATY_COMMLINK) || (i == 3))) {
                if (PlayersData[faction_id].enemy_best_weapon_value
                    < PlayersData[j].best_weapon_value) {
                    PlayersData[faction_id].enemy_best_weapon_value
                        = PlayersData[j].best_weapon_value;
                }
                if (PlayersData[faction_id].enemy_best_armor_value
                    < PlayersData[j].best_armor_value) {
                    PlayersData[faction_id].enemy_best_armor_value
                        = PlayersData[j].best_armor_value;
                }
                if (PlayersData[faction_id].enemy_best_land_speed
                    < PlayersData[j].best_land_speed) {
                    PlayersData[faction_id].enemy_best_land_speed = PlayersData[j].best_land_speed;
                }
                /*
                if (PlayersData[faction_id].enemyBestPsiAtkVal < PlayersData[j].bestPsiAtkVal) {
                    PlayersData[faction_id].enemyBestPsiAtkVal = PlayersData[j].bestPsiAtkVal;
                }
                if (PlayersData[faction_id].enemyBestPsiDefVal < PlayersData[j].bestPsiDefVal) {
                    PlayersData[faction_id].enemyBestPsiDefVal = PlayersData[j].bestPsiDefVal;
                }
                */
            }
        }
    }
    // PSI
    BOOL has_worms = veh_avail(BSC_MIND_WORMS, faction_id, -1);
    PlayersData[faction_id].best_psi_offense = has_worms
        ? weap_strat(WPN_PSI_ATTACK, faction_id) : 0;
    for (i = 0; i < MaxWeaponNum; i++) {
        if (has_tech(Weapon[i].preq_tech, faction_id) && Weapon[i].offense_rating < 99) {
            int weap_val = weap_strat(i, faction_id);
            if (Weapon[i].offense_rating < 0 && weap_val
                > PlayersData[faction_id].best_psi_offense) {
                PlayersData[faction_id].best_psi_offense = weap_val;
            }
        }
    }
    PlayersData[faction_id].best_psi_defense = has_worms
        ? arm_strat(ARM_PSI_DEFENSE, faction_id) : 0;
    for (i = 0; i < MaxArmorNum; i++) {
        if (has_tech(Armor[i].preq_tech, faction_id)) {
            int arm_val = arm_strat(i, faction_id);
            if (Armor[i].defense_rating < 0 && arm_val > PlayersData[faction_id].best_psi_defense) {
                PlayersData[faction_id].best_psi_defense = arm_val;
            }
        }
    }
    PlayersData[faction_id].enemy_best_psi_offense = 0;
    PlayersData[faction_id].enemy_best_psi_defense = 0;
    for (i = 0; i < 4 && !PlayersData[faction_id].enemy_best_psi_offense; i++) {
        // 1st pass: vendetta, no treaty, has commlink
        // 2nd pass: no treaty, has commlink
        // 3rd pass: has commlink
        // 4th pass: any non-pact faction
        for (uint32_t j = 1, treaties; j < MaxPlayerNum; j++) {
            if (j != faction_id
                && (treaties = PlayersData[i].diplo_treaties[j], !(treaties & DTREATY_PACT))
                && ((!i && treaties & DTREATY_VENDETTA && !(treaties & DTREATY_TREATY)
                    && treaties & DTREATY_COMMLINK)
                    || (i == 1 && !(treaties & DTREATY_TREATY) && treaties & DTREATY_COMMLINK)
                    || (i == 2 && treaties & DTREATY_COMMLINK) || (i == 3))) {
                if (PlayersData[faction_id].enemy_best_psi_offense
                    < PlayersData[j].best_psi_offense) {
                    PlayersData[faction_id].enemy_best_psi_offense
                        = PlayersData[j].best_psi_offense;
                }
                if (PlayersData[faction_id].enemy_best_psi_defense
                    < PlayersData[j].best_psi_defense) {
                    PlayersData[faction_id].enemy_best_psi_defense
                        = PlayersData[j].best_psi_defense;
                }
            }
        }
    }
}


/*
Purpose: Apply a treaty between two factions, or - in a network game - log
         it, broadcast it and wait for the receiving side's ack instead.
// ORIGINAL: 0x005391C0 ?net_treaty_on@@YAXHHHH@Z 0x005391C0-0x0053922A BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_treaty_on@@YAXHHHH@Z)(int, int, int, int)
// callers   4   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055B870 0x00592EE0 0x006262F0
// LEVER: `IsMultiplayerNet` for 0x0093F660 and `NetDaemonNet` for the
//        0x0093CD90 receiver - the artifact's fixed-address casts lower to
//        the same two immediates once the names are real objects. Homed
//        from the recovered archive.
Status: Complete
*/
void __cdecl net_treaty_on(int faction_id, int faction_id_with, int treaty,
                           int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, treaty);
        message_data(0x2441, 0, faction_id, faction_id_with, treaty, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x441);
        }
    } else {
        treaty_on(faction_id, faction_id_with, treaty);
    }
}
/*
Purpose: net_treaty_on's mirror - clear a treaty, or broadcast the clearing.
// ORIGINAL: 0x00539230 ?net_treaty_off@@YAXHHHH@Z 0x00539230-0x0053929A BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_treaty_off@@YAXHHHH@Z)(int, int, int, int)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055B760 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_treaty_off(int faction_id, int faction_id_with, int treaty,
                            int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, treaty);
        message_data(0x2442, 0, faction_id, faction_id_with, treaty, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x442);
        }
    } else {
        treaty_off(faction_id, faction_id_with, treaty);
    }
}
/*
Purpose: net_treaty_on's five-argument form - set or clear one treaty bit in
         one move, so the caller does not have to know which direction it is.
// ORIGINAL: 0x005392A0 ?net_set_treaty@@YAXHHHHH@Z 0x005392A0-0x00539310 BYTE_EXACT
// size      112 bytes
// prototype void (__cdecl ?net_set_treaty@@YAXHHHHH@Z)(int, int, int, int, int)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055BB30 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_set_treaty(int faction_id, int faction_id_with, int treaty,
                            int set, int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingSETTREATY, faction_id, faction_id_with, treaty);
        message_data(0x2443, 0, faction_id, faction_id_with, treaty, set);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x443);
        }
    } else {
        set_treaty(faction_id, faction_id_with, treaty, set);
    }
}
/*
Purpose: net_treaty_on's agenda counterpart.
// ORIGINAL: 0x00539310 ?net_agenda_on@@YAXHHHH@Z 0x00539310-0x0053937A BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_agenda_on@@YAXHHHH@Z)(int, int, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055BA80 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_agenda_on(int faction_id, int faction_id_with, int agenda,
                           int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingAGENDAON, faction_id, faction_id_with, agenda);
        message_data(0x2444, 0, faction_id, faction_id_with, agenda, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x444);
        }
    } else {
        agenda_on(faction_id, faction_id_with, agenda);
    }
}
/*
Purpose: net_agenda_on's mirror.
// ORIGINAL: 0x00539380 ?net_agenda_off@@YAXHHHH@Z 0x00539380-0x005393EA BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_agenda_off@@YAXHHHH@Z)(int, int, int, int)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055B820 0x00592EE0 0x006262F0
// LEVER: the archive transcription had an early `return` inside the
//        `wait != 0` branch. The image has none - it falls through to the
//        shared pop/je tail every other net_* body ends with, and the extra
//        jump moves the whole tail. Written like net_agenda_on it matches.
Status: Complete
*/
void __cdecl net_agenda_off(int faction_id, int faction_id_with, int agenda,
                            int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingAGENDAOFF, faction_id, faction_id_with, agenda);
        message_data(0x2445, 0, faction_id, faction_id_with, agenda, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x445);
        }
    } else {
        agenda_off(faction_id, faction_id_with, agenda);
    }
}
/*
Purpose: net_set_treaty's agenda counterpart.
// ORIGINAL: 0x005393F0 ?net_set_agenda@@YAXHHHHH@Z 0x005393F0-0x00539460 BYTE_EXACT
// size      112 bytes
// prototype void (__cdecl ?net_set_agenda@@YAXHHHHH@Z)(int, int, int, int, int)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055BBA0 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_set_agenda(int faction_id, int faction_id_with, int agenda,
                            int set, int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingSETAGENDA, faction_id, faction_id_with, agenda);
        message_data(0x2446, 0, faction_id, faction_id_with, agenda, set);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x446);
        }
    } else {
        set_agenda(faction_id, faction_id_with, agenda, set);
    }
}


/*
Purpose: Hand energy from one faction to another - either by broadcasting the
         transfer, or out of a network game by applying it straight to both
         sides' reserves and refreshing the Console's display.

         Note the log_say argument order: the label is followed by faction_id,
         then the AMOUNT and then the counterparty, which is not the order
         message_data takes them in.
// ORIGINAL: 0x00539460 ?net_energy@@YAXHHHHH@Z 0x00539460-0x00539506 BYTE_EXACT
// size      166 bytes
// prototype void (__cdecl ?net_energy@@YAXHHHHH@Z)(int, int, int, int, int)
// callers   3   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00514880 0x005315C0 0x00592EE0 0x006262F0
// LEVER: 0x0096CC00 is `&PlayersData[0].energy_reserves`, so the image's
//        `add [reg*4 + 0x96cc00]` is an ordinary field update once the
//        0x20CC PlayerData stride is named - 2099 dwords per faction.
Status: Complete
*/
void __cdecl net_energy(int faction_id, int amount, int faction_id_with,
                        int amount_with, int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingENERGY, faction_id, faction_id_with, amount);
        message_data(0x2447, 0, faction_id, amount, faction_id_with, amount_with);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x447);
            return;
        }
    } else {
        if (faction_id != 0) {
            PlayersData[faction_id].energy_reserves += amount;
        }
        if (faction_id_with != 0) {
            PlayersData[faction_id_with].energy_reserves += amount_with;
        }
        ConsoleGlobal.update_data(1);
    }
}

/*
Purpose: Record what one faction owes another for a trade - the outstanding
         balance and the per-turn payment - out of a network game, or
         broadcast the same in.
// ORIGINAL: 0x00539510 ?net_loan@@YAXHHHH@Z 0x00539510-0x0053957F BYTE_EXACT
// size      99 bytes
// prototype void (__cdecl ?net_loan@@YAXHHHH@Z)(int, int, int, int)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x00592EE0 0x006262F0
// RESIDUAL: the annotation's span is SHORT. `0x00539510-0x0053957F` (and its
//        own "size 99 bytes", inconsistent with that same end) stops inside
//        the penultimate store. The shipped bytes run
//        `89 88 a4 ca 96 00 | 89 90 c4 ca 96 00 | 5d c3` from 0x00539571, so
//        the function really ends at 0x0053957E - the `mov
//        [eax + 0x96cac4], edx` and the epilogue are outside the recorded
//        span and count as this tree's extra instructions. Every instruction
//        the span does carry agrees (43 of 43). Demoted for the span, not
//        for the body.
Status: Complete
*/
void __cdecl net_loan(int faction_id, int faction_id_with, int balance,
                      int payment) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingLOAN, faction_id, faction_id_with, balance);
        message_data(0x244e, 0, faction_id, faction_id_with, balance, payment);
    } else {
        // The image materialises the byte offset once (`shl eax, 2`) and
        // stores through `[eax + <table>]` twice - an index expression at
        // each store lets VC6 keep the scale in the SIB byte instead, which
        // is two instructions longer. Same for net_energy's single update.
        PlayersData[faction_id].loan_balance[faction_id_with] = balance;
        PlayersData[faction_id].loan_payment[faction_id_with] = payment;
    }
}

/*
Purpose: Swap world maps with another faction, locally or over the network.
// ORIGINAL: 0x00539580 ?net_maps@@YAXHHH@Z 0x00539580-0x005395E3 BYTE_EXACT
// size      99 bytes
// prototype void (__cdecl ?net_maps@@YAXHHH@Z)(int, int, int)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x00543A20 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_maps(int faction_id, int faction_id_with, int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTRADEMAPS, faction_id, faction_id_with, 0);
        message_data(0x2448, 0, faction_id, faction_id_with, 0, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x448);
        }
    } else {
        trade_maps(faction_id, faction_id_with);
    }
}

/*
Purpose: Give another faction a technology, locally or over the network.
// ORIGINAL: 0x005395F0 ?net_tech@@YAXHHHH@Z 0x005395F0-0x0053965C BYTE_EXACT
// size      108 bytes
// prototype void (__cdecl ?net_tech@@YAXHHHH@Z)(int, int, int, int)
// callers   3   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x00592EE0 0x005BB000 0x006262F0
Status: Complete
*/
void __cdecl net_tech(int faction_id, int faction_id_with, int tech_id,
                      int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTECH, faction_id, faction_id_with, tech_id);
        message_data(0x244b, 0, faction_id, faction_id_with, tech_id, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x44b);
        }
    } else {
        tech_achieved(faction_id, faction_id_with, tech_id, 0);
    }
}

/*
Purpose: Tell the local player that a pact between two AI factions has ended,
         and clear it - locally or over the network.
// ORIGINAL: 0x00539660 ?net_pact_ends@@YAXHHH@Z 0x00539660-0x005396C1 BYTE_EXACT
// size      97 bytes
// prototype void (__cdecl ?net_pact_ends@@YAXHHH@Z)(int, int, int)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0053C650 0x00592EE0 0x006262F0
// LEVER: this one logs the message with a zeroed argument triple - the pact
//        that ended is named in the message_data payload, not in the log
//        line, so the log call has nothing to interpolate.
Status: Complete
*/
void __cdecl net_pact_ends(int faction_id, int faction_id_with, int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingPACTENDS, 0, 0, 0);
        message_data(0x244d, 0, faction_id, faction_id_with, 0, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x44d);
        }
    } else {
        pact_ends(faction_id, faction_id_with);
    }
}

/*
Purpose: Withdraw from a base-to-base or faction-level contest. In a network
         game the announcement is broadcast and the call reports success; out
         of one, the local withdrawal decides the result.
// ORIGINAL: 0x005396D0 ?net_withdrawal@@YAXHHH@Z 0x005396D0-0x00539738 BYTE_EXACT
// size      104 bytes
// prototype void (__cdecl ?net_withdrawal@@YAXHHH@Z)(int, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x00549270 0x00592EE0 0x006262F0
// LEVER: the catalogue's `void` return is wrong here and the archive already
//        said so: both paths put something in EAX (1 on the network path, the
//        do_withdrawal result otherwise) and neither pops arguments, so this
//        is `int __cdecl`. That spells `?net_withdrawal@@YAHHHH@Z`, which no
//        image symbol carries - the catalogue's `X` is what the export table
//        has - so the `symbol` fact below names the image's spelling and the
//        body is matched against it anyway.
// symbol    ?net_withdrawal@@YAHHHH@Z
Status: Complete
*/
int __cdecl net_withdrawal(int faction_id, int faction_id_with, int wait) {
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingWITHDRAWAL, faction_id, faction_id_with, 0);
        message_data(0x244a, 0, faction_id, faction_id_with, 0, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x44a);
        }
        return 1;
    }
    return do_withdrawal(faction_id, faction_id_with);
}

/*
Purpose: Hand one of your bases to another faction - locally or over the
         network.
// ORIGINAL: 0x00539740 ?net_cede_base@@YAXHHH@Z 0x00539740-0x005397A3 BYTE_EXACT
// size      99 bytes
// prototype void (__cdecl ?net_cede_base@@YAXHHH@Z)(int, int, int)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0054CF40 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_cede_base(int faction_id, int faction_id_with, int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingCEDEBASE, faction_id, faction_id_with, 0);
        message_data(0x2449, 0, faction_id, faction_id_with, 0, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x449);
        }
    } else {
        give_a_base(faction_id, faction_id_with);
    }
}

/*
Purpose: Expose a faction that offered to betray its own ally - locally or
         over the network. Unlike net_withdrawal one address down, this one
         really is void: neither path leaves a meaningful value in EAX.
// ORIGINAL: 0x005397B0 ?net_double_cross@@YAXHHHH@Z 0x005397B0-0x0053981A BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_double_cross@@YAXHHHH@Z)(int, int, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0050BCC0 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_double_cross(int faction_id, int faction_id_with,
                              int victim,
                              int wait) {
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingDOUBLECROSS, faction_id, faction_id_with, victim);
        message_data(0x2440, 0, faction_id, faction_id_with, victim, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x440);
        }
    } else {
        double_cross(faction_id, faction_id_with, victim);
    }
}

/*
Purpose: Release the diplomacy lock diplo_lock took, and log that it went.
// ORIGINAL: 0x005398C0 ?diplo_unlock@@YAXXZ 0x005398C0-0x005398DD BYTE_EXACT
// size      29 bytes
// prototype void (__cdecl ?diplo_unlock@@YAXXZ)(void)
// callers   10   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00530D00 0x006262F0
Status: Complete
*/
void __cdecl diplo_unlock() {
    log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
    NetDaemonNet.unlock();
}

/*
Purpose: Let a modal popup wait for the network to catch up. Out of a network
         game there is nothing to wait for and the caller carries on; with the
         turn loop already exiting there is nothing left to serve either.
// ORIGINAL: 0x005398E0 ?pop_wait@@YAHXZ 0x005398E0-0x0053991F BYTE_EXACT
// size      64 bytes
// prototype int (__cdecl ?pop_wait@@YAHXZ)(void)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0051F200
Status: Complete
*/
int __cdecl pop_wait() {
    if (IsMultiplayerNet == 0) {
        return 1;
    }
    if (ExitTurnLoop != 0) {
        return 0;
    }
    PopupWaitActive = 1;
    wait_loop();
    PopupWaitActive = 0;
    return 1;
}

/*
Purpose: pop_wait's expiry check - same wait, but only while the game is
         still running a network turn, and never once the scenario's
         bonus-at-end window has opened.
// ORIGINAL: 0x00539920 ?pop_expire@@YAHXZ 0x00539920-0x0053996F BYTE_EXACT
// size      79 bytes
// prototype int (__cdecl ?pop_expire@@YAHXZ)(void)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0051F200
Status: Complete
*/
int __cdecl pop_expire() {
    if (IsMultiplayerNet != 0) {
        if (ExitTurnLoop != 0) {
            return 0;
        }
        if (GameDataStateStorage.fields_.game_state_ & STATE_UNK_800) {
            return 0;
        }
        PopupWaitActive = 1;
        wait_loop();
        PopupWaitActive = 0;
    }
    return 1;
}

// Original 0x005398F5/0x00539904 and 0x00539942/0x00539951 bracket wait_loop with 1/0; start only.
// RECOVERY_OBJECT: v1 0x0093FAC4 ?PopupWaitActive@@3HA
int PopupWaitActive;  // 0x0093FAC4


// Shared diplomacy evaluation state. Neutral names retain independently
// observed original slots until their meanings can be named conclusively.
// Original faction-indexed reads establish the known eight game slots;
// complete physical allocation is unresolved, hence a start-only binding.
// RECOVERY_OBJECT: v1 0x0093F678 ?FactionDiplomacyFlags@@3PAIA
uint32_t FactionDiplomacyFlags[MaxPlayerNum];
// setup_game clears exactly 64 DWORDs; DiploWin::exec sets a pair flag at
// faction_id_with_ * 8 + faction_id_. net_control releases one current-faction
// row. Keep the two faction indices distinct; this is shared state, not a
// member of whichever dialogue happened to discover the pair.
// RECOVERY_OBJECT: v2 0x0093F698 0x0093F798 ?PendingDiplomacyFlags@@3PAY07IA 0x005B3920
uint32_t PendingDiplomacyFlags[MaxPlayerNum][MaxPlayerNum];
// Original region walk clears indices1..127, deliberately retaining index0.
// RECOVERY_OBJECT: v1 0x0093F818 ?DiplomacyRegionMask@@3PAIA
uint32_t DiplomacyRegionMask[128];
// RECOVERY_OBJECT: v1 0x0093F7B4 ?DiploScratchF7B4@@3HA
// communicate clears this DWORD at 55778A, writes one at 55782E, and tests
// it at 55792C before the ordered agenda update at 557935. The broader
// allocation/lifetime/meaning remain unproved; this is a start-only binding.
int DiploScratchF7B4;


// RECOVERY_OBJECT: v1 0x0093F7B8 ?DiploScratchF7B8@@3HA
int DiploScratchF7B8;
// RECOVERY_OBJECT: v1 0x0093F7C8 ?DiploScratchF7C8@@3HA
int DiploScratchF7C8;
// RECOVERY_OBJECT: v1 0x0093F7D4 ?DiploScratchF7D4@@3HA
int DiploScratchF7D4;
// RECOVERY_OBJECT: v1 0x0093F7D8 ?DiploScratchF7D8@@3HA
int DiploScratchF7D8;
// RECOVERY_OBJECT: v1 0x0093F7E0 ?DiploScratchF7E0@@3HA
int DiploScratchF7E0;
// RECOVERY_OBJECT: v1 0x0093F7E4 ?DiploScratchF7E4@@3HA
int DiploScratchF7E4;
// RECOVERY_OBJECT: v1 0x0093F7E8 ?DiploScratchF7E8@@3HA
int DiploScratchF7E8;
// RECOVERY_OBJECT: v1 0x0093F7F0 ?DiploScratchF7F0@@3HA
int DiploScratchF7F0;
// RECOVERY_OBJECT: v1 0x0093F7F8 ?DiploScratchF7F8@@3HA
int DiploScratchF7F8;
// RECOVERY_OBJECT: v1 0x0093F7FC ?DiploScratchF7FC@@3HA
int DiploScratchF7FC;
// RECOVERY_OBJECT: v1 0x0093F808 ?DiploScratchF808@@3HA
int DiploScratchF808;
// RECOVERY_OBJECT: v1 0x0093FA24 ?DiploScratchFA24@@3HA
int DiploScratchFA24;

// Neutral cells written by battle_plans and read by battle_routine. The
// pinned initial DWORDs are zero; names are source roles, not recovered
// original COFF symbols.
// RECOVERY_OBJECT: v1 0x0093F7BC ?PlanBattleFactionA@@3HA
int PlanBattleFactionA;
// RECOVERY_OBJECT: v1 0x0093F810 ?PlanBattleFactionB@@3HA
int PlanBattleFactionB;

// Mutable label storage. Pinned data proves each start and literal bytes;
// v1 object bindings intentionally make no original-extent claim. Each unsized
// definition below uses only minimal null-terminated source storage.
// RECOVERY_OBJECT: v1 0x0068DD20 ?BattlePlansNoActionLabel@@3PADA
char BattlePlansNoActionLabel[] = "NEVERBATTLEPLANS";
// RECOVERY_OBJECT: v1 0x0068DD34 ?BattlePlansMapSharingLabel@@3PADA
char BattlePlansMapSharingLabel[] = "MAPBATTLEPLANS";
// RECOVERY_OBJECT: v1 0x0068DD44 ?BattlePlansInvalidPlanLabel@@3PADA
char BattlePlansInvalidPlanLabel[] = "NOBATTLEPLANS";
// RECOVERY_OBJECT: v1 0x0068DD54 ?BattlePlansLabel@@3PADA
char BattlePlansLabel[] = "BATTLEPLANS";
// RECOVERY_OBJECT: v1 0x0068DD60 ?BattlePlanLabel@@3PADA
char BattlePlanLabel[] = "BATTLEPLAN";
// RECOVERY_OBJECT: v1 0x0068DD6C ?BattlePlansRegionRejectedLabel@@3PADA
char BattlePlansRegionRejectedLabel[] = "BATTLENO";
// RECOVERY_OBJECT: v1 0x0068DD78 ?BattlePlansDistanceRejectedLabel@@3PADA
char BattlePlansDistanceRejectedLabel[] = "BATTLENO";
// RECOVERY_OBJECT: v1 0x0068DD84 ?BattlePlansSelectionLabel@@3PADA
char BattlePlansSelectionLabel[] = "BATTLEYES";

// RECOVERY_OBJECT: v1 0x0093FA2C ?DiploScratchFA2C@@3HA
int DiploScratchFA2C;
// RECOVERY_OBJECT: v1 0x0093FA38 ?DiploScratchFA38@@3HA
int DiploScratchFA38;
// RECOVERY_OBJECT: v1 0x0093FA3C ?DiploScratchFA3C@@3HA
int DiploScratchFA3C;
// communicate clears this DWORD at 0x00550080; tech_trade reads it at
// 0x005414ED and sets it at 0x0054200F. battle_plans reads it at 0x0054B2AA.
// The initial image word is zero. This neutral scalar models the observed
// accesses only: original allocation extent, meaning and full producer
// lifecycle remain unverified. Neighboring bindings do not establish bounds.
// RECOVERY_OBJECT: v1 0x0093FA48 ?DiploScratchFA48@@3HA
int DiploScratchFA48;
// RECOVERY_OBJECT: v1 0x0093FA4C ?DiploScratchFA4C@@3HA
int DiploScratchFA4C;
// RECOVERY_OBJECT: v1 0x0093FA50 ?DiploScratchFA50@@3HA
int DiploScratchFA50;
// RECOVERY_OBJECT: v1 0x0093FA54 ?DiploScratchFA54@@3HA
int DiploScratchFA54;
// RECOVERY_OBJECT: v1 0x0093FA58 ?DiploScratchFA58@@3HA
int DiploScratchFA58;
// RECOVERY_OBJECT: v1 0x0093FA60 ?DiploScratchFA60@@3HA
int DiploScratchFA60;
// RECOVERY_OBJECT: v1 0x0093FA64 ?DiploScratchFA64@@3HA
int DiploScratchFA64;
// RECOVERY_OBJECT: v1 0x0093FA68 ?DiploScratchFA68@@3HA
int DiploScratchFA68;
// RECOVERY_OBJECT: v1 0x0093FA70 ?DiploScratchFA70@@3HA
int DiploScratchFA70;
// RECOVERY_OBJECT: v1 0x0093FA78 ?DiploScratchFA78@@3HA
int DiploScratchFA78;
// RECOVERY_OBJECT: v1 0x0093FA80 ?DiploScratchFA80@@3HA
int DiploScratchFA80;
// RECOVERY_OBJECT: v1 0x0093FA88 ?DiploScratchFA88@@3HA
int DiploScratchFA88;
// communicate clears this separate DWORD at 0x0055003B and writes it at
// 0x005516C2; propose_pact/propose_treaty read it at 0x00543C01/0x00544EA9.
// Initial image storage is zero. The provisional scalar/start binding does
// not prove original allocation extent, semantics or complete lifecycle;
// do not alias it to the independently accessed neighboring scratch cells.
// RECOVERY_OBJECT: v1 0x0093FA8C ?DiploScratchFA8C@@3HA
int DiploScratchFA8C;
// RECOVERY_OBJECT: v1 0x0093FA90 ?DiploScratchFA90@@3HA
int DiploScratchFA90;
// RECOVERY_OBJECT: v1 0x0093FA98 ?DiploScratchFA98@@3HA
int DiploScratchFA98;
// RECOVERY_OBJECT: v1 0x0093FA9C ?DiploScratchFA9C@@3HA
int DiploScratchFA9C;
// RECOVERY_OBJECT: v1 0x0093FAA0 ?DiploScratchFAA0@@3HA
int DiploScratchFAA0;
// Native tech_analysis initializes all four IDs to -1 before ranking offers.
// Separate source-owned cells preserve the original sparse shared storage;
// these declarations do not replace the still-seeded producer implementation.
// RECOVERY_OBJECT: v2 0x0093FAA8 0x0093FAAC ?DiploScratchFAA8@@3HA 0x0053E800
int DiploScratchFAA8;
// RECOVERY_OBJECT: v2 0x0093FA18 0x0093FA1C ?DiploScratchFA18@@3HA 0x0053E800
int DiploScratchFA18;
// RECOVERY_OBJECT: v2 0x0093FA1C 0x0093FA20 ?DiploScratchFA1C@@3HA 0x0053E800
int DiploScratchFA1C;
// RECOVERY_OBJECT: v2 0x0093FA28 0x0093FA2C ?DiploScratchFA28@@3HA 0x0053E800
int DiploScratchFA28;
// RECOVERY_OBJECT: v2 0x0093F7C0 0x0093F7C4 ?DiploScratchF7C0@@3HA 0x0053E800
int DiploScratchF7C0;
// tech_trade clears this distinct dword at 0x00541158 and sets it at
// 0x00543094. It is not part of F7C0, whose original extent ends here.
// Keep a neutral name until its full producer/consumer role is established.
// RECOVERY_OBJECT: v2 0x0093F7C4 0x0093F7C8 ?DiploScratchF7C4@@3HA 0x00541100
int DiploScratchF7C4;
// RECOVERY_OBJECT: v2 0x0093F7EC 0x0093F7F0 ?DiploScratchF7EC@@3HA 0x0053E800
int DiploScratchF7EC;
// RECOVERY_OBJECT: v2 0x0093F7F4 0x0093F7F8 ?DiploScratchF7F4@@3HA 0x0053E800
int DiploScratchF7F4;
// RECOVERY_OBJECT: v2 0x0093F800 0x0093F804 ?DiploScratchF800@@3HA 0x0053E800
int DiploScratchF800;
// RECOVERY_OBJECT: v2 0x0093F80C 0x0093F810 ?DiploScratchF80C@@3HA 0x0053E800
int DiploScratchF80C;
// RECOVERY_OBJECT: v2 0x0093F814 0x0093F818 ?DiploScratchF814@@3HA 0x0053E800
int DiploScratchF814;
// RECOVERY_OBJECT: v2 0x0093FA20 0x0093FA24 ?DiploScratchFA20@@3HA 0x0053E800
int DiploScratchFA20;
// RECOVERY_OBJECT: v2 0x0093FA44 0x0093FA48 ?DiploScratchFA44@@3HA 0x0053E800
int DiploScratchFA44;
// RECOVERY_OBJECT: v2 0x0093FA6C 0x0093FA70 ?DiploScratchFA6C@@3HA 0x0053E800
int DiploScratchFA6C;
// RECOVERY_OBJECT: v2 0x0093FA84 0x0093FA88 ?DiploScratchFA84@@3HA 0x0053E800
int DiploScratchFA84;
// RECOVERY_OBJECT: v2 0x0093FAA4 0x0093FAA8 ?DiploScratchFAA4@@3HA 0x0053E800
int DiploScratchFAA4;

// Rank four technology offers and two incoming requests. Project-linked
// requests keep their native discount and suppress the secondary project bid.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0053E800 ?tech_analysis@@YAXHH@Z 0x0053E800-0x0053EB42
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0053e800.cpp", "src/base.h", "recovery/tests/test_tech_analysis_behavior.py"], "reason": "Complete technology trade ranking producer in the faction owner using genuine Player, PlayerData, Base and Facility storage, shared ranked result cells and existing project lookup. Native64project loop uses Facility stride30 and first prerequisite Facility70, correcting retained scaffold0C stride."}
void __cdecl tech_analysis(int faction_id, int faction_id_with) {
    DiploScratchF7F4 = -1;
    DiploScratchFA28 = -1;
    DiploScratchFA1C = -1;
    DiploScratchFA18 = -1;
    DiploScratchFAA8 = -1;
    DiploScratchF80C = -1;
    DiploScratchF800 = -1;
    DiploScratchF7EC = 0;
    DiploScratchFA44 = 0;
    DiploScratchFAA4 = 0;
    DiploScratchF814 = 0;
    DiploScratchF7C0 = 0;
    DiploScratchFA20 = 0;
    DiploScratchFA84 = -1;
    DiploScratchFA6C = -1;

    if (GameDataStateStorage.fields_.game_rules_ & 0x2000000) {
        return;
    }
    for (int i = 0; i < 0x59; i++) {
        if (has_tech(i, faction_id_with)) {
            if (has_tech(i, faction_id)) {
                continue;
            }
            int matched = -1;
            if (PlayersData[faction_id].diff_level >= 3 ||
                !strcmp(Players[faction_id_with].filename, "HIVE")) {
                for (int project = 0; project < MaxSecretProjectNum; ++project) {
                    if (base_project(project) != SP_Unbuilt ||
                        Facility[70 + project].preq_tech != i) {
                        continue;
                    }
                    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                        if (Bases[base].faction_id_current == faction_id_with &&
                            Bases[base].queue_production_id[0] == -project - 0x46) {
                            DiploScratchF7F4 = matched = project;
                        }
                    }
                    if (((PlayersData[faction_id].diff_level >= 5 &&
                          (!IsMultiplayerNet || faction_id != static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk2_))) ||
                         (IsMultiplayerNet && faction_id == static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk1_))) &&
                        DiploScratchF7F4 == -1) {
                        DiploScratchF7F4 = matched = -2 - project;
                    }
                }
            }
            int v = tech_val(i, faction_id, 0);
            if (matched != -1) {
                v = (v + 1) / 2;
            }
            if (v >= DiploScratchFAA4) {
                DiploScratchF7EC = DiploScratchFAA4;
                DiploScratchF80C = DiploScratchF800;
                DiploScratchFA84 = DiploScratchFA6C;
                DiploScratchFAA4 = v;
                DiploScratchF800 = i;
                DiploScratchFA6C = matched;
            } else if (v >= DiploScratchF7EC) {
                DiploScratchF7EC = v;
                DiploScratchF80C = i;
                DiploScratchFA84 = matched;
            }
        } else if (has_tech(i, faction_id)) {
            int v = tech_val(i, faction_id_with, 0);
            if (v >= DiploScratchF814) {
                DiploScratchF7C0 = DiploScratchFA20;
                DiploScratchFA20 = DiploScratchFA44;
                DiploScratchFA44 = DiploScratchF814;
                DiploScratchF814 = v;
                DiploScratchFA28 = DiploScratchFA1C;
                DiploScratchFA1C = DiploScratchFA18;
                DiploScratchFA18 = DiploScratchFAA8;
                DiploScratchFAA8 = i;
            } else if (v >= DiploScratchFA44) {
                DiploScratchF7C0 = DiploScratchFA20;
                DiploScratchFA20 = DiploScratchFA44;
                DiploScratchFA44 = v;
                DiploScratchFA28 = DiploScratchFA1C;
                DiploScratchFA1C = DiploScratchFA18;
                DiploScratchFA18 = i;
            } else if (v >= DiploScratchFA20) {
                DiploScratchF7C0 = DiploScratchFA20;
                DiploScratchFA20 = v;
                DiploScratchFA28 = DiploScratchFA1C;
                DiploScratchFA1C = i;
            } else if (v >= DiploScratchF7C0) {
                DiploScratchF7C0 = v;
                DiploScratchFA28 = i;
            }
        }
    }
    if (DiploScratchFA84 != -1) {
        DiploScratchF80C = -1;
    }
}

static int pending_social_model(const SocialCategory &selection, int category) {
    return static_cast<int>(selection.models_[category]);
}

static int diplo_clamp(int value, int low, int high) {
    return value < low ? low : value > high ? high : value;
}
static uint32_t diplo_bit(int faction) { return 1u << (faction & 31); }
static Map &diplo_tile(int x, int y) { return map_tiles()[MapLongitude * y + (x >> 1)]; }
static int diplo_tile_owner(const Map &tile) {
    return !(tile.bit & 2) || (tile.val2 & 15) > 7 ? -1 : tile.val2 & 15;
}

// ORIGINAL: 0x0053A980 ?diplomacy_check@@YAXHHH@Z 0x0053A980-0x0053C36E
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0053a980.cpp", "source:src/faction.h", "image:0x0053A980-0x0053C36E complete instructions and independently inspected scratch uses"], "reason": "Recover the complete faction diplomacy evaluation through genuine social-category arrays, faction/base/vehicle records and source-owned shared evaluation state. Original signed divisions, retained region0 state, unchecked malformed indices and final agenda paths remain explicit; new start-only storage bindings retain allocation uncertainty."}
void __cdecl diplomacy_check(int faction, int other, int update) {
    DiploFrictionFactionIDWith = faction;
    DiploFrictionFactionID = other;
    PopupDialogFactionID = other;
    DiploScratchFA2C = DiploScratchFA58 = DiploScratchFA50 = DiploScratchFA80
        = DiploScratchFA98 = DiploScratchFA88 = DiploScratchFAA0 = DiploScratchFA60 = 0;
    DiploScratchFA64 = at_climax(faction);
    DiploScratchFA90 = PlayersData[other].ranking > PlayersData[faction].ranking;
    const int satan = great_satan(faction, 0);
    if (!satan || PlayersData[other].current_num_bases < 2
        || ((PlayersData[faction].diplo_treaties[other] & 1)
            && (PlayersData[other].ranking < 5 || ((GameDataStateStorage.fields_.game_rules_ & 0x1000) && GameDataStateStorage.fields_.turn_current_num_ > 199))))
        DiploScratchFA3C = 0;
    else DiploScratchFA3C = 1;
    DiploScratchF7F0 = diplo_mul(FactionRankingsUnk[faction], 2)
        < FactionRankingsUnk[GameDataStateStorage.fields_.faction_rankings_[7]] && PlayersData[other].ranking != 7;
    if (PlayersData[other].flags & 0x800000) { DiploScratchFA3C = 1; DiploScratchF7F0 = 0; }
    if (PlayersData[other].flags & 0x400000) { DiploScratchFA3C = 0; DiploScratchF7F0 = 1; }
    DiploFriction = static_cast<int>(PlayersData[other].diplo_friction[faction]);
    DiploScratchFA78 = diplo_clamp(diplo_sub(PlayersData[faction].integrity_blemishes,
        PlayersData[faction].diplo_unk1[other]), 0, 99);
    int ideology = 0;
    const int category = Players[other].soc_ideology_category;
    if (category >= 0 && Players[other].soc_ideology_model
        && pending_social_model(PlayersData[faction].soc_category_pending, category)
        && pending_social_model(PlayersData[faction].soc_category_pending, category) != Players[other].soc_ideology_model) {
        ideology = 1;
        if (PlayersData[other].diplo_agenda[faction] & 1) ideology = 2;
    }
    int faction_history = diplo_add(diplo_add(PlayersData[faction].diplo_unk4[other], 1), GameDataStateStorage.fields_.turn_current_num_ / 50);
    int other_history = diplo_add(diplo_add(PlayersData[other].diplo_unk4[faction], 1), GameDataStateStorage.fields_.turn_current_num_ / 50);
    if (!(PlayersData[faction].diplo_treaties[other] & 0x10)) {
        faction_history = diplo_add(faction_history, diplo_add(diplo_add(2, PlayersData[faction].diplo_unk3[other]), GameDataStateStorage.fields_.turn_current_num_ / 25));
        other_history = diplo_add(other_history, diplo_add(diplo_add(2, PlayersData[other].diplo_unk3[faction]), GameDataStateStorage.fields_.turn_current_num_ / 25));
    }
    int fight = PlayersData[other].ai_fight;
    DiploScratchFA54 = DiploScratchF7E0 = DiploScratchF7FC = DiploScratchF7F8 = 0;
    if ((PlayersData[other].diplo_treaties[faction] & 0x20) && fight < 0) fight = diplo_sub(0, fight);
    int maximum_bases[7];
    for (int i = 0; i < 7; ++i) maximum_bases[i] = 1;
    const uint8_t alive = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
    for (int allied_faction = 1; allied_faction < 8; ++allied_faction) {
        if (alive & diplo_bit(allied_faction)) {
            if ((PlayersData[faction].diplo_treaties[allied_faction] & 1) && !(PlayersData[allied_faction].diplo_treaties[faction] & 0x2000000)) ++DiploScratchFA54;
            if (PlayersData[other].diplo_treaties[allied_faction] & 1) {
                if (PlayersData[allied_faction].ranking > 4 && PlayersData[other].ranking < 5 && allied_faction != faction && !DiploScratchF7F0) ++DiploScratchF7FC;
                ++DiploScratchF7E0;
                DiploScratchF7F8 = diplo_add(DiploScratchF7F8, FactionRankingsUnk[allied_faction]);
            }
        }
    }
    if (update) DiploScratchF7E4 = PlayersData[faction].diplo_spoke[other];
    const int faction_morale = diplo_add(diplo_add(Players[faction].rule_morale, 16), diplo_clamp(PlayersData[faction].soc_effect_pending.values_[SocialEffect::MORALE], -4, 4));
    const int other_morale = diplo_add(diplo_add(Players[other].rule_morale, 16), diplo_clamp(PlayersData[other].soc_effect_pending.values_[SocialEffect::MORALE], -4, 4));
    for (int cleared_region = 1; cleared_region < 128; ++cleared_region)
        DiplomacyRegionMask[cleared_region] = 0;
    for (int census_region = 1; census_region < 128; ++census_region)
        for (int census_faction = 1; census_faction < 8; ++census_faction)
            if (maximum_bases[census_faction - 1] < PlayersData[census_faction].region_total_bases[census_region])
                maximum_bases[census_faction - 1] = PlayersData[census_faction].region_total_bases[census_region];

    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        int byte, mask;
        bitmask(1, &byte, &mask);
        if (Bases[base].facilities_built[byte] & mask)
            DiplomacyRegionMask[diplo_tile(Bases[base].x, Bases[base].y).region] |= diplo_bit(Bases[base].faction_id_current);
        const int owner = Bases[base].faction_id_current;
        if (owner == faction && Bases[base].faction_id_former == other) ++DiploScratchFA2C;
        if ((owner != faction && owner != other) || (PlayersData[faction].diplo_treaties[other] & 1)) continue;
        const int x = Bases[base].x, y = Bases[base].y;
        Map &center = diplo_tile(x, y);
        if (diplo_tile_owner(center) < 0) {
            for (int radius = 0; radius < 21; ++radius) {
                int near_x = diplo_add(x, RadiusOffsetX[radius]);
                if (!(MapIsFlat & 1)) {
                    if (near_x < 0) near_x = diplo_add(near_x, MapLongitudeBounds);
                    else if (near_x >= MapLongitudeBounds) near_x = diplo_sub(near_x, MapLongitudeBounds);
                }
                const int near_y = diplo_add(y, RadiusOffsetY[radius]);
                if (near_y < 0 || near_y >= MapLatitudeBounds || near_x < 0 || near_x >= MapLongitudeBounds) continue;
                Map &neighbor_tile = diplo_tile(near_x, near_y);
                if (neighbor_tile.region != center.region) continue;
                if (owner == faction) {
                    if (diplo_tile_owner(neighbor_tile) == other) { ++DiploScratchFA50; break; }
                } else if (diplo_tile_owner(neighbor_tile) == faction) {
                    DiploScratchFA58 = base; // Original uses base index, including zero.
                    DiploScratchFA98 = diplo_add(DiploScratchFA98, diplo_mul(Bases[base].population_size, 8));
                    break;
                }
            }
        } else if (owner == faction) {
            const int vehicle = veh_at(x, y);
            const int limit = stack_check(vehicle, 14, -1, -1, -1);
            int threat = 0;
            for (int radius = 0; radius < 21; ++radius) {
                int near_x = diplo_add(Bases[base].x, RadiusOffsetX[radius]);
                if (!(MapIsFlat & 1)) {
                    if (near_x < 0) near_x = diplo_add(near_x, MapLongitudeBounds);
                    else if (near_x >= MapLongitudeBounds) near_x = diplo_sub(near_x, MapLongitudeBounds);
                }
                const int near_y = diplo_add(Bases[base].y, RadiusOffsetY[radius]);
                if (near_y < 0 || near_y >= MapLatitudeBounds || near_x < 0 || near_x >= MapLongitudeBounds) continue;
                const int nearby = stack_fix(veh_at(near_x, near_y));
                // Original reads the returned vehicle without a negative-id guard.
                if (Vehs[nearby].faction_id == other) {
                    const int first = stack_check(nearby, 2, 1, -1, -1);
                    const int second = stack_check(nearby, 2, 0, -1, -1);
                    threat = diplo_add(diplo_add(second, threat), first);
                }
            }
            if (limit < threat) { ++DiploScratchFAA0; ++DiploScratchFA88; }
        }
    }

    for (int region = 1; region < 128; ++region) {
        for (int ally = 1; ally < 8; ++ally)
            if (maximum_bases[ally - 1] - maximum_bases[ally - 1] / 4 <= PlayersData[ally].region_total_bases[region])
                DiplomacyRegionMask[region] |= diplo_bit(ally);
        const int faction_bases = PlayersData[faction].region_total_bases[region];
        const int other_bases = PlayersData[other].region_total_bases[region];
        const int faction_vehicles = PlayersData[faction].region_total_combat_vehs[region];
        const int other_vehicles = PlayersData[other].region_total_combat_vehs[region];
        if ((!faction_vehicles && !faction_bases) || (!other_vehicles && !other_bases)) continue;
        const int faction_weapon = diplo_add(PlayersData[faction].best_weapon_value, 1);
        const int other_weapon = diplo_add(PlayersData[other].best_weapon_value, 1);
        const int faction_force = diplo_mul(diplo_mul(PlayersData[faction].region_force_rating[region], faction_morale) / other_morale, faction_weapon) / other_weapon;
        const int other_force = diplo_mul(diplo_mul(PlayersData[other].region_force_rating[region], other_morale) / faction_morale, other_weapon) / faction_weapon;
        const uint32_t regions = DiplomacyRegionMask[region];
        const bool other_dominates = (regions & diplo_bit(other)) != 0;
        if (other_dominates) {
            int resistance = other_vehicles;
            if (faction_bases)
                resistance = diplo_add(other_force / diplo_add(diplo_sub(3, PlayersData[other].ai_fight), -((regions & diplo_bit(faction)) != 0)), other_vehicles);
            if (resistance < faction_force) {
                resistance = diplo_clamp(resistance, 1, 99999);
                DiploScratchFA98 = diplo_add(DiploScratchFA98, diplo_mul(diplo_add(faction_force, 1), 32) / resistance);
                continue;
            }
        }
        DiploScratchFA60 = 1;
        if (!faction_bases) {
            if (other_bases)
                DiploScratchFA88 = diplo_sub(DiploScratchFA88, diplo_sub(faction_force, other_vehicles) / (2 - other_dominates));
        } else {
            DiploScratchFA88 = diplo_add(DiploScratchFA88, other_force);
            if ((regions & diplo_bit(faction)) && faction_vehicles < other_force) {
                DiploScratchFA88 = diplo_add(DiploScratchFA88, other_force);
                if (diplo_mul(diplo_add(faction_force, faction_vehicles), 3) / 2 < other_force) ++DiploScratchFAA0;
            }
            if (other_bases || (PlayersData[faction].diplo_treaties[other] & 1))
                DiploScratchFA88 = diplo_sub(DiploScratchFA88, faction_force);
        }
    }
    DiploScratchFA88 = diplo_mul(diplo_add(DiploScratchFA50, 1), DiploScratchFA88);
    if (DiploScratchFA50 && !(PlayersData[faction].diplo_treaties[other] & 7)) DiploScratchFA60 = 1;
    if (DiploScratchFA64 && DiploScratchFA90) DiploScratchFA60 = 3 - (DiploScratchFA3C != 0);
    if (PlayersData[faction].current_num_bases && PlayersData[other].current_num_bases) {
        if (!(PlayersData[other].diplo_treaties[faction] & 0x20) && !(PlayersData[other].diplo_agenda[faction] & 0x200)) {
            const int difference = diplo_clamp(diplo_sub(diplo_add(diplo_sub(PlayersData[other].ai_wealth,
                PlayersData[faction].tech_commerce_bonus), PlayersData[other].tech_commerce_bonus), fight), 0, 99);
            DiploScratchFA88 = diplo_sub(DiploScratchFA88, diplo_mul(PlayersData[faction].unk_13 / PlayersData[faction].current_num_bases, difference));
            if (PlayersData[other].diplo_treaties[faction] & 0x2000) DiploScratchFA88 /= 2;
        } else {
            const int difference = diplo_clamp(diplo_add(diplo_sub(PlayersData[faction].tech_commerce_bonus, PlayersData[other].tech_commerce_bonus), fight), 0, 99);
            DiploScratchFA88 = diplo_mul(diplo_add(diplo_mul(PlayersData[other].unk_13, difference) / PlayersData[other].current_num_bases, DiploScratchFA88), 2);
        }
    }
    if (diplo_mul(PlayersData[other].best_weapon_value, 3) / 2 <= PlayersData[faction].best_weapon_value
        || diplo_mul(PlayersData[other].best_armor_value, 2) < PlayersData[faction].best_weapon_value)
        DiploScratchFA98 = diplo_mul(DiploScratchFA98, 2);
    if (diplo_mul(other_history, 2) <= faction_history) DiploScratchFA98 = diplo_mul(DiploScratchFA98, 2);
    if (diplo_mul(other_history, 4) <= faction_history && !(PlayersData[other].diplo_treaties[faction] & 0x20) && DiploScratchFA98 < 9) DiploScratchFA98 = 8;
    if (diplo_mul(faction_history, 2) < other_history) DiploScratchFA98 /= 2;
    if (diplo_mul(faction_history, 4) < other_history) DiploScratchFA98 /= 2;
    if (DiploScratchFA98 || (PlayersData[faction].diff_level <= ((PlayersData[other].diplo_treaties[faction] & 0x20) ? 2 : 4) && !(GameDataStateStorage.fields_.game_rules_ & 0x40))) DiploScratchFAA0 = 0;
    DiploScratchFA88 = diplo_mul(diplo_add(GameDataStateStorage.fields_.turn_current_num_ / 10, other_history), DiploScratchFA88) / diplo_add(faction_history, GameDataStateStorage.fields_.turn_current_num_ / 10);
    const int grace = IsMultiplayerNet ? 0 : diplo_mul(diplo_sub(5, PlayersData[faction].diff_level), 25);
    bool reconsider;
    const int faction_rank = PlayersData[faction].ranking;
    if (GameDataStateStorage.fields_.turn_current_num_ < diplo_add(grace, 50) || DiploScratchF7F0 || faction_rank < (GameDataStateStorage.fields_.diff_level_current_ < 3 ? 6 : 5)) reconsider = true;
    else if (PlayersData[other].ranking < (faction_rank < 6 ? 6 : 5)) {
        if (!DiploScratchF7FC || !(PlayersData[faction].diplo_treaties[other] & 0x8000000) || faction_rank < 6) reconsider = true;
        else { DiploScratchFA3C = 1; reconsider = false; }
    } else { DiploScratchFA3C = 1; reconsider = false; }
    if (!reconsider || DiploScratchFA3C || (((PlayersData[other].diplo_treaties[faction] & 0x20)
        || (PlayersData[other].diplo_agenda[faction] & 0x200)) && !DiploScratchFA58
        && (!DiploScratchFA98 || diplo_mul(PlayersData[other].mil_strength_1, 2) >= PlayersData[faction].mil_strength_1))) {
        DiploScratchFA98 = 0;
        DiploScratchFA60 = (DiploScratchFA3C != 0) + 1;
        const int minimum = diplo_mul(PlayersData[other].ranking, 2);
        if (DiploScratchFA88 < minimum) DiploScratchFA88 = minimum;
    }
    const int other_busters = PlayersData[other].planet_busters;
    if (other_busters && PlayersData[other].major_atrocities) DiploScratchFA98 = 0;
    if (PlayersData[faction].planet_busters && PlayersData[faction].major_atrocities) {
        if (!other_busters) DiploScratchFA98 = 32;
        DiploScratchFA88 = diplo_mul(other_busters, DiploScratchFA88) / PlayersData[faction].planet_busters;
    }
    DiploScratchFA80 = DiploScratchFA98;
    const bool pact = (PlayersData[other].diplo_treaties[faction] & 1) != 0;
    if (pact || !DiploScratchFA2C) DiploScratchFA80 = 0;
    const bool intense = (GameDataStateStorage.fields_.game_rules_ & 0x40) != 0;
    int difficulty = intense ? 5 : PlayersData[faction].diff_level;
    const int reputation = diplo_clamp(PlayersData[faction].integrity_blemishes / 2, 1, 99);
    DiploScratchFA98 /= diplo_mul(diplo_add(difficulty, 3), diplo_add(reputation, 1)) / 2;
    DiploScratchFA4C = DiploScratchF7B8 = 0;
    if (!pact || !(PlayersData[other].diplo_treaties[faction] & 0x2000000)) {
        for (int ally = 1; ally < 8; ++ally) {
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & diplo_bit(ally)) && ally != faction && ally != other
                && (PlayersData[other].diplo_treaties[ally] & 0x10)
                && (!(PlayersData[ally].diplo_treaties[faction] & 1) || !(PlayersData[ally].diplo_treaties[faction] & 0x2000000))
                && ((PlayersData[faction].diplo_treaties[ally] & 1) || ((PlayersData[faction].diplo_treaties[ally] & 2) && fight > 0))) {
                ++DiploScratchFA4C; DiploScratchF7B8 = ally;
            }
        }
    }
    DiploFriction = diplo_add(diplo_mul(DiploScratchFA4C, 3), DiploFriction);
    if (pact) DiploFriction = diplo_sub(DiploFriction, 5);
    if (PlayersData[other].diplo_treaties[faction] & 0x30) DiploFriction = diplo_add(DiploFriction, 5);
    uint32_t agenda = PlayersData[other].diplo_agenda[faction];
    if (agenda & 0x400) DiploFriction = diplo_add(DiploFriction, 3);
    if (agenda & 0x800) DiploFriction = diplo_add(DiploFriction, 3);
    if (DiploScratchFA78 > 1) DiploFriction = diplo_add(diplo_sub(DiploFriction, 1), DiploScratchFA78);
    const int other_commerce = PlayersData[other].tech_commerce_bonus;
    const int faction_commerce = PlayersData[faction].tech_commerce_bonus;
    if (faction_commerce < other_commerce) DiploFriction = diplo_add(DiploFriction, diplo_mul(diplo_sub(faction_commerce, other_commerce), 2));
    if (other_commerce < faction_commerce) DiploFriction = diplo_add(DiploFriction, diplo_sub(faction_commerce, other_commerce));
    if (ideology) { DiploFriction = diplo_add(DiploFriction, 5); if (DiploFriction < 5) DiploFriction = 5; }
    if (ideology > 1 && !DiploScratchFA98) DiploFriction = diplo_add(DiploFriction, 5);
    agenda &= 0x200;
    if (agenda) DiploScratchFA60 = 1;
    if (DiploFriction < 5) DiploScratchFA60 = 0;
    if (DiploFriction > 14) DiploScratchFA60 = 4;
    const bool demand = (PlayersData[other].diplo_treaties[faction] & 0x40) != 0;
    if (demand) DiploScratchFA60 = 1;
    if (!(GameDataStateStorage.fields_.game_rules_ & 0x800) && !PlayersData[other].se_alloc_labs) DiploScratchFA60 = 1;
    if (DiploScratchFA98 || diplo_mul(PlayersData[other].mil_strength_1, 3) < PlayersData[faction].mil_strength_1) DiploScratchFA60 = 0;
    if (diplo_mul(PlayersData[other].best_weapon_value, 2) <= PlayersData[faction].best_weapon_value) DiploScratchFA60 = 0;
    difficulty = intense ? 5 : PlayersData[faction].diff_level;
    // Original53BA0F is SAR six, NOT the seed's logical unsigned shift.
    int amount = diplo_mul(diplo_mul(diplo_mul(diplo_mul(diplo_add(difficulty, 1), diplo_add(DiploScratchFA78, 2)), diplo_add(DiploFriction, 10)), diplo_add(ideology, 1)), DiploScratchFA88) >> 6;
    if (demand) amount = diplo_mul(amount, 2);
    const int energy = PlayersData[faction].energy_reserves;
    if (diplo_mul(PlayersData[other].energy_reserves, 2) <= energy) {
        if (energy > 1999) amount = diplo_mul(amount, 3) / 2;
        if (energy > 3999) amount = diplo_mul(amount, 3) / 2;
    }
    if ((DiploScratchFA3C || (((PlayersData[other].diplo_treaties[faction] & 0x20) || agenda)
        && !DiploScratchFA58 && (!DiploScratchFA98 || PlayersData[faction].mil_strength_1 <= diplo_mul(PlayersData[other].mil_strength_1, 2))))
        && PlayersData[faction].ranking > 5) {
        const int minimum = diplo_mul(diplo_add(PlayersData[other].ranking, PlayersData[faction].ranking), energy) / 21;
        if (amount < minimum) amount = minimum;
    }
    DiploScratchFA88 = diplo_mul(diplo_add(amount, 24) / 25, 25);
    if (DiploScratchFA88 == 25) DiploScratchFA88 = 50;
    if (energy < DiploScratchFA88 && DiploScratchFA88 < diplo_mul(energy, 2)
        && (demand ? 100 : 50) <= energy && !(PlayersData[faction].diplo_treaties[other] & 0x40020))
        DiploScratchFA88 = diplo_mul(diplo_add(energy, 24) / 25, 25);
    if (!(PlayersData[other].diplo_treaties[faction] & 0x60)
        && (!DiploScratchFA3C || (PlayersData[other].diplo_treaties[faction] & 0x4000000))) {
        energy_compute(faction, 0);
        if (diplo_mul(NetIncomeGlobal, 5) < DiploScratchFA88
            && diplo_mul(diplo_mul(PlayersData[faction].energy_reserves, 2) / 3, PlayersData[other].ranking)
                / PlayersData[faction].ranking <= PlayersData[other].energy_reserves) {
            DiploScratchFA88 = diplo_mul(diplo_mul(NetIncomeGlobal, 5) / 25, 25);
            if (DiploScratchFA88 < 50) DiploScratchFA88 = 50;
        }
    }
    uint32_t treaty = PlayersData[other].diplo_treaties[faction];
    if ((((treaty & 7) && !(treaty & 0x10000)) || DiploScratchFA98) && !DiploScratchFA3C && !(treaty & 0x20)) DiploScratchFAA0 = 0;
    if (!DiploScratchFA88 && !DiploScratchFAA0 && !(PlayersData[other].diplo_agenda[faction] & 0x200)) DiploScratchFA60 = 0;
    DiploScratchFA68 = DiploScratchFA60;
    DiploScratchFA9C = 0;
    if (update) DiploScratchF7E8 = DiploScratchF7D4 = DiploScratchF7D8 = DiploScratchFA38 = DiploScratchF7C8 = 0;
    DiploScratchF808 = 0;
    for (int ally = 1; ally < 8; ++ally) {
        const uint32_t bit = diplo_bit(ally);
        uint32_t other_treaty = PlayersData[other].diplo_treaties[ally];
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & bit) || ally == faction || ally == other || !(other_treaty & 8)) continue;
        if (update && (PlayersData[faction].diplo_treaties[ally] & 0x10) && !(other_treaty & 0x10)
            && !(PlayersData[faction].diplo_treaties[ally] & 0x10000000)) DiploScratchF7D8 = ally;
        if (!(other_treaty & 1)) {
            if (!(other_treaty & 0x10)) {
                if (update && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & bit)) {
                    if (!(PlayersData[faction].diplo_treaties[ally] & 8)) {
                        if ((!DiploScratchFA38 || contiguous(faction, ally, 0, 0))
                            && ((PlayersData[other].diplo_treaties[ally] & 2) || !DiploScratchFA38)) DiploScratchFA38 = ally;
                    } else if (!(PlayersData[faction].diplo_treaties[ally] & 0x10000000)) DiploScratchF7C8 = ally;
                }
            } else {
                ++DiploScratchF808;
                const int strength = PlayersData[other].mil_strength_1;
                const int ally_strength = PlayersData[ally].mil_strength_1;
                DiploScratchFA9C = diplo_add(DiploScratchFA9C, diplo_mul(diplo_add(strength / 2, ally_strength), 2) / diplo_add(strength, 1));
                const uint32_t faction_treaty = PlayersData[faction].diplo_treaties[ally];
                if (!(FactionDiplomacyFlags[other] & 0x40)) {
                    if (!(faction_treaty & 0x10) && ((faction_treaty & 8) || (PlayersData[faction].diplo_treaties[other] & 1))
                        && (!(PlayersData[ally].diplo_treaties[faction] & 1) || !(PlayersData[ally].diplo_treaties[faction] & 0x2000000))) DiploScratchF7E8 = ally;
                    else if (!DiploScratchF7E8 && update && !(faction_treaty & 8) && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & bit)) {
                        if (strength < ally_strength || !wants_to_attack(other, ally, 0) || DiploScratchFA98) DiploScratchF7D4 = ally;
                    }
                } else if ((faction_treaty & 0x18) == 8 && strength < ally_strength) DiploScratchF7E8 = ally;
            }
        } else {
            if (PlayersData[faction].mil_strength_1 < PlayersData[ally].mil_strength_1) --DiploScratchFA9C;
            const int wicked = great_satan(faction, 0);
            if ((wicked || DiploScratchFA3C) && (PlayersData[ally].diplo_treaties[faction] & 0x10)) {
                DiploScratchFA98 = diplo_sub(DiploScratchFA98, 3);
                if (DiploScratchFA98 <= 0) { DiploScratchFA80 = 0; DiploScratchFA68 = DiploScratchFA60 = 2; }
                else if (DiploScratchFA98 >= 10000) DiploScratchFA98 = 9999;
            }
        }
    }
    const uint32_t final_agenda = PlayersData[other].diplo_agenda[faction];
    if (final_agenda & 0x800) --DiploScratchFA9C;
    if (final_agenda & 0x400) --DiploScratchFA9C;
    treaty = PlayersData[other].diplo_treaties[faction];
    const bool surrendered = (treaty & 0x20) != 0;
    if (surrendered) --DiploScratchFA9C;
    if (final_agenda & 0x200) --DiploScratchFA9C;
    DiploScratchFA9C = diplo_add(DiploScratchFA9C, diplo_sub(diplo_sub(-(DiploScratchFA78 - 1) / 3, DiploScratchFA50), fight));
    if (PlayersData[faction].tech_commerce_bonus < PlayersData[other].tech_commerce_bonus && fight < 1) ++DiploScratchFA9C;
    if (!(GameDataStateStorage.fields_.game_rules_ & 0x800) && !PlayersData[other].se_alloc_labs) --DiploScratchFA9C;
    if (PlayersData[faction].mil_strength_1 < PlayersData[other].mil_strength_1) --DiploScratchFA9C;
    if (diplo_mul(PlayersData[faction].best_armor_value, 2) < PlayersData[other].best_weapon_value) --DiploScratchFA9C;
    if (static_cast<int>((PlayersData[faction].diplo_treaties[other] >> 4) & 1) < DiploScratchFA9C
        && !DiploScratchFA3C && !(treaty & 0x41) && !DiploScratchFAA0) { DiploScratchFA60 = DiploScratchFA88 = 0; }
    if (!(treaty & 1) || !(treaty & 0x2000000)) {
        if ((DiploScratchFA50 || DiploScratchFAA0) && surrendered) DiploScratchFA68 = DiploScratchFA60 = 1;
    } else DiploScratchFAA0 = DiploScratchFA50 = DiploScratchFA68 = DiploScratchFA60 = 0;
    const int final_satan = great_satan(faction, 1);
    if (!final_satan || DiploScratchF7E0 > 1
        || FactionRankingsUnk[faction] < diplo_add(DiploScratchF7F8, diplo_mul(FactionRankingsUnk[other], 3))
        || PlayersData[other].ranking > 5 || PlayersData[faction].mil_strength_1 <= PlayersData[other].mil_strength_1) DiploScratchFA24 = 0;
    else {
        DiploScratchFA24 = 1;
        if ((PlayersData[other].diplo_treaties[faction] & 0x10) && !(PlayersData[other].diplo_agenda[faction] & 8)
            && !(PlayersData[other].diplo_treaties[faction] & 0x40000)) DiploScratchFA60 = 0;
    }
    if (update) {
        scan_prototypes(faction, other);
        const bool special = (PlayersData[other].flags_ext & 0x20) != 0;
        treaty = PlayersData[other].diplo_treaties[faction];
        if ((PlayersData[faction].diff_level < 2 && !special) || DiploScratchFA50 || (treaty & 1)
            || (PlayersData[other].diplo_agenda[faction] & 8) || ((treaty & 0x10) && DiploScratchFA2C < 2 && !special)
            || ((!DiploScratchFA64 && !DiploScratchFA60 && !ideology) && (!special || DiploScratchFA98)
                && ((!DiploScratchFA4C || !(treaty & 0x800000)) || DiploFriction < 12))
            || (!(treaty & 0x20) && (!special || !DiploScratchFA60) && DiploScratchFA78 < 6
                && ((!DiploScratchFA60 && ideology < 2) || (PlayersData[faction].mil_strength_1 <= PlayersData[other].mil_strength_1 && !special)))) DiploScratchFA70 = 0;
        else DiploScratchFA70 = 1;
    }
    if ((PlayersData[other].flags & 0x400000) && !(PlayersData[other].diplo_treaties[faction] & 0x20)) DiploScratchFA60 = 0;
    if (!(PlayersData[other].diplo_agenda[faction] & 8)) {
        if (DiploScratchFA70) { DiploScratchFA60 = 0; if (DiploFriction > 6) DiploFriction = 6; }
    } else DiploScratchFA60 = 1;
    treaty = PlayersData[faction].diplo_treaties[other];
    if ((treaty & 1) && (!(treaty & 0x20) || DiploScratchF808 > 1)
        && (!(treaty & 0x200) || DiploScratchF808)
        && PlayersData[other].ranking < PlayersData[faction].ranking
        && ((GameDataStateStorage.fields_.game_rules_ & 0x1000) || DiploScratchF808)) DiploScratchFA60 = 0;
    if (PlayersData[other].diplo_agenda[faction] & 0x2000) {
        DiploScratchFA70 = 0;
        DiploScratchFA60 = (PlayersData[other].diplo_treaties[faction] & 0x10) >> 4;
    }
    if (!DiploScratchFA68 && (DiploScratchFA98 || (treaty & 1)) && DiploFriction > 14) DiploFriction = 14;
    if ((DiploScratchFA60 || (treaty & 0x10) || (PlayersData[other].diplo_treaties[faction] & 0x20)) && DiploFriction < 6) DiploFriction = 6;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0053E470 ?diplomacy_ends@@YAXHH@Z 0x0053E470-0x0053E4BC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned native53E470..53E4BC: four state clears, DiploPop.stop receiver733990, synch_diplo arguments, NetDaemon.close_channel receiver93CD90, then Y/X reset.", "src/diplopop.cpp DiplomacyPopup; src/netdaemon.cpp NetDaemonNet; native53DCE0 contact-result and bounded focus uses. Initial states zero, X/Y -1.", "Root-only console-real-diplomacy-end-20261001 scope decision; preserved src/recovered/units/0053e470.cpp and unresolved settlement contact discovery."], "reason": "Recover complete diplomacy teardown in its genuine faction owner with independent owned state and coordinate storage, real popup lifecycle and network APIs. Retain the exact seed obligation and all original spans; this binding grants no match or acceptance claim."}
void __cdecl diplomacy_ends(int faction_id, int faction_id_with) {
    DiplomacyActive = 0;
    DiplomacyAuxiliaryState = 0;
    DiplomacyEndState = 0;
    DiplomacyContactResult = 0;
    DiplomacyPopup.stop();
    synch_diplo(faction_id, faction_id_with);
    NetDaemonNet.close_channel();
    DiploFocusY = -1;
    DiploFocusX = -1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005399A0 ?diplomacy_caption@@YAXHH@Z 0x005399A0-0x00539B6B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5399A0..539B6B; existing inactive diplomacy-presentation task evidence retained; counterpart Player noun fields and PlayerData20CC treaty strides independently decoded; literal68D594 is the bounded separator. Host source-body controls cover mood thresholds, bidirectional pact/surrender and post-parse visibility."], "reason": "Recover complete diplomacy caption in its faction owner with genuine treaty/noun/mood/scratch data and the actual visible-popup draw operation. Preserve native friction clamp, label predicate, parse defaults and redraw ordering."}
void __cdecl diplomacy_caption(int faction_id, int faction_id_with) {
    int mood = 0;
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with] &
         DTREATY_VENDETTA) != 0 &&
        DiploFriction < 15) {
        DiploFriction = 15;
        mood = 5;
    } else if (DiploFriction <= 0) {
        mood = 0;
    } else if (DiploFriction <= 2) {
        mood = 1;
    } else if (DiploFriction <= 4) {
        mood = 2;
    } else if (DiploFriction <= 8) {
        mood = 3;
    } else if (DiploFriction <= 12) {
        mood = 4;
    } else if (DiploFriction <= 15) {
        mood = 5;
    } else if (DiploFriction <= 17) {
        mood = 6;
    } else {
        mood = (DiploFriction > 19 ? 1 : 0) + 7;
    }

    char caption[256];
    caption[0] = '\0';
    GenderDefault = Players[faction_id_with].noun_gender;
    PluralityDefault = Players[faction_id_with].is_noun_plural;
    strcat(caption, Players[faction_id_with].noun_faction);
    strcat(caption, " - ");

    const bool use_special_label =
        (mood == 0 && DiploScratchFA98 != 0 &&
         (PlayersData[faction_id].diplo_treaties[faction_id_with] &
          DTREATY_PACT) == 0) ||
        ((PlayersData[faction_id_with].diplo_treaties[faction_id] &
          DTREATY_PACT) != 0 &&
         (PlayersData[faction_id_with].diplo_treaties[faction_id] &
          DTREATY_HAVE_SURRENDERED) != 0);
    if (use_special_label) {
        strcat(caption, label_get(0x1E8));
    } else {
        char *mood_text = Mood[mood];
        say_fac_special(caption, StringTable.get(mood_text), faction_id_with);
    }

    parse_says(7, caption, -1, -1);
    if (DiplomacyPopup.is_visible() != 0) {
        DiplomacyPopup.draw_info();
        DiplomacyPopup.soft_update(&DiplomacyPopup.rect1_);
    }
}

static void contact_dialog_arguments(int other_faction_id) {
    GenderDefault = Players[other_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[other_faction_id].title_leader, -1, -1);
    GenderDefault = Players[other_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[other_faction_id].name_leader, -1, -1);
    GenderDefault = Players[other_faction_id].noun_gender;
    PluralityDefault = Players[other_faction_id].is_noun_plural;
    parse_says(2, Players[other_faction_id].noun_faction, -1, -1);
    parse_num(0, random(0, 10) + 1);
    parse_says(3, get_him_her(other_faction_id, 0), -1, -1);
}
// This preserves its single implementation while giving NetDaemon a genuine
// source API for the original 1501 prompt preparation.
void __cdecl prepare_contact_dialog_arguments(int faction_id) {
    contact_dialog_arguments(faction_id);
}


// ORIGINAL: 0x0053DCE0 ?wants_to_speak@@YAHHH@Z 0x0053DCE0-0x0053E46C;0x0065E622-0x0065E75A FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0053dce0.cpp", "image:terranx_original.exe:0x0053DCE0"], "reason": "Recover the genuine faction contact/introduction owner using Popup lifetime, typed Player/PlayerData, real diplomacy state and portrait storage; native guards and callee order correct retained seed contradictions."}
// TRIED: this is a 27-stage SEH unwind (vtable pokes for partially- constructed FlatButton/Scroll/Dialog/Sprite/Spot/Heap locals around a `Popup` object) that could not be fully re-derived in budget; landed a single `Popup popup;` local plus the has_tech-gated / consent / diplomacy-request control flow (diplomacy_check, diplomacy_caption, parse_says, random, get_him_her, X_pops, Popup::start/exec, DiploPop::start, introduce), omitting the other ~10 locals' manual close()/dtor bookkeeping. Compiles; diverges at #17 (prologue/frame size).
int __cdecl wants_to_speak(int faction_id, int other_faction_id) {
    Popup popup;
    PopupDialogFactionID = other_faction_id;
    if ((Players[faction_id].rule_flags ^ Players[other_faction_id].rule_flags)
        & RFLAG_ALIEN) {
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            if (!has_tech(0x4E, other_faction_id) && !has_tech(4, faction_id)) {
                DiplomacyContactResult = 0;
                return 0;
            }
        } else if (!has_tech(0x4E, faction_id) && !has_tech(4, other_faction_id)) {
            DiplomacyContactResult = 0;
            return 0;
        }
    }
    if (DiplomacyEndState) {
        return DiplomacyContactResult;
    }

    bool urgent = false;
    if (DiplomacyContactResult) {
        DiplomacyEndState = 1;
        if (DiplomacyContactResult >= 8) {
            diplomacy_check(faction_id, other_faction_id, 0);
            diplomacy_caption(faction_id, other_faction_id);
            contact_dialog_arguments(other_faction_id);
            x_pops_minimal("URGENTCOMM", FactionDiplomacyPortraits[other_faction_id], pop_wait);
        }
    } else {
        ConsoleInfoWin.set_state(2, 1);
        if (!DiplomacyEndState) {
            for (;;) {
                if (PlayersData[other_faction_id].planet_busters &&
                    !(PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x4000)) {
                    urgent = true;
                }
                diplomacy_check(faction_id, other_faction_id, 0);
                if (DiploFocusY >= 0 && DiploFocusY < MapLatitudeBounds &&
                    DiploFocusX >= 0 && DiploFocusX < MapLongitudeBounds) {
                    if (!ConsoleGlobal.focus(DiploFocusX, DiploFocusY, faction_id)) {
                        draw_tiles(DiploFocusX, DiploFocusY, 2);
                    }
                }
                diplomacy_caption(faction_id, other_faction_id);
                contact_dialog_arguments(other_faction_id);
                if (urgent) {
                    x_pops_minimal("URGENTCOMM", FactionDiplomacyPortraits[other_faction_id], pop_wait);
                    break;
                }
                popup.start(PopupStartCaption, "COMM", -1, nullptr, 0, nullptr);
                Sprite *portrait = FactionDiplomacyPortraits[other_faction_id];
                popup.set_caller_picture(popup.picture_enabled() ? nullptr : portrait);
                popup.dialogs().sprite(portrait, nullptr, 0);
                const int selected = popup.exec(0, pop_wait);
                if (selected > 1) {
                    report_intel(other_faction_id);
                    if (!DiplomacyEndState) {
                        continue;
                    }
                } else {
                    DiplomacyEndState = 1;
                    DiplomacyContactResult = selected == 0;
                }
                break;
            }
        }
    }
    if (DiplomacyContactResult) {
        DiplomacyPopup.start(other_faction_id);
        introduce(faction_id, other_faction_id);
    }
    return DiplomacyContactResult;
}

// ORIGINAL: 0x0053D360 ?introduce@@YAXHH@Z 0x0053D360-0x0053DCB9;0x0065E53A-0x0065E622 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0053d360.cpp", "image:terranx_original.exe:0x0053D360"], "reason": "Recover the genuine faction contact/introduction owner using Popup lifetime, typed Player/PlayerData, real diplomacy state and portrait storage; native guards and callee order correct retained seed contradictions."}
// TRIED: `Popup popup;` RAII reproduces the whole BasePop/Scroll/ FlatButton/Sprite/Dialogs/Spot/Heap teardown cascade structurally (compiles, calls in the right order), but the prologue already diverges at instr #2 (mov vs push) - the frame/SEH setup this compiler emits for that one local doesn't match byte-for-byte. Field offsets into Players (0x00946A50, stride 0x59C) were cross-checked against src/faction.h's real Player layout and line up exactly.
void __cdecl introduce(int faction_id, int other_faction_id) {
    Popup popup;
    PopupDialogFactionID = other_faction_id;
    DiplomacyActive = 1;
    diplomacy_caption(faction_id, other_faction_id);

    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[faction_id].title_leader, -1, -1);
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[faction_id].name_leader, -1, -1);
    GenderDefault = Players[other_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(2, Players[other_faction_id].title_leader, -1, -1);
    GenderDefault = Players[other_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(3, Players[other_faction_id].name_leader, -1, -1);
    GenderDefault = Players[other_faction_id].noun_gender;
    PluralityDefault = Players[other_faction_id].is_noun_plural;
    parse_says(4, Players[other_faction_id].noun_faction, -1, -1);
    parse_says(8, Players[other_faction_id].adj_name_faction, -1, -1);
    parse_says(9, Players[faction_id].adj_leader, -1, -1);

    StringTemp[0] = '\0';
    strcat(StringTemp, "INTRO");
    const int treaty = PlayersData[faction_id].diplo_treaties[other_faction_id];
    unsigned int choice;
    if ((treaty & 0x08000000) &&
        !((IsMultiplayerNet || IsMultiplayerPBEM) &&
          (FactionDiplomacyFlags[other_faction_id] & 2))) {
        if (!(treaty & 0x20)) {
            choice = random(0, 4);
            const bool favorable =
                (PlayersData[faction_id].diplo_treaties[other_faction_id] & 1) ||
                !DiploScratchFA60;
            switch (choice) {
            case 0:
                parse_says(5, get_noun_phrase(faction_id, favorable ? 9 : 10), -1, -1);
                break;
            case 1:
                parse_says(9, favorable ? Players[faction_id].adj_leader :
                           Players[faction_id].adj_insult_leader, -1, -1);
                break;
            case 2:
                parse_says(5, favorable ? Players[faction_id].adj_faction :
                           Players[faction_id].adj_insult_faction, -1, -1);
                break;
            case 3:
                parse_says(9, favorable ? Players[faction_id].adj_leader :
                           Players[faction_id].adj_insult_leader, -1, -1);
                parse_says(5, favorable ? Players[faction_id].adj_faction :
                           Players[faction_id].adj_insult_faction, -1, -1);
                break;
            }
        } else {
            strcat(StringTemp, "MAD");
            choice = random(0, 5);
            switch (choice) {
            case 0:
                parse_says(5, get_phrase(faction_id, 3), -1, -1);
                // Original case 0 shares case 2's noun argument.
            case 2:
                parse_says(6, get_noun_phrase(faction_id, 8), -1, -1);
                break;
            case 1:
                parse_says(5, Players[faction_id].adj_insult_faction, -1, -1);
                parse_says(6, get_phrase(faction_id, 3), -1, -1);
                break;
            case 4:
                parse_says(5, Players[faction_id].adj_insult_leader, -1, -1);
                GenderDefault = Players[faction_id].is_leader_female;
                PluralityDefault = 0;
                parse_says(6, Players[faction_id].insult_leader, -1, -1);
                // Original case 4 shares case 3's phrase argument.
            case 3:
                parse_says(8, get_phrase(faction_id, 6), -1, -1);
                break;
            }
        }
    } else {
        strcat(StringTemp, "NEW");
        choice = random(0, 6);
        parse_says(6, get_phrase(other_faction_id, 1), -1, -1);
        if (choice == 2) {
            PluralityDefault = Players[faction_id].is_noun_plural;
            GenderDefault = Players[faction_id].noun_gender;
            parse_says(5, Players[faction_id].noun_faction, -1, -1);
        } else if (choice == 3) {
            char description[32];
            description[0] = '\0';
            int rank = 7 - PlayersData[faction_id].ranking;
            if (rank < 0) rank = 0;
            else if (rank > 6) rank = 6;
            char *word = StringTable.get(Might[rank].adj);
            if (!Language) {
                const char first = word[0];
                const bool vowel = first == 'A' || first == 'E' || first == 'I' ||
                                   first == 'O' || first == 'U';
                strcat(description, label_get(vowel ? 0x454 / 4 : 0x450 / 4));
                strcat(description, " ");
            }
            strcat(description, word);
            parse_says(5, description, -1, -1);
        }
    }
    char number[32];
    _itoa(choice, number, 10);
    strcat(StringTemp, number);
    X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
    if (PlayersData[other_faction_id].planet_busters) {
        X_pops("PLANETBUSTER0", 0x100000,
                     FactionDiplomacyPortraits[other_faction_id], pop_wait);
    }
    if (PlayersData[faction_id].planet_busters &&
        !(PlayersData[faction_id].diplo_treaties[other_faction_id] & 0x4000)) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "PLANETBUSTER");
        _itoa((PlayersData[other_faction_id].planet_busters != 0) + 1, number, 10);
        strcat(StringTemp, number);
        X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingSETTREATY, faction_id, other_faction_id, 0x4000);
            message_data(0x2443, 0, faction_id, other_faction_id, 0x4000, 1);
            NetDaemonNet.await_diplo(0x443);
        } else {
            set_treaty(faction_id, other_faction_id, 0x4000, 1);
        }
    }
}


// Shared map-transfer outcome. Native543A3B clears and543AF0 sets;
// proposal producers545E5B/545E72/54F62D and reader557AEE remain seed
// obligations, not proof of completed diplomacy workflow coverage.
// RECOVERY_OBJECT: v2 0x0093FAD8 0x0093FADC ?DiplomacyMapTransferChanged@@3HA 0x00543A20
int DiplomacyMapTransferChanged;
// Shared modal proposal result/mode, not a private default for base gifting.
// proposal_menu54DCF0 clears/stores it; native gift path reads mode3.
// RECOVERY_OBJECT: v2 0x0093FAB0 0x0093FAB4 ?DiplomacyProposalMode@@3HA 0x0054DCF0
int DiplomacyProposalMode;
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00543A20 ?trade_maps@@YAXHH@Z 0x00543A20-0x00543BC0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00543a20.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl trade_maps(int faction_id, int other_faction_id) {
    DiplomacyMapTransferChanged = 0;
    PlayersData[faction_id].traded_maps |=
        (1 << other_faction_id) | PlayersData[other_faction_id].traded_maps;
    PlayersData[other_faction_id].traded_maps |=
        (1 << faction_id) | PlayersData[faction_id].traded_maps;

    const uint8_t mask_a = static_cast<uint8_t>(1 << faction_id);
    const uint8_t mask_b = static_cast<uint8_t>(1 << other_faction_id);
    Map *tile = MapTiles;
    for (int tile_id = 0; tile_id < MapArea; ++tile, ++tile_id) {
        if ((tile->visibility & mask_a) != 0) {
            tile->visibility |= mask_b;
            if (faction_id != 0 && other_faction_id != 0) {
                tile->bit_visible[other_faction_id - 1] |=
                    tile->bit_visible[faction_id - 1] & tile->bit;
            }
        } else if ((tile->visibility & mask_b) != 0) {
            DiplomacyMapTransferChanged = 1;
            tile->visibility |= mask_a;
            if (faction_id != 0 && other_faction_id != 0) {
                tile->bit_visible[faction_id - 1] |=
                    tile->bit_visible[other_faction_id - 1] & tile->bit;
            }
        }
    }

    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        if ((Bases[base_id].visibility & mask_a) != 0 ||
            Bases[base_id].faction_id_current == faction_id) {
            spot_base(base_id, other_faction_id);
        } else if ((Bases[base_id].visibility & mask_b) != 0 ||
                   Bases[base_id].faction_id_current == other_faction_id) {
            spot_base(base_id, faction_id);
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_ ||
        other_faction_id == ConsoleGlobal.faction_id_) {
        WorldClimateWorldWin.field_239BC_ |= 4;
        WorldClimateWorldWin.GraphicWin::redraw();
        draw_map(1);
    }
}

// Native54C560 reads each independent DWORD after text callbacks. Initial
// image words are zero; semantics and complete producer coverage stay open.
// Start-only bindings do not assert complete original allocation extents.
// RECOVERY_OBJECT: v1 0x0093F7DC ?DiploScratchF7DC@@3HA
int DiploScratchF7DC;
// RECOVERY_OBJECT: v1 0x0093FA94 ?DiploScratchFA94@@3HA
int DiploScratchFA94;
// RECOVERY_OBJECT: v1 0x0093FAD0 ?DiploScratchFAD0@@3HA
int DiploScratchFAD0;
// RECOVERY_OBJECT: v1 0x0093FAD4 ?DiploScratchFAD4@@3HA
int DiploScratchFAD4;
// Original54DD57 resets this selector;54E3F9/54E6FD store modal faction
// choices, subsequently used to select Player records and by buy_tech mode13.
// Initial image word is zero; other producer coverage remains unfinished.
// RECOVERY_OBJECT: v1 0x0093FA5C ?DiploScratchFA5C@@3HA
int DiploScratchFA5C;

// Original54D410 selects these independent DWORD cells as offered/requested
// base IDs. Initial words are zero; allocation extent and other producers
// remain unverified, so these are start-only bindings, not a bounded layout claim.
// RECOVERY_OBJECT: v1 0x0093FA30 ?DiploScratchFA30@@3HA
int DiploScratchFA30;
// RECOVERY_OBJECT: v1 0x0093FA7C ?DiploScratchFA7C@@3HA
int DiploScratchFA7C;

namespace {
// Every source line is consumed even when its menu item is suppressed.
void read_diplomacy_menu_line() {
    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
}
}

// ORIGINAL: 0x0054BEC0 ?call_off_vendetta@@YAXHH@Z 0x0054BEC0-0x0054C552;0x0065F8FC-0x0065FA01
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned original54BEC0-54C552 and cleanup65F8FC-65FA01", "historical seed:src/unrecovered/0054bec0.cpp", "Native54BF8E other-to-candidate treaty;54BFDB open failure exits enumeration;54C1F3-54C221 atrocity refusal;54C41A-54C452 other-to-chosen consequences."], "reason": "Recover directed candidate selection, parser defaults, refusal dialogs and treaty consequences with one genuine lexical Popup and its embedded Dialogs/SpriteBox. Exact native call operands resolve the seed's nullary helpers. Literal keys are actual image strings, not executable-address pointers."}
// symbol    ?call_off_vendetta@@YAXHH@Z
// TRIED: raw-disassembly-derived body (Ghidra's own decompile is SEH-state-machine noise here too) with Popup+SpriteBox locals for the destructor tail; argument values were cross-checked against the asm for the faction-search loop and the two parse_says/set_treaty blocks, but the later fn_0055bc80/fn_005bf7d0/fn_00592ee0/fn_006262f0/fn_005315c0/fn_0055d760 calls have no established signature evidence and are called nullary. Compiles and matches the branch structure; diverges from the frameless build around #2.
void __cdecl call_off_vendetta(int faction_id, int faction_id_with) {
    Popup popup;
    PopupDialogFactionID = faction_id_with;
    popup.start(PopupStartCaption, "CALLOFF", -1, nullptr, 0x100042, 0);
    popup.set_caller_picture(popup.picture_enabled()
        ? nullptr : FactionDiplomacyPortraits[faction_id_with]);
    popup.dialogs().sprite(FactionDiplomacyPortraits[faction_id_with], nullptr,
                           faction_id_with);

    int available = 0;
    for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if (!is_alive(candidate) || candidate == faction_id ||
            candidate == faction_id_with ||
            (PlayersData[faction_id_with].diplo_treaties[candidate] & 0x10) == 0 ||
            (PlayersData[faction_id].diplo_treaties[candidate] & 8) == 0 ||
            (PlayersData[faction_id].diplo_treaties[candidate] & 7) == 0) continue;

        if (X_text_open(ScriptPopupFile, "ATTACKSTRING") != 0) break;
        text_get();
        GenderDefault = Players[candidate].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[candidate].title_leader, -1, -1);
        GenderDefault = Players[candidate].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[candidate].name_leader, -1, -1);
        GenderDefault = Players[candidate].noun_gender;
        PluralityDefault = Players[candidate].is_noun_plural;
        parse_says(2, Players[candidate].noun_faction, -1, -1);
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        ++available;
        popup.dialogs().item(StringTemp, candidate);
    }
    if (available == 0) return;
    const int chosen = popup.exec(0, pop_wait);
    if (chosen < 0) return;

    GenderDefault = Players[chosen].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[chosen].title_leader, -1, -1);
    GenderDefault = Players[chosen].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[chosen].name_leader, -1, -1);
    parse_says(2, Players[chosen].adj_insult_faction, -1, -1);
    GenderDefault = Players[chosen].noun_gender;
    PluralityDefault = Players[chosen].is_noun_plural;
    parse_says(3, Players[chosen].noun_faction, -1, -1);

    if ((PlayersData[faction_id_with].diplo_treaties[chosen] & 0x20) != 0 ||
        (PlayersData[chosen].major_atrocities != 0 &&
         PlayersData[faction_id_with].major_atrocities == 0)) {
        X_pops("TOTHEDEATH", 0x100000,
                     FactionDiplomacyPortraits[faction_id_with], nullptr);
        return;
    }
    if ((PlayersData[faction_id_with].diplo_treaties[chosen] & 0x40) != 0 &&
        (PlayersData[faction_id_with].diplo_treaties[faction_id] & 0x80) == 0) {
        X_pops("NOPEHIRED", 0x100000,
                     FactionDiplomacyPortraits[faction_id_with], nullptr);
        return;
    }

    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingSETTREATY, faction_id_with, chosen, 0x4000000);
        message_data(0x2443, 0, faction_id_with, chosen, 0x4000000, 1);
    } else {
        set_treaty(faction_id_with, chosen, 0x4000000, 1);
    }
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingSETTREATY, faction_id_with, chosen, 0x40);
        message_data(0x2443, 0, faction_id_with, chosen, 0x40, 0);
    } else {
        set_treaty(faction_id_with, chosen, 0x40, 0);
    }
    if (wants_to_attack(chosen, faction_id_with, 0) != 0) {
        X_pops("THEIRFAULT", 0x100000,
                     FactionDiplomacyPortraits[faction_id_with], nullptr);
        return;
    }
    X_pops("CALLEDOFF", 0x100000,
                 FactionDiplomacyPortraits[faction_id_with], nullptr);
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYON, faction_id_with, chosen, 2);
        message_data(0x2441, 0, faction_id_with, chosen, 2, 0);
        NetDaemonNet.await_diplo(0x441);
    } else {
        enemies_treaty(faction_id_with, chosen, 0);
    }
}

// ORIGINAL: 0x0054C560 ?diplomacy_menu@@YAHHH@Z 0x0054C560-0x0054CB46;0x0065FA01-0x0065FAE9
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned original54C560-54CB46 and cleanup65FA01-65FAE9", "historical seed:src/unrecovered/0054c560.cpp", "Native54C60D five-argument start;54C612-54C631 caller-picture selection;54C6AF-54C922 ordered reads/gates/exec; original DIPLO/DIPLOMENU literals."], "reason": "Recover all nine original menu-line reads and conditional entries with genuine lexical Popup lifetime and embedded Dialogs/SpriteBox ownership. Native portrait-selection store omitted by seed is restored. Shared scratch cells remain provisional independent globals; original constructor/cleanup supports lifecycle, not a byte or game acceptance claim."}
// symbol    ?diplomacy_menu@@YAHHH@Z
// TRIED: MISMATCH #16 'push' vs 'xor' (framed build, sim 0.78). SEH dialog builder: `Popup popup;` real automatic object using confirmed named members (scroll_, scroll_.flat_button_left_/right_, spot_, sprite_, dialogs_ cast to Dialogs*) plus popup-base-relative reinterpret_casts for the remaining teardown targets whose exact field identity in BasePop I could not confirm (two StringStruct- like locals torn down via the ORIGINAL()/original_method(0x4066C0) thiscall shim, matching Dialog::destroy()'s close_with_tables precedent; two more FlatButtons; one Heap). Offsets computed as (0x5388 - ebp-relative magnitude) from the disassembly's own `lea ecx,[ebp-N]` targets, not guessed. STRUCTURE (BasePop, confirmed against 0047C970's identical layout): flat_button1_/flat_button2_, sprite_, spot_, and dialogs_[0xC94] are real named members; Popup adds `Scroll scroll_` at offset 0x3230, and Scroll's own flat_button_left_/ right_ sit at 0xAAC/0x15F8 within it - all match the trailer thunks (`add ecx,0xaac`/`0x15f8`) in both this function and 0047C970 byte-for-byte.
int __cdecl diplomacy_menu(int faction_id, int faction_id_with) {
    Popup popup;
    PopupDialogFactionID = faction_id_with;
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(0, Players[faction_id].name_leader, -1, -1);
    parse_says(1, get_pact_hood(faction_id, faction_id_with), -1, -1);
    diplomacy_caption(faction_id, faction_id_with);

    popup.start(PopupStartCaption, "DIPLO", -1, nullptr, 0x100000);
    popup.set_caller_picture(popup.picture_enabled()
        ? nullptr : FactionDiplomacyPortraits[faction_id_with]);
    popup.dialogs().sprite(FactionDiplomacyPortraits[faction_id_with], nullptr, 0);
    if (X_text_open(ScriptPopupFile, "DIPLOMENU") != 0) return 0;

    read_diplomacy_menu_line();
    popup.dialogs().item(StringTemp, 0);
    read_diplomacy_menu_line();
    if (DiploScratchFAD0 != 0) popup.dialogs().item(StringTemp, 1);
    read_diplomacy_menu_line();
    if (DiploScratchFAD4 == 0) popup.dialogs().item(StringTemp, 2);
    read_diplomacy_menu_line();
    if (DiploScratchFAD4 != 0) popup.dialogs().item(StringTemp, 2);
    read_diplomacy_menu_line();
    if (IsMultiplayerNet == 0 && DiploScratchFA94 > 0)
        popup.dialogs().item(StringTemp, 5);
    read_diplomacy_menu_line();
    if (IsMultiplayerNet == 0 && DiploScratchF7DC > 0)
        popup.dialogs().item(StringTemp, 6);
    read_diplomacy_menu_line();
    if (can_call_council(faction_id, 0) != 0 &&
        (Players[faction_id].rule_flags & 0x80) == 0 &&
        (Players[faction_id_with].rule_flags & 0x80) == 0)
        popup.dialogs().item(StringTemp, 7);
    read_diplomacy_menu_line();
    if ((PlayersData[faction_id_with].diplo_treaties[faction_id] & 1) != 0)
        popup.dialogs().item(StringTemp, 3);
    read_diplomacy_menu_line();
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with] & 7) != 0)
        popup.dialogs().item(StringTemp, 4);
    text_close();
    return popup.exec(0, pop_wait);
}

// stand-in: the noun-form setup plus parse_says pair recurs across many TUs
// and was likely a shared inline; as a TU helper its /Ob2 cost spends the
// budget the image spends before pact_ends' Popup teardown.
static void say_faction_noun(int slot, int faction) {
    parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
    parse_says(slot, Players[faction].noun_faction, -1, -1);
}

/*
Purpose: Determine the value of the specified base between the requester and the respondent faction.
         This valuation could be triggered either from a gift, a threat or a base swap.
// ORIGINAL: 0x0054CB50 ?value_of_base@@YAHHHHHH@Z 0x0054CB50-0x0054CF39 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x54CD1A-0x54CD30 signed population compare and MOVSX; Base.population_size is signed byte at +6", "pinned:0x54CD33-0x54CD4F wrapped numerator, signed IDIV and signed valuation clamp at100", "pinned:0x54CD8F-0x54CD96,0x54CDE4-0x54CDEB,0x54CDF4-0x54CDFB use CDQ/SUB/SAR for signed half toward zero", "historical TRIED unsigned spelling remains experiment history, not current behavior policy"], "reason": "Restore original signed population, valuation clamps and truncating halves while preserving modulo-32-bit accumulation. This corrects a prior unsigned behavior change, not an original game bug. Exceptional signed division faults, runtime reachability and complete matching remain unverified."}
// size      1001 bytes
// prototype int (__cdecl ?value_of_base@@YAHHHHHH@Z)(int baseID, int factionIDReq, int factionIDRes, int baseIDCounter, int tgl)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050BA00 0x0059E980 0x005AC060 0x005B8E10 0x005BFE90 0x00645660
// LEVER: plain int arithmetic (no wrap helpers), x/y re-read from Bases,
//        region_at inside each owner branch, uint8 region counts read in
//        place, and an inlined veh_top copy. Remaining gap: the image walks
//        the secret-project loop with a pointer and keeps base_id unloaded.
Return Value: Value of base or -1 for invalid requests
Status: Complete - testing
*/
// The image inlines veh_top here (no call to 0x00579920).
static inline int value_of_base_veh_top(int veh_id) {
    int top_veh_id = veh_id;
    if (veh_id < 0) {
        return -1;
    }
    for (int16_t i = Vehs[top_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
        top_veh_id = i;
    }
    return top_veh_id;
}

// The image also inlines veh_next_stack_id (0x0050B8F0) for the walk.
static inline int value_of_base_veh_next(int veh_id) {
    if (veh_id < 0) {
        return veh_id;
    }
    return Vehs[veh_id].next_veh_id_stack;
}

int __cdecl value_of_base(int base_id, int faction_id_req, int faction_id_res,
                          int overmatch_deg, BOOL tgl) {
    if (base_id < 0) {
        return -1;
    }
    int dist_factor = vulnerable(faction_id_req, Bases[base_id].x, Bases[base_id].y);
    if (dist_factor <= 0) {
        return -1;
    }
    int region_base = region_at(Bases[base_id].x, Bases[base_id].y);
    for (int i = 1; i < 169; i++) { // RadiusRange[6], a literal in the image
        int x_radius = wrap_map_x(Bases[base_id].x + RadiusOffsetX[i]);
        int y_radius = Bases[base_id].y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            int base_id_found = base_at(x_radius, y_radius);
            if (base_id_found >= 0) {
                int faction_id_base = Bases[base_id_found].faction_id_current;
                if (faction_id_base != faction_id_req) {
                    if (region_base == region_at(x_radius, y_radius)) {
                        if (faction_id_base != faction_id_res) {
                            break;
                        }
                        dist_factor++;
                    }
                } else {
                    if (region_base == region_at(x_radius, y_radius)) {
                        dist_factor--;
                    }
                    break;
                }
            }
        }
    }
    int veh_id = stack_fix(veh_at(Bases[base_id].x, Bases[base_id].y));
    int most_reserves = (PlayersData[faction_id_req].energy_reserves
                         > PlayersData[faction_id_res].energy_reserves)
        ? PlayersData[faction_id_req].energy_reserves
        : PlayersData[faction_id_res].energy_reserves;
    int base_pop_factor;
    if (Bases[base_id].population_size < 3) {
        base_pop_factor = 3;
    } else {
        base_pop_factor = Bases[base_id].population_size;
    }
    int value = (most_reserves + 1000) / (dist_factor + 4) * base_pop_factor;
    if (value < 100) {
        value = 100;
    }
    if (veh_who(Bases[base_id].x, Bases[base_id].y) < 0) {
        value /= 2;
    }
    if (!PlayersData[faction_id_res].region_total_bases[region_base]) {
        value *= 2;
    }
    if (PlayersData[faction_id_req].region_total_bases[region_base] == 1) {
        if (!PlayersData[faction_id_res].region_total_bases[region_base]) {
            value *= 2;
        }
        if (PlayersData[faction_id_res].region_total_bases[region_base] == 1) {
            value *= 2;
        }
    }
    if (tgl) {
        value *= 4;
    }
    if (PlayersData[faction_id_res].region_total_bases[region_base]
        && PlayersData[faction_id_req].region_total_bases[region_base]) {
        if (PlayersData[faction_id_res].region_total_bases[region_base]
            >= PlayersData[faction_id_req].region_total_bases[region_base] * 5) {
            value /= 2;
        }
        if (overmatch_deg) {
            value /= 2;
        }
    }
    int facil_count = 0;
    for (int fac = 1; fac < FacilitySPStart; fac++) {
        if (has_fac(fac, base_id, 0)) {
            value += Facility[fac].cost * ++facil_count;
        }
    }
    for (int proj = 0; proj < MaxSecretProjectNum; proj++) {
        if (GameDataStateStorage.fields_.secret_project_.project_base[proj] == base_id) {
            value += Facility[FacilitySPStart + proj].cost * 25;
        }
    }
    for (int veh = value_of_base_veh_top(veh_id); veh >= 0; veh = value_of_base_veh_next(veh)) {
        if (Vehs[veh].faction_id == faction_id_req) {
            value += VehPrototypes[Vehs[veh].proto_id].cost * 2;
        }
    }
    if (!strcmp(Players[faction_id_req].filename, "BELIEVE")) {
        value *= 2;
    }
    if (is_objective(base_id)) {
        value *= 4;
    }
    return value;
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0054CF40 ?give_a_base@@YAXHH@Z 0x0054CF40-0x0054D410
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0054cf40.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl give_a_base(int base_id, int new_owner) {
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    int old_owner = Bases[base_id].faction_id_current;
    for (int i = 0; i < 21; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            map_loc(x_radius, y_radius)->visibility |= 1 << new_owner;
            synch_bit(x_radius, y_radius, new_owner);
        }
    }
    for (int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; veh_id--) {
        if (Vehs[veh_id].faction_id != old_owner) {
            continue;
        }
        int dist = del_site_dist(Vehs[veh_id].x, Vehs[veh_id].y, x, y);
        if (dist > ((map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->climate & ~0x1F) >= ALT_BIT_SHORE_LINE)) {
            continue;
        }
        uint8_t plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
        if (!(((plan == 2 || plan == 3) && !dist)
            || (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA
                && (!dist || (map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->climate & ~0x1F) >= ALT_BIT_SHORE_LINE)))) {
            Bases[base_id].faction_id_current = (uint8_t)new_owner;
            int found = base_find(x, y, old_owner);
            Bases[base_id].faction_id_current = (uint8_t)old_owner;
            if (found >= 0 && territory_dist(Bases[found].x, Bases[found].y, x, y)) {
                stack_put(stack_veh(veh_id, 1), Bases[found].x, Bases[found].y);
                Vehs[veh_id].order = ORDER_NONE;
                if (Vehs[veh_id].home_base_id == base_id) {
                    Vehs[veh_id].home_base_id = (int16_t)found;
                }
                continue;
            }
        }
        PlayersData[old_owner].proto_id_active[Vehs[veh_id].proto_id]--;
        Vehs[veh_id].faction_id = (uint8_t)new_owner;
        PlayersData[new_owner].proto_id_active[Vehs[veh_id].proto_id]++;
        Vehs[veh_id].home_base_id = (int16_t)base_id;
        Vehs[veh_id].moves_expended = 0;
        if (Vehs[veh_id].order != ORDER_HOLD) {
            Vehs[veh_id].order = ORDER_NONE;
        }
        owner_set(Vehs[veh_id].x, Vehs[veh_id].y, new_owner);
    }
    Bases[base_id].faction_id_current = (uint8_t)new_owner;
    Bases[base_id].minerals_accumulated_2 = 0;
    Bases[base_id].minerals_accumulated = 0;
    for (int j = 0; j <= Bases[base_id].queue_size; j++) {
        if (Bases[base_id].queue_production_id[j] >= 0) {
            PlayersData[old_owner].proto_id_queue[Bases[base_id].queue_production_id[j]]--;
        }
    }
    Bases[base_id].queue_size = 0;
    Bases[base_id].production_id_last = -69;
    Bases[base_id].queue_production_id[0] = -69;
    replay_base(1, x, y, new_owner);
    Bases[base_id].state &= 0xFFDEF7B1;
    Bases[base_id].nerve_staple_turns_left = 0;
    if (DiplomacyProposalMode == 3) {
        if (Bases[base_id].faction_id_former != new_owner) {
            Bases[base_id].faction_id_former = (uint8_t)old_owner;
            Bases[base_id].assimilation_turns_left = 50;
        } else {
            Bases[base_id].assimilation_turns_left = 0;
        }
    } else {
        if (Bases[base_id].faction_id_former == new_owner) {
            Bases[base_id].assimilation_turns_left = 0;
        }
        Bases[base_id].faction_id_former = (uint8_t)new_owner;
    }
    spot_base(base_id, old_owner);
    reset_territory();
    for (veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; veh_id--) {
        if (veh_id < GameDataStateStorage.fields_.vehicle_current_count_
            && Vehs[veh_id].faction_id == old_owner && Vehs[veh_id].home_base_id == base_id) {
            kill(veh_id);
        }
    }
    owner_set(x, y, new_owner);
    draw_tile_fixup(x, y, 1, 2);
    set_base(base_id);
    base_compute(1);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0053C650 ?pact_ends@@YAXHH@Z 0x0053C650-0x0053CAE6;0x0065E470-0x0065E53A
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0053c650.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl pact_ends(int faction_id, int partner_id) {
    Popup popup;
    PopupDialogFactionID = partner_id;
    treaty_off(faction_id, partner_id, 1);
    // stand-in: an array keeps both results in fresh frame slots, as the
    // image does, instead of the parameter homes.
    int withdraw[2];
    withdraw[0] = pact_withdraw(faction_id, partner_id);
    withdraw[1] = pact_withdraw(partner_id, faction_id);
    draw_map(1);

    if (faction_id != ConsoleGlobal.faction_id_ && partner_id != ConsoleGlobal.faction_id_) {
        // Neither side is the local player: announce only to an observer.
        if (spying(faction_id) || spying(partner_id)
            || (static_cast<uint8_t>(GameDataStateStorage.fields_.game_state_) & 0x80)
            || (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction_id] & 1)
            || (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[partner_id] & 1)) {
            say_faction_noun(0, faction_id);
            say_faction_noun(1, partner_id);
            parse_says(2, get_pact_hood(faction_id, partner_id), -1, -1);
            X_pop("ENDPACT3", 0);
        }
    } else {
        int use_withdraw;
        if (faction_id == ConsoleGlobal.faction_id_) {
            say_faction_noun(0, partner_id);
            parse_says(1, Players[partner_id].adj_name_faction, -1, -1);
            use_withdraw = withdraw[0];
        } else {
            say_faction_noun(0, faction_id);
            parse_says(1, Players[faction_id].adj_name_faction, -1, -1);
            use_withdraw = withdraw[1];
        }
        parse_num(0, use_withdraw);
        parse_says(2, get_pact_hood(faction_id, partner_id), -1, -1);
        popup.start(PopupStartCaption, "ENDPACT1", -1, nullptr,
                    (IsMultiplayerNet ? 0 : 0x100000));
        if (use_withdraw) {
            popup.start(PopupStartCaption, "ENDPACT2", -1, nullptr,
                        (IsMultiplayerNet ? 0 : 0x100000) | 0x80, nullptr);
        }
        popup.exec(0, nullptr);
    }
}


// ORIGINAL: 0x0053C370 ?pact_withdraw@@YAHHH@Z 0x0053C370-0x0053C649 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0053c370.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
// TRIED: real signature per the mangled name (int __cdecl(int,int)). Outer loop walks Veh[] (g_0095282a = &Vehs[0].y, stride 0x34 matches veh.h's static_assert; veh-2/veh/veh+0xc/veh+8/veh+0xf/veh+2/veh+0x23 line up with Veh.x/y/faction_id/proto_id/order/state/morale from veh.h). Inner loop walks Bases[] (g_0097d040/42/44 = &Bases[0].x/y/ faction_id_current, stride 0x134 matches base.h's static_assert), not "units" as an earlier sibling function's comment guessed - confirmed by cross-checking both structs' real field offsets in src/veh.h and src/base.h rather than trusting Ghidra's raw index arithmetic. `veh_lift` is called for its (used) return value right before `veh_drop`, contradicting the catalogued `void __cdecl veh_lift(int)` - proposed int return below. Compiles; diverges at instruction #11 (stack slot scheduling for the many nested locals) - not chased further given the function's size.
int __cdecl pact_withdraw(int faction_id, int faction_id_with) {
    int lifted_count = 0;
    for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
        if (faction_id != Vehs[vehicle_id].faction_id) continue;

        const int base_at_vehicle = base_find(Vehs[vehicle_id].x, Vehs[vehicle_id].y);
        if (base_at_vehicle < 0) continue;

        if (Bases[base_at_vehicle].faction_id_current != faction_id_with &&
            whose_territory(faction_id, Vehs[vehicle_id].x, Vehs[vehicle_id].y, 0, 0) != faction_id_with) {
            continue;
        }

        int best_base = -1;
        int best_distance = 9999;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current != faction_id) continue;

            if (get_proto_triad(Vehs[vehicle_id].proto_id) == TRIAD_SEA) {
                // Preserve the original's coordinate-as-index bug exactly:
                // native uses base.x as a Bases[] index and base.y as the
                // is_coast boolean, rather than using base_id / TRUE. The
                // two named locals reproduce the image's load order.
                const int probe = Bases[base_id].x;
                const int probe_flag = Bases[base_id].y;
                if (!is_coast(Bases[probe].x, Bases[probe].y, probe_flag)) continue;
            }

            if (get_proto_triad(Vehs[vehicle_id].proto_id) == TRIAD_SEA) {
                const Map *tile = map_loc(Vehs[vehicle_id].x, Vehs[vehicle_id].y);
                if ((tile->climate & ~0x1F) < 0x60 &&
                    !base_on_sea(base_id, tile->region)) {
                    continue;
                }
            }

            const int distance = del_site_dist(Bases[base_id].x, Bases[base_id].y, Vehs[vehicle_id].x, Vehs[vehicle_id].y);
            if (distance < best_distance) {
                best_distance = distance;
                best_base = base_id;
            }
        }

        if (best_base < 0) continue;

        veh_put(vehicle_id, Bases[best_base].x, Bases[best_base].y);
        Vehs[vehicle_id].order = 0;
        Vehs[vehicle_id].state &= 0xFCFFBDFFu;

        // Native receiver points at Veh.y (+2); its byte +0x23 is the
        // terraforming counter (+0x25), not the adjacent morale byte.
        if (get_triad(vehicle_id) == TRIAD_AIR)
            Vehs[vehicle_id].terraforming_turns = 0;

        ++lifted_count;
        if (ConsoleGlobal.cursor_active_ != 0 &&
            vehicle_id == ConsoleGlobal.selected_vehicle_) {
            ConsoleGlobal.set_move(1);
        }
    }
    return lifted_count;
}


// ORIGINAL: 0x0053A230 ?mention_prototypes@@YAXHH@Z 0x0053A230-0x0053A496 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The complete original diplomacy producer uses the existing faction, prototype, parser and network owners; the global cdecl interface and bounded span retain the seed identity.","evidence":["original:0x53a230","git:46625ff3:src/recovered/units/0053a230.cpp"]}
// kind      game
void __cdecl mention_prototypes(int faction_id, int faction_id_with) {
    PopupDialogFactionID = faction_id_with;
    // Native 0x53A23E..0x53A249 rejects an unset proposed prototype before
    // it enters wants_to_speak; that callback may itself change the selected
    // prototype, so the value used below is deliberately re-read afterwards.
    if (BestProtoToMention < 0) return;
    if (!wants_to_speak(faction_id, faction_id_with)) return;

    for (int i = 0; i < MaxVehProtoFactionNum; ++i) {
        VehPrototypes[faction_id_with * MaxVehProtoFactionNum + i].flags |= PROTO_UNK_10;
    }

    const int selected_proto = BestProtoToMention;
    parse_says(0, VehPrototypes[selected_proto].veh_name, -1, -1);

    StringTemp[0] = 0;
    say_stats_2(StringTemp, selected_proto);
    parse_says(1, StringTemp, -1, -1);

    VehPrototypes[selected_proto].flags |= PROTO_UNK_10;
    StringTemp[0] = 0;
    strcat(StringTemp, "PROTO");

    if (PlayersData[faction_id].diplo_treaties[faction_id_with] & DTREATY_PACT) {
        VehPrototypes[selected_proto].flags |= PROTO_UNK_20;
        strcat(StringTemp, "PACT");
        return;
    }

    strcat(StringTemp, "RIVAL");
    if (!X_pops(StringTemp, 0x100000,
                      FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
        BestProtoForTrade = -1;
        BestProtoToMention = -1;
        return;
    }

    for (int proto_id = 0; proto_id < MaxVehProtoFactionNum; ++proto_id) {
        const int faction_proto =
            faction_id * MaxVehProtoFactionNum + proto_id;
        const int faction_proto_code = faction_proto + 0x61;
        if (wants_prototype(faction_proto, faction_id_with)) {
            if (IsMultiplayerNet != 0) {
                log_say(DiploLabelSendingTECH, faction_id_with,
                        faction_proto_code, faction_id);
                message_data(0x244B, 0, faction_id_with,
                             faction_proto_code, faction_id, 0);
                NetDaemonNet.await_diplo(0x44B);
            } else {
                tech_achieved(faction_id_with, faction_proto_code,
                              faction_id, 0);
            }
        }
        VehPrototypes[faction_proto].flags |= PROTO_UNK_20;

        const int other_proto =
            faction_id_with * MaxVehProtoFactionNum + proto_id;
        const int other_proto_code = other_proto + 0x61;
        if (wants_prototype(other_proto, faction_id)) {
            if (IsMultiplayerNet != 0) {
                log_say(DiploLabelSendingTECH, faction_id,
                        other_proto_code, faction_id_with);
                message_data(0x244B, 0, faction_id,
                             other_proto_code, faction_id_with, 0);
                NetDaemonNet.await_diplo(0x44B);
            } else {
                tech_achieved(faction_id, other_proto_code,
                              faction_id_with, 0);
            }
        }
        VehPrototypes[other_proto].flags |= PROTO_UNK_20;
    }

    BestProtoForTrade = -1;
    BestProtoToMention = -1;
}

// ORIGINAL: 0x0053CAF0 ?pact_of_brotherhood@@YAXHH@Z 0x0053CAF0-0x0053CE05 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The complete original diplomacy producer uses the existing faction, prototype, parser and network owners; the global cdecl interface and bounded span retain the seed identity.","evidence":["original:0x53caf0","git:46625ff3:src/recovered/units/0053caf0.cpp"]}
// kind      game
void __cdecl pact_of_brotherhood(int faction_id, int faction_id_with) {
    PopupDialogFactionID = faction_id_with;
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYON, faction_id_with, faction_id,
                0x01000001);
        message_data(0x2441, 0, faction_id_with, faction_id,
                     0x01000001, 0);
    } else {
        treaty_on(faction_id_with, faction_id, 0x01000001);
    }

    PlayersData[faction_id_with].diplo_patience[faction_id] = 0;
    PlayersData[faction_id].diplo_spoke[faction_id_with] = GameDataStateStorage.fields_.turn_current_num_;
    diplomacy_check(faction_id, faction_id_with, 0);
    DiploScratchFA70 = 0;
    ++PlayersData[faction_id].global_reputation;
    ++PlayersData[faction_id_with].global_reputation;
    diplomacy_caption(faction_id, faction_id_with);

    PluralityDefault = 0;
    GenderDefault = Players[faction_id_with].is_leader_female;
    parse_says(0, Players[faction_id_with].title_leader, -1, -1);
    GenderDefault = Players[faction_id_with].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[faction_id_with].name_leader, -1, -1);
    GenderDefault = Players[faction_id_with].noun_gender;
    PluralityDefault = Players[faction_id_with].is_noun_plural;
    parse_says(2, Players[faction_id_with].noun_faction, -1, -1);
    parse_says(3, get_pact(faction_id), -1, -1);
    parse_says(4, get_phrase(faction_id, 0), -1, -1);
    parse_says(5, get_phrase(faction_id_with, 0), -1, -1);

// The pact, transferred-map and tutorial paths select distinct script topics.
// Bounds cover the literal payload, not surrounding allocation or lifetime.
// RECOVERY_LITERAL: v1 0x0068D5D4 0x0068D5DD ??_C@_08BKBK@MAKEPACT?$AA@ 0x0053CAF0
    X_pops("MAKEPACT", 0x100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait);
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTRADEMAPS, faction_id, faction_id_with, 0);
        message_data(0x2448, 0, faction_id, faction_id_with, 0, 0);
        NetDaemonNet.await_diplo(0x448);
    } else {
        trade_maps(faction_id, faction_id_with);
    }

    parse_says(0, get_pact(faction_id), -1, -1);
// RECOVERY_LITERAL: v1 0x0068D5E0 0x0068D5EC ??_C@_0M@OOPF@HERESTHEMAP?$AA@ 0x0053CAF0
    X_pops("HERESTHEMAP", 0x100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait);
    scan_prototypes(faction_id, faction_id_with);
    mention_prototypes(faction_id, faction_id_with);

    if (GameDataStateStorage.fields_.diff_level_current_ > 1 && tut_check2(0x1000) == 0) return;
    if (IsMultiplayerNet == 0) {
        GenderDefault = Players[faction_id_with].noun_gender;
        PluralityDefault = Players[faction_id_with].is_noun_plural;
        parse_says(0, Players[faction_id_with].noun_faction, -1, -1);
        GenderDefault = Players[faction_id_with].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction_id_with].title_leader, -1, -1);
        GenderDefault = Players[faction_id_with].is_leader_female;
        PluralityDefault = 0;
        parse_says(2, Players[faction_id_with].name_leader, -1, -1);
        parse_says(3, Players[faction_id_with].adj_name_faction, -1, -1);
// RECOVERY_LITERAL: v1 0x0068D5EC 0x0068D5F1 ??_C@_04OBKO@PACT?$AA@ 0x0053CAF0
        X_pop(TutorialPopupCaption, "PACT", nullptr);
    }
}

// ORIGINAL: 0x0053CE10 ?make_treaty@@YAXHH@Z 0x0053CE10-0x0053D158 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The complete original diplomacy producer uses the existing faction, prototype, parser and network owners; the global cdecl interface and bounded span retain the seed identity.","evidence":["original:0x53ce10","git:46625ff3:src/recovered/units/0053ce10.cpp"]}
// kind      game
void __cdecl make_treaty(int faction_id, int faction_id_with) {
    PopupDialogFactionID = faction_id_with;
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYON, faction_id_with, faction_id,
                0x01000202);
        message_data(0x2441, 0, faction_id_with, faction_id,
                     0x01000202, 0);
        NetDaemonNet.await_diplo(0x441);
    } else {
        treaty_on(faction_id_with, faction_id, 0x01000202);
    }

    PlayersData[faction_id_with].diplo_patience[faction_id] = 0;
    PlayersData[faction_id].diplo_spoke[faction_id_with] = GameDataStateStorage.fields_.turn_current_num_;
    diplomacy_check(faction_id, faction_id_with, 0);
    ++PlayersData[faction_id].global_reputation;
    ++PlayersData[faction_id_with].global_reputation;
    diplomacy_caption(faction_id, faction_id_with);

    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(0, Players[faction_id].title_leader, -1, -1);
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[faction_id].name_leader, -1, -1);
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(2, Players[faction_id].noun_faction, -1, -1);

    PluralityDefault = 0;
    GenderDefault = Players[faction_id_with].is_leader_female;
    parse_says(3, Players[faction_id_with].title_leader, -1, -1);
    GenderDefault = Players[faction_id_with].is_leader_female;
    PluralityDefault = 0;
    parse_says(4, Players[faction_id_with].name_leader, -1, -1);
    GenderDefault = Players[faction_id_with].noun_gender;
    PluralityDefault = Players[faction_id_with].is_noun_plural;
    parse_says(5, Players[faction_id_with].noun_faction, -1, -1);

    parse_says(6, get_noun_phrase(faction_id, 9), -1, -1);
    parse_says(7, get_noun_phrase(faction_id_with, 9), -1, -1);

    StringTemp[0] = 0;
// Treaty, commerce and withdrawal notifications retain their script-topic identities.
// Bounds cover the literal payload, not surrounding allocation or lifetime.
// RECOVERY_LITERAL: v1 0x0068D5F4 0x0068D5FF ??_C@_0L@LJNL@MAKETREATY?$AA@ 0x0053CE10
    strcat(StringTemp, "MAKETREATY");
    char number[80];
    const int choice = static_cast<int>(random(0, 2));
    _itoa(choice, number, 10);
    strcat(StringTemp, number);
    X_pops(StringTemp, 0x100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait);

    if (GameDataStateStorage.fields_.diff_level_current_ <= 1 || tut_check2(0x800) != 0) {
        if (IsMultiplayerNet == 0) {
            GenderDefault = Players[faction_id_with].noun_gender;
            PluralityDefault = Players[faction_id_with].is_noun_plural;
            parse_says(0, Players[faction_id_with].noun_faction, -1, -1);
// RECOVERY_LITERAL: v1 0x0068D600 0x0068D609 ??_C@_08DFDO@COMMERCE?$AA@ 0x0053CE10
            X_pop(TutorialPopupCaption, "COMMERCE", nullptr);
        }
    }

    if (territory(faction_id_with, faction_id, 2, nullptr, nullptr) != 0) {
        GenderDefault = Players[faction_id_with].noun_gender;
        PluralityDefault = Players[faction_id_with].is_noun_plural;
        parse_says(0, Players[faction_id_with].noun_faction, -1, -1);
        parse_says(1, Players[faction_id_with].adj_name_faction, -1, -1);
// RECOVERY_LITERAL: v1 0x0068D60C 0x0068D618 ??_C@_0M@PKFO@TIMETOLEAVE?$AA@ 0x0053CE10
        NetMsgPrimary.pop("TIMETOLEAVE", 5000, 0, nullptr);
    }
}

// ORIGINAL: 0x0053D160 ?pledge_truce@@YAXHHH@Z 0x0053D160-0x0053D351 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The complete original diplomacy producer uses the existing faction, prototype, parser and network owners; the global cdecl interface and bounded span retain the seed identity.","evidence":["original:0x53d160","git:46625ff3:src/recovered/units/0053d160.cpp"]}
// kind      game
void __cdecl pledge_truce(int faction_id, int faction_id_with,
                          int show_dialogue) {
    PopupDialogFactionID = faction_id_with;
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYON, faction_id_with, faction_id, 0x204);
        message_data(0x2441, 0, faction_id_with, faction_id, 0x204, 0);
    } else {
        treaty_on(faction_id_with, faction_id, 0x204);
    }

    PlayersData[faction_id].diplo_spoke[faction_id_with] = GameDataStateStorage.fields_.turn_current_num_;
    diplomacy_check(faction_id, faction_id_with, 0);
    ++PlayersData[faction_id].global_reputation;
    ++PlayersData[faction_id_with].global_reputation;
    if (show_dialogue != 0) {
        diplomacy_caption(faction_id, faction_id_with);
        faction_parse_leader_fields(faction_id, 0, 1, 2, -1);
        faction_parse_leader_fields(faction_id_with, 3, 4, 5, -1);
        X_pops("MAKETRUCE", 0x100000,
                     FactionDiplomacyPortraits[faction_id_with], pop_wait);
    }
}


static int battle_plans_selected_region_distance_result(
    int faction_id, const Base &selected_base, const Base &plan_base) {
    const int selected_region =
        map_loc(selected_base.x, selected_base.y)->region;
    const PlayerData &planner = PlayersData[faction_id];

    if (selected_region >= MaxRegionLandNum
        || planner.region_total_bases[selected_region] == 0) {
        int region;
        for (region = 0; region < MaxRegionLandNum; ++region) {
            if (region != 0 && region != MaxRegionLandNum - 1
                && planner.region_total_bases[region] != 0
                && planner.region_base_plan[region] == PLAN_NAVAL_TRANSPORT) {
                return 1;
            }
        }
    }

    base_find(plan_base.x, plan_base.y, faction_id);
    const int plan_distance = BaseFindDist;
    base_find(selected_base.x, selected_base.y, faction_id);
    const int doubled_plan_distance =
        (int32_t)((uint32_t)plan_distance * 2u);
    if (BaseFindDist >= doubled_plan_distance) {
        return 2;
    }
    return 0;
}

// ORIGINAL: 0x00547C10 ?make_gift@@YAXHH@Z 0x00547C10-0x00548DC8;0x0065F4C4-0x0065F662
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned original547C10-548DC8 and cleanup65F4C4-65F662", "historical seed:src/unrecovered/00547c10.cpp", "Native547CDC-547D39 single divide-by-two and /25 menu recurrence;547EF1-547F58 raw-selection transfer;547F6B-547FE7 friction buckets;548972 live mode read;548AE8 post-modal tech read;54845F/548C52 differing treaty directions;5485D9 literal BASEGIFT result."], "reason": "Recover all energy, selected/all-technology and base gift paths with one genuine lexical Popup and existing faction/global ownership. Preserve raw selected energy separately from generosity scoring, native nonlinear friction buckets, directed treaty/counter/patience updates and local response-key lifetime. Remove the register-machine object shim, not historical obligations."}
// symbol    ?make_gift@@YAXHH@Z
// TRIED: literal register-machine transliteration (pseudo-regs eax..edi, a raw 0x5440-byte buffer for the Popup/BasePop/Scroll object standing in for real ebp-relative locals, goto per basic block). Popup/BasePop/Scroll/FlatButton/ Dialogs/Dialog/Spot/Sprite/Heap/StringStruct lifecycle calls (ctor, start, exec, close, item, sprite, dtors) go through original_method<> on that buffer instead of real typed C++ locals, so no SEH frame or manual vtable re-stamp before each close()/dtor call is modeled - those writes are compiler-generated partial-construction unwind bookkeeping with no observable effect once the real ctor already ran. cdq/sub/sar divide-by- 2/3/4/6/10 idioms folded to `/N`; the negative-magic-constant /5 idiom (0x99999999) folded to `-(x/5)`. Covers the full control-flow graph and every external/method call in original order (4899 rebuilt vs 4536 original bytes); divergence is from the object-model/frame shape, not from missing branches or wrong call order.
void __cdecl make_gift(int giver, int receiver) {
    Popup popup;
    const int mode = DiplomacyProposalMode;
    PopupDialogFactionID = receiver;
    if (mode == 4 || mode == 7) {
        tech_analysis(giver, receiver);
        if (DiploScratchFAA8 < 0) {
            GenderDefault = Players[receiver].noun_gender;
            PluralityDefault = Players[receiver].is_noun_plural;
            parse_says(1, Players[receiver].noun_faction, -1, -1);
            X_pops("REJTECHNOTHING", 0x100000,
                         FactionDiplomacyPortraits[receiver], pop_wait);
            return;
        }

        int gift_value = 0;
        if (DiplomacyProposalMode == 7) {
            for (int tech = 0; tech < 0x59; ++tech) {
                if (has_tech(tech, giver) && !has_tech(tech, receiver)) {
                    gift_value = diplo_add(gift_value, tech_val(tech, receiver, 1));
                    if (IsMultiplayerNet != 0) {
                        log_say(DiploLabelSendingTECH, receiver, tech, giver);
                        message_data(0x244b, 0, receiver, tech, giver, 0);
                        NetDaemonNet.await_diplo(0x44b);
                    } else {
                        tech_achieved(receiver, tech, giver, 0);
                    }
                }
            }
        } else {
            StringTemp[0] = 0;
            say_tech(StringTemp, DiploScratchFAA8, 1);
            parse_says(0, StringTemp, -1, -1);
            parse_says(2, get_noun_phrase(receiver, 9), -1, -1);

            int response;
            if (DiploScratchFA18 >= 0) {
                StringTemp[0] = 0;
                say_tech(StringTemp, DiploScratchFA18, 1);
                parse_says(1, StringTemp, -1, -1);
                response = X_pops("OFFERTECH2", 0x100000,
                                        FactionDiplomacyPortraits[receiver], pop_wait);
            } else {
                response = X_pops("OFFERTECH", 0x100000,
                                        FactionDiplomacyPortraits[receiver], pop_wait);
            }
            if (response == 0) return;

            const int selected = (response == 1) ? DiploScratchFAA8
                                                  : DiploScratchFA18;
            gift_value = tech_val(selected, receiver, 1);
            if (IsMultiplayerNet != 0) {
                log_say(DiploLabelSendingTECH, receiver, selected, giver);
                message_data(0x244b, 0, receiver, selected, giver, 0);
                NetDaemonNet.await_diplo(0x44b);
            } else {
                tech_achieved(receiver, selected, giver, 0);
            }
        }

        const char *recipient_key = Players[receiver].search_key;
        if (strcmp(recipient_key, "UNIV") == 0 ||
            strcmp(recipient_key, "SPARTANS") == 0) gift_value = diplo_mul(gift_value, 2);
        if (strcmp(recipient_key, "BELIEVE") == 0) gift_value /= 2;

        const int friction_after_gift = diplo_clamp(
            diplo_sub(static_cast<int>(PlayersData[receiver].diplo_friction[giver]),
                      gift_value), 0, 20);
        PlayersData[receiver].diplo_friction[giver] =
            static_cast<uint32_t>(friction_after_gift);
        if (DiploFrictionFactionID == static_cast<uint32_t>(receiver) &&
            DiploFrictionFactionIDWith == static_cast<uint32_t>(giver))
            DiploFriction = diplo_sub(DiploFriction, gift_value);

        char popup_label[80];
        popup_label[0] = 0;
        strcat(popup_label, "GAVETECH");
        if (gift_value >= 0x10) {
            if ((PlayersData[receiver].diplo_treaties[giver] & 0x60u) != 0 &&
                DiploFriction != 0)
                strcat(popup_label, "2");
            else
                strcat(popup_label, "1");

            if (IsMultiplayerNet != 0) {
                log_say(DiploLabelSendingTREATYOFF, receiver, giver, 0x60);
                message_data(0x2442, 0, receiver, giver, 0x60, 0);
                NetDaemonNet.await_diplo(0x442);
            } else {
                treaty_off(receiver, giver, 0x60);
            }
        }

        int &counter = PlayersData[giver].diplo_unk1[receiver];
        const int counter_limit = PlayersData[giver].integrity_blemishes;
        if (gift_value >= 0x10) {
            if (counter < counter_limit) counter = diplo_add(counter, 1);
            if (counter < counter_limit || great_satan(giver, 0) != 0 ||
                IsMultiplayerNet != 0)
                counter = diplo_add(counter, 1);
        } else if (counter == 0 &&
                   (counter < counter_limit || great_satan(giver, 0) != 0 ||
                    IsMultiplayerNet != 0)) {
            counter = diplo_add(counter, 1);
        }

        diplomacy_caption(giver, receiver);
        X_pops(popup_label, 0x100000,
                     FactionDiplomacyPortraits[receiver], pop_wait);
        char &patience = PlayersData[receiver].diplo_patience[giver];
        patience = static_cast<char>(static_cast<unsigned char>(patience) + 0xFEu);
        return;
    }

    if (mode == 5) {
        popup.start(PopupStartCaption, "OFFERENERGY", -1, nullptr, 0x100000);
        popup.set_caller_picture(popup.picture_enabled()
            ? nullptr : FactionDiplomacyPortraits[receiver]);
        popup.dialogs().sprite(FactionDiplomacyPortraits[receiver], nullptr, 0);

        int offered_energy = diplo_mul(PlayersData[giver].energy_reserves, 2);
        for (;;) {
            offered_energy = diplo_mul((offered_energy / 2) / 25, 25);
            StringTemp[0] = 0;
            char amount_text[12];
            _itoa(offered_energy, amount_text, 10);
            strcat(StringTemp, amount_text);
            popup.dialogs().item(StringTemp, offered_energy);
            if (offered_energy == 0) break;
        }

        const int selection = popup.exec(0, pop_wait); // callback is 0x5398E0
        if (selection == 0) return;

        int amount = selection;
        const char *const search_key = Players[giver].search_key;
        if (strcmp(search_key, "MORGAN") == 0 ||
            strcmp(search_key, "SPARTANS") == 0) {
            amount = diplo_mul(amount, 2);
        }
        if (strcmp(search_key, "SPARTANS") == 0) amount /= 2;

        const int receiver_energy = PlayersData[receiver].energy_reserves;
        const int giver_energy = PlayersData[giver].energy_reserves;
        const int favored =
            amount >= receiver_energy / 2 &&
            amount >= giver_energy / 3 &&
            amount >= diplo_mul(GameDataStateStorage.fields_.turn_current_num_, 2);
        const int strong_offer =
            amount >= receiver_energy / 4 && amount >= 25 &&
            amount >= giver_energy / 6 && amount >= GameDataStateStorage.fields_.turn_current_num_;

        if (IsMultiplayerNet != 0) {
            log_say(DiploLabelSendingENERGY, giver, receiver, diplo_sub(0, selection));
            message_data(0x2447, 0, giver, diplo_sub(0, selection), receiver, selection);
            NetDaemonNet.await_diplo(0x447);
        } else {
            if (giver != 0) PlayersData[giver].energy_reserves = diplo_sub(PlayersData[giver].energy_reserves, selection);
            if (receiver != 0) PlayersData[receiver].energy_reserves = diplo_add(PlayersData[receiver].energy_reserves, selection);
            ConsoleGlobal.update_data(1);
        }

        ConsoleInfoWin.set_redraw_phase(0);
        ConsoleGlobal.update_data(0);

        int remaining_fifths = amount / 5;
        int bucket = 10;
        int divisor = 2;
        int accumulated = 0;
        while (remaining_fifths > 0) {
            const int part = remaining_fifths < bucket ? remaining_fifths : bucket;
            accumulated = diplo_add(accumulated, part / divisor);
            remaining_fifths -= bucket;
            bucket = 20;
            ++divisor;
        }
        const int fifth = diplo_sub(0, diplo_add(accumulated, 4)) / 5;
        const int friction_delta = diplo_mul(fifth, 3) / 2;
        int friction = diplo_add(PlayersData[receiver].diplo_friction[giver], friction_delta);
        if (friction < 0) friction = 0;
        if (friction > 20) friction = 20;
        PlayersData[receiver].diplo_friction[giver] = static_cast<uint32_t>(friction);
        if (DiploFrictionFactionID == receiver &&
            DiploFrictionFactionIDWith == giver) {
            DiploFriction = diplo_add(DiploFriction, friction_delta);
        }

        StringTemp[0] = 0;
        strcat(StringTemp, "GAVEENERGY");
        if (favored) {
            const int treaty60 = PlayersData[receiver].diplo_treaties[giver] & 0x60;
            const int is_great_satan = treaty60 == 0 ? great_satan(giver, 0) : 1;
            strcat(StringTemp, treaty60 == 0 && is_great_satan == 0 ? "1" : "2");
            if (IsMultiplayerNet != 0) {
                log_say(DiploLabelSendingTREATYOFF, giver, receiver, 0x60);
                message_data(0x2442, 0, giver, receiver, 0x60, 0);
                NetDaemonNet.await_diplo(0x442);
            } else {
                treaty_off(giver, receiver, 0x60);
            }
            int &counter = PlayersData[giver].diplo_unk1[receiver];
            const int limit = PlayersData[giver].integrity_blemishes;
            if (counter < limit) counter = diplo_add(counter, 1);
        }

        if (favored || (strong_offer && PlayersData[giver].diplo_unk1[receiver] == 0)) {
            int &counter = PlayersData[giver].diplo_unk1[receiver];
            const int limit = PlayersData[giver].integrity_blemishes;
            if (counter < limit || great_satan(giver, 0) != 0 || IsMultiplayerNet != 0)
                counter = diplo_add(counter, 1);
        }

        diplomacy_caption(giver, receiver);
        PluralityDefault = 0;
        GenderDefault = Players[giver].is_leader_female;
        parse_says(0, Players[giver].title_leader, -1, -1);
        GenderDefault = Players[giver].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[giver].name_leader, -1, -1);
        X_pops(StringTemp, 0x100000,
                     FactionDiplomacyPortraits[receiver], pop_wait);

        const int patience_delta = 0xFE - favored;
        PlayersData[receiver].diplo_patience[giver] = static_cast<char>(
            static_cast<unsigned char>(PlayersData[receiver].diplo_patience[giver]) +
            patience_delta);
        return;
    }

    if (mode == 8) {
        const int base_value = value_of_base(
            DiploScratchFA30, receiver, giver, DiploScratchFA98,
            (DiploScratchFA68 != 0 || DiploScratchFAA0 != 0));

        const int friction_delta =
            diplo_sub(0, diplo_mul(base_value, 3)) / 2;
        int friction = diplo_add(
            static_cast<int>(PlayersData[receiver].diplo_friction[giver]),
            friction_delta);
        if (friction < 0) friction = 0;
        if (friction > 20) friction = 20;
        PlayersData[receiver].diplo_friction[giver] =
            static_cast<uint32_t>(friction);
        if (DiploFrictionFactionID == static_cast<uint32_t>(receiver) &&
            DiploFrictionFactionIDWith == static_cast<uint32_t>(giver)) {
            DiploFriction = diplo_add(DiploFriction, friction_delta);
        }

        int favored_value = base_value;
        if (strcmp(Players[giver].search_key, "SPARTANS") == 0)
            favored_value = diplo_mul(favored_value, 2);

        int favored =
            favored_value >= static_cast<int>(PlayersData[receiver].energy_reserves) / 2 &&
            favored_value >= static_cast<int>(PlayersData[giver].energy_reserves) / 3 &&
            favored_value >= diplo_mul(GameDataStateStorage.fields_.turn_current_num_, 2);
        if (strcmp(Players[giver].filename, "BELIEVE") == 0)
            favored = 1;

        log_say(DiploLabelGettingDIPLOMATICLOCK, 0, 0, 0);
        timeGetTime();
        if (ExitTurnLoop == 0) {
            for (;;) {
                if (NetDaemonNet.add_global() == 0) break;
                check_spock();
                timeGetTime();
                if (ExitTurnLoop != 0) break;
            }
        }

        diplomacy_caption(giver, receiver);
        // The image builds a suffixed local key, but the final popup uses
        // the unsuffixed BASEGIFT literal rather than this local buffer.
        char gift_result_key[80];
        gift_result_key[0] = 0;
        strcat(gift_result_key, "BASEGIFT");
        if (favored) {
            const bool existing_pact_or_treaty =
                (PlayersData[receiver].diplo_treaties[giver] & 0x60u) != 0;
            const char *suffix = (existing_pact_or_treaty ||
                                  great_satan(giver, 0) != 0) ? "2" : "1";
            strcat(gift_result_key, suffix);

            if (IsMultiplayerNet != 0) {
                log_say(DiploLabelSendingTREATYOFF, giver, receiver, 0x60);
                message_data(0x2442, 0, giver, receiver, 0x60, 0);
            } else {
                treaty_off(giver, receiver, 0x60);
            }
            int &gift_counter = PlayersData[giver].diplo_unk1[receiver];
            if (gift_counter < PlayersData[giver].integrity_blemishes)
                gift_counter = diplo_add(gift_counter, 1);
        }

        int &gift_counter = PlayersData[giver].diplo_unk1[receiver];
        if (gift_counter < PlayersData[giver].integrity_blemishes ||
            great_satan(giver, 0) != 0 || IsMultiplayerNet != 0) {
            gift_counter = diplo_add(gift_counter, 1);
        }

        PluralityDefault = 0;
        GenderDefault = Players[giver].is_leader_female;
        parse_says(0, Players[giver].title_leader, -1, -1);

        if (IsMultiplayerNet != 0) {
            log_say(DiploLabelSendingCEDEBASE, DiploScratchFA30, receiver, 0);
            message_data(0x2449, 0, DiploScratchFA30, receiver, 0, 0);
            NetDaemonNet.await_diplo(0x449);
        } else {
            give_a_base(DiploScratchFA30, receiver);
        }

        log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
        NetDaemonNet.unlock();
        char &patience = PlayersData[receiver].diplo_patience[giver];
        patience = static_cast<char>(static_cast<unsigned char>(patience) + 0xFE - favored);
        X_pops("BASEGIFT", 0x100000,
                     FactionDiplomacyPortraits[receiver], pop_wait);
        return;
    }

}

// ORIGINAL: 0x0054D410 ?base_swap@@YAXHH@Z 0x0054D410-0x0054DC8C
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned original54D410-54DC8C; caller54F77E", "historical seed:src/unrecovered/0054d410.cpp", "Independent native review:54D483/54D4FF name pointers;54D4AC/54D52E former owner;54D5C2-54D5E4 facilities; original dialogue keys and energy/tech/base transfer tuples."], "reason": "Recover complete base exchange, energy purchase and all-technology settlement with real Base/PlayerData fields, original lock/transfer order and source-owned selectors. Native Base+5 is former owner, Base+13h is the name start. Preserve catalogue void ABI: its sole caller does not consume a result."}
// symbol    ?base_swap@@YAHHH@Z
// TRIED: transcribed full Ghidra structure (verified against raw disasm: value_of_base/parse_says/X_pops/log_say arg order, the parameter-slot reuse where [ebp+0xc] becomes the swap `amount`). Compiles; diverges at #7 (register alloc), close for a 2172-byte function with 10 return sites.
// Earlier recovery guessed a void signature because its caller ignores EAX.
// Each pinned exit explicitly returns 0 after transfers or 1 on no-transfer paths.
int __cdecl base_swap(int faction_id_res, int faction_id_req) {
    PopupDialogFactionID = faction_id_req;

    const int contact = (DiploScratchFA68 != 0 || DiploScratchFAA0 != 0);
    int requested_value = value_of_base(DiploScratchFA7C, faction_id_req,
                                        faction_id_res, DiploScratchFA98,
                                        contact);
    int offered_value = value_of_base(DiploScratchFA30, faction_id_req,
                                      faction_id_res, 0, 0);

    if (DiploScratchFA7C >= 0) {
        parse_says(0, Bases[DiploScratchFA7C].name_string, -1, -1);
        if (Bases[DiploScratchFA7C].faction_id_former == faction_id_res)
            requested_value /= 2;
        const uint32_t treaty = PlayersData[faction_id_req].diplo_treaties[faction_id_res];
        if ((treaty & 1u) != 0 && (treaty & 0x02000000u) != 0)
            requested_value /= 2;
    }

    if (DiploScratchFA30 >= 0) {
        parse_says(1, Bases[DiploScratchFA30].name_string, -1, -1);
        if (Bases[DiploScratchFA30].faction_id_former == faction_id_req)
            offered_value = diplo_mul(offered_value, 2);
        if (DiploScratchFA30 >= 0
            && (offered_value < 0 || PlayersData[faction_id_res].current_num_bases == 1)) {
            X_pops("NOHQ", 0x100000,
                   FactionDiplomacyPortraits[faction_id_req], pop_wait);
            return 1;
        }
    }

    if (DiploScratchFA7C >= 0
        && (requested_value < 0
            || PlayersData[faction_id_req].current_num_bases == 1
            || has_fac(FAC_HEADQUARTERS, DiploScratchFA7C, 0))) {
        X_pops("NOMYHQ", 0x100000,
               FactionDiplomacyPortraits[faction_id_req], pop_wait);
        return 1;
    }

    if ((PlayersData[faction_id_req].diplo_treaties[faction_id_res] & 0x20u) != 0 &&
        DiplomacyProposalMode != 3) {
        X_pops("NEVERBASESWAP", 0x100000,
               FactionDiplomacyPortraits[faction_id_req], pop_wait);
        return 1;
    }

    if (DiploScratchFA30 >= 0) {
        if (DiploScratchFA7C >= 0) {
            const int offered_roll = random(0, 100) + offered_value;
            if (static_cast<int>(random(0, 100) + requested_value) > offered_roll) {
                diplo_lock(0);
                net_cede_base(DiploScratchFA7C, faction_id_res, 1);
                net_cede_base(DiploScratchFA30, faction_id_req, 1);
                diplo_unlock();
                X_pops("YESBASESWAP", 0x100000,
                       FactionDiplomacyPortraits[faction_id_req], pop_wait);
                return 0;
            }
            X_pops("NOBASESWAP3", 0x100000,
                   FactionDiplomacyPortraits[faction_id_req], pop_wait);
        }
    } else {
        if (DiploScratchFA7C >= 0) {
            const int numerator =
                diplo_mul(diplo_add(PlayersData[faction_id_res].ranking,
                                   PlayersData[faction_id_req].ranking),
                          requested_value);
            int amount = numerator /
                diplo_mul(PlayersData[faction_id_req].ranking, 2);
            if (GameDataStateStorage.fields_.faction_rankings_[7] == static_cast<uint32_t>(faction_id_res)) amount = diplo_mul(amount, 2);

            if ((DiplomacyProposalMode != 5 && DiplomacyProposalMode != 2)
                || static_cast<int>(PlayersData[faction_id_res].energy_reserves) < amount) {
                if (DiplomacyProposalMode == 5) {
                    parse_num(0, amount);
                    X_pops("NOBASESWAP", 0x100000,
                           FactionDiplomacyPortraits[faction_id_req], pop_wait);
                    return 1;
                }
                if (DiplomacyProposalMode != 4 && DiplomacyProposalMode != 2) {
                    parse_num(0, amount);
                    X_pops("NOBASESWAP", 0x100000,
                           FactionDiplomacyPortraits[faction_id_req], pop_wait);
                    return 1;
                }

                int transferable_tech_value = 0;
                int tech_id;
                for (tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
                    if (!has_tech(tech_id, faction_id_req) &&
                        has_tech(tech_id, faction_id_res)) {
                        transferable_tech_value = diplo_add(transferable_tech_value,
                                                          tech_val(tech_id, faction_id_req, 1));
                    }
                }
                if (!((PlayersData[faction_id_req].tech_ranking / 2 >
                 PlayersData[faction_id_res].tech_ranking / 2 + 2) ||
                (diplo_mul(diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8, transferable_tech_value) < amount))) {
                    PluralityDefault = 0;
                    GenderDefault = Players[faction_id_req].is_leader_female;
                    parse_says(2, Players[faction_id_req].name_leader, -1, -1);
                    if (X_pops("TECHBASESWAP", 0x100000,
                               FactionDiplomacyPortraits[faction_id_req], pop_wait) == 0)
                        return 1;

                    diplo_lock(0);
                    for (tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
                        if (!has_tech(tech_id, faction_id_req) &&
                            has_tech(tech_id, faction_id_res)) {
                            net_tech(faction_id_req, tech_id, faction_id_res, 0);
                        }
                    }
                    net_cede_base(DiploScratchFA7C, faction_id_res, 1);
                    diplo_unlock();
                    X_pops("BASEOK", 0x100000,
                           FactionDiplomacyPortraits[faction_id_req], pop_wait);
                    return 0;
                }
                X_pops("NOBASESWAP2", 0x100000,
                       FactionDiplomacyPortraits[faction_id_req], pop_wait);
                return 1;
            }
            parse_num(0, amount);
            if (X_pops("PAYBASESWAP", 0x100000,
                       FactionDiplomacyPortraits[faction_id_req], pop_wait) == 0)
                return 1;

            diplo_lock(0);
            net_energy(faction_id_res, -amount, faction_id_req, amount, 0);
            net_cede_base(DiploScratchFA7C, faction_id_res, 1);
            diplo_unlock();
            X_pops("BASEOK", 0x100000,
                   FactionDiplomacyPortraits[faction_id_req], pop_wait);
            return 0;
        }
    }
    return 1;
}


// Provisional pact workflow gate; pinned signed-DWORD reads at544C7A..544C8E.
// RECOVERY_OBJECT: v1 0x0093F7D0 ?DiplomacyProposalGateCount@@3HA
int DiplomacyProposalGateCount = 0;

// ORIGINAL: 0x00543BC0 ?propose_pact@@YAHHH@Z 0x00543BC0-0x00544E36
// RECOVERY_BINDING: v1 {"evidence": ["task:propose-pact-owner", "Pinned543BC0..544E36: initial eligibility normalization, signed ranking/current base counts, ordinary array columns, cap staging and repeated adjustment, live callbacks, native popup labels and ordered network/help paths.", "Native93F7D0 is a signed four-byte workflow gate; source uses a provisional role name and pinned initial zero, with no claim of semantic completeness for its other producers."], "reason": "Recover the complete faction-owned pact proposal using genuine faction records, parser state, typed services and source-owned gate storage. Retained TRIED text describes the superseded seed approximation, not this implementation. No byte or game acceptance claim."}
// TRIED: no Popup/SEH here (free AI-heuristic function, not a UI handler) - the difficulty is a long chain of inc/dec on a score accumulator plus several reciprocal-multiplication constant-divide idioms, transcribed with plain `/` and `%` (semantically equivalent, not byte-identical). Two known simplifications, both commented in place: (1) the original unconditionally zeroes its score accumulator midway through (0x544408 `mov [ebp+0xc],ecx`) before re-deriving a fresh propose/decline decision from separate flags - this body keeps accumulating instead of discarding, to keep the later score-gated branches meaningful; (2) the "generosity cap" energy-gift arithmetic in the L54436D block approximates the original's fixed-point chain rather than reproducing it exactly. sim 0.28-0.58 across flag sets.
int __cdecl propose_pact(int myFac, int otherFac) {
    PopupDialogFactionID = otherFac;
    int strength_gap = 0;
    int score = DiploScratchF7E0 + DiploScratchFA54;
    if (DiploScratchFA54) {
        score++;
    }
    int partner_id;
    if (DiploScratchF7E8) {
        partner_id = DiploScratchF7E8;
    } else {
        partner_id = DiploScratchFA8C;
    }
    int designated = GameDataStateStorage.fields_.ranking_faction_id_unk1_;
    if (IsMultiplayerNet) {
        int designated_target = GameDataStateStorage.fields_.ranking_faction_id_unk2_;
        if (myFac == designated
            && (PlayersData[designated_target].ranking < 6
                || FactionRankingsUnk[myFac] * 2 > FactionRankingsUnk[designated_target] * 3)) {
            score++;
        }
        if (myFac == designated_target
            && (PlayersData[myFac].ranking < 6
                || FactionRankingsUnk[GameDataStateStorage.fields_.faction_rankings_[7]] * 2
                    > FactionRankingsUnk[myFac] * 3)) {
            score--;
        }
    }
    if (GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY && myFac == designated) {
        score++;
    }
    if (GameDataStateStorage.fields_.game_rules_ & 0x1000
        && !(GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)) {
        score--;
    }
    if (PlayersData[otherFac].flags & 0x400000) {
        score--;
    }
    if (PlayersData[myFac].ranking >= 6 && !DiploScratchF7F0
        && PlayersData[otherFac].ranking >= (PlayersData[myFac].ranking == 7 ? 5 : 6)) {
        score++;
        if (GameDataStateStorage.fields_.turn_current_num_ >= (IsMultiplayerNet ? 50 : 100)) {
            score++;
        }
    } else if (PlayersData[myFac].ranking >= 6 && PlayersData[otherFac].ranking >= 4
        && GameDataStateStorage.fields_.turn_current_num_ >= 50 && !DiploScratchF7F0) {
        if (DiploScratchF7FC) {
            score++;
        } else if (!DiploScratchFA54) {
            score--;
        }
    }
    if (DiploScratchF7FC) {
        score++;
    }
    if (partner_id) {
        score--;
        if (great_satan(partner_id, 0)) {
            score--;
        }
        if (PlayersData[partner_id].ranking > PlayersData[otherFac].ranking) {
            score--;
        }
        if (PlayersData[otherFac].diplo_treaties[partner_id] & 0x8000
            && PlayersData[otherFac].mil_strength_1 < PlayersData[partner_id].mil_strength_1) {
            strength_gap = 1;
            score -= (PlayersData[otherFac].mil_strength_1 / 2 + PlayersData[partner_id].mil_strength_1)
                / (PlayersData[otherFac].mil_strength_1 + 1);
        }
    }
    if (DiploFriction >= 10 && DiplomacyProposalMode != 3) {
        score += DiploFriction - 10;
    }
    if (PlayersData[myFac].global_reputation >= 2 && !DiploScratchFA78) {
        score--;
    }
    if (PlayersData[myFac].global_reputation < 2) {
        score++;
    }
    if (PlayersData[otherFac].tech_commerce_bonus < PlayersData[myFac].tech_commerce_bonus + 1) {
        score++;
    }
    if (PlayersData[otherFac].tech_commerce_bonus > PlayersData[myFac].tech_commerce_bonus) {
        score--;
    }
    if (!IsMultiplayerNet && !strcmp(Players[otherFac].filename, "HIVE")) {
        score--;
    }
    if (DiploScratchFA98) {
        score--;
    }
    if (DiploScratchFA68) {
        score++;
    }
    if (DiploScratchFA3C) {
        score++;
    }
    if (PlayersData[otherFac].ranking > PlayersData[myFac].ranking) {
        if (PlayersData[otherFac].ranking == 7) {
            score += 2;
        }
        if (PlayersData[otherFac].ranking == 6) {
            score++;
        }
        if (PlayersData[myFac].ranking <= 4) {
            score--;
        }
    } else {
        if (PlayersData[myFac].ranking == 7) {
            score += DiploScratchFA54;
        }
        if (PlayersData[myFac].ranking - PlayersData[otherFac].ranking >= 3) {
            score--;
        }
        if (great_satan(myFac, 0)) {
            score++;
        }
    }
    if (PlayersData[myFac].ranking <= 5 && PlayersData[otherFac].ranking <= 5) {
        score--;
    }
    score = (DiploScratchFA9C > score || !PlayersData[myFac].diff_level)
        && !(PlayersData[myFac].diplo_agenda[otherFac] & 0x2000)
        && DiploScratchFA78 <= (PlayersData[myFac].diff_level ? 0 : 2)
            + ((GameDataStateStorage.fields_.game_rules_ >> 12) & 1) + (strength_gap ? 2 : 0) + 2
        && !(PlayersData[otherFac].diplo_treaties[myFac] & 0x20)
        && !DiploScratchFA70;
    diplomacy_caption(myFac, otherFac);
    parse_says(0, get_pact(myFac), -1, -1);
    parse_set(Players[myFac].is_leader_female, 0);
    parse_says(1, Players[myFac].title_leader, -1, -1);
    parse_set(Players[myFac].is_leader_female, 0);
    parse_says(2, Players[myFac].name_leader, -1, -1);
    char number[80];
    if (DiplomacyProposalMode == 3 && DiploScratchFA24) {
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingSETTREATY, otherFac, myFac, 0x2000000);
            message_data(0x2443, 0, otherFac, myFac, 0x2000000, 1);
        } else {
            set_treaty(otherFac, myFac, 0x2000000, 1);
        }
        pact_of_brotherhood(myFac, otherFac);
        return 1;
    }
    if (DiplomacyProposalMode == 3 || !score) {
        if ((DiploScratchFA78 > 2 || DiploScratchFA2C) && (DiploScratchFA98 || DiploFriction <= 2)
            && !(PlayersData[otherFac].diplo_treaties[myFac] & 0x20)) {
            X_pops("REJPACTMEEK", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
            return 1;
        }
        if (DiplomacyProposalMode == 3) {
            return 0;
        }
        if (!(PlayersData[otherFac].diplo_treaties[myFac] & 0x20) && DiploScratchFA78 <= 2
            && !DiploScratchFA2C) {
            if (!great_satan(myFac, 0) && !DiploScratchFA3C) {
                if (PlayersData[otherFac].ranking <= PlayersData[myFac].ranking
                    && DiploFriction <= 10) {
                    parse_says(3, get_noun_phrase(otherFac, 9), -1, -1);
                    X_pops("REJPACT", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
                    return 1;
                }
                parse_says(3, get_noun_phrase(otherFac, 9), -1, -1);
                X_pops("REJPACTPOWER", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
                return 1;
            }
            parse_says(3, get_phrase(myFac, 3), -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, "REJPACTAHEAD");
            _itoa(random(0, 2), number, 10);
            strcat(StringTemp, number);
            X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
            return 1;
        }
        parse_says(3, get_phrase(myFac, 5), -1, -1);
        X_pops("REJPACTHATE", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
        PlayersData[otherFac].diplo_patience[myFac]++;
        return 1;
    }
    int other_commerce = PlayersData[otherFac].tech_commerce_bonus;
    int energy = PlayersData[myFac].energy_reserves;
    int cap = (energy + (PlayersData[myFac].tech_commerce_bonus - other_commerce + DiploScratchFA78) * 50)
        * PlayersData[otherFac].mil_strength_1
        / (PlayersData[myFac].mil_strength_1 + PlayersData[otherFac].mil_strength_1 + 1) / 50 * 50;
    if (DiplomacyProposalMode != 5) {
        while (cap && cap > energy) {
            cap = range(cap - 50, 0, 99999);
        }
    }
    int want_help = 0;
    score = 0;
    int accepted_offer = 0;
    int offer_made = 0;
    if (!strength_gap
        && ((DiploFriction > 4 && PlayersData[myFac].tech_commerce_bonus >= other_commerce)
            || DiploFriction > 10
            || (DiploScratchFA78 && DiploScratchFA78 > other_commerce - PlayersData[myFac].tech_commerce_bonus))) {
        score = 1;
    }
    if (DiplomacyProposalMode == 4 || DiplomacyProposalMode == 5) {
        want_help = 1;
    }
    if (DiplomacyProposalMode == 2) {
        if (!strength_gap && !DiploScratchF7E8 && (DiploScratchFAA8 >= 0 || cap)) {
            want_help = 1;
        }
        if (DiploScratchFAA8 < 0 && !cap) {
            score = 0;
        }
    }
    if (great_satan(myFac, 0) || DiploScratchFA54 > 1
        || (IsMultiplayerNet && myFac == (int)GameDataStateStorage.fields_.ranking_faction_id_unk1_
            && (DiploScratchFA54 || PlayersData[otherFac].ranking
                > PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].ranking))) {
        cap = (cap * 2 > GameDataStateStorage.fields_.turn_current_num_ * 5
            ? cap * 2 : GameDataStateStorage.fields_.turn_current_num_ * 5) / 50 * 50;
        if ((PlayersData[myFac].energy_reserves < cap && (DiplomacyProposalMode == 5 || DiploScratchFA18 < 0))
            || (PlayersData[myFac].energy_reserves >= cap && DiploScratchFA18 < 0 && DiplomacyProposalMode == 4)) {
            parse_says(3, get_noun_phrase(otherFac, 9), -1, -1);
            X_pops("REJPACT", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
            return 1;
        }
    } else {
        DiploScratchFA18 = -1;
    }
    if ((score || want_help) && DiploScratchFAA8 >= 0 && strcmp(Players[otherFac].filename, "BELIEVE")
        && (strcmp(Players[otherFac].filename, "MORGAN") || !cap || PlayersData[myFac].energy_reserves < cap)
        && (DiplomacyProposalMode == 4 || (DiplomacyProposalMode == 1 && DiploFriction < 4)
            || DiplomacyProposalMode == 2)) {
        offer_made = 1;
        int response;
        for (;;) {
            parse_set(Players[otherFac].is_leader_female, 0);
            parse_says(1, Players[otherFac].title_leader, -1, -1);
            StringTemp[0] = '\0';
            say_tech(StringTemp, DiploScratchFAA8, 1);
            parse_says(2, StringTemp, -1, -1);
            parse_says(3, get_noun_phrase(otherFac, 9), -1, -1);
            if (DiploScratchFA18 >= 0) {
                StringTemp[0] = '\0';
                say_tech(StringTemp, DiploScratchFA18, 1);
            }
            StringTemp[0] = '\0';
            strcat(StringTemp, "GIMMETECH");
            _itoa(DiploScratchFA18 >= 0 ? 2 : random(0, 2), number, 10);
            strcat(StringTemp, number);
            response = X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
            if (response <= 1) {
                break;
            }
            help_topic(14, DiploScratchFAA8);
        }
        if (response) {
            accepted_offer = 1;
            int tech_id = DiploScratchFAA8;
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingTECH, otherFac, tech_id, myFac);
                message_data(0x244b, 0, otherFac, tech_id, myFac, 0);
                NetDaemonNet.await_diplo(0x44b);
            } else {
                tech_achieved(otherFac, tech_id, myFac, 0);
            }
            if (DiploScratchFA18 >= 0) {
                tech_id = DiploScratchFA18;
                if (IsMultiplayerNet) {
                    log_say(DiploLabelSendingTECH, otherFac, tech_id, myFac);
                    message_data(0x244b, 0, otherFac, tech_id, myFac, 0);
                    NetDaemonNet.await_diplo(0x44b);
                } else {
                    tech_achieved(otherFac, tech_id, myFac, 0);
                }
            }
            int friction_delta = -(tech_val(DiploScratchFAA8, otherFac, 1) / 2);
            PlayersData[otherFac].diplo_friction[myFac]
                = range(friction_delta + PlayersData[otherFac].diplo_friction[myFac], 0, 20);
            if (DiploFrictionFactionID == (uint32_t)otherFac
                && DiploFrictionFactionIDWith == (uint32_t)myFac) {
                DiploFriction += friction_delta;
            }
            diplomacy_caption(myFac, otherFac);
            DiploScratchFAA8 = DiploScratchFA18 = -1;
        }
    }
    if ((score || want_help) && cap && PlayersData[myFac].energy_reserves >= cap && !offer_made
        && strcmp(Players[otherFac].filename, "PEACE")
        && (DiplomacyProposalMode == 5 || (DiplomacyProposalMode == 1 && DiploFriction < 4)
            || DiplomacyProposalMode == 2)) {
        offer_made = 1;
        parse_num(0, cap);
        parse_set(Players[otherFac].is_leader_female, 0);
        parse_says(1, Players[otherFac].title_leader, -1, -1);
        StringTemp[0] = '\0';
        strcat(StringTemp, "GIMMEENERGY");
        _itoa(random(0, 2), number, 10);
        strcat(StringTemp, number);
        if (X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait)) {
            int energy_debit = -cap;
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingENERGY, myFac, otherFac, energy_debit);
                message_data(0x2447, 0, myFac, energy_debit, otherFac, cap);
                NetDaemonNet.await_diplo(0x447);
            } else {
                if (myFac) {
                    PlayersData[myFac].energy_reserves += energy_debit;
                }
                if (otherFac) {
                    PlayersData[otherFac].energy_reserves += cap;
                }
                ConsoleGlobal.update_data(1);
            }
            want_help = 0;
            int friction_units = 0;
            accepted_offer = 1;
            int divisor = 2;
            int remaining = cap / 5;
            int chunk = 10;
            while (remaining > 0) {
                friction_units += range(remaining, 0, chunk) / divisor;
                remaining -= chunk;
                chunk = 20;
                divisor++;
            }
            int friction_delta = (friction_units + 4) / -5;
            PlayersData[otherFac].diplo_friction[myFac]
                = range(friction_delta + PlayersData[otherFac].diplo_friction[myFac], 0, 20);
            if (DiploFrictionFactionID == (uint32_t)otherFac
                && DiploFrictionFactionIDWith == (uint32_t)myFac) {
                DiploFriction += friction_delta;
            }
            ConsoleInfoWin.set_redraw_phase(0);
            ConsoleGlobal.update_data(0);
            diplomacy_caption(myFac, otherFac);
        }
    }
    if ((score || offer_made) && !accepted_offer) {
        if (!offer_made) {
            if (DiplomacyProposalMode == 3) {
                return 0;
            }
            X_pops("REJPACTBADBRIBE", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
            return 1;
        }
        X_pops("REJPACTNOBRIBE", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
        return 1;
    }
    if (partner_id && !DiploScratchFA8C) {
        parse_set(Players[myFac].is_leader_female, 0);
        parse_says(5, Players[myFac].title_leader, -1, -1);
        parse_set(Players[partner_id].noun_gender, Players[partner_id].is_noun_plural);
        parse_says(6, Players[partner_id].noun_faction, -1, -1);
        if (!X_pops("GIMMEHELP", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait)) {
            if (strength_gap
                || ((int)GameDataStateStorage.fields_.faction_rankings_[7] == myFac
                    && DiplomacyProposalGateCount >= 0)
                || DiploScratchFA78 > 2 || DiplomacyProposalGateCount > 0) {
                PlayersData[otherFac].diplo_patience[myFac]++;
                if (!accepted_offer) {
                    parse_says(2, get_pacts(myFac, otherFac), -1, -1);
                    X_pops("REJPACTNOHELP", 0x100000, FactionDiplomacyPortraits[otherFac], pop_wait);
                    return 1;
                }
            }
        } else {
            log_say(DiploLabelGettingDIPLOMATICLOCK, 0, 0, 0);
            timeGetTime();
            while (!ExitTurnLoop && NetDaemonNet.add_global()) {
                check_spock();
                timeGetTime();
            }
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingDOUBLECROSS, myFac, partner_id, otherFac);
                message_data(0x2440, 0, myFac, partner_id, otherFac, 0);
            } else {
                double_cross(myFac, partner_id, otherFac);
            }
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingTREATYOFF, myFac, otherFac, 0x800000);
                message_data(0x2442, 0, myFac, otherFac, 0x800000, 0);
            } else {
                treaty_off(myFac, otherFac, 0x800000);
            }
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingSETTREATY, otherFac, partner_id, 0x40);
                message_data(0x2443, 0, otherFac, partner_id, 0x40, 1);
                NetDaemonNet.await_diplo(0x443);
            } else {
                set_treaty(otherFac, partner_id, 0x40, 1);
            }
            log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
            NetDaemonNet.unlock();
            DiploScratchF7E8 = 0;
            DiploScratchFA4C = 0;
        }
    }
    pact_of_brotherhood(myFac, otherFac);
    return 1;
}

// ORIGINAL: 0x00558C60 ?commlink_attempt@@YAXH@Z 0x00558C60-0x00558FB4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:multiwin-contact-owner", "historical seed:src/recovered/units/00558c60.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete original mapped span and native call/branch order", "native:478500 subtracts item ID1; jump-table indices7..10 select items8renounce,9withdrawal,10contact,11chat; withdrawal arguments local then clicked", "source:existing typed subobjects, faction/player/parser/network and separate bounded contact-state storage; automatic PullDown/Buffer lifetime"], "reason": "Recover the complete original in its genuine MultiWin or diplomacy C++ owner, retaining identity, auxiliary spans and historical seed findings; no current match or runtime claim."}
void __cdecl commlink_attempt(int target_faction) {
    int local_faction = ConsoleGlobal.faction_id_;
    if (local_faction == target_faction) {
        return;
    }
    int target_bit = 1 << target_faction;
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & target_bit)) {
        return;
    }
    if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
        NetMsgPrimary.pop("SUNSPOTSNO", 5000, 0, NULL);
        return;
    }
    if (Players[local_faction].rule_flags & RFLAG_ALIEN && Players[target_faction].rule_flags & RFLAG_ALIEN) {
        GenderDefault = Players[local_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[local_faction].title_leader, -1, -1);
        GenderDefault = Players[local_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(3, Players[local_faction].name_leader, -1, -1);
        X_pops("ALIENHATRED2", 0x100000, FactionDiplomacyPortraits[target_faction], pop_wait);
        return;
    }
    if (Players[local_faction].rule_flags & RFLAG_ALIEN && !has_tech(0x4E, target_faction) && !has_tech(4, local_faction)) {
        NetMsgPrimary.pop("WENEEDHUMANPSYCH", -5000, 0, "Alopdir.pcx");
        return;
    }
    if (Players[target_faction].rule_flags & RFLAG_ALIEN
        && !has_tech(0x4E, local_faction) && !has_tech(4, target_faction)) {
        NetMsgPrimary.pop("WENEEDALIENPSYCH", -5000, 0, "talent_sm.pcx");
        return;
    }
    if (commlink_attempter(local_faction, target_faction)) {
        return;
    }
    if (IsMultiplayerNet) {
        if ((uint8_t)target_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) {
            CommlinkPending = 1;
            CommlinkFaction = target_faction;
            message_data(0x1501, target_faction, local_faction, target_faction, 0, 0);
            GenderDefault = Players[target_faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[target_faction].title_leader, -1, -1);
            GenderDefault = Players[target_faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[target_faction].name_leader, -1, -1);
            parse_num(0, target_faction);
            NetMsgPrimary.pop("DIPLOCOMM", 5000, 0, NULL);
            return;
        }
        log_say(const_cast<char *>("Commlink open channel"), local_faction, target_faction, 0);
        if (NetDaemonNet.open_channel(local_faction, target_faction)) {
            GenderDefault = Players[target_faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[target_faction].title_leader, -1, -1);
            GenderDefault = Players[target_faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[target_faction].name_leader, -1, -1);
            NetMsgPrimary.pop("NOCHANNEL", 5000, 0, NULL);
            return;
        }
    }
    if (IsMultiplayerPBEM && (uint8_t)target_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) {
        diplo(local_faction, target_faction);
        return;
    }
    communicate(local_faction, target_faction, 1);
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTREATYON, local_faction, target_faction, 0x88000008);
        message_data(0x2441, 0, local_faction, target_faction, 0x88000008, 0);
        NetDaemonNet.await_diplo(0x441);
    } else {
        treaty_on(local_faction, target_faction, 0x88000008);
    }
}

/*
Purpose: Wait out the diplomatic lock: announce it, clamp the caller's
         patience to ten seconds, and spin on the daemon's global-lock
         request - pumping spock checks and re-asking NEEDGLOBAL after
         each ten-second/count timeout - until the lock lands or the turn
         loop is told to exit.

// ORIGINAL: 0x00539820 ?diplo_lock@@YAHH@Z 0x00539820-0x005398BF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x539820..0x5398BF and original callers across faction, council, report and windows", "source:src/faction.cpp diplo_unlock owns the paired genuine NetDaemonNet unlock operation", "source:src/faction.h already declares diplo_lock; diplo_unlock remains unchanged"], "reason": "source-owned faction network diplomacy lock"}
// size      159 bytes
// prototype int (__cdecl ?diplo_lock@@YAHH@Z)(int a1)
// callers   ?   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0051F1B0 0x00530AA0 0x005BF310 0x006262F0
Return Value: 1 when the caller must back off (NEEDGLOBAL refused), else 0
Status: Coverage transcription - claim pending re-measurement
*/
int __cdecl diplo_lock(int patience) {
    int retry_count = 0;
    log_say(DiploLabelGettingDIPLOMATICLOCK, 0, 0, 0);
    if (patience != 0 && patience < 100) {
        patience = 100;
    }
    DWORD(WINAPI *const get_time)() = timeGetTime;
    DWORD start_time = get_time();
    while (ExitTurnLoop == 0 && NetDaemonNet.add_global() != 0) {
        check_spock();
        const DWORD now = get_time();
        if (patience != 0
            && (now - start_time >= 10000
                || (++retry_count, retry_count >= 10000))) {
            if (X_pop(DiploLabelNEEDGLOBAL, 0) != 0) {
                return 1;
            }
            retry_count = 0;
            start_time = get_time();
        }
    }
    return 0;
}

// ORIGINAL: 0x0053E5C0 ?renounce_pact@@YAXHH@Z 0x0053E5C0-0x0053E7F8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:multiwin-contact-owner", "historical seed:src/recovered/units/0053e5c0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete original mapped span and native call/branch order", "native:478500 subtracts item ID1; jump-table indices7..10 select items8renounce,9withdrawal,10contact,11chat; withdrawal arguments local then clicked", "source:existing typed subobjects, faction/player/parser/network and separate bounded contact-state storage; automatic PullDown/Buffer lifetime"], "reason": "Recover the complete original in its genuine MultiWin or diplomacy C++ owner, retaining identity, auxiliary spans and historical seed findings; no current match or runtime claim."}
void __cdecl renounce_pact(int faction_id, int faction_id_with) {
    PopupDialogFactionID = faction_id_with;
    if (commlink_attempter(faction_id, faction_id_with) != 0) return;

    // The image inlines diplo_lock here; any nonzero patience folds the same.
    if (diplo_lock(0x32) != 0) return;

    PlayersData[faction_id].integrity_blemishes =
        diplo_add(PlayersData[faction_id].integrity_blemishes, 1);

    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYOFF, faction_id_with, faction_id, 1);
        message_data(0x2442, 0, faction_id_with, faction_id, 1, 0);
        NetDaemonNet.await_diplo(0x442);
    } else {
        treaty_off(faction_id_with, faction_id, 1);
    }

    if (GameDataStateStorage.fields_.sunspot_duration_ <= 0 && !is_human(static_cast<uint32_t>(faction_id_with))) {
        diplomacy_check(faction_id, faction_id_with, 0);
        diplomacy_caption(faction_id, faction_id_with);

        parse_says(0, get_pact(faction_id), -1, -1);

        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction_id].title_leader, -1, -1);

        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(2, Players[faction_id].name_leader, -1, -1);

        X_pops("BROKEPACT", 0x100000,
               FactionDiplomacyPortraits[faction_id_with], pop_wait);
    }

    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingPACTENDS, 0, 0, 0);
        message_data(0x244D, 0, faction_id, faction_id_with, 0, 0);
        NetDaemonNet.await_diplo(0x44D);
    } else {
        pact_ends(faction_id, faction_id_with);
    }

    log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
    NetDaemonNet.unlock();
}



static void battle_plans_apply_selected_target(int faction_id,
                                                  int selected_base_id) {
    PlayerData &planner = PlayersData[faction_id];
    const Base &selected_base = Bases[selected_base_id];
    if (planner.x_target < 0) {
        return;
    }

    const Map &planned_tile = *map_loc(planner.x_target, planner.y_target);
    const Map &selected_tile = *map_loc(selected_base.x, selected_base.y);
    if (planned_tile.region == selected_tile.region) {
        return;
    }

    int vehicle_id;
    for (vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
        Veh &vehicle = Vehs[vehicle_id];
        if (vehicle.faction_id == faction_id
            && (vehicle.state & VSTATE_UNK_40000) != 0) {
            vehicle.state &= ~(VSTATE_UNK_20000 | VSTATE_UNK_40000);
        }
    }
    planner.y_target = -1;
    planner.x_target = -1;
    planner.unk_72 = 0;
    planner.unk_71 = 0;
}

// ORIGINAL: 0x0054B1C0 ?battle_plans@@YAXHH@Z 0x0054B1C0-0x0054BEBE;0x0065F662-0x0065F8FC
// RECOVERY_BINDING: v1 {"evidence": ["image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x0054B1C0:0054B1C0-0054BEBE,0065F662-0065F8FC", "source:src/planwin.h:PlanWin and genuine MapWin/GraphicWin interfaces", "source:src/faction.h:PlayerData, battle faction state and label declarations", "recovery/tasks/planwin-battle-owner-coverage.json"], "reason": "Use the pinned native body with the real typed owner and preserve the seed's original identity and history."}
// TRIED: same Popup-teardown-cascade shape as PickWin (mechanical EBP(x) buffer transcription, reused from that pattern). The post-BasePop::exec block (0x54B978-0x54BA25, writing g_0096cdd0/cdd4/da54/da58) is OMITTED - falls straight to the close@PlanWin cascade instead of computing those derived fields, since the base_find/tile-ownership arithmetic there wasn't independently re-verified in the time available. sim 0.18-0.61 across flag sets.
// The TRIED note describes the former seed body; target cleanup is expressed
// in battle_plans_apply_selected_target. Pinned 54B8F1..54B938 queries plan_b
// before the selected base, then applies signed JL after 32-bit LEA doubling.
void __cdecl battle_plans(int faction_a, int faction_b) {
    Popup popup;

    PopupDialogFactionID = faction_b;
    PlanBattleFactionA = faction_a;
    PlanBattleFactionB = faction_b;

    const int plan_a = suggest_plan(faction_a, faction_b);
    const int plan_b = suggest_plan(faction_b, faction_a);
    const int attack_a = attack_from(plan_a, faction_a);
    const int attack_b = attack_from(plan_b, faction_b);

    if ((PlayersData[faction_a].diplo_treaties[faction_b] & 1u) == 0) {
        if ((PlayersData[faction_b].diplo_treaties[faction_a] & 0x8020u) != 0
            || DiploScratchFA2C != 0 || DiploFriction > 0x11
            || DiploScratchFA60 != 0) {
            X_pops(BattlePlansNoActionLabel, 0x100000,
                         FactionDiplomacyPortraits[faction_b], pop_wait);
            return;
        }

        if (DiploScratchFA48 == 0
            && X_pops(BattlePlansMapSharingLabel, 0x100000,
                            FactionDiplomacyPortraits[faction_b], pop_wait)
                   == 0) {
            return;
        }

        if (IsMultiplayerNet == 0) {
            trade_maps(faction_a, faction_b);
        } else {
            log_say(DiploLabelSendingTRADEMAPS, faction_a, faction_b, 0);
            message_data(0x2448, 0, faction_a, faction_b, 0, 0);
            NetDaemonNet.await_diplo(0x448);
        }
    }

    // The native gate checks both suggested plans, not the attack_from result.
    if (plan_a < 0 || plan_b < 0) {
        X_pops(BattlePlansInvalidPlanLabel, 0x100000,
                     FactionDiplomacyPortraits[faction_b], pop_wait);
        return;
    }

    PlanWindow.init(0);
    PlanWindow.find_view(Bases[plan_a].x, Bases[plan_a].y,
                         Bases[plan_b].x, Bases[plan_b].y,
                         Bases[attack_a].x, Bases[attack_a].y,
                         Bases[attack_b].x, Bases[attack_b].y);
    PlanWindow.back_redraw();
    PlanWindow.add_line(attack_a, plan_a,
                        InfoFactionTextColors[faction_a]);
    PlanWindow.add_line(attack_b, plan_b,
                        InfoFactionTextColors[faction_b]);
    PlanWindow.show(0);

    PlayersData[faction_b].base_id_atk_target = plan_b;
    parse_says(0, Bases[plan_b].name_string, -1, -1);
    parse_says(1, Bases[plan_a].name_string, -1, -1);

    if (X_pops(BattlePlansLabel, 0x100000,
                     FactionDiplomacyPortraits[faction_a], pop_wait) != 0) {
        PluralityDefault = 0;
        GenderDefault = Players[faction_a].is_leader_female;
        parse_says(0, Players[faction_a].title_leader, -1, -1);
        GenderDefault = Players[faction_a].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction_a].name_leader, -1, -1);

        popup.start(PopupStartCaption, BattlePlanLabel, -1, 0,
                    0x100042, 0);
        popup.dialogs().sprite(FactionDiplomacyPortraits[faction_a], 0,
                               faction_b);

        int first_candidate_added = 0;
        int base_id;
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            Base &candidate = Bases[base_id];
            const int owner = candidate.faction_id_current;
            if (owner != faction_a && owner != faction_b
                && (PlayersData[faction_a].diplo_treaties[owner] & 0x10u) != 0
                && (PlayersData[owner].diplo_treaties[faction_b] & 0x10u) != 0
                && ((candidate.visibility & (1u << faction_a)) != 0
                    || (candidate.visibility & (1u << faction_b)) != 0)) {
                popup.dialogs().item(candidate.name_string, base_id);

                if (first_candidate_added == 0) {
                    const int candidate_attack_a =
                        attack_from(base_id, PlanBattleFactionA);
                    const int candidate_attack_b =
                        attack_from(base_id, PlanBattleFactionB);
                    PlanWindow.clear_lines();
                    PlanWindow.add_line(candidate_attack_a, base_id,
                        InfoFactionTextColors[PlanBattleFactionA]);
                    PlanWindow.add_line(candidate_attack_b, base_id,
                        InfoFactionTextColors[PlanBattleFactionB]);
                    PlanWindow.on_redraw();
                    first_candidate_added = 1;
                }
            }
        }

        if (first_candidate_added != 0) {
            const int selected_base_id = popup.exec(0, pop_wait);
            if (selected_base_id >= 0) {
                const Base &selected_base = Bases[selected_base_id];
                parse_says(0, selected_base.name_string, -1, -1);

                if (selected_base.faction_id_current != faction_b
                    && (selected_base.visibility
                        & (1u << faction_b)) == 0) {
                    spot_loc(selected_base.x, selected_base.y, faction_b);
                }

                const int gate_result =
                    battle_plans_selected_region_distance_result(
                        faction_b, selected_base, Bases[plan_b]);
                if (gate_result == 1) {
                    X_pops(BattlePlansRegionRejectedLabel, 0x100000,
                                 FactionDiplomacyPortraits[faction_b],
                                 pop_wait);
                } else if (gate_result == 2) {
                    X_pops(BattlePlansDistanceRejectedLabel,
                                 0x100000,
                                 FactionDiplomacyPortraits[faction_b],
                                 pop_wait);
                } else {
                    PlayersData[faction_b].base_id_atk_target =
                        selected_base_id;
                    X_pops(BattlePlansSelectionLabel, 0x100000,
                                 FactionDiplomacyPortraits[faction_b],
                                 pop_wait);
                    battle_plans_apply_selected_target(faction_b,
                                                          selected_base_id);
                }
            }
        }
    }

    PlanWindow.close();
}

// ORIGINAL: 0x00545D60 ?propose_trade_maps@@YAXHH@Z 0x00545D60-0x00545E80 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native instructions at 0x00545D60..0x00545E80 distinguish actor treaty lookup, target patience-byte update and local/network map-transfer paths.","source:src/faction.h PlayerData treaty/patience arrays and typed diplomacy/map APIs.","Pinned 0x00545DAB supplies Players base 0x00946A50 plus member offset 0x304 to parse_says: Player::adj_name_faction, not noun_faction at 0x2E4."],"reason":"Recover the trade-map proposal using the actor treaty row, target-owned patience, original adjective-name member and existing transfer APIs; preserve original role orientation and source-owned string literal behavior."}
void __cdecl propose_trade_maps(int faction_id, int faction_id_with) {
    PopupDialogFactionID = faction_id_with;

    const int has_prior_map_trade =
        (PlayersData[faction_id].diplo_treaties[faction_id_with] & 1) != 0;
    if ((!has_prior_map_trade && DiploFriction > 0) ||
        DiploFriction > 4) {
        parse_says(0, Players[faction_id_with].adj_name_faction, -1, -1);
// Map-trade refusal and proposal use separate script topics.
// Bounds cover the literal payload, not surrounding allocation or lifetime.
// RECOVERY_LITERAL: v1 0x0068DA60 0x0068DA68 ??_C@_07NILC@REJMAPS?$AA@ 0x00545D60
        X_pops("REJMAPS", 0x100000,
                     FactionDiplomacyPortraits[faction_id_with], pop_wait);
        return;
    }

// RECOVERY_LITERAL: v1 0x0068DA68 0x0068DA72 ??_C@_09BAJO@TRADEMAPS?$AA@ 0x00545D60
    X_pops("TRADEMAPS", 0x100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait);

    // Native performs an unsigned byte increment at PlayerData + 0xB8.
    // diplo_patience is the source-owned byte array at that displacement.
    PlayersData[faction_id_with].diplo_patience[faction_id] =
        static_cast<char>(static_cast<unsigned char>(
            PlayersData[faction_id_with].diplo_patience[faction_id]) + 2u);

    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTRADEMAPS, faction_id, faction_id_with, 0);
        message_data(0x2448, 0, faction_id, faction_id_with, 0, 0);
        NetDaemonNet.await_diplo(0x448);
        DiplomacyMapTransferChanged = 1;
        return;
    }

    trade_maps(faction_id, faction_id_with);
    DiplomacyMapTransferChanged = 1;
    return;
}
// ORIGINAL: 0x0053E4C0 ?demands_withdrawal@@YAXHH@Z 0x0053E4C0-0x0053E5B6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native instructions at 0x0053E4C0..0x0053E5B6: commlink callback precedes multiplayer human gate, demand result and perihelion dialogue.","source:src/faction.h and source:src/netdaemon.h typed dependency declarations."],"reason":"Recover the withdrawal-demand caller with the original callback/gate order and typed result topics; retain the original extent while its two callees are recovered as complete neighboring bodies."}
// TRIED: `(byte)(1 << (a2 & 0x1f))` (explicit mask before the shift, mnemonic similarity 0.896, 8 edit blocks - the mask compiles to a spurious `and` the original does not have, since `shl` already masks its count to 5 bits at the hardware level) vs. plain `(byte)(1 << a2)` (0.945, 2 edit blocks). Also: if/else-if chain over the 0/1/2 result vs. `switch` - the original lowers the three-way dispatch as a decrement chain (`sub eax,0 / je / dec / je / dec / jne`), which only the `switch` form reproduces. Landed the switch form. Neither reached BYTE_EXACT; first remaining divergence is at the record-base multiply by 0x59c (#35, `shl` vs `mov`).
void __cdecl demands_withdrawal(int faction_id, int faction_id_with) {
    PopupDialogFactionID = faction_id_with;
    if (commlink_attempter(faction_id, faction_id_with) != 0) {
        return;
    }
    if (IsMultiplayerNet != 0 &&
        (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1u << faction_id_with)) != 0) {
        return;
    }

    const int response = demand_withdrawal(faction_id, faction_id_with);
    if (GameDataStateStorage.fields_.sunspot_duration_ <= 0) {
        return;
    }

    GenderDefault = Players[faction_id_with].noun_gender;
    PluralityDefault = Players[faction_id_with].is_noun_plural;
    parse_says(0, Players[faction_id_with].noun_faction, -1, -1);

    switch (response) {
    case 0:
// RECOVERY_LITERAL: v1 0x0068D68C 0x0068D697 ??_C@_0L@OMKE@WITHSUNYES?$AA@ 0x0053E4C0
        NetMsgPrimary.pop("WITHSUNYES", 5000, 0, 0);
        break;
    case 1:
// RECOVERY_LITERAL: v1 0x0068D680 0x0068D68A ??_C@_09KHMJ@WITHSUNNO?$AA@ 0x0053E4C0
        NetMsgPrimary.pop("WITHSUNNO", 5000, 0, 0);
        break;
    case 2:
// The withdrawal response selects the war, refusal or acceptance script topic.
// Bounds cover the literal payload, not surrounding allocation or lifetime.
// RECOVERY_LITERAL: v1 0x0068D674 0x0068D67F ??_C@_0L@GAFL@WITHSUNWAR?$AA@ 0x0053E4C0
        NetMsgPrimary.pop("WITHSUNWAR", 5000, 0, 0);
        break;
    default:
        break;
    }
}
// ORIGINAL: 0x005589E0 ?commlink_attempter@@YAHHH@Z 0x005589E0-0x00558C55 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native instructions at 0x005589E0..0x00558C55: vendetta parser fields, key-state route, network waiting mask, CommlinkPending/CommlinkFaction selection and final channel-2 latch.","source:src/netdaemon.h CommlinkFaction/CommlinkPending and NetDaemonNet.lock_state_[2].payload_.flags; source:src/diplowin.h DiploWindow."],"reason":"Recover the complete commlink response gate in faction.cpp through typed player, daemon, console and DiploWindow owners; keep selected-faction reloads after parser callbacks and preserve the original seed history."}
// TRIED: straightforward translation matches from instr #3 on (prologue push/frame-pointer order); did not chase further given the three parse_says(chan, ptr, -1, -1) call sites and the shared 9bbfec/9bbff0 globals already reproduce the control flow.
int __cdecl commlink_attempter(int faction_id, int contact_flags) {
    PopupDialogFactionID = contact_flags;
    if ((contact_flags & DTREATY_VENDETTA) != 0) {
        contact_flags &= ~DTREATY_VENDETTA;
        GenderDefault = Players[contact_flags].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[contact_flags].title_leader, -1, -1);
        GenderDefault = Players[contact_flags].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[contact_flags].name_leader, -1, -1);
        GenderDefault = Players[contact_flags].noun_gender;
        PluralityDefault = Players[contact_flags].is_noun_plural;
        parse_says(2, Players[contact_flags].noun_faction, -1, -1);
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << contact_flags)) == 0) {
            NetMsgPrimary.pop("ERADICATED", 5000, 0, NULL);
            return 1;
        }
        NetMsgPrimary.pop("NOTMETYET", 5000, 0, NULL);
        return 1;
    }

    if ((HIBYTE(GetAsyncKeyState(VK_CONTROL)) != 0 ||
         HIBYTE(GetAsyncKeyState(VK_SHIFT)) != 0) &&
        (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
        diplo(faction_id, contact_flags);
        return 0;
    }

    const int local_faction = ConsoleGlobal.faction_id_;
    if (IsMultiplayerNet != 0 && (GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) == 0 &&
        (NetDaemonNet.factions_waiting_mask() & (1u << local_faction)) != 0) {
        NetMsgPrimary.pop("ALREADYDONE", 5000, 0, NULL);
        return 1;
    }

    if (CommlinkPending != 0) {
        GenderDefault = Players[CommlinkFaction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[CommlinkFaction].title_leader, -1, -1);
        GenderDefault = Players[CommlinkFaction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[CommlinkFaction].name_leader, -1, -1);
        const int response = X_pop("DIPLOALREADY", NULL);
        if (response == 0) return 1;
        message_data(0x1505, CommlinkFaction, 0, 0, 0, 0);
        DiploWindow.close();
        if (response > 1) return 1;
    }

    if (IsMultiplayerNet != 0 &&
        (ConsoleControlTurnActive != 0 ||
         NetDaemonNet.channels_[1].payload_.flags != 0))
        return 1;
    return 0;
}
// ORIGINAL: 0x00549270 ?demand_withdrawal@@YAHHH@Z 0x00549270-0x00549923 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native instructions at 0x00549270..0x00549923: both directional treaty-table walks, diplo_spoke writes, friction gates, popup returns, territory and withdrawal calls.","source:src/faction.h PlayerData/diplomacy scratch storage and typed diplomacy APIs; source:src/game.h GameRules/SunspotDuration.","Pinned native 0x0054935B/0x00549363 and 0x0054958C/0x0054959B load 0x9A64D4 (TurnCurrentNum) before both diplo_spoke timestamp stores.","Pinned native 0x005495F1 increments DiploFriction; use the existing faction.cpp diplo_add helper for native signed 32-bit wrap without C++ signed-overflow UB.","Pinned native 0x005496C6..0x00549709 confirms accepted territory routes perform the reversed callback then converge through the ordinary withdrawal path; declined popup uses only the ordinary path."],"reason":"Recover the complete withdrawal response in the faction owner with direction-correct typed status storage, original callback ordering, and native signed wrap/clamp behavior; preserve seed history without asserting byte or game acceptance. Both diplo_spoke timestamps use live TurnCurrentNum, and friction increments use diplo_add for native wrap. Accepted territory response intentionally performs reversed then ordinary withdrawals; rejection performs only the ordinary withdrawal."}
// TRIED: MISMATCH #4 'push' vs 'mov' (framed build). Full 1715-byte transcription of the treaty-offer/double-cross/territory-demand/ withdrawal state machine faithfully following the disassembly's argument orders (several calls swap (a1,a2) vs (a2,a1) between branches - transcribed as observed, not assumed symmetric). Register-scheduling divergence only; did not chase further.
int __cdecl demand_withdrawal(int faction_id, int against_id) {
    int territory_notice = 0;
    PopupDialogFactionID = against_id;
    diplomacy_caption(faction_id, against_id);
    if (!(PlayersData[against_id].diplo_treaties[faction_id] & 0x200)
        && !(PlayersData[against_id].diplo_treaties[faction_id] & DTREATY_PACT)
        && (DiploScratchFA70 || DiploScratchFAA0
            || (DiploFriction > (DiploScratchFA68 ? 5 : 15) && !DiploScratchFA98))) {
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYON, against_id, faction_id, 0x10);
            message_data(0x2441, 0, against_id, faction_id, 0x10, 0);
            NetDaemonNet.await_diplo(0x441);
        } else {
            treaty_on(against_id, faction_id, DTREATY_VENDETTA);
        }
        PlayersData[faction_id].diplo_spoke[against_id] = GameDataStateStorage.fields_.turn_current_num_;
        if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
            return 2;
        }
        diplomacy_caption(faction_id, against_id);
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(1, Players[faction_id].name_leader, -1, -1);
        StringTemp[0] = '\0';
        strcat(StringTemp, "NEVER");
        X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[against_id], pop_wait);
        return 2;
    }
    if (!(PlayersData[faction_id].diplo_treaties[against_id] & DTREATY_PACT)) {
        int withdrawal_target = dont_withdrawal(faction_id, against_id);
        if (withdrawal_target && PlayersData[faction_id].diplo_treaties[against_id] & DTREATY_TREATY) {
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingDOUBLECROSS, against_id, faction_id, -1);
                message_data(0x2440, 0, against_id, faction_id, -1, 0);
                NetDaemonNet.await_diplo(0x440);
            } else {
                double_cross(against_id, faction_id, -1);
            }
            if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
                return 2;
            }
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            parse_set(Players[withdrawal_target].is_leader_female, 0);
            parse_says(2, Players[withdrawal_target].title_leader, -1, -1);
            parse_set(Players[withdrawal_target].is_leader_female, 0);
            parse_says(3, Players[withdrawal_target].name_leader, -1, -1);
            X_pops("RUNNINGDOG", 0x100000, FactionDiplomacyPortraits[against_id], pop_wait);
            DiploScratchFA70 = 0;
            PlayersData[faction_id].diplo_spoke[against_id] = GameDataStateStorage.fields_.turn_current_num_;
            return 2;
        }
    }
    if (!(PlayersData[faction_id].diplo_treaties[against_id] & DTREATY_TREATY)) {
        PlayersData[against_id].diplo_friction[faction_id]
            = range(PlayersData[against_id].diplo_friction[faction_id] + 1, 0, 20);
        if (DiploFrictionFactionID == (uint32_t)against_id
            && DiploFrictionFactionIDWith == (uint32_t)faction_id) {
            DiploFriction++;
        }
        if (PlayersData[against_id].diplo_treaties[faction_id] & 0x20 || DiploScratchFA70
            || DiploScratchFAA0 || DiploScratchFA78 > DiploScratchFA98 + 3
            || (DiploScratchFA68 && !DiploScratchFA98)) {
            if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
                return 1;
            }
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            X_pops("NOWAYJOSE", 0x100000, FactionDiplomacyPortraits[against_id], pop_wait);
            return 1;
        }
        if (territory(against_id, faction_id, 0, NULL, NULL)) {
            if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
                return 1;
            }
            territory_notice = 1;
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            if (X_pops("TURNABOUT", 0x100000, FactionDiplomacyPortraits[against_id], pop_wait)) {
                if (IsMultiplayerNet) {
                    log_say(DiploLabelSendingWITHDRAWAL, against_id, faction_id, 0);
                    message_data(0x244a, 0, against_id, faction_id, 0, 0);
                } else {
                    do_withdrawal(against_id, faction_id);
                }
            }
        }
    }
    int result;
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingWITHDRAWAL, faction_id, against_id, 0);
        message_data(0x244a, 0, faction_id, against_id, 0, 0);
        NetDaemonNet.await_diplo(0x44a);
        result = 1;
    } else {
        result = do_withdrawal(faction_id, against_id);
    }
    if (!territory_notice && GameDataStateStorage.fields_.sunspot_duration_ <= 0) {
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(0, Players[faction_id].name_leader, -1, -1);
        parse_says(1, Players[faction_id].adj_leader, -1, -1);
        if (result) {
            uint32_t treaties = PlayersData[faction_id].diplo_treaties[against_id];
            if (treaties & DTREATY_PACT) {
                parse_says(2, get_pact(faction_id), -1, -1);
                X_pops("WITHDRAWPACT", 0x100000, FactionDiplomacyPortraits[against_id], pop_wait);
                return 0;
            }
            // The image pairs a treaty with WITHDRAW and anything else with WITHDRAWTRUCE.
            if (!(treaties & DTREATY_TREATY)) {
                X_pops("WITHDRAWTRUCE", 0x100000, FactionDiplomacyPortraits[against_id], pop_wait);
                return 0;
            }
            X_pops("WITHDRAW", 0x100000, FactionDiplomacyPortraits[against_id], pop_wait);
            return 0;
        }
        X_pops("NOWITHDRAW", 0x100000, FactionDiplomacyPortraits[against_id], pop_wait);
    }
    return 0;
}

// ORIGINAL: 0x005401A0 ?buy_tech@@YAHHHHHH@Z 0x005401A0-0x00540626 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned5401A0-540626 complete native instructions;540322/540328 tech_ranking35C;540335-54033B truncating divide4;54059C reloads multiplayer and selected tech after money callback", "historical seed:src/unrecovered/005401a0.cpp", "Pinned540298/5402A0/5402A7: load F800, unguarded byte at GameTechAchieved9A6670+F800, bit_count;5402AF..5402CF: subtract1, clamp1..999, signed divide. Pseudo IDs may read adjacent storage; original runtime effects remain unverified."], "reason": "Recover technology, prototype and commlink purchase as genuine faction C++ with typed faction/global storage and source-owned literal keys. Preserve native pricing, signed technology-rank discounts, live post-callback transfer state and network argument direction, including the original unguarded selected-ID bitmap lookup. Invalid-index runtime effects and complete matching remain obligations."}
// symbol    ?buy_tech@@YAHHHHHH@Z
// TRIED: direct transcription of the Ghidra decompile (tech-cost computation, string message build via say_tech/parse_says/ parse_num/strcat, X_pops purchase gate, energy/tech transfer with Console/NetDaemon dispatch); diverges at insn #5 in the prologue register-save order, not in control flow
int __cdecl buy_tech(int faction_id, int other_faction_id, int action,
                     int is_commlink, int mode) {
    PopupDialogFactionID = other_faction_id;

    if (mode == 5) {
        if (BestProtoForTrade < 0) {
            scan_prototypes(faction_id, other_faction_id);
            if (BestProtoForTrade < 0) return 0;
        }
        DiploScratchF800 = diplo_add(BestProtoForTrade, 0x61);
    } else if (mode != 0xD) {
        if (DiploScratchF800 < 0) return 0;
        if (strcmp(Players[other_faction_id].filename, "UNIV") == 0 &&
            DiploScratchFAA8 >= 0) return 0;
    }

    if (action == 1) return 0;

    int value = tech_val(DiploScratchF800, other_faction_id, 1) *
        ((GameDataStateStorage.fields_.turn_current_num_ + 7) / 8);
    if (great_satan(faction_id, 0)) value *= 2;
    if (DiploScratchFA3C != 0) value *= 2;
    if (is_commlink != 0) value = value * 3 / 2;

    // The original does not bound the selected ID before this byte read,
    // including pseudo-tech IDs. Preserve that lookup and its subsequent
    // clamp; do not substitute a divisor for an invalid array index.
    value /= range(bit_count(
        GameDataStateStorage.fields_.game_tech_achieved_[DiploScratchF800]) - 1, 1, 999);

    const uint32_t treaty = PlayersData[faction_id].diplo_treaties[other_faction_id];
    if ((treaty & 1) != 0 &&
        PlayersData[other_faction_id].ranking > PlayersData[faction_id].ranking) {
        const int actor_research = PlayersData[faction_id].tech_ranking;
        const int partner_research = PlayersData[other_faction_id].tech_ranking;
        if (partner_research > actor_research + 8) {
            value /= 4;
        } else if (partner_research > actor_research) {
            value /= 2;
        }
    }

    int cost = diplo_mul(diplo_add(value, 0x19) / 0x19, 0x19);
    if (mode != 0xD && cost < 100) cost = 100;
    if (PlayersData[faction_id].energy_reserves < cost &&
        action != 5 && mode != 5) return 0;

    char prompt_number[80];
    StringTemp[0] = '\0';
    say_tech(StringTemp, DiploScratchF800, 1);
    parse_says(0, StringTemp, -1, -1);
    parse_num(0, cost);
    StringTemp[0] = '\0';
    strcat(StringTemp, "BUY");
    if (mode == 5) {
        strcat(StringTemp, "PROTO");
    } else if (mode == 0xD) {
        strcat(StringTemp, "COMMLINK");
        parse_says(0, Players[DiploScratchFA5C].adj_name_faction, -1, -1);
    } else {
        strcat(StringTemp, "TECH");
    }

    if (PlayersData[faction_id].energy_reserves < cost) {
        strcat(StringTemp, "HIGH");
        const int prompt = (mode == 5 || mode == 0xD) ? 0 : random(0, 2);
        _itoa(prompt, prompt_number, 10);
        strcat(StringTemp, prompt_number);
        X_pops(StringTemp, 0x100000,
                     FactionDiplomacyPortraits[other_faction_id], pop_wait);
        return 1;
    }

    const int prompt = (mode == 5 || mode == 0xD) ? 0 : random(0, 2);
    _itoa(prompt, prompt_number, 10);
    strcat(StringTemp, prompt_number);
    const int answer = X_pops(
        StringTemp, 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
    if (answer != 1) return 1;

    const int debit = diplo_sub(0, cost);
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingENERGY, faction_id, other_faction_id, debit);
        message_data(0x2447, 0, faction_id, debit, other_faction_id, cost);
    } else {
        if (faction_id != 0) {
            PlayersData[faction_id].energy_reserves =
                diplo_add(PlayersData[faction_id].energy_reserves, debit);
        }
        if (other_faction_id != 0) {
            PlayersData[other_faction_id].energy_reserves =
                diplo_add(PlayersData[other_faction_id].energy_reserves, cost);
        }
        ConsoleGlobal.update_data(1);
    }
    const int purchased_tech = DiploScratchF800;
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTECH, faction_id, purchased_tech, other_faction_id);
        message_data(0x244B, 0, faction_id, purchased_tech, other_faction_id, 0);
        NetDaemonNet.await_diplo(0x44B);
    } else {
        tech_achieved(faction_id, purchased_tech, other_faction_id, 0);
    }

    if (mode == 5) BestProtoForTrade = -1;
    ConsoleInfoWin.set_redraw_phase(0);
    ConsoleGlobal.update_data(0);
    diplomacy_caption(faction_id, other_faction_id);
    return 1;
}

// ORIGINAL: 0x00540630 ?energy_trade@@YAHHH@Z 0x00540630-0x005410F3
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned540630-5410F3 full decoded instructions and verified literal keys68D74C-68D7CC", "Native54067D other-to-actor pact row;5406F1 tech_ranking35C;540D81/540D93 commerce304;54093B/54094A scaled3/4;540F15 wrapped signed product divided100;54106D storesperiodcount", "historical seed:src/unrecovered/00540630.cpp", "Pinned540958/540960/540967: load F800, unguarded byte at GameTechAchieved9A6670+F800, bit_count;54096F..54098F: subtract1, clamp1..99, signed divide. Independent outgoing FAA8 may be valid when F800=-1; neighboring-storage runtime effects remain unverified."], "reason": "Recover complete faction freebie, energy-for-technology and loan flows through existing typed faction/global owners and real popup/parser/network services. Preserve directed treaty rows, technology versus commerce ranking, wrapped signed valuation, live modal selections, network transfer order, native period-count loan field and the original unguarded selected-technology bitmap read. Invalid-index runtime effects, matching and game acceptance remain unproved."}
// symbol    ?energy_trade@@YAHHH@Z
// TRIED: full goto/label transcription mirroring Ghidra's control flow 1:1 (verified array strides/index scaling against raw disasm at the entry points); diverges at insn #11, near the DAT_0093fab0 tech-stage dispatch (call vs mov) - likely bVar2 computed via a branchy compare sequence in the original rather than a plain OR.
int __cdecl energy_trade(int faction, int other) {
    PopupDialogFactionID = other;
    tech_analysis(faction, other);
    const int mode = DiplomacyProposalMode;
    const bool offer_technology = mode == 4 || mode == 2;
    char number[80];

    if ((PlayersData[other].diplo_treaties[faction] & 1u) != 0 && mode != 6
        && PlayersData[other].ranking
            > diplo_add(PlayersData[faction].ranking, 1)
        && PlayersData[other].energy_reserves
            > diplo_mul(PlayersData[faction].energy_reserves, 2)
        && PlayersData[other].tech_ranking > PlayersData[faction].tech_ranking
        && PlayersData[other].energy_reserves > diplo_mul(GameDataStateStorage.fields_.turn_current_num_, 4)
        && PlayersData[other].diplo_patience[faction] < 2
        && (DiploScratchFAA8 < 0 || mode == 1)) {
        int amount = diplo_clamp(PlayersData[other].energy_reserves / 8, 1, 500);
        amount = diplo_mul(diplo_add(amount, 24) / 25, 25);
        // Native540777..54077B chooses the larger value, even when this
        // gives away the entire reserve. Do not silently reverse that test.
        if (amount < PlayersData[other].energy_reserves)
            amount = PlayersData[other].energy_reserves;
        const int debit = diplo_sub(0, amount);
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingENERGY, other, faction, debit);
            message_data(0x2447, 0, other, debit, faction, amount);
            NetDaemonNet.await_diplo(0x447);
        } else {
            if (other) PlayersData[other].energy_reserves =
                diplo_add(PlayersData[other].energy_reserves, debit);
            if (faction) PlayersData[faction].energy_reserves =
                diplo_add(PlayersData[faction].energy_reserves, amount);
            ConsoleGlobal.update_data(1);
        }
        PlayersData[other].diplo_patience[faction] = static_cast<char>(
            static_cast<unsigned char>(PlayersData[other].diplo_patience[faction]) + 2u);
        strcpy(StringTemp, "ENERGYFREEBIE");
        _itoa(random(0, 2), number, 10);
        strcat(StringTemp, number);
        X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other], pop_wait);
        return 0;
    }

    const bool loan_mode = mode == 6 || mode == 1 || (mode == 2 && DiploScratchFAA8 < 0);
    if (!loan_mode && offer_technology) {
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[faction].title_leader, -1, -1);
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction].name_leader, -1, -1);
        if (DiploScratchFAA8 < 0 || !strcmp(Players[other].filename, "BELIEVE")) {
            X_pops("REJSELLNONE", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
            return 0;
        }
        const char *other_filename = Players[other].filename;
        int scale = diplo_mul(diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8, 32);
        if (great_satan(faction, 0)) scale = diplo_mul(scale, 3) / 4;
        // F800 can remain -1 while the independent outgoing FAA8 is valid.
        // The original still reads at F800 relative to the bitmap before
        // clamping the population divisor; retain that original quirk.
        int divisor = diplo_clamp(bit_count(
            GameDataStateStorage.fields_.game_tech_achieved_[DiploScratchF800]) - 1, 1, 99);
        scale /= divisor;
        int amount = diplo_mul(tech_val(DiploScratchFAA8, other, 1), scale) / 32;
        int second_value = 0;
        if (DiploScratchFA18 >= 0)
            second_value = diplo_mul(tech_val(DiploScratchFA18, other, 1), scale) / 32;
        const int available = PlayersData[other].energy_reserves / 2;
        if (amount < 0 || available < 0) amount = 0;
        else if (available < amount) amount = available;
        amount = diplo_mul(amount / 25, 25);
        if (amount < 25) amount = 25;
        if (second_value < 25) second_value = 25;
        if (!amount) {
            X_pops("REJSELLAFFORD", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
            return 0;
        }
        int answer;
        for (;;) {
            StringTemp[0] = 0;
            say_tech(StringTemp, DiploScratchFAA8, 1);
            parse_says(0, StringTemp, -1, -1);
            if (DiploScratchFA18 >= 0) {
                StringTemp[0] = 0;
                say_tech(StringTemp, DiploScratchFA18, 1);
                parse_says(1, StringTemp, -1, -1);
            }
            parse_num(0, amount);
            strcpy(StringTemp, "ENERGYTECH");
            if (DiploScratchFA18 >= 0) strcat(StringTemp, "2");
            else {
                _itoa(random(0, 2), number, 10);
                strcat(StringTemp, number);
            }
            answer = X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other], pop_wait);
            if (!answer) return 0;
            if ((answer == 2 && DiploScratchFA18 < 0) || answer == 3) {
                help_topic(14, DiploScratchFAA8);
                continue;
            }
            break;
        }
        int tech;
        if (answer == 2) {
            if (!strcmp(other_filename, "HIVE")
                || (amount < second_value && strcmp(other_filename, "UNIV"))) {
                X_pops("REJSELLSECOND", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
                return 0;
            }
            X_pops("SELLSECOND", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
            tech = DiploScratchFA18;
        } else tech = DiploScratchFAA8;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTECH, other, tech, faction);
            message_data(0x244B, 0, other, tech, faction, 0);
        } else tech_achieved(other, tech, faction, 0);
        if (answer != 2) DiploScratchFAA8 = DiploScratchFA18;
        DiploScratchFA18 = -1;
        const int debit = diplo_sub(0, amount);
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingENERGY, other, faction, debit);
            message_data(0x2447, 0, other, debit, faction, amount);
            NetDaemonNet.await_diplo(0x447);
        } else {
            if (other) PlayersData[other].energy_reserves =
                diplo_add(PlayersData[other].energy_reserves, debit);
            if (faction) PlayersData[faction].energy_reserves =
                diplo_add(PlayersData[faction].energy_reserves, amount);
            ConsoleGlobal.update_data(1);
        }
        ConsoleInfoWin.set_redraw_phase(0);
        ConsoleGlobal.update_data(0);
        return 1;
    }

    if (loan_mode) {
        int principal;
        const int reserve = PlayersData[other].energy_reserves;
        if (PlayersData[other].tech_commerce_bonus < PlayersData[faction].tech_commerce_bonus)
            principal = reserve / 4;
        else if (PlayersData[other].tech_commerce_bonus == PlayersData[faction].tech_commerce_bonus)
            principal = reserve / 3;
        else principal = reserve / 2;
        if (reserve < PlayersData[faction].energy_reserves && strcmp(Players[other].filename, "MORGAN"))
            principal = 0;
        if (PlayersData[faction].loan_balance[other] || PlayersData[other].loan_balance[faction])
            principal = 0;
        const uint32_t other_treaty = PlayersData[other].diplo_treaties[faction];
        if ((other_treaty & 0x30u) || !(other_treaty & 2u) || principal < 10)
            principal = 0;
        int surcharge = diplo_mul(diplo_add(diplo_add(DiploScratchFA78, 10),
            diplo_mul(diplo_sub(PlayersData[other].tech_commerce_bonus,
                              PlayersData[faction].tech_commerce_bonus), 2)), 5);
        if (PlayersData[faction].diplo_treaties[other] & 1u) {
            surcharge = diplo_sub(surcharge, 10);
            if (PlayersData[other].ranking > PlayersData[faction].ranking)
                surcharge = diplo_sub(surcharge, 10);
        }
        if (surcharge < 0) surcharge = 0;
        int payment = diplo_mul(diplo_sub(10, DiploScratchFA78), 10);
        if (payment < 20) payment = 20;
        if (principal) {
            const int total = diplo_mul(diplo_add(surcharge, 100), principal) / 100;
            if (payment < 10 || total < 10) payment = 10;
            else if (total < payment) payment = total;
            const int periods = diplo_add(diplo_sub(total, 1), payment) / payment;
            const int balance = diplo_mul(periods, payment);
            parse_num(0, principal);
            parse_num(1, periods);
            parse_num(2, payment);
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[faction].title_leader, -1, -1);
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[faction].name_leader, -1, -1);
            strcpy(StringTemp, "ENERGYLOAN");
            _itoa((periods != 1) + 1, number, 10);
            strcat(StringTemp, number);
            if (!X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other], pop_wait))
                return 0;
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingLOAN, faction, other, balance);
                message_data(0x244E, 0, faction, other, balance, periods);
            } else {
                PlayersData[faction].loan_balance[other] = balance;
                // Native541067..54106D stores the rounded balance and the
                // period count, not the per-period amount displayed in slot2.
                PlayersData[faction].loan_payment[other] = periods;
            }
            const int debit = diplo_sub(0, principal);
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingENERGY, other, faction, debit);
                message_data(0x2447, 0, other, debit, faction, principal);
                NetDaemonNet.await_diplo(0x447);
            } else {
                if (other) PlayersData[other].energy_reserves =
                    diplo_add(PlayersData[other].energy_reserves, debit);
                if (faction) PlayersData[faction].energy_reserves =
                    diplo_add(PlayersData[faction].energy_reserves, principal);
                ConsoleGlobal.update_data(1);
            }
            return 1;
        }
    }
    strcpy(StringTemp, "REJENERGY");
    _itoa(random(0, 2), number, 10);
    strcat(StringTemp, number);
    X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other], pop_wait);
    return 0;
}

// ORIGINAL: 0x00544E40 ?propose_treaty@@YAHHH@Z 0x00544E40-0x00545D55
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned544E40-545D55 full decoded original instructions and independently read dialogue keys68D998-68DA50", "Native Player59C and PlayerData20CC field layout;545759 technology transfer;54593E energy transfer;545B58 nohelp CFG;545CD0 actor/other settlement", "historical seed:src/unrecovered/00544e40.cpp"], "reason": "Recover faction treaty negotiation with typed diplomacy ownership: directed relationship gates, staged score arithmetic, technology and energy prompts, help retry, network and local transfer paths, military-partner help and final treaty settlement. Preserve historical findings and obligations; coverage-first source is not an exact or game-acceptance claim."}
// symbol    ?propose_treaty@@YAHHH@Z
// RECOVERY_HISTORY: // TRIED: the acceptance-scoring block (percentage math against tech_val/relationship tables using reciprocal-multiply constants for `/100`-style division, several hundred instructions) is approximated with a smaller set of named boolean gates feeding the same dialogue/X_pops/log_say/message_data/treaty_off/double_cross/ make_treaty calls, not the per-register arithmetic or exact nesting - the branch order genuinely differs from the disassembly (val_8 is actually tested AFTER the opening-dialogue X_pops loop, not before it). The early gate (strcmp vs the AI-type name, the two "pact already blocked" table checks using the 2099-stride index) and the final treaty_off/double_cross/make_treaty tail are transcribed in call order. MISMATCH #5 (prologue-adjacent, mov vs push).
// The seed warning above is historical, not a current acceptance-gate finding.
// Pinned 0x544E80-0x545144 selects the numeric gates before the four introductory
// parse_says calls, then tests the saved decision at 0x5451EA. The FA78 gate
// uses clamped actor difficulty; the separate FA2C gate uses bias, not difficulty.
// These source predicates retain that distinction. Whole-function matching and
// the remaining structural/runtime obligations are still separate requirements.
int __cdecl propose_treaty(int actor, int other)
{
    PopupDialogFactionID = other;
    int bias = (DiplomacyProposalGateCount < 0)
        + ((!strcmp(Players[other].filename, "GAIANS") && DiplomacyProposalGateCount < 0) ? 1 : 0);
    int partner_id = DiploScratchF7E8 ? DiploScratchF7E8 : DiploScratchFA8C;
    int partner_threat = partner_id
        && PlayersData[other].diplo_treaties[partner_id] & 0x8000
        && PlayersData[partner_id].mil_strength_1 > PlayersData[other].mil_strength_1
        && !(PlayersData[other].diplo_agenda[actor] & 0x800);
    int eligible = DiplomacyProposalMode == 2 || DiplomacyProposalMode == 4
        || DiplomacyProposalMode == 5 || DiplomacyProposalMode == 8;
    int accepted = 0;
    if (!DiploScratchFA3C && !DiploScratchF7FC && PlayersData[other].ranking <= 4
        && PlayersData[actor].ranking >= 5 && !DiploScratchFA54) {
        bias++;
    }
    if (DiploFriction <= (eligible ? bias + 2 : 0) + 15
        && !(PlayersData[other].diplo_agenda[actor] & 0x400)
        && (!(PlayersData[other].diplo_treaties[actor] & 0x20)
            || (DiploScratchFA78 <= 6
                && DiploScratchFA78 <= PlayersData[other].tech_commerce_bonus
                    - PlayersData[actor].tech_commerce_bonus + (eligible ? 2 : 0)
                    - DiplomacyProposalGateCount + 2))) {
        if (partner_threat) {
            accepted = 1;
        } else if (!DiploScratchFA3C
            && DiploScratchFA78 <= (eligible ? 2 : 0) - range(PlayersData[actor].diff_level, 0, 2) + bias + 5
            && DiploScratchFA2C <= bias + (DiplomacyProposalMode == 3 ? 2 : 0) + (eligible ? 1 : 0) + 4
            && (DiploFriction <= 10 + 5 * eligible + bias * 3 || DiplomacyProposalMode == 3)) {
            accepted = 1;
        }
    }
    if (PlayersData[other].flags & 0x400000 && !(PlayersData[other].diplo_treaties[actor] & 0x20)) {
        accepted = 1;
    }
    if (PlayersData[actor].diplo_agenda[other] & 0x2000) {
        accepted = 0;
    }
    parse_set(Players[actor].is_leader_female, 0);
    parse_says(0, Players[actor].name_leader, -1, -1);
    parse_set(Players[actor].is_leader_female, 0);
    parse_says(1, Players[actor].title_leader, -1, -1);
    parse_set(Players[actor].noun_gender, Players[actor].is_noun_plural);
    parse_says(2, Players[actor].noun_faction, -1, -1);
    parse_says(3, Players[actor].adj_name_faction, -1, -1);
    if (!accepted) {
        if (DiploScratchFA78 > 4 && (DiploScratchFA98 || DiploFriction <= 2)
            && !(PlayersData[other].diplo_treaties[actor] & 0x20)) {
            X_pops("REJTREATYMEEK", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
            return 1;
        }
        if (DiplomacyProposalMode == 3) {
            return 0;
        }
        if (!(PlayersData[other].diplo_treaties[actor] & 0x20) && DiploScratchFA78 <= 4) {
            if (DiploScratchFA3C) {
                parse_says(4, get_noun_phrase(other, 9), -1, -1);
                parse_says(5, get_phrase(actor, 3), -1, -1);
                X_pops("REJTREATYAHEAD", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
                return 1;
            }
            X_pops("REJTREATY", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
            return 1;
        }
        parse_set(Players[actor].is_leader_female, 0);
        parse_says(4, Players[actor].insult_leader, -1, -1);
        X_pops("REJTREATYHATE", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
        return 1;
    }
    int treaty_score = (PlayersData[actor].energy_reserves + DiploScratchFA78)
        * PlayersData[other].mil_strength_1
        / (PlayersData[actor].mil_strength_1 + PlayersData[other].mil_strength_1 + 1) / 50 * 50;
    if (!(PlayersData[other].diplo_treaties[actor] & 0x20)) {
        treaty_score -= range(PlayersData[other].tech_commerce_bonus
            - PlayersData[actor].tech_commerce_bonus, 0, 99) * 50;
    }
    if (!strcmp(Players[other].filename, "MORGAN")
        && treaty_score > PlayersData[actor].energy_reserves) {
        if (DiploScratchFAA8 < 0) {
            treaty_score = PlayersData[actor].energy_reserves;
        } else {
            if (PlayersData[actor].energy_reserves >= tech_val(DiploScratchFAA8, actor, 1)
                    * ((GameDataStateStorage.fields_.turn_current_num_ + 7) / 8)) {
                treaty_score = PlayersData[actor].energy_reserves;
            }
        }
    }
    if (DiplomacyProposalMode != 5 && treaty_score > PlayersData[actor].energy_reserves) {
        treaty_score -= (uint32_t)(treaty_score - PlayersData[actor].energy_reserves + 49) / 50 * 50;
    }
    if (treaty_score < 50) {
        treaty_score = 0;
    }
    int should_offer_technology = 0;
    int mode_allows_offer = 0;
    int offer_made = 0;
    int offer_taken = 0;
    char number[80];
    if ((!partner_threat && DiploFriction > bias * 2 + 10
            && PlayersData[actor].tech_commerce_bonus >= PlayersData[other].tech_commerce_bonus)
        || DiploFriction > 15 || PlayersData[other].diplo_treaties[actor] & 0x20
        || DiploScratchFA78 > range(PlayersData[other].tech_commerce_bonus
            - PlayersData[actor].tech_commerce_bonus, 0, 99) + 2) {
        should_offer_technology = 1;
    }
    if (DiplomacyProposalMode == 5 || DiplomacyProposalMode == 4) {
        mode_allows_offer = 1;
    }
    if (DiplomacyProposalMode == 2) {
        mode_allows_offer = 1;
        should_offer_technology = 0;
    } else if (DiplomacyProposalMode == 3) {
        should_offer_technology = 0;
    }
    if ((should_offer_technology || mode_allows_offer) && DiploScratchFAA8 >= 0
        && strcmp(Players[other].filename, "BELIEVE")
        && (strcmp(Players[other].filename, "MORGAN") || !treaty_score
            || PlayersData[actor].energy_reserves < treaty_score
            || treaty_score < tech_val(DiploScratchFAA8, actor, 1) * 100)
        && (DiplomacyProposalMode == 4 || DiplomacyProposalMode == 2
            || (DiplomacyProposalMode == 1 && DiploFriction < 10))) {
        offer_made = 1;
        int response;
        for (;;) {
            parse_set(Players[other].is_leader_female, 0);
            parse_says(1, Players[other].title_leader, -1, -1);
            StringTemp[0] = '\0';
            say_tech(StringTemp, DiploScratchFAA8, 1);
            parse_says(2, StringTemp, -1, -1);
            parse_says(3, get_noun_phrase(other, 9), -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, "GIMMETECH");
            int pick = random(0, 2);
            _itoa(pick, number, 10);
            strcat(StringTemp, number);
            response = X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other], pop_wait);
            if (response <= 1) {
                break;
            }
            help_topic(14, DiploScratchFAA8);
        }
        if (!response) {
            accepted = 0;
        } else {
            int tech_id = DiploScratchFAA8;
            offer_taken = 1;
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingTECH, other, tech_id, actor);
                message_data(0x244b, 0, other, tech_id, actor, 0);
                NetDaemonNet.await_diplo(0x44b);
            } else {
                tech_achieved(other, tech_id, actor, 0);
            }
            int friction_delta = -(tech_val(DiploScratchFAA8, actor, 1) / 2);
            PlayersData[other].diplo_friction[actor]
                = range(friction_delta + PlayersData[other].diplo_friction[actor], 0, 20);
            if (DiploFrictionFactionID == (uint32_t)other
                && DiploFrictionFactionIDWith == (uint32_t)actor) {
                DiploFriction += friction_delta;
            }
            diplomacy_caption(actor, other);
            DiploScratchFAA8 = DiploScratchFA18;
            DiploScratchFA18 = -1;
        }
    }
    if ((should_offer_technology || mode_allows_offer) && treaty_score
        && PlayersData[actor].energy_reserves >= treaty_score && !offer_made
        && strcmp(Players[other].filename, "PEACE")
        && (DiplomacyProposalMode == 5 || DiplomacyProposalMode == 2
            || (DiplomacyProposalMode == 1 && DiploFriction < 10))) {
        offer_made = 1;
        parse_num(0, treaty_score);
        parse_set(Players[other].is_leader_female, 0);
        parse_says(1, Players[other].title_leader, -1, -1);
        StringTemp[0] = '\0';
        strcat(StringTemp, "GIMMEENERGY");
        int pick = random(0, 2);
        _itoa(pick, number, 10);
        strcat(StringTemp, number);
        if (!X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other], pop_wait)) {
            accepted = 0;
        } else {
            int energy_debit = -treaty_score;
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingENERGY, actor, other, energy_debit);
                message_data(0x2447, 0, actor, energy_debit, other, treaty_score);
                NetDaemonNet.await_diplo(0x447);
            } else {
                if (actor) {
                    PlayersData[actor].energy_reserves += energy_debit;
                }
                if (other) {
                    PlayersData[other].energy_reserves += treaty_score;
                }
                ConsoleGlobal.update_data(1);
            }
            offer_taken = 1;
            int friction_units = 0;
            int divisor = 2;
            int remaining = treaty_score / 5;
            int chunk = 10;
            while (remaining > 0) {
                friction_units += range(remaining, 0, chunk) / divisor;
                remaining -= chunk;
                chunk = 20;
                divisor++;
            }
            int friction_delta = (friction_units + 4) / -5;
            PlayersData[other].diplo_friction[actor]
                = range(friction_delta + PlayersData[other].diplo_friction[actor], 0, 20);
            if (DiploFrictionFactionID == (uint32_t)other
                && DiploFrictionFactionIDWith == (uint32_t)actor) {
                DiploFriction += friction_delta;
            }
            diplomacy_caption(actor, other);
            ConsoleInfoWin.set_redraw_phase(0);
            ConsoleGlobal.update_data(0);
        }
    }
    if (accepted && (!should_offer_technology || offer_made)) {
        if ((DiplomacyProposalMode == 4 || DiplomacyProposalMode == 5) && offer_taken) {
            DiploScratchF7E8 = 0;
        }
        if (partner_id && !DiploScratchFA8C) {
            parse_set(Players[actor].is_leader_female, 0);
            parse_says(5, Players[actor].title_leader, -1, -1);
            parse_set(Players[partner_id].noun_gender, Players[partner_id].is_noun_plural);
            parse_says(6, Players[partner_id].noun_faction, -1, -1);
            if (!X_pops("GIMMEHELP", 0x100000, FactionDiplomacyPortraits[other], pop_wait)) {
                if ((partner_threat
                        || (int)GameDataStateStorage.fields_.faction_rankings_[7] == actor
                        || DiploScratchFA78 > 3 || DiplomacyProposalMode == 1)
                    && !offer_taken && strcmp(Players[other].filename, "GAIANS")) {
                    X_pops("REJTREATYNOHELP", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
                    PlayersData[other].diplo_patience[actor]++;
                    return 1;
                }
            } else {
                log_say(DiploLabelGettingDIPLOMATICLOCK, 0, 0, 0);
                timeGetTime();
                while (!ExitTurnLoop && NetDaemonNet.add_global()) {
                    check_spock();
                    timeGetTime();
                }
                if (IsMultiplayerNet) {
                    log_say(DiploLabelSendingTREATYOFF, actor, other, 0x800000);
                    message_data(0x2442, 0, actor, other, 0x800000, 0);
                } else {
                    treaty_off(actor, other, 0x800000);
                }
                if (IsMultiplayerNet) {
                    log_say(DiploLabelSendingDOUBLECROSS, actor, partner_id, other);
                    message_data(0x2440, 0, actor, partner_id, other, 0);
                    NetDaemonNet.await_diplo(0x440);
                } else {
                    double_cross(actor, partner_id, other);
                }
                log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
                NetDaemonNet.unlock();
                DiploScratchF7E8 = 0;
                DiploScratchFA4C = 0;
            }
        }
        make_treaty(actor, other);
        return 1;
    }
    if (!offer_made) {
        if (DiplomacyProposalMode == 1) {
            return 0;
        }
        X_pops("REJTREATYBADBRIBE", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
        return 1;
    }
    X_pops("REJTREATYNOBRIBE", 0x100000, FactionDiplomacyPortraits[other], pop_wait);
    return 1;
}

// Shared proposal selection; threaten and proposal dispatch read this slot.
// RECOVERY_OBJECT: v1 0x0093FA34 ?DiploScratchFA34@@3HA
int DiploScratchFA34;

// ORIGINAL: 0x00549930 ?threaten@@YAXHH@Z 0x00549930-0x0054ACB9
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned549930-54ACB9 decoded span and native literal keys68DC8C-68DD18", "Fresh physical COFF PlayerData20CC:ai_fight3DC,region_total_bases117C,commerce304,tech_ranking35C;549930 directedmood;54A537-54A57F baserefusal;54A5C2 basevaluation", "historical seed:src/unrecovered/00549930.cpp"], "reason": "Recover complete faction threat, provocation, war request, energy and technology extortion, base cession and pact termination flows through typed faction/player/base/map and network owners. Native byte-indexed96DB5C is the declared regional base-count member at117C, not the force-rating array at127C. Preserve original identity and seed findings; coverage-first source is not byte-exact or game acceptance."}
// symbol    ?threaten@@YAXHH@Z
// TRIED: full-function transcription (~370 instructions, sim 0.69 vs original at /O2 /Oy-); diverges at instr #4 in the prologue frame shape. Approximated: log_say's 2nd-4th args where the decompiler elided them (derived from the paired message_data call's matching args, confirmed against raw disasm on 2/2 spot checks); the fa98 mood-recompute arithmetic order taken from Ghidra rather than re-derived by hand.
void __cdecl threaten(int faction_id, int other_faction_id) {
    PopupDialogFactionID = other_faction_id;
    int mood;
    if (PlayersData[other_faction_id].diplo_treaties[faction_id] & DTREATY_VENDETTA) {
        mood = 1;
    } else if (PlayersData[other_faction_id].diplo_treaties[faction_id] & DTREATY_PACT) {
        mood = (PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x2000000) ? 500 : 6;
    } else {
        mood = ((PlayersData[other_faction_id].diplo_treaties[faction_id] >> 1) & 1)
            - (DiploFriction + 3) / 8 + 3;
    }
    PlayersData[other_faction_id].diplo_patience[faction_id] = (char)mood;
    tech_analysis(faction_id, other_faction_id);
    char number[80];
    if (DiploScratchFA70) {
        PlayersData[faction_id].diplo_spoke[other_faction_id] = GameDataStateStorage.fields_.turn_current_num_;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYON, other_faction_id, faction_id, 0x10);
            message_data(0x2441, 0, other_faction_id, faction_id, 0x10, 0);
            NetDaemonNet.await_diplo(0x441);
        } else {
            treaty_on(other_faction_id, faction_id, 0x10);
        }
        diplomacy_caption(faction_id, other_faction_id);
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(1, Players[faction_id].name_leader, -1, -1);
        parse_says(2, get_noun_phrase(faction_id, 8), -1, -1);
        StringTemp[0] = '\0';
        strcat(StringTemp, "PROVOKED");
        int pick = random(0, 2);
        _itoa(pick, number, 10);
        strcat(StringTemp, number);
        X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
        return;
    }
    if (PlayersData[faction_id].diplo_treaties[other_faction_id] & DTREATY_PACT && !DiploScratchFA98) {
        int rank = PlayersData[faction_id].ranking;
        if (rank <= 5 && PlayersData[other_faction_id].ranking >= 6 && DiploScratchFA34 != 8) {
            int value = PlayersData[other_faction_id].tech_ranking / 2 - PlayersData[faction_id].tech_ranking / 2
                - PlayersData[faction_id].tech_commerce_bonus - PlayersData[other_faction_id].ai_fight
                + PlayersData[other_faction_id].tech_commerce_bonus - rank
                + PlayersData[other_faction_id].ranking;
            if (DiploScratchFA34 == 4) {
                value += PlayersData[other_faction_id].ai_tech - PlayersData[other_faction_id].ai_power;
            } else if (DiploScratchFA34 == 6) {
                value += PlayersData[other_faction_id].ai_wealth - PlayersData[other_faction_id].ai_power;
            }
            if (PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x2000000 && value < 4) {
                value = 4;
            } else {
                value = range(value, 0, 999);
            }
            DiploScratchFA98 = value;
        }
    }
    if (PlayersData[other_faction_id].diplo_treaties[faction_id] & DTREATY_PACT
        && PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x2000000 && DiploScratchFA98 < 2) {
        DiploScratchFA98 = 2;
    }
    int demand = (GameDataStateStorage.fields_.turn_current_num_ - DiploScratchF7E4) * DiploScratchFA98 * 25 / 16;
    if (demand > PlayersData[other_faction_id].energy_reserves) {
        demand = PlayersData[other_faction_id].energy_reserves / 25 * 25;
    }
    if (demand < 0) {
        demand = 0;
    }
    if (!DiploScratchFA98) {
        PlayersData[other_faction_id].diplo_friction[faction_id]
            = range(PlayersData[other_faction_id].diplo_friction[faction_id] + 2, 0, 20);
        if (DiploFrictionFactionID == (uint32_t)other_faction_id
            && DiploFrictionFactionIDWith == (uint32_t)faction_id) {
            DiploFriction += 2;
        }
    }
    diplomacy_caption(faction_id, other_faction_id);
    int did_action = 0;
    if (DiploScratchFA34 == 8) {
        if (PlayersData[other_faction_id].diplo_treaties[DiploScratchFA5C] & DTREATY_VENDETTA) {
            parse_set(Players[DiploScratchFA5C].is_leader_female, 0);
            parse_says(0, Players[DiploScratchFA5C].title_leader, -1, -1);
            parse_set(Players[DiploScratchFA5C].is_leader_female, 0);
            parse_says(1, Players[DiploScratchFA5C].name_leader, -1, -1);
            X_pops("REJWARALREADY", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
            return;
        }
        if (PlayersData[faction_id].diplo_treaties[other_faction_id] & DTREATY_PACT
            && ((PlayersData[faction_id].diplo_treaties[DiploScratchFA5C] & DTREATY_VENDETTA
                    && !(PlayersData[other_faction_id].diplo_treaties[DiploScratchFA5C] & DTREATY_PACT))
                || PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x2000000)) {
            if (wants_to_attack(other_faction_id, DiploScratchFA5C, 0)
                || wants_to_attack(DiploScratchFA5C, other_faction_id, 0)
                || PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x2000000
                || (PlayersData[other_faction_id].ranking <= 4 && PlayersData[faction_id].ranking >= 6)
                || (PlayersData[faction_id].diplo_treaties[DiploScratchFA5C] & 0x20
                    && PlayersData[DiploScratchFA5C].ranking > PlayersData[faction_id].ranking)
                || (PlayersData[DiploScratchFA5C].ranking > PlayersData[faction_id].ranking
                    && PlayersData[DiploScratchFA5C].ranking > PlayersData[other_faction_id].ranking)) {
                parse_set(Players[DiploScratchFA5C].is_leader_female, 0);
                parse_says(0, Players[DiploScratchFA5C].title_leader, -1, -1);
                parse_set(Players[DiploScratchFA5C].is_leader_female, 0);
                parse_says(1, Players[DiploScratchFA5C].name_leader, -1, -1);
                parse_set(Players[DiploScratchFA5C].noun_gender, Players[DiploScratchFA5C].is_noun_plural);
                parse_says(2, Players[DiploScratchFA5C].noun_faction, -1, -1);
                parse_says(3, get_pact(faction_id), -1, -1);
                X_pops("WARPACT", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
                net_double_cross(other_faction_id, DiploScratchFA5C, faction_id, 1);
                did_action = 1;
            }
            if (DiploScratchFA34 != 8) {
                goto offers;
            }
        }
        if (DiploScratchFA98 && !(PlayersData[other_faction_id].diplo_treaties[DiploScratchFA5C] & DTREATY_PACT)
            && (PlayersData[faction_id].diplo_treaties[other_faction_id] & DTREATY_TREATY
                || !(PlayersData[other_faction_id].diplo_treaties[DiploScratchFA5C] & DTREATY_TREATY))
            && PlayersData[faction_id].mil_strength_1 * DiploScratchFA98
                > PlayersData[DiploScratchFA5C].mil_strength_1 * PlayersData[DiploScratchFA5C].mil_strength_1
                    / (PlayersData[other_faction_id].mil_strength_1 + 1)) {
            parse_set(Players[DiploScratchFA5C].is_leader_female, 0);
            parse_says(0, Players[DiploScratchFA5C].title_leader, -1, -1);
            parse_set(Players[DiploScratchFA5C].is_leader_female, 0);
            parse_says(1, Players[DiploScratchFA5C].name_leader, -1, -1);
            parse_set(Players[DiploScratchFA5C].noun_gender, Players[DiploScratchFA5C].is_noun_plural);
            parse_says(2, Players[DiploScratchFA5C].noun_faction, -1, -1);
            X_pops("WARTHREAT", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
            net_double_cross(other_faction_id, DiploScratchFA5C, faction_id, 1);
            did_action = 1;
        }
    }
offers:
    if (demand && DiploScratchFA34 == 6) {
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(1, Players[faction_id].name_leader, -1, -1);
        parse_num(0, demand);
        X_pops("EXTORTENERGY", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
        int energy_debit = -demand;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingENERGY, faction_id, other_faction_id, demand);
            message_data(0x2447, 0, faction_id, demand, other_faction_id, energy_debit);
            NetDaemonNet.await_diplo(0x447);
        } else {
            if (faction_id) {
                PlayersData[faction_id].energy_reserves += demand;
            }
            if (other_faction_id) {
                PlayersData[other_faction_id].energy_reserves += energy_debit;
            }
            ConsoleGlobal.update_data(1);
        }
        ConsoleInfoWin.set_redraw_phase(0);
        ConsoleGlobal.update_data(0);
        did_action = 1;
    }
    if (DiploScratchFA98 && DiploScratchFA34 == 13) {
        parse_says(0, Players[DiploScratchFA5C].adj_name_faction, -1, -1);
        X_pops("EXTORTCOMMLINK", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
        int tech_id = DiploScratchFA5C + 0x59;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTECH, faction_id, tech_id, other_faction_id);
            message_data(0x244b, 0, faction_id, tech_id, other_faction_id, 0);
            NetDaemonNet.await_diplo(0x44b);
        } else {
            tech_achieved(faction_id, tech_id, other_faction_id, 0);
        }
        did_action = 1;
    }
    if (DiploScratchF800 >= 0 && DiploScratchFA98 && DiploScratchFA34 == 4) {
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        StringTemp[0] = '\0';
        say_tech(StringTemp, DiploScratchF800, 1);
        parse_says(1, StringTemp, -1, -1);
        X_pops("EXTORTTECH", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
        int tech_id = DiploScratchF800;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTECH, faction_id, tech_id, other_faction_id);
            message_data(0x244b, 0, faction_id, tech_id, other_faction_id, 0);
            NetDaemonNet.await_diplo(0x44b);
        } else {
            tech_achieved(faction_id, tech_id, other_faction_id, 0);
        }
        DiploScratchF800 = -1;
        did_action = 1;
    }
    if (DiploScratchFA34 == 9) {
        if (PlayersData[other_faction_id].diplo_treaties[faction_id] & DTREATY_PACT) {
            int region = region_at(Bases[DiploScratchFA7C].x, Bases[DiploScratchFA7C].y);
            int base_count;
            if (!(PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x2000000)
                || (base_count = PlayersData[other_faction_id].current_num_bases) <= 1
                || (PlayersData[faction_id].diplo_agenda[other_faction_id] & 0x1000
                    && Bases[DiploScratchFA7C].faction_id_former != faction_id
                    && (PlayersData[faction_id].region_total_bases[region]
                            < PlayersData[other_faction_id].region_total_bases[region]
                        || PlayersData[other_faction_id].region_total_bases[region] > base_count / 2))) {
                parse_set(Players[faction_id].is_leader_female, 0);
                parse_says(0, Players[faction_id].title_leader, -1, -1);
                parse_set(Players[faction_id].is_leader_female, 0);
                parse_says(1, Players[faction_id].name_leader, -1, -1);
                X_pops("BASENOCEDE", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
                return;
            }
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingAGENDAON, faction_id, other_faction_id, 0x1000);
                message_data(0x2444, 0, faction_id, other_faction_id, 0x1000, 0);
                NetDaemonNet.await_diplo(0x444);
            } else {
                agenda_on(faction_id, other_faction_id, 0x1000);
            }
        }
        int value = value_of_base(DiploScratchFA7C, other_faction_id, faction_id, DiploScratchFA98,
            (DiploScratchFA68 || DiploScratchFAA0) ? 1 : 0);
        if (value > 0 && value < DiploScratchFA98 * 25) {
            log_say(DiploLabelGettingDIPLOMATICLOCK, 0, 0, 0);
            timeGetTime();
            while (!ExitTurnLoop && NetDaemonNet.add_global()) {
                check_spock();
                timeGetTime();
            }
            parse_says(0, Bases[DiploScratchFA7C].name_string, -1, -1);
            net_cede_base(DiploScratchFA7C, faction_id, 1);
            X_pops("BASEOK", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
            if (!(PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x2000000)) {
                if (IsMultiplayerNet) {
                    log_say(DiploLabelSendingSETTREATY, other_faction_id, faction_id, 0x20);
                    message_data(0x2443, 0, other_faction_id, faction_id, 0x20, 1);
                    NetDaemonNet.await_diplo(0x443);
                } else {
                    set_treaty(other_faction_id, faction_id, 0x20, 1);
                }
            }
            log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
            NetDaemonNet.unlock();
            goto truce;
        }
    }
    if (did_action) {
truce:
        if (PlayersData[faction_id].diplo_treaties[other_faction_id] & DTREATY_TREATY) {
            return;
        }
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYON, other_faction_id, faction_id, 4);
            message_data(0x2441, 0, other_faction_id, faction_id, 4, 0);
            NetDaemonNet.await_diplo(0x441);
            return;
        }
        treaty_on(other_faction_id, faction_id, 4);
        return;
    }
    if (PlayersData[other_faction_id].diplo_treaties[faction_id] & DTREATY_PACT) {
        if ((DiploScratchFA68 || DiploScratchFA3C || DiploFriction > 10)
            && (PlayersData[faction_id].ranking > PlayersData[other_faction_id].ranking
                || PlayersData[faction_id].tech_ranking > PlayersData[other_faction_id].tech_ranking)
            && !(PlayersData[other_faction_id].diplo_treaties[faction_id] & 0x2000000)
            && PlayersData[faction_id].ranking >= 5) {
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            parse_says(2, get_pact(faction_id), -1, -1);
            X_pops("ENDPACT", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
            log_say(DiploLabelGettingDIPLOMATICLOCK, 0, 0, 0);
            timeGetTime();
            while (!ExitTurnLoop && NetDaemonNet.add_global()) {
                check_spock();
                timeGetTime();
            }
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingPACTENDS, 0, 0, 0);
                message_data(0x244d, 0, faction_id, other_faction_id, 0, 0);
                NetDaemonNet.await_diplo(0x44d);
            } else {
                pact_ends(faction_id, other_faction_id);
            }
            log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
            NetDaemonNet.unlock();
        }
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(0, Players[faction_id].name_leader, -1, -1);
        X_pops("FEND", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
        return;
    }
    if (DiploFriction > (DiploScratchFA68 ? 5 : 15) && !DiploScratchFA98
        && (PlayersData[other_faction_id].best_weapon_value >= PlayersData[faction_id].best_weapon_value
            || DiploScratchFAA0 || DiploScratchFA50)
        && PlayersData[other_faction_id].mil_strength_1
            > (PlayersData[faction_id].diff_level / 2 + PlayersData[faction_id].integrity_blemishes + 4)
                * PlayersData[faction_id].mil_strength_1 / 4) {
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYON, faction_id, other_faction_id, 0x10);
            message_data(0x2441, 0, faction_id, other_faction_id, 0x10, 0);
            NetDaemonNet.await_diplo(0x441);
        } else {
            treaty_on(faction_id, other_faction_id, 0x10);
        }
        PlayersData[faction_id].diplo_spoke[other_faction_id] = GameDataStateStorage.fields_.turn_current_num_;
        diplomacy_caption(faction_id, other_faction_id);
        StringTemp[0] = '\0';
        strcat(StringTemp, "PROVOKED");
        int pick = random(0, 2);
        _itoa(pick, number, 10);
        strcat(StringTemp, number);
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        parse_set(Players[faction_id].is_leader_female, 0);
        parse_says(1, Players[faction_id].name_leader, -1, -1);
        parse_says(2, get_noun_phrase(faction_id, 8), -1, -1);
        X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
        return;
    }
    parse_set(Players[faction_id].is_leader_female, 0);
    parse_says(0, Players[faction_id].title_leader, -1, -1);
    parse_set(Players[faction_id].is_leader_female, 0);
    parse_says(1, Players[faction_id].name_leader, -1, -1);
    X_pops("IGNORE", 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
}

/*
Purpose: Suggest the base the specified pair of factions should agree to attack together.
// ORIGINAL: 0x0054ACC0 ?suggest_plan@@YAHHH@Z 0x0054ACC0-0x0054AF9D BYTE_EXACT
// LEVER: nested ifs (not continue) with the visibility test re-reading
//        Bases[i].faction_id_current keep the image's dead owner compare;
//        the map.h vector_dist inline; base_find(x,y,faction) is kept
//        out of line with auto_inline(off). Remaining gap: ebx holds
//        faction_id across the distance block in the image (spills there).
// size      733 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3C60 0x004E3D50 0x00644F3A
Return Value: Base id of the suggested target, or -1 if the pair has no shared target
Status: Complete

WHO THE TWO PARAMETERS ARE. battle_plans (0x0054B1C0) is the only caller and it asks this twice,
once each way round, so neither parameter is "the AI" and neither is "the player": faction_id is
the side whose plan is being drawn up and faction_id_2 is the other side of the conversation. A
base is eligible only when BOTH factions are already at vendetta with its owner, so this proposes
a joint target within a war that exists rather than a war to start, and only when faction_id can
actually see the base.

THE NUMBER IS A COST, NOT A SCORE. The loop keeps the LOWEST value and seeds it at 9999. It is
assembled from three terms:

  - where the war already is. When faction_id holds a target tile (x_target >= 0) the cost opens
    at twice the radiating distance from that tile to the base, plus 512 when the base is not even
    in the same region as the tile.
  - how far the base sits from faction_id's own territory: BaseFindDist after searching for its
    nearest base in the SAME region as the candidate. Owning nothing in that region repeats the
    search unrestricted and adds 1024, the price of an amphibious war. This whole term is doubled
    when faction_id already holds a target tile, so a faction with a war in progress is pulled
    harder toward what it can reach than one starting fresh.
  - the same distance for faction_id_2, penalised 256 instead of 1024 and never doubled. The
    partner's convenience counts, at a quarter of the weight.

AN EXISTING PLAN WINS OUTRIGHT. If faction_id has already named a base in base_id_atk_target, that
base is returned the moment the loop reaches it, whatever it costs and whatever was cheaper
earlier. The suggestion machinery only decides while nothing has been decided.

Verification note: the visibility test transcribes a disjunct that cannot fire. The original
re-tests "base owner == faction_id" at 0x0054AD52 before consulting Bases[i].visibility, exactly as
the six-argument base_find does, but the loop already skipped that base at 0x0054AD00. It is kept
because it is what the image does, and it is still covered: inverting the comparison stops the
body skipping bases faction_id cannot see, which the fixtures observe.

Verification note: the image inlines the four-argument vector_dist (x_dist plus the two-argument
form, four abs calls) and region_at three times; the map.h vector_dist inline reproduces the former.

*/
int __cdecl suggest_plan(int faction_id, int faction_id_2) {
    PopupDialogFactionID = faction_id_2;
    int lowest_cost = 9999;
    int base_id_best = -1;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        int faction_id_base = Bases[i].faction_id_current;
        if (faction_id_base != faction_id && faction_id_base != faction_id_2
            && (PlayersData[faction_id].diplo_treaties[faction_id_base] & DTREATY_VENDETTA)
            && (PlayersData[faction_id_2].diplo_treaties[faction_id_base] & DTREATY_VENDETTA)) {
            if (Bases[i].faction_id_current == faction_id || ((1 << faction_id) & Bases[i].visibility)) {
                int cost = 0;
                if (PlayersData[faction_id].x_target >= 0) {
                    cost = 2 * vector_dist(PlayersData[faction_id].x_target,
                                              PlayersData[faction_id].y_target, Bases[i].x,
                                              Bases[i].y);
                    if (region_at(Bases[i].x, Bases[i].y)
                        != region_at(PlayersData[faction_id].x_target,
                                     PlayersData[faction_id].y_target)) {
                        cost += 512;
                    }
                }
                if (base_find(Bases[i].x, Bases[i].y, faction_id,
                              region_at(Bases[i].x, Bases[i].y), -1, -1) < 0) {
                    base_find(Bases[i].x, Bases[i].y, faction_id);
                    BaseFindDist += 1024;
                }
                cost += BaseFindDist * ((PlayersData[faction_id].x_target >= 0) ? 2 : 1);
                if (base_find(Bases[i].x, Bases[i].y, faction_id_2,
                              region_at(Bases[i].x, Bases[i].y), -1, -1) < 0) {
                    base_find(Bases[i].x, Bases[i].y, faction_id_2);
                    BaseFindDist += 256;
                }
                cost += BaseFindDist;
                if (PlayersData[faction_id].base_id_atk_target == i) {
                    return i;
                }
                if (cost < lowest_cost) {
                    lowest_cost = cost;
                    base_id_best = i;
                }
            }
        }
    }
    return base_id_best;
}

/*
Purpose: Determine the faction's best base to attack the specified base from.
// ORIGINAL: 0x0054AFA0 ?attack_from@@YAHHH@Z 0x0054AFA0-0x0054B137 BYTE_EXACT
// size      407 bytes
// prototype int (__cdecl ?attack_from@@YAHHH@Z)(int baseID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
// LEVER: open-coded the vector_dist(x, y, Bases[base_id_to_atk].x,
//        Bases[base_id_to_atk].y) ternary branch as the x_dist()+abs()x4
//        expansion (same shape as base_find 0x004E3B80/0x004E3D50) - the
//        image never calls vector_dist, only abs() x4. Took best similarity
//        from 3/135 to 4/135 agreeing (0.714 similar, /c /O2 /Oi- /Gy /GR-
//        /Oy- /GX). Unlike suggest_plan (TRIED above), this one measured
//        better, not worse - the two bodies disagree on the lever.
Return Value: Base id to attack from or 0
Status: Complete
*/
int __cdecl attack_from(int base_id_to_atk, int faction_id) {
    int search = 9999;        // value to prevent the edge case of base id 0 being incorrect target
    int base_id_atk_from = 0; // TODO: eventually change this to -1 and add handling to the return
    int i = 0; // initialized before region_src: keeps i out of base_id_to_atk's slot
    uint32_t region_src = region_at(Bases[base_id_to_atk].x, Bases[base_id_to_atk].y);
    for (; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            // Inlined vector_dist: the image calls abs() four times.
            int compare = vector_dist(Bases[i].x, Bases[i].y, Bases[base_id_to_atk].x,
                                         Bases[base_id_to_atk].y);
            if (Bases[i].x == PlayersData[faction_id].x_target
                && Bases[i].y == PlayersData[faction_id].y_target) {
                compare = 0;
            }
            if (region_at(Bases[i].x, Bases[i].y) != region_src) {
                compare += 1000;
            }
            if (compare <= search) {
                search = compare;
                base_id_atk_from = i;
            }
        }
    }
    return base_id_atk_from;
}

// ORIGINAL: 0x0054B140 ?battle_routine@@YAXH@Z 0x0054B140-0x0054B1B6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x0054B140:0054B140-0054B1B6", "source:src/planwin.h:PlanWin and genuine MapWin/GraphicWin interfaces", "source:src/faction.h:PlayerData, battle faction state and label declarations", "recovery/tasks/planwin-battle-owner-coverage.json"], "reason": "Use the pinned native body with the real typed owner and preserve the seed's original identity and history."}
void __cdecl battle_routine(int base_id) {
    const int first_attack = attack_from(base_id, PlanBattleFactionA);
    const int second_attack = attack_from(base_id, PlanBattleFactionB);

    PlanWindow.clear_lines();
    PlanWindow.add_line(first_attack, base_id,
                        InfoFactionTextColors[PlanBattleFactionA]);
    PlanWindow.add_line(second_attack, base_id,
                        InfoFactionTextColors[PlanBattleFactionB]);
    PlanWindow.on_redraw();
}

// ORIGINAL: 0x0054DCF0 ?proposal_menu@@YAHHH@Z 0x0054DCF0-0x0054F412;0x0065FAE9-0x0065FD5F
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned54DCF0-54F412 body and65FAE9-65FD5F generated cleanup; native modal keys and selection CFG", "Native54E25F proposalFA34;54EE5D counterofferFAB0;54E419 loopindex8=>other.agenda0;54E9A8/54EFB9 ListBox selection callback54B140", "Fresh COFF Base.visibility0A,facilities8C; Player59C;PlayerData20CC; existing Popup/Dialogs/PlanWin owners", "historical seed:src/unrecovered/0054dcf0.cpp"], "reason": "Recover the complete staged faction proposal and counteroffer menus with a real lexical Popup, typed faction/base/project filters, shared original proposal and counteroffer selectors, PlanWin base previews and ListBox selection notifications. Preserve original loop-index reuse through the declared agenda word rather than an out-of-bounds treaty array view. Preserve both original body and generated cleanup spans and seed findings; compiling coverage is not byte-exact or game acceptance."}
// symbol    ?proposal_menu@@YAHHH@Z
// TRIED: byte-exact prologue global-write order (a64cc load/cmp interleaved between fa30/fa7c/f7cc/fab0/fa34/fa5c in the original, sequential here); the six-panel Popup/Dialogs dispatch, item()/exec() gating and faction-loop strides are transcribed from raw disasm + the (uncorrupted for this function) Ghidra hypothesis cross-checked against it - diverges first at #11, in the prologue.
int __cdecl proposal_menu(int faction_id, int faction_id_with) {
    Popup popup;

    DiploScratchFA30 = -1;
    DiploScratchFA7C = -1;
    PopupDialogFactionID = faction_id_with;
    DiplomacyProposalMode = 0;
    DiploScratchFA34 = 0;
    DiploScratchFA5C = 0;

    int bases_without_facility = 0;
    if (GameDataStateStorage.fields_.base_current_count_ > 0) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            Base &base = Bases[base_id];
            if (base.faction_id_current == faction_id_with &&
                (base.faction_id_current == faction_id ||
                 (base.visibility & (1 << (faction_id & 0x1f))) != 0)) {
                int facility_byte = 0;
                int facility_bit = 0;
                bitmask(1, &facility_byte, &facility_bit);
                if ((base.facilities_built[facility_byte] & facility_bit) == 0)
                    ++bases_without_facility;
            }
        }
    }

    int eligible_other_factions = 0;
    for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        const unsigned char alive_mask =
            static_cast<unsigned char>((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xff);
        if ((alive_mask & (1 << (candidate & 0x1f))) != 0 &&
            candidate != faction_id && candidate != faction_id_with) {
            const uint32_t candidate_treaty =
                PlayersData[candidate].diplo_treaties[faction_id];
            if ((candidate_treaty & 8u) == 0 &&
                (GameDataStateStorage.fields_.secret_project_.empath_guild() < 0 ||
                 Bases[GameDataStateStorage.fields_.secret_project_.empath_guild()].faction_id_current ==
                     faction_id)) {
                ++eligible_other_factions;
            }
        }
    }

    parse_num(0, PlayersData[faction_id].loan_balance[faction_id_with]);
    diplomacy_caption(faction_id, faction_id_with);

    popup.start(PopupStartCaption, "PROPOSAL", -1, nullptr, 0x100000, 0);
    popup.set_caller_picture(popup.picture_enabled()
        ? nullptr : FactionDiplomacyPortraits[faction_id_with]);
    popup.dialogs().sprite(FactionDiplomacyPortraits[faction_id_with],
                           nullptr, 0);
    if (X_text_open(ScriptPopupFile, "PROPOSALMENU") != 0) return 0;

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    popup.dialogs().item(StringTemp, 0);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    popup.dialogs().item(StringTemp, 1);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with] & 2u) != 0 &&
        (PlayersData[faction_id].diplo_treaties[faction_id_with] & 1u) == 0)
        popup.dialogs().item(StringTemp, 2);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with] & 2u) == 0)
        popup.dialogs().item(StringTemp, 3);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    if ((GameDataStateStorage.fields_.game_rules_ & 0x02000000u) == 0)
        popup.dialogs().item(StringTemp, 4);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    if (bases_without_facility != 0)
        popup.dialogs().item(StringTemp, 13);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    popup.dialogs().item(StringTemp, 6);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    const int offered_value =
        PlayersData[faction_id].loan_balance[faction_id_with];
    if (offered_value != 0 &&
        offered_value <= PlayersData[faction_id].energy_reserves)
        popup.dialogs().item(StringTemp, 11);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    if (PlayersData[faction_id_with].current_num_bases != 0)
        popup.dialogs().item(StringTemp, 8);

    text_get();
    StringTemp[0] = 0;
    parse_string(TextBufferGetPtr, StringTemp);
    if (PlayersData[faction_id].current_num_bases > 1 &&
        eligible_other_factions != 0 && IsMultiplayerNet == 0)
        popup.dialogs().item(StringTemp, 9);

    DiploScratchFA34 = popup.exec(0, pop_wait);

    if (DiploScratchFA34 == 8) {
        popup.start(PopupStartCaption, "PROPOSEATTACK", -1, nullptr,
                    0x100040, 0);
        popup.set_caller_picture(popup.picture_enabled()
            ? nullptr : FactionDiplomacyPortraits[faction_id_with]);
        popup.dialogs().sprite(FactionDiplomacyPortraits[faction_id_with],
                               nullptr, 0);

        if (X_text_open(ScriptPopupFile, "ATTACKSTRING") == 0) {
            text_get();
            for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
                const unsigned char alive_mask =
                    static_cast<unsigned char>((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xff);
                const uint32_t target_flags =
                    PlayersData[faction_id].diplo_treaties[candidate];
                const bool candidate_is_alive =
                    (alive_mask & (1 << (candidate & 0x1f))) != 0;
                const bool target_is_human =
                    (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << (candidate & 0x1f))) != 0;
                if (candidate_is_alive && candidate != faction_id &&
                    candidate != faction_id_with &&
                    ((target_flags & 8u) != 0 || target_is_human)) {
                    GenderDefault = Players[candidate].is_leader_female;
                    PluralityDefault = 0;
                    parse_says(0, Players[candidate].title_leader, -1, -1);
                    GenderDefault = Players[candidate].is_leader_female;
                    PluralityDefault = 0;
                    parse_says(1, Players[candidate].name_leader, -1, -1);
                    GenderDefault = Players[candidate].noun_gender;
                    PluralityDefault = Players[candidate].is_noun_plural;
                    parse_says(2, Players[candidate].noun_faction, -1, -1);
                    StringTemp[0] = 0;
                    parse_string(TextBufferGetPtr, StringTemp);
                    popup.dialogs().item(StringTemp, candidate);
                }
            }

            DiploScratchFA5C = popup.exec(0, pop_wait);
            if (DiploScratchFA5C >= 0) {
                // The original reuses the completed loop index8 here, reading
                // other.treaties[8], which is this declared agenda word.
                if ((PlayersData[faction_id_with].diplo_agenda[0] & 0x10u) != 0) {
                    const int target = DiploScratchFA5C;
                    GenderDefault = Players[target].is_leader_female;
                    PluralityDefault = 0;
                    parse_says(0, Players[target].title_leader, -1, -1);
                    GenderDefault = Players[target].is_leader_female;
                    PluralityDefault = 0;
                    parse_says(1, Players[target].name_leader, -1, -1);
                    X_pops("REJWARALREADY", 0x100000,
                                 FactionDiplomacyPortraits[faction_id_with],
                                 pop_wait);
                    DiploScratchFA34 = 0;
                    goto return_selection;
                }
            } else {
                DiploScratchFA34 = 0;
                goto return_selection;
            }
        } else {
            DiploScratchFA34 = 0;
            goto return_selection;
        }
    } else if (DiploScratchFA34 == 13) {
        popup.start(PopupStartCaption, "PROPOSECOMMLINK", -1, nullptr,
                    0x100040, 0);
        popup.set_caller_picture(popup.picture_enabled()
            ? nullptr : FactionDiplomacyPortraits[faction_id_with]);
        popup.dialogs().sprite(FactionDiplomacyPortraits[faction_id_with],
                               nullptr, 0);

        if (X_text_open(ScriptPopupFile, "ATTACKSTRING") != 0) {
            DiploScratchFA34 = 0;
            goto return_selection;
        }

        text_get();
        for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
            const unsigned char alive_mask =
                static_cast<unsigned char>((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xff);
            const uint32_t target_flags =
                PlayersData[faction_id].diplo_treaties[candidate];
            const bool candidate_is_alive =
                (alive_mask & (1 << (candidate & 0x1f))) != 0;
            if (candidate_is_alive && candidate != faction_id &&
                candidate != faction_id_with && (target_flags & 8u) == 0 &&
                (GameDataStateStorage.fields_.secret_project_.empath_guild() < 0 ||
                 Bases[GameDataStateStorage.fields_.secret_project_.empath_guild()].faction_id_current !=
                     faction_id_with)) {
                GenderDefault = Players[candidate].is_leader_female;
                PluralityDefault = 0;
                parse_says(0, Players[candidate].title_leader, -1, -1);
                GenderDefault = Players[candidate].is_leader_female;
                PluralityDefault = 0;
                parse_says(1, Players[candidate].name_leader, -1, -1);
                GenderDefault = Players[candidate].noun_gender;
                PluralityDefault = Players[candidate].is_noun_plural;
                parse_says(2, Players[candidate].noun_faction, -1, -1);
                StringTemp[0] = 0;
                parse_string(TextBufferGetPtr, StringTemp);
                popup.dialogs().item(StringTemp, candidate);
            }
        }

        DiploScratchFA5C = popup.exec(0, pop_wait);
        if (DiploScratchFA5C < 0) {
            DiploScratchFA34 = 0;
            goto return_selection;
        }

        const uint32_t target_flags =
            PlayersData[faction_id_with]
                .diplo_treaties[DiploScratchFA5C];
        if ((target_flags & 8u) == 0 &&
            (GameDataStateStorage.fields_.secret_project_.empath_guild() < 0 ||
             Bases[GameDataStateStorage.fields_.secret_project_.empath_guild()].faction_id_current !=
                 faction_id_with)) {
            const int target = DiploScratchFA5C;
            GenderDefault = Players[target].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[target].title_leader, -1, -1);
            GenderDefault = Players[target].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[target].name_leader, -1, -1);
            GenderDefault = Players[target].noun_gender;
            PluralityDefault = Players[target].is_noun_plural;
            parse_says(2, Players[target].noun_faction, -1, -1);
            DiploScratchFA34 = 0;
            x_pops_minimal("NOCOMMLINK",
                           FactionDiplomacyPortraits[faction_id_with], pop_wait);
        }
    } else if (DiploScratchFA34 == 9) {
        PlanWindow.init(nullptr);
        PlanWindow.back_redraw();
        popup.start(PopupStartCaption, "PROPOSEBASE", -1, nullptr,
                    0x100040, 0);
        popup.set_caller_picture(popup.picture_enabled()
            ? nullptr : FactionDiplomacyPortraits[faction_id_with]);
        popup.dialogs().sprite(FactionDiplomacyPortraits[faction_id_with],
                               nullptr, 0);

        bool has_base = false;
        int item_id = 0;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
                Base &base = Bases[base_id];
                if (base.faction_id_current == faction_id_with) {
                    int facility_byte = 0;
                    int facility_bit = 0;
                    bitmask(1, &facility_byte, &facility_bit);
                    if ((base.facilities_built[facility_byte] & facility_bit) ==
                        0) {
                        StringTemp[0] = 0;
                        say_base(StringTemp, item_id);
                        popup.dialogs().item(StringTemp, item_id);
                        if (!has_base) {
                            has_base = true;
                            PlanWindow.clear_lines();
                            PlanWindow.monocenter(base.x, base.y, 0);
                            PlanWindow.add_line(base_id, base_id, 0);
                            PlanWindow.on_redraw();
                        }
                    }
                }
                ++item_id;
            }
        }

        if (!has_base) {
            DiploScratchFA7C = -1;
            DiploScratchFA34 = 0;
        } else {
            PlanWindow.show(0);
            popup.dialogs().ListBox::set_selection_callback(battle_routine);
            DiploScratchFA7C = popup.exec(0, pop_wait);
            if (DiploScratchFA7C < 0) DiploScratchFA34 = 0;
        }
        PlanWindow.close();
    }

    if (DiploScratchFA34 != 0 && DiploScratchFA34 != 7 &&
        DiploScratchFA34 != 10 && DiploScratchFA34 != 11 &&
        DiploScratchFA34 != 5 && DiploScratchFA34 != 12) {
        GenderDefault = Players[faction_id_with].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[faction_id_with].title_leader, -1, -1);
        GenderDefault = Players[faction_id_with].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction_id_with].name_leader, -1, -1);
        parse_says(2, get_pact_hood(faction_id, faction_id_with), -1, -1);
        diplomacy_caption(faction_id, faction_id_with);
        StringTemp[0] = 0;
        strcat(StringTemp, "COUNTER");
        char counter_text[16];
        _itoa(DiploScratchFA34 == 1, counter_text, 10);
        strcat(StringTemp, counter_text);

        popup.start(PopupStartCaption, StringTemp, -1, nullptr,
                    0x100000, 0);
        popup.set_caller_picture(popup.picture_enabled()
            ? nullptr : FactionDiplomacyPortraits[faction_id_with]);
        popup.dialogs().sprite(FactionDiplomacyPortraits[faction_id_with],
                               nullptr, 0);
        if (X_text_open(ScriptPopupFile, "COUNTERMENU") != 0) return 0;

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        popup.dialogs().item(StringTemp, 0);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if (DiploScratchFA34 == 6)
            popup.dialogs().item(StringTemp, 6);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if (DiploScratchFA34 != 1)
            popup.dialogs().item(StringTemp, 1);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if (DiploScratchFA34 != 1)
            popup.dialogs().item(StringTemp, 2);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if (DiploScratchFA34 != 1 &&
            (PlayersData[faction_id].diplo_treaties[faction_id_with] & 1u) ==
                0)
            popup.dialogs().item(StringTemp, 3);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if (DiploScratchFA34 != 1 &&
            (PlayersData[faction_id].diplo_treaties[faction_id_with] & 1u) !=
                0)
            popup.dialogs().item(StringTemp, 3);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if ((GameDataStateStorage.fields_.game_rules_ & 0x02000000u) == 0)
            popup.dialogs().item(StringTemp, 4);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if (DiploScratchFA34 == 1)
            popup.dialogs().item(StringTemp, 7);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if (DiploScratchFA34 != 6)
            popup.dialogs().item(StringTemp, 5);

        text_get();
        StringTemp[0] = 0;
        parse_string(TextBufferGetPtr, StringTemp);
        if (PlayersData[faction_id].current_num_bases > 1 &&
            IsMultiplayerNet == 0 &&
            (DiploScratchFA34 == 9 || DiploScratchFA34 == 1))
            popup.dialogs().item(StringTemp, 8);

        DiplomacyProposalMode = popup.exec(0, pop_wait);
        if (DiplomacyProposalMode == 8) {
            PlanWindow.init(nullptr);
            PlanWindow.back_redraw();
            popup.start(PopupStartCaption, "COUNTERBASE", -1, nullptr,
                        0x100040, 0);
            popup.set_caller_picture(popup.picture_enabled()
                ? nullptr : FactionDiplomacyPortraits[faction_id_with]);
            popup.dialogs().sprite(FactionDiplomacyPortraits[faction_id_with],
                                   nullptr, 0);

            bool has_counter_base = false;
            int counter_item_id = 0;
            if (GameDataStateStorage.fields_.base_current_count_ > 0) {
                for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
                    Base &base = Bases[base_id];
                    if (base.faction_id_current == faction_id) {
                        int facility_byte = 0;
                        int facility_bit = 0;
                        bitmask(1, &facility_byte, &facility_bit);
                        if ((base.facilities_built[facility_byte] &
                             facility_bit) == 0) {
                            StringTemp[0] = 0;
                            say_base(StringTemp, counter_item_id);
                            popup.dialogs().item(StringTemp, counter_item_id);
                            if (!has_counter_base) {
                                has_counter_base = true;
                                PlanWindow.clear_lines();
                                PlanWindow.monocenter(base.x, base.y, 0);
                                PlanWindow.add_line(base_id, base_id, 0);
                                PlanWindow.on_redraw();
                            }
                        }
                    }
                    ++counter_item_id;
                }
            }

            PlanWindow.show(0);
            popup.dialogs().ListBox::set_selection_callback(battle_routine);
            DiploScratchFA30 = popup.exec(0, pop_wait);
            if (DiploScratchFA30 < 0) DiploScratchFA34 = 0;
            PlanWindow.close();
        }

        if (DiplomacyProposalMode == 0) DiploScratchFA34 = 0;
        return DiploScratchFA34;
    }

return_selection:
    DiplomacyProposalMode = 1;
    return DiploScratchFA34;
}

// ORIGINAL: 0x00545E80 ?propose_attack@@YAHHH@Z 0x00545E80-0x00547C08;0x0065F1B4-0x0065F4C4
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned545E80-547C08 body and65F1B4-65F4C4 cleanup", "Native546B5E jumps546B9C: PEACE cutoffs reuse updated total; 5466BE offline547397 vs network54739F", "Native54755C outcome actor-to-target;5475A4 friction target-to-actor;54741D setter other-to-actor", "Fresh COFF Player59C andPlayerData20CC; pair members12C/14C; region_total_bases117C", "historical seed:src/unrecovered/00545e80.cpp"], "reason": "Recover the complete incitement negotiation in its faction owner with a lexical Popup, typed regional and diplomatic state, technology and credit offers, and original lock/transfer/treaty/result ordering. Preserve original primary and generated cleanup spans and seed identity. This is compiling recovery, not byte matching or game acceptance."}
// symbol    ?propose_attack@@YAHHH@Z
int __cdecl propose_attack(int actor, int other) {
// Native EBP-0x540C is the real automatic Popup local. Its constructor and
// destructor stay in this lexical scope across every early return.
Popup popup;

// Native 545E80 captures the target that the caller already selected in
// DiploScratchFA5C. There is no target-selection menu in propose_attack.
const int actor_faction_id = actor;
const int other_faction_id = other;
const int target = DiploScratchFA5C;
const int selected_target_faction_id = target;
PopupDialogFactionID = other_faction_id;

// At 545EE0 the directed row is PlayersData[other][selected target]. A
// pre-existing vendetta against that target takes the early refusal branch
// at 545EF0..545F97. The native code parses the selected target's leader
// fields, then displays its refusal popup with the other faction's portrait.
const uint32_t other_to_target_treaty =
    PlayersData[other_faction_id]
        .diplo_treaties[selected_target_faction_id];
const int other_to_target_pact_bit =
    static_cast<int>(other_to_target_treaty & DTREATY_PACT);
if ((other_to_target_treaty & DTREATY_VENDETTA) != 0) {
    GenderDefault = Players[selected_target_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[selected_target_faction_id].title_leader, -1, -1);
    GenderDefault = Players[selected_target_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[selected_target_faction_id].name_leader, -1, -1);
    X_pops("REJWARALREADY", 0x100000,
                 FactionDiplomacyPortraits[other_faction_id], pop_wait);
    // The lexical Popup/helper objects run the same original cleanup order.
    return 0;
}

// The next refusal is bypassed if the other faction already has a commlink
// to the selected target. Otherwise native calls contiguous(other, target,
// region 0, mode 1), then checks the other-to-actor pact bit.
int target_is_contiguous = 1;
if ((other_to_target_treaty & DTREATY_COMMLINK) == 0) {
    target_is_contiguous =
        contiguous(other_faction_id, selected_target_faction_id, 0, 1);
}
const int other_has_actor_pact =
    (PlayersData[other_faction_id]
         .diplo_treaties[actor_faction_id] & DTREATY_PACT) != 0;
if ((other_to_target_treaty & DTREATY_COMMLINK) == 0 &&
    target_is_contiguous == 0 &&
    !other_has_actor_pact) {
    // Native 545FD1..546079 parses only the actor's leader title and the
    // selected target's adj_name_faction, then displays REJWARFAR.
    GenderDefault = Players[actor_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[actor_faction_id].title_leader, -1, -1);
    parse_says(2, Players[selected_target_faction_id].adj_name_faction,
               -1, -1);
    X_pops("REJWARFAR", 0x100000,
                 FactionDiplomacyPortraits[other_faction_id], pop_wait);
    return 0;
}

// Native 54607E..5460D8 assigns a policy class by scanning region IDs 1..127.
// Its byte reads are PlayerData::region_total_bases at +0x117C (not the force
// rating array at +0x127C). The shipped scan checks whether both factions
// have at least one base in any same region.
int policy_class = 0;
if ((other_to_target_treaty & DTREATY_COMMLINK) == 0 &&
    target_is_contiguous == 0 && other_has_actor_pact) {
    policy_class = 2;
    for (int region = 1; region < 128; ++region) {
        if ((region & 0x3f) == 0 || (region & 0x3f) == 0x3f) continue;
        if (PlayersData[other_faction_id].region_total_bases[region] != 0 &&
            PlayersData[selected_target_faction_id]
                .region_total_bases[region] != 0) {
            policy_class = 1;
            break;
        }
    }
}

// Native score, policy, and immediate WARPACT branch (5460DF..546635).
// diplo_* uses unsigned operations then converts to int, preserving the
// native signed-32 wrap without signed-overflow undefined behavior.
int proposal_attack_score = diplo_sub(
    DiploScratchFA78,
    PlayersData[selected_target_faction_id].integrity_blemishes);
proposal_attack_score = diplo_mul(
    proposal_attack_score, GameDataStateStorage.fields_.turn_current_num_ / 10);
proposal_attack_score = diplo_add(
    proposal_attack_score,
    diplo_mul(PlayersData[selected_target_faction_id].mil_strength_1, 5));
proposal_attack_score = diplo_mul(
    proposal_attack_score,
    diplo_sub(2, PlayersData[other_faction_id].ai_fight));

const int target_rank =
    PlayersData[selected_target_faction_id].ranking;
const int other_rank =
    PlayersData[other_faction_id].ranking;
const int actor_rank = PlayersData[actor_faction_id].ranking;
if (target_rank > other_rank && target_rank > actor_rank &&
    DiploScratchF7FC == 0 && (actor_rank >= 4 || target_rank >= 4)) {
    proposal_attack_score /= 2;
}

if (selected_target_faction_id == static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[7])) {
    const int target_power = FactionRankingsUnk[selected_target_faction_id];
    const int next_ranked_faction = static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[6]);
    const int next_ranked_power = FactionRankingsUnk[next_ranked_faction];
    if (target_power >= diplo_mul(next_ranked_power, 2)) {
        proposal_attack_score /= 4;
    } else if (great_satan(selected_target_faction_id, 0) != 0) {
        proposal_attack_score /= 3;
    } else if (diplo_mul(next_ranked_power, 3) <=
               diplo_mul(target_power, 2)) {
        proposal_attack_score /= 2;
    }
}

if ((PlayersData[other_faction_id].flags & 0x00400000u) != 0)
    proposal_attack_score /= 2;

const int other_to_target_pact = other_to_target_pact_bit != 0;
const uint32_t other_to_actor_treaty =
    PlayersData[other_faction_id].diplo_treaties[actor_faction_id];
const int other_to_actor_vendetta_bit =
    static_cast<int>(other_to_actor_treaty & DTREATY_VENDETTA);
if (other_to_target_pact) {
    proposal_attack_score = diplo_mul(
        proposal_attack_score,
        diplo_sub(3, PlayersData[other_faction_id].ai_fight));
    if (actor_rank >= 6 && target_rank >= 5 && DiploScratchF7F0 == 0 &&
        other_rank <= 4 && GameDataStateStorage.fields_.turn_current_num_ >= 50 &&
        (DiploScratchFA54 != 0 ||
         PlayersData[actor_faction_id].integrity_blemishes > 2)) {
        proposal_attack_score = diplo_add(
            proposal_attack_score, proposal_attack_score);
    }
} else if ((other_to_actor_treaty & DTREATY_PACT) != 0) {
    proposal_attack_score /= 2;
}

const uint32_t actor_to_target_treaty =
    PlayersData[actor_faction_id].diplo_treaties[selected_target_faction_id];
const uint32_t target_to_actor_treaty =
    PlayersData[selected_target_faction_id].diplo_treaties[actor_faction_id];
if (PlayersData[selected_target_faction_id].integrity_blemishes >
    PlayersData[actor_faction_id].integrity_blemishes) {
    proposal_attack_score /= 2;
    if ((actor_to_target_treaty & DTREATY_WANT_REVENGE) != 0 &&
        (other_to_actor_treaty & DTREATY_WANT_REVENGE) == 0 &&
        (target_to_actor_treaty & DTREATY_WANT_REVENGE) == 0) {
        proposal_attack_score /= 2;
    }
}

if (DiploScratchFA3C != 0)
    proposal_attack_score = diplo_add(
        proposal_attack_score, proposal_attack_score);
if ((other_to_actor_treaty &
     (DTREATY_PACT | DTREATY_HAVE_SURRENDERED)) ==
    (DTREATY_PACT | DTREATY_HAVE_SURRENDERED)) {
    proposal_attack_score /= 2;
}

// Native 5463A8 reads the byte at 0x009A64E8, i.e. LockEnableMask byte 0
// (the existing human-faction mask), then tests the selected target bit.
const uint32_t human_factions = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
const uint32_t target_faction_bit =
    1u << (selected_target_faction_id & 0x1f);
if ((actor_to_target_treaty & DTREATY_VENDETTA) == 0 &&
    (human_factions & target_faction_bit) == 0) {
    proposal_attack_score = diplo_add(
        proposal_attack_score, proposal_attack_score / 2);
}

if (target_rank >= 6 && other_rank >= 6) proposal_attack_score /= 2;

// For a pairwise field indexed by the PlayerData stride, use the declared
// members directly. Native compares the target row's diplo_unk1/diplo_wrongs
// at this point; the target's difference plus one scales the current score.
const int target_trade_favor =
    PlayersData[selected_target_faction_id].diplo_unk1[other_faction_id];
const int target_trade_wrong =
    PlayersData[selected_target_faction_id].diplo_wrongs[other_faction_id];
if (target_trade_favor > target_trade_wrong) {
    const int target_favor_delta =
        diplo_add(diplo_sub(target_trade_favor, target_trade_wrong), 1);
    proposal_attack_score = diplo_mul(proposal_attack_score,
                                      target_favor_delta);
}

const int score_after_class = diplo_mul(
    diplo_sub(4, policy_class), proposal_attack_score) / 4;
const int proposal_attack_threshold = diplo_mul(
    diplo_add(score_after_class, 24) / 25, 25);

const uint32_t target_to_other_treaty =
    PlayersData[selected_target_faction_id]
        .diplo_treaties[other_faction_id];
const int proposal_mode = DiplomacyProposalMode;
const int proposal_attack_special_gate =
    (target_to_other_treaty & DTREATY_PACT) != 0 &&
    (proposal_mode == 1 || proposal_mode == 3) &&
    (actor_to_target_treaty & DTREATY_VENDETTA) != 0 &&
    !other_to_target_pact;

int proposal_attack_must_double_cross = 0;
if (proposal_attack_special_gate) {
    const int other_wants_attack = wants_to_attack(
        other_faction_id, selected_target_faction_id, 0);
    if (other_wants_attack != 0) {
        proposal_attack_must_double_cross = 1;
    } else if (wants_to_attack(selected_target_faction_id,
                               other_faction_id, 0) != 0) {
        proposal_attack_must_double_cross = 1;
    } else {
        const uint32_t actor_to_target_after_policy =
            PlayersData[actor_faction_id]
                .diplo_treaties[selected_target_faction_id];
        const int target_rank_after_policy = static_cast<int>(
            PlayersData[selected_target_faction_id].ranking);
        const int actor_rank_after_policy = static_cast<int>(
            PlayersData[actor_faction_id].ranking);
        const int actor_to_target_asks_for_revenge =
            (actor_to_target_after_policy & DTREATY_WANT_REVENGE) != 0;
        if (target_rank_after_policy > actor_rank_after_policy &&
            (!actor_to_target_asks_for_revenge ||
             target_rank_after_policy > other_rank)) {
            proposal_attack_must_double_cross = 1;
        }
    }
}

if (proposal_attack_must_double_cross) {
    GenderDefault = Players[selected_target_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[selected_target_faction_id].title_leader, -1, -1);
    GenderDefault = Players[selected_target_faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[selected_target_faction_id].name_leader, -1, -1);
    GenderDefault = Players[selected_target_faction_id].noun_gender;
    PluralityDefault = Players[selected_target_faction_id].is_noun_plural;
    parse_says(2, Players[selected_target_faction_id].noun_faction, -1, -1);
    parse_says(3, get_pact(actor_faction_id), -1, -1);
    X_pops("WARPACT", 0x100000,
                 FactionDiplomacyPortraits[other_faction_id], pop_wait);
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingDOUBLECROSS, actor_faction_id,
                selected_target_faction_id, other_faction_id);
        message_data(0x2440, 0, other_faction_id,
                     selected_target_faction_id, actor_faction_id, 0);
        NetDaemonNet.await_diplo(0x440);
    } else {
        double_cross(other_faction_id, selected_target_faction_id,
                     actor_faction_id);
    }
    return 0;
}

// This threshold is consumed by the common diplomacy-policy body from
// 546640. Its EBP frame reuses [ebp-0x14], which has already changed from the
// temporary other-row base index used earlier in this function.
const int proposal_attack_policy_threshold = proposal_attack_threshold;

const int attack_cached_mode = DiplomacyProposalMode;
const uint32_t attack_cached_other_actor_treaty =
    PlayersData[other].diplo_treaties[actor];

// Native 546640-5466E6 requires both the pair's pact bit and
// HAVE_SURRENDERED, and only enters the diplomatic-lock transaction for
// proposal modes 1 and 3. Keep this cached native condition intact while the
// network loop calls helpers that may update diplomacy state.
if ((attack_cached_other_actor_treaty & DTREATY_PACT) != 0 &&
    (attack_cached_other_actor_treaty & DTREATY_HAVE_SURRENDERED) != 0 &&
    (attack_cached_mode == 3 || attack_cached_mode == 1)) {
attack_retry:
    log_say(DiploLabelGettingDIPLOMATICLOCK, 0, 0, 0);
    timeGetTime();
    while (ExitTurnLoop == 0 && NetDaemonNet.add_global() != 0) {
        check_spock();
        timeGetTime();
    }

    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYON, other, target, 0x80000018);
        message_data(0x2441, 0, other, target, 0x80000018, 0);
        // The native network edge skips the offline treaty_on call at 547397.
        goto attack_accept_treaty_network;
    }
    goto attack_accept_treaty;
}

// The first refusal family is selected by the directed other->actor treaty
// word. WANT_REVENGE and FA78 are tested in their original order. The mode-3
// cleanup edge has no direct presentation call in this range; retain its
// zero-return transition rather than substituting a guessed popup.
if ((attack_cached_other_actor_treaty & DTREATY_PACT) == 0) {
    if ((attack_cached_other_actor_treaty & DTREATY_WANT_REVENGE) != 0 ||
        DiploScratchFA78 > 5) {
        if (attack_cached_mode == 3) {
            goto attack_return_zero;
        }
        GenderDefault = Players[actor].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[actor].title_leader, -1, -1);
        GenderDefault = Players[actor].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[actor].name_leader, -1, -1);
        GenderDefault = Players[target].noun_gender;
        PluralityDefault = Players[target].is_noun_plural;
        parse_says(2, Players[target].noun_faction, -1, -1);
        X_pops("REJWARTRUST", 0x100000,
                     FactionDiplomacyPortraits[other], pop_wait);
        goto attack_return_zero;
    }
}

// Modes 1 and 3 alone apply the paired wants_to_attack policy. Preserve its
// call order and the later directed actor->target vendetta test: either the
// first call or the full second-call condition enters the shared trade/offer
// block at 546A20.
if (attack_cached_mode == 1 || attack_cached_mode == 3) {
    if (wants_to_attack(other, target, 0) != 0)
        goto attack_trade_offer;

    if ((attack_cached_other_actor_treaty & DTREATY_PACT) != 0 ||
        DiploScratchFA98 != 0) {
        if (wants_to_attack(target, other, 0) != 0 &&
            (PlayersData[actor].diplo_treaties[target] &
             DTREATY_VENDETTA) != 0) {
            goto attack_trade_offer;
        }
    }

    // Unlike the earlier EDX-based mode gates, 54689E reloads the global after
    // both wants_to_attack calls. Preserve that fresh observation.
    if (DiplomacyProposalMode == 3) goto attack_return_zero;

    GenderDefault = Players[actor].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[actor].title_leader, -1, -1);
    X_pops("REJWARNOBRIBE", 0x100000,
                 FactionDiplomacyPortraits[other], pop_wait);
    goto attack_return_zero;
}

goto attack_trade_offer;

// Technology and credit offers (native 546A20-547397).
attack_trade_offer:
{
    int transaction_accepted = 0;
    int offer_shown = 0;
    const int price = proposal_attack_policy_threshold;
    int technology_value = 0;
    if (DiploScratchFAA8 >= 0) {
        const int value = tech_val(DiploScratchFAA8, other, 1);
        technology_value = diplo_mul(value, diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8);
    }
    int first_value = technology_value;
    if (DiploScratchFA18 >= 0) {
        const int value = tech_val(DiploScratchFA18, other, 1);
        technology_value = diplo_add(technology_value,
            diplo_mul(value, diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8));
    }
    int second_value = technology_value;
    if (DiploScratchFA1C >= 0) {
        const int value = tech_val(DiploScratchFA1C, other, 1);
        technology_value = diplo_add(technology_value,
            diplo_mul(value, diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8));
    }
    int third_value = technology_value;
    if (DiploScratchFA28 >= 0) {
        const int value = tech_val(DiploScratchFA28, other, 1);
        technology_value = diplo_add(technology_value,
            diplo_mul(value, diplo_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8));
    }
    if (strcmp(Players[other].filename, "UNIV") == 0) {
        technology_value = diplo_mul(technology_value, 2);
        first_value = diplo_mul(first_value, 2);
        second_value = diplo_mul(second_value, 2);
        third_value = diplo_mul(third_value, 2);
    } else if (strcmp(Players[other].filename, "PEACE") == 0) {
        technology_value = diplo_mul(technology_value, 3) / 2;
        // Original 546B50 reuses the UPDATED total for all three cutoffs.
        first_value = second_value = third_value =
            diplo_mul(technology_value, 3) / 2;
    } else if (strcmp(Players[other].filename, "MORGAN") == 0) {
        technology_value /= 2;
        first_value /= 2;
        second_value /= 2;
        third_value /= 2;
    }
    const int cutoff = diplo_mul(price, 4) / 3;
    if (third_value >= cutoff) DiploScratchFA28 = -1;
    if (second_value >= cutoff) DiploScratchFA1C = -1;
    if (first_value >= cutoff) DiploScratchFA18 = -1;

    if (DiploScratchFAA8 >= 0 &&
        strcmp(Players[other].filename, "BELIEVE") != 0 &&
        technology_value >= price &&
        (DiplomacyProposalMode == 4 ||
         (DiplomacyProposalMode == 2 &&
          (PlayersData[other].energy_reserves > PlayersData[actor].energy_reserves ||
           technology_value >= diplo_mul(price, 3) / 2)))) {
        GenderDefault = Players[target].noun_gender;
        PluralityDefault = Players[target].is_noun_plural;
        parse_says(0, Players[target].noun_faction, -1, -1);
        GenderDefault = Players[target].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[target].title_leader, -1, -1);
        GenderDefault = Players[target].is_leader_female;
        PluralityDefault = 0;
        parse_says(2, Players[target].name_leader, -1, -1);
        StringTemp[0] = '\0';
        say_tech(StringTemp, DiploScratchFAA8, 1);
        parse_says(3, StringTemp, -1, -1);
        if (DiploScratchFA18 >= 0) {
            StringTemp[0] = '\0';
            say_tech(StringTemp, DiploScratchFA18, 1);
            parse_says(4, StringTemp, -1, -1);
        }
        if (DiploScratchFA1C >= 0) {
            StringTemp[0] = '\0';
            say_tech(StringTemp, DiploScratchFA1C, 1);
            parse_says(8, StringTemp, -1, -1);
        }
        if (DiploScratchFA28 >= 0) {
            StringTemp[0] = '\0';
            say_tech(StringTemp, DiploScratchFA28, 1);
            parse_says(9, StringTemp, -1, -1);
        }
        GenderDefault = Players[other].is_leader_female;
        PluralityDefault = 0;
        parse_says(5, Players[other].title_leader, -1, -1);
        parse_says(6, Players[target].adj_name_faction, -1, -1);
        StringTemp[0] = '\0';
        strcat(StringTemp, "MAYBEWARTECH");
        if ((PlayersData[other].diplo_treaties[target] & DTREATY_PACT) != 0)
            strcat(StringTemp, "PACT");
        else if (policy_class != 0) strcat(StringTemp, "FAR");
        if (DiploScratchFA28 >= 0) strcat(StringTemp, "4");
        else if (DiploScratchFA1C >= 0) strcat(StringTemp, "3");
        else if (DiploScratchFA18 >= 0) strcat(StringTemp, "2");
        offer_shown = 1;
        if (X_pops(StringTemp, 0x100000,
                         FactionDiplomacyPortraits[other], pop_wait) != 0) {
            transaction_accepted = 1;
            // Read each selector after preceding transfers/acknowledgements.
            for (int selection = 0; selection < 4; ++selection) {
                const int tech = selection == 0 ? DiploScratchFA28
                    : selection == 1 ? DiploScratchFA1C
                    : selection == 2 ? DiploScratchFA18 : DiploScratchFAA8;
                if (selection != 3 && tech < 0) continue;
                if (IsMultiplayerNet != 0) {
                    log_say(DiploLabelSendingTECH, other, tech, actor);
                    message_data(0x244b, 0, other, tech, actor, 0);
                    NetDaemonNet.await_diplo(0x44b);
                } else {
                    tech_achieved(other, tech, actor, 0);
                }
            }
            DiploScratchFA28 = DiploScratchFA1C =
                DiploScratchFA18 = DiploScratchFAA8 = -1;
        }
    } else if (strcmp(Players[other].filename, "PEACE") != 0 &&
               (DiplomacyProposalMode == 2 || DiplomacyProposalMode == 5)) {
        GenderDefault = Players[target].noun_gender;
        PluralityDefault = Players[target].is_noun_plural;
        parse_says(0, Players[target].noun_faction, -1, -1);
        parse_says(1, get_pact(target), -1, -1);
        parse_says(2, Players[target].adj_insult_faction, -1, -1);
        parse_num(0, price);
        parse_says(6, Players[target].adj_name_faction, -1, -1);
        StringTemp[0] = '\0';
        strcat(StringTemp, "MAYBEWARENERGY");
        if ((PlayersData[other].diplo_treaties[target] & DTREATY_PACT) != 0)
            strcat(StringTemp, "PACT");
        else if (policy_class != 0) strcat(StringTemp, "FAR");
        popup.start(PopupStartCaption, StringTemp, -1, nullptr, 0x100000, 0);
        if (PlayersData[actor].energy_reserves >= price)
            popup.start(PopupStartCaption, "MAYBEWARPRICE", -1, nullptr,
                        0x100080, 0);
        popup.set_caller_picture(popup.picture_enabled() ? nullptr :
                                 FactionDiplomacyPortraits[other]);
        popup.dialogs().sprite(FactionDiplomacyPortraits[other], nullptr, 0);
        offer_shown = 1;
        if (popup.exec(0, pop_wait) > 0) {
            const int amount = diplo_sub(0, price);
            if (IsMultiplayerNet != 0) {
                log_say(DiploLabelSendingENERGY, actor, other, amount);
                message_data(0x2447, 0, actor, amount, other, price);
                NetDaemonNet.await_diplo(0x447);
            } else {
                if (actor != 0) PlayersData[actor].energy_reserves =
                    diplo_add(PlayersData[actor].energy_reserves, amount);
                if (other != 0) PlayersData[other].energy_reserves =
                    diplo_add(PlayersData[other].energy_reserves, price);
                ConsoleGlobal.update_data(1);
            }
            transaction_accepted = 1;
        }
    }
    ConsoleInfoWin.set_redraw_phase(0);
    ConsoleGlobal.update_data(0);
    if (transaction_accepted != 0) goto attack_retry;
    if (offer_shown == 0) {
        GenderDefault = Players[target].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[target].title_leader, -1, -1);
        GenderDefault = Players[target].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[target].name_leader, -1, -1);
        parse_says(2, get_him_her(target, 0), -1, -1);
        X_pops("REJWAR", 0x100000,
                     FactionDiplomacyPortraits[other], pop_wait);
    }
    goto attack_return_zero;
}

// The offline edge applies the staged status locally. The multiplayer sibling
// already sent 0x2441 with (other, target, 0x80000018, 0) and branches directly
// to attack_accept_treaty_network, bypassing this call.
attack_accept_treaty:
treaty_on(other, target, 0x80000018);

attack_accept_treaty_network:
{
// Native 0x54739F-0x547423 performs two fresh IsMultiplayerNet reads. The
// existing wrappers preserve each exact log/message/local operation; neither
// waits for an acknowledgement.
net_set_treaty(other, target, 0x40, 1, 0);
net_set_treaty(other, actor, 0x80, 1, 0);

// 0x547426-0x54755C prepares localized result feedback for the same target.
// Native Players rows have a 0x59C stride; these reads resolve to the named
// Player members below. The parser's default grammar is explicitly updated in
// the same sequence as the native stores to 0x9BBFEC/0x9BBFF0.
GenderDefault = Players[target].is_leader_female;
PluralityDefault = 0;
parse_says(0, Players[target].title_leader, -1, -1);
GenderDefault = Players[target].is_leader_female;
PluralityDefault = 0;
parse_says(1, Players[target].name_leader, -1, -1);
GenderDefault = Players[target].noun_gender;
PluralityDefault = Players[target].is_noun_plural;
parse_says(2, Players[target].noun_faction, -1, -1);
parse_says(3, Players[target].adj_insult_faction, -1, -1);
parse_says(4, get_phrase(target, 5), -1, -1);

// Build "INCITED" plus the native random suffix in StringTemp. _itoa's
// measured cdecl operands are (value, buffer, 10); StringTemp is source-owned.
char incited_number[32];
StringTemp[0] = '\0';
strcat(StringTemp, "INCITED");
_itoa(random(0, 2), incited_number, 10);
strcat(StringTemp, incited_number);
X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[other], pop_wait);
parse_says(3, get_pacts(target), -1, -1);
parse_says(4, Players[target].adj_name_faction, -1, -1);

// The directed actor->target treaty word gates the two possible outcomes.
// A pact-end branch wins if present; otherwise an existing treaty bit can
// trigger the reciprocal treaty update. Both are additionally gated by the
// actor's current difficulty and a fresh random(0, 2) result of zero.
const uint32_t actor_to_target =
    PlayersData[actor].diplo_treaties[target];
if ((actor_to_target & 1U) != 0) {
    // When the pact bit is present, native jumps to the common end on either
    // failed gate; it does not fall through to the treaty-bit test.
    if (PlayersData[actor].diff_level > 1 && random(0, 2) == 0) {
        if (IsMultiplayerNet == 0) {
            // Native PlayerData::diplo_friction is an integer pair matrix;
            // clamp the increment to [0,20], then update the live friction
            // prompt only when its selectors still name (target, actor).
            int friction = diplo_add(
                static_cast<int>(PlayersData[target].diplo_friction[actor]), 4);
            if (friction < 0) friction = 0;
            if (friction > 0x14) friction = 0x14;
            PlayersData[target].diplo_friction[actor] =
                static_cast<uint32_t>(friction);
            if (DiploFrictionFactionID == static_cast<uint32_t>(target) &&
                DiploFrictionFactionIDWith == static_cast<uint32_t>(actor)) {
                DiploFriction = diplo_add(DiploFriction, 4);
            }
        }

        X_pop("INCITEDPACT", pop_wait);
        // net_pact_ends reproduces the native live mode read, logging, 0x244D
        // payload, 0x44D wait, or local pact_ends(actor,target) call.
        net_pact_ends(actor, target, 1);
    }
} else if ((actor_to_target & 2U) != 0 &&
           PlayersData[actor].diff_level > 1 && random(0, 2) == 0) {
    if (IsMultiplayerNet == 0) {
        int friction = diplo_add(
            static_cast<int>(PlayersData[target].diplo_friction[actor]), 4);
        if (friction < 0) friction = 0;
        if (friction > 0x14) friction = 0x14;
        PlayersData[target].diplo_friction[actor] =
            static_cast<uint32_t>(friction);
        if (DiploFrictionFactionID == static_cast<uint32_t>(target) &&
            DiploFrictionFactionIDWith == static_cast<uint32_t>(actor)) {
            DiploFriction = diplo_add(DiploFriction, 4);
        }
    }

    X_pop("INCITEDTREATY", pop_wait);
    // The local 0x10 path calls treaty_on(actor,target,0x10); multiplayer
    // sends its distinct 0x2441 packet. The reciprocal 0x20 remains a setter.
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYON, actor, target, 0x10);
        message_data(0x2441, 0, actor, target, 0x10, 0);
    } else {
        treaty_on(actor, target, 0x10);
    }
    net_set_treaty(target, actor, 0x20, 1, 1);
}

// Every path converges on the native diplomatic-lock release. This is the
// owning NetDaemon operation, not a raw receiver or a destructor call.
log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
NetDaemonNet.unlock();
return 1;
}

attack_return_zero:
    return 0;
}


enum TechTradeContactResult {
    TECH_TRADE_CONTACT_ALLOWED,
    TECH_TRADE_CONTACT_REJECTED
};

enum TechTradePolicyRoute {
    TECH_TRADE_POLICY_REJECT_542097,
    TECH_TRADE_POLICY_REJECT_54222B,
    TECH_TRADE_POLICY_REJECT_5423D8,
    TECH_TRADE_POLICY_REJECT_542585,
    TECH_TRADE_POLICY_REJECT_5425BF,
    TECH_TRADE_POLICY_REJECT_5427FA,
    TECH_TRADE_POLICY_READY_FOR_OFFERS
};

struct TechTradePolicyDecision {
    TechTradePolicyRoute route;
    bool partner_has_missing_prerequisite;
    bool special_action;
};
static TechTradeContactResult tech_trade_contact_policy(
    int actor, int partner) {
    const uint32_t actor_to_partner =
        PlayersData[actor].diplo_treaties[partner];
    if ((actor_to_partner & 1u) != 0) {
        return TECH_TRADE_CONTACT_ALLOWED;
    }

    const uint32_t partner_to_actor =
        PlayersData[partner].diplo_treaties[actor];
    const bool blocked = (partner_to_actor & 0x8020u) != 0 ||
        DiploScratchFA2C != 0 || DiploFriction > 0x11;
    const bool missing_universal_identity = DiploScratchFA60 != 0 &&
        strcmp(Players[partner].filename, "UNIV") != 0;
    if (blocked || missing_universal_identity) {
        return TECH_TRADE_CONTACT_REJECTED;
    }
    return TECH_TRADE_CONTACT_ALLOWED;
}
static void tech_trade_show_contact_rejection(int actor, int partner,
                                              int context) {
    GenderDefault = Players[actor].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[actor].title_leader, -1, -1);
    GenderDefault = Players[actor].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[actor].name_leader, -1, -1);
    GenderDefault = Players[actor].is_leader_female;
    PluralityDefault = 0;
    parse_says(2, Players[actor].assistant_city, -1, -1);
    parse_says(3, Players[actor].adj_insult_faction, -1, -1);
    parse_says(4, get_phrase(actor, 3), -1, -1);

    StringTemp[0] = '\0';
    strcat(StringTemp, "REJTECHHATE");
    const int displayed_number = context == 13 ? 0 : parse_num(3, 0);
    char number_text[16];
    _itoa(displayed_number, number_text, 10);
    strcat(StringTemp, number_text);
    X_pops(StringTemp, 0x100000,
                 FactionDiplomacyPortraits[partner], pop_wait);
}
static TechTradePolicyDecision tech_trade_pre_offer_policy(
    int actor, int partner, int request_mode, int action, int context,
    bool special_action) {
    TechTradePolicyDecision decision = {
        TECH_TRADE_POLICY_READY_FOR_OFFERS, false, special_action};
    const uint32_t partner_to_actor =
        PlayersData[partner].diplo_treaties[actor];
    if ((partner_to_actor & 0x02000001u) != 0x02000001u) {
        for (int reactor = 0; reactor < 2; ++reactor) {
            const int prerequisite = Reactor[reactor].preq_tech;
            if (has_tech(prerequisite, partner) &&
                !has_tech(prerequisite, actor)) {
                decision.partner_has_missing_prerequisite = true;
            }
        }

        const uint32_t actor_to_partner =
            PlayersData[actor].diplo_treaties[partner];
        if ((actor_to_partner & 1u) == 0 &&
            (PlayersData[partner].flags & PFLAG_COOP_WITH_HUMAN) == 0 &&
            great_satan(actor, 0)) {
            const int partner_tech_cost_delta =
                diplo_sub(Players[partner].rule_techcost, 100);
            const int cost_rank = partner_tech_cost_delta > 0 ? 1
                : partner_tech_cost_delta == 0 ? 0 : -1;
            const int threshold = diplo_add(
                (partner_to_actor & 2u) != 0 ? 3 : 0,
                diplo_mul(2, diplo_sub(2, cost_rank)));
            if (DiploFriction >= threshold) {
                decision.route = TECH_TRADE_POLICY_REJECT_54222B;
                return decision;
            }
        }

        if ((IsMultiplayerNet != 0 || (GameDataStateStorage.fields_.game_rules_ & 0x40) != 0) &&
            PlayersData[actor].ranking == 7 &&
            (actor_to_partner & 1u) == 0 &&
            (PlayersData[partner].flags & PFLAG_COOP_WITH_HUMAN) == 0) {
            if (great_satan(actor, 0)) {
                decision.route = TECH_TRADE_POLICY_REJECT_5423D8;
                return decision;
            }
            const int actor_power = FactionRankingsUnk[actor];
            const int current_power =
                FactionRankingsUnk[GameDataStateStorage.fields_.ranking_faction_id_unk2_];
            if (diplo_mul(actor_power, 3) > diplo_mul(current_power, 4) &&
                diplo_mul(PlayersData[actor].tech_ranking, 3) >
                    diplo_mul(PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].tech_ranking,
                              4)) {
                decision.route = TECH_TRADE_POLICY_REJECT_5423D8;
                return decision;
            }
        }

        const int partner_tech_rank = PlayersData[partner].tech_ranking;
        const int actor_tech_rank = PlayersData[actor].tech_ranking;
        if (partner_tech_rank > actor_tech_rank ||
            decision.partner_has_missing_prerequisite) {
            if (request_mode != 0) {
                decision.route = TECH_TRADE_POLICY_REJECT_542585;
                return decision;
            }

            const bool is_hive =
                strcmp(Players[partner].filename, "HIVE") == 0;
            const int special_threshold = diplo_sub(
                diplo_add((partner_to_actor & 2u) != 0 ? 3 : 0,
                    decision.partner_has_missing_prerequisite ? 3 : 9),
                is_hive ? 3 : 0);
            if (DiploFriction >= special_threshold &&
                       (actor_to_partner & 1u) == 0 &&
                       (PlayersData[partner].flags &
                        PFLAG_COOP_WITH_HUMAN) == 0) {
                decision.route = TECH_TRADE_POLICY_REJECT_5425BF;
                return decision;
            }
        }
    }

    if (DiploScratchFA6C != -1 && context != 13) {
        decision.route = TECH_TRADE_POLICY_REJECT_5427FA;
        return decision;
    }
    (void)action;
    return decision;
}
static void tech_trade_handle_pre_offer_rejection(
    Popup &popup, TechTradePolicyDecision decision, int actor, int partner,
    int request_mode, int action, int context) {
    (void)popup; // Preserved in the contract; lexical scope owns teardown.
    if (decision.route == TECH_TRADE_POLICY_REJECT_542585) {
        return;
    }

    if (decision.route == TECH_TRADE_POLICY_REJECT_54222B ||
        decision.route == TECH_TRADE_POLICY_REJECT_5423D8) {
        if (request_mode == 0 &&
            buy_tech(actor, partner, action, 1, context) == 0) {
            GenderDefault = Players[actor].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[actor].title_leader, -1, -1);
            parse_says(1, Players[actor].name_leader, -1, -1);
            parse_says(2, get_phrase(actor, 3), -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, decision.route ==
                TECH_TRADE_POLICY_REJECT_54222B
                    ? "REJTECHFEAR" : "REJTECHFEAR");
            const int displayed_number = context == 13 ? 0 : parse_num(2, 0);
            char number_text[16];
            _itoa(displayed_number, number_text, 10);
            strcat(StringTemp, number_text);
            X_pops(StringTemp, 0x100000,
                         FactionDiplomacyPortraits[partner], pop_wait);
        }
        return;
    }

    if (decision.route == TECH_TRADE_POLICY_REJECT_5425BF) {
        if (!decision.partner_has_missing_prerequisite && request_mode == 0 &&
            buy_tech(actor, partner, action, 1, context) != 0) {
            return;
        }

        if (action == 5 && context != 13) {
            GenderDefault = Players[actor].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[actor].title_leader, -1, -1);
            parse_says(1, Players[actor].name_leader, -1, -1);
            parse_says(2, get_phrase(actor, 3), -1, -1);
            X_pops("REJTECHFEAR1", 0x100000,
                         FactionDiplomacyPortraits[partner], pop_wait);
        } else if (decision.special_action) {
            parse_says(0, Players[actor].adj_insult_faction, -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, "REJTECHADVANCED");
            const int displayed_number =
                decision.partner_has_missing_prerequisite ? 2 : parse_num(3, 0);
            char number_text[16];
            _itoa(displayed_number, number_text, 10);
            strcat(StringTemp, number_text);
            X_pops(StringTemp, 0x100000,
                         FactionDiplomacyPortraits[partner], pop_wait);
        }
        return;
    }

    if (decision.route == TECH_TRADE_POLICY_REJECT_5427FA) {
        if (request_mode == 0) {
            const int project = DiploScratchFA6C;
            const int facility_id = project < 0 ? diplo_sub(0x44, project)
                                                 : diplo_add(project, 0x46);
            parse_says(0, Facility[facility_id].name, -1, -1);
            GenderDefault = Players[actor].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[actor].title_leader, -1, -1);
            parse_says(2, Players[actor].name_leader, -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, "REJTECHPROJ");
            const int displayed_number = project < 0 ? parse_num(3, 0) : 2;
            char number_text[16];
            _itoa(displayed_number, number_text, 10);
            strcat(StringTemp, number_text);
            X_pops(StringTemp, 0x100000,
                         FactionDiplomacyPortraits[partner], pop_wait);
        }
        return;
    }
}

// vector_dist without x_dist's wrap; only its two abs calls survive.
static inline int flat_vector_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b) {
    int x_distance = abs(x_point_a - x_point_b);
    int y_distance = abs(y_point_a - y_point_b);
    int largest = (x_distance > y_distance) ? x_distance : y_distance;
    int smallest = (x_distance < y_distance) ? x_distance : y_distance;
    return largest - ((((y_distance + x_distance) >> 1) - smallest + 1) >> 1);
}

/*
Purpose: Should faction a1 refuse to honour faction a2's withdrawal
         demand: gate on the pair's 0x800000 treaty word, then scan the
         vehicles a2 runs toward a1's territory - skipping probe-plan
         prototypes, ocean tiles and unplaced units - and return the
         third faction that both holds the captured base chain and keeps
         the treaties that make the hand-back profitable.

// ORIGINAL: 0x00548DD0 ?dont_withdrawal@@YAHHH@Z 0x00548DD0-0x00548F7D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x548DD0 full body: faction treaty/prototype/vehicle and base state walk", "source:src/faction.h existing dont_withdrawal declaration and faction diplomacy callers", "source:src/faction.cpp owns the adjacent do_withdrawal and faction diplomacy logic"], "reason": "SRC faction diplomacy"}
// TRIED: the per-base coordinate walk was spelled with a 0x9A stride; the image scales Bases by 0x134 (0x00548E94..0x548EA0), so the typed Bases[] walk replaces it.
// size      429 bytes
// prototype int (__cdecl ?dont_withdrawal@@YAHHH@Z)(int, int)
// callers   ?   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3C60 0x004E3EF0 0x00628A50 0x00644F3A 0x0064FD20
Return Value: the blocking faction id, or 0 to honour the demand
Status: Coverage transcription - unclaimed body
*/
int __cdecl dont_withdrawal(int faction_id, int against_id) {
    PopupDialogFactionID = against_id;
    if ((PlayersData[faction_id].diplo_treaties[against_id] & 0x800000) == 0) {
        return 0;
    }
    for (int index = 0; index < GameDataStateStorage.fields_.vehicle_current_count_; ++index) {
        if (Vehs[index].faction_id != against_id) {
            continue;
        }
        if (VehPrototypes[Vehs[index].proto_id].plan == 0xB) {
            continue;
        }
        const int x = Vehs[index].x;
        const int y = Vehs[index].y;
        if ((map_loc(x, y)->climate & ~0x1F) < 0x60) {
            continue;
        }
        const int base_idx = base_find(x, y, faction_id);
        if (base_idx < 0) {
            continue;
        }
        const int target_y = Bases[base_idx].y;
        const int target_x = Bases[base_idx].x;
        // Unwrapped vector_dist, inlined; its result is never read.
        flat_vector_dist(target_x, target_y, x, y);
        if (whose_territory(faction_id, x, y, 0, 0) != against_id) {
            continue;
        }
        if (Vehs[index].order != ORDER_MOVE_TO) {
            continue;
        }
        const int waypoint_x = Vehs[index].waypoint_x[0];
        const int waypoint_y = Vehs[index].waypoint_y[0];
        const int base2 = base_at(waypoint_x, waypoint_y);
        if (base2 < 0) {
            continue;
        }
        const int owner = Bases[base2].faction_id_current;
        if (owner == faction_id || owner == against_id || owner == 0) {
            continue;
        }
        if ((PlayersData[against_id].diplo_treaties[owner] & 0x10) == 0) {
            continue;
        }
        const uint32_t pair = PlayersData[faction_id].diplo_treaties[owner];
        if ((pair & 1) != 0) {
            return owner;
        }
        if ((pair & 2) != 0 && DiplomacyProposalGateCount > 0) {
            return owner;
        }
    }
    return 0;
}


// ORIGINAL: 0x00548F80 ?do_withdrawal@@YAHHH@Z 0x00548F80-0x00549264 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00548f80.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
// TRIED: MISMATCH #6 'push' vs 'mov'. Tried both `if (n>0){...}` and the inverted `if (n<=0) return 0;` early-out for the outer guard; neither reproduces the original's push-esi-before-branch / xor+cmp zero-reuse pattern. veh_lift/veh_drop's second push (used as the leftover eax arg to the paired veh_drop call) is reproduced by calling veh_lift(i); veh_drop(i, x, y) as two independent statements rather than chaining a return value, since veh_lift is declared void in this unit.
// The image tests the flat-map bit first and wraps on the fall-through.
static inline int withdrawal_wrap_x(int x) {
    if (MapIsFlat & 1) return x;
    const int longitude_bounds = MapLongitudeBounds;
    if (x < 0) return x + longitude_bounds;
    if (x >= longitude_bounds) return x - longitude_bounds;
    return x;
}

int __cdecl do_withdrawal(int faction_id, int other_faction_id) {
    int result = 0;
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; ++veh_id) {
        if (Vehs[veh_id].faction_id != other_faction_id) continue;
        const int proto_id = Vehs[veh_id].proto_id;
        if (VehPrototypes[proto_id].plan == PLAN_INFO_WARFARE) continue;

        const int x = Vehs[veh_id].x;
        const int y = Vehs[veh_id].y;
        Map &tile = MapTiles[MapLongitude * y + (x >> 1)];
        if ((tile.climate & ~0x1F) < ALT_BIT_SHORE_LINE) continue;
        if (tile.territory != faction_id) continue;

        if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_LAND &&
            veh_cargo(veh_id) != 0) {
            for (int neighbor = 0; neighbor < 8; ++neighbor) {
                const int dest_x = withdrawal_wrap_x(VehMoveDeltaX[neighbor] + x);
                const int dest_y = VehMoveDeltaY[neighbor] + y;
                if (!on_map(dest_x, dest_y)) continue;

                Map &dest = MapTiles[MapLongitude * dest_y + (dest_x >> 1)];
                if ((dest.climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
                    if (dest.territory != other_faction_id) continue;
                } else {
                    const int occupant = stack_fix(veh_at(dest_x, dest_y));
                    if (stack_check(occupant, 8, other_faction_id, -1, -1) <= 0)
                        continue;
                }

                Map &live_dest = MapTiles[MapLongitude * dest_y + (dest_x >> 1)];
                if ((live_dest.bit & 3) != 0) {
                    const int owner = live_dest.val2 & 0x0F;
                    if (owner < 8 && owner >= 0 && owner != other_faction_id) continue;
                }

                veh_drop(veh_lift(veh_id), dest_x, dest_y);
                Vehs[veh_id].visibility = 0;
                Vehs[veh_id].order = ORDER_NONE;
                Vehs[veh_id].state &= 0xFCFFBDFF;
                draw_tiles(x, y, 2);
                ++result;
            }
        }

        const int base_id = base_find(x, y, other_faction_id);
        if (base_id >= 0) {
            const int base_y = Bases[base_id].y;
            const int base_x = Bases[base_id].x;
            veh_drop(veh_lift(veh_id), base_x, base_y);
            Vehs[veh_id].visibility = 0;
            Vehs[veh_id].order = ORDER_NONE;
            Vehs[veh_id].state &= 0xFCFFBDFF;
            draw_tile_fixup(x, y, has_abil(Vehs[veh_id].proto_id, 2), 2);
            ++result;
        }
    }

    if (result != 0) {
        set_agenda(other_faction_id, faction_id, 2, 1);
        set_treaty(other_faction_id, faction_id, 0x100000, 1);
    }
    return result;
}


/*
Purpose: Test whether two factions' territories are contiguous in one map
         region: both must hold offensive vehicles there, the path scratch
         flag clears across the map, and every pair of the two factions'
         bases on same-region land tiles gets a Path::find probe with the
         region/contiguity flags until one connects.

// ORIGINAL: 0x0053A780 ?contiguous@@YAHHHHH@Z 0x0053A780-0x0053A97C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x53A780..0x53A97C reads faction regional vehicle counts, Bases, Map tiles and Paths", "source:src/faction.h already declares contiguous; source:src/path.h declares the actual Paths owner", "original seed TRIED prologue ordering is retained; no new byte claim is made"], "reason": "faction territory/path operation"}
// TRIED: prologue register order (mov ecx,[ebp+0x10] before pushes vs a push earlier) - not chased; while/if structure matches disasm.
// size      508 bytes
// prototype int (__cdecl ?contiguous@@YAHHHHH@Z)(int32 factionID1, int32 factionID2, int32 region, int32 flag)
// callers   5   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20 0x00625EC0
// notes     Seed provenance (TRIED): the surviving gap is the prologue
//           register order (mov ecx,[ebp+0x10] before the pushes), not the
//           while/flag structure - not measured exact.
Return Value: 1 a connecting path exists, 0 otherwise
Status: Coverage transcription - unclaimed body, seed
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0053a780.cpp retained.
*/
int __cdecl contiguous(int faction_id1, int faction_id2, int region,
                       int flag) {
    if (region != 0) {
        if (PlayersData[faction_id1].region_total_bases[region] == 0) {
            return 0;
        }
        if (PlayersData[faction_id2].region_total_bases[region] == 0) {
            return 0;
        }
    }

    Map *tile = map_tiles();
    for (int fill_idx = 0; fill_idx < MapArea; ++tile, ++fill_idx) {
        tile->unk_1 = 0;
    }

    for (int outer_idx = 0;
         outer_idx < GameDataStateStorage.fields_.base_current_count_;
         ++outer_idx) {
        if (Bases[outer_idx].faction_id_current != faction_id1) continue;
        const Map *tile1 = map_loc(Bases[outer_idx].x, Bases[outer_idx].y);
        const int type = tile1->region;
        if ((region != 0 && type != region) || tile1->unk_1 != 0) continue;
        Paths.territory(Bases[outer_idx].x, Bases[outer_idx].y, type, faction_id1);
        for (int inner_idx = 0;
             inner_idx < GameDataStateStorage.fields_.base_current_count_;
             ++inner_idx) {
            const Base &base2 = Bases[inner_idx];
            if (base2.faction_id_current != faction_id2) continue;
            const Map *tile2 = map_loc(base2.x, base2.y);
            if (tile2->region != type || tile2->unk_1 != 0) continue;
            Paths.territory(base2.x, base2.y, type, faction_id2);
            if (Paths.find(Bases[outer_idx].x, Bases[outer_idx].y, base2.x, base2.y, faction_id1,
                           faction_id2, (flag != 0 ? 0xc00 : 0) | 0x201, -1) >= 0) {
                return 1;
            }
        }
    }
    return 0;
}

static TechTradePolicyDecision tech_trade_policy_prefix(
    Popup &popup, int actor, int partner, int request_mode, int action,
    int context, bool first_entry) {
    (void)popup; // Popup destruction is lexical and closes exactly once.
    const bool special_action = action == 4 || action == 2;
    if (first_entry) {
        PopupDialogFactionID = partner;
        DiploScratchF7C4 = 0;
    }
    tech_analysis(actor, partner);
    if (context == 13) {
        DiploScratchF800 = diplo_add(DiploScratchFA5C, 0x59);
    }
    diplomacy_caption(actor, partner);

    if (tech_trade_contact_policy(actor, partner) ==
        TECH_TRADE_CONTACT_REJECTED) {
        if (request_mode == 0) {
            tech_trade_show_contact_rejection(actor, partner, context);
        }
        TechTradePolicyDecision decision = {
            TECH_TRADE_POLICY_REJECT_542097, false, special_action};
        return decision;
    }
    return tech_trade_pre_offer_policy(actor, partner, request_mode, action,
                                       context, special_action);
}
static int tech_trade_show_later_response(int actor, int partner, int mode) {
    StringTemp[0] = '\0';
    if (mode == 0xD) {
        strcat(StringTemp, "REJCOMMLINK");
    } else {
        strcat(StringTemp, "REJTECHLATER");
        int response = static_cast<int>(random(0, 3));
        if ((PlayersData[partner].diplo_treaties[actor] & 3u) != 0) {
            if (DiploScratchFAA8 < 0) {
                response = 4;
            } else if (DiploScratchF800 < 0) {
                response = 3;
            }
        }
        char digits[16];
        _itoa(response, digits, 10);
        strcat(StringTemp, digits);
    }
    return X_pops(StringTemp, 0x100000,
                        FactionDiplomacyPortraits[partner], pop_wait);
}
enum TechTradeTailEntry {
    TECH_TRADE_TAIL_CONTINUE = -1,
    TECH_TRADE_TAIL_COMMLINK_FREE_54295F,
    TECH_TRADE_TAIL_TECH_SLAVE_542A78,
    TECH_TRADE_TAIL_TECH_FREEBIE_542BE7,
    TECH_TRADE_TAIL_NO_SPEAK_542E07,
    TECH_TRADE_TAIL_COMMLINK_TECH_542E80,
    TECH_TRADE_TAIL_BUY_RESULT_5437B7,
    TECH_TRADE_TAIL_ACCEPT_543094,
    TECH_TRADE_TAIL_REJECT_SECOND_543169,
    TECH_TRADE_TAIL_SUCCESS_5433A0
};
static void tech_trade_dispatch_technology(int recipient, int technology,
                                           int source) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTECH, recipient, technology, source);
        message_data(0x244B, 0, recipient, technology, source, 0);
        NetDaemonNet.await_diplo(0x44B);
    } else {
        tech_achieved(recipient, technology, source, 0);
    }
}
static void tech_trade_update_partner_patience(int actor, int partner) {
    const uint32_t treaty = PlayersData[partner].diplo_treaties[actor];
    int patience;
    if ((treaty & 0x10u) != 0) {
        patience = 1;
    } else if ((treaty & 1u) != 0) {
        patience = (treaty & 0x02000000u) != 0 ? 500 : 6;
    } else {
        const int rounded_eighths = diplo_add(DiploFriction, 3) / 8;
        patience = diplo_add(
            diplo_sub((treaty & 2u) != 0 ? 1 : 0, rounded_eighths), 3);
    }
    PlayersData[partner].diplo_patience[actor] =
        static_cast<char>(patience);
}

static void tech_trade_send_live_request(int actor, int partner) {
    const int technology = DiploScratchF800;
    tech_trade_dispatch_technology(actor, technology, partner);
    DiploScratchF800 = -1;
}
static int tech_trade_run_tail(TechTradeTailEntry entry, int actor,
                               int partner, int request_mode, int action,
                               int context) {
    switch (entry) {
    case TECH_TRADE_TAIL_COMMLINK_FREE_54295F:
        if (!wants_to_speak(actor, partner)) return 0;
        parse_says(0, Players[DiploScratchFA5C].adj_name_faction, -1, -1);
        x_pops_minimal("COMMLINKFREE",
                       FactionDiplomacyPortraits[partner], pop_wait);
        tech_trade_send_live_request(actor, partner);
        return 0;

    case TECH_TRADE_TAIL_TECH_SLAVE_542A78:
        if (!wants_to_speak(actor, partner)) return 0;
        StringTemp[0] = '\0';
        say_tech(StringTemp, DiploScratchF800, 1);
        parse_says(0, StringTemp, -1, -1);
        X_pops("TECHSLAVE", 0x100000,
                     FactionDiplomacyPortraits[partner], pop_wait);
        tech_trade_send_live_request(actor, partner);
        return 0;

    case TECH_TRADE_TAIL_TECH_FREEBIE_542BE7:
        if (!wants_to_speak(actor, partner)) return 0;
        StringTemp[0] = '\0';
        say_tech(StringTemp, DiploScratchF800, 1);
        parse_says(0, StringTemp, -1, -1);
        X_pops("TECHFREEBIE", 0x100000,
                     FactionDiplomacyPortraits[partner], pop_wait);
        tech_trade_send_live_request(actor, partner);
        tech_trade_update_partner_patience(actor, partner);
        return 0;

    case TECH_TRADE_TAIL_NO_SPEAK_542E07:
        return 0;

    case TECH_TRADE_TAIL_COMMLINK_TECH_542E80:
        parse_says(2, Players[DiploScratchFA5C].adj_name_faction, -1, -1);
        if (x_pops_minimal("COMMLINKTECH",
                           FactionDiplomacyPortraits[partner], pop_wait) == 0) {
            return 0;
        }
        tech_trade_dispatch_technology(partner, DiploScratchFAA8, actor);
        tech_trade_dispatch_technology(actor, DiploScratchF800, partner);
        DiploScratchF800 = -1;
        DiploScratchFAA8 = DiploScratchFA18;
        DiploScratchFA18 = -1;
        return 1;

    case TECH_TRADE_TAIL_BUY_RESULT_5437B7:
        if (request_mode != 0) return 0;
        if (buy_tech(actor, partner, action, 0, context) != 0) return 0;
        tech_trade_show_later_response(actor, partner, context);
        return 0;

    case TECH_TRADE_TAIL_ACCEPT_543094:
        DiploScratchF7C4 = 1;
        return 0;

    case TECH_TRADE_TAIL_REJECT_SECOND_543169:
        X_pops("REJTECHSECOND", 0x100000,
                     FactionDiplomacyPortraits[partner], pop_wait);
        PlayersData[partner].diplo_patience[actor] = static_cast<char>(
            static_cast<unsigned char>(
                PlayersData[partner].diplo_patience[actor]) + 1u);
        return 0;

    case TECH_TRADE_TAIL_SUCCESS_5433A0:
        return 1;
    }
    return -1;
}
static void tech_trade_parse_initial_offer_lines() {
    if (DiploScratchFAA8 != 0x270F) {
        StringTemp[0] = '\0';
        say_tech(StringTemp, DiploScratchFAA8, TRUE);
        parse_says(0, StringTemp, -1, -1);
    }
    if (DiploScratchF800 >= 0) {
        StringTemp[0] = '\0';
        say_tech(StringTemp, DiploScratchF800, TRUE);
        parse_says(1, StringTemp, -1, -1);
    }
}

static int tech_trade_offer_dialog(Popup &popup, int faction_id,
                                   int other_faction_id, int context) {
    tech_trade_parse_initial_offer_lines();
    if (context != 0xD && DiploScratchFA18 >= 0) {
        StringTemp[0] = '\0';
        say_tech(StringTemp, DiploScratchFA18, TRUE);
        parse_says(2, StringTemp, -1, -1);
    }

    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(3, Players[faction_id].title_leader, -1, -1);
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(4, Players[faction_id].name_leader, -1, -1);
    parse_says(5, get_noun_phrase(faction_id, 9), -1, -1);
    parse_says(6, get_noun_phrase(other_faction_id, 9), -1, -1);
    GenderDefault = Players[other_faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(8, Players[other_faction_id].name_leader, -1, -1);
    parse_says(9, get_noun_phrase(other_faction_id, 7), -1, -1);

    StringTemp[0] = '\0';
    strcat(StringTemp, "TRADETECH");
    if (DiploScratchFAA8 == 0x270F) {
        const int suffix = DiploScratchF800 < 0 ? 5 : 4;
        char digits[16];
        _itoa(suffix, digits, 10);
        strcat(StringTemp, digits);
    } else if (DiploScratchF800 < 0) {
        char digits[16];
        _itoa(3, digits, 10);
        strcat(StringTemp, digits);
    } else {
        const int suffix = parse_num(0, 3);
        char digits[16];
        _itoa(suffix, digits, 10);
        strcat(StringTemp, digits);
    }

    popup.start(PopupStartCaption, StringTemp, -1, 0, 0x100000, 0);
    popup.set_caller_picture(popup.picture_enabled()
        ? 0 : FactionDiplomacyPortraits[other_faction_id]);
    popup.dialogs().sprite(FactionDiplomacyPortraits[other_faction_id],
                           0, 0);
    if (X_text_open(ScriptPopupFile, "TRADETECHOPT") == 0) {
        text_get();
        if (DiploScratchF80C >= 0 &&
            DiploScratchF80C != DiploScratchF800) {
            StringTemp[0] = '\0';
            say_tech(StringTemp, DiploScratchF80C, TRUE);
            parse_says(9, StringTemp, -1, -1);
            parse_string(TextBufferGetPtr, StringTemp);
            popup.dialogs().item(StringTemp, 4);
        }
        text_get();
        if (DiploScratchFA18 >= 0 &&
            DiploScratchFA18 != DiploScratchFAA8) {
            StringTemp[0] = '\0';
            parse_string(TextBufferGetPtr, StringTemp);
            popup.dialogs().item(StringTemp, 2);
        }
        text_get();
        popup.dialogs().item(TextBufferGetPtr, 3);
    }
    return popup.exec(0, pop_wait);
}
static TechTradeTailEntry tech_trade_commit_dialog_state(
    int actor, int partner, int request_mode, int friction_eighths) {
    const int friction_delta = diplo_sub(0, friction_eighths);

    if (request_mode != 0) {
        if (IsMultiplayerNet != 0) {
            log_say(DiploLabelSendingTREATYON, partner, actor, 0x1000000);
            message_data(0x2441, 0, partner, actor, 0x1000000, 0);
        } else {
            treaty_on(partner, actor, 0x1000000);
        }
    }
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingAGENDAON, partner, actor, 0x180);
        message_data(0x2444, 0, partner, actor, 0x180, 0);
    } else {
        agenda_on(partner, actor, 0x180);
    }

    if (DiploScratchF800 < 0) DiploScratchF800 = 0x270F;
    int selected = DiploScratchFAA8;
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTECH, partner, selected, actor);
        message_data(0x244B, 0, partner, selected, actor, 0);
        NetDaemonNet.await_diplo(0x44B);
    } else {
        tech_achieved(partner, selected, actor, 0);
    }

    selected = DiploScratchF800;
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTECH, actor, selected, partner);
        message_data(0x244B, 0, actor, selected, partner, 0);
        NetDaemonNet.await_diplo(0x44B);
    } else {
        tech_achieved(actor, selected, partner, 0);
    }

    if (DiploScratchFAA8 == 0x270F || DiploScratchF800 == 0x270F)
        DiploScratchFA48 = 1;
    DiploScratchFAA8 = DiploScratchFA18;
    DiploScratchFA18 = -1;

    uint32_t &partner_friction = PlayersData[partner].diplo_friction[actor];
    int next_friction = diplo_add(static_cast<int>(partner_friction),
                                  friction_delta);
    if (next_friction < 0) next_friction = 0;
    else if (next_friction > 20) next_friction = 20;
    partner_friction = static_cast<uint32_t>(next_friction);

    if (DiploFrictionFactionID == static_cast<uint32_t>(partner) &&
        DiploFrictionFactionIDWith == static_cast<uint32_t>(actor))
        DiploFriction = diplo_add(DiploFriction, friction_delta);
    DiploScratchFA60 = 0;
    return request_mode == 0 ? TECH_TRADE_TAIL_SUCCESS_5433A0
        : static_cast<TechTradeTailEntry>(-1);
}

struct TechTradeDialogStep {
    bool rebuild_dialog;
    TechTradeTailEntry tail;
    int friction_eighths;

    TechTradeDialogStep(bool rebuild, TechTradeTailEntry route,
                        int friction)
        : rebuild_dialog(rebuild), tail(route), friction_eighths(friction) {}
};

static int tech_trade_signed_shift_divide(int value, int correction_mask,
                                          int shift) {
    const int correction = (value >> 31) & correction_mask;
    return diplo_add(value, correction) >> shift;
}

static TechTradeDialogStep tech_trade_handle_dialog_result(
    int dialog_result, int actor, int partner, int context) {
    int friction_eighths = tech_trade_signed_shift_divide(
        DiploScratchF814, 3, 2);
    if (dialog_result == 0)
        return TechTradeDialogStep(false, TECH_TRADE_TAIL_ACCEPT_543094,
                                  friction_eighths);

    if (dialog_result == 3) {
        const int help_technology = DiploScratchFAA8 == 0x270F
            ? DiploScratchF800 : DiploScratchFAA8;
        help_topic(0xE, help_technology);
        return TechTradeDialogStep(true,
            static_cast<TechTradeTailEntry>(-1), friction_eighths);
    }

    if (dialog_result == 4) {
        bool reject_for_tech_value = false;
        if ((PlayersData[partner].diplo_treaties[actor] & 1u) == 0 &&
            DiploScratchFA18 < 0) {
            const int alternative = DiploScratchF80C;
            const int alternative_value =
                tech_val(alternative, partner, FALSE);
            const int requested = DiploScratchFAA8;
            const int requested_value =
                tech_val(requested, partner, FALSE);
            reject_for_tech_value = alternative_value > requested_value;
        }
        if (reject_for_tech_value) {
            if (DiploScratchF800 >= 0) {
                StringTemp[0] = '\0';
                say_tech(StringTemp, DiploScratchF800, TRUE);
                parse_says(0, StringTemp, -1, -1);
            }
            X_pops("REJTECHINSTEAD", 0x100000,
                         FactionDiplomacyPortraits[partner], pop_wait);
            DiploScratchF80C = -1;
            return TechTradeDialogStep(true,
                static_cast<TechTradeTailEntry>(-1), friction_eighths);
        }
        swap(&DiploScratchF800, &DiploScratchF80C);
        friction_eighths = tech_trade_signed_shift_divide(
            DiploScratchF814, 7, 3);
    } else if (dialog_result == 2) {
        const uint32_t partner_treaty =
            PlayersData[partner].diplo_treaties[actor];
        const bool special_partner_state =
            (partner_treaty & 1u) != 0 &&
            (partner_treaty & 0x02000000u) != 0;
        if (!special_partner_state) {
            if (strcmp(Players[partner].filename, "HIVE") == 0 ||
                (IsMultiplayerNet != 0 &&
                 GameDataStateStorage.fields_.ranking_faction_id_unk1_ == static_cast<uint32_t>(actor))) {
                return TechTradeDialogStep(false,
                    TECH_TRADE_TAIL_REJECT_SECOND_543169, friction_eighths);
            }
            if (strcmp(Players[partner].filename, "UNIV") != 0 &&
                (great_satan(actor, 0) != 0 ||
                 PlayersData[partner].tech_ranking >
                     PlayersData[actor].tech_ranking))
                return TechTradeDialogStep(false,
                    TECH_TRADE_TAIL_REJECT_SECOND_543169, friction_eighths);
        }
        swap(&DiploScratchFAA8, &DiploScratchFA18);
        friction_eighths = tech_trade_signed_shift_divide(
            DiploScratchFA44, 7, 3);
    }

    (void)context;
    return TechTradeDialogStep(false,
        static_cast<TechTradeTailEntry>(-1), friction_eighths);
}
static TechTradeTailEntry tech_trade_ui_gate(int faction_id,
                                              int other_faction_id,
                                              int request_mode, int action,
                                              int context) {
    const int actor = faction_id;
    const int partner = other_faction_id;

    if (DiploScratchFAA8 < 0 && context != 5 && context != 0xD &&
        (PlayersData[partner].traded_maps & (1u << actor)) == 0 &&
        DiploScratchFA48 == 0 &&
        (PlayersData[actor].diplo_treaties[partner] & 1u) == 0 &&
        (PlayersData[partner].flags & 0x200u) == 0) {
        int seen_partner_bases = 0;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            const Base &base = Bases[base_id];
            if (base.faction_id_current == actor &&
                (base.faction_id_current == partner ||
                 (base.visibility & (1u << partner)) != 0)) {
                ++seen_partner_bases;
            }
        }

        bool enough_base_coverage = false;
        if ((GameDataStateStorage.fields_.game_rules_ & 0x100u) != 0) {
            enough_base_coverage = seen_partner_bases == 0;
        } else {
            const int threshold = diplo_add(
                PlayersData[actor].current_num_bases, 1) / 2;
            enough_base_coverage = seen_partner_bases < threshold;
        }
        if (enough_base_coverage && DiploFriction <= 7)
            DiploScratchFAA8 = 0x270F;
    }

    const uint32_t partner_to_actor =
        PlayersData[partner].diplo_treaties[actor];
    if (context == 0xD && (partner_to_actor & 1u) != 0)
        return TECH_TRADE_TAIL_COMMLINK_FREE_54295F;

    if (DiploScratchF800 >= 0 && context != 0xD &&
        (partner_to_actor & 1u) != 0 &&
        (partner_to_actor & 0x02000000u) != 0)
        return TECH_TRADE_TAIL_TECH_SLAVE_542A78;

    if (DiploScratchFAA8 < 0 && DiploScratchF800 >= 0 &&
        context != 5 && context != 0xD && (partner_to_actor & 1u) != 0 &&
        DiploFriction < 10 &&
        ((IsMultiplayerNet == 0 && (GameDataStateStorage.fields_.game_rules_ & 0x40u) == 0) ||
         GameDataStateStorage.fields_.ranking_faction_id_unk1_ != static_cast<uint32_t>(actor)) &&
        DiploScratchFA3C == 0) {
        const int actor_rank = PlayersData[actor].ranking;
        const int partner_rank = PlayersData[partner].ranking;
        if (partner_rank > actor_rank ||
            (actor_rank >= 6 && partner_rank >= 3)) {
            int rank_delta = diplo_sub(actor_rank, partner_rank);
            rank_delta = diplo_sub(rank_delta, rank_delta >> 31);
            const int rank_half = rank_delta >> 1;
            int pressure = diplo_add(4, diplo_mul(
                2, static_cast<int>(PlayersData[partner].ai_power)));
            pressure = diplo_add(pressure, rank_half);
            pressure = diplo_sub(pressure,
                static_cast<int>(PlayersData[partner].ai_tech));
            pressure = diplo_add(pressure, PlayersData[partner].ai_fight);
            pressure = diplo_add(pressure, PlayersData[actor].tech_ranking);
            if (PlayersData[partner].tech_ranking > pressure)
                return TECH_TRADE_TAIL_TECH_FREEBIE_542BE7;
        }
    }

    int partner_seen_bases = 0;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        const Base &base = Bases[base_id];
        if (base.faction_id_current == partner &&
            (base.faction_id_current == actor ||
             (base.visibility & (1u << actor)) != 0)) {
            ++partner_seen_bases;
        }
    }

    const bool wants_more_tech = action == 4 || action == 2;
    if (DiploScratchFAA8 < 0 || !wants_more_tech || DiploScratchFA3C != 0)
        return TECH_TRADE_TAIL_BUY_RESULT_5437B7;

    if (IsMultiplayerNet != 0) {
        const int actor_agenda = PlayersData[actor].diplo_agenda[partner];
        if ((actor_agenda & 0x80) != 0 &&
            GameDataStateStorage.fields_.ranking_faction_id_unk1_ == static_cast<uint32_t>(actor) &&
            (PlayersData[actor].diplo_treaties[partner] & 1u) == 0) {
            const int ranker = static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk2_);
            const int ranker_power = FactionRankingsUnk[ranker];
            if (PlayersData[ranker].ranking < 5 ||
                diplo_mul(FactionRankingsUnk[actor], 2) >
                    diplo_mul(ranker_power, 3))
                return TECH_TRADE_TAIL_BUY_RESULT_5437B7;
        }
        if ((actor_agenda & 0x100) != 0 &&
            (GameDataStateStorage.fields_.ranking_faction_id_unk2_ != static_cast<uint32_t>(actor) ||
             (PlayersData[actor].ranking >= 6 &&
              DiploScratchF7F0 == 0)))
            return TECH_TRADE_TAIL_BUY_RESULT_5437B7;
    }

    const int partner_agenda = PlayersData[partner].diplo_agenda[actor];
    if ((partner_to_actor & 0x20u) != 0 ||
        (partner_agenda & 0x200) != 0 ||
        PlayersData[actor].integrity_blemishes > 3)
        return TECH_TRADE_TAIL_BUY_RESULT_5437B7;

    if ((GameDataStateStorage.fields_.game_rules_ & 0x40u) != 0) {
        const int actor_agenda = PlayersData[actor].diplo_agenda[partner];
        if ((actor_agenda & 0x80) != 0 &&
            GameDataStateStorage.fields_.ranking_faction_id_unk1_ == static_cast<uint32_t>(actor) &&
            (PlayersData[actor].diplo_treaties[partner] & 1u) == 0 &&
            diplo_mul(FactionRankingsUnk[GameDataStateStorage.fields_.faction_rankings_[7]],
                      3) >
                diplo_mul(FactionRankingsUnk[GameDataStateStorage.fields_.faction_rankings_[6]],
                          4))
            return TECH_TRADE_TAIL_BUY_RESULT_5437B7;
        if ((actor_agenda & 0x100) != 0 &&
            (PlayersData[actor].diplo_treaties[partner] & 1u) == 0 &&
            PlayersData[actor].ranking >= 6 &&
            DiploScratchF7F0 == 0)
            return TECH_TRADE_TAIL_BUY_RESULT_5437B7;
    }

    if (DiploScratchF800 < 0) {
        if (DiploScratchFA48 != 0 ||
            (PlayersData[actor].diplo_treaties[partner] & 1u) != 0 ||
            (PlayersData[actor].flags & 0x200u) != 0)
            return TECH_TRADE_TAIL_BUY_RESULT_5437B7;

        if ((PlayersData[actor].traded_maps & (1u << partner)) != 0) {
            const int partner_bases =
                PlayersData[partner].current_num_bases;
            const int threshold = (GameDataStateStorage.fields_.game_rules_ & 0x100u) != 0
                ? diplo_add(partner_bases, 3) / 4
                : diplo_add(partner_bases, 1) / 2;
            if (partner_seen_bases >= threshold)
                return TECH_TRADE_TAIL_BUY_RESULT_5437B7;
        }
    }

    if (!wants_to_speak(actor, partner))
        return TECH_TRADE_TAIL_NO_SPEAK_542E07;
    if (context == 0xD) {
        tech_trade_parse_initial_offer_lines();
        return TECH_TRADE_TAIL_COMMLINK_TECH_542E80;
    }
    (void)request_mode;
    return static_cast<TechTradeTailEntry>(-1);
}
static TechTradeTailEntry tech_trade_ui_phase(
    Popup &popup, int actor, int partner, int request_mode, int action,
    int context) {
    TechTradeTailEntry route = tech_trade_ui_gate(
        actor, partner, request_mode, action, context);
    if (static_cast<int>(route) != -1) return route;

    for (;;) {
        const int dialog_result =
            tech_trade_offer_dialog(popup, actor, partner, context);
        const TechTradeDialogStep decision = tech_trade_handle_dialog_result(
            dialog_result, actor, partner, context);
        if (static_cast<int>(decision.tail) != -1) return decision.tail;
        if (decision.rebuild_dialog) continue;

        route = tech_trade_commit_dialog_state(actor, partner, request_mode,
                                                decision.friction_eighths);
        return route;
    }
}

// ORIGINAL: 0x00541100 ?tech_trade@@YAHHHHHH@Z 0x00541100-0x00543A20;0x0065EB33-0x0065F1B4
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned541100-543A20 and65EB33-65F1B4; native54119D/54122E establish directed pair cells", "Native541D18 quarter;541DE2 incoming eighth;541EA9 alternate eighth;541EFF unconditional partner-to-actor agenda", "Native542066 friction selector pair;542F3A accepted commlink return1;543094 cancellation state", "historical seed:src/unrecovered/00541100.cpp"], "reason": "Recover the full technology/commlink negotiation in its faction owner with a lexical Popup, directed player treaty/agenda/friction members, live offer selectors and original modal retry/transaction/result ordering. Preserve primary and cleanup spans and seed findings; no byte matching or game acceptance is asserted."}
// symbol    ?tech_trade@@YAHHHHHH@Z
// TRIED: full nested AI-mood reject condition tree (>10 inlined "trade rejected" popups, each with its own message id/number) was factored into one tech_trade_reject() helper covering the first few reject reasons only; the 4-slot trade loop's own per-slot popup/negotiation sequence (FUN_0053dce0 gating, per-partner commlink offers) is approximated, not reproduced.

int __cdecl tech_trade(int actor, int partner, int request_mode, int action,
                       int context) {
    Popup popup;
    bool first_entry = true;

    for (;;) {
        const TechTradePolicyDecision policy = tech_trade_policy_prefix(
            popup, actor, partner, request_mode, action, context,
            first_entry);
        first_entry = false;
        if (policy.route == TECH_TRADE_POLICY_REJECT_542097)
            return 0;
        if (policy.route != TECH_TRADE_POLICY_READY_FOR_OFFERS) {
            tech_trade_handle_pre_offer_rejection(
                popup, policy, actor, partner, request_mode, action, context);
            return 0;
        }

        const TechTradeTailEntry tail = tech_trade_ui_phase(
            popup, actor, partner, request_mode, action, context);
        if (static_cast<int>(tail) == -1)
            continue;
        const int native_result = tech_trade_run_tail(
            tail, actor, partner, request_mode, action, context);
        return native_result;
    }
}

// ORIGINAL: 0x0054F420 ?make_a_proposal@@YAXHHH@Z 0x0054F420-0x0054FFAB;0x0065FD5F-0x0065FF4F
// RECOVERY_BINDING: v1 {"evidence": ["task:diplomacy-actions-owner-coverage", "Pinned54F420-54FFAB and65FD5F-65FF4F; entryFAB0 counteroffer selector andFA34 proposal identity", "Native54F95F rereads multiplayer mode after repayment callback;54F9B6 rereads live Council selector", "Physical54FFAC nine-entry jump table establishes Council cases1-9;54FBEC vote count=-1-reply", "Native54FCDF governor default usesPopup.set_value; no raw modal storage or manual destruction", "historical seed:src/unrecovered/0054f420.cpp"], "reason": "Recover the proposal dispatcher, repayment and Council dialogs in their faction owner using a lexical Popup, source-owned selectors and directed loan/player state. Preserve original primary and generated cleanup spans and all seed findings; no byte or game acceptance claim is granted."}
// symbol    ?make_a_proposal@@YAXHHH@Z
// TRIED: no real SEH frame (raw scratch buffer + manual destructor calls instead of automatic-local RAII); sub_4066c0 called with no receiver since its declared prototype is a 0-arg extern "C" and changing it would break linkage to the real symbol; the case-0xb dialog-item text building (parse_says/ Strings::get/strcat chains) and the eligibility gate per faction index were transcribed at reduced fidelity given the size of this function relative to the batch budget.
void __cdecl make_a_proposal(int faction_id, int other_faction_id,
                             int selection_ready) {
    Popup popup;

    PopupDialogFactionID = other_faction_id;
    DiploScratchFAD0 = 0;

    if (selection_ready == 0 && proposal_menu(faction_id, other_faction_id) == 0) {
        return;
    }

    const int counteroffer_mode = DiplomacyProposalMode;
    const bool coercion_mode = counteroffer_mode == 3;
    DiploScratchFAD4 = 1;
    const int proposal_kind = DiploScratchFA34;

    if (proposal_kind == 2) {
        if (propose_pact(faction_id, other_faction_id) == 0 && coercion_mode) {
            threaten(faction_id, other_faction_id);
        }
        return;
    }
    if (proposal_kind == 3) {
        if (propose_treaty(faction_id, other_faction_id) == 0 && coercion_mode) {
            threaten(faction_id, other_faction_id);
        }
        return;
    }
    if (proposal_kind == 7) {
        PopupDialogFactionID = other_faction_id;
        const uint32_t actor_treaty =
            PlayersData[faction_id].diplo_treaties[other_faction_id];
        const bool refusal =
            (((actor_treaty & DTREATY_PACT) == 0) && DiploFriction > 0) ||
            DiploFriction > 4;
        if (refusal) {
            parse_says(0, Players[other_faction_id].adj_name_faction, -1, -1);
            X_pops("REJMAPS", 0x100000,
                         FactionDiplomacyPortraits[other_faction_id],
                         pop_wait);
        } else {
            X_pops("TRADEMAPS", 0x100000,
                         FactionDiplomacyPortraits[other_faction_id],
                         pop_wait);
            PlayersData[other_faction_id].diplo_patience[faction_id] += 2;
            net_maps(faction_id, other_faction_id, 1);
            DiplomacyMapTransferChanged = 1;
        }
        return;
    }
    if (proposal_kind == 10) return;

    if (proposal_kind == 8) {
        const uint32_t target_treaty =
            PlayersData[other_faction_id].diplo_treaties[faction_id];
        if ((target_treaty & (DTREATY_PACT | DTREATY_HAVE_SURRENDERED)) ==
            (DTREATY_PACT | DTREATY_HAVE_SURRENDERED)) {
            propose_attack(faction_id, other_faction_id);
            return;
        }
    }

    if (coercion_mode) {
        threaten(faction_id, other_faction_id);
        return;
    }
    if (proposal_kind == 8) {
        propose_attack(faction_id, other_faction_id);
        return;
    }
    if (proposal_kind == 9) {
        base_swap(faction_id, other_faction_id);
        return;
    }
    if (proposal_kind == 4 || proposal_kind == 5 || proposal_kind == 13) {
        DiploScratchFAD0 = tech_trade(faction_id, other_faction_id, 0,
                                      counteroffer_mode, proposal_kind);
        return;
    }
    if (proposal_kind == 6) {
        DiploScratchFAD0 = energy_trade(faction_id, other_faction_id);
        return;
    }
    if (proposal_kind == 1) {
        make_gift(faction_id, other_faction_id);
        return;
    }
    if (proposal_kind == 11) {
        // Return the outstanding loan. Native builds the speaker text from
        // the actor's title/name before touching balances.
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction_id].name_leader, -1, -1);

        const int amount = PlayersData[faction_id].loan_balance[other_faction_id];
        if (IsMultiplayerNet != 0) {
            log_say(DiploLabelSendingENERGY, faction_id, other_faction_id,
                    diplo_sub(0, amount));
            message_data(0x2447, 0, faction_id, diplo_sub(0, amount), other_faction_id,
                         amount);
            NetDaemonNet.await_diplo(0x447);
        } else {
            if (faction_id != 0) {
                PlayersData[faction_id].energy_reserves = diplo_sub(
                    PlayersData[faction_id].energy_reserves, amount);
            }
            if (other_faction_id != 0) {
                PlayersData[other_faction_id].energy_reserves = diplo_add(
                    PlayersData[other_faction_id].energy_reserves, amount);
            }
            ConsoleGlobal.update_data(1);
        }

        if (IsMultiplayerNet != 0) {
            log_say(DiploLabelSendingLOAN, faction_id, other_faction_id, 0);
            message_data(0x244e, 0, faction_id, other_faction_id, 0, 0);
        } else {
            PlayersData[faction_id].loan_balance[other_faction_id] = 0;
            PlayersData[faction_id].loan_payment[other_faction_id] = 0;
        }

        X_pops("REPAIDLOAN", 0x100000,
                     FactionDiplomacyPortraits[other_faction_id], pop_wait);
        // The modal can change this independent selector. Native falls into
        // the Council path only when its live value is now 12.
        if (DiploScratchFA34 != 12) return;
    }

    if (DiploScratchFA34 == 12) {
        // Native Council jump-table cases 1..9 map to these source-owned
        // proposal records and resolution counters.
        popup.start(PopupStartCaption, "COUNCILMATTER", -1, 0,
                    0x100040, 0);
        popup.set_caller_picture(popup.picture_enabled()
            ? 0 : FactionDiplomacyPortraits[other_faction_id]);
        popup.dialogs().sprite(FactionDiplomacyPortraits[other_faction_id],
                               0, 0);

        for (int proposal = 0; proposal < MaxProposalNum; ++proposal) {
            const bool proposal_has_technology =
                has_tech(Proposal[proposal].preq_tech, faction_id) ||
                (GameDataStateStorage.fields_.game_state_ & 0x1080) == 0x1080;
            bool available = proposal_has_technology;
            if (available && proposal == 1) {
                available = (GameDataStateStorage.fields_.game_state_ & 1) == 0 &&
                    GameDataStateStorage.fields_.council_resolution_count_[1] == 0 &&
                    (GameDataStateStorage.fields_.game_rules_ & 8) != 0 && eligible(faction_id);
            } else if (available && (proposal == 2 || proposal == 5)) {
                available = GameDataStateStorage.fields_.council_resolution_count_[proposal] == 0;
            } else if (available && proposal == 3) {
                available = GameDataStateStorage.fields_.council_resolution_count_[3] <= GameDataStateStorage.fields_.council_resolution_count_[4];
            } else if (available && proposal == 4) {
                available = GameDataStateStorage.fields_.council_resolution_count_[4] != GameDataStateStorage.fields_.council_resolution_count_[3];
            } else if (available && proposal == 6) {
                available = GameDataStateStorage.fields_.council_resolution_count_[5] != 0;
            } else if (available && proposal == 8) {
                available = GameDataStateStorage.fields_.council_resolution_count_[8] <= GameDataStateStorage.fields_.council_resolution_count_[9];
            } else if (available && proposal == 9) {
                available = GameDataStateStorage.fields_.council_resolution_count_[9] < GameDataStateStorage.fields_.council_resolution_count_[8];
            }
            if (available) {
                StringTemp[0] = '\0';
                strcat(StringTemp, StringTable.get(Proposal[proposal].name));
                strcat(StringTemp, "|");
                strcat(StringTemp, StringTable.get(Proposal[proposal].description));
                popup.dialogs().item(StringTemp, proposal);
            }
        }

        const int first_choice = popup.exec(0, pop_wait);
        if (first_choice < 0) return;
        if (first_choice != 0) {
            const int reply = X_pops("COUNCILVOTE", 0x100000,
                         FactionDiplomacyPortraits[other_faction_id],
                         pop_wait);
            const int vote_count = diplo_sub(-1, reply);
            if (vote_count != 0) {
                buy_council_vote(faction_id, other_faction_id, first_choice,
                                 vote_count);
            }
            return;
        }

        popup.start(PopupStartCaption, "COUNCILVOTEGOV", -1, 0,
                    0x100000, 0);
        popup.set_caller_picture(popup.picture_enabled()
            ? 0 : FactionDiplomacyPortraits[other_faction_id]);
        popup.dialogs().sprite(FactionDiplomacyPortraits[other_faction_id],
                               0, 0);
        for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1u << candidate)) != 0 &&
                eligible(candidate)) {
                StringTemp[0] = '\0';
                GenderDefault = Players[candidate].is_leader_female;
                PluralityDefault = 0;
                strcat(StringTemp, Players[candidate].title_leader);
                strcat(StringTemp, "|");
                GenderDefault = Players[candidate].is_leader_female;
                PluralityDefault = 0;
                strcat(StringTemp, Players[candidate].name_leader);
                popup.dialogs().item(StringTemp, candidate);
                if (candidate == faction_id) {
                    popup.set_value(candidate);
                }
            }
        }
        const int governor_choice = popup.exec(0, pop_wait);
        if (governor_choice >= 0) {
            buy_council_vote(faction_id, other_faction_id, 0,
                             governor_choice);
        }
        return;
    }
}

// Hand-written diplomacy sequences that the image repeats in place.
#define COMMUNICATE_LOCK()                                                   \
    log_say(DiploLabelGettingDIPLOMATICLOCK, 0, 0, 0);                       \
    timeGetTime();                                                           \
    while (!ExitTurnLoop && NetDaemonNet.add_global()) {                     \
        check_spock();                                                       \
        timeGetTime();                                                       \
    }
#define COMMUNICATE_UNLOCK()                                                 \
    log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);                     \
    NetDaemonNet.unlock();
#define COMMUNICATE_FRICTION(amount)                                         \
    PlayersData[faction_id_with].diplo_friction[faction_id] = range(         \
        PlayersData[faction_id_with].diplo_friction[faction_id] + (amount),  \
        0, 20);                                                              \
    if (DiploFrictionFactionID == faction_id_with &&                         \
        DiploFrictionFactionIDWith == faction_id) {                          \
        DiploFriction += (amount);                                           \
    }
#define COMMUNICATE_WITHDRAW(a, b)                                           \
    if (IsMultiplayerNet) {                                                  \
        log_say(DiploLabelSendingWITHDRAWAL, (a), (b), 0);                   \
        message_data(0x244A, 0, (a), (b), 0, 0);                             \
        NetDaemonNet.await_diplo(0x44A);                                     \
    } else {                                                                 \
        do_withdrawal((a), (b));                                             \
    }

// Complete retained communicate continuation; the prior unfinished
// binding/history below remains a historical restoration obligation.
int communicate_round_down_multiple(int value, int step);

// The image expands no inline helper anywhere in communicate.
#pragma inline_depth(0)
// ORIGINAL: 0x0054FFD0 ?communicate@@YAXHHH@Z 0x0054FFD0-0x00558980;0x0065FF4F-0x0065FF64
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0054FFD0-0x00558980;0x0065FF4F-0x0065FF64", "source:src/unrecovered/0054ffd0.cpp:retained-seed", "task:base-colony-economy-coverage-wave:diplomacy-communicate-link-continuity-20261003", "Retired seed src/unrecovered/0054ffd0.cpp retains its complete original identity, hot/cold spans and historical findings in this owner", "SocialEffect.values_ is the genuine eleven-word indexed owner; original out-of-domain selector effects remain unresolved", "recovery/tests/test_communicate_owner.py: actual-source bounded behavior and mutation controls", "recovery/tasks/owner-coverage-wave-oct6.json"], "reason": "Recover the complete diplomacy conversation, topic and action paths through real faction, Base, social-effect array and UI owners, preserving original unchecked selectors, arithmetic, error and uninitialized-state behavior. Prior unfinished decision retained as history: Explicitly unfinished owner body for link continuity while complete communicate hot/cold behavior remains under recovery. The seed and original mapping are retained; this empty body does not implement diplomacy, reproduce its bytes or satisfy the pending dependency."}
// TRIED: the seven-way relationship-topic greeting text build (war/ vendetta/truce/treaty/pact, chosen via has_treaty bits and distance, each with its own get_adjective_leader/ get_noun_phrase string chain and jump-table case 0-6) was collapsed to one generic diplomacy_caption() + say_num(0) call. The diplomacy_menu() result loop and its 8-way action dispatch (cases 0/1-2/3/4/5/6/7) were read from the raw disassembly (no Ghidra hypothesis was reliable there either) and reproduced with real callee names/order; the patience/ loop-exit condition nesting is approximated, not exact.
void __cdecl communicate(int faction_id, int faction_id_with, int mode) {
// Single shared function-scope declarations for the private whole-body
// candidate and typed host harness. They are not an installed source edit.
int communicate_stack_c;
int communicate_stack_2c;
int communicate_territory_base_id;
int communicate_getout_result;
int communicate_late_contact_base_id;
int communicate_late_contact_best_tech;
int communicate_late_contact_best_tech_value;
int communicate_late_contact_bully_random, communicate_late_contact_choice;
int communicate_late_contact_rank;
int communicate_late_contact_research_tech;
int communicate_late_contact_tech, communicate_late_contact_tech_mask;
int communicate_late_contact_tech_mask_offset;
int communicate_late_contact_text_result;
int communicate_late_contact_vendetta_response;
int communicate_vendetta_text_variant;
int communicate_late_offer_amount, communicate_late_offer_eligible;
int communicate_late_offer_range;
int communicate_late_offer_special;
int communicate_local_menu_mode;
int communicate_rival_faction;
int communicate_special_war_region;
int communicate_special_war_selected_base_id;
int communicate_special_war_smallest_population;
char *communicate_patience_ptr;

int communicate_pair_treaties;
int communicate_plea_reserves;
int communicate_plea_amount;
char communicate_demand_suffix[24];
char communicate_demand_number[80];
int communicate_demand_variant;
int communicate_demand_friction;
int communicate_temp;
int communicate_map_cost;
int communicate_gift_kind;
int communicate_best_tech;
int communicate_best_value;
int communicate_tech_id;
int communicate_tech_value;
int communicate_gift_base;
int communicate_gift;
int communicate_news_faction;
int communicate_ransom_base;
int communicate_ransom_size;
int communicate_wronged_id;
int communicate_wronged_count;
int communicate_bribe_half;
int communicate_bribe_weasel;
int communicate_bribe_multiplier;
int communicate_bribe_result;
int communicate_tech_price;
int communicate_tech_result;
int communicate_loan_amount;
int communicate_loan_limit;
int communicate_loan_rate;
int communicate_loan_term;
int communicate_loan_factor;
int communicate_loan_total;
int communicate_loan_payment;
int communicate_loan_balance;
int communicate_loan_result;
int communicate_truce_result;
int communicate_truce_variant;
int communicate_truce_limit;
int communicate_truce_tech;
int communicate_truce_energy;
int communicate_truce_mine;
int communicate_truce_theirs;
int communicate_truce_other;
int communicate_truce_shared;
int communicate_report_min;
int communicate_report_third;
int communicate_report_region_rival;
int communicate_report_rival;
int communicate_report_region;

char communicate_getout_number[80];
char communicate_goaway_number[80];
char communicate_outrage_number[80];
int communicate_stack_20 = 0;
int communicate_stack_38 = 0;
int communicate_stack_3c = 0;
int communicate_stack_34 = 0;
int communicate_stack_54 = 0;
int communicate_stack_1c = 0;
int communicate_stack_48 = 0;
Popup popup;
PopupDialogFactionID = faction_id_with;
DiplomacyActive = 1;
DiploScratchFA8C = 0;
int communicate_stack_44 =
    !(PlayersData[faction_id_with].diplo_treaties[faction_id] & 0x8000000) ||
    (IsMultiplayerNet && (FactionDiplomacyFlags[faction_id_with] & 2));
DiploScratchFA48 = 0;
int communicate_stack_50 =
    (PlayersData[faction_id].diplo_treaties[faction_id_with] & 4) &&
    !(PlayersData[faction_id].diplo_treaties[faction_id_with] & 0x10000);
int communicate_stack_14 = 0;
DiplomacyContactResult = mode ? 3 : 0;
if (communicate_stack_44) {
    int category = Players[faction_id_with].soc_ideology_category;
    if (category >= 0) {
        int model = Players[faction_id_with].soc_ideology_model;
        if (model) {
            int pending = PlayersData[faction_id].soc_category_pending.models_[category];
            if (pending && pending != model) {
                COMMUNICATE_FRICTION(3);
            }
        }
    }
    int effect = Players[faction_id_with].soc_ideology_effect;
    if (effect >= 0 &&
        PlayersData[faction_id].soc_effect_pending.values_[effect] < 0) {
        COMMUNICATE_FRICTION(3);
    }
}
diplomacy_check(faction_id, faction_id_with, 1);
int dialogue_flags = FactionDiplomacyFlags[faction_id_with];
FactionDiplomacyFlags[faction_id_with] = 0;
if (dialogue_flags & 0x2C) {
    if (dialogue_flags & 0x24) {
        DiplomacyContactResult = 8;
    }
    if (!wants_to_speak(faction_id, faction_id_with)) {
        goto communicate_final_cleanup;
    }
    if (dialogue_flags & 4) {
        territory(faction_id_with, faction_id, 0, &communicate_territory_base_id, 0);
        diplomacy_caption(faction_id, faction_id_with);
        if (Bases[communicate_territory_base_id].faction_id_current != faction_id &&
            !(Bases[communicate_territory_base_id].visibility & (1 << faction_id))) {
            spot_base(communicate_territory_base_id, faction_id);
            draw_tile(Bases[communicate_territory_base_id].x,
                      Bases[communicate_territory_base_id].y, 2);
            ConsoleGlobal.focus(Bases[communicate_territory_base_id].x,
                                Bases[communicate_territory_base_id].y, faction_id);
        }
        parse_says(0, Bases[communicate_territory_base_id].name_string, -1, -1);
        GenderDefault = Players[faction_id_with].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction_id_with].title_leader, -1, -1);
        GenderDefault = Players[faction_id_with].is_leader_female;
        PluralityDefault = 0;
        parse_says(2, Players[faction_id_with].name_leader, -1, -1);
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(3, Players[faction_id].title_leader, -1, -1);
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(4, Players[faction_id].name_leader, -1, -1);
        parse_says(5, Players[faction_id].adj_insult_faction, -1, -1);
        parse_says(6, Players[faction_id_with].adj_name_faction, -1, -1);
        StringTemp[0] = 0;
        strcat(StringTemp, "GETOUT");
        _itoa(random(0, 5) + 1, communicate_getout_number, 10);
        strcat(StringTemp, communicate_getout_number);
        communicate_getout_result = X_pops(StringTemp, 0x100000,
                            FactionDiplomacyPortraits[faction_id_with], pop_wait);
        if (!communicate_getout_result) {
            COMMUNICATE_LOCK();
            COMMUNICATE_WITHDRAW(faction_id_with, faction_id);
            COMMUNICATE_UNLOCK();
        } else if (communicate_getout_result == 1) {
            if (DiploScratchFA60 &&
                ((PlayersData[faction_id_with].diplo_treaties[faction_id] & 0x800) ||
                 !DiploScratchFA98) &&
                ((PlayersData[faction_id_with].diplo_treaties[faction_id] & 0x20) ||
                 (DiploFriction >= (5 - PlayersData[faction_id_with].ai_fight) * 3 &&
                  PlayersData[faction_id_with].mil_strength_1 >
                      PlayersData[faction_id].mil_strength_1))) {
                communicate_stack_38 = 1;
            communicate_introduction_conversation:
                communicate_local_menu_mode =
                    DiploScratchF7E4 < 0 || mode ||
                    has_treaty(faction_id_with, faction_id, 0x2000) ||
                    has_agenda(faction_id_with, faction_id, 4);
                goto communicate_late_contact_vendetta_after_set_mode;
            }
            COMMUNICATE_LOCK();
            COMMUNICATE_WITHDRAW(faction_id_with, faction_id);
            COMMUNICATE_WITHDRAW(faction_id, faction_id_with);
            COMMUNICATE_UNLOCK();
            X_pops("BOTHOUT", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
        } else {
            COMMUNICATE_FRICTION(10);
            COMMUNICATE_LOCK();
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingDOUBLECROSS, faction_id, faction_id_with, -1);
                message_data(0x2440, 0, faction_id, faction_id_with, -1, 0);
                NetDaemonNet.await_diplo(0x440);
            } else {
                double_cross(faction_id, faction_id_with, -1);
            }
            COMMUNICATE_UNLOCK();
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = 0;
            parse_says(2, Players[faction_id].insult_leader, -1, -1);
            parse_says(3, Players[faction_id].adj_insult_faction, -1, -1);
            StringTemp[0] = 0;
            strcat(StringTemp, "OUTRAGE");
            _itoa(random(0, 3) + 1, communicate_outrage_number, 10);
        strcat(StringTemp, communicate_outrage_number);
            X_pops(StringTemp, 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
            goto communicate_final_cleanup;
        }
    } else if (dialogue_flags & 8) {
        territory(faction_id_with, faction_id, 0, &communicate_territory_base_id, 0);
        if (dialogue_flags & 0x10) {
            if (IsMultiplayerNet) {
                log_say(DiploLabelSendingSETTREATY, faction_id, faction_id_with, 0x400);
                message_data(0x2443, 0, faction_id, faction_id_with, 0x400, 1);
                NetDaemonNet.await_diplo(0x443);
            } else {
                set_treaty(faction_id, faction_id_with, 0x400, 1);
            }
        }
        if (Bases[communicate_territory_base_id].faction_id_current != faction_id &&
            !(Bases[communicate_territory_base_id].visibility & (1 << faction_id))) {
            spot_base(communicate_territory_base_id, faction_id);
            draw_tile(Bases[communicate_territory_base_id].x,
                      Bases[communicate_territory_base_id].y, 2);
        }
        ConsoleGlobal.focus(Bases[communicate_territory_base_id].x,
                            Bases[communicate_territory_base_id].y, faction_id);
        parse_says(0, Bases[communicate_territory_base_id].name_string, -1, -1);
        StringTemp[0] = 0;
        strcat(StringTemp, "GOAWAY");
        _itoa(random(0, 2) + 1, communicate_goaway_number, 10);
        strcat(StringTemp, communicate_goaway_number);
        X_pops(StringTemp, 0x100000,
               FactionDiplomacyPortraits[faction_id_with], pop_wait);
    } else if (dialogue_flags & 0x20) {
        territory(faction_id_with, faction_id, 0, &communicate_territory_base_id, 0);
        if (communicate_territory_base_id >= 0 &&
            Bases[communicate_territory_base_id].faction_id_current != faction_id_with &&
            !(Bases[communicate_territory_base_id].visibility & (1 << faction_id_with))) {
            spot_base(communicate_territory_base_id, faction_id_with);
            draw_map(1);
        }
        parse_says(0, Bases[communicate_territory_base_id].name_string, -1, -1);
        parse_says(1, Players[faction_id_with].adj_name_faction, -1, -1);
        X_pops("INOURTERRITORY2", 0x100000,
               FactionDiplomacyPortraits[faction_id_with], pop_wait);
    }
    if (!(dialogue_flags & 3)) {
        goto communicate_final_cleanup;
    }
}

if (!mode) {
    PlayersData[faction_id_with].diplo_patience[faction_id] = 0;
    PlayersData[faction_id].diplo_spoke[faction_id_with] =
        GameDataStateStorage.fields_.turn_current_num_;
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTREATYOFF, faction_id_with, faction_id, 0x4000000);
        message_data(0x2442, 0, faction_id_with, faction_id, 0x4000000, 0);
        NetDaemonNet.await_diplo(0x442);
    } else {
        treaty_off(faction_id_with, faction_id, 0x4000000);
    }
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingAGENDAOFF, faction_id_with, faction_id, 0x100);
        message_data(0x2445, 0, faction_id_with, faction_id, 0x100, 0);
        NetDaemonNet.await_diplo(0x445);
    } else {
        agenda_off(faction_id_with, faction_id, 0x100);
    }
    if (!(PlayersData[faction_id_with].diplo_treaties[faction_id] & 0x10000) &&
        dialogue_flags < 4) {
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYOFF, faction_id_with, faction_id, 4);
            message_data(0x2442, 0, faction_id_with, faction_id, 4, 0);
            NetDaemonNet.await_diplo(0x442);
        } else {
            treaty_off(faction_id_with, faction_id, 4);
        }
    }
} else {
    if (PlayersData[faction_id].diplo_treaties[faction_id_with] & 0x2000) {
        PlayersData[faction_id_with].diplo_patience[faction_id] = 0;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYOFF, faction_id_with, faction_id, 0x100);
            message_data(0x2442, 0, faction_id_with, faction_id, 0x100, 0);
            NetDaemonNet.await_diplo(0x442);
        } else {
            treaty_off(faction_id_with, faction_id, 0x100);
        }
    }
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(2, Players[faction_id].title_leader, -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(3, Players[faction_id].name_leader, -1, -1);
    if (!communicate_stack_44) {
        int treaties = PlayersData[faction_id_with].diplo_treaties[faction_id];
        int patience_limit;
        if (treaties & 0x10) {
            patience_limit = 1;
        } else if (treaties & 1) {
            patience_limit = (treaties & 0x2000000) ? 500 : 6;
        } else {
            patience_limit = ((treaties & 2) != 0) - (DiploFriction + 3) / 8 + 3;
        }
        if (PlayersData[faction_id_with].diplo_patience[faction_id] >= patience_limit ||
            (treaties & 0x140)) {
            if (!DiplomacyEndState) {
                diplomacy_caption(faction_id, faction_id_with);
            }
            if (PlayersData[faction_id].diplo_treaties[faction_id_with] & 3) {
                PluralityDefault = 0;
                GenderDefault = Players[faction_id_with].is_leader_female;
                parse_says(0, Players[faction_id_with].title_leader, -1, -1);
                PluralityDefault = 0;
                GenderDefault = Players[faction_id_with].is_leader_female;
                parse_says(1, Players[faction_id_with].name_leader, -1, -1);
                X_pops("NOPATIENCEFRIEND", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            } else {
                GenderDefault = Players[faction_id_with].noun_gender;
                PluralityDefault = Players[faction_id_with].is_noun_plural;
                parse_says(0, Players[faction_id_with].noun_faction, -1, -1);
                X_pops("NOPATIENCE", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            }
            goto communicate_final_cleanup;
        }
    }
    wants_to_speak(faction_id, faction_id_with);
}
communicate_after_intro:
communicate_pair_treaties = PlayersData[faction_id_with].diplo_treaties[faction_id];
if (communicate_pair_treaties & 0x40000) {
    DiploScratchFA60 = 1;
    DiploScratchFA88 = -1;
    goto communicate_introduction_conversation;
}
if (communicate_pair_treaties & 1) {
    if (communicate_pair_treaties & 0x2000000) {
        tech_trade(faction_id, faction_id_with, 1, 4, 4);
        goto communicate_after_trade;
    }
    if (DiploScratchFA64 && DiploScratchFA90) {
        DiploScratchF7E8 = 0;
    }
} else if (DiploScratchFA98 && !DiploScratchFA60 && !mode &&
           !(dialogue_flags & 0x40) && (communicate_pair_treaties & 7)) {
    tech_trade(faction_id, faction_id_with, 1, 4, 4);
    goto communicate_final_cleanup;
}
tech_analysis(faction_id, faction_id_with);
if (dialogue_flags & 0x40) {
if (!DiploScratchF7E8 || !wants_to_speak(faction_id, faction_id_with)) {
    goto communicate_tech_greeting_done;
}
if (PlayersData[DiploScratchF7E8].mil_strength_1 >=
        PlayersData[faction_id_with].mil_strength_1 * 2 &&
    PlayersData[faction_id_with].energy_reserves >= 10) {
communicate_plea_reserves = PlayersData[faction_id_with].energy_reserves;
communicate_plea_amount = communicate_plea_reserves / 2 / 25 * 25;
if (communicate_plea_amount < 25) {
    communicate_plea_amount = communicate_plea_reserves;
    if (communicate_plea_reserves >= 25) {
        communicate_plea_amount = 25;
    }
}
parse_num(0, communicate_plea_amount);
PluralityDefault = 0;
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
parse_says(0, Players[DiploScratchF7E8].title_leader, -1, -1);
PluralityDefault = 0;
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
parse_says(1, Players[DiploScratchF7E8].name_leader, -1, -1);
GenderDefault = Players[DiploScratchF7E8].noun_gender;
PluralityDefault = Players[DiploScratchF7E8].is_noun_plural;
parse_says(2, Players[DiploScratchF7E8].noun_faction, -1, -1);
parse_says(3, get_pact_hood(faction_id, faction_id_with), -1, -1);
parse_says(5, get_pact_hood(faction_id, DiploScratchF7E8), -1, -1);
parse_says(6, get_his_her(DiploScratchF7E8, 0), -1, -1);
parse_says(8, Players[DiploScratchF7E8].adj_insult_leader, -1, -1);
parse_says(9, get_phrase(DiploScratchF7E8, 2), -1, -1);
StringTemp[0] = 0;
strcat(StringTemp, "PLEA");
if (PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 1) {
    strcat(StringTemp, "PACT");
}
if (DiploScratchF800 >= 0) {
    strcat(StringTemp, "TECH");
    parse_say(4, Technology[DiploScratchF800].name, -1, -1);
}
if (X_pops(StringTemp, 0x100000,
            FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
COMMUNICATE_LOCK();
pact_of_brotherhood(faction_id, faction_id_with);
if (IsMultiplayerNet) {
    log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, 0x60);
    message_data(0x2442, 0, faction_id, faction_id_with, 0x60, 0);
} else {
    treaty_off(faction_id, faction_id_with, 0x60);
}
communicate_stack_2c = DiploScratchF7E8;
if (IsMultiplayerNet) {
    log_say(DiploLabelSendingDOUBLECROSS, faction_id, communicate_stack_2c, faction_id_with);
    message_data(0x2440, 0, faction_id, communicate_stack_2c, faction_id_with, 0);
} else {
    double_cross(faction_id, communicate_stack_2c, faction_id_with);
}
if (DiploScratchF800 >= 0) {
    communicate_stack_14 = DiploScratchF800;
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTECH, faction_id, communicate_stack_14, faction_id_with);
        message_data(0x244B, 0, faction_id, communicate_stack_14, faction_id_with, 0);
    } else {
        tech_achieved(faction_id, communicate_stack_14, faction_id_with, 0);
    }
    DiploScratchF800 = -1;
}
communicate_stack_2c = -communicate_plea_amount;
if (IsMultiplayerNet) {
    log_say(DiploLabelSendingENERGY, faction_id_with, communicate_stack_2c, faction_id);
    message_data(0x2447, 0, faction_id_with, communicate_stack_2c, faction_id,
                 communicate_plea_amount);
    NetDaemonNet.await_diplo(0x447);
} else {
    if (faction_id_with) {
        PlayersData[faction_id_with].energy_reserves += communicate_stack_2c;
    }
    if (faction_id) {
        PlayersData[faction_id].energy_reserves += communicate_plea_amount;
    }
    ConsoleGlobal.update_data(1);
}
communicate_stack_14 = 1;
COMMUNICATE_UNLOCK();
DiploScratchFA8C = DiploScratchF7E8;
DiploScratchF7E8 = 0;
DiploScratchFA98 = 0;
DiploScratchFA88 = 0;
DiploScratchFA68 = 0;
DiploScratchFA60 = 0;
communicate_stack_20 = 1;
DiploScratchFA4C = 0;
communicate_stack_34 = 1;
goto communicate_tech_greeting_done;
} else {
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, 0x800000);
        message_data(0x2441, 0, faction_id, faction_id_with, 0x800000, 0);
    } else {
        treaty_on(faction_id, faction_id_with, 0x800000);
    }
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, 0x1000000);
        message_data(0x2442, 0, faction_id, faction_id_with, 0x1000000, 0);
    } else {
        treaty_off(faction_id, faction_id_with, 0x1000000);
    }
    DiploScratchF7B8 = DiploScratchF7E8;
    goto communicate_tech_greeting_done;
}
} else {

PluralityDefault = 0;
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
parse_says(0, Players[DiploScratchF7E8].title_leader, -1, -1);
PluralityDefault = 0;
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
parse_says(1, Players[DiploScratchF7E8].name_leader, -1, -1);
GenderDefault = Players[DiploScratchF7E8].noun_gender;
PluralityDefault = Players[DiploScratchF7E8].is_noun_plural;
parse_says(2, Players[DiploScratchF7E8].noun_faction, -1, -1);
parse_says(3, get_him_her(DiploScratchF7E8, 0), -1, -1);
parse_says(4, get_pact_hood(faction_id, faction_id_with), -1, -1);
PluralityDefault = 0;
GenderDefault = Players[faction_id].is_leader_female;
parse_says(5, Players[faction_id].title_leader, -1, -1);
parse_says(6, get_pact(DiploScratchF7E8), -1, -1);
StringTemp[0] = 0;
strcat(StringTemp, "PROPOSE");
if (PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 1) {
    strcat(StringTemp, "PACT");
    PluralityDefault = 0;
    GenderDefault = Players[faction_id_with].is_leader_female;
    parse_says(8, Players[faction_id_with].title_leader, -1, -1);
    parse_says(9, get_phrase(DiploScratchF7E8, 2), -1, -1);
} else {
    parse_says(8, Players[DiploScratchF7E8].adj_insult_faction, -1, -1);
    parse_says(9, get_phrase(DiploScratchF7E8, 5), -1, -1);
}
if (X_pops(StringTemp, 0x100000,
            FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
COMMUNICATE_LOCK();
pact_of_brotherhood(faction_id, faction_id_with);
if (IsMultiplayerNet) {
    log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, 0x60);
    message_data(0x2442, 0, faction_id, faction_id_with, 0x60, 0);
} else {
    treaty_off(faction_id, faction_id_with, 0x60);
}
communicate_stack_2c = DiploScratchF7E8;
if (IsMultiplayerNet) {
    log_say(DiploLabelSendingDOUBLECROSS, faction_id, communicate_stack_2c, faction_id_with);
    message_data(0x2440, 0, faction_id, communicate_stack_2c, faction_id_with, 0);
    NetDaemonNet.await_diplo(0x440);
} else {
    double_cross(faction_id, communicate_stack_2c, faction_id_with);
}
COMMUNICATE_UNLOCK();
DiploScratchFA8C = DiploScratchF7E8;
DiploScratchF7E8 = 0;
DiploScratchFA98 = 0;
DiploScratchFA88 = 0;
DiploScratchFA68 = 0;
DiploScratchFA60 = 0;
communicate_stack_20 = 1;
DiploScratchFA4C = 0;
communicate_stack_34 = 1;
goto communicate_tech_greeting_done;
} else {
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, 0x800000);
        message_data(0x2441, 0, faction_id, faction_id_with, 0x800000, 0);
    } else {
        treaty_on(faction_id, faction_id_with, 0x800000);
    }
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, 0x1000000);
        message_data(0x2442, 0, faction_id, faction_id_with, 0x1000000, 0);
    } else {
        treaty_off(faction_id, faction_id_with, 0x1000000);
    }
    DiploScratchF7B8 = DiploScratchF7E8;
    goto communicate_tech_greeting_done;
}
}

} else {
if (!(PlayersData[faction_id_with].diplo_agenda[faction_id] & 0x2408) &&
    !(PlayersData[faction_id_with].diplo_treaties[faction_id] & 0x40) &&
    (!DiploScratchFAA0 || communicate_stack_44)) {
if (DiploScratchF7E8 &&
    !(PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 0x80) &&
    (DiploScratchFA60 || !DiploScratchFA80) &&
    (!DiploScratchFA98 || DiplomacyProposalGateCount > 0 ||
     (DiploScratchFA60 && !DiploScratchFA70) ||
     (PlayersData[faction_id].ranking >= 6 &&
      PlayersData[faction_id_with].ranking <= 4 && !DiploScratchFA54 &&
      !DiploScratchF7FC) ||
     great_satan(DiploScratchF7E8, 0) ||
     (PlayersData[faction_id_with].diplo_treaties[faction_id] & 1)) &&
    wants_to_speak(faction_id, faction_id_with)) {
    parse_says(0, get_pact(DiploScratchF7E8), -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[DiploScratchF7E8].is_leader_female;
    parse_says(1, Players[DiploScratchF7E8].title_leader, -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[DiploScratchF7E8].is_leader_female;
    parse_says(2, Players[DiploScratchF7E8].name_leader, -1, -1);
    GenderDefault = Players[DiploScratchF7E8].noun_gender;
    PluralityDefault = Players[DiploScratchF7E8].is_noun_plural;
    parse_says(3, Players[DiploScratchF7E8].noun_faction, -1, -1);
    parse_says(4, get_his_her(DiploScratchF7E8, 0), -1, -1);
    parse_says(5, get_him_her(DiploScratchF7E8, 0), -1, -1);
    parse_says(6, Players[DiploScratchF7E8].adj_name_faction, -1, -1);
    communicate_demand_suffix[0] = 0;
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with] & 1) &&
        ((PlayersData[faction_id].diplo_treaties[faction_id_with] & 0x2000) ||
         !(PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 8))) {
        communicate_demand_variant = 3;
    } else if (PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 1) {
        communicate_demand_variant = 0;
        parse_says(9, get_phrase(DiploScratchF7E8, 5), -1, -1);
    } else if (PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 2) {
        communicate_demand_variant = 1;
        parse_says(6, get_noun_phrase(DiploScratchF7E8, 8), -1, -1);
        parse_says(9, get_phrase(DiploScratchF7E8, 5), -1, -1);
        parse_says(8, get_phrase(DiploScratchF7E8, 2), -1, -1);
        if (random(0, 2) || (PlayersData[faction_id].diplo_treaties[faction_id_with] & 0x20)) {
            strcat(communicate_demand_suffix, "a");
        } else {
            strcat(communicate_demand_suffix, "b");
        }
    } else if ((PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 4) &&
               !(PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 0x10000)) {
        communicate_demand_variant = 2;
        if (random(0, 2)) {
            strcat(communicate_demand_suffix, "a");
            parse_says(8, Players[DiploScratchF7E8].adj_insult_leader, -1, -1);
            parse_says(9, Players[DiploScratchF7E8].adj_insult_faction, -1, -1);
        } else {
            strcat(communicate_demand_suffix, "b");
            parse_says(8, get_noun_phrase(faction_id_with, 9), -1, -1);
            parse_says(9, get_phrase(DiploScratchF7E8, 2), -1, -1);
        }
    } else {
        communicate_demand_variant = 4;
    }
    StringTemp[0] = 0;
    strcat(StringTemp, "DEMANDATTACK");
    _itoa(communicate_demand_variant, communicate_demand_number, 10);
    strcat(StringTemp, communicate_demand_number);
    strcat(StringTemp, communicate_demand_suffix);
    if (!X_pops(StringTemp, 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
        if (PlayersData[faction_id_with].diplo_treaties[faction_id] & 1) {
            communicate_stack_38 = 1;
            if (PlayersData[faction_id_with].diplo_treaties[faction_id] & 0x2000) {
                DiploScratchFA60 = 1;
            }
            communicate_demand_friction =
                PlayersData[faction_id_with].diplo_friction[faction_id] + 4;
            PlayersData[faction_id_with].diplo_friction[faction_id] =
                communicate_demand_friction < 0 ? 0
                : (communicate_demand_friction > 20 ? 20 : communicate_demand_friction);
            if (DiploFrictionFactionID == faction_id_with &&
                DiploFrictionFactionIDWith == faction_id) {
                DiploFriction += 4;
            }
        } else if (DiploScratchFA4C && DiploFriction >= 10 && !DiploScratchFA98 &&
                   PlayersData[faction_id_with].mil_strength_1 >
                       PlayersData[faction_id].mil_strength_1) {
            DiploScratchFA60 = 1;
        }
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, 0x800000);
            message_data(0x2441, 0, faction_id, faction_id_with, 0x800000, 0);
        } else {
            treaty_on(faction_id, faction_id_with, 0x800000);
        }
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, 0x1000000);
            message_data(0x2442, 0, faction_id, faction_id_with, 0x1000000, 0);
        } else {
            treaty_off(faction_id, faction_id_with, 0x1000000);
        }
        DiploScratchF7B8 = DiploScratchF7E8;
        diplomacy_caption(faction_id, faction_id_with);
        goto communicate_menu_continuation;
    }
    COMMUNICATE_LOCK();
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, 0x1000000);
        message_data(0x2441, 0, faction_id, faction_id_with, 0x1000000, 0);
    } else {
        treaty_on(faction_id, faction_id_with, 0x1000000);
    }
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, 0x800000);
        message_data(0x2442, 0, faction_id, faction_id_with, 0x800000, 0);
    } else {
        treaty_off(faction_id, faction_id_with, 0x800000);
    }
    communicate_stack_2c = DiploScratchF7E8;
    if (IsMultiplayerNet) {
        log_say(DiploLabelSendingDOUBLECROSS, faction_id, communicate_stack_2c, faction_id_with);
        message_data(0x2440, 0, faction_id, communicate_stack_2c, faction_id_with, 0);
        NetDaemonNet.await_diplo(0x440);
    } else {
        double_cross(faction_id, communicate_stack_2c, faction_id_with);
    }
    COMMUNICATE_UNLOCK();
    DiploScratchFA8C = DiploScratchF7E8;
    DiploScratchF7E8 = 0;
    communicate_stack_20 = 1;
    DiploScratchFA4C = 0;
    diplomacy_caption(faction_id, faction_id_with);
    goto communicate_menu_continuation;
}
if (DiploScratchFA60 || DiploScratchFA70) {
    goto communicate_menu_continuation;
}

if (DiploScratchF7D4 &&
    !(PlayersData[faction_id].diplo_treaties[DiploScratchF7D4] & 0x8000008) &&
    (!(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << DiploScratchF7D4)) ||
     ((!IsMultiplayerNet || !Rules.tgl_human_always_contact_net) &&
      (!IsMultiplayerPBEM || !Rules.tgl_humans_always_contact_pbem))) &&
    !(PlayersData[faction_id].diplo_treaties[faction_id_with] & 0x31) &&
    !great_satan(faction_id, 0) &&
    (PlayersData[faction_id].ranking <= 5 ||
     PlayersData[faction_id_with].ranking <= 4) &&
    great_satan(DiploScratchF7D4, 0) +
            (PlayersData[DiploScratchF7D4].ranking > PlayersData[faction_id].ranking) +
            (DiploScratchFA54 == 0) + (IsMultiplayerNet == 0) >= 2 &&
    !(Players[faction_id].rule_flags & 0x80) &&
    !(Players[faction_id_with].rule_flags & 0x80) &&
    wants_to_speak(faction_id, faction_id_with)) {
    PluralityDefault = 0;
    GenderDefault = Players[DiploScratchF7D4].is_leader_female;
    parse_says(0, Players[DiploScratchF7D4].title_leader, -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[DiploScratchF7D4].is_leader_female;
    parse_says(1, Players[DiploScratchF7D4].name_leader, -1, -1);
    GenderDefault = Players[DiploScratchF7D4].noun_gender;
    PluralityDefault = Players[DiploScratchF7D4].is_noun_plural;
    parse_says(2, Players[DiploScratchF7D4].noun_faction, -1, -1);
    parse_says(3, get_phrase(DiploScratchF7D4, 5), -1, -1);
    parse_says(4, get_pact_hood(faction_id, faction_id_with), -1, -1);
    if (Players[DiploScratchF7E8].rule_flags & 0x80) {
        if (!X_pops("INTRODUCEENEMYALIEN", 0x100000,
                    FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
            goto communicate_menu_continuation;
        }
        COMMUNICATE_LOCK();
        communicate_temp = DiploScratchF7D4;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingSETTREATY, faction_id, communicate_temp, 0x10000000);
            message_data(0x2443, 0, faction_id, communicate_temp, 0x10000000, 1);
        } else {
            set_treaty(faction_id, communicate_temp, 0x10000000, 1);
        }
        pact_of_brotherhood(faction_id, faction_id_with);
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, 0x1000000);
            message_data(0x2441, 0, faction_id, faction_id_with, 0x1000000, 0);
        } else {
            treaty_on(faction_id, faction_id_with, 0x1000000);
        }
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, 0x800000);
            message_data(0x2442, 0, faction_id, faction_id_with, 0x800000, 0);
        } else {
            treaty_off(faction_id, faction_id_with, 0x800000);
        }
        communicate_temp = DiploScratchF7D4;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingDOUBLECROSS, faction_id, communicate_temp, faction_id_with);
            message_data(0x2440, 0, faction_id, communicate_temp, faction_id_with, 0);
            NetDaemonNet.await_diplo(0x440);
        } else {
            double_cross(faction_id, communicate_temp, faction_id_with);
        }
        communicate_temp = DiploScratchF7D4;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYON, faction_id, communicate_temp, 0x8000008);
            message_data(0x2441, 0, faction_id, communicate_temp, 0x8000008, 0);
        } else {
            treaty_on(faction_id, communicate_temp, 0x8000008);
        }
        COMMUNICATE_UNLOCK();
    } else {
        if (!X_pops("INTRODUCEENEMY", 0x100000,
                    FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
            goto communicate_menu_continuation;
        }
        COMMUNICATE_LOCK();
        communicate_temp = DiploScratchF7D4;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingSETTREATY, faction_id, communicate_temp, 0x10000000);
            message_data(0x2443, 0, faction_id, communicate_temp, 0x10000000, 1);
        } else {
            set_treaty(faction_id, communicate_temp, 0x10000000, 1);
        }
        pact_of_brotherhood(faction_id, faction_id_with);
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, 0x1000000);
            message_data(0x2441, 0, faction_id, faction_id_with, 0x1000000, 0);
        } else {
            treaty_on(faction_id, faction_id_with, 0x1000000);
        }
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, 0x800000);
            message_data(0x2442, 0, faction_id, faction_id_with, 0x800000, 0);
        } else {
            treaty_off(faction_id, faction_id_with, 0x800000);
        }
        communicate_temp = DiploScratchF7D4;
        if (IsMultiplayerNet) {
            log_say(DiploLabelSendingDOUBLECROSS, faction_id, communicate_temp, faction_id_with);
            message_data(0x2440, 0, faction_id, communicate_temp, faction_id_with, 0);
            NetDaemonNet.await_diplo(0x440);
        } else {
            double_cross(faction_id, communicate_temp, faction_id_with);
        }
        net_treaty_on(faction_id, DiploScratchF7D4, 0x8000008, 0);
        diplo_unlock();
    }
    DiploScratchFA8C = DiploScratchF7D4;
    DiploScratchF7D4 = 0;
    DiploScratchF7E8 = 0;
    communicate_stack_20 = 1;
    DiploScratchFA4C = 0;
    communicate_stack_34 = 1;
    goto communicate_menu_continuation;
}

if (DiploScratchFA38 && !has_treaty(faction_id, faction_id_with, 0x31) &&
    !has_treaty(faction_id, DiploScratchFA38, 0x10) &&
    (DiploScratchFA54 < 2 || great_satan(DiploScratchFA38, 0) ||
     (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << DiploScratchFA38))) &&
    wants_to_attack(faction_id_with, DiploScratchFA38, faction_id) &&
    (PlayersData[faction_id].ranking <= 5 || PlayersData[faction_id_with].ranking <= 4 ||
     great_satan(DiploScratchFA38, 0)) &&
    (contiguous(faction_id, DiploScratchFA38, 0, 0) ||
     !contiguous(faction_id_with, DiploScratchFA38, 0, 0)) &&
    wants_to_speak(faction_id, faction_id_with)) {
    parse_says(0, get_name(faction_id), -1, -1);
    parse_says(1, get_title(DiploScratchFA38), -1, -1);
    parse_says(2, get_name(DiploScratchFA38), -1, -1);
    parse_says(3, get_adjective_insult_faction(DiploScratchFA38), -1, -1);
    parse_says(4, get_phrase(DiploScratchFA38, 2), -1, -1);
    parse_says(5, get_pact_hood(faction_id, faction_id_with), -1, -1);
    parse_says(6, get_noun(DiploScratchFA38), -1, -1);
    if (!X_pops("BETRAYFRIEND", 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
        goto communicate_menu_continuation;
    }
    net_set_treaty(faction_id, DiploScratchFA38, 0x10000000, 1, 0);
    diplo_lock(0);
    pact_of_brotherhood(faction_id, faction_id_with);
    net_treaty_on(faction_id, faction_id_with, 0x1000000, 0);
    net_treaty_off(faction_id, faction_id_with, 0x800000, 0);
    net_double_cross(faction_id, DiploScratchFA38, faction_id_with, 1);
    net_treaty_on(faction_id, DiploScratchFA38, 0x8000008, 0);
    diplo_unlock();
    DiploScratchFA8C = DiploScratchFA38;
    DiploScratchFA38 = 0;
    DiploScratchF7E8 = 0;
    communicate_stack_20 = 1;
    DiploScratchFA4C = 0;
    communicate_stack_34 = 1;
    goto communicate_menu_continuation;
}
if (DiploScratchF7C8 && !has_treaty(faction_id, faction_id_with, 0x31) &&
    !has_treaty(faction_id, DiploScratchF7C8, 0x10) &&
    (DiploScratchFA54 < 2 || great_satan(DiploScratchF7C8, 0) ||
     (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << DiploScratchF7C8))) &&
    wants_to_attack(faction_id_with, DiploScratchF7C8, faction_id) &&
    (PlayersData[faction_id].ranking <= 5 || PlayersData[faction_id_with].ranking <= 4 ||
     great_satan(DiploScratchF7C8, 0)) &&
    (contiguous(faction_id, DiploScratchF7C8, 0, 0) ||
     !contiguous(faction_id_with, DiploScratchF7C8, 0, 0)) &&
    wants_to_speak(faction_id, faction_id_with)) {
    parse_says(0, get_name(faction_id), -1, -1);
    parse_says(1, get_title(DiploScratchF7C8), -1, -1);
    parse_says(2, get_name(DiploScratchF7C8), -1, -1);
    parse_says(3, get_adjective_insult_faction(DiploScratchF7C8), -1, -1);
    parse_says(4, get_phrase(DiploScratchF7C8, 2), -1, -1);
    parse_says(5, get_pact_hood(faction_id, faction_id_with), -1, -1);
    parse_says(6, get_noun(DiploScratchF7C8), -1, -1);
    if (!X_pops("BETRAYFRIEND", 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
        goto communicate_menu_continuation;
    }
    diplo_lock(0);
    net_set_treaty(faction_id, DiploScratchF7C8, 0x10000000, 1, 0);
    pact_of_brotherhood(faction_id, faction_id_with);
    net_treaty_on(faction_id, faction_id_with, 0x1000000, 0);
    net_treaty_off(faction_id, faction_id_with, 0x800000, 0);
    net_double_cross(faction_id, DiploScratchF7C8, faction_id_with, 1);
    net_treaty_on(faction_id, DiploScratchF7C8, 0x8000008, 0);
    diplo_unlock();
    DiploScratchFA8C = DiploScratchF7C8;
    DiploScratchF7C8 = 0;
    DiploScratchF7E8 = 0;
    communicate_stack_20 = 1;
    DiploScratchFA4C = 0;
    communicate_stack_34 = 1;
    goto communicate_menu_continuation;
}
if (DiploScratchF7D8 && !has_treaty(faction_id, faction_id_with, 0x31) &&
    !has_treaty(faction_id, DiploScratchF7D8, 0x10000001) &&
    !(PlayersData[faction_id].traded_maps & (1 << DiploScratchF7D8)) &&
    (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << DiploScratchF7D8)) ||
     (PlayersData[faction_id_with].traded_maps & (1 << DiploScratchF7D8))) &&
    ((DiploScratchFAA8 >= 0 &&
      !(GameDataStateStorage.fields_.game_rules_ & 0x2000000)) ||
     PlayersData[faction_id].energy_reserves >=
         (GameDataStateStorage.fields_.turn_current_num_ >= 50 ? 50 : 25)) &&
    wants_to_speak(faction_id, faction_id_with)) {
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_title(DiploScratchF7D8), -1, -1);
    parse_says(3, get_name(DiploScratchF7D8), -1, -1);
    parse_says(4, get_adjective(DiploScratchF7D8), -1, -1);
    clear();
    say_tech(DiploScratchFAA8, 1);
    parse_says(5, StringTemp, -1, -1);
    communicate_map_cost = GameDataStateStorage.fields_.turn_current_num_ >= 50 ? 50 : 25;
    parse_num(0, communicate_map_cost);
    clear();
    says("ENEMYMAP");
    says(PlayersData[faction_id].energy_reserves >= communicate_map_cost ? "1" : "2");
    net_set_treaty(faction_id, DiploScratchF7D8, 0x10000000, 1, 0);
    if (!X_pops(StringTemp, 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
        goto communicate_menu_continuation;
    }
    net_treaty_on(faction_id, faction_id_with, 0x1000000, 0);
    net_tech(faction_id_with, 9999, DiploScratchF7D8, 1);
    net_tech(faction_id, 9999, DiploScratchF7D8, 1);
    if (PlayersData[faction_id].energy_reserves >= communicate_map_cost) {
        net_energy(faction_id, -communicate_map_cost, faction_id_with,
                   communicate_map_cost, 1);
    } else {
        net_tech(faction_id_with, DiploScratchFAA8, faction_id, 1);
    }
    DiploScratchFA48 = 1;
    DiploScratchFAA8 = DiploScratchFA18;
    DiploScratchFA18 = -1;
    communicate_stack_20 = 1;
    DiploScratchF7D8 = 0;
    goto communicate_menu_continuation;
}
if (DiploScratchFA38 && !has_treaty(faction_id, faction_id_with, 0x30) &&
    !has_project(4, faction_id) &&
    !has_treaty(faction_id, DiploScratchFA38, 8) &&
    (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << DiploScratchFA38)) ||
     !auto_contact()) &&
    wants_to_speak(faction_id, faction_id_with)) {
    communicate_gift_kind = 0;
    parse_says(2, get_title(DiploScratchFA38), -1, -1);
    parse_says(3, get_name(DiploScratchFA38), -1, -1);
    parse_says(4, get_noun(DiploScratchFA38), -1, -1);
    parse_says(5, get_noun_phrase(DiploScratchFA38, 9), -1, -1);
    communicate_best_tech = -1;
    communicate_best_value = 0;
    for (communicate_tech_id = 0; communicate_tech_id < 89; communicate_tech_id++) {
        if (has_tech(communicate_tech_id, DiploScratchFA38) &&
            !has_tech(communicate_tech_id, faction_id) &&
            !has_tech(communicate_tech_id, faction_id_with)) {
            communicate_tech_value = tech_val(communicate_tech_id, faction_id, 1);
            if (communicate_tech_value > communicate_best_value) {
                communicate_best_value = communicate_tech_value;
                communicate_best_tech = communicate_tech_id;
            }
        }
    }
    communicate_gift_base = (GameDataStateStorage.fields_.turn_current_num_ / 25 + 1) * 10;
    if (PlayersData[faction_id].ranking < 5) {
        communicate_gift_base /= 2;
    }
    if (PlayersData[faction_id].ranking == 7) {
        communicate_gift_base += communicate_gift_base / 2;
    }
    if (PlayersData[faction_id].energy_reserves >= communicate_gift_base) {
        communicate_gift_kind = 2;
        communicate_gift = communicate_gift_base +
            random(0, GameDataStateStorage.fields_.turn_current_num_ / 10 + 2) * 5;
        if (communicate_gift > PlayersData[faction_id].energy_reserves) {
            communicate_gift -= (unsigned)(communicate_gift -
                PlayersData[faction_id].energy_reserves + 4) / 5 * 5;
        }
        parse_num(0, communicate_gift);
        if (communicate_gift <= 0) {
            communicate_gift_kind = 0;
        }
    }
    if (!communicate_gift_kind && DiploScratchFAA8 >= 0 &&
        GameDataStateStorage.fields_.turn_current_num_ > 25 &&
        !(GameDataStateStorage.fields_.game_rules_ & 0x2000000)) {
        communicate_gift_kind = 1;
        clear();
        say_tech(DiploScratchFAA8, 1);
        parse_says(6, StringTemp, -1, -1);
    }
    if (communicate_best_tech >= 0) {
        parse_say(8, Technology[communicate_best_tech].name, -1, -1);
    }
    clear();
    says(Players[DiploScratchFA38].rule_flags & 0x80 ? "METALIEN" : "METFRIEND");
    say_num(communicate_gift_kind);
    if (communicate_best_tech >= 0) {
        says("a");
    }
    if (!X_pops(StringTemp, 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
        goto communicate_menu_continuation;
    }
    net_treaty_on(faction_id, DiploScratchFA38, 8, 0);
    net_treaty_on(faction_id, faction_id_with, 0x1000000, 0);
    switch (communicate_gift_kind) {
    case 1:
        cause_friction(faction_id_with, faction_id, -1);
        net_tech(faction_id_with, DiploScratchFAA8, faction_id, 1);
        DiploScratchFAA8 = -1;
        break;
    case 2:
        cause_friction(faction_id_with, faction_id, -1);
        net_energy(faction_id, -communicate_gift, faction_id_with, communicate_gift, 1);
        break;
    }
    goto communicate_menu_continuation;
}
if (DiploScratchF7C8 && !has_treaty(faction_id, faction_id_with, 0x30) &&
    !has_treaty(faction_id, DiploScratchF7C8, 0x10000001) &&
    !(PlayersData[faction_id].traded_maps & (1 << DiploScratchF7C8)) &&
    (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << DiploScratchF7C8)) ||
     (PlayersData[faction_id_with].traded_maps & (1 << DiploScratchF7C8))) &&
    has_treaty(faction_id, DiploScratchF7C8, 0x8000000) &&
    DiploScratchFAA8 >= 0 && wants_to_speak(faction_id, faction_id_with)) {
    net_maps(faction_id_with, DiploScratchF7C8, 1);
    parse_says(2, get_title(DiploScratchF7C8), -1, -1);
    parse_says(3, get_name(DiploScratchF7C8), -1, -1);
    parse_says(4, get_adjective_insult_faction(DiploScratchF7C8), -1, -1);
    parse_says(5, get_adjective(DiploScratchF7C8), -1, -1);
    clear();
    say_tech(DiploScratchFAA8, 1);
    parse_says(6, StringTemp, -1, -1);
    net_set_treaty(faction_id, DiploScratchF7C8, 0x10000000, 1, 0);
    if (!X_pops(Players[DiploScratchF7C8].rule_flags & 0x80 ? "BOTHFRIENDALIEN" : "BOTHFRIEND",
                0x100000, FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
        goto communicate_menu_continuation;
    }
    net_treaty_on(faction_id, faction_id_with, 0x1000000, 0);
    net_maps(faction_id, faction_id_with, 1);
    net_tech(faction_id_with, DiploScratchFAA8, faction_id, 1);
    DiploScratchFA48 = 1;
    DiploScratchFAA8 = DiploScratchFA18;
    DiploScratchFA18 = -1;
    communicate_stack_20 = 1;
    DiploScratchF7D8 = 0;
    goto communicate_menu_continuation;
}
if (!has_treaty(faction_id, faction_id_with, 0x30) && !has_project(4, faction_id_with) &&
    PlayersData[faction_id_with].energy_reserves >=
        (GameDataStateStorage.fields_.turn_current_num_ / 25 + 1) * 10) {
    for (communicate_news_faction = 1; communicate_news_faction < 8; communicate_news_faction++) {
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << communicate_news_faction)) &&
            communicate_news_faction != faction_id &&
            communicate_news_faction != faction_id_with &&
            has_treaty(faction_id, communicate_news_faction, 8) &&
            !has_treaty(faction_id_with, communicate_news_faction, 8) &&
            !has_treaty(faction_id_with, communicate_news_faction, 0x10000000) &&
            !wants_to_attack(communicate_news_faction, faction_id_with, 0) &&
            !wants_to_attack(faction_id_with, communicate_news_faction, 0)) {
            goto communicate_news_found;
        }
    }
    goto communicate_menu_continuation;
communicate_news_found:
    if (wants_to_speak(faction_id, faction_id_with)) {
        parse_says(0, get_title(communicate_news_faction), -1, -1);
        parse_says(1, get_name(communicate_news_faction), -1, -1);
        parse_says(2, get_adjective(communicate_news_faction), -1, -1);
        communicate_stack_c = (GameDataStateStorage.fields_.turn_current_num_ / 25 + 1) * 10;
        parse_num(0, communicate_stack_c);
        if (Players[communicate_news_faction].rule_flags & 0x80) {
            if (!X_pops("ANYALIENNEWS", 0x100000,
                        FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
                goto communicate_menu_continuation;
            }
        } else if (!X_pops("ANYNEWS", 0x100000,
                           FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
            goto communicate_menu_continuation;
        }
        net_set_treaty(faction_id_with, communicate_news_faction, 0x10000000, 1, 0);
        net_treaty_on(faction_id_with, communicate_news_faction, 8, 0);
        net_energy(faction_id_with, -communicate_stack_c, faction_id, communicate_stack_c, 1);
        communicate_stack_14 = 1;
        cause_friction(faction_id_with, faction_id, -2);
    }
}

communicate_menu_continuation:
if (!has_treaty(faction_id_with, faction_id, 3) && !DiploScratchFA98 &&
    !has_treaty(faction_id, faction_id_with, 0x2000000) && DiploScratchFA88 > 0 &&
    DiploScratchFA60 && !DiploScratchFA70 && !IsMultiplayerNet &&
    !communicate_stack_20 &&
    PlayersData[faction_id_with].mil_strength_1 > PlayersData[faction_id].mil_strength_1) {
    communicate_ransom_base = -1;
    communicate_ransom_size = 999;
    for (communicate_territory_base_id = 0;
         communicate_territory_base_id < GameDataStateStorage.fields_.base_current_count_;
         communicate_territory_base_id++) {
        if (Bases[communicate_territory_base_id].faction_id_current == faction_id &&
            Bases[communicate_territory_base_id].faction_id_former == faction_id_with &&
            Bases[communicate_territory_base_id].population_size < communicate_ransom_size) {
            communicate_ransom_size = Bases[communicate_territory_base_id].population_size;
            communicate_ransom_base = communicate_territory_base_id;
        }
    }
    if (communicate_ransom_base >= 0 && wants_to_speak(faction_id, faction_id_with)) {
        communicate_territory_base_id = communicate_ransom_base;
        parse_says(0, get_title(faction_id), -1, -1);
        parse_says(1, get_name(faction_id), -1, -1);
        parse_says(2, Bases[communicate_territory_base_id].name_string, -1, -1);
        if (X_pops("RANSOMDEMAND", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
            communicate_stack_20 = 1;
            net_cede_base(communicate_territory_base_id, faction_id_with, 1);
            net_set_treaty(faction_id, faction_id_with, 0x2000000, 0, 0);
            net_treaty_off(faction_id_with, faction_id, 0x20, 1);
            diplomacy_caption(faction_id, faction_id_with);
        } else {
            communicate_stack_38 = 1;
        }
    }
}
}
}

communicate_post_ransom_call_gates:
communicate_tech_greeting_done:
communicate_shared_dialogue_outro:
communicate_protective_treaty_done:
communicate_after_proposal_menu:
if (!(GameDataStateStorage.fields_.game_rules_ & 0x2000000)) {
    tech_trade(faction_id, faction_id_with, 1, 4, 4);
}
if ((great_satan(faction_id, 0) ||
     (PlayersData[faction_id].ranking >= 6 && PlayersData[faction_id_with].ranking <= 4 &&
      !DiploScratchF7F0)) &&
    !DiploScratchFA60 && !DiploScratchFA70 &&
    !has_agenda(faction_id, faction_id_with, 0x2000) &&
    (!has_treaty(faction_id, faction_id_with, 1) ||
     (!(GameDataStateStorage.fields_.game_rules_ & 0x1000) && !DiploScratchF808))) {
    communicate_stack_1c = 1;
    if (!DiploScratchFA88 && PlayersData[faction_id].energy_reserves >= 125) {
        DiploScratchFA88 = communicate_round_down_multiple(
            PlayersData[faction_id].energy_reserves / 5, 25);
    }
}
if (mode) {
    if (!has_treaty(faction_id_with, faction_id, 0x10)) {
        DiploScratchFA88 = 0;
        DiploScratchFA60 = 0;
        goto communicate_post_contact_preparation;
    }
} else if (has_treaty(faction_id, faction_id_with, 1) && !communicate_stack_20 &&
           !has_agenda(faction_id, faction_id_with, 0x2000) &&
           !(GameDataStateStorage.fields_.game_rules_ & 0x1000) && !DiploScratchF808 &&
           ((DiploScratchFA64 && DiploScratchFA90) || DiploScratchFA78 > 4 ||
            DiploFriction > 8)) {
    communicate_stack_1c = 1;
}
if (DiploScratchFA88 > 0 &&
    PlayersData[faction_id].energy_reserves >= DiploScratchFA88 &&
    !communicate_stack_20 &&
    ((!DiploScratchFA50 && !DiploScratchFAA0) || DiploScratchFA18 < 0) &&
    !has_agenda(faction_id_with, faction_id, 0x2008) &&
    (DiploScratchFA60 || !DiploScratchFA80) &&
    (!has_treaty(faction_id, faction_id_with, 1) || DiploScratchFAA8 < 0 ||
     DiploScratchFA60) &&
    ((DiploScratchFA60 && !DiploScratchFA70) || communicate_stack_38 ||
     (communicate_stack_1c && PlayersData[faction_id].energy_reserves >= 125 &&
      DiploScratchFAA8 < 0)) &&
    wants_to_speak(faction_id, faction_id_with)) {
    if (!has_treaty(faction_id_with, faction_id, 0x11)) {
        mention_prototypes(faction_id, faction_id_with);
    }
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_title(faction_id_with), -1, -1);
    parse_says(3, get_name(faction_id_with), -1, -1);
    parse_says(4, get_adjective(faction_id_with), -1, -1);
    parse_says(5, get_pact(faction_id), -1, -1);
    parse_says(6, get_noun(faction_id_with), -1, -1);
    parse_num(0, DiploScratchFA88);
    communicate_bribe_half = DiploScratchFA88 / 20 * 10;
    if (!communicate_bribe_half) {
        communicate_bribe_half = 1;
    }
    parse_num(1, communicate_bribe_half);
    clear();
    says("DEMANDBRIBE");
    communicate_stack_14 = 1;
    communicate_stack_c = random(0, 4);
    if (communicate_stack_1c) {
        communicate_stack_c = 8;
    }
    if (has_treaty(faction_id_with, faction_id, 0x20) &&
        (DiploScratchFA98 ||
         PlayersData[faction_id_with].mil_strength_1 < PlayersData[faction_id].mil_strength_1)) {
        communicate_stack_c = 9;
    }
    if (great_satan(faction_id, 0) &&
        PlayersData[faction_id].mil_strength_1 >
            PlayersData[faction_id_with].mil_strength_1 * 4 / 3 &&
        DiploScratchFA68 == 2) {
        communicate_stack_c = 7;
    }
    if (has_treaty(faction_id_with, faction_id, 1)) {
        communicate_stack_c = 6;
    }
    if (has_treaty(faction_id_with, faction_id, 0x10)) {
        communicate_stack_c = mode ? 4 : 5;
    }
    if (has_treaty(faction_id_with, faction_id, 0x40)) {
        communicate_stack_c = 10;
    }
    if (!DiploScratchFA60 && !DiploScratchFA68) {
        communicate_stack_48 = 1;
    }
    communicate_stack_3c = 1;
    say_num(communicate_stack_c);
    switch (communicate_stack_c) {
    case 0:
        parse_says(5, get_adjective_leader(faction_id), -1, -1);
        parse_says(6, get_insult_leader(faction_id_with), -1, -1);
        parse_says(8, get_noun_phrase(faction_id_with, 11), -1, -1);
        parse_says(9, get_phrase(faction_id_with, 12), -1, -1);
        break;
    case 1:
        parse_says(8, get_noun_phrase(faction_id, 9), -1, -1);
        parse_says(9, get_phrase(faction_id_with, 13), -1, -1);
        break;
    case 2:
        parse_says(6, get_noun_phrase(faction_id_with, 10), -1, -1);
        if (has_treaty(faction_id_with, faction_id, 0x20)) {
            parse_says(8, get_noun_phrase(faction_id, 8), -1, -1);
        } else {
            parse_says(8, get_noun_phrase(faction_id, 9), -1, -1);
        }
        parse_says(9, get_phrase(faction_id_with, 1), -1, -1);
        break;
    case 3:
        parse_says(9, get_phrase(faction_id_with, 14), -1, -1);
        break;
    case 5:
        if (!DiploScratchFA98 &&
            PlayersData[faction_id_with].diplo_unk4[faction_id] >
                PlayersData[faction_id].diplo_unk4[faction_id_with]) {
            says("a");
        } else if (has_treaty(faction_id_with, faction_id, 0x20)) {
            clear();
            says("DEMANDBRIBE9");
        } else {
            says("b");
        }
        break;
    case 6:
        parse_says(9, get_noun_phrase(faction_id_with, 9), -1, -1);
        break;
    }
    communicate_bribe_result = X_pops(StringTemp, 0x100000,
                                      FactionDiplomacyPortraits[faction_id_with], pop_wait);
    if (communicate_bribe_result == 2) {
        if (communicate_stack_c == 6) {
            if (!random(0, 2) ||
                !(DiploScratchFA60 || (DiploScratchFA64 && DiploScratchFA90) ||
                  DiploScratchFA50 || DiploScratchFAA0)) {
                DiploScratchFA88 = communicate_bribe_half;
                goto communicate_bribe_pay;
            }
            X_pops("HALFBRIBE", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
communicate_bribe_refuse:
            net_treaty_off(faction_id_with, faction_id, 0x1000000, 0);
            communicate_stack_38 = 1;
            goto communicate_post_contact_preparation;
        }
        communicate_bribe_weasel = 0;
        if (great_satan(faction_id, 0) || (DiploScratchFA64 && DiploScratchFA90) ||
            DiploScratchFA50 || DiploScratchFAA0 ||
            has_treaty(faction_id_with, faction_id, 0x20)) {
            communicate_bribe_weasel =
                has_treaty(faction_id_with, faction_id, 0x400) ? 1 : random(0, 3);
            if (!random(0, 2)) {
                goto communicate_bribe_refuse;
            }
        }
        if (DiploScratchFA60 && (communicate_bribe_weasel || !random(0, 2))) {
            parse_says(6, get_insult_leader(faction_id_with), -1, -1);
            communicate_bribe_result = X_pops("WEASELOUT", 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait);
        } else {
            parse_num(2, communicate_bribe_half);
            X_pops("WEASELEDOUT", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
            DiploScratchFA88 = communicate_bribe_half;
            goto communicate_bribe_pay;
        }
    }
    if (!communicate_bribe_result) {
        goto communicate_bribe_refuse;
    }
communicate_bribe_pay:
    communicate_stack_20 = 1;
    if (communicate_stack_c == 10 && PlayersData[faction_id].diplo_unk1[faction_id_with]) {
        PlayersData[faction_id].diplo_unk1[faction_id_with]--;
    }
    net_energy(faction_id, -DiploScratchFA88, faction_id_with, DiploScratchFA88, 1);
    ConsoleInfoWin.set_redraw_phase(0);
    ConsoleGlobal.update_data(0);
    diplomacy_caption(faction_id, faction_id_with);
    net_treaty_on(faction_id_with, faction_id, 0x5000000, 0);
    if (communicate_stack_50) {
        net_treaty_on(faction_id_with, faction_id, 4, 1);
    }
}
communicate_post_contact_preparation:
if (communicate_stack_44 && !communicate_stack_20 &&
    (DiploScratchFAA8 >= 0 || communicate_stack_38) && !DiploScratchFA70 &&
    !DiploScratchFA98 &&
    (DiploScratchFA68 ||
     ((DiploScratchF7FC || PlayersData[faction_id_with].ranking >= 6) &&
      PlayersData[faction_id].ranking >= 6 && !DiploScratchF7F0)) &&
    DiploFriction + PlayersData[faction_id_with].ai_fight * 2 >
        (5 - ((GameDataStateStorage.fields_.game_rules_ & 0x40) ? 5
              : PlayersData[faction_id].diff_level)) / 2 + 10) {
    DiploScratchFA60 = 1;
}
if (DiploScratchFA60 && !communicate_stack_20 && !communicate_stack_38 &&
    !DiploScratchFA98 &&
    ((!DiploScratchFA50 && !DiploScratchFAA0) || DiploScratchFA18 < 0) &&
    DiploScratchFA88 > PlayersData[faction_id].energy_reserves &&
    (great_beelzebub(faction_id, 0) ||
     (PlayersData[faction_id].ranking >= 6 &&
      (DiploScratchF7FC || PlayersData[faction_id_with].ranking >= 6)))) {
    communicate_bribe_multiplier = (GameDataStateStorage.fields_.game_rules_ & 0x40) ? 2 : 3;
    if (!great_satan(faction_id, 0)) {
        communicate_bribe_multiplier++;
    }
    if (DiploScratchFA88 > tech_val(DiploScratchFAA8, faction_id_with, 1) *
            ((GameDataStateStorage.fields_.turn_current_num_ + 7) / 8) *
            communicate_bribe_multiplier / 2) {
        DiploScratchFAA8 = -1;
        goto communicate_relation_later_action_dispatch;
    }
}

communicate_post_relation_menu:
if (DiploScratchFAA8 >= 0 && DiploScratchFAA8 != 9999 && !communicate_stack_20 &&
    !communicate_stack_38 && !(GameDataStateStorage.fields_.game_rules_ & 0x2000000) &&
    (!DiploScratchFAA0 || DiploScratchFA18 >= 0) &&
    (DiploScratchFAA0 < 2 ||
     PlayersData[faction_id_with].tech_ranking / 2 < PlayersData[faction_id].tech_ranking / 2) &&
    !has_agenda(faction_id_with, faction_id, 0x2008) &&
    (!has_treaty(faction_id_with, faction_id, 0x40) || DiploScratchFA18 >= 0 ||
     PlayersData[faction_id_with].ranking > PlayersData[faction_id].ranking) &&
    (DiploScratchFA60 || !DiploScratchFA80) &&
    ((!DiploScratchFA98 && !has_treaty(faction_id_with, faction_id, 1)) ||
     communicate_stack_1c) &&
    (PlayersData[faction_id_with].tech_commerce_bonus <=
         PlayersData[faction_id].tech_commerce_bonus ||
     GameDataStateStorage.fields_.faction_rankings_[7] == (uint32_t)faction_id ||
     communicate_stack_1c || has_treaty(faction_id_with, faction_id, 0x20) ||
     (!has_treaty(faction_id_with, faction_id, 2) && !communicate_stack_44)) &&
    wants_to_speak(faction_id, faction_id_with)) {
    if (!has_treaty(faction_id_with, faction_id, 0x11)) {
        mention_prototypes(faction_id, faction_id_with);
    }
    for (;;) {
        if (DiploScratchFA18 >= 0 && !DiploScratchFA50 && !DiploScratchFAA0 &&
            DiploScratchF7C4 && !has_treaty(faction_id_with, faction_id, 0x40)) {
            swap(&DiploScratchFAA8, &DiploScratchFA18);
        }
        clear();
        say_tech(DiploScratchFAA8, 1);
        parse_says(0, StringTemp, -1, -1);
        parse_says(1, get_title(faction_id), -1, -1);
        parse_says(2, get_name(faction_id), -1, -1);
        parse_says(3, get_title(faction_id_with), -1, -1);
        parse_says(4, get_name(faction_id_with), -1, -1);
        parse_says(5, get_pact(faction_id_with), -1, -1);
        if (DiploScratchFA18 >= 0) {
            clear();
            say_tech(DiploScratchFA18, 1);
            parse_says(6, StringTemp, -1, -1);
        }
        clear();
        says("DEMANDTECH");
        communicate_stack_c = random(0, 3);
        if (!DiploScratchFA60) {
            if (!DiploScratchFA68) {
                communicate_stack_48 = 1;
            }
            communicate_stack_c = random(0, 3) + 9;
            if (has_treaty(faction_id_with, faction_id, 0x20) && !communicate_stack_1c) {
                communicate_stack_c = 8;
            }
        }
        if (great_satan(faction_id, 0) || communicate_stack_1c) {
            communicate_stack_c = 7;
        }
        if (has_treaty(faction_id_with, faction_id, 0x10)) {
            communicate_stack_c = (PlayersData[faction_id_with].diplo_unk4[faction_id] <=
                                   PlayersData[faction_id].diplo_unk4[faction_id_with]) + 3;
        }
        if (has_treaty(faction_id_with, faction_id, 1)) {
            communicate_stack_c = 5;
        }
        if (has_treaty(faction_id_with, faction_id, 0x40) &&
            !has_treaty(faction_id_with, faction_id, 0x20)) {
            if (!PlayersData[faction_id].diplo_unk1[faction_id_with] ||
                PlayersData[faction_id].diplo_unk1[faction_id_with] <=
                    PlayersData[faction_id].diplo_wrongs[faction_id_with]) {
                if (DiploScratchFA1C >= 0) {
                    clear();
                    say_tech(DiploScratchFA1C, 1);
                    parse_says(8, StringTemp, -1, -1);
                    communicate_stack_c = 14;
                }
                if (DiploScratchFA28 >= 0) {
                    clear();
                    say_tech(DiploScratchFA28, 1);
                    parse_says(9, StringTemp, -1, -1);
                    communicate_stack_c = 15;
                }
            }
            if (communicate_stack_c < 14) {
                communicate_stack_c = (DiploScratchFA18 < 0) + 12;
            }
        }
        say_num(communicate_stack_c);
        switch (communicate_stack_c) {
        case 0:
            parse_says(9, get_noun_phrase(faction_id, 8), -1, -1);
            break;
        case 1:
            parse_says(9, get_phrase(faction_id, 7), -1, -1);
            break;
        case 2:
            parse_says(9, get_phrase(faction_id, 6), -1, -1);
            break;
        case 9:
        case 10:
            parse_says(9, get_noun_phrase(faction_id_with, 9), -1, -1);
            break;
        }
        if (communicate_stack_c >= 9 && communicate_stack_c < 12) {
            communicate_tech_price = communicate_round_down_multiple(
                tech_val(DiploScratchFAA8, faction_id, 1) *
                        ((GameDataStateStorage.fields_.turn_current_num_ + 7) / 8) /
                    (bit_count(GameDataStateStorage.fields_.game_tech_achieved_[DiploScratchFAA8]) + 1) +
                    24,
                25);
            parse_num(0, communicate_tech_price);
            popup.start(StringTemp, 0x100000);
            popup.set_picture(FactionDiplomacyPortraits[faction_id_with], 0, 0);
            says("A");
            if (X_text_open(ScriptPopupFile, StringTemp)) {
                communicate_tech_result = 0;
                break;
            }
            if (DiploScratchF800 >= 0) {
                clear();
                say_tech(DiploScratchF800, 1);
                parse_says(6, StringTemp, -1, -1);
            }
            text_get();
            clear();
            parse_string(TextBufferGetPtr, StringTemp);
            popup.item(StringTemp, 0);
            text_get();
            clear();
            parse_string(TextBufferGetPtr, StringTemp);
            popup.item(StringTemp, 1);
            text_get();
            clear();
            parse_string(TextBufferGetPtr, StringTemp);
            if (communicate_tech_price > 0) {
                popup.item(StringTemp, 2);
            }
            text_get();
            clear();
            parse_string(TextBufferGetPtr, StringTemp);
            if (DiploScratchF800 >= 0) {
                popup.item(StringTemp, 3);
            }
            text_get();
            clear();
            parse_string(TextBufferGetPtr, StringTemp);
            popup.item(StringTemp, 4);
            communicate_tech_result = popup.exec(pop_wait);
        } else {
            communicate_tech_result = X_pops(StringTemp, 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait);
            if (communicate_tech_result == 2) {
                help_topic(14, DiploScratchFAA8);
                continue;
            }
        }
        if (communicate_tech_result != 4) {
            break;
        }
        help_topic(14, DiploScratchFAA8);
    }
    clear();
    say_tech(DiploScratchFAA8, 1);
    parse_says(0, StringTemp, -1, -1);
    parse_says(1, get_title(faction_id), -1, -1);
    parse_says(2, get_name(faction_id), -1, -1);
    parse_says(3, get_title(faction_id_with), -1, -1);
    parse_says(4, get_name(faction_id_with), -1, -1);
    if (communicate_tech_result == 2) {
        if (PlayersData[faction_id_with].energy_reserves / 2 >= communicate_tech_price &&
            tech_val(DiploScratchFAA8, faction_id_with, 1) *
                    ((GameDataStateStorage.fields_.turn_current_num_ + 7) / 8) >
                communicate_tech_price) {
            X_pops("AGREEDENERGY", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
            net_energy(faction_id_with, -communicate_tech_price, faction_id,
                       communicate_tech_price, 0);
            goto communicate_tech_give;
        }
        if (X_pops("DEMANDTECHAGAIN1", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
            goto communicate_tech_give;
        }
        communicate_tech_result = -1;
        goto communicate_large_offer_zero_limit;
    } else if (communicate_tech_result == 3) {
        if (!DiploScratchFA68 &&
            tech_val(DiploScratchF800, faction_id_with, 1) * 2 / 3 <=
                tech_val(DiploScratchFAA8, faction_id_with, 1)) {
            X_pops("AGREEDTECH", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
            net_tech(faction_id, DiploScratchF800, faction_id_with, 0);
            DiploScratchF800 = -1;
            goto communicate_tech_give;
        }
        if (X_pops("DEMANDTECHAGAIN2", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
            goto communicate_tech_give;
        }
        communicate_tech_result = -1;
    } else if (communicate_tech_result > 0) {
communicate_tech_give:
        net_tech(faction_id_with, DiploScratchFAA8, faction_id, 1);
        if (communicate_stack_c >= 15) {
            net_tech(faction_id_with, DiploScratchFA28, faction_id, 1);
        }
        if (communicate_stack_c >= 14) {
            net_tech(faction_id_with, DiploScratchFA1C, faction_id, 1);
            communicate_stack_c = 6;
        } else if (communicate_stack_c == 12) {
            communicate_stack_c = 6;
        } else if ((DiploScratchFA50 || DiploScratchFAA0) && DiploScratchFA60 &&
                   DiploScratchFA18 >= 0) {
            for (;;) {
                clear();
                say_tech(DiploScratchFAA8, 1);
                parse_says(0, StringTemp, -1, -1);
                parse_says(1, get_title(faction_id), -1, -1);
                parse_says(2, get_name(faction_id), -1, -1);
                parse_says(3, get_title(faction_id_with), -1, -1);
                parse_says(4, get_name(faction_id_with), -1, -1);
                communicate_stack_c = 6;
                clear();
                says("DEMANDTECH");
                say_num(6);
                communicate_tech_result = X_pops(StringTemp, 0x100000,
                    FactionDiplomacyPortraits[faction_id_with], pop_wait);
                if (communicate_tech_result <= 1) {
                    break;
                }
                help_topic(14, DiploScratchFA18);
            }
        }
        if (communicate_tech_result) {
            DiploScratchFAA8 = DiploScratchFA18;
            DiploScratchFA18 = -1;
            if (communicate_stack_c == 6) {
                net_tech(faction_id_with, DiploScratchFAA8, faction_id, 1);
                DiploScratchFAA8 = -1;
            }
            net_treaty_on(faction_id_with, faction_id, 0x5000000, 0);
            if (!has_treaty(faction_id_with, faction_id, 0x20)) {
                DiploScratchFA70 = 0;
            }
            communicate_stack_20 = 1;
            if (communicate_stack_50) {
                net_treaty_on(faction_id_with, faction_id, 4, 1);
            }
            if (communicate_tech_result == 1) {
                if (PlayersData[faction_id_with].diplo_patience[faction_id]) {
                    PlayersData[faction_id_with].diplo_patience[faction_id]--;
                }
                if (communicate_stack_c >= 9 && communicate_stack_c < 12) {
                    cause_friction(faction_id_with, faction_id,
                                   -(tech_val(DiploScratchFAA8, faction_id_with, 1) / 2));
                    if (!PlayersData[faction_id].diplo_unk1[faction_id_with] ||
                        PlayersData[faction_id].diplo_unk1[faction_id_with] <
                            PlayersData[faction_id].integrity_blemishes) {
                        PlayersData[faction_id].diplo_unk1[faction_id_with]++;
                    }
                }
                if (communicate_stack_c >= 12 &&
                    PlayersData[faction_id].diplo_unk1[faction_id_with]) {
                    PlayersData[faction_id].diplo_unk1[faction_id_with]--;
                }
            }
        } else {
            communicate_stack_38 = 1;
            net_treaty_off(faction_id_with, faction_id, 0x1000000, 0);
        }
        goto communicate_tech_caption;
    }
communicate_large_offer_zero_limit:
    net_treaty_off(faction_id_with, faction_id, 0x1000000, 0);
    communicate_stack_38 = 1;
    if (has_treaty(faction_id_with, faction_id, 1) && !communicate_tech_result) {
        cause_friction(faction_id_with, faction_id, (DiploScratchFA78 * 2 + 3) / 5);
        PlayersData[faction_id_with].diplo_patience[faction_id]++;
    }
communicate_tech_caption:
    diplomacy_caption(faction_id, faction_id_with);
}
communicate_relation_later_action_dispatch:
if (has_treaty(faction_id, faction_id_with, 1)) {
    DiploScratchF808++;
}
if (!strcmp(Players[faction_id_with].filename, "MORGAN")) {
    DiploScratchF808++;
}
if (PlayersData[faction_id].energy_reserves > PlayersData[faction_id_with].energy_reserves &&
    PlayersData[faction_id].energy_reserves >= 200 && DiploScratchFA78 < 4 &&
    (PlayersData[faction_id].energy_reserves >= PlayersData[faction_id_with].energy_reserves * 2 ||
     (has_treaty(faction_id, faction_id_with, 1) &&
      PlayersData[faction_id].ranking > PlayersData[faction_id_with].ranking)) &&
    (!DiploScratchFA60 || communicate_stack_20) && !communicate_stack_14 &&
    (communicate_stack_20 || !communicate_stack_38 || communicate_stack_48 ||
     has_treaty(faction_id, faction_id_with, 1)) &&
    !has_treaty(faction_id, faction_id_with, 0x10) &&
    has_treaty(faction_id, faction_id_with, 0x10003) &&
    !has_treaty(faction_id_with, faction_id, 0x20) &&
    !PlayersData[faction_id].loan_balance[faction_id_with] &&
    !PlayersData[faction_id_with].loan_balance[faction_id] &&
    (PlayersData[faction_id_with].energy_reserves < (DiploScratchF808 * 5 + 5) * 20 ||
     (DiploScratchF7E8 > 0 &&
      PlayersData[faction_id_with].energy_reserves <
          PlayersData[DiploScratchF7E8].energy_reserves * 3 / 4)) &&
    wants_to_speak(faction_id, faction_id_with)) {
    communicate_loan_amount = PlayersData[faction_id].energy_reserves / 2;
    communicate_loan_limit = (PlayersData[faction_id_with].unk_13 -
                              PlayersData[faction_id_with].maint_cost_total) * 100;
    if (communicate_loan_amount >= communicate_loan_limit) {
        communicate_loan_amount = communicate_loan_limit;
    }
    if (communicate_loan_amount >= 50) {
        communicate_loan_rate = (PlayersData[faction_id_with].integrity_blemishes +
            (PlayersData[faction_id].tech_commerce_bonus -
             PlayersData[faction_id_with].tech_commerce_bonus) * 2 + 10) * 5;
        if (has_treaty(faction_id, faction_id_with, 1)) {
            communicate_loan_rate -= 10;
        }
        if (communicate_loan_rate < 10) {
            communicate_loan_rate = 10;
        }
        communicate_loan_term = (10 - PlayersData[faction_id_with].integrity_blemishes) * 10;
        if (communicate_loan_term < 20) {
            communicate_loan_term = 20;
        }
        communicate_loan_factor = communicate_loan_rate + 100;
        communicate_loan_total = communicate_loan_factor * communicate_loan_amount / 100;
        if (communicate_loan_total < communicate_loan_term) {
            communicate_loan_term = communicate_loan_total;
        }
        communicate_loan_payment = (communicate_loan_total + communicate_loan_term - 1) /
            (communicate_loan_term < 1 ? 1 : communicate_loan_term);
        communicate_loan_balance = communicate_loan_term * communicate_loan_payment;
        parse_says(0, get_title(faction_id), -1, -1);
        parse_says(1, get_name(faction_id), -1, -1);
        parse_says(2, get_title(faction_id_with), -1, -1);
        parse_says(3, get_name(faction_id_with), -1, -1);
        parse_num(0, communicate_loan_amount);
        parse_num(1, communicate_loan_payment);
        parse_num(2, communicate_loan_term);
        clear();
        says("ASKFORLOAN");
        say_num((communicate_loan_payment != 1) + 1);
        communicate_loan_result = X_pops(StringTemp, 0x100000,
            FactionDiplomacyPortraits[faction_id_with], pop_wait);
        if (communicate_loan_result) {
            if (communicate_loan_result == 2) {
                communicate_loan_amount /= 2;
                communicate_loan_total = communicate_loan_factor * communicate_loan_amount / 100;
                if (communicate_loan_total < communicate_loan_term) {
                    communicate_loan_term = communicate_loan_total;
                }
                communicate_loan_payment = (communicate_loan_total + communicate_loan_term - 1) /
                    (communicate_loan_term < 1 ? 1 : communicate_loan_term);
                communicate_loan_balance = communicate_loan_term * communicate_loan_payment;
            }
            net_treaty_on(faction_id_with, faction_id, 0x5000000, 0);
            net_energy(faction_id, -communicate_loan_amount, faction_id_with,
                       communicate_loan_amount, 1);
            net_loan(faction_id_with, faction_id, communicate_loan_balance,
                     communicate_loan_payment);
            cause_friction(faction_id_with, faction_id,
                           -energy_value(communicate_loan_amount));
            communicate_stack_20 = 1;
        } else if (has_treaty(faction_id, faction_id_with, 1)) {
            cause_friction(faction_id_with, faction_id, 1);
        }
    }
}

communicate_large_offer_rejected:
if (!mode && !communicate_stack_20 &&
    (DiploScratchF7FC || !DiploScratchFA98 || has_treaty(faction_id_with, faction_id, 1) ||
     has_agenda(faction_id_with, faction_id, 0x400)) &&
    has_agenda(faction_id_with, faction_id, 1) &&
    !has_agenda(faction_id, faction_id_with, 0x2000)) {
    // stand-in: the image indexes the priority/opposition pairs as arrays.
    for (communicate_stack_c = 0; communicate_stack_c < 1; communicate_stack_c++) {
        if ((&Players[faction_id_with].soc_ideology_category)[communicate_stack_c] >= 0 &&
            (&Players[faction_id_with].soc_ideology_model)[communicate_stack_c] &&
            PlayersData[faction_id].soc_category_pending.models_[
                (&Players[faction_id_with].soc_ideology_category)[communicate_stack_c]] &&
            PlayersData[faction_id].soc_category_pending.models_[
                (&Players[faction_id_with].soc_ideology_category)[communicate_stack_c]] !=
                (&Players[faction_id_with].soc_ideology_model)[communicate_stack_c]) {
            goto communicate_social_break_pact;
        }
    }
}
communicate_stack_c = 0;
if (PlayersData[faction_id].diplo_unk1[faction_id_with] &&
    PlayersData[faction_id].diplo_unk1[faction_id_with] >
        PlayersData[faction_id].diplo_wrongs[faction_id_with] &&
    !has_treaty(faction_id_with, faction_id, 0x50) &&
    !has_agenda(faction_id_with, faction_id, 0x2008) &&
    !has_treaty(faction_id_with, faction_id, 0x40020)) {
    communicate_stack_c = 1;
}
if (has_treaty(faction_id_with, faction_id, 1)) {
    if (communicate_stack_20) {
        goto communicate_late_contact_stack_done;
    }
    if (mode || has_agenda(faction_id_with, faction_id, 0x2000)) {
        goto communicate_late_contact_rank_route_alt;
    }
    if (!DiploScratchFA60 &&
        (PlayersData[faction_id].ranking < 6 || PlayersData[faction_id_with].ranking < 6) &&
        (DiploFriction < (PlayersData[faction_id_with].tech_commerce_bonus -
                          PlayersData[faction_id].tech_commerce_bonus) * 2 + 10 ||
         (PlayersData[faction_id_with].ranking < 5 &&
          (GameDataStateStorage.fields_.game_rules_ & 0x1000) &&
          GameDataStateStorage.fields_.turn_current_num_ >= 200)) &&
        (!DiploScratchFA64 || !DiploScratchFA90 ||
         (GameDataStateStorage.fields_.game_rules_ & 0x1000))) {
        goto communicate_late_contact_rank_route_alt;
    }
    goto communicate_late_contact_rank_fallback;
communicate_social_break_pact:
    wants_to_speak(faction_id, faction_id_with);
    if (!DiplomacyEndState) {
        diplomacy_caption(faction_id, faction_id_with);
    }
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_pact_hood(faction_id, faction_id_with), -1, -1);
    parse_says(3, get_noun(faction_id_with), -1, -1);
    clear();
    says("SOCIAL");
    say_num((&Players[faction_id_with].soc_ideology_category)[communicate_stack_c]);
    says("CAT");
    say_num(PlayersData[faction_id].soc_category_pending.models_[
        (&Players[faction_id_with].soc_ideology_category)[communicate_stack_c]]);
    says(has_treaty(faction_id, faction_id_with, 1) ? "PACT" : "WAR");
    say_num((&Players[faction_id_with].soc_ideology_model)[communicate_stack_c]);
    X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[faction_id_with], pop_wait);
    if (has_treaty(faction_id, faction_id_with, 1)) {
        diplo_lock(0);
        net_pact_ends(faction_id_with, faction_id, 1);
        diplo_unlock();
        PlayersData[faction_id].diplo_spoke[faction_id_with] =
            GameDataStateStorage.fields_.turn_current_num_;
        goto communicate_final_cleanup;
    }
    net_set_treaty(faction_id_with, faction_id, 0x40, 1, 1);
    net_treaty_on(faction_id_with, faction_id, 0x10, 1);
    PlayersData[faction_id].diplo_spoke[faction_id_with] =
        GameDataStateStorage.fields_.turn_current_num_;
    goto communicate_final_cleanup;
communicate_late_contact_rank_fallback:
    if (communicate_stack_c) {
        DiploScratchFA60 = 0;
        PlayersData[faction_id].diplo_unk1[faction_id_with]--;
        goto communicate_late_contact_rank_route_alt;
    }
    wants_to_speak(faction_id, faction_id_with);
    if (!DiplomacyEndState) {
        diplomacy_caption(faction_id, faction_id_with);
    }
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_pacts(faction_id, faction_id_with), -1, -1);
    parse_says(3, get_noun_phrase(faction_id, 8), -1, -1);
    X_pops("BREAKPACT", 0x100000, FactionDiplomacyPortraits[faction_id_with], pop_wait);
    diplo_lock(0);
    net_pact_ends(faction_id_with, faction_id, 1);
    diplo_unlock();
    PlayersData[faction_id].diplo_spoke[faction_id_with] =
        GameDataStateStorage.fields_.turn_current_num_;
    goto communicate_final_cleanup;
} else if (communicate_stack_20) {
    goto communicate_late_contact_stack_done;
}
communicate_late_contact_rank_route_alt:
if (has_treaty(faction_id, faction_id_with, 1)) {
communicate_late_contact_stack_done:
    DiploScratchFA60 = 0;
} else if (DiploScratchFA60) {
    if (communicate_stack_c) {
        DiploScratchFA60 = 0;
        PlayersData[faction_id].diplo_unk1[faction_id_with]--;
    } else if (PlayersData[faction_id].ranking > PlayersData[faction_id_with].ranking &&
               DiploScratchFA2C > PlayersData[faction_id].integrity_blemishes &&
               !DiploScratchFAA0 && !DiploScratchFA50 &&
               has_treaty(faction_id_with, faction_id, 0x10) &&
               !has_agenda(faction_id_with, faction_id, 8) &&
               has_treaty(faction_id_with, faction_id, 0x2000)) {
        goto communicate_late_contact_special_war;
    }
}
if (DiploScratchFA4C && !has_treaty(faction_id, faction_id_with, 0x10) &&
    ((DiploScratchFA60 && DiploScratchF7E8) ||
     ((Players[faction_id_with].rule_flags & 0x80) &&
      (Players[DiploScratchF7B8].rule_flags & 0x80)))) {
    wants_to_speak(faction_id, faction_id_with);
    net_set_treaty(faction_id_with, faction_id, 0x40, 1, 1);
    net_treaty_on(faction_id_with, faction_id, 0x10, 1);
    PlayersData[faction_id].diplo_spoke[faction_id_with] =
        GameDataStateStorage.fields_.turn_current_num_;
    diplomacy_caption(faction_id, faction_id_with);
    clear();
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_title(DiploScratchF7B8), -1, -1);
    parse_says(3, get_name(DiploScratchF7B8), -1, -1);
    X_pops(((Players[faction_id_with].rule_flags & 0x80) &&
            (Players[DiploScratchF7B8].rule_flags & 0x80)) ? "ALIENHATRED" : "COLLABORATOR",
           0x100000, FactionDiplomacyPortraits[faction_id_with], pop_wait);
    goto communicate_final_cleanup;
}

communicate_late_contact_hate_popup_exit: // native 0x0055586D
if (DiploScratchFA60 == 0 || communicate_stack_38 == 0 ||
    has_treaty(faction_id, faction_id_with, 0x00000006) == 0) {
    goto communicate_introduction_conversation; // native 0x00550522
}
if (has_treaty(faction_id_with, faction_id, 0x00000010) != 0) {
    goto communicate_introduction_conversation; // native 0x00550522
}
if (DiploScratchF7E4 <= 0) {
    goto communicate_introduction_conversation; // native 0x00550529
}
if (has_treaty(faction_id_with, faction_id, 0x00002000) != 0) {
    goto communicate_introduction_conversation; // native 0x00550522
}
communicate_late_contact_bully: // native 0x005558CD
(void)wants_to_speak(faction_id, faction_id_with);
net_treaty_on(faction_id_with, faction_id, 0x00000010, 1);
diplomacy_caption(faction_id, faction_id_with);
PlayersData[faction_id].diplo_spoke[faction_id_with] = GameDataStateStorage.fields_.turn_current_num_;
clear();
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_adjective_leader(faction_id), -1, -1);
parse_says(3, get_adjective_insult_faction(faction_id), -1, -1);
parse_says(4, get_phrase(faction_id_with, 13), -1, -1);
parse_says(5, get_noun_phrase(faction_id, 10), -1, -1);
says("BULLY");
communicate_late_contact_bully_random = random(0, 7);
say_num(communicate_late_contact_bully_random);
X_pops(StringTemp, 0x00100000,
             FactionDiplomacyPortraits[faction_id_with], pop_wait);
goto communicate_final_cleanup; // native 0x005587C8
communicate_late_contact_vendetta_after_set_mode: // native 0x005559D0
if (DiploScratchFA60) {
if (communicate_stack_38 == 0 && communicate_local_menu_mode == 0) {
    goto communicate_final_cleanup; // native 0x005587C8
}
if (DiploScratchFA88 >= 0) {
    (void)wants_to_speak(faction_id, faction_id_with);
}
// The native query result is overwritten by the unconditional network update;
// preserve both calls and the latter's return value for the later selector.
communicate_late_contact_vendetta_response =
    has_treaty(faction_id_with, faction_id, 0x00000010);
net_treaty_on(faction_id_with, faction_id, 0x00000010, 1);
PlayersData[faction_id].diplo_spoke[faction_id_with] = GameDataStateStorage.fields_.turn_current_num_;
diplomacy_caption(faction_id, faction_id_with);
clear();
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_adjective_insult_leader(faction_id), -1, -1);
parse_says(3, get_adjective_leader(faction_id), -1, -1);
parse_says(4, get_noun_phrase(faction_id, 8), -1, -1);
parse_says(5, get_phrase(faction_id, 3), -1, -1);
parse_says(6, get_phrase(faction_id_with, 13), -1, -1);
parse_says(8, get_phrase(faction_id, 4), -1, -1);
parse_says(9, get_insult_leader(faction_id), -1, -1);
says("VENDETTA");
communicate_vendetta_text_variant =
    (communicate_stack_38 != 0 &&
     communicate_late_contact_vendetta_response == 0)
        ? random(0, 3) : random(0, 6) + 3;
if (great_satan(faction_id, 0) != 0 && random(0, 3) != 0) {
    communicate_vendetta_text_variant = random(0, 4) + 15;
}
if (communicate_local_menu_mode != 0 && DiploScratchFA88 >= 0) {
    communicate_vendetta_text_variant = 14;
}
if (has_treaty(faction_id_with, faction_id, 0x00040000) != 0 &&
    has_treaty(faction_id_with, faction_id, 0x00000020) != 0) {
    communicate_vendetta_text_variant = random(0, 5) + 9;
}
say_num(communicate_vendetta_text_variant);
if (communicate_vendetta_text_variant == 5 &&
    strcmp(Players[faction_id].filename, "CYBORG") == 0) {
    says("X");
}
if (has_agenda(faction_id_with, faction_id, 4) != 0) {
    clear();
    says("ATROCIOUS");
}
X_pops(StringTemp, 0x00100000,
             FactionDiplomacyPortraits[faction_id_with], pop_wait);
goto communicate_final_cleanup; // native 0x005587C8
} else {
communicate_late_contact_special_war: // native 0x00555C42
communicate_special_war_response:
if (communicate_stack_50 != 0 && communicate_stack_20 != 0) {
    (void)net_treaty_on(faction_id_with, faction_id, 0x00000004, 1);
}
if (has_treaty(faction_id_with, faction_id, 0x00000002) == 0) {
    if (DiploScratchFA8C == 0 ||
        has_treaty(faction_id_with, faction_id, 0x00000010) != 0) {
        goto communicate_special_war_rival_gates; // native 0x00556113
    }
}
if (has_treaty(faction_id_with, faction_id, 0x00000021) != 0 ||
    has_agenda(faction_id_with, faction_id, 0x00002000) != 0 ||
    DiploScratchFA3C != 0) {
    goto communicate_special_war_rival_gates; // native 0x00556113
}
communicate_special_war_rival_scan: // native 0x00555CC7
communicate_stack_1c = 0;
communicate_stack_14 = 0;
communicate_rival_faction = 1;
do {
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1u << (communicate_rival_faction))) &&
        has_treaty(faction_id_with, communicate_rival_faction,
                   0x00000010) != 0) {
        if (has_treaty(faction_id, communicate_rival_faction,
                       0x00000014) != 0) {
            communicate_stack_14 = 1;
        }
        if (has_treaty(faction_id, communicate_rival_faction,
                       0x00000001) != 0) {
            goto communicate_special_war_rival_gates; // native 0x00555D29
        }
        for (communicate_special_war_region = 1;
             communicate_special_war_region < 0x80;
             ++communicate_special_war_region) {
            if (bad_reg(communicate_special_war_region) == 0 &&
                PlayersData[faction_id_with].region_total_bases[
                    communicate_special_war_region] != 0 &&
                PlayersData[communicate_rival_faction].region_total_bases[
                    communicate_special_war_region] != 0) {
                ++communicate_stack_1c;
                break;
            }
        }
    }
    ++communicate_rival_faction;
} while (communicate_rival_faction < 8);
if (communicate_stack_14 < 0 ||
    great_satan(faction_id, 0) != 0 ||
    has_agenda(faction_id_with, faction_id, 0x00000400) != 0 ||
    communicate_stack_3c != 0) {
    goto communicate_special_war_rival_gates; // native 0x00555D95
}
// Native 0x555DD0..0x556113: the original pact offer fallthrough.
// The rival scan retains one hit per faction and the original zero/one flags.
communicate_pact_offer_fallthrough:
{
    if (DiploScratchFA78 >=
        range(PlayersData[faction_id_with].tech_commerce_bonus -
                  PlayersData[faction_id].tech_commerce_bonus, 0, 99) +
            (strcmp(Players[faction_id_with].filename, "HIVE") == 0) +
            ((GameDataStateStorage.fields_.game_rules_ >> 12) & 1) +
            (DiploScratchFA8C != 0) + 3) {
        goto communicate_special_war_rival_gates;
    }

    communicate_stack_3c = 3;
    if (PlayersData[faction_id].global_reputation <= 3) {
        communicate_stack_3c = PlayersData[faction_id].global_reputation;
    }
    const int shared_victory =
        (GameDataStateStorage.fields_.game_rules_ & 0x1000) != 0 && (GameDataStateStorage.fields_.game_rules_ & 0x40) == 0;
    if (DiploScratchFA78 - communicate_stack_3c + DiploFriction >=
        (communicate_stack_14 ? 11 : 6) + shared_victory +
            range(PlayersData[faction_id_with].tech_commerce_bonus -
                      PlayersData[faction_id].tech_commerce_bonus + 1, 0, 99) +
            (strcmp(Players[faction_id_with].filename, "HIVE") == 0) + (DiploScratchFA8C != 0)) {
        goto communicate_special_war_rival_gates;
    }
    if (communicate_stack_1c != 0 && DiploScratchFA54 == 0 &&
        DiploScratchF7FC == 0) {
        if (DiploScratchFA98 == 0 && communicate_stack_14 == 0 &&
            PlayersData[faction_id].ranking > 5 &&
            PlayersData[faction_id_with].ranking > 5) {
            goto communicate_special_war_rival_gates;
        }
    } else if (PlayersData[faction_id].ranking > 5 ||
               PlayersData[faction_id_with].ranking > 5) {
        goto communicate_special_war_rival_gates;
    }
    if (wants_to_speak(faction_id, faction_id_with) == 0) {
        goto communicate_special_war_rival_gates;
    }
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_pact_hood(faction_id, faction_id_with), -1, -1);
    if (X_pops("SWEARAPACT", 0x100000,
                     FactionDiplomacyPortraits[faction_id_with], pop_wait) != 0) {
        goto communicate_special_war_rival_gates;
    }
    if (DiploScratchF7E8 != 0) {
        parse_says(5, get_title(faction_id), -1, -1);
        parse_says(6, get_noun(DiploScratchF7E8), -1, -1);
        if (X_pops("GIMMEHELP", 0x100000,
                         FactionDiplomacyPortraits[faction_id_with], pop_wait) != 0) {
            diplo_lock(0);
            net_double_cross(faction_id, DiploScratchF7E8, faction_id_with, 0);
            net_treaty_off(faction_id, faction_id_with, 0x800000, 0);
            net_set_treaty(faction_id, DiploScratchF7E8, 0x40, 1, 1);
            diplo_unlock();
            DiploScratchF7E8 = 0;
            DiploScratchFA4C = 0;
        } else {
            if (DiploScratchFA98 == 0) {
                parse_says(2, get_pacts(faction_id, faction_id_with), -1, -1);
                X_pops("REJPACTNOHELP", 0x100000,
                             FactionDiplomacyPortraits[faction_id_with], pop_wait);
                communicate_stack_34 = 1;
                goto communicate_special_war_rival_gates;
            }
            if (communicate_stack_34 == 0) {
                communicate_stack_34 = 1;
                goto communicate_special_war_rival_gates;
            }
        }
    }
    pact_of_brotherhood(faction_id, faction_id_with);
    diplomacy_caption(faction_id, faction_id_with);
    communicate_stack_34 = 1;
}

communicate_special_war_rival_gates: // native 0x00556113
if (has_treaty(faction_id_with, faction_id, 0x00000010) == 0 ||
    DiploScratchFA98 == 0 || DiploScratchFA78 > 2 ||
    has_treaty(faction_id_with, faction_id, 0x00040000) != 0 ||
    has_treaty(faction_id_with, faction_id, 0x02000000) != 0 ||
    has_agenda(faction_id_with, faction_id, 0x00002000) != 0 ||
    DiploScratchFA98 <= (DiplomacyProposalGateCount + 2) || IsMultiplayerNet != 0 ||
    wants_to_speak(faction_id, faction_id_with) == 0) {
    goto communicate_special_war_ransom_finish; // native 0x005562D9
}
communicate_special_war_selected_base_id = -1;
communicate_special_war_smallest_population = 999;
{
for (int communicate_base_scan_id = 0;
     communicate_base_scan_id < GameDataStateStorage.fields_.base_current_count_;
     ++communicate_base_scan_id) {
    const Base &communicate_candidate_base = Bases[communicate_base_scan_id];
    if (communicate_candidate_base.faction_id_current != faction_id_with ||
        communicate_candidate_base.faction_id_former != faction_id) {
        continue;
    }
    if (communicate_candidate_base.population_size <
        communicate_special_war_smallest_population) {
        communicate_special_war_smallest_population =
            communicate_candidate_base.population_size;
        communicate_special_war_selected_base_id = communicate_base_scan_id;
    }
}
}
if (communicate_special_war_selected_base_id < 0) {
    goto communicate_special_war_ransom_finish; // native 0x005562D9
}
if (wants_to_speak(faction_id, faction_id_with) == 0) {
    goto communicate_special_war_ransom_finish; // native 0x005562D9
}
clear();
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, Bases[communicate_special_war_selected_base_id]
                         .name_string, -1, -1);
if (X_pops("RANSOMTREATY", 0x00100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait) != 0) {
    goto communicate_special_war_ransom_finish; // native 0x005562D9
}
communicate_stack_34 = 1; // original 5562A3
net_cede_base(communicate_special_war_selected_base_id, faction_id, 1);
net_set_treaty(faction_id_with, faction_id, 0x02000000, 1, 0);
(void)pledge_truce(faction_id, faction_id_with, 0);
(void)make_treaty(faction_id, faction_id_with);
(void)diplomacy_caption(faction_id, faction_id_with);
communicate_special_war_ransom_finish:
if (has_treaty(faction_id_with, faction_id, 0x10) && wants_to_speak(faction_id, faction_id_with)) {
    if (DiploScratchFA24 &&
        (has_agenda(faction_id_with, faction_id, 0x800) ||
         (has_treaty(faction_id_with, faction_id, 0x20) &&
          !random(0, 3 - (has_treaty(faction_id_with, faction_id, 0x400) != 0))))) {
        communicate_truce_result = 1;
    } else if (communicate_stack_20) {
        communicate_truce_result = X_pops("OFFERTRUCE", 0x100000,
            FactionDiplomacyPortraits[faction_id_with], pop_wait);
    } else if (DiploScratchFA60) {
        communicate_truce_result = X_pops("MUSTTRUCE", 0x100000,
            FactionDiplomacyPortraits[faction_id_with], pop_wait);
    } else {
        mention_prototypes(faction_id, faction_id_with);
        parse_says(0, get_title(faction_id), -1, -1);
        parse_says(1, get_name(faction_id), -1, -1);
        parse_says(2, get_title(faction_id_with), -1, -1);
        parse_says(3, get_name(faction_id_with), -1, -1);
        clear();
        communicate_truce_variant = random(0, 4);
        if ((communicate_truce_variant != 2 || !has_treaty(faction_id_with, faction_id, 0x20)) &&
            communicate_truce_variant < 3) {
            says("WANTTOTRUCE");
            say_num(communicate_truce_variant);
            communicate_truce_result = X_pops(StringTemp, 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait);
        } else {
            if (Players[faction_id].rule_flags & 0x80) {
                says("ALIEN");
            }
            says("FACTIONTRUCE");
            communicate_truce_result = x_pops_caption_value(
                Players[faction_id_with].filename, StringTemp, 0x100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait);
        }
    }
    communicate_stack_1c = communicate_truce_result;
    communicate_stack_c = DiploScratchFA78 + 2;
    if (has_treaty(faction_id_with, faction_id, 0x20)) {
        communicate_stack_c += communicate_stack_c;
    }
    communicate_truce_limit = range(communicate_stack_c, 0,
                                    PlayersData[faction_id_with].current_num_bases + 1);
    if (communicate_truce_result) {
        if (random(0, communicate_truce_limit) >= DiploScratchFA98 && !DiploScratchFA24) {
            goto communicate_late_score_declined;
        }
        if (DiploScratchF800 >= 0 &&
            DiploScratchFA98 >= tech_val(DiploScratchF800, faction_id_with, 1) / 2 &&
            !DiploScratchFA24 && !DiploScratchFA70 &&
            !(GameDataStateStorage.fields_.game_rules_ & 0x2000000)) {
            for (;;) {
                clear();
                say_tech(DiploScratchF800, 1);
                parse_says(0, StringTemp, -1, -1);
                communicate_truce_tech = X_pops("TECHTRUCE", 0x100000,
                    FactionDiplomacyPortraits[faction_id_with], pop_wait);
                if (communicate_truce_tech <= 1) {
                    break;
                }
                help_topic(14, DiploScratchF800);
            }
            if (!communicate_truce_tech) {
                communicate_stack_1c = communicate_truce_tech;
                net_tech(faction_id, DiploScratchF800, faction_id_with, 1);
                communicate_stack_54 = 1;
                goto communicate_late_truce_tech_complete;
            }
        }
        if ((DiploScratchF800 < 0 || DiploScratchFA98 < 4) && !DiploScratchFA24 &&
            !DiploScratchFA70) {
            communicate_truce_energy = DiploScratchFA98 * 25;
            if (communicate_truce_energy >= PlayersData[faction_id_with].energy_reserves) {
                communicate_truce_energy = PlayersData[faction_id_with].energy_reserves;
            }
            communicate_truce_energy = communicate_round_down_multiple(communicate_truce_energy, 25);
            if (communicate_truce_energy) {
                parse_num(0, communicate_truce_energy);
                communicate_truce_tech = X_pops("ENERGYTRUCE", 0x100000,
                    FactionDiplomacyPortraits[faction_id_with], pop_wait);
                if (!communicate_truce_tech) {
                    communicate_stack_1c = communicate_truce_tech;
                    communicate_stack_54 = 1;
                    net_energy(faction_id_with, -communicate_truce_energy, faction_id,
                               communicate_truce_energy, 1);
                    ConsoleInfoWin.set_redraw_phase(0);
                    ConsoleGlobal.update_data(0);
                }
            }
        }
communicate_late_truce_tech_complete:
        communicate_truce_mine = (PlayersData[faction_id].mil_strength_2 +
                                  PlayersData[faction_id_with].total_mil_veh) *
            (PlayersData[faction_id].diplo_unk3[faction_id_with] +
             PlayersData[faction_id].diplo_unk4[faction_id_with]) / 4 + 1;
        communicate_truce_theirs = PlayersData[faction_id_with].diplo_unk3[faction_id] +
            PlayersData[faction_id_with].diplo_unk4[faction_id] + 1;
        for (communicate_stack_c = 0; communicate_stack_c < 128; communicate_stack_c++) {
            if (!PlayersData[faction_id_with].region_total_bases[communicate_stack_c]) {
                communicate_truce_mine +=
                    PlayersData[faction_id].region_force_rating[communicate_stack_c] / 2;
            } else {
                communicate_truce_theirs +=
                    PlayersData[faction_id_with].region_force_rating[communicate_stack_c] +
                    PlayersData[faction_id_with].region_total_combat_vehs[communicate_stack_c];
                communicate_truce_mine +=
                    PlayersData[faction_id].region_force_rating[communicate_stack_c];
                for (communicate_truce_other = 1; communicate_truce_other < 8;
                     communicate_truce_other++) {
                    if (has_treaty(communicate_truce_other, faction_id_with, 1) ||
                        !has_treaty(communicate_truce_other, faction_id, 0x10)) {
                        communicate_truce_theirs +=
                            PlayersData[communicate_truce_other].region_force_rating[communicate_stack_c];
                    }
                }
            }
        }
        if (communicate_stack_1c) {
            if (Players[faction_id_with].rule_flags & 0x80) {
                goto communicate_late_score_declined;
            }
            if (!DiploScratchFA24) {
                if (DiploScratchFA98 < PlayersData[faction_id_with].current_num_bases) {
                    goto communicate_late_score_declined;
                }
                if (PlayersData[faction_id_with].current_num_bases > 1 &&
                    (PlayersData[faction_id_with].current_num_bases >
                         communicate_truce_mine / communicate_truce_theirs ||
                     PlayersData[faction_id_with].current_num_bases > 4 ||
                     PlayersData[faction_id_with].mil_strength_1 >=
                         PlayersData[faction_id].mil_strength_1)) {
                    goto communicate_late_score_declined;
                }
            }
            parse_says(0, get_title(faction_id), -1, -1);
            parse_says(1, get_name(faction_id), -1, -1);
            parse_num(0, PlayersData[faction_id_with].energy_reserves);
            communicate_stack_c = 0;
            for (communicate_truce_other = 0; communicate_truce_other < 89; communicate_truce_other++) {
                if (has_tech(communicate_truce_other, faction_id_with) &&
                    !has_tech(communicate_truce_other, faction_id)) {
                    communicate_stack_c++;
                }
            }
            parse_num(1, communicate_stack_c);
            if (!X_pops("TRUCEPLEASE", 0x100000,
                        FactionDiplomacyPortraits[faction_id_with], pop_wait)) {
                communicate_stack_54 = 1;
                if (!(GameDataStateStorage.fields_.game_rules_ & 0x2000000)) {
                    for (communicate_truce_other = 0; communicate_truce_other < 89;
                         communicate_truce_other++) {
                        if (has_tech(communicate_truce_other, faction_id_with) &&
                            !has_tech(communicate_truce_other, faction_id)) {
                            net_tech(faction_id, communicate_truce_other, faction_id_with, 0);
                        }
                    }
                }
                net_energy(faction_id, PlayersData[faction_id_with].energy_reserves,
                           faction_id_with, -PlayersData[faction_id_with].energy_reserves, 1);
                ConsoleInfoWin.set_redraw_phase(0);
                ConsoleGlobal.update_data(0);
                PlayersData[faction_id].diplo_friction[faction_id_with] = 0;
                net_treaty_off(faction_id_with, faction_id, 0x20, 1);
                net_treaty_on(faction_id, faction_id_with, 2, 0);
                net_set_treaty(faction_id_with, faction_id, 0x2000000, 1, 0);
                pact_of_brotherhood(faction_id, faction_id_with);
                diplomacy_caption(faction_id, faction_id_with);
                if (!IsMultiplayerNet && (GameDataStateStorage.fields_.game_rules_ & 2)) {
                    communicate_stack_c = 0;
                    communicate_truce_shared = 0;
                    for (communicate_truce_other = 1; communicate_truce_other < 8;
                         communicate_truce_other++) {
                        if (GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &
                            (1 << communicate_truce_other)) {
                            communicate_stack_c++;
                            if (has_treaty(communicate_truce_other, ConsoleGlobal.faction_id_, 1) &&
                                has_treaty(communicate_truce_other, ConsoleGlobal.faction_id_,
                                           0x2000000)) {
                                communicate_stack_c--;
                            } else if (has_treaty(communicate_truce_other,
                                                  ConsoleGlobal.faction_id_, 1) &&
                                       (GameDataStateStorage.fields_.game_rules_ & 0x1000) &&
                                       communicate_truce_shared < 2) {
                                communicate_truce_shared++;
                            }
                        }
                    }
                    if (communicate_stack_c - communicate_truce_shared < 2) {
                        EndgameExitReason = 4;
                        GameDataStateStorage.fields_.game_state_ |= 0x2001;
                        if (communicate_truce_shared) {
                            EndgameExitReason = 5;
                        }
                    }
                }
            } else {
                net_set_agenda(faction_id_with, faction_id, 8, 1, 0);
                net_treaty_on(faction_id_with, faction_id, 0x20, 1);
communicate_late_score_declined:
                parse_says(0, get_title(faction_id), -1, -1);
                parse_says(1, get_name(faction_id), -1, -1);
                parse_says(2, get_adjective_insult_faction(faction_id), -1, -1);
                X_pops("MOREWAR", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
                PlayersData[faction_id].diplo_spoke[faction_id_with] =
                    GameDataStateStorage.fields_.turn_current_num_;
                goto communicate_final_cleanup;
            }
        }
        DiploScratchF7E4 = GameDataStateStorage.fields_.turn_current_num_;
    }
    communicate_stack_34 = 1;
    if (!has_treaty(faction_id, faction_id_with, 1)) {
        pledge_truce(faction_id, faction_id_with, 1);
    }
    diplomacy_caption(faction_id, faction_id_with);
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    if (DiploScratchFA78 > 5) {
        X_pops("NOTORIETY", 0x100000,
               FactionDiplomacyPortraits[faction_id_with], pop_wait);
    }
    if (PlayersData[faction_id].loan_balance[faction_id_with]) {
        parse_num(0, PlayersData[faction_id].loan_payment[faction_id_with]);
        parse_num(1, PlayersData[faction_id].loan_balance[faction_id_with]);
        X_pops("TRUCEMEPAY", 0x100000,
               FactionDiplomacyPortraits[faction_id_with], pop_wait);
    } else if (PlayersData[faction_id_with].loan_balance[faction_id]) {
        parse_num(0, PlayersData[faction_id_with].loan_payment[faction_id]);
        X_pops("TRUCEYOUPAY", 0x100000,
               FactionDiplomacyPortraits[faction_id_with], pop_wait);
    }
}

communicate_special_war_late_exit: // native 0x00556CDE
// PRIVATE native-order source proposal for 0x00556CDE..0x00556D6D.
// This joins the planetary offer continuation. The entry-result live-in is
// the existing communicate_stack_44 value (the BOOL set at 0x550078).
// Scratch names below are existing source-owned cells; no semantic rename is
// asserted by this fragment.
if (communicate_stack_44 != 0 &&
    has_treaty(faction_id_with, faction_id, 0x10) == 0) {
    // Native passes peer, actor, the combined 0x10004 treaty value, wait=1.
    net_treaty_on(faction_id_with, faction_id, 0x10004, 1); // 0x00556CFE
}
if (DiploScratchFA78 > 5 &&
    has_treaty(faction_id_with, faction_id, 0x20) != 0) {
    if (DiploScratchF7E0 != 0 || DiploScratchFA3C != 0 ||
        PlayersData[faction_id_with].mil_strength_1 >
            PlayersData[faction_id].mil_strength_1) {
        DiploScratchFA70 = 1; // 0x00556D63
    }
}
// Continue with the independent bounded-action/rank gates at 0x00556D6D.
// PRIVATE corrected candidate for 0x00556D6D..0x00556EEE. The full-start
// native row algebra resolves both computed pointers to PlayersData[peer] and
// PlayersData[actor], stride 2099 dwords. This supersedes the withdrawn first
// draft; it remains an incomplete section of communicate.
// Required function-scope locals:
//   int communicate_late_offer_amount;
//   int communicate_late_offer_special;
//   int communicate_late_offer_eligible;
communicate_late_offer_eligible = 0;
if (has_treaty(faction_id, faction_id_with, 2) == 0 &&
    has_agenda(faction_id, faction_id_with, 0x2000) == 0) {
    communicate_late_offer_eligible = 1;
    communicate_late_offer_special = 0;
    if (communicate_stack_50 != 0) {
        communicate_late_offer_special =
            has_treaty(faction_id, faction_id_with, 4) == 0;
    }
    communicate_late_offer_amount = (((PlayersData[faction_id].mil_strength_1 + 1) * (4 - PlayersData[faction_id_with].ai_fight)) * 2) / (PlayersData[faction_id_with].mil_strength_1 + 1);
    // The pinned literal at 0x0068E2B0 is "GAIANS".
    if (strcmp(Players[faction_id_with].filename, "GAIANS") != 0) {
        communicate_late_offer_amount = (communicate_late_offer_amount + 2);
    }
    if (communicate_late_offer_special != 0) {
        communicate_late_offer_amount = (communicate_late_offer_amount + (3 - PlayersData[faction_id_with].ai_fight));
    }
    const int actor_base_count =
        PlayersData[faction_id].current_num_bases;
    if (actor_base_count != 0 &&
        PlayersData[faction_id_with].tech_commerce_bonus >=
            PlayersData[faction_id].tech_commerce_bonus) {
        communicate_late_offer_amount = (communicate_late_offer_amount + PlayersData[faction_id].unk_13 / actor_base_count);
        if (PlayersData[faction_id_with].tech_commerce_bonus >
            PlayersData[faction_id].tech_commerce_bonus) {
            communicate_late_offer_amount = (communicate_late_offer_amount + 1);
        }
    }
    if (great_satan(faction_id, 0) == 0) {
        const int actor_rank = PlayersData[faction_id].ranking;
        const int peer_rank = PlayersData[faction_id_with].ranking;
        if (actor_rank <= 4 || peer_rank <= 4) {
            communicate_late_offer_amount = (communicate_late_offer_amount + 1);
            if (DiploScratchF7FC == 0) {
                communicate_late_offer_amount = (communicate_late_offer_amount + 1);
            }
        }
    }
}
// PRIVATE provisional draft for 0x00556EEE..0x00556FA4. It depends on the
// unresolved amount calculation in `late-contact-offer-score-556d6d-556eee.inc`
// and must not be integrated until that native indexed source owner is resolved.
// Requires the prior amount
// fragment's function-scope offer locals plus
// `int communicate_late_offer_range`. `communicate_late_offer_eligible`
// is the continuation predicate consumed by the following UI fragment.
if (communicate_late_offer_eligible != 0) {
    if ((DiploScratchFA98 == 0 &&
         DiploFriction > communicate_late_offer_amount) ||
        DiploScratchFA3C != 0) {
        communicate_late_offer_eligible = 0;
    }
}
if (communicate_late_offer_eligible != 0) {
    communicate_late_offer_amount =
        communicate_late_offer_special != 0 ||
        strcmp(Players[faction_id_with].filename, "GAIANS") == 0 ? 1 : 0;
    const int commerce_difference = (PlayersData[faction_id_with].tech_commerce_bonus - PlayersData[faction_id].tech_commerce_bonus);
    communicate_late_offer_range = range(commerce_difference, 0, 99);
    const int total_score = (((communicate_late_offer_amount + communicate_late_offer_range) + DiploScratchFA98 != 0 ? 1 : 0) + 5);
    if ((DiploScratchFA78 + DiploScratchFA2C) >= total_score) {
        communicate_late_offer_eligible = 0;
    }
}
if (communicate_late_offer_eligible != 0 &&
    has_treaty(faction_id, faction_id_with, 0x20) != 0 &&
    (DiploScratchFA98 == 0 || communicate_late_offer_special == 0 ||
     DiploScratchFA78 > 4)) {
    communicate_late_offer_eligible = 0;
}
if (communicate_late_offer_eligible != 0 &&
    wants_to_speak(faction_id, faction_id_with) == 0) {
    communicate_late_offer_eligible = 0;
}
// A nonzero predicate now enters parser/popup construction at 0x00556FA4.
// PRIVATE provisional candidate for the 0x00556FA4..0x005571E7 offer/popup
// continuation. It depends on unresolved upstream offer amount/indexing, so do
// not integrate the connected offer path until that is resolved.
// It depends on the preceding offer-gate fragment's live locals and is not a
// full-function replacement. All post-popup PlayerData reads are deliberately
// made after the callback, from current typed owners.
if (communicate_late_offer_eligible != 0) {
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_adjective_faction(faction_id), -1, -1);
    parse_says(3, get_name(faction_id_with), -1, -1);
    parse_says(4, get_title(faction_id_with), -1, -1);
    int communicate_late_offer_result = 0;
    if (communicate_stack_50 == 0) {
        clear();
        if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
            says("ALIEN");
        }
        says("FACTIONTREATY");
        communicate_late_offer_result = x_pops_caption_value(
            Players[faction_id_with].filename, StringTemp, 0x00100000,
            FactionDiplomacyPortraits[faction_id_with], pop_wait);
    } else {
        clear();
        says("OFFERTREATY");
        if (has_treaty(faction_id, faction_id_with, 4) == 0) says("2");
        communicate_late_offer_result = X_pops(
            StringTemp, 0x00100000,
            FactionDiplomacyPortraits[faction_id_with], pop_wait);
    }
    communicate_stack_1c = communicate_late_offer_result; // 0x5570D8
    if (communicate_late_offer_result != 0 &&
        has_treaty(faction_id_with, faction_id, 0x20) == 0) {
        const int energy_limit = (DiploScratchFA98 * 50);
        const int peer_energy = PlayersData[faction_id_with].energy_reserves;
        int communicate_late_energy_offer =
            peer_energy >= energy_limit ? energy_limit : peer_energy;
        communicate_late_energy_offer = communicate_round_down_multiple(
            communicate_late_energy_offer, 25);
        if (communicate_late_energy_offer != 0) {
            parse_says(0, get_title(faction_id_with), -1, -1);
            parse_says(1, get_name(faction_id_with), -1, -1);
            parse_says(2, get_noun_phrase(faction_id, 9), -1, -1);
            parse_num(0, communicate_late_energy_offer);
            communicate_late_offer_result = X_pops(
                "ENERGYTREATY", 0x00100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait);
            if (communicate_late_offer_result == 0) {
                communicate_stack_54 = 1;
                communicate_stack_1c = 0;
                net_energy(faction_id_with, (0 - communicate_late_energy_offer),
                           faction_id, communicate_late_energy_offer, 1);
                ConsoleInfoWin.set_redraw_phase(0);
                ConsoleGlobal.update_data(0);
            }
        }
    }
    communicate_stack_34 = 1; // native 0x5571CB
    if (communicate_stack_1c == 0) {
        make_treaty(faction_id, faction_id_with);
        diplomacy_caption(faction_id, faction_id_with);
    }
}
// Native rejoins the next contact-mode dispatch at 0x005571E7.
// PRIVATE v3 native-owner correction for 0x005571E7..0x0055792C.
// Continues from the late-contact offer branches. This is still a WIP: the
// subsequent alliance/treaty section and common dialog tail are not included.
// EDI/ESI remain the original actor/peer arguments. Do not merge as complete.
// Native 0x5571E7..0x5575C7, preceding the retained vendetta matrix.
}
communicate_after_trade:
if (communicate_stack_44) {
    wants_to_speak(faction_id, faction_id_with);
}
if ((!DiplomacyEndState || !DiplomacyContactResult) && !mode) {
    goto communicate_final_cleanup;
}
if (!communicate_stack_34) {
    if (communicate_stack_38 && !communicate_stack_20) {
        parse_says(0, get_title(faction_id), -1, -1);
        parse_says(1, get_name(faction_id), -1, -1);
        parse_says(2, get_pacts(faction_id, faction_id_with), -1, -1);
        parse_says(3, get_pact(faction_id), -1, -1);
        if (communicate_stack_48) {
            if (has_treaty(faction_id_with, faction_id, 1)) {
                X_pops("MILDPACT", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            } else if (has_treaty(faction_id, faction_id_with, 2)) {
                X_pops("MILDTREATY", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            } else {
                X_pops("MILD", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
                mention_prototypes(faction_id, faction_id_with);
            }
        } else if (has_treaty(faction_id_with, faction_id, 1)) {
            X_pops("REBUFFEDPACT", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
        } else {
            X_pops(has_treaty(faction_id, faction_id_with, 6) ? "REBUFFEDTREATY" : "REBUFFED",
                   0x100000, FactionDiplomacyPortraits[faction_id_with], pop_wait);
            mention_prototypes(faction_id, faction_id_with);
        }
    } else if (mode) {
        if (PlayersData[faction_id_with].diplo_patience[faction_id] > 1) {
            if (has_treaty(faction_id_with, faction_id, 1)) {
                parse_says(0, get_pact(faction_id), -1, -1);
                X_pops("LOWPATIENCEPACT", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            } else {
                parse_says(0, get_title(faction_id), -1, -1);
                X_pops("LOWPATIENCE", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            }
        } else {
            parse_says(0, get_title(faction_id), -1, -1);
            parse_says(1, get_name(faction_id), -1, -1);
            if (has_treaty(faction_id_with, faction_id, 1)) {
                X_pops("WHATSUPPACT", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            } else if (has_treaty(faction_id_with, faction_id, 2)) {
                X_pops("WHATSUPTREATY", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            } else {
                X_pops("WHATSUP", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
            }
        }
    } else {
        parse_says(0, get_pact_hood(faction_id, faction_id_with), -1, -1);
        parse_says(1, get_title(faction_id), -1, -1);
        parse_says(2, get_name(faction_id), -1, -1);
        parse_says(3, get_noun_phrase(faction_id_with, 9), -1, -1);
        if (has_treaty(faction_id_with, faction_id, 1)) {
            X_pops("HELLOPACT", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
        } else if (has_treaty(faction_id_with, faction_id, 2)) {
            X_pops("HELLOTREATY", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
        } else {
            X_pops("HELLO", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
            mention_prototypes(faction_id, faction_id_with);
        }
    }
} else if (!has_treaty(faction_id_with, faction_id, 0x60) ||
           (!mode && DiploScratchFA78 < 5)) {
    communicate_wronged_id = -1;
    communicate_wronged_count = 0;
    for (communicate_stack_c = 1; communicate_stack_c < 8; communicate_stack_c++) {
        if (communicate_stack_c != faction_id && communicate_stack_c != faction_id_with &&
            !has_treaty(faction_id, communicate_stack_c, 0x10) &&
            PlayersData[faction_id].diplo_wrongs[communicate_stack_c] > communicate_wronged_count) {
            communicate_wronged_id = communicate_stack_c;
            communicate_wronged_count = PlayersData[faction_id].diplo_wrongs[communicate_stack_c];
        }
    }
    if (communicate_wronged_id >= 0) {
        parse_says(0, get_title(communicate_wronged_id), -1, -1);
        parse_says(1, get_name(communicate_wronged_id), -1, -1);
        parse_says(2, get_noun(communicate_wronged_id), -1, -1);
        parse_says(3, get_him_her(communicate_wronged_id, 0), -1, -1);
        parse_says(4, get_title(faction_id), -1, -1);
        parse_says(5, get_name(faction_id), -1, -1);
        PlayersData[faction_id].diplo_wrongs[communicate_wronged_id] = 0;
        clear();
        says("WRONGED");
        says(has_treaty(faction_id_with, communicate_wronged_id, 0x10) ? "ENEMY"
             : (PlayersData[faction_id_with].mil_strength_1 >
                PlayersData[faction_id].mil_strength_1 ? "MIGHTY" : "MEEK"));
        X_pops(StringTemp, 0x100000,
               FactionDiplomacyPortraits[faction_id_with], pop_wait);
        mention_prototypes(faction_id, faction_id_with);
    }
}
DiploScratchF7B4 = 0;
// stand-in: the image indexes the priority/opposition pairs as arrays.
for (communicate_stack_c = 0; communicate_stack_c < 1; communicate_stack_c++) {
    if ((&Players[faction_id_with].soc_ideology_category)[communicate_stack_c] >= 0 &&
        (&Players[faction_id_with].soc_ideology_model)[communicate_stack_c]) {
        parse_says(0, get_title(faction_id), -1, -1);
        parse_says(1, get_name(faction_id), -1, -1);
        clear();
        says("SOCIAL");
        say_num((&Players[faction_id_with].soc_ideology_category)[communicate_stack_c]);
        says("CAT");
        if (PlayersData[faction_id].soc_category_pending.models_[
                (&Players[faction_id_with].soc_ideology_category)[communicate_stack_c]] ==
            (&Players[faction_id_with].soc_ideology_model)[communicate_stack_c]) {
            goto communicate_social_good;
        }
        if (PlayersData[faction_id].soc_category_pending.models_[
                (&Players[faction_id_with].soc_ideology_category)[communicate_stack_c]]) {
            DiploScratchF7B4 = 1;
            if (!DiploScratchFA98 &&
                (!has_treaty(faction_id_with, faction_id, 1) ||
                 !has_treaty(faction_id_with, faction_id, 0x2000000))) {
                goto communicate_social_bad;
            }
        }
    }
}
goto communicate_social_done;
communicate_social_good:
say_num((&Players[faction_id_with].soc_ideology_model)[communicate_stack_c]);
says("GOOD");
X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[faction_id_with], pop_wait);
goto communicate_social_done;
communicate_social_bad:
say_num(PlayersData[faction_id].soc_category_pending.models_[
    (&Players[faction_id_with].soc_ideology_category)[communicate_stack_c]]);
says("BAD");
say_num((&Players[faction_id_with].soc_ideology_model)[communicate_stack_c]);
X_pops(StringTemp, 0x100000, FactionDiplomacyPortraits[faction_id_with], pop_wait);
net_set_agenda(faction_id_with, faction_id, 1, 1, 1);
communicate_social_done:

communicate_late_social_terms_done: // native 0x0055792C
// PRIVATE v3 native-order correction for 0x0055792C..0x00558152.
// Preserves v1/v2; applies native per-region/map-mask/base-count gates. Scratch F7B4 is centrally admitted
// neutral start-only owner; its extent/lifecycle/meaning remain unknown.
// Required locals: rival ID/counters, region ID, selected IDs/ratings and the
// temporary region-hit booleans. Verify this section in the composed host body.
if (DiploScratchF7B4 == 0) {
    net_set_agenda(faction_id_with, faction_id, 1, 0, 1); // 0x557935
}
if (PlayersData[faction_id].atrocities != 0) {
    if (has_treaty(faction_id_with, faction_id, 0x00200000) != 0) {
        if (has_treaty(faction_id_with, faction_id, 0x00400000) == 0) {
            goto communicate_late_contact_atrocity_done;
        }
        // 0x96CDBC is PlayerData.ai_fight, not ai_growth.
        if (PlayersData[faction_id].atrocities <=
            (PlayersData[faction_id_with].ai_fight + 1) ||
            (Players[faction_id].rule_flags & 0x80) != 0 ||
            (has_treaty(faction_id_with, faction_id, 1) != 0 &&
             has_treaty(faction_id_with, faction_id, 0x02000000) != 0)) {
            goto communicate_late_contact_atrocity_done;
        }
        for (communicate_late_contact_rank = 1;
             communicate_late_contact_rank < MaxPlayerNum;
             ++communicate_late_contact_rank) {
            if (communicate_late_contact_rank == faction_id ||
                communicate_late_contact_rank == faction_id_with) {
                continue;
            }
            if (has_treaty(communicate_late_contact_rank, faction_id,
                           0x00040000) != 0) {
                goto communicate_late_contact_third_party_atrocity;
            }
        }
    }
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    X_pops("SELFATROCITY", 0x00100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait);
    if (has_treaty(faction_id_with, faction_id, 0x00200000) == 0) {
        if (PlayersData[faction_id].atrocities > 1) {
            PlayersData[faction_id].atrocities = (PlayersData[faction_id].atrocities - 1);
        }
        if (PlayersData[faction_id].integrity_blemishes > 1) {
            PlayersData[faction_id].integrity_blemishes = (PlayersData[faction_id].integrity_blemishes - 1);
        }
    }
    net_set_treaty(faction_id_with, faction_id, 0x00200000, 1, 0);
    net_set_treaty(faction_id_with, faction_id, 0x00400000, 0, 0);
    goto communicate_late_contact_atrocity_done;
}
// The native outer-attempt guard also skips the committed-atrocity block
// when actor.atrocities is zero; the label is not a fallthrough continuation.
goto communicate_late_contact_atrocity_done;
communicate_late_contact_third_party_atrocity:
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_noun(communicate_late_contact_rank), -1, -1);
X_pops("COMMITTEDATROCITY", 0x00100000,
             FactionDiplomacyPortraits[faction_id_with], pop_wait);
if (has_treaty(faction_id_with, faction_id, 0x00200000) == 0) {
    if (PlayersData[faction_id].atrocities > 1) {
        PlayersData[faction_id].atrocities = (PlayersData[faction_id].atrocities - 1);
    }
    if (PlayersData[faction_id].integrity_blemishes > 1) {
        PlayersData[faction_id].integrity_blemishes = (PlayersData[faction_id].integrity_blemishes - 1);
    }
}
net_set_treaty(faction_id_with, faction_id, 0x00200000, 1, 0);
net_set_treaty(faction_id_with, faction_id, 0x00400000, 0, 0);
communicate_late_contact_atrocity_done: // native 0x557AC0
if (has_treaty(faction_id, faction_id_with, 1) != 0) {
    parse_says(0, get_pact(faction_id), -1, -1);
    net_maps(faction_id, faction_id_with, 1);
    if (DiplomacyMapTransferChanged != 0) {
        X_pops("HERESTHEMAP", 0x00100000,
                     FactionDiplomacyPortraits[faction_id_with], pop_wait);
    }
}
if ((has_treaty(faction_id, faction_id_with, 1) || has_treaty(faction_id_with, faction_id, 0x80)) &&
    !(dialogue_flags & 0x40)) {
    communicate_stack_3c = 0;
    communicate_report_min = 9999;
    communicate_report_third = 0;
    communicate_report_region_rival = 0;
    communicate_report_rival = 0;
    for (communicate_stack_c = 1; communicate_stack_c < 8; communicate_stack_c++) {
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << communicate_stack_c)) &&
            communicate_stack_c != faction_id && communicate_stack_c != faction_id_with &&
            has_treaty(faction_id_with, communicate_stack_c, 0x10) &&
            has_treaty(faction_id_with, communicate_stack_c, 8) &&
            has_treaty(faction_id_with, communicate_stack_c, 0x8000) &&
            has_treaty(faction_id, communicate_stack_c, 8) &&
            (has_treaty(faction_id_with, communicate_stack_c, 0x40) ||
             has_treaty(faction_id_with, faction_id, 1))) {
            if (PlayersData[communicate_stack_c].mil_strength_1 > communicate_stack_3c) {
                communicate_stack_3c = PlayersData[communicate_stack_c].mil_strength_1;
                communicate_report_rival = communicate_stack_c;
            }
            if (PlayersData[communicate_stack_c].mil_strength_1 < communicate_report_min) {
                for (communicate_report_region = 1; communicate_report_region < 64;
                     communicate_report_region++) {
                    if (!bad_reg(communicate_report_region) &&
                        PlayersData[faction_id_with].region_total_bases[communicate_report_region] &&
                        PlayersData[communicate_stack_c].region_total_bases[communicate_report_region] &&
                        PlayersData[communicate_stack_c].region_force_rating[communicate_report_region] <=
                            PlayersData[faction_id_with].region_total_combat_vehs[communicate_report_region] +
                            PlayersData[faction_id_with].region_force_rating[communicate_report_region] &&
                        contiguous(faction_id_with, communicate_stack_c, communicate_report_region, 0)) {
                        communicate_report_region_rival = communicate_stack_c;
                        communicate_report_min = PlayersData[communicate_stack_c].mil_strength_1;
                        goto communicate_report_next;
                    }
                }
            }
            if (communicate_report_region_rival != communicate_stack_c &&
                communicate_report_rival != communicate_stack_c) {
                communicate_report_third = communicate_stack_c;
            }
        }
communicate_report_next:;
    }
    communicate_stack_3c = 0;
    if (communicate_report_rival) {
        for (communicate_stack_c = 1; communicate_stack_c < 128; communicate_stack_c++) {
            if (!bad_reg(communicate_stack_c) &&
                (DiplomacyRegionMask[communicate_stack_c] & (1 << faction_id_with)) &&
                PlayersData[communicate_report_rival].region_force_rating[communicate_stack_c] >
                    PlayersData[faction_id_with].region_total_combat_vehs[communicate_stack_c]) {
                communicate_stack_3c = 1;
                break;
            }
        }
        if (has_treaty(faction_id, faction_id_with, 0x2000) || communicate_stack_3c) {
            parse_says(1, get_title(communicate_report_rival), -1, -1);
            parse_says(2, get_name(communicate_report_rival), -1, -1);
            parse_says(3, get_noun(communicate_report_rival), -1, -1);
            parse_says(4, get_his_her(communicate_report_rival, 0), -1, -1);
            parse_says(5, get_adjective_insult_faction(communicate_report_rival), -1, -1);
            X_pops("REPORTLOSING", 0x100000,
                   FactionDiplomacyPortraits[faction_id_with], pop_wait);
            goto communicate_late_contact_map_done;
        }
    }
    if (communicate_report_region_rival && !has_treaty(faction_id, faction_id_with, 0x2000)) {
        for (communicate_stack_c = 1; communicate_stack_c < 128; communicate_stack_c++) {
            if (!bad_reg(communicate_stack_c) && DiplomacyRegionMask[communicate_stack_c] &&
                (1 << communicate_report_region_rival) &&
                PlayersData[faction_id_with].region_force_rating[communicate_stack_c] >
                    PlayersData[communicate_report_region_rival].region_total_combat_vehs[communicate_stack_c]) {
                parse_says(0, get_adjective(communicate_report_region_rival), -1, -1);
                parse_says(1, get_title(communicate_report_region_rival), -1, -1);
                parse_says(2, get_name(communicate_report_region_rival), -1, -1);
                parse_says(3, get_phrase(communicate_report_region_rival, 5), -1, -1);
                X_pops("REPORTWINNING", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
                goto communicate_late_contact_map_done;
            }
        }
    }
    if (communicate_report_third) {
        for (communicate_stack_c = 1; communicate_stack_c < 128; communicate_stack_c++) {
            if (!bad_reg(communicate_stack_c) &&
                PlayersData[communicate_report_third].region_total_bases[communicate_stack_c] &&
                PlayersData[faction_id_with].region_total_bases[communicate_stack_c]) {
                parse_says(1, get_title(communicate_report_third), -1, -1);
                parse_says(2, get_name(communicate_report_third), -1, -1);
                parse_says(3, get_noun(communicate_report_third), -1, -1);
                parse_says(4, get_adjective(communicate_report_third), -1, -1);
                parse_says(5, get_noun_phrase(communicate_report_third, 8), -1, -1);
                X_pops("REPORTEVEN", 0x100000,
                       FactionDiplomacyPortraits[faction_id_with], pop_wait);
                goto communicate_late_contact_map_done;
            }
        }
    }
}

communicate_late_contact_map_done: // native 0x558152
// PRIVATE, STILL-INCOMPLETE function section: native 0x00558152..0x005584AC.
// This version replaces the rough tech/territory draft, preserving the
// original branch predicates, directionality, typed owners, and ordered
// bitmask/parser side effects. It is not a standalone function.
// Native asks whether the peer (ESI) has treaty bit 1 with this actor (EDI).
if (has_treaty(faction_id, faction_id_with, 1) != 0) {
    mention_prototypes(faction_id, faction_id_with); // 0x558162
}
if (has_treaty(faction_id, faction_id_with, 1) != 0 &&
    (GameDataStateStorage.fields_.game_rules_ & 0x200) == 0) {
    communicate_late_contact_research_tech =
        PlayersData[faction_id_with].tech_id_researching;
    if (communicate_late_contact_research_tech >= 0 &&
        has_tech(communicate_late_contact_research_tech, faction_id) == 0 &&
        communicate_late_contact_research_tech !=
            PlayersData[faction_id].tech_id_researching &&
        (GameDataStateStorage.fields_.game_rules_ & 0x06000000) == 0 &&
        PlayersData[faction_id].tech_accumulated >=
            tech_rate(faction_id) / 2) {
        communicate_late_contact_best_tech_value = 0;
        communicate_late_contact_best_tech = -1;
        for (communicate_late_contact_tech = 0;
             communicate_late_contact_tech < MaxTechnologyNum;
             ++communicate_late_contact_tech) {
            const int tech_id = communicate_late_contact_tech;
            const RulesTechnology &candidate = Technology[tech_id];
            if (has_tech(tech_id, faction_id) != 0 ||
                tech_id == PlayersData[faction_id].tech_id_researching ||
                tech_id == PlayersData[faction_id_with].tech_id_researching ||
                tech_avail(tech_id, faction_id) == 0) {
                continue;
            }
            // 0x55825D..0x55827F is signed IDIV by 3. For IDs below 87,
            // the image rejects remainder zero; for IDs 87 and 88 it rejects
            // nonzero remainder. Keep the native 32-bit wrapped numerator.
            const int alignment = (((tech_id - PlayersData[faction_id].tech_ranking) + faction_id) + 2);
            const int alignment_remainder = alignment % 3;
            if ((tech_id < 87 && alignment_remainder == 0) ||
                (tech_id >= 87 && alignment_remainder != 0)) {
                continue;
            }
            if (has_tech(candidate.preq_tech_1, faction_id) == 0 ||
                has_tech(candidate.preq_tech_2, faction_id) == 0) {
                continue;
            }
            const int candidate_value = tech_val(tech_id, faction_id_with, 0);
            if (candidate_value >= 0x270F) {
                break;
            }
            if (candidate_value > communicate_late_contact_best_tech_value) {
                communicate_late_contact_best_tech_value = candidate_value;
                communicate_late_contact_best_tech = tech_id;
            }
        }
        if (communicate_late_contact_best_tech >= 0) {
            // Native order: mark the peer's current research in the peer row,
            // parse it into slot 0, then mark/parse the selected tech in the
            // actor row/slot 1. Do not collapse or reorder these effects.
            bitmask(communicate_late_contact_research_tech,
                    &communicate_late_contact_tech_mask_offset,
                    &communicate_late_contact_tech_mask);
            PlayersData[faction_id_with].tech_pact_shared_goals[
                communicate_late_contact_tech_mask_offset] |=
                    static_cast<uint32_t>(communicate_late_contact_tech_mask);
            parse_say(0, Technology[communicate_late_contact_research_tech].name, -1, -1);
            bitmask(communicate_late_contact_best_tech,
                    &communicate_late_contact_tech_mask_offset,
                    &communicate_late_contact_tech_mask);
            PlayersData[faction_id].tech_pact_shared_goals[
                communicate_late_contact_tech_mask_offset] |=
                    static_cast<uint32_t>(communicate_late_contact_tech_mask);
            parse_say(1, Technology[communicate_late_contact_best_tech].name, -1, -1);
            X_pops("TEAMTECH", 0x00100000,
                         FactionDiplomacyPortraits[faction_id_with], pop_wait);
        }
    }
}
if (has_treaty(faction_id_with, faction_id, 3) == 0) { // 0x558392
    communicate_late_contact_base_id = -1;
    if (territory(faction_id_with, faction_id, 0,
                  &communicate_late_contact_base_id, 0) != 0) {
        spot_base(communicate_late_contact_base_id, faction_id);
        MapWinTable[0]->focus(
            Bases[communicate_late_contact_base_id].x,
            Bases[communicate_late_contact_base_id].y);
        parse_says(1, get_adjective(faction_id_with), -1, -1);
        X_pops("TERRITORY", 0x00100000,
                     FactionDiplomacyPortraits[faction_id_with], pop_wait);
        mention_prototypes(faction_id, faction_id_with); // 0x558425
    }
}
if (DiploScratchFA4C != 0) { // 0x55842D
    parse_says(0, get_title(DiploScratchF7B8), -1, -1);
    parse_says(1, get_name(DiploScratchF7B8), -1, -1);
    parse_says(2, get_adjective_insult_leader(DiploScratchF7B8), -1, -1);
    X_pops("QUISLING", 0x00100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait);
}
// Native continuation point 0x5584AC.
// PRIVATE, INCOMPLETE correction of native 0x005584AC..0x00558980.
// Supersedes v2 only for the approved typed start-only neutral owners at
// 0x93FAD0 and 0x93FAD4. The owner proposal does not assert their allocation
// extents beyond the separately admitted DWORD cells or infer their roles.
DiploScratchFAD0 = 0; // native 0x5584AE
DiploScratchFAD4 = 0; // native 0x5584B9
communicate_patience_ptr =
    &PlayersData[faction_id_with].diplo_patience[faction_id];
communicate_contact_faction_scan: // native re-entry 0x005584D2
if (*communicate_patience_ptr < 0) {
    *communicate_patience_ptr = 0;
}
DiploScratchFA94 = -1;
DiploScratchF7DC = -1;
for (communicate_late_contact_rank = 1;
     communicate_late_contact_rank < MaxPlayerNum;
     ++communicate_late_contact_rank) {
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1u << (communicate_late_contact_rank))) == 0 ||
        communicate_late_contact_rank == faction_id ||
        communicate_late_contact_rank == faction_id_with) {
        continue;
    }
    if (has_treaty(faction_id_with, communicate_late_contact_rank, 0x10) == 0 ||
        has_treaty(faction_id, communicate_late_contact_rank, 8) == 0) {
        continue;
    }
    if (has_treaty(faction_id, communicate_late_contact_rank, 7) != 0) {
        DiploScratchF7DC = communicate_late_contact_rank;
    }
    if (has_treaty(faction_id, communicate_late_contact_rank, 0x10) != 0) {
        DiploScratchFA94 = communicate_late_contact_rank;
    }
}
if (IsMultiplayerNet != 0 && (GameDataStateStorage.fields_.game_state_ & 0x800) != 0) {
    goto communicate_late_contact_exit;
}
{
    communicate_late_contact_choice = diplomacy_menu(faction_id, faction_id_with);
    if (static_cast<uint32_t>(communicate_late_contact_choice) <= 7u) {
        switch (communicate_late_contact_choice) {
        case 0:
            goto communicate_final_cleanup; // native 0x5587C8
        case 1:
        case 2:
            make_a_proposal(faction_id, faction_id_with,
                            communicate_late_contact_choice == 1);
            if (DiplomacyProposalMode == 6) {
                goto communicate_late_contact_after_proposal; // 0x558701
            }
            goto communicate_late_contact_menu_loop; // 0x55870B
        case 3:
            communicate_late_contact_text_result = X_pops(
                "REALLYBROKE", 0x00100000,
                FactionDiplomacyPortraits[faction_id_with], pop_wait);
            if (communicate_late_contact_text_result == 0) {
                // Native re-enters after showing BROKEPACT and decrementing
                // the signed patience byte.
                parse_says(0, get_pact_hood(faction_id, faction_id_with), -1, -1);
                X_pops("DROIDS", 0x00100000,
                             FactionDiplomacyPortraits[faction_id_with], pop_wait);
                --*communicate_patience_ptr;
                goto communicate_late_contact_menu_loop; // native 0x55870B
            }
            diplo_lock(0);
            PlayersData[faction_id].integrity_blemishes = (PlayersData[faction_id].integrity_blemishes + 1);
            net_treaty_off(faction_id_with, faction_id, 1, 1);
            diplomacy_check(faction_id, faction_id_with, 0);
            diplomacy_caption(faction_id, faction_id_with);
            parse_says(0, get_pact(faction_id), -1, -1);
            parse_says(1, get_title(faction_id), -1, -1);
            parse_says(2, get_name(faction_id), -1, -1);
            X_pops("BROKEPACT", 0x00100000,
                         FactionDiplomacyPortraits[faction_id_with], pop_wait);
            net_pact_ends(faction_id, faction_id_with, 1);
            diplo_unlock();
            goto communicate_late_contact_menu_loop; // native 0x55870B
        case 4:
            demand_withdrawal(faction_id, faction_id_with);
            goto communicate_late_contact_menu_loop;
        case 5:
            battle_plans(faction_id, faction_id_with);
            goto communicate_late_contact_menu_loop;
        case 6:
            call_off_vendetta(faction_id, faction_id_with);
            goto communicate_late_contact_menu_loop;
        case 7:
            DiploScratchFA34 = 12; // admitted start-only cell at 0x93FA34
            DiplomacyProposalMode = 1;
            make_a_proposal(faction_id, faction_id_with, 1);
            goto communicate_late_contact_after_proposal;
        }
    }
}
communicate_late_contact_after_proposal: // native 0x558701
DiploScratchFAD0 = 0;
communicate_late_contact_menu_loop: // native 0x55870B
*communicate_patience_ptr = static_cast<char>(
    static_cast<uint8_t>(*communicate_patience_ptr) + 1u);
if (static_cast<int8_t>(*communicate_patience_ptr) <
    get_patience(faction_id, faction_id_with)) {
    goto communicate_contact_faction_scan; // native 0x5584D2
}
communicate_late_contact_exit: // native 0x558725
diplomacy_caption(faction_id, faction_id_with);
if (has_treaty(faction_id, faction_id_with, 0x10) != 0) {
    goto communicate_final_cleanup;
}
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_name(faction_id_with), -1, -1);
if (has_treaty(faction_id_with, faction_id, 1) != 0) {
    X_pops("TIMESUPALLY", 0x00100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait);
} else {
    X_pops("TIMESUP", 0x00100000,
                 FactionDiplomacyPortraits[faction_id_with], pop_wait);
}
communicate_cleanup: // native common cleanup entry
communicate_common_epilogue:
communicate_epilogue:
communicate_final_cleanup: // native 0x5587C8
net_treaty_off(faction_id_with, faction_id, 0x2000, 0);
net_agenda_off(faction_id_with, faction_id, 4, 0);
communicate_late_contact_text_result = 0;
if (DiploScratchFAA0 != 0 &&
    has_treaty(faction_id, faction_id_with, 0x11) == 0) {
    communicate_late_contact_text_result = 1;
}
net_set_agenda(faction_id_with, faction_id, 0x10,
               communicate_late_contact_text_result, 0);
communicate_late_contact_text_result = 0;
if (DiploScratchFA70 != 0 &&
    has_treaty(faction_id, faction_id_with, 0x11) == 0) {
    communicate_late_contact_text_result = 1;
}
net_set_treaty(faction_id_with, faction_id, 0x400,
               communicate_late_contact_text_result, 1);
if (communicate_stack_54 != 0 || (DiploScratchFA60 != 0 &&
    has_treaty(faction_id, faction_id_with, 0x10) == 0)) {
    char &patience = PlayersData[faction_id_with].diplo_patience[faction_id];
    patience = static_cast<char>(range(
        static_cast<int8_t>(patience),
        get_patience(faction_id, faction_id_with), 0x3E7));
}
diplomacy_ends(faction_id, faction_id_with);
if (communicate_stack_50 != 0 &&
    has_treaty(faction_id, faction_id_with, 0x10013) == 0 && mode == 0) {
    parse_says(0, get_title(faction_id_with), -1, -1);
    parse_says(1, get_name(faction_id_with), -1, -1);
    parse_says(2, get_noun(faction_id_with), -1, -1);
    if (has_treaty(faction_id, faction_id_with, 4) != 0) {
        X_pops("RENEWTRUCE", 0x00100000,
                     FactionDiplomacyPortraits[faction_id_with], pop_wait);
    } else {
        X_pops("ENDTRUCE", 0x00100000,
                     FactionDiplomacyPortraits[faction_id_with], pop_wait);
    }
}
goto communicate_popup_destructor; // native 0x55895D
communicate_popup_destructor: // cold span 0x65FF4F..0x65FF64
// Let the single lexical Popup in the eventual whole body run its destructor.

; // The automatic Popup generates both normal and exceptional destruction.

}
#undef COMMUNICATE_LOCK
#undef COMMUNICATE_UNLOCK
#undef COMMUNICATE_FRICTION
#undef COMMUNICATE_WITHDRAW
#pragma inline_depth(8)


/*
Purpose: Round an integer DOWN to a multiple of another, toward zero.

             mov eax, [ebp+8] / mov ecx, [ebp+0xc]
             cdq / idiv ecx / imul eax, ecx
             ret

         `x / y * y` with signed division, so it truncates toward zero rather
         than toward negative infinity: -7 with y=3 gives -6, not -9.

         The compiler rewrites this as `x - x % y`, which is the same value for
         every input and traps in the same two places, because both spellings
         go through one `idiv`: y == 0 and INT_MIN / -1 raise #DE here exactly
         as they do in the original.


// ORIGINAL: 0x00559210 sub_559210 0x00559210-0x00559221 BYTE_EXACT
// symbol    ?communicate_round_down_multiple@@YAHHH@Z
// size      17 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: the largest multiple of `step` no further from zero than `value`
Status: Complete
*/
int communicate_round_down_multiple(int value, int step) {
    return value / step * step;
}


// ORIGINAL: 0x0054DC90 sub_54dc90 0x0054DC90-0x0054DCE4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x0054DC90:0054DC90-0054DCE4", "source:src/planwin.h:PlanWin and genuine MapWin/GraphicWin interfaces", "source:src/faction.h:PlayerData, battle faction state and label declarations", "recovery/tasks/planwin-battle-owner-coverage.json"], "reason": "Use the pinned native body with the real typed owner and preserve the seed's original identity and history."}
// symbol    _sub_54dc90
extern "C" void __cdecl sub_54dc90(int base_id) {
    PlanWindow.clear_lines();
    PlanWindow.monocenter(Bases[base_id].x, Bases[base_id].y, 0);
    PlanWindow.add_line(base_id, base_id, 0);
    PlanWindow.on_redraw();
}
