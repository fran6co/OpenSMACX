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

#include "temp.h"
#include "faction.h"
#include "tutwin.h"
#include "alpha.h"
#include "base.h"
#include "council.h"
#include "councwin.h"
#include "popup.h"
#include "dialogs.h"
#include "text.h"
#include "game.h"
#include "map.h"
#include "probe.h"
#include "technology.h"
#include "veh.h"
#include "spying_recovery.h"  // SpyingStatusTable, SpyingStatusStride
#include "netmsg.h"  // NetMsgPrimary, which the diplomacy bodies pop through
#include "netdaemon.h"  // NetDaemonNet, message_data - the net_* writers
#include "log.h"  // log_say, which the net_* writers announce each move with
#include "console.h"  // ConsoleGlobal, which net_energy refreshes
#include "socialwin.h"
#include "mapwin.h"
#include "alphamovie.h"
#include "strings.h"
#include "xpops.h"
#include "diplopop.h"
#include "infowin.h"
#include "reportif.h"
#include "random.h"
#include "replaywin.h"

void __cdecl draw_tile_fixup(int x, int y, int wider_radius, int redraw_flags);
void __cdecl mon_killed_faction(int faction, int killed_faction);
char *__cdecl get_him_her(int faction_id, int sentence_form);
void __cdecl interlude(int id, char *filename, int arg3, int arg4);
int __cdecl base_init(int faction, int x, int y);
void __cdecl base_kill(int base);
void __cdecl kill(int vehicle);
void __cdecl clear_bunglist(int faction);
int __cdecl veh_init(int prototype, int faction, int x, int y);
void __cdecl veh_kill(int vehicle);
int __cdecl veh_find(int x, int y, int faction, int exclude);
int __cdecl wants_prop(int faction, int proposal, int option);
static int diplo_add(int a, int b);
static int diplo_sub(int a, int b);
static int diplo_mul(int a, int b);
extern int DiploScratchFA98;  // Diplomacy producer storage defined below.

// ORIGINAL: 0x0053EB50 ?buy_council_vote@@YAXHHHH@Z 0x0053EB50-0x00540193;0x0065E75A-0x0065EB33
// RECOVERY_BINDING: v1 {"evidence": ["Pinned53EB50..540193 and65E75A..65EB33; source:src/unrecovered/0053eb50.cpp", "Native EDI=OTHER and EBX=FACTION; peer vote query precedes OTHER query; technology valuation rereads turn after every call; promised gift and treaty state reload after modal/parser callbacks. Popup teardown is ordinary lexical destruction, without duplicate close.", "RulesTechnology.name is the heap string address returned by Strings::put, not an ordinal; parse_say takes that address through its native32-bit integer ABI. Dialogue labels verified against bounded original strings and owned as C++ literals.", "recovery/tasks/interface-contract-report-closure.json; recovery/tests/test_council_vote_purchase.py actual-source corruption controls"], "reason": "Recover complete council vote purchasing in its genuine faction owner, including the custom modal negotiation missing from the retained seed. Use lexical Popup and source-owned labels, actual player/council/network storage and calls; preserve original spans, retained seed and all historical obligations. This is not matching or game acceptance."}
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
        x_pops_short("BUYPROMISED", 0x100000,
                     FactionDiplomacyPortraits[other], pop_wait);
        return;
    }

    if (PlayersData[other].soc_effect_base.economy > 0 && proposal > 1 &&
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
                x_pops_short("BUYNO", 0x100000,
                             FactionDiplomacyPortraits[other], pop_wait);
            }
            return;
        }

        if (margin < -3) {
            x_pops_short("BUYNO", 0x100000,
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
    const int minimum_self = static_cast<int>(PlayersData[faction].current_num_bases);
    if (spend_self < minimum_self) {
        spend_self = minimum_self;
    }
    const int minimum_other = static_cast<int>(PlayersData[other].current_num_bases);
    int spend_other = diplo_sub(PlayersData[other].unk_13,
                                static_cast<int>(PlayersData[other].maint_cost_total));
    if (spend_other < minimum_other) spend_other = minimum_other;

    votes = diplo_add(PlayersData[faction].energy_reserves / 5,
            diplo_mul(diplo_mul(diplo_add(spend_other, spend_self), votes), 2));
    if (DiploScratchFA98 != 0) votes /= 2;
    if (minimum_other <= DiploScratchFA98) votes /= 2;
    if (other == GovernorFaction) votes = diplo_mul(votes, 2);
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
            int turn_rounded = diplo_add(TurnCurrentNum, 7);
            int scale = diplo_add(turn_rounded, ((turn_rounded >> 31) & 7)) >> 3;
            have = diplo_mul(have, scale);
            if (have < need && DiploScratchFA18 >= 0) {
                int value = tech_val(DiploScratchFA18, other, 1);
                turn_rounded = diplo_add(TurnCurrentNum, 7);
                scale = diplo_add(turn_rounded, ((turn_rounded >> 31) & 7)) >> 3;
                have = diplo_add(have, diplo_mul(value, scale));
            } else {
                DiploScratchFA28 = -1;
                DiploScratchFA1C = -1;
                DiploScratchFA18 = -1;
            }
            if (have < need && DiploScratchFA1C >= 0) {
                int value = tech_val(DiploScratchFA1C, other, 1);
                turn_rounded = diplo_add(TurnCurrentNum, 7);
                scale = diplo_add(turn_rounded, ((turn_rounded >> 31) & 7)) >> 3;
                have = diplo_add(have, diplo_mul(value, scale));
            } else {
                DiploScratchFA28 = -1;
                DiploScratchFA1C = -1;
            }
            if (have < need && DiploScratchFA28 >= 0) {
                int value = tech_val(DiploScratchFA28, other, 1);
                turn_rounded = diplo_add(TurnCurrentNum, 7);
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
            x_pops_short(StringTemp, 0x100000,
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
            } else {
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
                parse_num(0, choice);
                if (DiploScratchFAA8 >= 0) {
                    parse_say(0, static_cast<int>(reinterpret_cast<intptr_t>(
                        Technology[DiploScratchFAA8].name)), -1, -1);
                    if (DiploScratchFA18 < 0) {
                    strcat(StringTemp, "1");
                    } else {
                        parse_say(1, static_cast<int>(reinterpret_cast<intptr_t>(
                            Technology[DiploScratchFA18].name)), -1, -1);
                        if (DiploScratchFA1C < 0) {
                            strcat(StringTemp, "2");
                        } else {
                            parse_say(4, static_cast<int>(reinterpret_cast<intptr_t>(
                                Technology[DiploScratchFA1C].name)), -1, -1);
                            if (DiploScratchFA28 >= 0) {
                                parse_say(5, static_cast<int>(reinterpret_cast<intptr_t>(
                                    Technology[DiploScratchFA28].name)), -1, -1);
                                strcat(StringTemp, "4");
                            } else {
                                strcat(StringTemp, "3");
                            }
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
                    log_say("Sending ENERGY", faction, other, delta);
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
                        log_say("Sending TECH", other, tech_id, faction);
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

static int lifecycle_wrap_x(int x) {
    if (!(MapIsFlat & 1)) {
        if (x < 0) x += MapLongitudeBounds;
        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
    }
    return x;
}

static uint8_t lifecycle_faction_mask(int faction) {
    return static_cast<uint8_t>(1u << (faction & 31));
}

static bool lifecycle_is_fresh(int conqueror) {
    return TurnCurrentNum == 0 || conqueror == -282;
}

static int lifecycle_distance(int x, int y, int other_x, int other_y) {
    int dx = abs(x - other_x);
    if (!(MapIsFlat & 1) && dx > static_cast<int>(MapLongitude)) dx = MapLongitudeBounds - dx;
    dx = abs(dx);
    const int dy = abs(y - other_y);
    const int hi = dx > dy ? dx : dy;
    const int lo = dx < dy ? dx : dy;
    return hi - ((((dx + dy) >> 1) - lo + 1) >> 1);
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
// ORIGINAL: 0x00455F10 ??0FactionArt@@QAE@XZ 0x00455F10-0x0045600B;0x00654ED0-0x00654F71 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retained seed src/recovered/units/00455f10.cpp supplies the original identity and both main/cleanup spans; complete original audit confirms the same member groups and their forward/reverse order.", "Original constructor455F10 and destructor456010 cover24 base Sprites,2 council logos,six individual Sprites,3 small report logos,2 report logos; current FactionArt declaration owns those objects with0x65C total extent.", "Original CRT6820B4 selects44CC50, which constructs8 elements at78E978 and registers44CC80 for reverse teardown. The compiled lifecycle is observed, but this binding is not a byte, exception-equivalence or linked-startup claim."], "reason": "Original FactionArt member construction/destruction has no additional application statements. Genuine C++ members own normal and partial-construction cleanup; the empty user body is not an unfinished placeholder."}
FactionArt::FactionArt() {}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00456010 ??1FactionArt@@QAE@XZ 0x00456010-0x004560FC;0x00654F80-0x00655021 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retained seed src/recovered/units/00456010.cpp supplies the original identity and both main/cleanup spans; complete original audit confirms the same member groups and their forward/reverse order.", "Original constructor455F10 and destructor456010 cover24 base Sprites,2 council logos,six individual Sprites,3 small report logos,2 report logos; current FactionArt declaration owns those objects with0x65C total extent.", "Original CRT6820B4 selects44CC50, which constructs8 elements at78E978 and registers44CC80 for reverse teardown. The compiled lifecycle is observed, but this binding is not a byte, exception-equivalence or linked-startup claim."], "reason": "Original FactionArt member construction/destruction has no additional application statements. Genuine C++ members own normal and partial-construction cleanup; the empty user body is not an unfinished placeholder."}
FactionArt::~FactionArt() {}

// CRT slot6820B4 selects44CC50: construct eight elements at78E978 with
// stride65C, then register44CC80 to destroy that same vector at exit.
// This independently bounds78E978..791C58; the last two members of each
// element occupy604..65C. Content loading and linked startup ordering
// remain unfinished; the compiler owns the new vector's lifetime.
// RECOVERY_OBJECT: v2 0x0078E978 0x00791C58 ?FactionsArt@@3PAUFactionArt@@A 0x00455F10
FactionArt FactionsArt[MaxPlayerNum];

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
// RECOVERY_OBJECT: v2 0x0076E590 0x0076E890 ?FactionBaseSprites@@3PAY2132PAVSprite@@A 0x00453A20
Sprite *FactionBaseSprites[MaxPlayerNum][2][4][3];

uint32_t RankingFactionIDUnk1;  // 0x009A650C
uint32_t RankingFactionIDUnk2;  // 0x009A6510
int DiploFriction;  // 0x0093FA74 // not always bounded, should it be 0-20?
uint32_t DiploFrictionFactionIDWith;  // 0x0093FABC
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
int PopupDialogFactionID;  // 0x0093F7CC
// The diplomacy flag word at 0x0074DA9C and the multiplayer-window gate at
// 0x007FE06C, both declared in faction.h beside the writers that drive them.
int DiploRedrawFlag;  // 0x0074DA9C
int MultiWindowOpen;  // 0x007FE06C

/*
Purpose: Determine if the specified faction is a Progenitor alien faction (Caretakers / Usurpers).
Original Offset: n/a
Return Value: Is faction a Progenitor? true/false
Status: Complete
*/
// BODY IN faction.h: six bodies call it where the image calls nothing.



/*
Purpose: Calculate the base amount of talents and drones for the specified faction.
// ORIGINAL: 0x004EA4A0 ?psych_check@@YAXHPAHPAH@Z 0x004EA4A0-0x004EA533
// LEVER: the image clamps efficiency to zero with a BRANCHLESS
//        `setl dl; dec edx; and edx, ecx` before adding 4 - splitting the
//        combined ternary `efficiency < 0 ? 4 : efficiency + 4` into
//        `(efficiency < 0 ? 0 : efficiency) + 4` isolates the classic
//        clamp-to-zero idiom and reaches it. Moved 21/52 (0.796) -> 25/52
//        (0.871).
// TRIED: the `test al,dl` vs `test dl,al` register-order-only divergence
//            on is_human()'s bit test is the same one already documented at
//            this file's `TRIED` notes near line 179/223 (register
//            allocation, not a source shape this tree controls).
// LEVER (2026-09-03): `static_cast<int>(MapAreaSqRoot)`. map.h declares it
//            `uint32_t`, so this tree's product was UNSIGNED and the whole
//            /56 //2 tail compiled to `mul`/`shr` against the image's
//            `imul`/`sar` - 13 divergent instructions, not a scheduling
//            difference. The cast (which technology.cpp's cost model already
//            makes) moved 33/52 -> 36/52. Naming the clamp `weight` before
//            the multiply tied at 36/52; the last two instructions are the
//            image forming `drones + 2` with `lea ecx,[eax+2]` where this
//            tree adds in place (`add eax,2`) - register selection only.
//            --all-flags: the default /O2 /Gy /GR- /Oy- /GX at 36/52 (0.914)
//            beats every other set tried (best rival 9/52).
// symbol    ?psych_check@@YAXHPAH0@Z
// size      147 bytes
// prototype void (__cdecl ?psych_check@@YAXHPAHPAH@Z)(int factionID, int* drones, int* talents)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl psych_check(int faction_id, int *drones, int *talents) {
    int drones_val = 6 - (is_human(faction_id) ? PlayersData[faction_id].diff_level : DLVL_LIBRARIAN);
    *drones = drones_val;
    int efficiency = PlayersData[faction_id].soc_effect_pending.efficiency;
    // Naming the clamp ties the inlined spelling at 36/52 - the image's
    // `lea ecx,[eax+2]` for drones + 2 is not source-reachable either way -
    // kept because it reads better than the nested ternary inline did.
    int weight = (efficiency < 0 ? 0 : efficiency) + 4;
    // MapAreaSqRoot is declared uint32_t in map.h; the image multiplies it as
    // a SIGNED value (imul/sar all the way through the /56), so the signedness
    // has to be recovered here or VC6 emits mul/shr and the whole /56 //2
    // tail changes shape. Same cast technology.cpp's cost model makes.
    *talents = ((drones_val + 2) * weight * static_cast<int>(MapAreaSqRoot)) / 56 / 2;
}

/*
Purpose: Check whether the primary faction has at least one of the specified treaties (bitfield) 
         with the secondary faction.
// ORIGINAL: 0x005002F0 ?has_treaty@@YAHHHH@Z 0x005002F0-0x00500319 BYTE_EXACT
// size      41 bytes
// prototype int (__cdecl ?has_treaty@@YAHHHH@Z)(int factionID, int factionIDWith, int treaty)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Treaty status between the two factions, generally treated as a boolean
Status: Complete
*/
// BODY IN faction.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Get the faction's name adjective.
// ORIGINAL: 0x0050B910 ?get_adjective@@YAHH@Z 0x0050B910-0x0050B92A BYTE_EXACT
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
// BODY IN faction.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Get the faction's noun.
// ORIGINAL: 0x0050B930 ?get_noun@@YAHH@Z 0x0050B930-0x0050B964 BYTE_EXACT
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
// BODY IN faction.h, as `MEASURED inline`: name_base (base.cpp) is the caller
// that needs it folded in place, matching the image.

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
    if (is_human(faction_id) && FactionRankings[7] == faction_id) {
        int bases_threat = (TurnCurrentNum + 25) / 50;
        if (bases_threat < 4) {
            bases_threat = 4;
        }
        if ((int)PlayersData[faction_id].current_num_bases > bases_threat
            && (PlayersData[faction_id].diff_level > DLVL_SPECIALIST
                || GameRules & RULES_INTENSE_RIVALRY || is_aggressive)) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Determine if the specified faction is considered a threat based on the game state and
         ranking.
// ORIGINAL: 0x00539C00 ?great_satan@@YAHHH@Z 0x00539C00-0x00539D3B
// LEVER: the image never calls great_beelzebub - its whole body is open-coded
//        here and the register holding GameRules/diff_level is reused past
//        the inline boundary. Wrote it inlined (is_minor_threat local) instead
//        of calling great_beelzebub(); moved from 1/113 (wrong shape, esp-frame,
//        real call) to 9/113 agreeing. TRIED: open-coding is_human's bit
//        test directly instead of the call did not move the tier further.
//        TRIED: `PlayerData *p = &PlayersData[faction_id];` before the
//        repeated `.current_num_bases`/`.diff_level` reads - dropped to 4/113;
//        the image keeps faction_id itself live across the whole body instead.
//        TRIED (2026-09-03): `bases_threat` was `uint32_t` here while
//        great_beelzebub's copy of the same arithmetic is `int` - the image's
//        compares are `jge`/`jle` and this tree emitted `jae`/`jbe`. Made it
//        `int` with `(int)PlayersData[...].current_num_bases` at its one use
//        (same lever great_beelzebub records): 23/113 -> 24/113. Splitting the
//        leading `!is_human || FactionRankings[7]` guard into two `if`s tied
//        at 24/113 - the image MERGES these two tails - so the chained form
//        stays. Rest is register selection (ebx vs ecx for TurnCurrentNum,
//        `lea ecx,[ebx+0x19]` vs `add ecx,0x19`, `mov al,[mem]` vs a dword
//        load for is_human's byte).
//        TRIED: the image never materializes an is_minor_threat flag - all
//        four guard conditions are separate early `return false` sites, no
//        variable. Rewrote as guard clauses instead of the flag+outer-if; ties
//        at 9/113 (identical codegen), kept for shape. Remaining divergence at
//        instruction 9 is `test byte ptr [mem],al` (image) vs `mov cl,[mem];
//        test cl,al` (this tree) for is_human's bit test - register/instruction
//        selection only, not chased per the no-register-only-diffs rule.
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
    if (!is_human(faction_id) || FactionRankings[7] != faction_id) {
        return false;
    }
    // Signed, as great_beelzebub's note records: the image's compares against
    // both bases_threat and current_num_bases are `jge`/`jle`, so the local is
    // `int` and the uint32_t field is cast at its one use.
    int bases_threat = (TurnCurrentNum + 25) / 50;
    if (bases_threat < 4) {
        bases_threat = 4;
    }
    if ((int)PlayersData[faction_id].current_num_bases <= bases_threat
        || !(PlayersData[faction_id].diff_level > DLVL_SPECIALIST
            || GameRules & RULES_INTENSE_RIVALRY || is_aggressive)) {
        return false;
    }
    BOOL has_intense_riv = (GameRules & RULES_INTENSE_RIVALRY);
    if (TurnCurrentNum <= ((has_intense_riv ? 0
        : (DLVL_TRANSCEND - PlayersData[faction_id].diff_level) * 50) + 100)) {
        return false;
    }
    if (climactic_battle() && aah_ooga(faction_id, -1) == faction_id) {
        return true;
    }
    uint32_t diff_factor;
    uint32_t factor;
    if (has_intense_riv) {
        factor = 4;
        diff_factor = DLVL_TRANSCEND;
    } else if (PlayersData[faction_id].diff_level >= DLVL_LIBRARIAN
        || GameRules & RULES_VICTORY_CONQUEST || ObjectiveReqVictory <= 1000) {
        factor = 2;
        diff_factor = DLVL_LIBRARIAN;
    } else {
        factor = 1;
        diff_factor = DLVL_TALENT;
    }
    return (factor * FactionRankingsUnk[FactionRankings[7]]
        >= diff_factor * FactionRankingsUnk[FactionRankings[6]]);
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
// symbol    ?aah_ooga@@YAIHH@Z
// size      254 bytes
// prototype 
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0052AD30 0x005B9F20
Return Value: Faction id if nearing diplomatic victory or zero
Status: Complete
*/
uint32_t __cdecl aah_ooga(int faction_id, int pact_faction_id) {
    if (!(GameRules & RULES_VICTORY_DIPLOMATIC)) {
        return 0; // Diplomatic Victory not allowed
    }
    int votes_total = 0;
    for (int i = 1; i < MaxPlayerNum; i++) {
        votes_total += council_votes(i);
    }
    uint32_t faction_id_ret = 0;
    for (int player = 1; player < MaxPlayerNum; player++) {
        int votes = council_votes(player);
        if (player != pact_faction_id
            && (!pact_faction_id || !has_treaty(player, pact_faction_id, DTREATY_PACT)
                || !(GameRules & RULES_VICTORY_COOPERATIVE))) {
            int proposal_preq = Proposal[PROP_UNITE_SUPREME_LEADER].preq_tech;
            // has_tech_call, not has_tech: the image CALLS 0x005B9F20 at all
            // three of these sites (0x00539DC2/0x00539DE3/0x00539DF7) rather
            // than open-coding the preq walk, so this is a best_specialist
            // caller, not one of the 108 genuine inline sites.
            if ((has_tech_call(proposal_preq, faction_id)
                || (proposal_preq >= 0
                    && (has_tech_call(Technology[proposal_preq].preq_tech_1, faction_id)
                    || has_tech_call(Technology[proposal_preq].preq_tech_2, faction_id))))
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
        if (is_human(i) && PlayersData[i].corner_market_turn > TurnCurrentNum) {
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
            if (is_human(player) && (has_tech_call(Facility[FAC_PSI_GATE].preq_tech, player)
                || has_tech_call(Facility[FAC_VOICE_OF_PLANET].preq_tech, player))) {
                return true; // Human controlled player has tech to build PSI Gates or VoP
            }
        }
    }
    return false;
}

// Non-inline forwarder to base.h's `MEASURED inline` ascending: at_climax
// below is one of the image's four real callers of 0x005AC680 (see that
// marker's `calls` line), where facility_avail (base.cpp) folds it in place
// instead. Same idiom as technology.h's has_tech_call - the E8 target is a
// relocation on both sides, so this symbol's own name costs nothing.
BOOL __cdecl ascending_call(int faction_id) {
    return ascending(faction_id);
}

/*
Purpose: Determine if the specified AI faction is at the game climax based on certain conditions.
// ORIGINAL: 0x00539EF0 ?at_climax@@YAHH@Z 0x00539EF0-0x0053A022
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
    if (is_human(faction_id) || GameState & STATE_UNK_1 || DiffLevelCurrent == DLVL_CITIZEN
        || !climactic_battle()) {
        return false;
    }
    if (aah_ooga(faction_id, faction_id)) {
        return true;
    }
    for (uint32_t i = 1; i < MaxPlayerNum; i++) {
        if (i != faction_id && PlayersData[i].corner_market_turn > TurnCurrentNum
            && (!has_treaty(faction_id, i, DTREATY_PACT)
                || !(GameRules & RULES_VICTORY_COOPERATIVE))) {
            return true;
        }
    }
    int trans_most_min_them = 0;
    int trans_most_min_us = 0;
    for (int player = 0; player < BaseCurrentCount; player++) {
        if (Bases[player].queue_production_id[0] == -FAC_ASCENT_TO_TRANSCENDENCE) {
            int min_accum = Bases[player].minerals_accumulated;
            if (Bases[player].faction_id_current == faction_id) {
                if (trans_most_min_us <= min_accum) {
                    trans_most_min_us = min_accum;
                }
            } else if (trans_most_min_them <= min_accum) {
                trans_most_min_them = min_accum;
            }
        }
    }
    for (i = 1; i < MaxPlayerNum; i++) {
        if (i != faction_id) {
            // ascending_call, not ascending: the image CALLS 0x005AC680 at
            // both of these sites (see the calls line), where base.h's
            // MEASURED inline would fold base_project in place.
            if (ascending_call(i) && !ascending_call(faction_id)) { // both return same so irrelevant check
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
void __cdecl cause_friction(int faction_id, int faction_id_with, int friction) {
    // diplo_friction is the same 2099-stride view sixteen dwords past the
    // treaty word - SpyingStatusTable[index + 16], exactly the +8 spelling
    // treaty_off uses for the agenda view. One index local reaches both the
    // read and the write, which is what the image keeps in a single register.
    int index = faction_id_with + faction_id * SpyingStatusStride;
    SpyingStatusTable[index + 16] = range(SpyingStatusTable[index + 16] + friction, 0, 20);
    if (DiploFrictionFactionID == faction_id && DiploFrictionFactionIDWith == faction_id_with) {
        DiploFriction += friction; // not bounded?
    }
}

/*
Purpose: Normalize the diplomatic friction value into a mood offset.
// ORIGINAL: 0x0053A090 ?get_mood@@YAHH@Z 0x0053A090-0x0053A0F5 BYTE_EXACT
// symbol    ?get_mood@@YAIH@Z
// size      101 bytes
// prototype int (__cdecl ?get_mood@@YAHH@Z)(int friction)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Mood (0-8)
Status: Complete
*/
uint32_t __cdecl get_mood(int friction) {
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
int __cdecl reputation(int faction_id, int faction_id_with) {
    return range(PlayersData[faction_id].integrity_blemishes
        - PlayersData[faction_id].diplo_unk1[faction_id_with], 0, 99);
}

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
// ORIGINAL: 0x0053A4A0 ?scan_prototypes@@YAXHH@Z 0x0053A4A0-0x0053A774
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
        VehPrototype &proto = VehPrototypes[proto_id];
        if (!(proto.flags & PROTO_ACTIVE) || !(proto.flags & PROTO_TYPED_COMPLETE)) {
            continue;
        }
        if (proto.obsolete_factions & (1 << faction_id_with)) {
            continue;
        }
        int value = 3 * Weapon[proto.weapon_id].offense_rating
            + 4 * (((7 - proto.plan) << 14) + Armor[proto.armor_id].defense_rating)
            + Chassis[proto.chassis_id].speed;
        int unmatched = 1;
        for (int j = 0; j < MaxVehProtoFactionNum && unmatched; j++) {
            VehPrototype &rival = VehPrototypes[faction_id * MaxVehProtoFactionNum + j];
            if (!(rival.flags & PROTO_ACTIVE) || !(rival.flags & PROTO_TYPED_COMPLETE)) {
                continue;
            }
            if (rival.obsolete_factions & (1 << faction_id)) {
                continue;
            }
            if (rival.plan != proto.plan) {
                continue;
            }
            if (Chassis[rival.chassis_id].triad != Chassis[proto.chassis_id].triad) {
                continue;
            }
            if (rival.reactor_id > proto.reactor_id) {
                unmatched = 0;
            }
            if (Chassis[rival.chassis_id].speed > Chassis[proto.chassis_id].speed) {
                unmatched = 0;
            }
            // Both arm_strat calls read the weapon id; see the note above.
            int rival_weapon = weap_strat(rival.weapon_id, faction_id);
            int proto_weapon = weap_strat(proto.weapon_id, faction_id);
            int rival_armor = arm_strat(rival.weapon_id, faction_id);
            int proto_armor = arm_strat(proto.weapon_id, faction_id);
            if (Chassis[rival.chassis_id].speed >= Chassis[proto.chassis_id].speed
                && rival_weapon >= proto_weapon && rival_armor >= proto_armor
                && (rival_weapon > proto_weapon || rival_armor > proto_armor)) {
                unmatched = 0;
            }
        }
        int trade_value = unmatched ? value : (value >> 4);
        if (trade_value > best_trade_value) {
            best_trade_value = trade_value;
            BestProtoForTrade = proto_id;
        }
        uint32_t treaty = PlayersData[faction_id].diplo_treaties[faction_id_with];
        if (treaty & DTREATY_PACT) {
            if (proto.flags & PROTO_UNK_20) {
                continue;
            }
        } else if (proto.flags & (PROTO_UNK_10 | PROTO_UNK_20)) {
            continue;
        }
        if (proto.plan > PLAN_NAVAL_SUPERIORITY || proto.plan == PLAN_RECONNAISANCE) {
            continue;
        }
        if (!unmatched) {
            value = ((treaty & DTREATY_VENDETTA) && proto.plan == PLAN_DEFENSIVE)
                ? (value >> 4) : 0;
        }
        if (value > best_mention_value) {
            best_mention_value = value;
            BestProtoToMention = proto_id;
        }
    }
}

/*
Purpose: Clear `treaty` in the diplomatic status word between two factions, both
         directions of the pair, and take the side effects the clear implies:
         clearing any of the three low bits also clears 0x1100000, clearing
         VENDETTA also clears 0x8040, and clearing WANT_REVENGE also clears
         SHALL_BETRAY in the +8 view of the same table.
// ORIGINAL: 0x0055B760 ?treaty_off@@YAXHHH@Z 0x0055B760-0x0055B818 BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0055b760.cpp, whose claim proved the
//         ARTIFACT reproduced while the shipped program contained nothing - the
//         same promotion set_state_id made beside CheckBox::get_state_id. The
//         artifact's two raw table bindings are SpyingStatusTable and the same
//         table eight uint32_t elements further in, which is
//         SpyingStatusTable[index + 8] - the view do_check reads for its
//         0x80000000-id boxes, and the fold lands on the same displacement.
// size      184 bytes
// prototype void (__cdecl ?treaty_off@@YAXHHH@Z)(int faction_id, int faction_id_with, int treaty)
// callers   14   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl treaty_off(int faction_id, int faction_id_with, int treaty) {
    if (treaty & 7) {
        treaty |= 0x1100000;
    }
    if (treaty & DTREATY_VENDETTA) {
        treaty |= 0x8040;
    }
    if (treaty & DTREATY_WANT_REVENGE) {
        // 0xfffff3ff == ~(SHALL_BETRAY | 0x800), and in the +8 view: dropping
        // WANT_REVENGE drops that pair as well.
        int index = faction_id_with + faction_id * SpyingStatusStride;
        SpyingStatusTable[index + 8] &= 0xfffff3ff;
        index = faction_id + faction_id_with * SpyingStatusStride;
        SpyingStatusTable[index + 8] &= 0xfffff3ff;
    }
    int index = faction_id_with + faction_id * SpyingStatusStride;
    SpyingStatusTable[index] &= ~treaty;
    index = faction_id + faction_id_with * SpyingStatusStride;
    SpyingStatusTable[index] &= ~treaty;
}

/*
Purpose: Set `treaty` in the diplomatic status word between two factions, both
         directions of the pair, and take the side effects the set implies:
         a PACT carries TREATY with it, a vendetta declared on top of a live
         pact/treaty/truce zeroes both directions of the third status view and
         clears a block of status bits in both directions, and either faction
         being the local one asks for the multiplayer window to be redrawn.
// ORIGINAL: 0x0055B870 ?treaty_on@@YAXHHH@Z 0x0055B870-0x0055BA76
// LEVER: the local faction is a CACHED LOCAL, not a global read at each use -
// `int local_faction = LocalFaction;` at entry plus two re-reads (after the
// vendetta-table clearing, after the ambience call) is what puts the image's
// `mov ecx,[0x939284]` at instruction 2 and keeps 94/152 instructions
// agreeing. Reading LocalFaction at each use dropped it to 15/152, because
// VC6 then loads the global lazily inside the first block.
// TRIED: naming `index` at the TOP of the `treaty & 7` block (the treaty_off
//            spelling) so the stores could use scaled addressing - VC6 then
//            hoists the whole lea chain ABOVE the two LocalFaction compares
//            and the match falls to 12/152. The image computes the index
//            between the compares and the clear, so the first block spells
//            the expression inline in the `if` and names `index` only for the
//            two clears.
// RESIDUAL: 13 divergent instructions. Ten are one cluster the source cannot
//            reach from here: the vendetta block wants its index scaled ONCE -
//            `shl eax,2` then `[eax + 0x96c9f8]` and `[eax + 0x96cb8c]` for
//            the byte test, the vendetta-table store and the status load - and
//            one `index` local reaching two different table bases compiles as
//            a SIB `[eax*4 + base]` per access instead, plus a
//            `lea eax,[eax*4+..]; mov [eax],edx` for the last
//            `&= 0xfaeef378`. The three after that are register choice
//            (eax vs edx) in the two final lea chains. The archived unit bound
//            both tables as `extern int g_0096c9f8_tbl[]` arrays rather than
//            folded `uint32_t *const` pointers, which may be what buys the
//            shared scaling - untested, since binding those addresses as real
//            arrays means defining storage the tree does not own.
// TRIED: three further flag sets under --all-flags: /O2 without /Oy- scores
//            26/152, /O1 7/152 - the default /O2 /Gy /GR- /Oy- /GX at 0.993
//            similar is the winner.
// size      518 bytes
// prototype void (__cdecl ?treaty_on@@YAXHHH@Z)(int faction_id, int faction_id_with, int treaty)
// callers   33   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00446B40 0x005D5A70
Return Value: n/a
Status: Complete
*/
void __cdecl treaty_on(int faction_id, int faction_id_with, int treaty) {
    // The local faction, cached for the pair tests and re-read after each
    // store or call that could alias it - the image keeps it in ecx and does
    // exactly two reloads, one after the vendetta-table clearing and one
    // after the ambience call.
    int local_faction = ConsoleGlobal.faction_id_;
    if (treaty & 7) {
        if ((faction_id == local_faction || faction_id_with == local_faction)
            && (SpyingStatusTable[faction_id_with
                                  + faction_id * SpyingStatusStride]
                & DTREATY_VENDETTA)) {
            // ~1 on a bit-0 flag: the request goes away here.
            DiploRedrawFlag &= ~1;
        }
        // ~0x118050: VENDETTA, both UNK_40/UNK_8000 and the 0x1100000 pair -
        // the same bits treaty_off adds to its mask for the same three low
        // bits, cleared here rather than or'ed on.
        int index = faction_id_with + faction_id * SpyingStatusStride;
        SpyingStatusTable[index] &= 0xffee7faf;
        index = faction_id + faction_id_with * SpyingStatusStride;
        SpyingStatusTable[index] &= 0xffee7faf;
    }
    if (treaty & DTREATY_PACT) {
        treaty |= DTREATY_TREATY;
    }
    if (treaty & DTREATY_VENDETTA) {
        int index = faction_id_with + faction_id * SpyingStatusStride;
        if (SpyingStatusTable[index] & 7) {
            DiploVendettaTable[faction_id + faction_id_with
                               * SpyingStatusStride] = 0;
            DiploVendettaTable[index] = 0;
            local_faction = ConsoleGlobal.faction_id_;
        }
        treaty |= 0x80000108;
        SpyingStatusTable[index] &= 0xfaeef378;
        SpyingStatusTable[faction_id + faction_id_with * SpyingStatusStride]
            &= 0xfaeef378;
        if (faction_id == local_faction
            || faction_id_with == local_faction) {
            DiploRedrawFlag |= 1;
            ambience(0xcc);
            local_faction = ConsoleGlobal.faction_id_;
        }
    }
    if (treaty & 0x17) {
        int index = faction_id_with + faction_id * SpyingStatusStride;
        SpyingStatusTable[index] &= 0xff7fffff;
        index = faction_id + faction_id_with * SpyingStatusStride;
        SpyingStatusTable[index] &= 0xff7fffff;
    }
    int index = faction_id_with + faction_id * SpyingStatusStride;
    SpyingStatusTable[index] |= treaty;
    index = faction_id + faction_id_with * SpyingStatusStride;
    SpyingStatusTable[index] |= treaty;
    if ((faction_id == local_faction || faction_id_with == local_faction)
        && (treaty & 0x1f) && MultiWindowOpen) {
        reinterpret_cast<GraphicWin *>(&MultiWindow)->GraphicWin::redraw();
    }
}

/*
Purpose: Set `agenda` in the diplomatic agenda word between two factions, both
         directions of the pair; setting the 0x20 agenda bit also clears the
         0x40 bit beside it.
// ORIGINAL: 0x0055BA80 ?agenda_on@@YAXHHH@Z 0x0055BA80-0x0055BB29 BYTE_EXACT
// LEVER: the agenda view is `SpyingStatusTable[index + 8]`, exactly
//         treaty_off's spelling for the +8 view, and `index` is declared
//         INSIDE each block the way treaty_off declares it - one `int index`
//         per block, reassigned for the second direction. Declaring it before
//         the `agenda & 0x20` test hoisted the index chain above the test and
//         cost 49 of 56 instructions; the image computes it inside the branch.
// size      169 bytes
// prototype void (__cdecl ?agenda_on@@YAXHHH@Z)(int faction_id, int faction_id_with, int agenda)
// callers   8   call targets   0
// kind      game
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl agenda_on(int faction_id, int faction_id_with, int agenda) {
    // The agenda view is the same table eight dwords in - 0x96CA18, which is
    // PlayerData::diplo_agenda[0].
    if (agenda & 0x20) {
        int index = faction_id_with + faction_id * SpyingStatusStride;
        SpyingStatusTable[index + 8] &= ~0x40;
        index = faction_id + faction_id_with * SpyingStatusStride;
        SpyingStatusTable[index + 8] &= ~0x40;
    }
    int index = faction_id_with + faction_id * SpyingStatusStride;
    SpyingStatusTable[index + 8] |= agenda;
    index = faction_id + faction_id_with * SpyingStatusStride;
    SpyingStatusTable[index + 8] |= agenda;
}

/*
Purpose: Clear `agenda` in the diplomatic agenda word between two factions,
         both directions of the pair. agenda_on's mirror, without its 0x20/0x40
         interaction - the caller has already decided which bit it wants gone.
// ORIGINAL: 0x0055B820 ?agenda_off@@YAXHHH@Z 0x0055B820-0x0055B870 BYTE_EXACT
// LEVER: `int mask = ~agenda;` FIRST - naming the complement up front is what
//         makes VC6 emit the image's `not edx` before the index arithmetic and
//         keeps the mask in a register for both directions. Writing
//         `SpyingStatusTable[index + 8] &= ~agenda` twice deferred the `not`
//         past the first `push edi` and diverged at instruction 5 (18/30).
// size      80 bytes
// prototype void (__cdecl ?agenda_off@@YAXHHH@Z)(int faction_id, int faction_id_with, int agenda)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl agenda_off(int faction_id, int faction_id_with, int agenda) {
    int mask = ~agenda;
    int index = faction_id_with + faction_id * SpyingStatusStride;
    SpyingStatusTable[index + 8] &= mask;
    index = faction_id + faction_id_with * SpyingStatusStride;
    SpyingStatusTable[index + 8] &= mask;
}

/*
Purpose: Set or unset the diplomatic treaty for the specified faction with another faction.
// ORIGINAL: 0x0055BB30 ?set_treaty@@YAXHHHH@Z 0x0055BB30-0x0055BB98
// LEVER: naming the diplo_treaties element's address in a local
//            `uint32_t *treaty_slot` and reaching diplo_merc through it via
//            `(char *)treaty_slot + (offsetof(diplo_merc)-offsetof(diplo_treaties))`
//            moved 15/36 -> 27/36 agreeing (0.933 similar) over the un-named
//            `&PlayersData[...].diplo_treaties[...]` spelling, which ties the
//            original 15/36. Remaining divergence: the image's `set`-branch
//            materialises the index as a byte offset via one `shl eax,2` and
//            reuses plain `[eax+base]` addressing for the load/store/second
//            field; this tree's /O2 always folds `*4` into `[eax*4+base]`
//            scaled addressing instead, and the `else` branch RMW does not
//            fold to the image's single `and dword ptr [mem], reg`.
// TRIED: pointer to diplo_treaties field (shared or per-branch); local index var;
//            unsigned cast on index; explicit RMW temp; if-condition hoisted before the |=.
//            Set branch: image computes the index via explicit `shl eax,2` then reuses
//            the plain [eax+base] address for both the load and the store; this tree's
//            compile always folds the *4 into the addressing mode instead. Plateaus at
//            15/36 agreeing across all tried flag sets.
//            TRIED (2026-08-22): a `uint32_t *diplo_treaties` pointer bound to the
//            ROW (`PlayersData[faction_id].diplo_treaties`, no `[faction_id_with]` yet,
//            indexed at each of the three uses) - the lever that took cause_friction
//            13/33 -> 21/33. Here it is WORSE, not a tie: drops to 0/36. This is a
//            single read-modify-write per branch (not cause_friction's read-then-separate-
//            write of the same element), so there is no shared address to amortise.
//            TRIED (this pass) three spellings of the `shl eax,2`-once shape,
//            all measured and rejected: (1) the flat view - `int index` local
//            plus `SpyingStatusTable[index]` / `SpyingStatusTable[index + 32]`
//            for the merc store, the treaty_off/agenda_on idiom - falls to
//            15/36 because with three accesses at one base VC6 folds *4 into
//            every SIB and never scales eagerly; (2) a byte-granular slot
//            (`char *treaty_slot`) - ties the 32/36 baseline but the address
//            materialises as `lea eax,[eax*4+0x96c9f8]`, the full address,
//            not the image's base-less `shl eax,2`; (3) the byte offset added
//            inline at the use (`(char *)&SpyingStatusTable[index] + 0x80`)
//            - 15/36, the whole chain folds into two SIB accesses and the
//            compile is one instruction SHORTER (35). The image's register
//            holds 4*index with the table base folded into three separate
//            displacements, which is a scheduling choice VC6 6.0 does not
//            reach from any of these source forms; the 32/36 spelling kept
//            below is the closest of the four.
// size      104 bytes
// prototype 
// callers   24   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl set_treaty(int faction_id, int faction_id_with, int treaty, BOOL set) {
    if (set) {
        uint32_t *treaty_slot = &PlayersData[faction_id].diplo_treaties[faction_id_with];
        *treaty_slot |= treaty;
        if (treaty & DTREATY_UNK_40) {
            *(int *)((char *)treaty_slot
                     + (offsetof(PlayerData, diplo_merc) - offsetof(PlayerData, diplo_treaties))) = 50;
        }
    } else {
        PlayersData[faction_id].diplo_treaties[faction_id_with] &= ~treaty;
    }
}

/*
Purpose: Set or unset the diplomatic agenda for the specified faction with another faction.
// ORIGINAL: 0x0055BBA0 ?set_agenda@@YAXHHHH@Z 0x0055BBA0-0x0055BBF1 BYTE_EXACT
// size      81 bytes
// prototype void (__cdecl ?set_agenda@@YAXHHHH@Z)(int factionID, int factionIDWith, int agenda, int set)
// callers   10   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl set_agenda(int faction_id, int faction_id_with, int agenda, BOOL set) {
    if (set) {
        PlayersData[faction_id].diplo_agenda[faction_id_with] |= agenda;
    } else {
        PlayersData[faction_id].diplo_agenda[faction_id_with] &= ~agenda;
    }
}

/*
Purpose: Check whether the primary faction has at least one of the specified agendas (bitfield)
         with the secondary faction.
// ORIGINAL: 0x005591E0 ?has_agenda@@YAHHHH@Z 0x005591E0-0x00559209 BYTE_EXACT
// size      41 bytes
// prototype int (__cdecl ?has_agenda@@YAHHHH@Z)(int factionID, int factionIDWith, int agenda)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Agenda status between the two factions, generally treated as a boolean
Status: Complete
*/
int __cdecl has_agenda(int faction_id, int faction_id_with, int agenda) {
    return PlayersData[faction_id].diplo_agenda[faction_id_with] & agenda;
}

/*
Purpose: Determine if the specified faction want to attack the target faction.
// ORIGINAL: 0x0055BC80 ?wants_to_attack@@YAHHHH@Z 0x0055BC80-0x0055C42B
// LEVER: the image's `sub esp, 0x134` (308 bytes) did not fit the committed
//        `region_top_base_count[8]` (32 bytes) plus the ~52 bytes of scalar
//        locals seen elsewhere in the frame - 256 + 52 = 308 only if the
//        array is sized MaxRegionLandNum (64), matching a stray copy-paste
//        of the outer loop bound into the array size. Resized the array (see
//        the BUG IN THE ORIGINAL comment at its declaration) and dropped the
//        prior `= {0}` zero-init the same comment had added, since the image
//        never zeroes it either - moved this tree's own frame from 0x58 to
//        0x138 (4 bytes over the image's 0x134, not chased). Not otherwise
//        landed close to byte-exact: this is a 650-instruction function and
//        a near-total register-role swap (faction_id in edi vs this tree's
//        ebx) cascades from instruction 4 onward: MISMATCH, left for a
//        dedicated pass. call_diff agrees (0x0050BA00/great_beelzebub/
//        great_satan call counts all match).
// size      1963 bytes
// prototype int (__cdecl ?wants_to_attack@@YAHHHH@Z)(uint32_t factionID, uint32_t factionIDTarget, int factionIDUnk)
// callers   9   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00539B70 0x00539C00
Return Value: Does the faction want to attack target? true/false
Status: Complete - testing
*/
BOOL __cdecl wants_to_attack(int faction_id, int faction_id_tgt, int faction_id_unk) {
    uint32_t peace_faction_id = 0;
    BOOL unk_tgl = false;
    if (Players[faction_id].rule_flags & RFLAG_ALIEN
        && Players[faction_id_tgt].rule_flags & RFLAG_ALIEN) {
        return true;
    }
    if (has_treaty(faction_id, faction_id_tgt,
        DTREATY_WANT_REVENGE | DTREATY_UNK_40 | DTREATY_ATROCITY_VICTIM)) {
        return true;
    }
    if (PlayersData[faction_id_tgt].major_atrocities && PlayersData[faction_id].major_atrocities) {
        return true;
    }
    if (has_treaty(faction_id, faction_id_tgt, DTREATY_UNK_4000000)) {
        return false;
    }
    if (PlayersData[faction_id].current_num_bases <= 1) {
        return false;
    }
    if (!is_human(faction_id_tgt) && PlayersData[faction_id].flags & PFLAG_TEAM_UP_VS_HUMAN) {
        return false;
    }
    uint32_t want_to_attack = 0;
    for (uint32_t i = 1; i < MaxPlayerNum; i++) {
        if (i != faction_id && i != faction_id_tgt) {
            if (has_treaty(faction_id, i, DTREATY_HAVE_SURRENDERED | DTREATY_PACT)) {
                peace_faction_id = i;
            }
            if (has_treaty(faction_id, i, DTREATY_VENDETTA)
                && !has_treaty(faction_id_tgt, i, DTREATY_PACT)) {
                want_to_attack++;
                if (PlayersData[i].mil_strength_1 
                    > ((PlayersData[faction_id_tgt].mil_strength_1 * 3) / 2)) {
                    want_to_attack++;
                }
            }
            if (great_beelzebub(i, false)
                && (TurnCurrentNum >= 100 || GameRules & RULES_INTENSE_RIVALRY)) {
                if (has_treaty(faction_id_tgt, i, DTREATY_VENDETTA)) {
                    want_to_attack++;
                }
                if (has_treaty(faction_id_tgt, i, DTREATY_COMMLINK) 
                    && has_treaty(faction_id, i, DTREATY_COMMLINK)) {
                    want_to_attack++;
                }
            }
            if (has_treaty(faction_id, i, DTREATY_PACT) && is_human(i)) {
                if (has_treaty(faction_id_tgt, i, DTREATY_PACT)) {
                    want_to_attack += 2;
                }
                BOOL has_surrender = has_treaty(faction_id, i, DTREATY_HAVE_SURRENDERED);
                if (has_surrender && has_treaty(i, faction_id_tgt, DTREATY_PACT | DTREATY_TREATY)) {
                    return false;
                }
                if (has_treaty(faction_id_tgt, i, DTREATY_VENDETTA)) {
                    unk_tgl = true;
                    want_to_attack -= (has_surrender ? 4 : 2);
                }
            }
        }
    }
    if (peace_faction_id) {
        if (has_treaty(faction_id_tgt, peace_faction_id, DTREATY_VENDETTA)) {
            return true;
        }
        if (has_treaty(faction_id_tgt, peace_faction_id, DTREATY_PACT | DTREATY_TREATY)) {
            return false;
        }
    }
    if (PlayersData[faction_id].ai_fight < 0 && !unk_tgl && FactionRankings[7] != faction_id_tgt) {
        return false;
    }
    // BUG IN THE ORIGINAL: this array is sized by MaxRegionLandNum (matching the stack frame's
    // 0x134-byte allocation) even though only indices 1..MaxPlayerNum-1 are ever written by the
    // loop below, and it is never zero-initialized - reproduced here rather than fixed, since the
    // shipped code reads whatever was on the stack for region_top_base_count[0] and any faction_id
    // that is compared before this array is populated for that index.
    uint32_t region_top_base_count[MaxRegionLandNum];
    for (int region = 1; region < MaxRegionLandNum; region++) {
        for (uint32_t f = 1; f < MaxPlayerNum; f++) {
            uint32_t total_bases = PlayersData[f].region_total_bases[region];
            if (total_bases > region_top_base_count[f]) {
                region_top_base_count[f] = total_bases;
            }
        }
    }
    for (uint32_t f = 1; f < MaxPlayerNum; f++) {
        region_top_base_count[f] -= (region_top_base_count[f] / 4);
    }
    int region_target_hq = -1;
    int region_hq = -1;
    for (int player = 0; player < BaseCurrentCount; player++) {
        if (has_fac_built(FAC_HEADQUARTERS, player)) {
            uint32_t base_faction = Bases[player].faction_id_current;
            if (base_faction == faction_id) {
                region_hq = region_at(Bases[player].x, Bases[player].y);
            } else if (base_faction == faction_id_tgt) {
                region_target_hq = region_at(Bases[player].x, Bases[player].y);
            }
        }
    }
    uint32_t factor_force_rating = 0;
    uint32_t factor_count = 0;
    uint32_t factor_unk = 1;
    for (region = 1; region < MaxRegionLandNum; region++) {
        if (!bad_reg(region)) {
            uint32_t force_rating = PlayersData[faction_id].region_force_rating[region];
            if (force_rating) {
                uint32_t total_cmbt_vehs 
                    = PlayersData[faction_id_tgt].region_total_combat_vehs[region];
                uint32_t total_bases_tgt = PlayersData[faction_id_tgt].region_total_bases[region];
                if (total_cmbt_vehs || total_bases_tgt) {
                    if (PlayersData[faction_id].region_total_bases[region] 
                        >= ((region_top_base_count[faction_id] / 4) * 3) || region == region_hq) {
                        uint32_t compare = force_rating +
                            PlayersData[faction_id].region_total_combat_vehs[region] +
                            (faction_id_unk > 0 
                                ? PlayersData[faction_id_unk].region_force_rating[region] / 4 : 0);
                        if (PlayersData[faction_id_tgt].region_force_rating[region] > compare) {
                            return false;
                        }
                    }
                    if (total_bases_tgt) {
                        factor_force_rating += force_rating + (faction_id_unk > 0
                            ? PlayersData[faction_id_unk].region_force_rating[region] / 2 : 0);
                    }
                    if ((total_bases_tgt >= ((region_top_base_count[faction_id_tgt] / 4) * 3)
                        || region == region_target_hq) && force_rating > total_cmbt_vehs) {
                        factor_force_rating += force_rating + (faction_id_unk > 0
                            ? PlayersData[faction_id_unk].region_force_rating[region] / 2 : 0);
                    }
                    factor_unk += total_cmbt_vehs 
                        + PlayersData[faction_id].region_total_bases[region]
                        ? PlayersData[faction_id_tgt].region_force_rating[region] / 2 : 0;
                    if (PlayersData[faction_id].region_total_bases[region]) {
                        factor_count++;
                    }
                }
            }
        }
    }
    want_to_attack -= PlayersData[faction_id].ai_fight * 2;
    int tech_comm_bonus = PlayersData[faction_id].tech_commerce_bonus;
    int tech_comm_bonus_target = PlayersData[faction_id_tgt].tech_commerce_bonus;
    if (tech_comm_bonus > ((tech_comm_bonus_target * 3) / 2)) {
        want_to_attack++;
    }
    if (tech_comm_bonus < ((tech_comm_bonus_target * 2) / 3)) {
        want_to_attack--;
    }
    int best_armor_target = PlayersData[faction_id_tgt].best_armor_value;
    int best_weapon = PlayersData[faction_id_tgt].best_weapon_value;
    if (best_weapon > (best_armor_target * 2)) {
        want_to_attack--;
    }
    if (best_weapon <= best_armor_target) {
        want_to_attack++;
    }
    if (!has_treaty(faction_id, faction_id_tgt, DTREATY_VENDETTA)) {
        want_to_attack++;
    }
    if (!has_treaty(faction_id, faction_id_tgt, DTREATY_PACT)) {
        want_to_attack++;
    }
    if (faction_id_unk > 0 && !great_satan(faction_id_unk, false)) {
        want_to_attack--;
    }
    if (has_agenda(faction_id, faction_id_tgt, DAGENDA_UNK_200)
        && GameRules & RULES_INTENSE_RIVALRY) {
        want_to_attack--;
    }
    want_to_attack -= range((PlayersData[faction_id_tgt].integrity_blemishes 
        - PlayersData[faction_id].integrity_blemishes + 2) / 3, 0, 2);
    int morale_factor = range(PlayersData[faction_id].soc_effect_pending.morale, -4, 4) 
        + Players[faction_id].rule_morale + 16;
    int morale_factor_target 
        = range(PlayersData[faction_id_tgt].soc_effect_pending.morale, -4, 4)
        + Players[faction_id_tgt].rule_morale + 16;
    if ((factor_count || want_to_attack > 0
        || has_treaty(faction_id, faction_id_tgt, DTREATY_UNK_20000000)) &&
        ((morale_factor * factor_force_rating * 6) / (morale_factor_target * factor_unk)) 
        < (want_to_attack + 6)) {
        return false;
    }
    return true;
}

/*
Purpose: Weigh how badly the specified faction's territory is being trespassed on by another
         faction's units, and mark those units so the rest of diplomacy can find them again.
// ORIGINAL: 0x0055EB80 ?territory@@YAHHHHPAHPAH@Z 0x0055EB80-0x0055EEDE
// TRIED: `call_diff` already agrees (0 disagree). MISMATCH is 15/278 at
// this pass, first divergence right in the prologue (frame size: image
// `sub esp, 0x118` vs this tree's `sub esp, 0x11c`, one extra dword of
// locals) - a stack-layout gap across an 862-byte body, not chased at the
// instruction level this pass; outside this batch's budget. Semantically
// verified in the Purpose block below (kept where it was written, not
// re-derived here).
// symbol    ?territory@@YAHHHHPAH0@Z
// size      862 bytes
// prototype
// callers   4   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0 0x00644F3A
Return Value: Trespass weight; zero when there is nothing to complain about
Status: Complete

The two factions are asymmetric. faction_id owns the ground, faction_id_with owns the units,
and every treaty read is PlayersData[faction_id].diplo_treaties[faction_id_with] - the owner's
view of the trespasser, never the reverse. A pact or a vendetta makes the question moot and
returns zero before anything is walked, and DTREATY_UNK_200 does the same unless bit 1 of
flags overrides it, which is how the caller asks anyway.

flags is two independent bits and neither is a mode:
  - bit 0 arms everything after the raw count: the distance test, the count_out tally, the two
    unit flag writes, and the weighting below. Without it the function only counts and marks.
  - bit 1 suppresses the DTREATY_UNK_200 early return.

region_shared is the "we are both fighting the same faction there" exemption, and it is
computed per land region before any unit is looked at. A region is exempt when some third
faction that BOTH sides hold a vendetta against has at least one base in it: the trespasser is
plausibly there for the war rather than for the land. Faction zero - the native life faction -
is never that third party, because the scan starts at 1. Two things switch the whole exemption
off: DTREATY_WANT_REVENGE on the owner's side, and a trespasser whose integrity_blemishes has
reached 5, at which point it has earned the complaint regardless of who else is at war.

Only land is considered. A unit below ALT_SHORE_LINE is skipped before its region is read,
which is what keeps the 64-entry land-region array in bounds - Map::region runs to 127 and
water regions start at 64.

Every unit of the trespasser gets VSTATE_UNK_400 cleared whether or not it is counted, so the
flag means "counted by the most recent call" rather than "counted at some point". The units
that survive every filter get it set again, plus VFLAG bits 0 and 1, and the LAST such unit's
base is the one left in base_id_out.

The weight is not the unit count. A qualifying unit is worth 1, plus 1 more if it is standing
on improved terrain, plus - once the distance test has been passed - a further 5 if the two
factions have a treaty and 1 if they do not, because a treaty partner's units deep inside your
borders is the worse offence. The improved-terrain mask 0x81608850 is exactly mine, solar
collector, bunker, farm, forest, condenser, thermal borehole and sensor array; it deliberately
leaves out road, magtube, airbase, soil enricher and echelon mirror.

That last increment is skipped entirely the first time a unit is seen while sunspots are down:
VSTATE_UNK_800 is a once-per-unit grace flag, set on the quiet pass and honoured forever after.

A unit further than vector_dist 2 from the base whose territory it is in is ignored, unless the
owner already has a grievance (DTREATY_WANT_REVENGE, DTREATY_SHALL_BETRAY or DTREATY_UNK_800)
or general diplomatic friction is above 12. The distance is doubled rather than halved -
(dy + dx) & ~1 compared against 4 - which is vector_dist's numerator with its floor still in
place, and the x term wraps on a round map exactly as x_dist does, testing bit 0 of MapIsFlat
rather than the whole int the way reset_territory's own wrap does.

Verification note: the region loop's UPPER bound is the one perturbation the suite cannot
observe. Running it to 64 inclusive only adds a store to the word after region_shared, which no
read in the body reaches: the altitude gate admits land only, land regions stop at 63, and a
region of 64 or more would read past the array in the correct body just as it does in the
mutant, so no fixture can distinguish them without reading uninitialised stack itself. The
bound is pinned by the disassembly instead, twice over - `cmp esi, 0x40` at 0055ECAE, and the
frame, where the array occupies exactly [ebp-118h] through [ebp-18h]. The loop's START, and the
unit walk's upper bound, are both covered by paired fixtures.

Verification note: base_id is deliberately left uninitialised. whose_territory only writes
through the pointer when the observer is not the owner, so a call with faction_id ==
faction_id_with that matches leaves whatever the previous unit's call stored - which is what
the original does, having given the slot to region_shared's treaty temporary until the walk
starts. No caller passes the same faction twice.
*/
int __cdecl territory(int faction_id, int faction_id_with, int flags, int *base_id_out,
                      int *count_out) {
    int weight = 0;
    if (base_id_out) {
        *base_id_out = -1;
    }
    if (count_out) {
        *count_out = 0;
    }
    uint32_t treaty = PlayersData[faction_id].diplo_treaties[faction_id_with];
    if (treaty & (DTREATY_PACT | DTREATY_VENDETTA)) {
        return 0;
    }
    if (!(flags & 2) && treaty & DTREATY_UNK_200) {
        return 0;
    }
    int region_shared[MaxRegionLandNum];
    for (int region = 0; region < MaxRegionLandNum; region++) {
        region_shared[region] = 0;
        if (treaty & DTREATY_WANT_REVENGE) {
            continue;
        }
        if (PlayersData[faction_id_with].integrity_blemishes >= 5) {
            continue;
        }
        for (int other_id = 1; other_id < MaxPlayerNum; other_id++) {
            if (other_id == faction_id || other_id == faction_id_with) {
                continue;
            }
            if (!(PlayersData[faction_id].diplo_treaties[other_id] & DTREATY_VENDETTA)) {
                continue;
            }
            if (!(PlayersData[faction_id_with].diplo_treaties[other_id] & DTREATY_VENDETTA)) {
                continue;
            }
            if (PlayersData[other_id].region_total_bases[region]) {
                region_shared[region] = 1;
                break;
            }
        }
    }
    for (int veh_id = 0; veh_id < VehCurrentCount; veh_id++) {
        Veh &veh = Vehs[veh_id];
        veh.state &= ~VSTATE_UNK_400;
        if (veh.faction_id != (uint8_t)faction_id_with) {
            continue;
        }
        uint32_t plan = VehPrototypes[veh.proto_id].plan;
        if (plan > PLAN_COLONIZATION
            && (plan != PLAN_SUPPLY_CONVOY || veh.order != ORDER_CONVOY)) {
            continue;
        }
        Map *tile = map_loc(veh.x, veh.y);
        if ((tile->climate & 0xE0) < (ALT_SHORE_LINE << 5)) {
            continue;
        }
        if (region_shared[tile->region]) {
            continue;
        }
        int base_id;
        if (whose_territory(faction_id_with, veh.x, veh.y, &base_id, true) != faction_id) {
            continue;
        }
        if (base_id < 0) {
            continue;
        }
        if (base_id_out) {
            *base_id_out = base_id;
        }
        veh.state |= VSTATE_UNK_400;
        weight++;
        if (map_loc(veh.x, veh.y)->bit & (BIT_MINE | BIT_SOLAR_TIDAL | BIT_BUNKER | BIT_FARM
            | BIT_FOREST | BIT_CONDENSER | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY)) {
            weight++;
        }
        if (!(flags & 1)) {
            continue;
        }
        int x_delta = abs(Bases[base_id].x - veh.x);
        if (!(MapIsFlat & 1) && x_delta > (int)MapLongitude) {
            x_delta = MapLongitudeBounds - x_delta;
        }
        if (((abs(Bases[base_id].y - veh.y) + x_delta) & ~1) > 4
            && !(PlayersData[faction_id].diplo_treaties[faction_id_with]
                & (DTREATY_WANT_REVENGE | DTREATY_SHALL_BETRAY | DTREATY_UNK_800))
            && DiploFriction <= 12) {
            continue;
        }
        if (count_out && veh.flags & 1) {
            (*count_out)++;
        }
        veh.flags |= 3;
        if (!(veh.state & VSTATE_UNK_800) && SunspotDuration <= 0) {
            veh.state |= VSTATE_UNK_800;
        } else {
            weight += (PlayersData[faction_id].diplo_treaties[faction_id_with] & DTREATY_TREATY)
                ? 5 : 1;
        }
    }
    if (weight) {
        PlayersData[faction_id].flags |= PFLAG_UNK_10000;
    }
    return weight;
}

/*
Purpose: Determine the ideal unit count to protect a faction's bases in the specified land region.
// ORIGINAL: 0x00560D50 ?guard_check@@YAHHH@Z 0x00560D50-0x00560DC3 BYTE_EXACT
// LEVER: `plan_region` as `uint32_t` forced the image's byte load into a
//        zero-extend (`xor edx,edx; mov dl,...`) followed by 32-bit `cmp edx,N`;
//        the field is `uint8_t region_base_plan[128]`, and typing the local
//        `uint8_t` drops the `xor` and compares `dl` directly, matching the
//        image byte-for-byte. Also needed `--flags` with `/Oy-` to see the kept
//        frame pointer - the tool's own default flag set omits it.
// size      115 bytes
// prototype int (__cdecl ?guard_check@@YAHHH@Z)(int factionID, int region)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Amount of non-offensive units needed to guard region
Status: Complete
*/
int __cdecl guard_check(int faction_id, int region) {
    if (region >= MaxRegionLandNum) {
        return 0;
    }
    int factor = 2 - PlayersData[faction_id].ai_fight;
    uint8_t plan_region = PlayersData[faction_id].region_base_plan[region];
    if (plan_region == PLAN_COLONIZATION) {
        factor += 2;
    } else if (plan_region == PLAN_DEFENSIVE) {
        factor = 1; // 1-1 unit per base ratio
    }
    if (PlayersData[faction_id].flags & PFLAG_STRAT_DEF_OBJECTIVES) {
        factor = 1; // 1-1 unit per base ratio
    }
    return (PlayersData[faction_id].region_total_bases[region] + factor - 1) / factor;
}

/*
Purpose: Add the specific goal to the faction's goals for the specified tile. Optional base param.
// ORIGINAL: 0x00579A30 ?add_goal@@YAXHHHHHH@Z 0x00579A30-0x00579B64
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
    int priority_search = 0;
    int goal_id = -1;
    for (int i = 0; i < MaxGoalsNum; i++) {
        Goal &goals = PlayersData[faction_id].goals[i];
        if (goals.x == x && goals.y == y && goals.type == type) {
            if (goals.priority <= priority) {
                goals.priority = (int16_t)priority;
            }
            return;
        }
    }
    for (i = 0; i < MaxGoalsNum; i++) {
        Goal &goals = PlayersData[faction_id].goals[i];
        int type_cmp = goals.type;
        int priroty_cmp = goals.priority;
        if (type_cmp < 0 || priroty_cmp < priority) {
            int cmp = type_cmp >= 0 ? 0 : 1000;
            if (!cmp) {
                cmp = priroty_cmp > 0 ? 20 - priroty_cmp : priroty_cmp + 100;
            }
            if (cmp > priority_search) {
                priority_search = cmp;
                goal_id = i;
            }
        }
    }
    if (goal_id >= 0) {
        Goal &goals = PlayersData[faction_id].goals[goal_id];
        goals.type = (int16_t)type;
        goals.priority = (int16_t)priority;
        goals.x = x;
        goals.y = y;
        goals.base_id = base_id;
    }
}

/*
Purpose: Add the specific site to the faction's site goals for the specified tile.
// ORIGINAL: 0x00579B70 ?add_site@@YAXHHHHH@Z 0x00579B70-0x00579CB5
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
    if ((x ^ y) & 1 && GameState & STATE_DEBUG_MODE) {
        danger("Bad SITE", "", x, y, type);
    }
    for (int i = 0; i < MaxSitesNum; i++) {
        Goal &sites = PlayersData[faction_id].sites[i];
        if (sites.x == x && sites.y == y && sites.type == type) {
            if (sites.priority <= priority) {
                sites.priority = (int16_t)priority;
            }
            return;
        }
    }
    int priority_search = 0;
    int site_id = -1;
    for (i = 0; i < MaxSitesNum; i++) {
        Goal &sites = PlayersData[faction_id].sites[i];
        int type_cmp = sites.type;
        int priroty_cmp = sites.priority;
        if (type_cmp < 0 || priroty_cmp < priority) {
            int cmp = type_cmp >= 0 ? 0 : 1000;
            if (!cmp) {
                cmp = 20 - priroty_cmp;
            }
            if (cmp > priority_search) {
                priority_search = cmp;
                site_id = i;
            }
        }
    }
    if (site_id >= 0) {
        Goal &sites = PlayersData[faction_id].sites[site_id];
        sites.type = (int16_t)type;
        sites.priority = (int16_t)priority;
        sites.x = x;
        sites.y = y;
        add_goal(faction_id, type, priority, x, y, -1);
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
// ORIGINAL: 0x00579E00 ?init_goals@@YAXH@Z 0x00579E00-0x00579E66 SEMANTIC
// TRIED: already SHAPE_EXACT (1.000 similarity, 28/38 raw) - the only
//            divergence is the loop pointer's anchor field (image anchors at
//            &goals[i].priority, this tree at &goals[i].type; every absolute
//            byte address written is identical either way). Swapping the
//            priority/type write order to move the anchor scored WORSE
//            (MISMATCH, 22/38) - left as the tree already had it.
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
        Goal &goals = PlayersData[faction_id].goals[i];
        // y BEFORE x: `store_order 0x00579E00` shows the image writing @+8
        // before @+4 in both loops.
        goals.type = -1;
        goals.priority = 0;
        goals.y = 0;
        goals.x = 0;
        goals.base_id = 0;
    }
    for (i = 0; i < MaxSitesNum; i++) {
        Goal &sites = PlayersData[faction_id].sites[i];
        sites.type = -1;
        sites.priority = 0;
        sites.y = 0;
        sites.x = 0;
        sites.base_id = 0;
    }
}

/*
Purpose: Delete sites of the specified type within proximity of the tile along with related goals.
// ORIGINAL: 0x00579E70 ?del_site@@YAXHHHHH@Z 0x00579E70-0x00579F73
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
            int dx = x_dist(x, sites->x);
            int dy = abs(y - sites->y);
            int abs_dx = abs(dx);
            int abs_dy = abs(dy);
            int largest = abs_dx;
            if (abs_dx <= abs_dy) {
                largest = abs_dy;
            }
            int smallest = abs_dx;
            if (abs_dx >= abs_dy) {
                smallest = abs_dy;
            }
            int dist = largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1);
            if (dist <= proximity) {
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
    for (int i = 0; i < BaseCurrentCount; i++) {
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
                && has_tech_call(tech_id, faction_id)) {
                PlayersData[faction_id].flags |= PFLAG_MAP_REVEALED;
            }
        }
    }
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
            && VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE) {
            int offense = Weapon[VehPrototypes[proto_id].weapon_id].offense_rating;
            if (offense >= best_offense && offense < 99) {
                best_offense = offense;
            }
        }
    }
    for (int faction_id = 0; faction_id < MaxPlayerNum; faction_id++) {
        if (apply_ranks) {
            PlayersData[faction_id].ranking = 0;
        }
        if (!faction_id) {
            continue;
        }
        int score;
        if (!is_alive(faction_id)) {
            score = 0;
        } else if ((ObjectiveReqVictory < 9000 || ObjectivesSuddenDeathVictory < 9000)
            && !ObjectiveAchievePts) {
            score = num_objectives(faction_id, false) * 10;
        } else {
            score = (PlayersData[faction_id].pop_total
                + PlayersData[faction_id].theory_of_everything) * 4;
            for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
                // has_tech_call: the image CALLS 0x005B9F20 twice from here
                // (0x005AC779 / 0x005AC823) rather than open-coding the walk.
                if (has_tech_call(tech_id, faction_id)) {
                    score += (tech_id < MaxTechnologyNum)
                        ? Technology[tech_id].growth_value + Technology[tech_id].tech_value
                            + Technology[tech_id].wealth_value + Technology[tech_id].power_value
                        : 4;
                }
            }
            const int *projects = reinterpret_cast<const int *>(&SecretProject);
            for (int i = 0; i < MaxSecretProjectNum; i++) {
                if (projects[i] >= 0
                    && Bases[projects[i]].faction_id_current == (uint8_t)faction_id) {
                    score += 10;
                }
            }
            score += num_objectives(faction_id, false) * (int)ObjectiveAchievePts;
            for (int proto_id = 0; proto_id < MaxVehProtoNum; proto_id++) {
                if (proto_id < MaxVehProtoFactionNum
                    && !has_tech(VehPrototypes[proto_id].preq_tech, faction_id)) {
                    continue;
                }
                uint32_t active = PlayersData[faction_id].proto_id_active[proto_id];
                if (active > 250) {
                    continue;
                }
                int value = VehPrototypes[proto_id].cost * (int)active;
                if (!value) {
                    continue;
                }
                int offense = Weapon[VehPrototypes[proto_id].weapon_id].offense_rating;
                if (!offense) {
                    score += value / 4;
                } else if (offense < 0) {
                    score += value / 2;
                } else if (offense >= 99) {
                    score += value;
                } else {
                    score += (offense / best_offense) * value;
                }
            }
        }
        FactionRankingsUnk[faction_id] = (uint32_t)score;
        if (TurnCurrentNum < MaxRankingHistoryTurns) {
            FactionRankingHistory[faction_id + TurnCurrentNum * MaxPlayerNum] = (uint16_t)score;
        }
    }
    if (!apply_ranks) {
        return;
    }
    for (int rank = MaxPlayerNum - 1; rank > 0; rank--) {
        int best_faction = 0;
        int best_score = -1;
        for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
            if ((int)FactionRankingsUnk[faction_id] > best_score) {
                best_faction = faction_id;
                best_score = (int)FactionRankingsUnk[faction_id];
            }
        }
        FactionRankingsUnk[best_faction] = (uint32_t)-1;
        PlayersData[best_faction].ranking = (uint32_t)rank;
    }
    for (rank = 0; rank < MaxPlayerNum; rank++) {
        FactionRankings[rank] = 0;
    }
    for (faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
        if (is_alive(faction_id)) {
            FactionRankings[PlayersData[faction_id].ranking] = (uint32_t)faction_id;
        }
    }
    uint32_t humans = (LockEnableMask & 0xFF);
    RankingFactionIDUnk1 = 0;
    RankingFactionIDUnk2 = 0;
    for (rank = MaxPlayerNum - 1; rank >= 0; rank--) {
        if (humans & (1 << FactionRankings[rank])) {
            RankingFactionIDUnk1 = FactionRankings[rank];
            break;
        }
    }
    for (rank = 0; rank < MaxPlayerNum; rank++) {
        if (humans & (1 << FactionRankings[rank])) {
            RankingFactionIDUnk2 = FactionRankings[rank];
            break;
        }
    }
    int target_id = great_satan(FactionRankings[MaxPlayerNum - 1], false)
        ? (int)FactionRankings[MaxPlayerNum - 1] : 0;
    if (climactic_battle()) {
        for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
            if (is_human(faction_id)
                && (PlayersData[faction_id].diff_level >= 4
                    || GameRules & RULES_INTENSE_RIVALRY)) {
                target_id = faction_id;
            }
        }
    }
    if (!target_id) {
        GameState &= ~STATE_UNK_200;
        return;
    }
    GameState |= STATE_UNK_200;
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
        int weight = (GameRules & RULES_INTENSE_RIVALRY)
            ? 5 : PlayersData[target_id].diff_level;
        int urge = (int)reputation(target_id, faction_id);
        if (climactic_battle()) {
            urge += 4;
        }
        if (PlayersData[faction_id].flags_ext & PFLAGEXT_SHAMELESS_BETRAY_HUMANS) {
            urge += 4;
        }
        if (PlayersData[target_id].diplo_treaties[faction_id] & DTREATY_WANT_REVENGE) {
            urge += 4;
        }
        urge *= weight;
        int bound = (vendetta_count - PlayersData[faction_id].ai_fight + 2) * 256;
        if ((int)rnd(bound, NULL) < urge) {
            set_treaty(faction_id, target_id, DTREATY_SHALL_BETRAY, true);
        }
    }
}

/*
Purpose: Calculate the basic social engineering modifiers for the specified faction.
// ORIGINAL: 0x005B0D70 ?compute_faction_modifiers@@YAXH@Z 0x005B0D70-0x005B0DF3
// LEVER: three WALKING POINTERS (bonus_id/bonus_val1/bonus_val2) advanced
//        together in the `for`'s increment, rather than `faction_bonus_id[i]`
//        &c re-subscripted each iteration - the image walks ONE pointer and
//        reaches the other two arrays through fixed +-0x20 offsets from it
//        (all three are 8-int arrays, 32 bytes apart), which three parallel
//        pointers incremented by a constant stride lets VC6 fold the same
//        way. Moved 6/48 (0.757) -> 6/48 (0.875) - same raw count, tighter
//        similarity, and the image's `push ebp`/`ebp`-frame prologue is
//        restored, which the re-subscripted form did not have at any flag
//        set tried.
// size      131 bytes
// prototype void (__cdecl ?compute_faction_modifiers@@YAXH@Z)(int factionID)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl compute_faction_modifiers(int faction_id) {
    ZeroMemory(&PlayersData[faction_id].soc_effect_base, sizeof(SocialEffect));
    int count = Players[faction_id].faction_bonus_count;
    int *bonus_id = Players[faction_id].faction_bonus_id;
    int *bonus_val1 = Players[faction_id].faction_bonus_val1;
    int *bonus_val2 = Players[faction_id].faction_bonus_val2;
    for (int i = 0; i < count; i++, bonus_id++, bonus_val1++, bonus_val2++) {
        if (*bonus_id == RULE_SOCIAL) {
            *(&PlayersData[faction_id].soc_effect_base.economy + *bonus_val1)
                += *bonus_val2;
        }
    }
}

/*
Purpose: Calculate the social engineering effect modifiers for the specified faction.
// ORIGINAL: 0x005B4210 ?social_calc@@YAXPAHPAHHHH@Z 0x005B4210-0x005B44C9
// TRIED: hoisting `base_project(SP_NETWORK_BACKBONE)`/`base_project(
//            SP_CLONING_VATS)` into locals ahead of the loop, keeping the
//            per-branch `Bases[base_id].faction_id_current == faction_id`
//            checks in place (the image loads both project base ids ONCE
//            via `mov ecx,[0x9a6574]`/`mov esi,[0x9a6568]` before the cat/eff
//            loop, then reuses them at both the SE_THOUGHT_CONTROL and
//            SOCIAL_CAT_VALUES/SE_POWER sites, but re-checks Bases[]/
//            faction_id_current locally each time) - agreeing dropped 4->3,
//            no improvement. `store_order` shows this tree spills 24 locals
//            to the stack against the image's 16, so every jump target
//            downstream of instruction ~10 is offset and reads as a
//            mismatch even where the branch shape matches - a stack-layout/
//            register-pressure divergence, not a single fixable bug found so
//            far. Plateaus at 10/244 agreeing (0.155 similar); not chased
//            further given the size of the function.
// symbol    ?social_calc@@YAXPAUSocialCategory@@PAUSocialEffect@@HHH@Z
// size      697 bytes
// prototype void (__cdecl ?social_calc@@YAXPAHPAHHHH@Z)(social_category* category, social_effect* effect, uint32_t factionID, BOOL toggle, BOOL isQuickCalc)
// callers   7   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF130
Return Value: n/a
Status: Complete
*/
void __cdecl social_calc(SocialCategory *category, SocialEffect *effect, int faction_id,
    BOOL UNUSED(toggle), BOOL is_quick_calc) {
    ZeroMemory(effect, sizeof(SocialEffect));
    for (int cat = 0; cat < MaxSocialCatNum; cat++) {
        uint32_t model = category->models_[cat];
        for (int eff = 0; eff < MaxSocialEffectNum; eff++) {
            int effect_val = *(&SocialCategories[cat].model_effect[model].economy + eff);
            if (effect_val < 0) {
                if (cat == SOCIAL_CAT_FUTURE) {
                    if (model == SE_CYBERNETIC) {
                        if (has_project(SP_NETWORK_BACKBONE, faction_id)) {
                            effect_val = 0;
                        }
                    } else if (model == SE_THOUGHT_CONTROL) {
                        if (has_project(SP_CLONING_VATS, faction_id)) {
                            effect_val = 0;
                        }
                    }
                } else if (cat == SOCIAL_CAT_VALUES && model == SE_POWER 
                    && has_project(SP_CLONING_VATS, faction_id)) {
                    effect_val = 0;
                }
                if (effect_val < 0) {
                    for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
                        if (Players[faction_id].faction_bonus_val1[i] == cat
                            && Players[faction_id].faction_bonus_val2[i] == (int)model) {
                            if (Players[faction_id].faction_bonus_id[i] == RULE_IMPUNITY) {
                                *(&effect->economy + eff) -= effect_val; // negates neg effects
                            } else if (Players[faction_id].faction_bonus_id[i] == RULE_PENALTY) {
                                *(&effect->economy + eff) += effect_val; // doubles neg effects
                            }
                        }
                    }
                }
            }
            *(&effect->economy + eff) += effect_val;
        }
    }
    if (!is_quick_calc) {
        if (has_project(SP_ASCETIC_VIRTUES, faction_id)) {
            effect->police++;
        }
        if (has_project(SP_LIVING_REFINERY, faction_id)) {
            effect->support += 2;
        }
        if (has_temple(faction_id)) {
            effect->planet++;
            if (is_alien_faction(faction_id)) {
                effect->research++; // bonus documented in conceptsx.txt but not manual
            }
        }
        SocialEffect *effect_calc = effect;
        SocialEffect *effect_base = &PlayersData[faction_id].soc_effect_base;
        for (int eff = 0; eff < MaxSocialEffectNum; eff++) {
            *(&effect_calc->economy + eff) += *(&effect_base->economy + eff);
        }
        for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
            if (Players[faction_id].faction_bonus_id[i] == RULE_IMMUNITY) { // cancels neg effects
                int *eff_faction_mod 
                    = (&effect->economy + Players[faction_id].faction_bonus_val1[i]);
                *eff_faction_mod = range(*eff_faction_mod, 0, 999);
            } else if (Players[faction_id].faction_bonus_id[i] == RULE_ROBUST) {
                int *eff_faction_mod 
                    = (&effect->economy + Players[faction_id].faction_bonus_val1[i]);
                if (*eff_faction_mod < 0) {
                    *eff_faction_mod /= 2; // halves neg effects
                }
            }
        }
    }
}

/*
Purpose: Handle the social engineering turn upkeep for the specified faction.
// ORIGINAL: 0x005B44D0 ?social_upkeep@@YAXH@Z 0x005B44D0-0x005B4550 BYTE_EXACT
// size      128 bytes
// prototype void (__cdecl ?social_upkeep@@YAXH@Z)(int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B4210
Return Value: n/a
Status: Complete
*/
void __cdecl social_upkeep(int faction_id) {
    for (int i = 0; i < MaxSocialCatNum; i++) {
        PlayersData[faction_id].soc_category_active.models_[i] =
            PlayersData[faction_id].soc_category_pending.models_[i];
    }
    SocialCategory *soc_cat_pen = &PlayersData[faction_id].soc_category_pending;
    social_calc(soc_cat_pen, &PlayersData[faction_id].soc_effect_pending, faction_id, false, false);
    social_calc(soc_cat_pen, &PlayersData[faction_id].soc_effect_active, faction_id, false, false);
    social_calc(soc_cat_pen, &PlayersData[faction_id].soc_effect_temp, faction_id, true, false);
    PlayersData[faction_id].soc_upheaval_cost_paid = 0;
}

/*
Purpose: Calculate the cost of the social upheaval for the specified faction.
// ORIGINAL: 0x005B4550 ?social_upheaval@@YAHHPAH@Z 0x005B4550-0x005B45F8
// symbol    ?social_upheaval@@YAIHPAUSocialCategory@@@Z
// size      168 bytes
// prototype int (__cdecl ?social_upheaval@@YAHHPAH@Z)(int factionID, social_category* category)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Social upheaval cost
Status: Complete
*/
// BODY IN faction.h, as `MEASURED inline`: social_ai's own `calls` list
// (0x004EF090 0x00539D40 0x00581260 0x005B4210 0x005B4600 0x005B9F20
// 0x005FCB20) has no 0x005B4550 in it, so both of its call sites are
// inlined in the image, not real calls - even though this address is a
// real out-of-line body in its own right. The marker stays here because
// that is where the catalogue reads it.

/*
Purpose: Check to see whether the faction can utilize a specific social category and model.
// ORIGINAL: 0x005B4730 ?society_avail@@YAHHHH@Z 0x005B4730-0x005B4786 BYTE_EXACT
// size      86 bytes
// prototype int (__cdecl ?society_avail@@YAHHHH@Z)(int socCategory, int socModel, int factionID)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
// LEVER (2026-09-03): `has_tech_call` on the `return`. 0 of 36 agreeing,
//   53 instructions against the image's 36, all of it technology.h's
//   MEASURED inline open-coded into a six-line body. With the forwarder the
//   body is the image's own 36 and the winning set flips to `/O2 /Oy- /GX`,
//   which is what produces the image's frame. 36 of 36, BYTE_EXACT. Same
//   shape as see_map_check 0x005A96D0.
Return Value: Is social category/model available to faction? true/false
Status: Complete
*/
BOOL __cdecl society_avail(int soc_category, int soc_model, int faction_id) {
    if (Players[faction_id].soc_anti_ideology_category == soc_category
        && Players[faction_id].soc_anti_ideology_model == soc_model) {
        return false;
    }
    // has_tech_call, not has_tech: the image CALLS 0x005B9F20 once from here
    // (0x005B477B) and technology.h's MEASURED inline was open-coded into
    // this body.
    return has_tech_call(SocialCategories[soc_category].preq_tech[soc_model], faction_id);
}

/*
Purpose: Calculate an AI faction's social engineering.
// ORIGINAL: 0x005B4790 ?social_ai@@YAXHHHHHPAH@Z 0x005B4790-0x005B5612
// LEVER (2026-09-03): `has_tech_call` at the loop site (0x005B4AA1 in the
//        image's own calls list) - 22/1353 -> 50/1353. Still MISMATCH, and
//        call_diff still says MORE (9 vs 7), for two reasons this file cannot
//        reach: do_all_non_input is MEASURED inline in temp.h, so the image's
//        single `call 0x005FCB20` arrives here as its do_non_input/do_net/
//        check_net expansion - the same ceiling the brief records on
//        Path::continent - and the social_calc call both sides make resolves
//        to DIFFERENT SYMBOLS (the image's `?social_calc@@YAXPAHPAHHHH@Z`
//        spells both pointers `PAH`/char *, this tree's spells the class
//        pointers), so call_diff pairs neither. Measured and rejected:
//        `#pragma auto_inline(off)` around social_calc's definition changes
//        nothing at all (compiled stays 1308 instructions, same call set) -
//        VC6 was never folding it into social_ai.
// LEVER: call_diff showed 9 vs the image's 7 - this body's two calls to
//        social_upheaval() are not in the image's own `calls` list below
//        (no 0x005B4550), so both are inlined there even though 0x005B4550
//        also exists as a real out-of-line body. Moved social_upheaval to
//        faction.h as `MEASURED inline` (see its ORIGINAL marker, which now
//        points there) per the "helper the image inlines" lever.
//        TRIED: `MEASURED __forceinline` on the same definition - call
//        count stayed 9 vs 7 under every flag set `call_diff` tries, so this
//        tree's O2 will not fold social_upheaval's 4-iteration loop in
//        either spelling; reverted to plain `MEASURED inline` since it did
//        not help. This is a 1353-instruction function ("likely has multiple
//        issues due to length + complexity" per its own status line);
//        MISMATCH at 24/1353 agreeing, left for a dedicated pass rather than
//        chased further here.
// symbol    ?social_ai@@YAXHHHHHPAUSocialCategory@@@Z
// size      3714 bytes
// prototype void (__cdecl ?social_ai@@YAXHHHHHPAH@Z)(int factionID, int growthVal, int techVal, int wealthVal, int powerVal, social_category* output)
// callers   2   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004EF090 0x00539D40 0x00581260 0x005B4210 0x005B4600 0x005B9F20 0x005FCB20
Return Value: n/a
Status: Complete - testing (likely has multiple issues due to length + complexity)
*/
void __cdecl social_ai(int faction_id, int growth_val, int tech_val, int wealth_val, 
                       int power_val, SocialCategory *output) {
    // setup
    int fight_val;
    int unk_val1 = -1;
    if (!output) {
        if (is_human(faction_id)) {
            return;
        } else {
            growth_val = PlayersData[faction_id].ai_growth;
            tech_val = PlayersData[faction_id].ai_tech;
            wealth_val = PlayersData[faction_id].ai_wealth;
            power_val = PlayersData[faction_id].ai_power;
            fight_val = PlayersData[faction_id].ai_fight;
        }
    } else if (growth_val < 0) {
        return;
    } else if (growth_val > 100) {
        unk_val1 = growth_val - 100;
        growth_val = PlayersData[faction_id].ai_growth;
        tech_val = PlayersData[faction_id].ai_tech;
        wealth_val = PlayersData[faction_id].ai_wealth;
        power_val = PlayersData[faction_id].ai_power;
        fight_val = PlayersData[faction_id].ai_fight;
    } else {
        if (!power_val) {
            fight_val = -(wealth_val != 0);
        } else {
            fight_val = 1;
        }
    }
    // Four choices for each of four categories: original 5B4831..5B483F.
    uint32_t unk_val2 = 4;
    for (int i = 3; i > 0; i--) {
        unk_val2 *= 4;
    }
    // future pop growth
    int pop_goal_growth = 0;
    for (i = 0; i < BaseCurrentCount; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            pop_goal_growth += pop_goal(i) - Bases[i].population_size;
        }
    }
    // plan and enemy region metrics
    uint32_t unk_sum1 = 0;
    uint32_t unk_count1 = 0;
    uint32_t unk_count2 = 0;
    uint32_t unk_sum2 = 0;
    for (uint32_t region = 1; region < MaxRegionLandNum; region++) {
        if (!bad_reg(region)) {
            uint8_t base_count = PlayersData[faction_id].region_total_bases[region];
            if (base_count) {
                uint8_t plan = PlayersData[faction_id].region_base_plan[region];
                if (plan == PLAN_DEFENSIVE || PlayersData[faction_id].unk_77[region] & 0x400) {
                    unk_sum1 += PlayersData[faction_id].region_total_combat_vehs[region] * 2;
                } else if (plan == PLAN_OFFENSIVE) {
                    unk_sum1 += PlayersData[faction_id].region_total_combat_vehs[region];
                    unk_count2++;
                } else if (plan != PLAN_COLONIZATION) {
                    unk_count1++;
                }
                if (plan <= PLAN_DEFENSIVE) {
                    for (uint32_t f = 1; f < MaxPlayerNum; f++) {
                        if (f != faction_id) {
                            if (has_treaty(faction_id, f, DTREATY_VENDETTA | DTREATY_WANT_REVENGE)
                                && PlayersData[f].enemy_best_psi_offense 
                                >= PlayersData[f].enemy_best_weapon_value 
                                && PlayersData[f].proto_id_active[BSC_MIND_WORMS] > 1) {
                                unk_sum2 += base_count; // not used
                            }
                        }
                    }
                }
            }
        }
    }
    if (output && power_val && unk_sum1 < 1) {
        unk_sum1 = 1;
    }
    // tech ranking
    uint32_t unk_val3 = 1;
    if (GameState & 0x200 // set in rankings(), related to intense riv + end game
        && PlayersData[faction_id].ranking < 6) {
        int tech_rank_diff = PlayersData[RankingFactionIDUnk1].tech_ranking / 2
            - PlayersData[faction_id].tech_ranking / 2;
        if (tech_rank_diff > 5) {
            unk_val3 = 2;
        }
        if (tech_rank_diff > 10) {
            unk_val3++;
        }
    }
    // search / calculate
    int unk_val4 = -9999; // search val?
    int soc_cat_bits = -1;
    // unk_sum2 = 0 > used as iterator, optimization re-use, var unused
    for (uint32_t player = 0; player < unk_val2; player++) {
        SocialCategory soc_cat;
        SocialEffect soc_eff;
        int k = player;
        bool invalid_combination = false;
        for (int j = 0; j < MaxSocialCatNum; j++) {
            int model = k & 3;
            soc_cat.models_[j] = model;
            if (model) {
                if ((Players[faction_id].soc_anti_ideology_category == j
                    && Players[faction_id].soc_anti_ideology_model == model)
                    // has_tech_call: the image CALLS 0x005B9F20 once here
                    // (0x005B4AA1, see the calls line) instead of open-coding
                    // the preq walk inside this loop.
                    || !has_tech_call(SocialCategories[j].preq_tech[model], faction_id)) {
                    invalid_combination = true;
                    break;
                }
            }
            k >>= 2;
        }
        // Original 5B4A90/5B4AAB skip directly to the outer increment.
        if (invalid_combination) continue;
        do_all_non_input();
        social_calc(&soc_cat, &soc_eff, faction_id, false, false);
        int unk_val6 = 0;
        if (unk_val1 >= 0) {
            unk_val6 = *(&soc_eff.economy + unk_val1) * 1000;
        }
        BOOL has_ideology = false;
        int ideology_cat = Players[faction_id].soc_ideology_category;
        if (ideology_cat >= 0) {
            int ideology_mod = Players[faction_id].soc_ideology_model;
            if (ideology_mod) {
                int unk_val7 = soc_cat.models_[ideology_cat];
                if (!unk_val7 && unk_val7 == ideology_mod) {
                    unk_val6 += PlayersData[faction_id].current_num_bases;
                } else {
                    has_ideology = true;
                }
            }
        }
        int ideology_eff = Players[faction_id].soc_anti_ideology_effect;
        if (ideology_eff >= 0) {
            int unk_val8 = *(&soc_eff.economy + ideology_eff);
            if (unk_val8 > 0) {
                unk_val6 += PlayersData[faction_id].current_num_bases;
            }
            if (unk_val8 < 0) {
                has_ideology = true;
            }
        }
        if (!has_ideology) {
            // economy
            int effic_val = PlayersData[faction_id].unk_46[range(4 - soc_eff.efficiency, 0, 8)];
            int econ_val = soc_eff.economy;
            int econ_weight = 0;
            if (econ_val < 2) {
                if (econ_val <= 0) {
                    if (econ_val < -1) {
                        for (int region = 0; region < MaxContinentNum; region++) {
                            econ_weight -= (soc_eff.economy + 1)
                                * PlayersData[faction_id].region_total_bases[region]
                                * ((PlayersData[faction_id].region_base_plan[region] 
                                    != PLAN_DEFENSIVE) + 1);
                        }
                        econ_weight /= (unk_sum1 + 1);
                    } else {
                        econ_weight = -1;
                    }
                } else {
                    for (int region = 0; region < MaxContinentNum; region++) {
                        econ_weight += PlayersData[faction_id].region_total_bases[region]
                            * ((PlayersData[faction_id].region_base_plan[region] != PLAN_DEFENSIVE) 
                                + 1);
                    }
                    econ_weight /= (unk_sum1 + 1);
                }
            } else {
                if (econ_val > 4) {
                    econ_val = 4;
                }
                econ_weight = PlayersData[faction_id].unk_47 + econ_val * 2 - 4 / (unk_sum1 + 1);
            }
            for (; econ_weight > 0; econ_weight--, effic_val--) {
                if (effic_val <= 0) {
                    break;
                }
            }
            if (!unk_sum1) {
                econ_weight *= 2;
            }
            econ_weight *= ((PlayersData[faction_id].ai_fight < 0 && unk_sum1 < 2) 
                + tech_val * 2 + wealth_val + 1);
            if (output) {
                if (wealth_val || tech_val) {
                    econ_weight *= 2;
                }
                if (growth_val || power_val) {
                    econ_weight /= 2;
                }
            } else if (wealth_val && !power_val && !growth_val) {
                econ_weight *= 2;
            }
            unk_val6 += econ_weight / unk_val3;
            // support
            int support_val = range(soc_eff.support + 4, 0, 7);
            int support_weight = PlayersData[faction_id].unk_38[support_val];
            if (soc_eff.support <= -4) {
                support_weight *= 3;
            }
            if (unk_sum1) {
                support_weight *= 2;
                if (soc_eff.support <= -3) {
                    support_weight *= 2;
                }
                if ((soc_eff.support == -1 || soc_eff.support == -2) && soc_eff.economy < 2) {
                    support_weight += support_weight / 2;
                }
            }
            if (output) {
                if (power_val) {
                    support_weight *= 3;
                }
                if (growth_val) {
                    support_weight *= 2;
                }
                if (tech_val) {
                    support_weight /= 2;
                }
            } else {
                if (fight_val > 0) {
                    support_weight *= 2;
                }
                if (growth_val > 0) {
                    support_weight += support_weight / 2;
                }
                if (wealth_val > 0) {
                    support_weight /= 2;
                }
                if (tech_val > 0) {
                    support_weight += support_weight / -4;
                }
                if (power_val > 0) {
                    support_weight += support_weight / 2;
                }
            }
            unk_val6 -= support_weight;
            // morale
            int morale_val = range(soc_eff.morale, -4, 4) * 2;
            if (morale_val < -6) {
                morale_val++;
            }
            if (morale_val < -2) {
                morale_val++;
            }
            if (morale_val > 6) {
                morale_val--;
            }
            if (morale_val > 2) {
                morale_val--;
            }
            int morale_weight = (unk_val3 * morale_val * (fight_val + 2) * (unk_sum1 + 1)
                * (PlayersData[faction_id].current_num_bases + PlayersData[faction_id].unk_48)) / 8;
            if (output) {
                if (power_val) {
                    morale_weight *= 3;
                }
                if (growth_val || wealth_val) {
                    morale_weight /= 2;
                }
            } else {
                if (fight_val > 1) {
                    morale_weight *= 2;
                }
                if (power_val && !growth_val && !wealth_val) {
                    morale_weight *= 2;
                }
                if (fight_val < 0 && !power_val && (wealth_val || tech_val) && !unk_sum1) {
                    morale_weight /= 2;
                }
            }
            unk_val6 += morale_weight;
            // efficiency
            int effic_weight = effic_val;
            if (soc_eff.efficiency == -3) {
                effic_weight = 2 * effic_val;
            } else if (soc_eff.efficiency <= -4) {
                effic_weight = 4 * effic_val;
            }
            if (!unk_sum1) {
                effic_weight *= 2;
            }
            if (output) {
                if (tech_val) {
                    effic_weight *= 3;
                }
                if (growth_val || power_val) {
                    effic_weight /= 2;
                }
            } else {
                if (!wealth_val && !tech_val) {
                    if (power_val) {
                        effic_weight /= 2;
                    }
                } else if (power_val) {
                    if (!wealth_val) {
                        effic_weight /= 2;
                    }
                } else if (!growth_val) {
                    effic_weight *= 2;
                }
            }
            unk_val6 -= effic_weight;
            // growth
            int growth_weight = (soc_eff.growth * (unk_count1 + 1) 
                * ((growth_val + 1) * 2 - wealth_val)
                * (pop_goal_growth + PlayersData[faction_id].nutrient_surplus_total)) / 5;
            if (aah_ooga(faction_id, faction_id)) {
                growth_weight *= 2;
            }
            if (Players[faction_id].rule_population > 0) {
                growth_weight /= 2;
            }
            if (output) {
                if (growth_val || wealth_val) {
                    growth_weight *= 2;
                }
                if (tech_val) {
                    growth_weight /= 2;
                }
            } else {
                if (growth_val && power_val) {
                    growth_weight *= 2;
                }
            }
            unk_val6 += growth_weight;
            // police
            int police_weight 
                = range(soc_eff.police, -2, 2) * PlayersData[faction_id].current_num_bases 
                * (unk_count2 * 2 + 1);
            if (PlayersData[faction_id].tech_ranking 
                < PlayersData[RankingFactionIDUnk2].tech_ranking
                && PlayersData[faction_id].ranking < PlayersData[RankingFactionIDUnk2].ranking) {
                police_weight *= 2;
            }
            if (PlayersData[faction_id].tech_ranking 
                < PlayersData[RankingFactionIDUnk1].tech_ranking
                && PlayersData[faction_id].ranking < PlayersData[RankingFactionIDUnk1].ranking) {
                police_weight /= 2;
            }
            if (output && (power_val || growth_val)) {
                police_weight *= 2;
            }
            unk_val6 += police_weight / (2 - fight_val);
            // talent
            int talent_weight = PlayersData[faction_id].unk_39[range(soc_eff.talent + 4, 0, 7)]
                [range(soc_eff.police + 5, 0, 8)];
            if (unk_sum1) {
                talent_weight += soc_eff.police * PlayersData[faction_id].current_num_bases;
            }
            if (output) {
                if (power_val || growth_val) {
                    talent_weight *= 2;
                }
                if (wealth_val) {
                    talent_weight /= 2;
                }
            } else {
                if (growth_val && tech_val) {
                    talent_weight *= 2;
                }
                if (power_val && (tech_val || wealth_val)) {
                    talent_weight /= 2;
                }
            }
            unk_val6 += talent_weight;
            // planet
            int planet_weight = (4 - soc_eff.planet) * PlayersData[faction_id].unk_49 * 4 / 4; // ?
            if (PlayersData[faction_id].soc_effect_base.planet > 0
                || PlayersData[faction_id].best_psi_offense 
                > PlayersData[faction_id].best_weapon_value) {
                planet_weight += PlayersData[faction_id].total_mil_veh;
            } else if (unk_val3 > 1) {
                planet_weight /= 2;
            }
            if (output) {
                if (growth_val) {
                    planet_weight *= 2;
                }
                if (wealth_val) {
                    planet_weight /= 2;
                }
            } else {
                if (power_val) {
                    if (growth_val) {
                        planet_weight /= 2;
                    }
                    if ((tech_val || wealth_val) && !growth_val) {
                        planet_weight /= 2;
                    }
                } else {
                    if (growth_val) {
                        if (tech_val) {
                            planet_weight *= 2;
                            if (!wealth_val && growth_val) {
                                planet_weight /= 2;
                            }
                        } else {
                            if (growth_val) {
                                planet_weight /= 2;
                            }
                        }
                    } else {
                        if ((tech_val || wealth_val) && growth_val) {
                            planet_weight /= 2;
                        }
                    }
                }
                unk_val6 -= planet_weight;
                // research
                int research_weight = range(soc_eff.research, -5, 5) 
                    * PlayersData[faction_id].labs_total / 10;
                if (!unk_sum1) {
                    research_weight *= 2;
                }
                if (output) {
                    if (tech_val) {
                        research_weight *= 2;
                    }
                    if (power_val) {
                        research_weight /= 2;
                    }
                }
                unk_val6 += research_weight;
                // industry
                uint32_t unk_val3_temp = range(unk_val3, 1, 2);
                int industry_weight = soc_eff.industry * unk_val3_temp
                    * PlayersData[faction_id].pop_total * (2 * (wealth_val + unk_sum1) + 1) / 10;
                if (power_val) {
                    industry_weight *= 2;
                }
                if (wealth_val) {
                    industry_weight += industry_weight / 2;
                }
                if (tech_val) {
                    industry_weight /= 2;
                }
                if (fight_val < 0 && !wealth_val) {
                    industry_weight /= 2;
                }
                unk_val6 += industry_weight;
                // why is this before probe? significance?
                unk_val6 += social_upheaval(faction_id, &soc_cat) / -3;
                // probe
                if (soc_eff.probe) {
                    int probe_weight;
                    for (uint32_t f = 0; f < MaxPlayerNum; f++) {
                        if (f != faction_id) {
                            probe_weight = PlayersData[player].mind_control_total / 4 + 1;
                            if (has_treaty(faction_id, player, DTREATY_PACT)) {
                                probe_weight /= 2;
                            }
                            if (!has_treaty(faction_id, player, DTREATY_WANT_REVENGE)) {
                                if (has_treaty(faction_id, player, DTREATY_TREATY)) {
                                    probe_weight /= 2;
                                }
                                if (!has_treaty(faction_id, player, DTREATY_COMMLINK)) {
                                    probe_weight /= 2;
                                }
                                if (!has_treaty(faction_id, player, DTREATY_VENDETTA)) {
                                    probe_weight /= 2;
                                }
                            }
                            probe_weight = ((PlayersData[player].current_num_bases
                                + PlayersData[faction_id].current_num_bases)
                                * range(soc_eff.probe, -2, 3) * probe_weight) / 2;
                            if (tech_val || wealth_val) {
                                probe_weight *= 2;
                            }
                            unk_val6 += probe_weight;
                        }
                    }
                }
                if (unk_val6 >= unk_val4) {
                    unk_val4 = unk_val6;
                    soc_cat_bits = unk_sum2;
                }
            }
        }
    }
    if (soc_cat_bits < 0) {
        if (output) {
            for (int i = 0; i < MaxSocialCatNum; i++) {
                output->models_[i]
                    = PlayersData[faction_id].soc_category_pending.models_[i];
            }
        }
    } else {
        if (!output) {
            BOOL no_cat_change = true;
            for (int i = 0; i < MaxSocialCatNum; i++) {
                uint32_t cat_bit = soc_cat_bits & 3;
                if (cat_bit != PlayersData[faction_id].soc_category_pending.models_[i]) {
                    PlayersData[faction_id].soc_category_pending.models_[i] = cat_bit;
                    no_cat_change = false;
                }
                soc_cat_bits >>= 2;
            }
            if (!no_cat_change) {
                social_set(faction_id);
                uint32_t cost 
                    = social_upheaval(faction_id, &PlayersData[faction_id].soc_category_pending);
                PlayersData[faction_id].energy_reserves 
                    = range(PlayersData[faction_id].energy_reserves - cost, 0, 999999999);
                PlayersData[faction_id].soc_upheaval_cost_paid += cost;
                if (!is_human(faction_id)) {
                    consider_designs(faction_id);
                }
            }
        } else {
            for (int i = 0; i < MaxSocialCatNum; i++) {
                output->models_[i] = soc_cat_bits & 3;
                soc_cat_bits >>= 2;
            }
        }
    }
}

/*
Purpose: Calculate specified faction's best available weapon and armor ratings as well as the
         fastest moving ground Veh chassis. Compare these capabilities to faction's best opponent
         capabilities based on current diplomacy.
// ORIGINAL: 0x00560DD0 ?enemy_capabilities@@YAXH@Z 0x00560DD0-0x00561076
// LEVER: call_diff showed this tree FEWER (8 vs 9) - the image calls
//        veh_avail(BSC_MIND_WORMS,...) TWICE, once for the best_psi_offense
//        ternary and again for best_psi_defense, rather than caching the
//        result in one `has_worms` local and reusing it. Un-cached both call
//        sites to match. Also every "keep the running max" comparison in the
//        function compiles non-strict in the image (`jl`/`jg` skip-the-store,
//        i.e. store on >=/<=) where this tree had strict `>`/`<`: fixed
//        weap_val/best_psi_offense, weap_val/best_weapon_value, arm_val/
//        best_psi_defense, arm_val/best_armor_value, Chassis[i].speed/
//        best_land_speed, and all five enemy_best_X vs PlayersData[j].best_X
//        comparisons in the closing loop (storing the same value back on a
//        tie is behaviourally inert, so this only affects codegen). Moved
//        0.430 -> 0.606 similar at the winning flag set. Remaining
//        divergence is the closing loop's has_treaty(i,j,*) reuse: the image
//        loads `PlayersData[i].diplo_treaties[j]` ONCE and reuses the byte
//        for all four DTREATY_* bit tests plus a `+=4`-per-j pointer walk;
//        this tree's O2 recomputes the address per has_treaty() call site -
//        register/CSE allocation, not chased per the no-register-only rule.
// LEVER (2026-09-03): both of those "register/CSE" residuals were the two
//        inlined helpers, and both closed. 18/210 (0.711 at the recorded
//        /O1 set) -> 133/210 at the DEFAULT /O2 /Gy /GR- /Oy- /GX:
//        (1) has_tech_call in all three loops - the image CALLS 0x005B9F20
//        three times, and technology.h's MEASURED inline was open-coded at
//        every site (measured 133/210 under /Ob0 /O2 BEFORE the switch,
//        which is what named the inline as the defect); (2) the closing
//        loop's four has_treaty() calls are now ONE read of
//        `PlayersData[i].diplo_treaties[j]` into a local with the DTREATY_*
//        tests applied to it, which is exactly what the image's single load
//        and byte reuse is. First divergence moved from instruction 4 to
//        126. Remaining: the closing loop's register roles and the
//        `+=4`-per-j pointer walk.
// size      678 bytes
// prototype void (__cdecl ?enemy_capabilities@@YAXH@Z)(int factionID)
// callers   3   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D270 0x0057D2E0 0x005B9F20 0x005BA910
Return Value: n/a
Status: Complete - testing / WIP
*/
void __cdecl enemy_capabilities(int faction_id) {
    PlayersData[faction_id].best_psi_offense = veh_avail(BSC_MIND_WORMS, faction_id, -1)
        ? weap_strat(WPN_PSI_ATTACK, faction_id) : 0;
    PlayersData[faction_id].best_weapon_value = 1;
    // has_tech_call, not has_tech, in all three loops: the image CALLS
    // 0x005B9F20 at 0x00560E2F / 0x00560EC1 / 0x00560F20 rather than
    // open-coding the preq walk, so /Ob0 (which stops technology.h's
    // MEASURED inline) agrees with the image on 133 of 210 instructions
    // where plain /O2 manages 33.
    for (int i = 0; i < MaxWeaponNum; i++) {
        if (has_tech_call(Weapon[i].preq_tech, faction_id) && Weapon[i].offense_rating < 99) {
            int weap_val = weap_strat(i, faction_id);
            if (Weapon[i].offense_rating < 0 && weap_val
                >= PlayersData[faction_id].best_psi_offense) {
                PlayersData[faction_id].best_psi_offense = weap_val;
            }
            if (weap_val >= PlayersData[faction_id].best_weapon_value) {
                PlayersData[faction_id].best_weapon_value = weap_val;
            }
        }
    }
    PlayersData[faction_id].best_psi_defense = veh_avail(BSC_MIND_WORMS, faction_id, -1)
        ? arm_strat(ARM_PSI_DEFENSE, faction_id) : 0;
    PlayersData[faction_id].best_armor_value = 1;
    for (i = 0; i < MaxArmorNum; i++) {
        if (has_tech_call(Armor[i].preq_tech, faction_id)) {
            int arm_val = arm_strat(i, faction_id);
            if (Armor[i].defense_rating < 0 && arm_val >= PlayersData[faction_id].best_psi_defense) {
                PlayersData[faction_id].best_psi_defense = arm_val;
            }
            if (arm_val >= PlayersData[faction_id].best_armor_value) {
                PlayersData[faction_id].best_armor_value = arm_val;
            }
        }
    }
    PlayersData[faction_id].best_land_speed = 1;
    for (i = 0; i < MaxChassisNum; i++) {
        if (has_tech_call(Chassis[i].preq_tech, faction_id) && Chassis[i].triad == TRIAD_LAND) {
            if (Chassis[i].speed >= PlayersData[faction_id].best_land_speed) {
                PlayersData[faction_id].best_land_speed = Chassis[i].speed;
            }
        }
    }
    PlayersData[faction_id].enemy_best_weapon_value = 0;
    PlayersData[faction_id].enemy_best_armor_value = 0;
    PlayersData[faction_id].enemy_best_land_speed = 0;
    PlayersData[faction_id].enemy_best_psi_offense = 0;
    PlayersData[faction_id].enemy_best_psi_defense = 0;
    for (i = 0; i < 4 && !PlayersData[faction_id].enemy_best_weapon_value; i++) {
        // 1st pass: vendetta, no treaty, has commlink
        // 2nd pass: no treaty, has commlink
        // 3rd pass: has commlink
        // 4th pass: any non-pact faction
        for (uint32_t j = 1; j < MaxPlayerNum; j++) {
            // The pair's treaty word is read ONCE and the bit tests all reuse
            // it - the image loads PlayersData[i].diplo_treaties[j] a single
            // time per j and reaches every DTREATY_* test off that byte,
            // where one has_treaty() call per bit recomputes the address.
            // Same value, same tests: has_treaty is exactly `word & treaty`.
            uint32_t treaties = PlayersData[i].diplo_treaties[j];
            if (j != faction_id
                && !(treaties & DTREATY_PACT)
                && ((!i && (treaties & DTREATY_VENDETTA) && !(treaties & DTREATY_TREATY)
                    && (treaties & DTREATY_COMMLINK))
                    || (i == 1 && !(treaties & DTREATY_TREATY)
                        && (treaties & DTREATY_COMMLINK))
                    || (i == 2 && (treaties & DTREATY_COMMLINK)) || (i == 3))) {
                if (PlayersData[faction_id].enemy_best_weapon_value
                    <= PlayersData[j].best_weapon_value) {
                    PlayersData[faction_id].enemy_best_weapon_value
                        = PlayersData[j].best_weapon_value;
                }
                if (PlayersData[faction_id].enemy_best_armor_value
                    <= PlayersData[j].best_armor_value) {
                    PlayersData[faction_id].enemy_best_armor_value
                        = PlayersData[j].best_armor_value;
                }
                if (PlayersData[faction_id].enemy_best_land_speed
                    <= PlayersData[j].best_land_speed) {
                    PlayersData[faction_id].enemy_best_land_speed = PlayersData[j].best_land_speed;
                }
                if (PlayersData[faction_id].enemy_best_psi_offense
                    <= PlayersData[j].best_psi_offense) {
                    PlayersData[faction_id].enemy_best_psi_offense
                        = PlayersData[j].best_psi_offense;
                }
                if (PlayersData[faction_id].enemy_best_psi_defense
                    <= PlayersData[j].best_psi_defense) {
                    PlayersData[faction_id].enemy_best_psi_defense
                        = PlayersData[j].best_psi_defense;
                }
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
Purpose: Tell the local player that a pact between two factions has turned
         into a joint attack - the AI announcing that its pact partner is
         coming for you.

         Every parse_says call here sets the gender/plurality globals the
         parser reads first; the image stores them one at a time in the order
         written, and the order differs between the first call and the rest.
// ORIGINAL: 0x0055C730 ?pact_attacks@@YAXHH@Z 0x0055C730-0x0055C833 BYTE_EXACT
// size      259 bytes
// prototype void (__cdecl ?pact_attacks@@YAXHH@Z)(int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0058FCE0 0x00625EC0
Status: homed 2026-09-03 out of the recovered-units archive
*/
void __cdecl pact_attacks(int faction_id, int faction_id_with) {
    PluralityDefault = FALSE;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(0, Players[faction_id].name_leader, -1, -1);

    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(1, Players[faction_id].search_key, -1, -1);

    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(2, Players[faction_id].noun_faction, -1, -1);

    parse_says(3, get_he_she(faction_id, 0), -1, -1);

    GenderDefault = Players[faction_id_with].noun_gender;
    PluralityDefault = Players[faction_id_with].is_noun_plural;
    parse_says(4, Players[faction_id_with].noun_faction, -1, -1);

    NetMsgPrimary.pop(DiploLabelPACTATTACKS, 5000, 0, 0);
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
        int offset = (faction_id * SpyingStatusStride + faction_id_with)
                     * static_cast<int>(sizeof(uint32_t));
        *reinterpret_cast<uint32_t *>(reinterpret_cast<char *>(DiploLoanBalanceTable)
                                      + offset) = balance;
        *reinterpret_cast<uint32_t *>(reinterpret_cast<char *>(DiploLoanPaymentTable)
                                      + offset) = payment;
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
        if (GameState & STATE_UNK_800) {
            return 0;
        }
        PopupWaitActive = 1;
        wait_loop();
        PopupWaitActive = 0;
    }
    return 1;
}

int PopupWaitActive;  // 0x0093FAC4

/*
Purpose: Tell the local player that two factions have signed a truce with
         each other.

         Seven parse_says calls, both factions' leader and faction nouns plus
         the pronoun the second sentence needs, and the same
         set-globals-then-speak pattern pact_attacks uses.
// ORIGINAL: 0x0055D040 ?pact_truce@@YAXHH@Z 0x0055D040-0x0055D18F BYTE_EXACT
// size      321 bytes
// prototype void (__cdecl ?pact_truce@@YAXHH@Z)(int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0058FCE0 0x00625EC0
Status: homed 2026-09-03 out of the recovered-units archive
*/
void __cdecl pact_truce(int faction_id, int faction_id_with) {
    PluralityDefault = FALSE;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(0, Players[faction_id].name_leader, -1, -1);
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(1, Players[faction_id].search_key, -1, -1);
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(2, Players[faction_id].noun_faction, -1, -1);
    parse_says(3, get_he_she(faction_id, 0), -1, -1);

    PluralityDefault = FALSE;
    GenderDefault = Players[faction_id_with].is_leader_female;
    parse_says(4, Players[faction_id_with].name_leader, -1, -1);
    GenderDefault = Players[faction_id_with].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(5, Players[faction_id_with].search_key, -1, -1);
    GenderDefault = Players[faction_id_with].noun_gender;
    PluralityDefault = Players[faction_id_with].is_noun_plural;
    parse_says(6, Players[faction_id_with].noun_faction, -1, -1);

    NetMsgPrimary.pop(DiploLabelPACTTRUCE, 5000, 0, 0);
}

/*
Purpose: Tell the local player that a pact between two factions has been
         dissolved - the same seven-sentence shape as pact_truce, with the
         possessive pronoun and the old-pact description added.
// ORIGINAL: 0x0055CCA0 ?pact_unpact@@YAXHH@Z 0x0055CCA0-0x0055CE1D
// size      373 bytes
// prototype void (__cdecl ?pact_unpact@@YAXHH@Z)(int, int)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0058FB00 0x0058FBC0 0x0058FCE0 0x00625EC0
// LEVER: the tail block keeps the record base in ESI across all three
//        parse_says calls in the image, which the direct `Players[...]`
//        spelling reproduces (107 of 117) - naming a `Player &` local instead
//        hoists the base to the top of the block and cost 37 instructions of
//        agreement.
// TRIED: a `Player &other` local for the second faction's three calls -
//        70 of 117, first divergence at instruction 8. Residual: from
//        instruction 72 the image pushes the two `-1` arguments BEFORE
//        computing the record base and storing the two globals; this tree
//        emits the zero for PluralityDefault first. Two sentences of
//        scheduling, not control flow.
Status: homed 2026-09-03 out of the recovered-units archive
*/
void __cdecl pact_unpact(int faction_id, int faction_id_with) {
    PluralityDefault = FALSE;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(0, Players[faction_id].name_leader, -1, -1);
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(1, Players[faction_id].search_key, -1, -1);
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(2, Players[faction_id].noun_faction, -1, -1);
    parse_says(3, get_he_she(faction_id, 0), -1, -1);
    parse_says(4, get_his_her(faction_id, 0), -1, -1);
    parse_says(5, get_pact_hood(faction_id, faction_id_with), -1, -1);

    PluralityDefault = FALSE;
    GenderDefault = Players[faction_id_with].is_leader_female;
    parse_says(6, Players[faction_id_with].name_leader, -1, -1);
    GenderDefault = Players[faction_id_with].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(7, Players[faction_id_with].search_key, -1, -1);
    GenderDefault = Players[faction_id_with].noun_gender;
    PluralityDefault = Players[faction_id_with].is_noun_plural;
    parse_says(8, Players[faction_id_with].noun_faction, -1, -1);

    NetMsgPrimary.pop(DiploLabelPACTUNPACT, 5000, 0, 0);
}

/*
Purpose: Two AI factions that were on the same side turn on each other. The
         local player only hears about it if neither of them has a commlink to
         him, and the pact is then dissolved both ways.

         Everything here reads the pairwise status table the treaty writers
         maintain, indexed `faction * 2099 + faction_with` - that index is
         SpyingStatusStride, and 2099 dwords is one 0x20CC PlayerData stride.
// ORIGINAL: 0x0055D2A0 ?enemies_team_up@@YAXHHH@Z 0x0055D2A0-0x0055D444
// size      389 bytes
// prototype void (__cdecl ?enemies_team_up@@YAXHHH@Z)(int, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00539C00 0x0055B870 0x00625EC0
// TRIED: computing the second pair index inside the branch (59 of 146) and
//        hoisting it above the guard (61 of 146) - the image loads
//        `faction_id` into EDI and `a3` into ESI in its first two instructions
//        and defers `faction_id_with` to EBX until the second index is built,
//        which neither order reproduces; this tree never pushes EDI that
//        early. That is the callee-saved-register-dedication wall, not a
//        source-shape difference: the archive's own transcription plateaued at
//        381 of 389 bytes on it.
// TRIED: `is_human(a3)` for the LockEnableMask test - the image does an 8-bit
//        `shl edx, cl / test dl, al` against the byte view, where BOOL's
//        32-bit result costs a widen, so the explicit
//        `(uint8_t)(1 << a3) & factions_status_byte(0)` is what lands here.
Status: homed 2026-09-03 out of the recovered-units archive
*/
void __cdecl enemies_team_up(int faction_id, int faction_id_with,
                             int turncoat_faction) {
    // The image reuses the two parameter slots for the pair indexes it
    // computes, so these are assignments to the parameters' own locals.
    int index = turncoat_faction + faction_id * SpyingStatusStride;
    if (!(SpyingStatusTable[index] & 0x10)
        && (static_cast<uint8_t>(1 << turncoat_faction) & factions_status_byte(0)) != 0) {
        int index_with = turncoat_faction + faction_id_with * SpyingStatusStride;
        if (SpyingStatusTable[index_with] & 0x10) {
            if (great_satan(turncoat_faction, 0) == 0 && !(SpyingStatusTable[index] & 0x20)) {
                treaty_on(faction_id, turncoat_faction, 0x2000);
                return;
            }
            if (turncoat_faction == ConsoleGlobal.faction_id_) {
                PluralityDefault = Players[faction_id].is_noun_plural;
                GenderDefault = Players[faction_id].noun_gender;
                parse_says(0, Players[faction_id].noun_faction, -1, -1);
                NetMsgPrimary.pop(DiploLabelTEAM, 5000, 0, 0);
            }
            DiploSpokeTable[turncoat_faction * SpyingStatusStride + faction_id_with] = TurnCurrentNum;
            DiploSpokeTable[turncoat_faction * SpyingStatusStride + faction_id] = TurnCurrentNum;
            treaty_on(faction_id, turncoat_faction, 0x10);
            return;
        }
        if (great_satan(turncoat_faction, 0) == 0 && !(SpyingStatusTable[index] & 0x20)
            && !(SpyingStatusTable[index_with] & 0x20)) {
            treaty_on(faction_id, turncoat_faction, 0x2000);
            return;
        }
        SpyingStatusTable[index] |= 0x400;
    }
}

// Shared diplomacy evaluation state. Neutral names retain independently
// observed original slots until their meanings can be named conclusively.
// RECOVERY_OBJECT: v1 0x00945DD8 ?FactionRankingsUnk@@3PAIA
uint32_t FactionRankingsUnk[MaxPlayerNum];
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
// RECOVERY_OBJECT: v1 0x0093FA2C ?DiploScratchFA2C@@3HA
int DiploScratchFA2C;
// RECOVERY_OBJECT: v1 0x0093FA38 ?DiploScratchFA38@@3HA
int DiploScratchFA38;
// RECOVERY_OBJECT: v1 0x0093FA3C ?DiploScratchFA3C@@3HA
int DiploScratchFA3C;
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

    if ((GameRules & 0x2000000) == 0) {
        for (int i = 0; i < 0x59; i++) {
            if (!has_tech(i, faction_id_with)) {
                if (has_tech(i, faction_id)) {
                    int v = tech_val(i, faction_id_with, 0);
                    int oldA20 = DiploScratchFA20;
                    int oldA814 = DiploScratchF814;
                    if (v < DiploScratchF814) {
                        if (v < DiploScratchFA44) {
                            if (v < DiploScratchFA20) {
                                if (DiploScratchF7C0 <= v) {
                                    DiploScratchF7C0 = v;
                                    DiploScratchFA28 = i;
                                }
                            } else {
                                DiploScratchF7C0 = DiploScratchFA20;
                                DiploScratchFA28 = DiploScratchFA1C;
                                DiploScratchFA1C = i;
                                DiploScratchFA20 = v;
                            }
                        } else {
                            DiploScratchFA20 = DiploScratchFA44;
                            DiploScratchF7C0 = oldA20;
                            DiploScratchFA28 = DiploScratchFA1C;
                            DiploScratchFA1C = DiploScratchFA18;
                            DiploScratchFA18 = i;
                            DiploScratchFA44 = v;
                        }
                    } else {
                        DiploScratchFA28 = DiploScratchFA1C;
                        DiploScratchFA1C = DiploScratchFA18;
                        DiploScratchFA18 = DiploScratchFAA8;
                        DiploScratchF7C0 = DiploScratchFA20;
                        DiploScratchF814 = v;
                        DiploScratchFA20 = DiploScratchFA44;
                        DiploScratchFA44 = oldA814;
                        DiploScratchFAA8 = i;
                    }
                }
            } else if (!has_tech(i, faction_id)) {
                int matched = -1;
                if (PlayersData[faction_id].diff_level > 2 ||
                    strcmp(Players[faction_id_with].search_key, "UNIV") == 0) {
                    for (int project = 0; project < MaxSecretProjectNum; ++project) {
                        if (base_project(project) != SP_Unbuilt ||
                            Facility[70 + project].preq_tech != i) continue;
                        const int rank = -2 - project;
                        const int base_count = BaseCurrentCount;
                        for (int base = 0; base < base_count; ++base) {
                            if (Bases[base].faction_id_current == faction_id_with &&
                                Bases[base].queue_production_id[0] == rank - 0x44) {
                                DiploScratchF7F4 = project;
                                matched = project;
                            }
                        }
                        if (((PlayersData[faction_id].diff_level > 4 &&
                              (!IsMultiplayerNet || faction_id != static_cast<int>(RankingFactionIDUnk2))) ||
                             (IsMultiplayerNet && faction_id == static_cast<int>(RankingFactionIDUnk1))) &&
                            DiploScratchF7F4 == -1) {
                            DiploScratchF7F4 = rank;
                            matched = rank;
                        }
                    }
                }
                int v = tech_val(i, faction_id, 0);
                if (matched != -1) {
                    // Native INC wraps before the signed, toward-zero divide.
                    v = static_cast<int>(static_cast<uint32_t>(v) + 1u) / 2;
                }
                if (v < DiploScratchFAA4) {
                    if (DiploScratchF7EC <= v) {
                        DiploScratchF7EC = v;
                        DiploScratchF80C = i;
                        DiploScratchFA84 = matched;
                    }
                } else {
                    DiploScratchF7EC = DiploScratchFAA4;
                    DiploScratchFA84 = DiploScratchFA6C;
                    DiploScratchF80C = DiploScratchF800;
                    DiploScratchF800 = i;
                    DiploScratchFA6C = matched;
                    DiploScratchFAA4 = v;
                }
            }
        }
        if (DiploScratchFA84 != -1) {
            DiploScratchF80C = -1;
        }
    }
}

static int pending_social_model(const SocialCategory &selection, int category) {
    return static_cast<int>(selection.models_[category]);
}

static int diplo_clamp(int value, int low, int high) {
    return value < low ? low : value > high ? high : value;
}
static uint32_t diplo_bit(int faction) { return 1u << (faction & 31); }
// Native i386 word arithmetic; preserve wrapping before signed division.
static int diplo_add(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b)); }
static int diplo_sub(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b)); }
static int diplo_mul(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b)); }
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
    DiploScratchFA60 = DiploScratchFAA0 = DiploScratchFA88 = DiploScratchFA98 = 0;
    DiploScratchFA80 = DiploScratchFA50 = DiploScratchFA58 = DiploScratchFA2C = 0;
    DiploScratchFA64 = at_climax(faction);
    DiploScratchFA90 = static_cast<int>(PlayersData[faction].ranking) < static_cast<int>(PlayersData[other].ranking);
    const int satan = great_satan(faction, 0);
    if (!satan || static_cast<int>(PlayersData[other].current_num_bases) < 2
        || ((PlayersData[faction].diplo_treaties[other] & 1)
            && (static_cast<int>(PlayersData[other].ranking) < 5 || ((GameRules & 0x1000) && TurnCurrentNum > 199))))
        DiploScratchFA3C = 0;
    else DiploScratchFA3C = 1;
    DiploScratchF7F0 = diplo_mul(static_cast<int>(FactionRankingsUnk[faction]), 2)
        < static_cast<int>(FactionRankingsUnk[FactionRankings[7]]) && PlayersData[other].ranking != 7;
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
    int faction_history = diplo_add(diplo_add(PlayersData[faction].diplo_unk4[other], 1), TurnCurrentNum / 50);
    int other_history = diplo_add(diplo_add(PlayersData[other].diplo_unk4[faction], 1), TurnCurrentNum / 50);
    if (!(PlayersData[faction].diplo_treaties[other] & 0x10)) {
        faction_history = diplo_add(faction_history, diplo_add(diplo_add(2, PlayersData[faction].diplo_unk3[other]), TurnCurrentNum / 25));
        other_history = diplo_add(other_history, diplo_add(diplo_add(2, PlayersData[other].diplo_unk3[faction]), TurnCurrentNum / 25));
    }
    int fight = PlayersData[other].ai_fight;
    DiploScratchFA54 = DiploScratchF7E0 = DiploScratchF7FC = DiploScratchF7F8 = 0;
    if ((PlayersData[other].diplo_treaties[faction] & 0x20) && fight < 0) fight = diplo_sub(0, fight);
    int maximum_bases[7];
    for (int i = 0; i < 7; ++i) maximum_bases[i] = 1;
    const uint8_t alive = factions_status_byte(1);
    for (int allied_faction = 1; allied_faction < 8; ++allied_faction) {
        if (alive & diplo_bit(allied_faction)) {
            if ((PlayersData[faction].diplo_treaties[allied_faction] & 1) && !(PlayersData[allied_faction].diplo_treaties[faction] & 0x2000000)) ++DiploScratchFA54;
            if (PlayersData[other].diplo_treaties[allied_faction] & 1) {
                if (static_cast<int>(PlayersData[allied_faction].ranking) > 4 && static_cast<int>(PlayersData[other].ranking) < 5 && allied_faction != faction && !DiploScratchF7F0) ++DiploScratchF7FC;
                ++DiploScratchF7E0;
                DiploScratchF7F8 = diplo_add(DiploScratchF7F8, static_cast<int>(FactionRankingsUnk[allied_faction]));
            }
        }
    }
    if (update) DiploScratchF7E4 = PlayersData[faction].diplo_spoke[other];
    const int faction_morale = diplo_add(diplo_add(Players[faction].rule_morale, 16), diplo_clamp(PlayersData[faction].soc_effect_pending.morale, -4, 4));
    const int other_morale = diplo_add(diplo_add(Players[other].rule_morale, 16), diplo_clamp(PlayersData[other].soc_effect_pending.morale, -4, 4));
    for (int cleared_region = 1; cleared_region < 128; ++cleared_region)
        DiplomacyRegionMask[cleared_region] = 0;
    for (int census_region = 1; census_region < 128; ++census_region)
        for (int census_faction = 1; census_faction < 8; ++census_faction)
            if (maximum_bases[census_faction - 1] < PlayersData[census_faction].region_total_bases[census_region])
                maximum_bases[census_faction - 1] = PlayersData[census_faction].region_total_bases[census_region];

    for (int base = 0; base < BaseCurrentCount; ++base) {
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
            DiploScratchFA88 = diplo_sub(DiploScratchFA88, diplo_mul(PlayersData[faction].unk_13 / static_cast<int>(PlayersData[faction].current_num_bases), difference));
            if (PlayersData[other].diplo_treaties[faction] & 0x2000) DiploScratchFA88 /= 2;
        } else {
            const int difference = diplo_clamp(diplo_add(diplo_sub(PlayersData[faction].tech_commerce_bonus, PlayersData[other].tech_commerce_bonus), fight), 0, 99);
            DiploScratchFA88 = diplo_mul(diplo_add(diplo_mul(PlayersData[other].unk_13, difference) / static_cast<int>(PlayersData[other].current_num_bases), DiploScratchFA88), 2);
        }
    }
    if (diplo_mul(PlayersData[other].best_weapon_value, 3) / 2 <= PlayersData[faction].best_weapon_value
        || diplo_mul(PlayersData[other].best_armor_value, 2) < PlayersData[faction].best_weapon_value)
        DiploScratchFA98 = diplo_mul(DiploScratchFA98, 2);
    if (diplo_mul(other_history, 2) <= faction_history) DiploScratchFA98 = diplo_mul(DiploScratchFA98, 2);
    if (diplo_mul(other_history, 4) <= faction_history && !(PlayersData[other].diplo_treaties[faction] & 0x20) && DiploScratchFA98 < 9) DiploScratchFA98 = 8;
    if (diplo_mul(faction_history, 2) < other_history) DiploScratchFA98 /= 2;
    if (diplo_mul(faction_history, 4) < other_history) DiploScratchFA98 /= 2;
    if (DiploScratchFA98 || (PlayersData[faction].diff_level <= ((PlayersData[other].diplo_treaties[faction] & 0x20) ? 2 : 4) && !(GameRules & 0x40))) DiploScratchFAA0 = 0;
    DiploScratchFA88 = diplo_mul(diplo_add(TurnCurrentNum / 10, other_history), DiploScratchFA88) / diplo_add(faction_history, TurnCurrentNum / 10);
    const int grace = IsMultiplayerNet ? 0 : diplo_mul(diplo_sub(5, PlayersData[faction].diff_level), 25);
    bool reconsider;
    const int faction_rank = static_cast<int>(PlayersData[faction].ranking);
    if (TurnCurrentNum < diplo_add(grace, 50) || DiploScratchF7F0 || faction_rank < (DiffLevelCurrent < 3 ? 6 : 5)) reconsider = true;
    else if (static_cast<int>(PlayersData[other].ranking) < (faction_rank < 6 ? 6 : 5)) {
        if (!DiploScratchF7FC || !(PlayersData[faction].diplo_treaties[other] & 0x8000000) || faction_rank < 6) reconsider = true;
        else { DiploScratchFA3C = 1; reconsider = false; }
    } else { DiploScratchFA3C = 1; reconsider = false; }
    if (!reconsider || DiploScratchFA3C || (((PlayersData[other].diplo_treaties[faction] & 0x20)
        || (PlayersData[other].diplo_agenda[faction] & 0x200)) && !DiploScratchFA58
        && (!DiploScratchFA98 || diplo_mul(PlayersData[other].mil_strength_1, 2) >= PlayersData[faction].mil_strength_1))) {
        DiploScratchFA98 = 0;
        DiploScratchFA60 = (DiploScratchFA3C != 0) + 1;
        const int minimum = diplo_mul(static_cast<int>(PlayersData[other].ranking), 2);
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
    const bool intense = (GameRules & 0x40) != 0;
    int difficulty = intense ? 5 : PlayersData[faction].diff_level;
    const int reputation = diplo_clamp(PlayersData[faction].integrity_blemishes / 2, 1, 99);
    DiploScratchFA98 /= diplo_mul(diplo_add(difficulty, 3), diplo_add(reputation, 1)) / 2;
    DiploScratchFA4C = DiploScratchF7B8 = 0;
    if (!pact || !(PlayersData[other].diplo_treaties[faction] & 0x2000000)) {
        for (int ally = 1; ally < 8; ++ally) {
            if ((factions_status_byte(1) & diplo_bit(ally)) && ally != faction && ally != other
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
    if (!(GameRules & 0x800) && !PlayersData[other].se_alloc_labs) DiploScratchFA60 = 1;
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
        && static_cast<int>(PlayersData[faction].ranking) > 5) {
        const int minimum = diplo_mul(diplo_add(static_cast<int>(PlayersData[other].ranking), static_cast<int>(PlayersData[faction].ranking)), energy) / 21;
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
            && diplo_mul(diplo_mul(PlayersData[faction].energy_reserves, 2) / 3, static_cast<int>(PlayersData[other].ranking))
                / static_cast<int>(PlayersData[faction].ranking) <= PlayersData[other].energy_reserves) {
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
        if (!(factions_status_byte(1) & bit) || ally == faction || ally == other || !(other_treaty & 8)) continue;
        if (update && (PlayersData[faction].diplo_treaties[ally] & 0x10) && !(other_treaty & 0x10)
            && !(PlayersData[faction].diplo_treaties[ally] & 0x10000000)) DiploScratchF7D8 = ally;
        if (!(other_treaty & 1)) {
            if (!(other_treaty & 0x10)) {
                if (update && !(factions_status_byte(0) & bit)) {
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
                    else if (!DiploScratchF7E8 && update && !(faction_treaty & 8) && !(factions_status_byte(0) & bit)) {
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
    if (!(GameRules & 0x800) && !PlayersData[other].se_alloc_labs) --DiploScratchFA9C;
    if (PlayersData[faction].mil_strength_1 < PlayersData[other].mil_strength_1) --DiploScratchFA9C;
    if (diplo_mul(PlayersData[faction].best_armor_value, 2) < PlayersData[other].best_weapon_value) --DiploScratchFA9C;
    if (static_cast<int>((PlayersData[faction].diplo_treaties[other] >> 4) & 1) < DiploScratchFA9C
        && !DiploScratchFA3C && !(treaty & 0x41) && !DiploScratchFAA0) { DiploScratchFA60 = DiploScratchFA88 = 0; }
    if (!(treaty & 1) || !(treaty & 0x2000000)) {
        if ((DiploScratchFA50 || DiploScratchFAA0) && surrendered) DiploScratchFA68 = DiploScratchFA60 = 1;
    } else DiploScratchFAA0 = DiploScratchFA50 = DiploScratchFA68 = DiploScratchFA60 = 0;
    const int final_satan = great_satan(faction, 1);
    if (!final_satan || DiploScratchF7E0 > 1
        || static_cast<int>(FactionRankingsUnk[faction]) < diplo_add(DiploScratchF7F8, diplo_mul(static_cast<int>(FactionRankingsUnk[other]), 3))
        || static_cast<int>(PlayersData[other].ranking) > 5 || PlayersData[faction].mil_strength_1 <= PlayersData[other].mil_strength_1) DiploScratchFA24 = 0;
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
        && static_cast<int>(PlayersData[other].ranking) < static_cast<int>(PlayersData[faction].ranking)
        && ((GameRules & 0x1000) || DiploScratchF808)) DiploScratchFA60 = 0;
    if (PlayersData[other].diplo_agenda[faction] & 0x2000) {
        DiploScratchFA70 = 0;
        DiploScratchFA60 = (PlayersData[other].diplo_treaties[faction] & 0x10) >> 4;
    }
    if (!DiploScratchFA68 && (DiploScratchFA98 || (treaty & 1)) && DiploFriction > 14) DiploFriction = 14;
    if ((DiploScratchFA60 || (treaty & 0x10) || (PlayersData[other].diplo_treaties[faction] & 0x20)) && DiploFriction < 6) DiploFriction = 6;
}

static void lifecycle_clear_social(SocialEffect &effect) {
    SocialEffect zero = {0};
    effect = zero;
}

static void lifecycle_add_social(SocialEffect &effect, int index, int value) {
    // The bonus axis is the original11 signed social-effect words. Use
    // genuine declared members rather than indexing across scalar neighbors.
    // Index validation remains the caller/parser's original responsibility.
    static int SocialEffect::* const fields[11] = {
        &SocialEffect::economy, &SocialEffect::efficiency, &SocialEffect::support,
        &SocialEffect::talent, &SocialEffect::morale, &SocialEffect::police,
        &SocialEffect::growth, &SocialEffect::planet, &SocialEffect::probe,
        &SocialEffect::industry, &SocialEffect::research
    };
    effect.*fields[index] += value;
}

static void lifecycle_reset_faction(int faction) {
    PlayerData &p = PlayersData[faction];
    p.flags = 0;
    p.base_governor_adv = DiffLevelCurrent ? 0xdfbe41 : 0xc0dfbe41;
    p.base_name_offset = p.base_sea_name_offset = 0;
    p.ai_fight = Players[faction].ai_fight;
    p.ai_growth = Players[faction].ai_growth;
    p.ai_tech = Players[faction].ai_tech;
    p.ai_wealth = Players[faction].ai_wealth;
    p.ai_power = Players[faction].ai_power;
    p.unk_37 = 0;
    for (int queue = 0; queue < 8; ++queue) {
        p.saved_queue_size[queue] = 0;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x984 / 4));
        strcat(StringTemp, " ");
        char number[80];
        _itoa(queue + 1, number, 10);
        strcat(StringTemp, number);
        strcpy(p.saved_queue_name[queue], StringTemp);
    }
    for (int category = 0; category < 4; ++category)
        p.soc_category_pending.models_[category] = p.soc_category_active.models_[category] = 0;
    lifecycle_clear_social(p.soc_effect_pending);
    p.soc_upheaval_cost_paid = 0;
    for (int other = 0; other < 8; ++other) p.secret_project_intel[other] = 0;
    p.diff_level = (LockEnableMask & lifecycle_faction_mask(faction)) ? DiffLevelCurrent : 3;
    p.integrity_blemishes = p.global_reputation = 0;
    p.traded_maps = p.energy_reserves = p.atrocities = p.major_atrocities = 0;
    p.planet_ecology = p.sanction_turns = 0;
    TectonicDetonationCount[faction] = 0;
    p.council_call_turn = -999;
    if (GameState & 0x800000) {
        p.ai_fight = rand() % 3 - 1;
        p.ai_power = rand() % 2;
        p.ai_tech = rand() % 2;
        p.ai_wealth = rand() % 2;
        p.ai_growth = rand() % 2;
    }
    if (GameRules & 0x40) p.ai_fight = 1;
    p.total_mil_veh = p.current_num_bases = p.mil_strength_1 = p.mil_strength_2 = p.pop_total = p.unk_70 = 0;
    p.mind_control_total = 0;
    for (int other2 = 0; other2 < 8; ++other2)
        p.diplo_mind_control[other2] = p.stolen_data_count[other2] = 0;
    p.net_random_event = 0;
    for (int region = 0; region < 128; ++region) {
        p.region_total_combat_vehs[region] = p.region_total_bases[region] = 0;
        p.region_total_offensive_vehs[region] = p.region_force_rating[region] = p.unk_77[region] = 0;
    }
    for (int council = 0; council < 11; ++council) p.unk_29[council] = p.unk_30[council] = 0;
    p.corner_market_turn = p.corner_market_active = p.unk_101 = p.unk_102 = p.prototype_history_cursor = 0;
    p.flags_ext = p.unk_105 = p.unk_106 = p.unk_107 = p.unk_108 = p.unk_109 = 0;
    p.unk_93 = p.unk_94 = p.unk_95 = p.unk_96 = p.unk_97 = p.tech_achieved = 0;
    p.time_bonus_count = 1;
}

// ORIGINAL: 0x005B0E00 ?setup_player@@YAHHHH@Z 0x005B0E00-0x005B337C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005b0e00.cpp: canonical original identity, full spans and retained seed provenance", "source:src/faction.h: real faction record ownership and typed relationships", "source:recovery/tasks/faction-lifecycle-owner-closure.json: coordinator authorization and retained task baseline"], "reason": "The original faction setup operation belongs to the Faction owner; real Player/PlayerData records, prototype scoring, owned network state, shared game counters and named base/vehicle dependencies express creation and respawn paths without seed executable addresses."}
int __cdecl setup_player(int faction, int conqueror, int retry) {
    Player &definition = Players[faction];
    PlayerData &p = PlayersData[faction];
    char number[80];
    char prototype_name[32];
    int duplicate = 0;
    for (int previous = 1; previous < faction; ++previous)
        if (!strcmp(Players[previous].name_leader, definition.name_leader)) ++duplicate;
    if (duplicate) {
        definition.name_leader[21] = 0;
        strcat(definition.name_leader, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(definition.name_leader, number);
    }
    duplicate = 0;
    for (int previous2 = 1; previous2 < faction; ++previous2)
        if (!strcmp(Players[previous2].noun_faction, definition.noun_faction)) ++duplicate;
    if (duplicate) {
        definition.noun_faction[21] = 0;
        strcat(definition.noun_faction, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(definition.noun_faction, number);
    }
    duplicate = 0;
    for (int previous3 = 1; previous3 < faction; ++previous3)
        if (!strcmp(Players[previous3].formal_name_faction, definition.formal_name_faction)) ++duplicate;
    if (duplicate) {
        definition.formal_name_faction[37] = 0;
        strcat(definition.formal_name_faction, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(definition.formal_name_faction, number);
    }
    for (int base = BaseCurrentCount - 1; base >= 0; --base)
        if (Bases[base].faction_id_current == faction) base_kill(base);
    for (int vehicle = VehCurrentCount - 1; vehicle >= 0; --vehicle)
        if (vehicle < VehCurrentCount && Vehs[vehicle].faction_id == faction
            && (Vehs[vehicle].proto_id != 0x13 || faction != 0)) kill(vehicle);
    p.x_target = p.y_target = p.base_id_atk_target = -1;
    if (lifecycle_is_fresh(conqueror)) {
        init_goals(faction);
        clear_bunglist(faction);
        lifecycle_reset_faction(faction);
    }
    p.energy_reserves += definition.rule_energy
        + (IsMultiplayerNet ? 100 : Rules.player_start_energy_reserve);
    p.hurry_cost_total = 0;
    p.tech_cost = -1;
    p.last_turn_new_base = TurnCurrentNum;
    for (int other = 0; other < 8; ++other) {
        if (lifecycle_is_fresh(conqueror)) {
            p.diplo_treaties[other] = p.diplo_merc[other] = 0;
            int limit = ((GameRules & 0x40) ? 5 : DiffLevelCurrent) * 3 + 8;
            int friction = limit <= 1 ? 1 : rand() % limit;
            if (friction < 1) friction = 1;
            if (friction > 16) friction = 16;
            p.diplo_friction[other] = friction;
            p.diplo_wrongs[other] = p.diplo_betrayed[other] = p.diplo_unk3[other] = 0;
            p.diplo_patience[other] = 0;
            p.diplo_spoke[other] = -1;
            p.diplo_unk4[other] = p.diplo_unk1[other] = 0;
        } else treaty_off(faction, other, 0x9000ff48);
        p.loan_balance[other] = p.loan_payment[other] = p.unk_12[other] = 0;
    }
    for (int proto = 0; proto < 512; ++proto) {
        p.proto_id_active[proto] = p.proto_id_queue[proto] = 0;
        if (TurnCurrentNum == 0) p.proto_id_lost[proto] = 0;
    }
    if (lifecycle_is_fresh(conqueror)) {
        for (int goal = 0; goal < 12; ++goal) p.tech_pact_shared_goals[goal] = 0;
        // The original clears22 words and then one byte, leaving the last
        // three bytes of this92-byte field unchanged.
        for (int source = 0; source < 89; ++source) p.tech_trade_source[source] = 0;
        for (int announced = 0; announced < 9; ++announced) p.facility_announced[announced] = 0;
        for (int satellite = 0; satellite < 4; ++satellite) p.satellites_[satellite] = 0;
        p.theory_of_everything = p.tech_ranking = p.sat_odp_deployed = 0;
        p.tech_id_researching = -1;
        p.tech_accumulated = p.earned_techs_saved = p.unk_26 = p.unk_102 = 0;
    }
    p.unk_28 = p.unk_13 = p.maint_cost_total = p.unk_18 = p.unk_17 = 0;
    p.tech_commerce_bonus = p.turn_commerce_income = 0;
    p.tech_fungus_nutrient = p.tech_fungus_mineral = p.tech_fungus_energy = p.tech_fungus_unk = 0;
    p.best_weapon_value = p.best_armor_value = p.best_land_speed = 0;
    p.enemy_best_weapon_value = p.enemy_best_armor_value = p.enemy_best_land_speed = 0;
    p.best_psi_offense = p.best_psi_defense = p.enemy_best_psi_offense = p.enemy_best_psi_defense = 0;
    if (!faction || !((LockEnableMask >> 8) & lifecycle_faction_mask(faction))) return 0;
    if (conqueror < -1 && conqueror != -282) {
        LockEnableMask &= ~(static_cast<uint32_t>(lifecycle_faction_mask(faction)) << 8);
        return 0;
    }

    int attempts = 0;
    int x = 0, y = 0, region = 0, nearest = -1, occupied = 0, isolation = 32;
    int failed = 0;
    int area_quality = 0;
    int spacing_relaxation = 0;
    Map *tile = 0;
search:
    {
        int margin = (1024 - attempts) * MapLongitudeBounds / 8192;
        if (margin < 0) margin = 0;
        if (margin > 9999) margin = 9999;
        int inset = (MapIsFlat & 1) ? margin * 2 : 0;
        int width = MapLongitudeBounds - inset;
        x = ((MapIsFlat & 1) ? margin : 0) + (width <= 1 ? 0 : rand() % width);
        margin = (1024 - attempts) * MapLatitudeBounds / 8192;
        if (margin < 0) margin = 0;
        if (margin > 9999) margin = 9999;
        int height = MapLatitudeBounds - 2 * margin;
        y = margin + (height <= 1 ? 0 : rand() % height);
        if (x & 1) --x;
        if (y & 1) ++x;
        isolation = 32;
        for (int ring = 0; ring < 8; ++ring) {
            const int begin = ring ? RadiusRange[ring] : 0;
            for (int offset = begin; offset < RadiusRange[ring + 1]; ++offset) {
                int rx = lifecycle_wrap_x(x + RadiusOffsetX[offset]);
                int ry = y + RadiusOffsetY[offset];
                if (rx >= 0 && rx < MapLongitudeBounds && ry >= 0 && ry < MapLatitudeBounds) {
                    uint32_t landmark = map_loc(rx, ry)->bit2;
                    if (landmark && static_cast<int>(landmark & 0xff000000) > 0) {
                        isolation = ring;
                        ring = 99;
                        break;
                    }
                }
            }
        }
        area_quality = 0;
        for (int offset2 = 0; offset2 < 21; ++offset2) {
            int rx = lifecycle_wrap_x(x + RadiusOffsetX[offset2]);
            int ry = y + RadiusOffsetY[offset2];
            if (rx < 0 || rx >= MapLongitudeBounds || ry < 0 || ry >= MapLatitudeBounds) continue;
            Map &square = *map_loc(rx, ry);
            const int altitude = square.climate & 0xe0;
            if (!(definition.rule_flags & 0x100)) {
                if ((!(square.bit & 0x20) || altitude < 0x40)
                    && altitude > 0x5f && (square.val3 & 0xc0) < 0x41
                    && (square.climate & 0x18)) ++area_quality;
            } else if (!(square.bit & 0x20) || altitude < 0x40)
                area_quality += (2 - (square.climate >> 5)) * 2 + 1;
        }
        if (definition.rule_flags & 0x100) area_quality /= 3;
        region = map_loc(x, y)->region;
        occupied = 0;
        for (int player = 1; player < 8; ++player)
            if (PlayersData[player].region_total_bases[region]) occupied |= 1 << player;
        nearest = (!(definition.rule_flags & 0x100) && conqueror != -282)
            ? base_find(x, y, -1, region, -1, -1) : base_find(x, y);
        if (nearest < 0) BaseFindDist = 32;
        if (lifecycle_is_fresh(conqueror)) {
            for (int vehicle2 = 0; vehicle2 < VehCurrentCount; ++vehicle2) {
                if (map_loc(Vehs[vehicle2].x, Vehs[vehicle2].y)->region != region) continue;
                occupied |= 1 << Vehs[vehicle2].faction_id;
                int distance = lifecycle_distance(x, y, Vehs[vehicle2].x, Vehs[vehicle2].y);
                if (distance < BaseFindDist) BaseFindDist = distance;
            }
        }
        ++attempts;
        if (attempts < 1025 || ((LockEnableMask & lifecycle_faction_mask(faction)) && TurnCurrentNum == 0)) {
            tile = map_loc(x, y);
            int altitude = tile->climate & 0xe0;
            bool bad_altitude = (definition.rule_flags & 0x100) ? altitude > 0x40 : altitude < 0x60;
            if (bad_altitude || ((tile->bit & 0x20) && altitude > 0x3f)
                || (!(definition.rule_flags & 0x100) && (tile->val3 & 0xc0) > 0x40)) goto search;
            int separation = lifecycle_is_fresh(conqueror) ? 6 : 12;
            spacing_relaxation = attempts / 64;
            if (separation * static_cast<int>(MapAreaSqRoot) / 56 > BaseFindDist
                || altitude >= 0xa0 || ((tile->bit & 3) && (tile->val2 & 15) <= 7)
                || y <= 1 || y >= MapLatitudeBounds - 2
                || 12 - attempts / 80 > area_quality
                || (16 - spacing_relaxation) * static_cast<int>(MapAreaSqRoot) / 56 > BaseFindDist
                || (lifecycle_is_fresh(conqueror) && attempts <= 899 && 36 - spacing_relaxation > static_cast<int>(Continents[region].tile_count))) goto search;
            if (!(definition.rule_flags & 0x100)) {
                int handicap = (LockEnableMask & lifecycle_faction_mask(faction)) ? (6 - DiffLevelCurrent) * 8 : 8;
                if (static_cast<int>(Continents[region].open_terrain * (MapOceanCoverage + 1)) / 2
                    < 24 + handicap - TurnCurrentNum / 16 - attempts / 16) goto search;
            }
            int difficulty = (LockEnableMask & lifecycle_faction_mask(faction)) ? DiffLevelCurrent : 3;
            if ((5 - difficulty) * 48 - TurnCurrentNum / 16 - attempts / 4 > static_cast<int>(Continents[region].tile_count)
                || (occupied && conqueror != -282
                    && 64 - spacing_relaxation > static_cast<int>(Continents[region].tile_count) / (bit_count(occupied) + 1))) goto search;
            int quality = (tile->bit & 0x1000) ? 0 : tile->val2 >> 4;
            if (!(tile->bit & 0x1000) && !quality) {
                quality = world_site(x, y, 0);
                site_set(x, y, quality);
            }
            if ((15 - spacing_relaxation > quality && !TurnCurrentNum && !(GameRules & 0x10))
                || (!(definition.rule_flags & 0x100) && ((1024 - attempts) / 128 > isolation || !isolation))
                || (TurnCurrentNum && conqueror != -282 && tile->territory >= 1)
                || (nearest >= 0 && Bases[nearest].faction_id_current == conqueror
                    && map_loc(Bases[nearest].x, Bases[nearest].y)->region == region)) goto search;
        } else failed = 1;
    }
    if (p.flags & 0x1000) failed = 1;
    if (!failed || retry != 0) {
        if (lifecycle_is_fresh(conqueror)) {
            int best = -1, best_offset = 0;
            for (int offset3 = 0; offset3 < 21; ++offset3) {
                int rx = lifecycle_wrap_x(x + RadiusOffsetX[offset3]);
                int ry = y + RadiusOffsetY[offset3];
                if (rx < 0 || rx >= MapLongitudeBounds || ry < 0 || ry >= MapLatitudeBounds) continue;
                Map &candidate = *map_loc(rx, ry);
                int altitude = candidate.climate & 0xe0;
                if ((altitude < 0x60) != ((definition.rule_flags >> 8) & 1)
                    || candidate.region != region || altitude >= 0xa0
                    || ((candidate.bit & 0x20) && altitude >= 0x40)
                    || ((candidate.bit & 3) && (candidate.val2 & 15) <= 7)) continue;
                base_find(rx, ry);
                // Original5B204E checks distance from the old center, not
                // from this candidate. Keep that quirk when choosing a site.
                for (int vehicle3 = 0; vehicle3 < VehCurrentCount; ++vehicle3) {
                    int distance = lifecycle_distance(x, y, Vehs[vehicle3].x, Vehs[vehicle3].y);
                    if (distance < BaseFindDist) BaseFindDist = distance;
                }
                if (BaseFindDist <= 5) continue;
                int quality = (candidate.bit & 0x1000) ? 0 : candidate.val2 >> 4;
                if (!(candidate.bit & 0x1000) && !quality) {
                    quality = world_site(rx, ry, 0);
                    site_set(rx, ry, quality);
                }
                for (int neighbor = 0; neighbor < 8; ++neighbor) {
                    int nx = lifecycle_wrap_x(rx + RadiusBaseX[neighbor]);
                    int ny = ry + RadiusBaseY[neighbor];
                    if (nx < 0 || nx >= MapLongitudeBounds || ny < 0 || ny >= MapLatitudeBounds) continue;
                    int altitude2 = map_loc(nx, ny)->climate >> 5;
                    if (!(definition.rule_flags & 0x100)) {
                        if (altitude2 < 3) { ++quality; break; }
                    } else quality += 2 - altitude2;
                }
                if (quality > best) { best = quality; best_offset = offset3; }
            }
            x = lifecycle_wrap_x(x + RadiusOffsetX[best_offset]);
            y += RadiusOffsetY[best_offset];
        }
        if (retry != 0) failed = 0;
        if (!failed && (lifecycle_is_fresh(conqueror) || !(GameRules & 1)) && TurnCurrentNum < 101) goto respawn;
    }
    if (!retry) {
        if ((LockEnableMask & lifecycle_faction_mask(faction)) && !IsMultiplayerNet) {
            EndgameExitReason = 7;
            LockEnableMask &= ~(static_cast<uint32_t>(lifecycle_faction_mask(faction)) << 8);
            GameState |= 1;
            return 0;
        }
        p.unk_102 = 0;
        LockEnableMask &= ~(static_cast<uint32_t>(lifecycle_faction_mask(faction)) << 8);
        int selected = faction == static_cast<int>(FactionRankings[7])
            ? static_cast<int>(FactionRankings[6])
            : static_cast<int>(FactionRankings[7]);
        int survivors = 0, partners = 0;
        for (int survivor = 1; survivor < 8; ++survivor) {
            if (!((LockEnableMask >> 8) & (1 << survivor))) continue;
            ++survivors;
            uint32_t treaty = PlayersData[survivor].diplo_treaties[ConsoleGlobal.faction_id_];
            if (!IsMultiplayerNet && (treaty & 1) && (treaty & 0x2000000)) { --survivors; continue; }
            if (survivor != selected && (PlayersData[survivor].diplo_treaties[selected] & 1)
                && (GameRules & 0x1000) && partners < 2) ++partners;
        }
        if (faction == ConsoleGlobal.faction_id_) {
            int successor = conqueror < 0 ? 0 : conqueror;
            GenderDefault = Players[successor].noun_gender;
            PluralityDefault = Players[successor].is_noun_plural;
            parse_says(0, Players[successor].noun_faction, -1, -1);
            if (IsMultiplayerNet) {
                NetDaemonNet.hang_up();
                // These are the original individual lock-state words, not
                // a clear of the whole surrounding network allocation.
                NetDaemonNet.lock_state_[0].leading_words_[1] = 0;
                NetDaemonNet.lock_state_[0].leading_words_[2] = 0;
                NetDaemonNet.lock_state_[0].leading_words_[3] = 0;
                NetDaemonNet.lock_state_[0].payload_.flags = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[8] = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[7] = 0;
                NetDaemonNet.lock_state_[1].leading_words_[1] = 0;
                NetDaemonNet.lock_state_[1].leading_words_[0] = 0;
                NetDaemonNet.lock_state_[1].trailing_words_[0] = 0;
                NetDaemonNet.lock_state_[2].leading_words_[1] = 0;
                NetDaemonNet.lock_state_[1].trailing_words_[9] = 0;
                NetDaemonNet.lock_state_[2].leading_words_[0] = 0;
                NetDaemonNet.lock_state_[2].trailing_words_[5] = 0;
                NetDaemonNet.lock_state_[2].trailing_words_[4] = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[4] = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[5] = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[6] = 0;
                NetDaemonNet.lock_state_[2].trailing_words_[3] = 0;
                NetDaemonNet.lock_state_[2].trailing_words_[2] = 0;
                NetDaemonNet.lock_state_[2].leading_words_[3] = 0;
                NetDaemonNet.lock_state_[2].leading_words_[2] = 0;
                NetDaemonNet.lock_state_[2].payload_.flags = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[0] = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[1] = 0;
                NetDaemonNet.lock_state_[2].trailing_words_[8] = 0;
                NetDaemonNet.lock_state_[2].trailing_words_[7] = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[2] = 0;
                NetDaemonNet.lock_state_[0].trailing_words_[3] = 0;
                NetDaemonNet.lock_state_[2].trailing_words_[6] = 255;
                NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow = 0;
                for (int lockword = 6; lockword < 14; ++lockword)
                    NetDaemonNet.lock_state_[1].trailing_words_[lockword - 5] = 0;
                NetDaemonNet.locks_.clear();
                NetDaemonNet.close();
                IsMultiplayerNet = 0;
                ExitTurnLoop = 1;
            }
            X_pop(!conqueror && !Language ? "YOULOSE2" : "YOULOSE", 0);
            if (faction == ConsoleGlobal.faction_id_) {
                GameState |= 1;
                EndgameExitReason = conqueror ? 7 : 15;
                return 0;
            }
        }
        if (survivors - partners < 2) {
            if (!partners) {
                if (selected == ConsoleGlobal.faction_id_) {
                    popp(ScriptPopupFile, "CONQSING", 0, "conq_sm.pcx", 0);
                    if (!EndgameExitReason) EndgameExitReason = 4;
                }
            } else {
                popp(ScriptPopupFile, "CONQCOOP", 0, "conq_sm.pcx", 0);
                if (!EndgameExitReason) EndgameExitReason = 5;
            }
            GameState |= 0x2001;
        }
        return 0;
    }
respawn:
    p.se_alloc_labs = 5;
    p.se_alloc_psych = 0;
    if (lifecycle_is_fresh(conqueror) && DiffLevelCurrent > 1) {
        if ((p.ai_wealth || p.ai_tech) && rand() % 4 == 0) p.flags_ext |= 8;
        strcpy(StringTemp, definition.filename);
        CharUpperA(StringTemp);
        if (!strcmp(StringTemp, "ANGELS")) p.flags_ext |= 8;
        if (definition.rule_flags & 0x100) { p.flags_ext |= 4; p.flags |= 0x8000000; }
        if (definition.rule_flags & 0x80) { p.flags_ext |= 0x10; p.flags |= 0x10000000; }
        if (!strcmp(StringTemp, "DRONE")) p.flags_ext = (p.flags_ext & ~8u) | 2;
        if (static_cast<int>(Continents[region].tile_count) < 250 && rand() % 3 == 0) p.flags_ext |= 4;
        if (p.ai_fight >= 0 && rand() % 4 == 0) p.flags_ext |= 0x10;
        if (rand() % 6 == 0) p.flags_ext |= 2;
        if (rand() % 6 == 0) p.flags_ext |= 1;
        if (rand() % 5 == 0) p.flags_ext |= 0x40;
        if (((GameRules & 0x40) || DiffLevelCurrent > 3)
            && -static_cast<int>((GameRules & 0x40) != 0) < p.ai_fight
            && rand() % 4 == 0) p.flags_ext |= 0x20;
        if (((GameRules & 0x40) || DiffLevelCurrent > 3)
            && -static_cast<int>((GameRules & 0x40) != 0) < p.ai_fight
            && rand() % 4 == 0) {
            p.flags |= 0x1000000;
            if (rand() % 2 == 0) p.flags |= 0x2000000;
        }
        if (p.ai_power || p.ai_fight > 0) {
            if (static_cast<int>(Continents[region].tile_count) > 399 && rand() % 4 == 0) p.flags |= 0x4000000;
            if (rand() % (static_cast<int>(Continents[region].tile_count) > 250 ? 6 : 4) == 0) p.flags |= 0x8000000;
            if (rand() % 4 == 0) p.flags |= 0x10000000;
        }
    }
    for (int offset4 = 0; offset4 < 21; ++offset4) {
        int rx = lifecycle_wrap_x(x + RadiusOffsetX[offset4]);
        int ry = y + RadiusOffsetY[offset4];
        if (rx >= 0 && rx < MapLongitudeBounds && ry >= 0 && ry < MapLatitudeBounds) {
            map_loc(rx, ry)->visibility |= lifecycle_faction_mask(faction);
            synch_bit(rx, ry, faction);
        }
    }
    int scout = 2;
    if (lifecycle_is_fresh(conqueror)) {
        for (int bonus = 0; bonus < definition.faction_bonus_count; ++bonus)
            if (definition.faction_bonus_id[bonus] == 0)
                tech_achieved(faction, definition.faction_bonus_val1[bonus], 0, 0);
        if (ExpansionEnabled && (definition.rule_flags & 0x80)) {
            tech_achieved(faction, 0x4e, 0, 0);
            tech_achieved(faction, 0x4f, 0, 0);
        }
        if (IsMultiplayerNet) {
            int learned = 0;
            for (;;) {
                int research = p.soc_effect_base.research > 0 ? 1 : p.soc_effect_base.research < 0 ? -1 : 0;
                int morale = p.soc_effect_base.morale > 0 ? 1 : p.soc_effect_base.morale < 0 ? -1 : 0;
                if (learned >= research - morale + 1) break;
                int technology = tech_ai(faction);
                tech_achieved(faction, technology, 0, 0);
                ++learned;
            }
            p.energy_reserves += definition.rule_energy / 2;
        }
        consider_designs(faction);
        for (int local_proto = 0; local_proto < 64; ++local_proto) {
            VehPrototype &prototype = VehPrototypes[faction * 64 + local_proto];
            if (prototype.flags & 1) {
                prototype.flags |= 4;
                if (prototype.plan == 3) scout = faction * 64 + local_proto;
            }
        }
    }
    if (lifecycle_is_fresh(conqueror)) {
        if ((!IsMultiplayerNet && !(GameRules & 0x10)) || (GameRules & 0x80)) {
            int base = base_init(faction, x, y);
            strcpy(StringTemp, definition.filename);
            CharUpperA(StringTemp);
            if ((!strcmp(StringTemp, "FUNGBOY") || (definition.rule_flags & 0x80)) && conqueror == -282)
                Bases[base].population_size = 3;
        } else veh_init((definition.rule_flags & 0x100) ? 0x15 : 0, faction, x, y);
        if (!(definition.rule_flags & 0x100)) {
            int vehicle4 = veh_init(scout, faction, x, y);
            Vehs[vehicle4].home_base_id = -1;
            veh_demote(vehicle4);
        }
        if (IsMultiplayerNet && !(definition.rule_flags & 0x100)) {
            int vehicle5 = veh_init(0xb, faction, x, y);
            Vehs[vehicle5].home_base_id = -1;
            veh_demote(vehicle5);
            vehicle5 = veh_init(1, faction, x, y);
            Vehs[vehicle5].home_base_id = -1;
            veh_demote(vehicle5);
        }
        int coast = is_coast(x, y, 0);
        if (coast && (DiffLevelCurrent > 1 || IsMultiplayerNet
            || static_cast<int>(Continents[region].tile_count) < 33)) {
            // Both naval tests reuse the original coast-direction index.
            region = map_loc(x, y)->region;
            if (IsMultiplayerNet
                && static_cast<int>(Continents[region].tile_count) / (bit_count(occupied) + 1) < 76
                && (LockEnableMask & lifecycle_faction_mask(faction))) {
                int vessel = veh_init(0xd, faction, lifecycle_wrap_x(x + RadiusOffsetX[coast]), y + RadiusOffsetY[coast]);
                Vehs[vessel].morale = 2;
                veh_demote(vessel);
                Vehs[vessel].home_base_id = -1;
            }
            if (static_cast<int>(Continents[region].tile_count) / (bit_count(occupied) + 1) < 51) {
                int vessel2 = veh_init(0xd, faction, lifecycle_wrap_x(x + RadiusOffsetX[coast]), y + RadiusOffsetY[coast]);
                Vehs[vessel2].morale = 2;
                veh_demote(vessel2);
                Vehs[vessel2].home_base_id = -1;
            }
        }
    } else {
        veh_init((definition.rule_flags & 0x100) ? 0x15 : 0, faction, x, y);
        int vehicle6 = veh_init((definition.rule_flags & 0x100) ? 4 : 1, faction, x, y);
        veh_demote(vehicle6);
    }
    if (lifecycle_is_fresh(conqueror)) {
        lifecycle_clear_social(p.soc_effect_base);
        for (int bonus2 = 0; bonus2 < definition.faction_bonus_count; ++bonus2) {
            if (definition.faction_bonus_id[bonus2] == 3)
                lifecycle_add_social(p.soc_effect_base, definition.faction_bonus_val1[bonus2], definition.faction_bonus_val2[bonus2]);
        }
        for (int bonus3 = 0; bonus3 < definition.faction_bonus_count; ++bonus3) {
            if (definition.faction_bonus_id[bonus3] != 1) continue;
            int vehicle7 = veh_init(definition.faction_bonus_val1[bonus3], faction, x, y);
            Vehs[vehicle7].home_base_id = -1;
            veh_demote(vehicle7);
            if (!definition.faction_bonus_val1[bonus3]) p.flags |= 0x100;
        }
        if (ExpansionEnabled && (definition.rule_flags & 0x80)) {
            int vehicle8 = veh_init(0x10, faction, x, y);
            Vehs[vehicle8].home_base_id = -1;
            veh_demote(vehicle8);
            vehicle8 = veh_init(0, faction, x, y);
            Vehs[vehicle8].home_base_id = -1;
            veh_demote(vehicle8);
        }
        if (definition.rule_flags & 0x100) {
            if (!(LockEnableMask & lifecycle_faction_mask(faction))) {
                int prototype = conqueror;
                for (int local_proto2 = 0; local_proto2 < 64; ++local_proto2) {
                    int index = faction * 64 + local_proto2;
                    if (!(VehPrototypes[index].flags & 1)) { prototype = index; break; }
                }
                // If no free design exists, the original retains conqueror
                // as the index. No recovery-added guard is inserted.
                make_proto(prototype, 3, 0x11, 0, 0, 1);
                VehPrototypes[prototype].flags = (VehPrototypes[prototype].flags & 0xff3f) | 4;
                name_proto(prototype_name, prototype, faction, 3, 0x11, 0, 0, 1);
                VehPrototypes[prototype].flags &= ~2;
                prototype_name[31] = 0;
                strcpy(VehPrototypes[prototype].veh_name, prototype_name);
            }
            int vehicle9 = veh_init(0x15, faction, x, y);
            Vehs[vehicle9].home_base_id = -1;
            veh_demote(vehicle9);
            vehicle9 = veh_init(0x16, faction, x, y);
            Vehs[vehicle9].home_base_id = -1;
            veh_demote(vehicle9);
        }
    }
    if (TurnCurrentNum && !failed && conqueror != -282) {
        int defense_value = -1, defense = 2, attack_value = -1, attack = -1;
        int fast_value = -1, fast = -1;
        for (int prototype2 = 0; prototype2 < 512; ++prototype2) {
            VehPrototype &design = VehPrototypes[prototype2];
            if (!veh_avail(prototype2, faction, -1) || !Weapon[design.weapon_id].offense_rating) continue;
            int triad = Chassis[design.chassis_id].triad;
            if ((definition.rule_flags & 0x100) ? triad != 1 : triad != 0) continue;
            int value = armor_val(prototype2, faction) * 8 - weap_val(prototype2, faction);
            if (value > defense_value) { defense_value = value; defense = prototype2; }
            value = Chassis[design.chassis_id].speed + (armor_val(prototype2, faction) + weap_val(prototype2, faction) * 2) * 2;
            if (value > attack_value) { attack_value = value; attack = prototype2; }
            value = Chassis[design.chassis_id].speed * 4 - weap_val(prototype2, faction) - armor_val(prototype2, faction);
            if (value > fast_value && Chassis[design.chassis_id].speed > 1) { fast_value = value; fast = prototype2; }
        }
        int colony = veh_init((definition.rule_flags & 0x100) ? 0x15 : 0, faction, x, y);
        veh_init(defense, faction, x, y);
        if (TurnCurrentNum > 20 && fast >= 0) veh_init(fast, faction, x, y);
        if (TurnCurrentNum > 40 && attack >= 0) veh_init(attack, faction, x, y);
        if (TurnCurrentNum > 60 && defense >= 0) veh_init(defense, faction, x, y);
        if (TurnCurrentNum > 80 && attack >= 0) veh_init(attack, faction, x, y);
        if (TurnCurrentNum > 100) veh_init((definition.rule_flags & 0x100) ? 0x15 : 0, faction, x, y);
        veh_promote(colony);
        p.unk_26 = p.tech_ranking * 3 / 4;
    }
    if (faction == ConsoleGlobal.faction_id_) {
        MapWinTable[0]->field_1DD9C_ = x;
        MapWinTable[0]->field_1DDA0_ = y;
        for (int history = 0; history < 32; ++history) {
            ConsoleGlobal.cursor_history_x_[history] = x;
            ConsoleGlobal.cursor_history_y_[history] = y;
        }
    }
    return 1;
}

// ORIGINAL: 0x005B3380 ?eliminate_player@@YAHHH@Z 0x005B3380-0x005B38C6
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005b3380.cpp: canonical original identity, full spans and retained seed provenance", "source:src/faction.h: real faction record ownership and typed relationships", "source:recovery/tasks/faction-lifecycle-owner-closure.json: coordinator authorization and retained task baseline"], "reason": "The original faction elimination operation belongs to the Faction owner; typed faction diplomacy, base/vehicle lifecycles and shared endgame/script state preserve actual cross-owner relationships rather than raw executable references."}
int __cdecl eliminate_player(int faction, int conqueror) {
    if (!faction) return 0;
    for (int base = 0; base < BaseCurrentCount; ++base) {
        if (Bases[base].faction_id_current == faction) return 0;
    }
    for (int vehicle = VehCurrentCount - 1; vehicle >= 0; --vehicle) {
        if (vehicle < VehCurrentCount && Vehs[vehicle].faction_id == faction)
            veh_kill(vehicle);
    }
    PlayersData[faction].current_num_bases = 0;
    const int restored = setup_player(faction, conqueror, 0);
    if (!restored) {
        if (conqueror > 0 && !(GameState & 9))
            ++PlayersData[conqueror].unk_108;
        if (faction == ConsoleGlobal.faction_id_) return 1;
    }
    int successor = conqueror < 0 ? 0 : conqueror;
    if (successor > 0) set_treaty(faction, successor, 0x2030, 1);
    draw_map(1);
    GenderDefault = Players[faction].noun_gender;
    PluralityDefault = Players[faction].is_noun_plural;
    parse_says(0, Players[faction].noun_faction, -1, -1);
    GenderDefault = Players[successor].noun_gender;
    PluralityDefault = Players[successor].is_noun_plural;
    parse_says(1, Players[successor].noun_faction, -1, -1);
    GenderDefault = Players[faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(2, Players[faction].title_leader, -1, -1);
    GenderDefault = Players[faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(3, Players[faction].name_leader, -1, -1);
    parse_says(4, get_his_her(faction, 0), -1, -1);
    strcpy(StringTemp, "WIPEOUT");
    char number[80];
    if (!successor) _itoa(2, number, 10);
    else {
        if (successor == ConsoleGlobal.faction_id_) strcat(StringTemp, "YOU");
        _itoa(restored != 0, number, 10);
    }
    strcat(StringTemp, number);
    char portrait[80];
    strcpy(portrait, Players[faction].filename);
    strcat(portrait, "2.PCX");
    const bool alien = (Players[faction].rule_flags & 0x80) != 0;
    popp(ScriptPopupFile, StringTemp, 0,
         !restored ? portrait : alien ? "al_pod_sm.pcx" : "escpod_sm.pcx", 0);
    if (!restored) mon_killed_faction(successor, faction);
    if (!IsMultiplayerNet && (GamePreferences & 0x10000000)
        && successor == ConsoleGlobal.faction_id_) {
        if (!restored) {
            amovie_project(Players[faction].filename);
            goto eliminated;
        }
    } else if (!restored || IsMultiplayerNet) goto eliminated;
    if (successor == ConsoleGlobal.faction_id_) {
        const int chance = (PlayersData[successor].diplo_treaties[faction] & 0x1000) ? 2 : 4;
        if (rand() % chance == 0) {
            const int first = veh_find(0, 0, faction, -1);
            if (first >= 0) {
                int x = 0, y = 0;
                for (int offset = 0; offset < 9; ++offset) {
                    x = RadiusOffsetX[offset] + Vehs[first].x;
                    if (!(MapIsFlat & 1)) {
                        if (x < 0) x += MapLongitudeBounds;
                        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
                    }
                    y = RadiusOffsetY[offset] + Vehs[first].y;
                    if (y >= 0 && y < MapLatitudeBounds && x >= 0 && x < MapLongitudeBounds)
                        spot_loc(x, y, successor);
                }
                int vehicle = first;
                while (Vehs[vehicle].prev_veh_id_stack >= 0)
                    vehicle = Vehs[vehicle].prev_veh_id_stack;
                bool marked = false;
                for (; vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack) {
                    Vehs[vehicle].visibility = 0;
                    if (VehPrototypes[Vehs[vehicle].proto_id].plan == 8 && !marked) {
                        marked = true;
                        Vehs[vehicle].visibility = lifecycle_faction_mask(successor);
                    }
                }
                // The original deliberately retains the last radius coordinates.
                draw_tiles(x, y, 2);
                MapWinTable[0]->set_center(x, y, 1);
                parse_says(6, Players[faction].adj_name_faction, -1, -1);
                popp(ScriptPopupFile, "TRACKED", 0,
                     (Players[faction].rule_flags & 0x80) ? "al_pod_sm.pcx" : "escpod_sm.pcx", 0);
            }
        }
    }
eliminated:
    if ((Players[faction].rule_flags & 0x80) && (Players[ConsoleGlobal.faction_id_].rule_flags & 0x80)
        && faction != ConsoleGlobal.faction_id_) interlude(0x2a, 0, 1, 0);
    return 1;
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
// ORIGINAL: 0x005399A0 ?diplomacy_caption@@YAXHH@Z 0x005399A0-0x00539B6B
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5399A0..539B6B; existing inactive diplomacy-presentation task evidence retained; counterpart Player noun fields and PlayerData20CC treaty strides independently decoded; literal68D594 is the bounded separator. Host source-body controls cover mood thresholds, bidirectional pact/surrender and post-parse visibility."], "reason": "Recover complete diplomacy caption in its faction owner with genuine treaty/noun/mood/scratch data and the actual visible-popup draw operation. Preserve native friction clamp, label predicate, parse defaults and redraw ordering."}
void __cdecl diplomacy_caption(int faction_id, int faction_id_with) {
    uint32_t incoming_treaty =
        PlayersData[faction_id].diplo_treaties[faction_id_with];
    int mood = 0;
    if ((incoming_treaty & DTREATY_VENDETTA) != 0 &&
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

    char caption[256] = {0};
    const Player &counterpart = Players[faction_id_with];
    GenderDefault = counterpart.noun_gender;
    PluralityDefault = counterpart.is_noun_plural;
    strcat(caption, counterpart.noun_faction);
    strcat(caption, " - ");

    const uint32_t reverse_treaty =
        PlayersData[faction_id_with].diplo_treaties[faction_id];
    const bool use_special_label =
        (mood == 0 && DiploScratchFA98 != 0 &&
         (incoming_treaty & DTREATY_PACT) == 0) ||
        ((reverse_treaty & (DTREATY_PACT | DTREATY_HAVE_SURRENDERED)) ==
         (DTREATY_PACT | DTREATY_HAVE_SURRENDERED));
    if (use_special_label) {
        strcat(caption, label_get(0x1E8));
    } else {
        char *mood_label = StringTable.get(
            static_cast<int>(reinterpret_cast<intptr_t>(Mood[mood])));
        say_fac_special(caption, mood_label, faction_id_with);
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

// ORIGINAL: 0x0053DCE0 ?wants_to_speak@@YAHHH@Z 0x0053DCE0-0x0053E46C;0x0065E622-0x0065E75A FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0053dce0.cpp", "image:terranx_original.exe:0x0053DCE0"], "reason": "Recover the genuine faction contact/introduction owner using Popup lifetime, typed Player/PlayerData, real diplomacy state and portrait storage; native guards and callee order correct retained seed contradictions."}
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0053d360.cpp", "image:terranx_original.exe:0x0053D360"], "reason": "Recover the genuine faction contact/introduction owner using Popup lifetime, typed Player/PlayerData, real diplomacy state and portrait storage; native guards and callee order correct retained seed contradictions."}
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
            int rank = 7 - static_cast<int>(PlayersData[faction_id].ranking);
            if (rank < 0) rank = 0;
            else if (rank > 6) rank = 6;
            char *word = StringTable.get(static_cast<int>(
                reinterpret_cast<intptr_t>(Might[rank].adj)));
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
    x_pops_short(StringTemp, 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
    if (PlayersData[other_faction_id].planet_busters) {
        x_pops_short("PLANETBUSTER0", 0x100000,
                     FactionDiplomacyPortraits[other_faction_id], pop_wait);
    }
    if (PlayersData[faction_id].planet_busters &&
        !(PlayersData[faction_id].diplo_treaties[other_faction_id] & 0x4000)) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "PLANETBUSTER");
        _itoa((PlayersData[other_faction_id].planet_busters != 0) + 1, number, 10);
        strcat(StringTemp, number);
        x_pops_short(StringTemp, 0x100000, FactionDiplomacyPortraits[other_faction_id], pop_wait);
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
// ORIGINAL: 0x00543A20 ?trade_maps@@YAXHH@Z 0x00543A20-0x00543BC0
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00543a20.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl trade_maps(int faction_id, int other_faction_id) {
    DiplomacyMapTransferChanged = 0;

    int &trade_a = PlayersData[faction_id].traded_maps;
    int &trade_b = PlayersData[other_faction_id].traded_maps;
    const uint32_t combined = static_cast<uint32_t>(trade_a) |
        (1u << (other_faction_id & 31)) | trade_b;
    trade_a = static_cast<int>(combined);
    trade_b = static_cast<int>(static_cast<uint32_t>(trade_b) |
        (1u << (faction_id & 31)) | combined);

    const uint8_t mask_a = static_cast<uint8_t>(1u << (faction_id & 31));
    const uint8_t mask_b = static_cast<uint8_t>(1u << (other_faction_id & 31));
    for (int tile_id = 0; tile_id < static_cast<int>(MapArea); ++tile_id) {
        Map &tile = MapTiles[tile_id];
        const uint8_t visible_to = static_cast<uint8_t>(tile.visibility);
        if ((visible_to & mask_a) == 0) {
            if ((visible_to & mask_b) != 0) {
                DiplomacyMapTransferChanged = 1;
                tile.visibility = visible_to | mask_a;
                if (faction_id != 0 && other_faction_id != 0) {
                    tile.bit_visible[faction_id - 1] |=
                        tile.bit_visible[other_faction_id - 1] & tile.bit;
                }
            }
        } else {
            tile.visibility = visible_to | mask_b;
            if (faction_id != 0 && other_faction_id != 0) {
                tile.bit_visible[other_faction_id - 1] |=
                    tile.bit_visible[faction_id - 1] & tile.bit;
            }
        }
    }

    for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
        const Base &base = Bases[base_id];
        int revealed_to = other_faction_id;
        if ((base.visibility & mask_a) != 0 ||
            base.faction_id_current == faction_id) {
            spot_base(base_id, revealed_to);
        } else if ((base.visibility & mask_b) != 0 ||
                   base.faction_id_current == other_faction_id) {
            revealed_to = faction_id;
            spot_base(base_id, revealed_to);
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_ ||
        other_faction_id == ConsoleGlobal.faction_id_) {
        UnkBitfield1 |= 4;
        WorldClimateWorldWin.GraphicWin::redraw();
        draw_map(1);
    }
}

// ORIGINAL: 0x00548F80 ?do_withdrawal@@YAHHH@Z 0x00548F80-0x00549264
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00548f80.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
int __cdecl do_withdrawal(int faction_id, int other_faction_id) {
    int result = 0;
    if (VehCurrentCount <= 0) return 0;

    for (int veh_id = 0; veh_id < VehCurrentCount; ++veh_id) {
        Veh &veh = Vehs[veh_id];
        if (veh.faction_id != other_faction_id) continue;
        const int proto_id = veh.proto_id;
        if (VehPrototypes[proto_id].armor_id == 0x0B) continue;

        const int x = veh.x;
        const int y = veh.y;
        Map &tile = MapTiles[MapLongitude * y + (x >> 1)];
        if ((tile.climate & 0xE0) < ALT_BIT_SHORE_LINE) continue;
        if (tile.territory != faction_id) continue;

        if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_LAND &&
            veh_cargo(veh_id) != 0) {
            for (int neighbor = 0; neighbor < 8; ++neighbor) {
                int dest_x = RadiusBaseX[neighbor] + x;
                if ((MapIsFlat & 1) == 0) {
                    if (dest_x < 0) dest_x += MapLongitudeBounds;
                    else if (dest_x >= MapLongitudeBounds)
                        dest_x -= MapLongitudeBounds;
                }
                const int dest_y = RadiusBaseY[neighbor] + y;
                if (dest_y < 0 || dest_y >= MapLatitudeBounds || dest_x < 0 ||
                    dest_x >= MapLongitudeBounds) continue;

                Map &dest = MapTiles[MapLongitude * dest_y + (dest_x >> 1)];
                int allowed = 0;
                if ((dest.climate & 0xE0) < ALT_BIT_SHORE_LINE) {
                    const int occupant = stack_fix(veh_at(dest_x, dest_y));
                    if (stack_check(occupant, 8, other_faction_id, -1, -1) > 0)
                        allowed = 1;
                } else if (dest.territory == other_faction_id) {
                    allowed = 1;
                }
                if (!allowed) continue;

                const unsigned flags = dest.val2 & 0x0F;
                if ((dest.bit & 3) != 0 && flags < 8 &&
                    static_cast<int>(flags) != other_faction_id) continue;

                const int lifted_neighbor_vehicle = veh_lift(veh_id);
                veh_drop(lifted_neighbor_vehicle, dest_x, dest_y);
                veh.visibility = 0;
                veh.order = ORDER_NONE;
                veh.state &= 0xFCFFBDFF;
                draw_tiles(x, y, 2);
                ++result;
            }
        }

        const int base_id = base_find(x, y, other_faction_id);
        if (base_id >= 0) {
            const int dest_x = Bases[base_id].x;
            const int dest_y = Bases[base_id].y;
            const int lifted_base_vehicle = veh_lift(veh_id);
            veh_drop(lifted_base_vehicle, dest_x, dest_y);
            veh.visibility = 0;
            veh.order = ORDER_NONE;
            veh.state &= 0xFCFFBDFF;
            const int ability = has_abil(veh.proto_id, 2);
            draw_tile_fixup(x, y, ability, 2);
            ++result;
        }
    }

    if (result != 0) {
        set_agenda(other_faction_id, faction_id, 2, 1);
        set_treaty(other_faction_id, faction_id, 0x100000, 1);
    }
    return result;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0054CF40 ?give_a_base@@YAXHH@Z 0x0054CF40-0x0054D410
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0054cf40.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl give_a_base(int base_id, int new_owner) {
    Base &base = Bases[base_id];
    const int x = base.x;
    const int y = base.y;
    const unsigned old_owner = base.faction_id_current;

    for (int offset = 0; offset < 0x54; offset += 4) {
        int dest_x = x + RadiusOffsetX[offset / 4];
        if ((MapIsFlat & 1) == 0) {
            if (dest_x < 0) dest_x += MapLongitudeBounds;
            else if (dest_x >= MapLongitudeBounds)
                dest_x -= MapLongitudeBounds;
        }
        const int dest_y = y + RadiusOffsetY[offset / 4];
        if (dest_y >= 0 && dest_y < MapLatitudeBounds && dest_x >= 0 &&
            dest_x < MapLongitudeBounds) {
            Map &tile = MapTiles[MapLongitude * dest_y + (dest_x >> 1)];
            tile.visibility |= static_cast<uint8_t>(1u << new_owner);
            synch_bit(dest_x, dest_y, new_owner);
        }
    }

    for (int veh_id = VehCurrentCount - 1; veh_id >= 0; --veh_id) {
        Veh &veh = Vehs[veh_id];
        if (veh.faction_id != old_owner) continue;
        const int veh_x = veh.x;
        const int veh_y = veh.y;
        int dx = abs(veh_x - x);
        if ((MapIsFlat & 1) == 0 && static_cast<int>(MapLongitude) < dx)
            dx = MapLongitudeBounds - dx;
        const int dy = abs(veh_y - y);
        const int adx = abs(dx);
        const int ady = abs(dy);
        const int hi = adx > ady ? adx : ady;
        const int lo = adx < ady ? adx : ady;
        const int cheby = (((adx + ady) >> 1) - lo + 1) >> 1;
        const int remaining = hi - cheby;
        const uint8_t terrain = MapTiles[MapLongitude * veh_y + (veh_x >> 1)].climate;
        const unsigned near_base = ((terrain & 0xE0) >= ALT_BIT_SHORE_LINE) ? 1u : 0u;
        if (remaining > static_cast<int>(near_base)) continue;

        const int prototype_id = veh.proto_id;
        const int armor = VehPrototypes[prototype_id].armor_id;
        bool find_other_base = true;
        if ((armor == 2 || armor == 3) && remaining == 0) {
            find_other_base = false;
        } else {
            const int chassis = VehPrototypes[prototype_id].chassis_id;
            if (Chassis[chassis].triad == 1 &&
                (remaining == 0 || (terrain & 0xE0) >= ALT_BIT_SHORE_LINE)) {
                find_other_base = false;
            }
        }

        bool transferred = false;
        if (find_other_base) {
            base.faction_id_current = static_cast<uint8_t>(new_owner);
            const int found_base = base_find(x, y, static_cast<int>(old_owner));
            base.faction_id_current = static_cast<uint8_t>(old_owner);
            if (found_base >= 0) {
                const int found_x = Bases[found_base].x;
                const int found_y = Bases[found_base].y;
                int dx2 = abs(found_x - x);
                if ((MapIsFlat & 1) == 0 &&
                    static_cast<int>(MapLongitude) < dx2) {
                    dx2 = MapLongitudeBounds - dx2;
                }
                const int dy2 = abs(found_y - y);
                if (dx2 + dy2 >= 2) {
                    stack_put(stack_veh(veh_id, 1), found_x, found_y);
                    veh.order = ORDER_NONE;
                    if (veh.home_base_id == base_id)
                        veh.home_base_id = static_cast<int16_t>(found_base);
                    transferred = true;
                }
            }
        }

        if (!transferred) {
            --PlayersData[old_owner].proto_id_active[prototype_id];
            veh.faction_id = static_cast<uint8_t>(new_owner);
            ++PlayersData[new_owner].proto_id_active[prototype_id];
            veh.home_base_id = static_cast<int16_t>(base_id);
            if (veh.order != ORDER_HOLD) veh.order = ORDER_NONE;
            veh.waypoint_y[1] = static_cast<int16_t>(veh.waypoint_y[1] & ~0xFF);
            owner_set(veh_x, veh_y, new_owner);
        }
    }

    base.faction_id_current = static_cast<uint8_t>(new_owner);
    base.minerals_accumulated_2 = 0;
    base.minerals_accumulated = 0;
    const int queue_size = static_cast<int32_t>(base.queue_size);
    if (queue_size >= 0) {
        for (int index = 0; index <= queue_size; ++index) {
            if (base.queue_production_id[index] >= 0) {
                --PlayersData[old_owner].proto_id_queue[
                    base.queue_production_id[index]];
            }
        }
    }
    base.queue_size = 0;
    base.production_id_last = -69;
    base.queue_production_id[0] = -69;

    replay_base(1, x, y, new_owner);
    const int proposal_mode = DiplomacyProposalMode;
    base.state &= 0xFFDEF7B1;
    base.nerve_staple_turns_left = 0;
    if (proposal_mode == 3) {
        if (base.faction_id_former == new_owner) {
            base.assimilation_turns_left = 0;
        } else {
            base.faction_id_former = static_cast<uint8_t>(old_owner);
            base.assimilation_turns_left = 0x32;
        }
    } else {
        if (base.faction_id_former == new_owner)
            base.assimilation_turns_left = 0;
        base.faction_id_former = static_cast<uint8_t>(new_owner);
    }

    spot_base(base_id, static_cast<int>(old_owner));
    reset_territory();
    for (int cleanup_vehicle = VehCurrentCount - 1; cleanup_vehicle >= 0; --cleanup_vehicle) {
        if (cleanup_vehicle < VehCurrentCount &&
            Vehs[cleanup_vehicle].faction_id == old_owner &&
            Vehs[cleanup_vehicle].home_base_id == base_id) {
            kill(cleanup_vehicle);
        }
    }
    owner_set(x, y, new_owner);
    draw_tile_fixup(x, y, 1, 2);
    set_base(base_id);
    base_compute(1);
}

// ORIGINAL: 0x0050BCC0 ?double_cross@@YAXHHH@Z 0x0050BCC0-0x0050C2D7
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0050bcc0.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl double_cross(int faction_id, int partner_id, int victim_id) {
    if (IsMultiplayerNet) {
        if (partner_id == ConsoleGlobal.faction_id_ && victim_id > 0) {
            PluralityDefault = 0;
            GenderDefault = Players[victim_id].is_leader_female;
            parse_says(0, Players[victim_id].title_leader, -1, -1);
            GenderDefault = Players[victim_id].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[victim_id].name_leader, -1, -1);
            GenderDefault = Players[victim_id].noun_gender;
            PluralityDefault = Players[victim_id].is_noun_plural;
            parse_says(2, Players[victim_id].noun_faction, -1, -1);

            PluralityDefault = 0;
            GenderDefault = Players[faction_id].is_leader_female;
            parse_says(3, Players[faction_id].title_leader, -1, -1);
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = 0;
            parse_says(4, Players[faction_id].name_leader, -1, -1);
            GenderDefault = Players[faction_id].noun_gender;
            PluralityDefault = Players[faction_id].is_noun_plural;
            parse_says(5, Players[faction_id].noun_faction, -1, -1);
            NetMsgPrimary.pop("INCITED", 0x1388, 0, nullptr);
        } else if (partner_id == ConsoleGlobal.faction_id_ &&
                   (LockEnableMask & (1u << (faction_id & 31))) != 0) {
            GenderDefault = Players[faction_id].is_leader_female;
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            GenderDefault = Players[faction_id].noun_gender;
            PluralityDefault = Players[faction_id].is_noun_plural;
            parse_says(2, Players[faction_id].noun_faction, -1, -1);
            NetMsgPrimary.pop("VENDETTAWARNING", 0x1388, 0, nullptr);
        }
    }

    // Index A is row=partner, column=actor; B is row=actor, column=partner.
    PlayersData[partner_id].loan_balance[faction_id] = 0;
    const uint32_t treaty_flags =
        PlayersData[faction_id].diplo_treaties[partner_id];
    int treaty_active = 0;
    if ((treaty_flags & 0x40000) != 0) {
        treaty_active = 1;
    } else if (PlayersData[partner_id].major_atrocities != 0) {
        treaty_active = 1;
    }

    int friction_flag = 0;
    if (victim_id >= 0) {
        const uint32_t victim_treaty =
            PlayersData[faction_id].diplo_treaties[victim_id];
        if ((victim_treaty & 1) != 0) {
            if ((treaty_flags & 1) == 0) friction_flag = 1;
        } else if ((victim_treaty & 2) != 0) {
            if ((treaty_flags & 2) == 0) friction_flag = 1;
        } else if ((victim_treaty & 4) != 0) {
            if ((treaty_flags & 4) == 0) friction_flag = 1;
        }
        ++PlayersData[faction_id].diplo_unk1[victim_id];
    }

    if ((treaty_flags & 1) != 0) {
        if ((PlayersData[faction_id].diplo_agenda[partner_id] & 0x20) == 0) {
            int &reputation = PlayersData[faction_id].global_reputation;
            reputation = reputation > 0 ? reputation - 1 : 0;
            if (PlayersData[faction_id].diff_level != 0 &&
                treaty_active == 0 && friction_flag == 0) {
                ++PlayersData[faction_id].integrity_blemishes;
                ++PlayersData[partner_id].diplo_betrayed[faction_id];
            }
            if (victim_id >= 0) {
                if (treaty_active == 0)
                    ++PlayersData[faction_id].integrity_blemishes;
                ++PlayersData[faction_id].diplo_wrongs[partner_id];
                ++PlayersData[partner_id].diplo_betrayed[faction_id];
            } else {
                cause_friction(victim_id, faction_id, -4);
            }
            if ((LockEnableMask & (1u << (faction_id & 31))) != 0) {
                const uint32_t actor_partner_agenda =
                    PlayersData[partner_id].diplo_agenda[faction_id];
                if ((actor_partner_agenda & 0x400) != 0)
                    set_agenda(partner_id, faction_id, 0x800, 1);
                if ((PlayersData[partner_id].diplo_treaties[faction_id] & 0x20) != 0)
                    set_agenda(partner_id, faction_id, 0x400, 1);
            }
        }
        pact_ends(faction_id, partner_id);
        treaty_on(faction_id, partner_id, 0x10);
        return;
    }

    if ((PlayersData[faction_id].diplo_agenda[partner_id] & 0x20) == 0) {
        if ((treaty_flags & 6) != 0) {
            if ((treaty_flags & 0x10000) == 0) {
                int &reputation = PlayersData[faction_id].global_reputation;
                reputation = reputation > 0 ? reputation - 1 : 0;
                if (great_satan(faction_id, 0) == 0 &&
                    PlayersData[faction_id].ranking >= 6 &&
                    PlayersData[partner_id].ranking <= 4 && victim_id < 0 &&
                    treaty_active == 0) {
                    ++PlayersData[faction_id].integrity_blemishes;
                    ++PlayersData[partner_id].diplo_betrayed[faction_id];
                }
            }
            if (PlayersData[faction_id].diff_level != 0 &&
                treaty_active == 0 && friction_flag == 0) {
                ++PlayersData[faction_id].integrity_blemishes;
                ++PlayersData[partner_id].diplo_betrayed[faction_id];
            }
            if (victim_id < 0)
                ++PlayersData[faction_id].diplo_wrongs[partner_id];

            if ((LockEnableMask & (1u << (faction_id & 31))) != 0) {
                const uint32_t actor_partner_agenda =
                    PlayersData[partner_id].diplo_agenda[faction_id];
                if ((actor_partner_agenda & 0x400) != 0)
                    set_agenda(partner_id, faction_id, 0x800, 1);
                if ((PlayersData[partner_id].diplo_treaties[faction_id] & 0x20) != 0)
                    set_agenda(partner_id, faction_id, 0x400, 1);
                set_treaty(partner_id, faction_id, 0x20, 1);
            }
            if (victim_id >= 0) cause_friction(victim_id, faction_id, -2);
        } else if (victim_id >= 0) {
            if ((LockEnableMask & (1u << (faction_id & 31))) != 0) {
                const uint32_t actor_partner_agenda =
                    PlayersData[partner_id].diplo_agenda[faction_id];
                if ((actor_partner_agenda & 0x400) != 0)
                    set_agenda(partner_id, faction_id, 0x800, 1);
                if ((PlayersData[partner_id].diplo_treaties[faction_id] & 0x20) != 0)
                    set_agenda(partner_id, faction_id, 0x400, 1);
                set_treaty(partner_id, faction_id, 0x20, 1);
            }
            cause_friction(victim_id, faction_id, -2);
        }
    }

    treaty_on(faction_id, partner_id, 0x10);
    if ((PlayersData[faction_id].diplo_agenda[partner_id] & 0x20) == 0) {
        const bool actor_human =
            (LockEnableMask & (1u << (faction_id & 31))) != 0;
        const bool partner_human =
            (LockEnableMask & (1u << (partner_id & 31))) != 0;
        if (actor_human != partner_human) {
            set_treaty(faction_id, partner_id, 0x40, 1);
            PlayersData[faction_id].diplo_spoke[partner_id] = TurnCurrentNum;
        }
        intervention(partner_id, faction_id);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0053C650 ?pact_ends@@YAXHH@Z 0x0053C650-0x0053CAE6;0x0065E470-0x0065E53A
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0053c650.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl pact_ends(int faction_id, int partner_id) {
    Popup popup;
    PopupDialogFactionID = partner_id;
    treaty_off(faction_id, partner_id, 1);
    const int withdraw_actor = pact_withdraw(faction_id, partner_id);
    const int withdraw_partner = pact_withdraw(partner_id, faction_id);
    draw_map(1);

    const int human = ConsoleGlobal.faction_id_;
    int use_withdraw = 0;
    if (faction_id == human) {
        const Player &other = Players[partner_id];
        GenderDefault = other.noun_gender;
        PluralityDefault = other.is_noun_plural;
        parse_says(0, other.noun_faction, -1, -1);
        parse_says(1, other.adj_name_faction, -1, -1);
        use_withdraw = withdraw_actor;
    } else if (partner_id == human) {
        const Player &other = Players[faction_id];
        GenderDefault = other.noun_gender;
        PluralityDefault = other.is_noun_plural;
        parse_says(0, other.noun_faction, -1, -1);
        parse_says(1, other.adj_name_faction, -1, -1);
        use_withdraw = withdraw_partner;
    } else {
        if (spying(faction_id) == 0 && spying(partner_id) == 0 &&
            (static_cast<uint8_t>(GameState) & 0x80) == 0 &&
            (PlayersData[human].diplo_treaties[faction_id] & 1) == 0 &&
            (PlayersData[human].diplo_treaties[partner_id] & 1) == 0) {
            return;
        }
        const Player &first = Players[faction_id];
        GenderDefault = first.noun_gender;
        PluralityDefault = first.is_noun_plural;
        parse_says(0, first.noun_faction, -1, -1);
        const Player &second = Players[partner_id];
        GenderDefault = second.noun_gender;
        PluralityDefault = second.is_noun_plural;
        parse_says(1, second.noun_faction, -1, -1);
        parse_says(2, get_pact_hood(faction_id, partner_id), -1, -1);
        X_pop("ENDPACT3", 0);
        return;
    }

    parse_num(0, use_withdraw);
    parse_says(2, get_pact_hood(faction_id, partner_id), -1, -1);
    const uint32_t net_mask = IsMultiplayerNet ? 0xFFFFFFFFu : 0;
    const uint32_t first_flag = (net_mask & 0xFFF00000u) + 0x00100000u;
    popup.start(PopupStartCaption, "ENDPACT1", -1, nullptr,
                static_cast<int>(first_flag), nullptr);
    if (use_withdraw != 0) {
        const uint32_t second_flag = (IsMultiplayerNet ? 0u : 0x100000u) | 0x80u;
        popup.start(PopupStartCaption, "ENDPACT2", -1, nullptr,
                    static_cast<int>(second_flag), nullptr);
    }
    popup.exec(0, nullptr);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050BA50 ?intervention@@YAXHH@Z 0x0050BA50-0x0050BCB6
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050ba50.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl intervention(int target_faction, int actor_faction) {
    for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if (candidate == target_faction || candidate == actor_faction) continue;

        // LockEnableMask byte +1 (native 0x009A64E9) is the faction-alive mask.
        const uint8_t alive = static_cast<uint8_t>(LockEnableMask >> 8);
        if ((alive & (1u << candidate)) == 0) continue;
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

        GenderDefault = 0;
        PluralityDefault = FALSE;
        parse_says(3, Players[candidate].name_leader, -1, -1);

        GenderDefault = Players[actor_faction].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(4, Players[actor_faction].title_leader, -1, -1);

        GenderDefault = 0;
        PluralityDefault = FALSE;
        parse_says(5, Players[actor_faction].name_leader, -1, -1);

        GenderDefault = Players[actor_faction].noun_gender;
        PluralityDefault = Players[actor_faction].is_noun_plural;
        parse_says(6, Players[actor_faction].noun_faction, -1, -1);

        const uint32_t human_mask = LockEnableMask & 0xFFu;
        if ((human_mask & (1u << target_faction)) != 0 ||
            (human_mask & (1u << actor_faction)) != 0) {
            if ((human_mask & (1u << actor_faction)) != 0) {
                PlayersData[actor_faction].diplo_spoke[candidate] = TurnCurrentNum;
            }
            treaty_on(candidate, actor_faction, 0x80000018);
            set_treaty(candidate, actor_faction, 0x40, TRUE);
        }

        if (ConsoleGlobal.faction_id_ == actor_faction) {
            X_pop("AIDSTHEM", 0);
        } else if (ConsoleGlobal.faction_id_ == target_faction) {
            X_pop("AIDSUS", 0);
        }
    }
}

// ORIGINAL: 0x0053C370 ?pact_withdraw@@YAHHH@Z 0x0053C370-0x0053C649
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0053c370.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
int __cdecl pact_withdraw(int faction_id, int faction_id_with) {
    int lifted_count = 0;
    for (int vehicle_id = 0; vehicle_id < VehCurrentCount; ++vehicle_id) {
        Veh &vehicle = Vehs[vehicle_id];
        if (vehicle.faction_id != faction_id) continue;

        const int base_at_vehicle = base_find(vehicle.x, vehicle.y);
        if (base_at_vehicle < 0) continue;

        if (Bases[base_at_vehicle].faction_id_current != faction_id_with &&
            whose_territory(faction_id, vehicle.x, vehicle.y, 0, 0) != faction_id_with) {
            continue;
        }

        int best_base = -1;
        int best_distance = 9999;
        for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
            Base &base = Bases[base_id];
            if (base.faction_id_current != faction_id) continue;

            const int chassis_id_for_coast = VehPrototypes[vehicle.proto_id].chassis_id;
            if (Chassis[chassis_id_for_coast].triad == TRIAD_SEA) {
                // Preserve the original's coordinate-as-index bug exactly:
                // native uses base.x as a Bases[] index and base.y as the
                // is_coast boolean, rather than using base_id / TRUE.
                const Base &coastal_probe = Bases[base.x];
                if (!is_coast(coastal_probe.x, coastal_probe.y, base.y)) continue;
            }

            const int chassis_id_for_sea_region =
                VehPrototypes[vehicle.proto_id].chassis_id;
            if (Chassis[chassis_id_for_sea_region].triad == TRIAD_SEA) {
                const Map *tile = map_loc(vehicle.x, vehicle.y);
                if ((tile->climate & 0xE0) < 0x60 &&
                    !base_on_sea(base_id, tile->region)) {
                    continue;
                }
            }

            int dx = abs(base.x - vehicle.x);
            if ((MapIsFlat & 1) == 0 && static_cast<int>(MapLongitude) < dx) {
                dx = static_cast<int>(MapLongitudeBounds) - dx;
            }
            dx = abs(dx);
            const int dy = abs(base.y - vehicle.y);
            const int maximum = (dx > dy) ? dx : dy;
            const int minimum = (dx < dy) ? dx : dy;
            const int distance = maximum - (((dx + dy) / 2 - minimum + 1) / 2);
            if (distance < best_distance) {
                best_distance = distance;
                best_base = base_id;
            }
        }

        if (best_base < 0) continue;

        const int destination_x = Bases[best_base].x;
        const int destination_y = Bases[best_base].y;
        const int lifted_vehicle = veh_lift(vehicle_id);
        veh_drop(lifted_vehicle, destination_x, destination_y);
        vehicle.order = 0;
        vehicle.state &= 0xFCFFBDFFu;

        const int chassis_id_after_move = VehPrototypes[vehicle.proto_id].chassis_id;
        // Native receiver points at Veh.y (+2); its byte +0x23 is the
        // terraforming counter (+0x25), not the adjacent morale byte.
        if (Chassis[chassis_id_after_move].triad == TRIAD_AIR)
            vehicle.terraforming_turns = 0;

        ++lifted_count;
        if (ConsoleGlobal.cursor_active_ != 0 &&
            vehicle_id == ConsoleGlobal.selected_vehicle_) {
            ConsoleGlobal.set_move(1);
        }
    }
    return lifted_count;
}

// ORIGINAL: 0x005267B0 ?allocate_energy@@YAXH@Z 0x005267B0-0x00526E63
// RECOVERY_BINDING: v1 {"evidence": ["original:0x005267B0-0x00526E63", "git:38853a3f:src/unrecovered/005267b0.cpp", "source:src/faction.h", "source:src/base.h", "source:src/veh.h"], "reason": "Recover the complete faction energy allocation operation through real PlayerData, Base, Console and VehPrototype owners. Preserve the original first scan of global BaseCurrent without selecting each base, the later severe-police selection/recompute sequence, signed allocation arithmetic and callback order; historical spelling notes are not fresh measurements."}
// TRIED: direct register-mirroring transcription (eax/ebx/.../var_4/var_c/var_10 locals) compiles and matches control flow/offsets faithfully but the extra scratch locals (vs the original's 6-int 0x18 frame) shift register allocation from instr #4; sim 0.68 framed. Not retried with fewer locals given size (1715 bytes).
void __cdecl allocate_energy(int faction_id) {
    PlayerData &faction = PlayersData[faction_id];
    const unsigned int faction_bit = 1u << (faction_id & 31);

    // Human-only emergency notification path, intentionally distinct from
    // the social-allocation rebalance below.
    if ((factions_status_byte(0) & faction_bit) != 0) {
        if (faction.diff_level <= 1 || faction_id != ConsoleGlobal.faction_id_
            || faction.pop_total <= 1 || faction.energy_reserves >= 100)
            return;
        if (diplo_mul(faction.energy_reserves, 10)
            >= diplo_mul(CurrentFactionEnergyReserves, 10))
            return;
        const int unassigned = 10 - faction.se_alloc_psych - faction.se_alloc_labs;
        if (unassigned < 5) {
            if (popp(ScriptPopupFile, "INFRASTRUCTURE", 0, "genwarning_sm.pcx", nullptr) == 0)
                return;
            social_select(faction_id);
            return;
        }
        if (!has_tech(VehPrototypes[1].preq_tech, faction_id)) return;
        if (TurnCurrentNum % 5 != 0) return;
        popp(ScriptPopupFile, "INFRASTRUCTURE2", 0, "genwarning_sm.pcx", nullptr);
        return;
    }

    const int desired_total = 10 - faction.se_alloc_psych - faction.se_alloc_labs;
    const int police = faction.soc_effect_pending.police;
    int max_psych;
    if (police < -2) max_psych = 4;
    else if (police == -2) max_psych = 2;
    else if (police <= 1) max_psych = 1;
    else max_psych = 0;

    int &psych = faction.se_alloc_psych;
    int skip_increase = 1;
    const int first_pass_base_count = BaseCurrentCount;
    for (int base_id = 0; base_id < first_pass_base_count; ++base_id) {
        if (Bases[base_id].faction_id_current == faction_id) {
            if ((BaseCurrent->state & 2u) != 0) goto increase_psych;
            if (BaseCurrent->specialist_total > BaseCurrent->specialist_unk_1)
                skip_increase = 0;
        }
    }
    if (skip_increase == 0) {
        const int min_psych = max_psych / 2;
        if (psych > min_psych) --psych;
        goto psych_rebalance_done;
    }

increase_psych:
    if (psych >= max_psych) goto psych_rebalance_done;
    ++psych;
    if (psych >= max_psych) goto psych_rebalance_done;
    if (police >= -2) goto psych_rebalance_done;

    // Under severe police effects the native path recomputes each owned base
    // before checking its current talent/drone totals and its riot bit.
    {
        for (int base_id2 = 0; base_id2 < BaseCurrentCount; ++base_id2) {
            if (Bases[base_id2].faction_id_current == faction_id) {
                set_base(base_id2);
                base_compute(1);
                if (BaseCurrent->talent_total < BaseCurrent->drone_total
                    && (BaseCurrent->state & 2u) != 0) {
                    ++psych;
                    if (psych >= max_psych) break;
                }
            }
        }
    }

psych_rebalance_done:
    const int min_psych = police >= -1 ? 0 : 2;
    if (psych < min_psych) psych = min_psych;
    else if (psych > 4) psych = 4;
    if (has_project(SP_TELEPATHIC_MATRIX, faction_id)) psych = 0;
    faction.se_alloc_psych = psych;

    int initial_labs = diplo_sub(
        diplo_sub(faction.ai_tech, faction.ai_power), faction.se_alloc_psych);
    initial_labs = diplo_add(initial_labs, 11) / 2;
    initial_labs = diplo_sub(initial_labs, faction.ai_wealth);
    faction.se_alloc_labs = initial_labs;
    int &labs = faction.se_alloc_labs;
    if (diplo_mul(
            diplo_add(
                diplo_sub(diplo_sub(faction.ai_tech, faction.ai_power),
                               faction.ai_wealth), 3), 25) > TurnCurrentNum)
        ++labs;

    const int research_denominator = diplo_add(
        diplo_add(faction.ai_power, faction.ai_wealth), 1);
    const int tech_cost = diplo_add(
        diplo_mul(Players[faction_id].rule_techcost, research_denominator),
        TurnCurrentNum);
    labs = diplo_add(labs, faction.energy_reserves / diplo_mul(tech_cost, 2));

    int region_id = 1;
    while (region_id < 0x80) {
        if (faction.region_total_bases[region_id] != 0) {
            if (faction.region_base_plan[region_id] == 2) {
                --labs;
                break;
            }
            if (faction.region_base_plan[region_id] == 0
                && (faction.ai_power != 0 || faction.ai_wealth != 0))
                --labs;
        }
        ++region_id;
    }
    if (faction.ai_tech != 0 && faction.ai_power == 0 && faction.ai_wealth == 0
        && region_id >= 0x80)
        ++labs;

    if (desired_total != 0) {
        const int free_slots = 10 - labs - psych;
        if (free_slots >= desired_total) {
            int base_value = 0;
            for (int base_id3 = 0; base_id3 < BaseCurrentCount; ++base_id3) {
                if (Bases[base_id3].faction_id_current == faction_id) {
                    base_value = diplo_add(base_value, Bases[base_id3].unk_10);
                    for (int facility_id = 0; facility_id < 0x46; ++facility_id) {
                        if (facility_id < 0x41
                            && has_fac_built(facility_id, base_id3))
                            base_value = diplo_sub(
                                base_value, fac_maint(facility_id, faction_id));
                    }
                }
            }
            const int expected_value = diplo_mul(
                diplo_mul(
                    diplo_sub(diplo_add(
                        diplo_mul(faction.ai_wealth, 2), 4), faction.ai_tech),
                    faction.pop_total), 1) / 4;
            if (base_value > expected_value) {
                while (10 - labs - psych >= desired_total) ++labs;
            } else if (base_value < expected_value / 2) {
                --labs;
            }
        }
    }

    if (faction.tech_id_researching == 0x58) --labs;
    const int max_by_psych = 10 - psych;
    const int energy_cap = energy_limit(faction_id);
    if (labs < 0 || energy_cap < 0) labs = 0;
    else if (labs > energy_cap) labs = energy_cap;
    if (labs > max_by_psych) labs = max_by_psych;
    faction.se_alloc_labs = labs;

    int remaining = 10 - psych - labs;
    const int second_cap = energy_limit(faction_id);
    if (remaining > second_cap) {
        do {
            ++labs;
            faction.se_alloc_labs = labs;
            remaining = 10 - psych - labs;
        } while (remaining > energy_limit(faction_id));
    }

    int efficiency = faction.soc_effect_temp.efficiency;
    if (efficiency < 0) efficiency = 0;
    else if (efficiency > 0x63) efficiency = 0x63;
    int population_adjustment = 3 - faction.pop_total;
    if (population_adjustment < 0) population_adjustment = 0;
    else if (population_adjustment > 3) population_adjustment = 3;
    const int threshold = population_adjustment + efficiency + 1;
    if (labs < 0) labs = 0;
    else if (labs > 10) labs = 10;
    faction.se_alloc_labs = labs;
    while (labs > threshold - psych - labs + 10) {
        --labs;
        faction.se_alloc_labs = labs;
    }
    while (10 - labs - psych > labs + threshold) {
        ++labs;
        faction.se_alloc_labs = labs;
    }
}

// ORIGINAL: 0x0054FFD0 ?communicate@@YAXHHH@Z 0x0054FFD0-0x00558980;0x0065FF4F-0x0065FF64
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0054FFD0-0x00558980;0x0065FF4F-0x0065FF64", "source:src/unrecovered/0054ffd0.cpp:retained-seed", "task:base-colony-economy-coverage-wave:diplomacy-communicate-link-continuity-20261003"], "reason": "Explicitly unfinished owner body for link continuity while complete communicate hot/cold behavior remains under recovery. The seed and original mapping are retained; this empty body does not implement diplomacy, reproduce its bytes or satisfy the pending dependency."}
// TRIED: the seven-way relationship-topic greeting text build (war/ vendetta/truce/treaty/pact, chosen via has_treaty bits and distance, each with its own get_adjective_leader/ get_noun_phrase string chain and jump-table case 0-6) was collapsed to one generic diplomacy_caption() + say_num(0) call. The diplomacy_menu() result loop and its 8-way action dispatch (cases 0/1-2/3/4/5/6/7) were read from the raw disassembly (no Ghidra hypothesis was reliable there either) and reproduced with real callee names/order; the patience/ loop-exit condition nesting is approximated, not exact.
void __cdecl communicate(int faction_id, int faction_id_with, int mode) {
    // BODY GOES HERE.
}

// ORIGINAL: 0x0055C430 ?comm_check@@YAXHH@Z 0x0055C430-0x0055C726
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055C430:complete-mapped-spans", "git:13693539:src/recovered/units/0055c430.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Retain the original multiplayer and PBEM paths, asymmetric pending row and treaty writers. Recheck the local console and treaty state after modal notifications; forward the original actor, peer and zero mode to the separately unfinished communicate owner."}
void __cdecl comm_check(int faction_id, int faction_id_with) {
    const uint32_t human_mask = LockEnableMask & 0xFFu;
    if ((human_mask & diplo_bit(faction_id)) == 0) {
        return;
    }

    const uint32_t treaty =
        PlayersData[faction_id].diplo_treaties[faction_id_with];
    const bool has_comm_link = (treaty & DTREATY_COMMLINK) != 0;
    if (has_comm_link && (treaty & DTREATY_WANT_TO_TALK) == 0) {
        const int last_contact =
            PlayersData[faction_id].diplo_spoke[faction_id_with];
        if (last_contact >= 0 &&
            diplo_sub(TurnCurrentNum, last_contact) < 16) {
            return;
        }
    }

    if (IsMultiplayerNet != 0) {
        if (faction_id == ConsoleGlobal.faction_id_) {
            if ((human_mask & diplo_bit(faction_id_with)) != 0) {
                if (!has_comm_link) {
                    treaty_on(faction_id, faction_id_with, 0x10004);
                }
            } else {
                uint32_t &diplomacy_flags =
                    FactionDiplomacyFlags[faction_id_with];
                diplomacy_flags |= 1u;
                if (has_comm_link) {
                    return;
                }
                diplomacy_flags |= 2u;
                net_treaty_on(faction_id, faction_id_with, 0x10004, 0);
                return;
            }
        }

        if (faction_id == ConsoleGlobal.faction_id_ &&
            (PlayersData[faction_id].diplo_treaties[faction_id_with] &
             0x08000000u) == 0) {
            GenderDefault = Players[faction_id_with].is_leader_female;
            PluralityDefault = FALSE;
            parse_says(0, Players[faction_id_with].title_leader, -1, -1);
            GenderDefault = Players[faction_id_with].is_leader_female;
            PluralityDefault = FALSE;
            parse_says(1, Players[faction_id_with].name_leader, -1, -1);
            GenderDefault = Players[faction_id_with].noun_gender;
            PluralityDefault = Players[faction_id_with].is_noun_plural;
            parse_says(2, Players[faction_id_with].noun_faction, -1, -1);
            NetMsgPrimary.pop("NETMET", 5000, 0, nullptr);
        }

        // Native code rechecks the console faction and treaty after the modal
        // notification; either may have changed during NetMsg::pop.
        if (faction_id_with == ConsoleGlobal.faction_id_ &&
            (PlayersData[faction_id].diplo_treaties[faction_id_with] &
             0x08000000u) == 0) {
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = FALSE;
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            GenderDefault = Players[faction_id].is_leader_female;
            PluralityDefault = FALSE;
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            GenderDefault = Players[faction_id].noun_gender;
            PluralityDefault = Players[faction_id].is_noun_plural;
            parse_says(2, Players[faction_id].noun_faction, -1, -1);
            NetMsgPrimary.pop("NETMET", 5000, 0, nullptr);
            return;
        }

        return;
    }

    if (IsMultiplayerPBEM != 0) {
        if ((human_mask & diplo_bit(faction_id_with)) != 0) {
            if (!has_comm_link) {
                treaty_on(faction_id, faction_id_with, 0x10004);
            }
            return;
        }

        if (faction_id != ConsoleGlobal.faction_id_) {
            uint32_t &pending =
                PendingDiplomacyFlags[faction_id][faction_id_with];
            pending |= 1u;
            if (has_comm_link) {
                return;
            }
            pending |= 2u;
            treaty_on(faction_id, faction_id_with, 0x10004);
            return;
        }
    }

    communicate(faction_id, faction_id_with, 0);
}

// ORIGINAL: 0x0055C840 ?enemies_war@@YAXHH@Z 0x0055C840-0x0055CC9A
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055C840:complete-mapped-spans", "git:13693539:src/recovered/units/0055c840.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Use genuine player treaty rows, local console, owned NetMsg and parser state for war declarations. The observed protected-notice bit is 0x1000, not 0x100000; preserve the original treaty and agenda calls and notice ordering."}
void __cdecl enemies_war(int faction_id, int faction_id_with) {
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with]
         & DTREATY_VENDETTA) != 0) {
        return;
    }

    const int local_faction_id = ConsoleGlobal.faction_id_;
    if ((PlayersData[local_faction_id].diplo_treaties[faction_id]
         & DTREATY_PACT) != 0) {
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(1, Players[faction_id].name_leader, -1, -1);
        parse_says(2, get_noun(faction_id), -1, -1);
        parse_says(3, get_he_she(faction_id, 0), -1, -1);
        parse_says(4, get_noun(faction_id_with), -1, -1);
        NetMsgPrimary.pop("PACTATTACKS", 5000, 0, nullptr);
    } else if ((PlayersData[local_faction_id].diplo_treaties[faction_id_with]
                & DTREATY_PACT) != 0) {
        GenderDefault = Players[faction_id_with].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(0, Players[faction_id_with].title_leader, -1, -1);
        GenderDefault = Players[faction_id_with].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(1, Players[faction_id_with].name_leader, -1, -1);
        parse_says(2, get_noun(faction_id_with), -1, -1);
        parse_says(3, get_he_she(faction_id_with, 0), -1, -1);
        parse_says(4, get_noun(faction_id), -1, -1);
        NetMsgPrimary.pop("PACTATTACKS", 5000, 0, nullptr);
    } else {
        const uint32_t protected_diplomacy_bits = 0x00001000u;
        bool notify =
            (PlayersData[local_faction_id].diplo_treaties[faction_id]
             & protected_diplomacy_bits) != 0;
        if (!notify) {
            notify = has_project(SP_EMPATH_GUILD, local_faction_id) != 0;
        }
        if (!notify && local_faction_id == GovernorFaction) {
            notify = (Players[faction_id].rule_flags & RFLAG_ALIEN) == 0;
        }
        if (!notify) {
            notify =
                (PlayersData[local_faction_id].diplo_treaties[faction_id_with]
                 & protected_diplomacy_bits) != 0;
        }
        if (!notify) {
            notify = has_project(SP_EMPATH_GUILD, local_faction_id) != 0;
        }
        if (!notify && local_faction_id == GovernorFaction) {
            notify = (Players[faction_id_with].rule_flags & RFLAG_ALIEN) == 0;
        }
        if (!notify) {
            notify = (GameState & 0x80) != 0;
        }
        if (notify) {
            GenderDefault = Players[faction_id].noun_gender;
            PluralityDefault = Players[faction_id].is_noun_plural;
            parse_says(0, Players[faction_id].noun_faction, -1, -1);
            GenderDefault = Players[faction_id_with].noun_gender;
            PluralityDefault = Players[faction_id_with].is_noun_plural;
            parse_says(1, Players[faction_id_with].noun_faction, -1, -1);
            NetMsgPrimary.pop("ENEMYWAR", 5000, 0, nullptr);
        }
    }

    treaty_on(faction_id, faction_id_with, DTREATY_VENDETTA);
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with]
         & 0x40000000u) == 0) {
        trade_maps(faction_id, faction_id_with);
    }

    if ((PlayersData[faction_id].diplo_treaties[faction_id_with]
         & DTREATY_WANT_REVENGE) != 0 &&
        (PlayersData[faction_id].diplo_treaties[faction_id_with]
         & DTREATY_ATROCITY_VICTIM) == 0 && DiffLevelCurrent > 1 &&
        rand() % DiffLevelCurrent != 0) {
        PlayersData[faction_id].diplo_agenda[faction_id_with] &=
            ~(DAGENDA_UNK_400 | DAGENDA_UNK_800);
        PlayersData[faction_id_with].diplo_agenda[faction_id] &=
            ~(DAGENDA_UNK_400 | DAGENDA_UNK_800);
        PlayersData[faction_id].diplo_treaties[faction_id_with] &=
            ~DTREATY_WANT_REVENGE;
        PlayersData[faction_id_with].diplo_treaties[faction_id] &=
            ~DTREATY_WANT_REVENGE;
        PlayersData[faction_id].diplo_treaties[faction_id_with] |=
            DTREATY_UNK_40;
        PlayersData[faction_id].diplo_merc[faction_id_with] = 50;
    }

    if ((PlayersData[faction_id_with].diplo_treaties[faction_id]
         & DTREATY_WANT_REVENGE) != 0 &&
        (PlayersData[faction_id_with].diplo_treaties[faction_id]
         & DTREATY_ATROCITY_VICTIM) == 0 && DiffLevelCurrent > 1 &&
        rand() % DiffLevelCurrent != 0) {
        PlayersData[faction_id_with].diplo_agenda[faction_id] &=
            ~(DAGENDA_UNK_400 | DAGENDA_UNK_800);
        PlayersData[faction_id].diplo_agenda[faction_id_with] &=
            ~(DAGENDA_UNK_400 | DAGENDA_UNK_800);
        PlayersData[faction_id_with].diplo_treaties[faction_id] &=
            ~DTREATY_WANT_REVENGE;
        PlayersData[faction_id].diplo_treaties[faction_id_with] &=
            ~DTREATY_WANT_REVENGE;
        PlayersData[faction_id_with].diplo_treaties[faction_id] |=
            DTREATY_UNK_40;
        set_treaty(faction_id_with, faction_id, DTREATY_UNK_40, 1);
    }
}

// ORIGINAL: 0x0055CE20 ?enemies_unpact@@YAXHH@Z 0x0055CE20-0x0055D031
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055CE20:complete-mapped-spans", "git:13693539:src/recovered/units/0055ce20.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Recover pact withdrawal through genuine player rows, pact_unpact calls and the existing Empath Guild project/base ownership. Preserve directional calls, protected-notice bit 0x1000, and both-direction treaty clearing mask 0x01100001."}
void __cdecl enemies_unpact(int faction_id, int faction_id_with) {
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with]
         & DTREATY_PACT) == 0) {
        return;
    }

    const int local_faction_id = ConsoleGlobal.faction_id_;
    if ((PlayersData[faction_id].diplo_treaties[local_faction_id]
         & DTREATY_PACT) != 0) {
        pact_unpact(faction_id, faction_id_with);
    } else if ((PlayersData[faction_id_with].diplo_treaties[local_faction_id]
                & DTREATY_PACT) != 0) {
        pact_unpact(faction_id_with, faction_id);
    }

    const int current_faction_id = ConsoleGlobal.faction_id_;
    const uint32_t protected_diplomacy_bits = 0x00001000u;
    bool show_notice =
        (PlayersData[current_faction_id].diplo_treaties[faction_id]
         & protected_diplomacy_bits) != 0;
    const int empath_guild_base = SecretProject.empath_guild;
    if (!show_notice && empath_guild_base >= 0) {
        show_notice =
            Bases[empath_guild_base].faction_id_current == current_faction_id;
    }
    if (!show_notice && current_faction_id == GovernorFaction) {
        show_notice = (Players[faction_id].rule_flags & RFLAG_ALIEN) == 0;
    }
    if (!show_notice) {
        show_notice =
            (PlayersData[current_faction_id].diplo_treaties[faction_id_with]
             & protected_diplomacy_bits) != 0;
    }
    if (!show_notice && empath_guild_base >= 0) {
        show_notice =
            Bases[empath_guild_base].faction_id_current == current_faction_id;
    }
    if (!show_notice && current_faction_id == GovernorFaction) {
        show_notice = (Players[faction_id_with].rule_flags & RFLAG_ALIEN) == 0;
    }
    if (!show_notice) {
        show_notice = (GameState & 0x80) != 0;
    }

    if (show_notice) {
        GenderDefault = Players[faction_id].noun_gender;
        PluralityDefault = Players[faction_id].is_noun_plural;
        parse_says(0, Players[faction_id].noun_faction, -1, -1);

        GenderDefault = Players[faction_id_with].noun_gender;
        PluralityDefault = Players[faction_id_with].is_noun_plural;
        parse_says(1, Players[faction_id_with].noun_faction, -1, -1);
        parse_says(2, get_pact_hood(faction_id, faction_id_with), -1, -1);
        NetMsgPrimary.pop("ENEMYPACTENDS", 5000, 0, nullptr);
    }

    const uint32_t cleared_native_status_bits = 0x01100001u;
    PlayersData[faction_id].diplo_treaties[faction_id_with] &=
        ~cleared_native_status_bits;
    PlayersData[faction_id_with].diplo_treaties[faction_id] &=
        ~cleared_native_status_bits;
}

// ORIGINAL: 0x0055D430 ?enemies_trade_tech@@YAXHH@Z 0x0055D430-0x0055D753
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055D430:complete-mapped-spans", "git:13693539:src/unrecovered/0055d430.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Recover bilateral and conditional one-way technology transfers using typed player technology-source bytes, treaty and friction rows. The second direction intentionally uses actor source/relation but reloads the peer source for peer friction; repeated has_tech calls and signed wrapped valuation decrement are original behavior."}
// TRIED: 0x20cc-stride diplomacy tables (0x96cd4c tech-source byte, 0x96c9f8 relation-flags dword, 0x96ca38 tech-level dword, 0x96cd3c tech-rank dword) rederived directly from the lea-chain address math (fac*65,*131,*1049,*2099 -> fac*0x20cc), not from Ghidra's mixed-unit indexing. Matches to #23/803 bytes; the nested-if rewrite of the has_tech/goto-LAB_0055d552 sharing is semantically equivalent but not chased for byte order.
void __cdecl enemies_trade_tech(int faction_id, int faction_id_with) {
    if ((GameRules & 0x02000000u) != 0) return;
    int best_tech_to_faction = -1, best_tech_to_partner = -1;
    int best_value_to_faction = -9999, best_value_to_partner = -9999;

    for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
        if (!has_tech(tech_id, faction_id)) {
            if (has_tech(tech_id, faction_id_with)) {
                const int donor_source =
                    PlayersData[faction_id_with].tech_trade_source[tech_id];
                bool eligible = true;
                if (donor_source != 0 &&
                    (PlayersData[faction_id_with].diplo_treaties[donor_source]
                     & DTREATY_PACT) != 0) eligible = false;
                if (eligible &&
                    (PlayersData[faction_id_with].diplo_treaties[donor_source]
                     & DTREATY_TREATY) != 0) {
                    const int minimum_friction =
                        (PlayersData[faction_id].diplo_treaties[faction_id_with]
                         & DTREATY_PACT) != 0 ? 6 : 16;
                    if (PlayersData[faction_id_with].diplo_friction[donor_source]
                        < minimum_friction) eligible = false;
                }
                if (eligible) {
                    const int value = tech_val(tech_id, faction_id, FALSE);
                    const int score = diplo_sub(value, 1) > 0
                        ? rand() % value : 0;
                    if (score > best_value_to_faction) {
                        best_value_to_faction = score;
                        best_tech_to_faction = tech_id;
                    }
                }
                // Native jumps to the next tech after considering this path.
                continue;
            }
            // When neither faction owns it, execution falls through and
            // performs the native second peer/actor query pair below.
        }

        if (has_tech(tech_id, faction_id_with)) continue;
        if (!has_tech(tech_id, faction_id)) continue;

        const int donor_source =
            PlayersData[faction_id].tech_trade_source[tech_id];
        bool eligible = true;
        if (donor_source != 0 &&
            (PlayersData[faction_id].diplo_treaties[donor_source]
             & DTREATY_PACT) != 0) eligible = false;
        if (eligible &&
            (PlayersData[faction_id].diplo_treaties[donor_source]
             & DTREATY_TREATY) != 0) {
            const int minimum_friction =
                (PlayersData[faction_id].diplo_treaties[faction_id_with]
                 & DTREATY_PACT) != 0 ? 6 : 16;
            const int recipient_source =
                PlayersData[faction_id_with].tech_trade_source[tech_id];
            if (PlayersData[faction_id_with].diplo_friction[recipient_source]
                < minimum_friction) eligible = false;
        }
        if (eligible) {
            const int value = tech_val(tech_id, faction_id_with, FALSE);
            const int score = diplo_sub(value, 1) > 0 ? rand() % value : 0;
            if (score > best_value_to_partner) {
                best_value_to_partner = score;
                best_tech_to_partner = tech_id;
            }
        }
    }

    if (best_tech_to_faction >= 0 && best_tech_to_partner >= 0) {
        tech_achieved(faction_id, best_tech_to_faction, faction_id_with, 0);
        tech_achieved(faction_id_with, best_tech_to_partner, faction_id, 0);
        return;
    }
    if ((PlayersData[faction_id].diplo_treaties[faction_id_with]
         & DTREATY_PACT) == 0 ||
        !great_satan(FactionRankings[MaxPlayerNum - 1], 0)) return;
    if (best_tech_to_faction >= 0) {
        const int leader = static_cast<int>(RankingFactionIDUnk1);
        if (PlayersData[leader].tech_ranking >=
            PlayersData[faction_id_with].tech_ranking)
            tech_achieved(faction_id, best_tech_to_faction, faction_id_with, 0);
    }
    if (best_tech_to_partner >= 0) {
        const int leader = static_cast<int>(RankingFactionIDUnk1);
        if (PlayersData[leader].tech_ranking >= PlayersData[faction_id].tech_ranking)
            tech_achieved(faction_id_with, best_tech_to_partner, faction_id, 0);
    }
}

static int faction_treaty_signed_random16() {
    const uint32_t masked_random =
        static_cast<uint32_t>(rand()) & 0x8000000Fu;
    const int low_bits = static_cast<int>(masked_random & 0x0Fu);
    if ((masked_random & 0x80000000u) == 0 || low_bits == 0) {
        return low_bits;
    }
    return low_bits - 16;
}

static void faction_parse_leader_fields(int faction_id, int title_slot,
                                        int name_slot, int noun_slot,
                                        int adj_slot) {
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(title_slot, Players[faction_id].title_leader, -1, -1);
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(name_slot, Players[faction_id].name_leader, -1, -1);
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(noun_slot, Players[faction_id].noun_faction, -1, -1);
    if (adj_slot >= 0) {
        parse_says(adj_slot, get_he_she(faction_id, 0), -1, -1);
    }
}

void __cdecl enemies_unpact(int faction_id, int faction_id_with);

// ORIGINAL: 0x0055D760 ?enemies_treaty@@YAXHHH@Z 0x0055D760-0x0055E370
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055D760:complete-mapped-spans", "git:13693539:src/unrecovered/0055d760.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Recover the complete original truce/treaty/pact decision paths through typed player rows, ranking, rules and genuine parser/NetMsg calls. Initial pact notices use actor noun in slot 6; lower local-pact notices use the opposite faction noun in slot 4, PACTTREATY and treaty bit 2. Keep the original signed random masking and ally scan."}
// TRIED: shared-tail gotos (LAB_0055dab5/LAB_0055ddb9) replaced with inline duplication at each call site (same call sequence, fewer scope-crossing gotos); parse_says calls normalized to their real 4-arg form (idx, ptr, -1, -1) where Ghidra's own printout dropped the trailing -1,-1 pair pushed by a sibling call and shared across the two (get_noun/get_he_she then parse_says, same push-sharing idiom as veh_lift/veh_drop).
void __cdecl enemies_treaty(int faction_id, int faction_id_with,
                             int check_technology) {
    if (check_technology != 0) {
        enemies_trade_tech(faction_id, faction_id_with);
    }

    uint32_t relationship =
        PlayersData[faction_id].diplo_treaties[faction_id_with];
    if ((relationship & DTREATY_TREATY) == 0 &&
        (relationship & (DTREATY_TRUCE | DTREATY_VENDETTA)) != 0) {
        const uint32_t reverse_relationship =
            PlayersData[faction_id_with].diplo_treaties[faction_id];
        if ((relationship & 0x60u) == 0 &&
            (reverse_relationship & 0x60u) == 0 &&
            (PlayersData[faction_id_with].ai_fight < 6 ||
             PlayersData[faction_id].ai_fight < 6)) {
            const int threshold =
                (GameRules & 0x40u) != 0 ? 5 : DiffLevelCurrent;
            const int aggression_total = diplo_add(
                diplo_sub(diplo_add(PlayersData[faction_id].ai_fight,
                                    PlayersData[faction_id_with].ai_fight),
                          threshold),
                8);
            if (faction_treaty_signed_random16() >= aggression_total) {
                goto evaluate_existing_state;
            }
        }

        const int local_faction_id = ConsoleGlobal.faction_id_;
        const uint32_t local_to_a =
            PlayersData[local_faction_id].diplo_treaties[faction_id];
        if ((local_to_a & DTREATY_PACT) == 0) {
            const uint32_t local_to_b =
                PlayersData[local_faction_id].diplo_treaties[faction_id_with];
            if ((local_to_b & DTREATY_PACT) != 0) {
                faction_parse_leader_fields(faction_id_with, 0, 1, 2, 3);
                PluralityDefault = FALSE;
                GenderDefault = Players[faction_id].is_leader_female;
                parse_says(4, Players[faction_id].title_leader, -1, -1);
                PluralityDefault = FALSE;
                GenderDefault = Players[faction_id].is_leader_female;
                parse_says(5, Players[faction_id].name_leader, -1, -1);
                parse_says(6, get_noun(faction_id), -1, -1);
                NetMsgPrimary.pop("PACTTRUCE", 5000, 0, nullptr);
                treaty_on(faction_id, faction_id_with, DTREATY_TRUCE);
                return;
            }

            bool automatic_truce =
                (PlayersData[local_faction_id].diplo_treaties[faction_id] &
                 0x1000u) == 0 &&
                !has_project(SP_EMPATH_GUILD, local_faction_id) &&
                (local_faction_id != GovernorFaction ||
                 (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) &&
                (PlayersData[local_faction_id].diplo_treaties[faction_id_with] &
                 0x1000u) == 0 &&
                !has_project(SP_EMPATH_GUILD, local_faction_id) &&
                (local_faction_id != GovernorFaction ||
                 (Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0) &&
                (GameState & 0x80u) == 0;
            if (automatic_truce) {
                treaty_on(faction_id, faction_id_with, DTREATY_TRUCE);
                return;
            }

            GenderDefault = Players[faction_id].noun_gender;
            PluralityDefault = Players[faction_id].is_noun_plural;
            parse_says(0, Players[faction_id].noun_faction, -1, -1);
            GenderDefault = Players[faction_id_with].noun_gender;
            PluralityDefault = Players[faction_id_with].is_noun_plural;
            parse_says(1, Players[faction_id_with].noun_faction, -1, -1);
            NetMsgPrimary.pop("ENEMYTREATY", 5000, 0, nullptr);
            treaty_on(faction_id, faction_id_with, DTREATY_TRUCE);
            return;
        }

        faction_parse_leader_fields(faction_id, 0, 1, 2, 3);
        PluralityDefault = FALSE;
        GenderDefault = Players[faction_id_with].is_leader_female;
        parse_says(4, Players[faction_id_with].title_leader, -1, -1);
        PluralityDefault = FALSE;
        GenderDefault = Players[faction_id_with].is_leader_female;
        parse_says(5, Players[faction_id_with].name_leader, -1, -1);
        parse_says(6, get_noun(faction_id), -1, -1);
        NetMsgPrimary.pop("PACTTRUCE", 5000, 0, nullptr);
        treaty_on(faction_id, faction_id_with, DTREATY_TRUCE);
        return;
    }

evaluate_existing_state:
    relationship = PlayersData[faction_id].diplo_treaties[faction_id_with];
    if ((relationship & DTREATY_TREATY) != 0) {
        const bool keep_war =
            (FactionRankings[MaxPlayerNum - 1] !=
                 static_cast<uint32_t>(faction_id) ||
             PlayersData[faction_id_with].ai_fight < 5) &&
            (FactionRankings[MaxPlayerNum - 1] !=
                 static_cast<uint32_t>(faction_id_with) ||
             PlayersData[faction_id].ai_fight < 5);
        if (keep_war &&
            (relationship & 0x00040020u) == 0 &&
            (PlayersData[faction_id_with].diplo_treaties[faction_id] &
             0x00040020u) == 0 &&
            PlayersData[faction_id].major_atrocities == 0 &&
            PlayersData[faction_id_with].major_atrocities == 0) {
            int existing_ally_count = 0;
            for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
                if (candidate == faction_id || candidate == faction_id_with) {
                    continue;
                }

                const uint32_t relation_to_a =
                    PlayersData[faction_id].diplo_treaties[candidate];
                const uint32_t relation_to_b =
                    PlayersData[faction_id_with].diplo_treaties[candidate];
                const bool either_has_link =
                    (relation_to_a & DTREATY_COMMLINK) != 0 ||
                    (relation_to_b & DTREATY_COMMLINK) != 0;
                const bool both_have_link =
                    (relation_to_a & DTREATY_COMMLINK) != 0 &&
                    (relation_to_b & DTREATY_COMMLINK) != 0;
                if (!either_has_link ||
                    (!both_have_link && !great_satan(candidate, FALSE))) {
                    continue;
                }

                const bool neither_has_pact =
                    (relation_to_a & DTREATY_PACT) == 0 &&
                    (relation_to_b & DTREATY_PACT) == 0;
                const bool native_candidate_eligible =
                    neither_has_pact &&
                    PlayersData[candidate].ranking != 0 &&
                    (great_satan(candidate, FALSE) ||
                     (((relation_to_a & 7u) == 0) &&
                      ((relation_to_b & 7u) == 0) &&
                      (((relationship & DTREATY_PACT) != 0) ||
                       (((relation_to_a & DTREATY_VENDETTA) != 0) &&
                        (PlayersData[faction_id].ranking <
                             PlayersData[candidate].ranking ||
                         PlayersData[faction_id_with].ranking <
                             PlayersData[candidate].ranking)))));
                if (native_candidate_eligible) {
                    if ((relationship & DTREATY_PACT) != 0) {
                        return;
                    }
                    const int candidate_rank = great_satan(candidate, FALSE)
                                                   ? static_cast<int>(PlayersData[candidate].ranking)
                                                   : 0;
                    if (existing_ally_count <= candidate_rank) {
                        treaty_on(faction_id, faction_id_with, DTREATY_PACT);
                        enemies_team_up(faction_id, faction_id_with, candidate);
                        enemies_team_up(faction_id_with, faction_id, candidate);

                        const int current_faction_id = ConsoleGlobal.faction_id_;
                        bool suppress_message =
                            candidate != current_faction_id &&
                            (PlayersData[current_faction_id].diplo_treaties[
                                 faction_id] & 0x1000u) == 0;
                        const int empath_guild_base =
                            SecretProject.empath_guild;
                        if (suppress_message && empath_guild_base >= 0) {
                            suppress_message =
                                Bases[empath_guild_base].faction_id_current !=
                                current_faction_id;
                        }
                        if (suppress_message &&
                            current_faction_id == GovernorFaction) {
                            suppress_message =
                                (Players[faction_id].rule_flags & RFLAG_ALIEN) !=
                                0;
                        }
                        if (suppress_message) {
                            suppress_message =
                                (PlayersData[current_faction_id]
                                     .diplo_treaties[faction_id_with] &
                                 0x1000u) == 0;
                        }
                        if (suppress_message && empath_guild_base >= 0) {
                            suppress_message =
                                Bases[empath_guild_base].faction_id_current !=
                                current_faction_id;
                        }
                        if (suppress_message &&
                            current_faction_id == GovernorFaction) {
                            suppress_message =
                                (Players[faction_id_with].rule_flags &
                                 RFLAG_ALIEN) != 0;
                        }

                        if (!suppress_message) {
                            GenderDefault = Players[faction_id].noun_gender;
                            PluralityDefault = Players[faction_id].is_noun_plural;
                            parse_says(0, Players[faction_id].noun_faction,
                                       -1, -1);
                            GenderDefault = Players[faction_id_with].noun_gender;
                            PluralityDefault = Players[faction_id_with].is_noun_plural;
                            parse_says(1,
                                       Players[faction_id_with].noun_faction,
                                       -1, -1);
                            GenderDefault = Players[candidate].noun_gender;
                            PluralityDefault = Players[candidate].is_noun_plural;
                            parse_says(2, Players[candidate].noun_faction,
                                       -1, -1);
                            StringTemp[0] = '\0';
                            strcat(StringTemp, "ENEMIESTEAMUP");
                            if (candidate == ConsoleGlobal.faction_id_) {
                                if ((PlayersData[faction_id]
                                         .diplo_treaties[candidate] &
                                     DTREATY_VENDETTA) == 0 &&
                                    (PlayersData[faction_id_with]
                                         .diplo_treaties[candidate] &
                                     DTREATY_VENDETTA) == 0) {
                                    strcat(StringTemp, "0");
                                }
                            } else {
                                strcat(StringTemp, "1");
                            }
                            NetMsgPrimary.pop(StringTemp, 5000, 0, nullptr);
                        }
                        return;
                    }
                    break;
                }

                if (!neither_has_pact &&
                    (PlayersData[faction_id].ranking <=
                         PlayersData[candidate].ranking ||
                     PlayersData[faction_id_with].ranking <=
                         PlayersData[candidate].ranking)) {
                    ++existing_ally_count;
                }
            }
        }
        enemies_unpact(faction_id, faction_id_with);
        return;
    }

    const int local_faction_id = ConsoleGlobal.faction_id_;
    if ((PlayersData[local_faction_id].diplo_treaties[faction_id] &
         DTREATY_PACT) == 0) {
        if ((PlayersData[local_faction_id].diplo_treaties[faction_id_with] &
             DTREATY_PACT) != 0) {
            faction_parse_leader_fields(faction_id_with, 0, 1, 2, 3);
            parse_says(4, get_noun(faction_id), -1, -1);
            NetMsgPrimary.pop("PACTTREATY", 5000, 0, nullptr);
            treaty_on(faction_id, faction_id_with, DTREATY_TREATY);
            return;
        }

        GenderDefault = Players[faction_id].noun_gender;
        PluralityDefault = Players[faction_id].is_noun_plural;
        parse_says(0, Players[faction_id].noun_faction, -1, -1);
        GenderDefault = Players[faction_id_with].noun_gender;
        PluralityDefault = Players[faction_id_with].is_noun_plural;
        parse_says(1, Players[faction_id_with].noun_faction, -1, -1);
        NetMsgPrimary.pop("ENEMYTRUCE", 5000, 0, nullptr);
        treaty_on(faction_id, faction_id_with, DTREATY_TRUCE);
        return;
    }

    faction_parse_leader_fields(faction_id, 0, 1, 2, 3);
    parse_says(4, get_noun(faction_id_with), -1, -1);
    NetMsgPrimary.pop("PACTTREATY", 5000, 0, nullptr);
    treaty_on(faction_id, faction_id_with, DTREATY_TREATY);
}

// ORIGINAL: 0x0055E370 ?encounter@@YAXHHHHHH@Z 0x0055E370-0x0055EB7A
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0055E370:complete-mapped-spans", "git:13693539:src/unrecovered/0055e370.cpp", "source:src/faction.h:PlayersData-and-diplomacy-interfaces"], "reason": "Recover human/alien contact, tutorial notifications and AI treaty/war routing in the genuine faction owner. Coordinate checks use full MapLongitudeBounds/MapLatitudeBounds. Preserve short-circuit technology queries, the independent peer classification after actor interludes, console reloads, and classification after parser callbacks. Actor-local and peer-local tutorial paths each retain their human/alien masks and labels; two alien factions suppress all AI responses. Use the existing owned TutorialWindow."}
// TRIED: full structured transcription (verified every call-argument order and jump target against raw disasm) reaches MISMATCH #3 'push' vs 'mov' - a prologue register-allocation divergence (push ebx order), not a logic error; not chased further given function size (2058 bytes).
void __cdecl encounter(int faction_id, int faction_id_with, int x, int y,
                       int event_flag_a, int event_flag_b) {
    if (faction_id == 0 || TurnCurrentNum == 0) return;

    if (faction_id_with == 0) {
        if (faction_id != ConsoleGlobal.faction_id_ ||
            (Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) return;
        if (event_flag_a != 0) interlude(0x2C, nullptr, 1, 0);
        if (event_flag_b != 0) interlude(0x2B, nullptr, 1, 0);
        return;
    }

    const bool faction_is_alien =
        (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;
    const bool partner_is_alien =
        (Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0;
    if (faction_is_alien && partner_is_alien) {
        treaty_on(faction_id, faction_id_with, 0x80000008u);
        if (y >= 0 && y < MapLatitudeBounds &&
            x >= 0 && x < MapLongitudeBounds)
            treaty_on(faction_id, faction_id_with, 0x40000000u);

        StringTemp[0] = '\0';
        strcat(StringTemp, Players[faction_id].search_key);
        CharUpperA(StringTemp);
        const bool is_caretaker = strcmp(StringTemp, "CARETAKE") == 0;
        if (is_caretaker) {
            if (faction_id == ConsoleGlobal.faction_id_) interlude(0x27, nullptr, 1, 0);
            if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x28, nullptr, 1, 0);
            return;
        }
        if (faction_id == ConsoleGlobal.faction_id_) interlude(0x28, nullptr, 1, 0);
        if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x27, nullptr, 1, 0);
        return;
    }

    if (faction_is_alien) {
        if (!has_tech(0x4E, faction_id_with) && !has_tech(4, faction_id)) {
            treaty_on(faction_id, faction_id_with, 0x80000008u);
            if (y >= 0 && y < MapLatitudeBounds &&
                x >= 0 && x < MapLongitudeBounds)
                treaty_on(faction_id, faction_id_with, 0x40000000u);
            if (faction_id == ConsoleGlobal.faction_id_) interlude(0x22, nullptr, 1, 0);
            if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x16, nullptr, 1, 0);
            return;
        }
        if (faction_id == ConsoleGlobal.faction_id_) interlude(0x23, nullptr, 1, 0);
        if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x17, nullptr, 1, 0);
    }
    // The peer classification is read after the actor's interlude callbacks.
    if ((Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0) {
        if (!has_tech(0x4E, faction_id) && !has_tech(4, faction_id_with)) {
            treaty_on(faction_id, faction_id_with, 0x80000008u);
            if (y >= 0 && y < MapLatitudeBounds &&
                x >= 0 && x < MapLongitudeBounds)
                treaty_on(faction_id, faction_id_with, 0x40000000u);
            if (faction_id == ConsoleGlobal.faction_id_) interlude(0x16, nullptr, 1, 0);
            if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x22, nullptr, 1, 0);
            return;
        }
        if (faction_id == ConsoleGlobal.faction_id_) interlude(0x17, nullptr, 1, 0);
        if (faction_id_with == ConsoleGlobal.faction_id_) interlude(0x23, nullptr, 1, 0);
    }

    if (SunspotDuration > 0) {
        treaty_on(faction_id, faction_id_with, 0x80000008u);
        if (y >= 0 && y < MapLatitudeBounds &&
            x >= 0 && x < MapLongitudeBounds)
            treaty_on(faction_id, faction_id_with, 0x40000000u);
        return;
    }

    DiploFocusX = x;
    DiploFocusY = y;
    comm_check(faction_id, faction_id_with);
    comm_check(faction_id_with, faction_id);
    DiploFocusY = -1;
    DiploFocusX = -1;

    const uint32_t treaty =
        PlayersData[faction_id].diplo_treaties[faction_id_with];
    if ((treaty & DTREATY_COMMLINK) == 0 &&
        (((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) ||
         ((Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0))) {
        const char *tutorial_text = nullptr;
        if (faction_id == ConsoleGlobal.faction_id_) {
            GenderDefault = Players[faction_id].noun_gender;
            PluralityDefault = Players[faction_id].is_noun_plural;
            parse_says(0, Players[faction_id].noun_faction, -1, -1);
            if ((Players[faction_id_with].rule_flags & RFLAG_ALIEN) == 0) {
                tutorial_text = !has_tech(0x4E, faction_id) &&
                    !has_tech(4, faction_id_with)
                    ? (tut_check2(0x8000) ? "GUYMEETSALNOPSYCH" : nullptr)
                    : (tut_check2(0x8000) ? "GUYMEETSAL" : nullptr);
            } else if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
                tutorial_text = !has_tech(4, faction_id) &&
                    !has_tech(0x4E, faction_id_with)
                    ? (tut_check2(0x4000) ? "ALMEETSGUYNOPSYCH" : nullptr)
                    : (tut_check2(0x4000) ? "ALMEETSGUY" : nullptr);
            } else if (tut_check2(0x10000)) {
                tutorial_text = "ALMEETSAL";
            }
        } else if (faction_id_with == ConsoleGlobal.faction_id_) {
            GenderDefault = Players[faction_id_with].noun_gender;
            PluralityDefault = Players[faction_id_with].is_noun_plural;
            parse_says(0, Players[faction_id_with].noun_faction, -1, -1);
            if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
                tutorial_text = !has_tech(0x4E, faction_id_with) &&
                    !has_tech(4, faction_id)
                    ? (tut_check2(0x8000) ? "GUYMEETSALNOPSYCH" : nullptr)
                    : (tut_check2(0x8000) ? "GUYMEETSAL" : nullptr);
            } else if ((Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0) {
                tutorial_text = tut_check2(0x10000) ? "ALMEETSAL" : nullptr;
            } else {
                tutorial_text = !has_tech(4, faction_id_with) &&
                    !has_tech(0x4E, faction_id)
                    ? (tut_check2(0x4000) ? "ALMEETSGUYNOPSYCH" : nullptr)
                    : (tut_check2(0x4000) ? "ALMEETSGUY" : nullptr);
            }
        }
        if (tutorial_text != nullptr)
            TutorialWindow.tut_map(tutorial_text, x, y, -1, nullptr, 0, -1, -1);
    }

    treaty_on(faction_id, faction_id_with, 0x88000008u);
    if (y >= 0 && y < MapLatitudeBounds &&
        x >= 0 && x < MapLongitudeBounds)
        treaty_on(faction_id, faction_id_with, 0x40000000u);

    const uint32_t human_mask = static_cast<uint32_t>(LockEnableMask) & 0xFFu;
    if ((human_mask & diplo_bit(faction_id)) != 0 ||
        (human_mask & diplo_bit(faction_id_with)) != 0) return;
    const bool periodic_update =
        (diplo_add(diplo_add(TurnCurrentNum, faction_id), faction_id_with) & 3) == 0;
    if (!periodic_update && (treaty & DTREATY_COMMLINK) != 0) return;
    const bool faction_alien_for_ai =
        (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;
    const bool partner_alien_for_ai =
        (Players[faction_id_with].rule_flags & RFLAG_ALIEN) != 0;
    if (faction_alien_for_ai && partner_alien_for_ai) return;
    if ((PlayersData[faction_id].diplo_agenda[faction_id_with] &
         DAGENDA_DIPLO_PERMANENT) != 0) {
        enemies_trade_tech(faction_id, faction_id_with);
        return;
    }
    if (!wants_to_attack(faction_id, faction_id_with, 0) &&
        !wants_to_attack(faction_id_with, faction_id, 0)) {
        enemies_treaty(faction_id, faction_id_with, 1);
        return;
    }
    enemies_war(faction_id, faction_id_with);
}
