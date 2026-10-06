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
#include "council.h"
#include "councwin.h"
#include "base.h"
#include "alpha.h"
#include "console.h"
#include "game.h"
#include "map.h"
#include "buffer.h"
#include "faction.h"
#include "general.h"
#include "netdaemon.h"
#include "netmsg.h"
#include "popup.h"
#include "sprite.h"
#include "strings.h"
#include "technology.h"
#include "veh.h"
#include <string.h>

int __cdecl diplo_lock(int);
void __cdecl diplo_unlock();

// council_action increments a proposal's resolution count and records the
// current turn before dispatching its effect. setup_game's native paired
// reset loop (005B3A7A..3A90) independently establishes eleven signed entries
// in each table; its owning implementation remains unfinished.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.


// make_resolution publishes per-faction notifications; both overloads of
// clear_council_notify clear these same eight signed cells per table.
// RECOVERY_OBJECT: v2 0x00703DE8 0x00703DEC ?CouncilPbemGate@@3HA 0x0052C670
// PBEM resolution stores the calling faction ID here, not a boolean.
int CouncilPbemGate;
// init raises this modal-council flag; key/button teardown clears it.
// RECOVERY_OBJECT: v2 0x00703DE4 0x00703DE8 ?CouncilDialogActive@@3HA 0x00427230
int CouncilDialogActive;
// RECOVERY_OBJECT: v2 0x00703DEC 0x00703E0C ?CouncilNotifyFormSlots@@3PAHA 0x004243F0
int CouncilNotifyFormSlots[8];
// RECOVERY_OBJECT: v2 0x00703E0C 0x00703E2C ?CouncilNotifyPluralSlots@@3PAHA 0x004243F0
int CouncilNotifyPluralSlots[8];

// call_council's original multiplayer deferral stores three message arguments
// here; net_upkeep 5294D3..529536 sends them and resets each to -1. Their
// initial image bytes are twelve FF bytes. The producer stores faction,
// proposal and selected faction in that order; keep the existing shared array.
// RECOVERY_OBJECT: v1 0x0068BF38 ?CouncilPendingMessageArgs@@3PAHA
int CouncilPendingMessageArgs[3] = {-1, -1, -1};

// Original .data text, as bounded by the pinned image's terminating NULs.
static const char CouncilSunspotPcx[] = "solar_sm.pcx";
static const char CouncilSunspotLabel[] = "SUNSPOTSNO2";
static const char CouncilPbemLabel[] = "COUNCILHOT";
static const char CouncilBlockedPcx[] = "council_sm.pcx";
static const char CouncilBlockedLabel[] = "COUNCILNEEDALL";

// ORIGINAL: 0x0052C880 ?call_council@@YAXH@Z 0x0052C880-0x0052DA98;0x0065DB70-0x0065DF45
// RECOVERY_BINDING: v1 {"reason":"Recover the complete faction-taking council dialog in its existing owner with genuine automatic Popup and Sprite lifetimes, typed game/faction storage and live post-modal reads.","evidence":["Pinned52C880 and its65DB70 lifetime tail establish the proposal selection, cooldown and network dispatch paths.","Existing Rules.min_turns_councils and Players/PlayersData members account for the observed storage; no executable-address aliases or manual vtable writes.","The original s14 local is initialized only on eligible proposal1 paths; alternate_selection retains that unresolved stack-residue obligation rather than inventing a default."]}
// TRIED: literal register/goto transcription of all ~85 branches and the 10-entry switch resolved from the image; Popup+embedded Scroll+ 2 embedded FlatButton locals (BasePop+0x3230=Scroll start, Scroll+0xAAC/0x15F8=flat_button_left_/right_, matching the SEH thunks at 0x65DB99/0x65DBA7); vtable-reassert writes before every close() and the per-site EH state bytes are compiler artifacts, not transcribed. #18 first divergence still open.
void __cdecl call_council(int faction_id) {
    // These automatic objects are significant even on all of the early
    // returns: Popup owns Dialogs and Scroll, and Sprite owns its initialized
    // image.  Let the compiler's ordinary destruction chain run.  In
    // particular, do not spell Popup::close/Scroll::close here: Popup's
    // destructor calls close(), and its Scroll member then has its own dtor.
    Popup popup;
    Sprite council_sprite;

    int faction_bit;
    int human_mask;
    int local_faction;
    int last_selected_faction;
    int last_proposal;
    int max_wants;
    int rendered_count;
    int offered_count;
    int proposal;
    int proposal_tech;
    int has_proposal_tech;
    int option;
    int target;
    int candidate;
    int candidate_votes;
    int larger_vote_counts;
    int current_votes;
    int total_votes;
    int pledged_votes;
    int turn;
    int council_delay;
    int last_resolution_turn;
    int picture_width;
    int picture_height;
    int alternate_selection;
    int sprite_initialized;
    int selected_item;
    int proposal_text_id;
    int proposal_description_id;
    int due_turn;

    if (ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_ALIEN)) {
        return;
    }
    if (faction_id == 0) {
        return;
    }

    if (ConsoleControlTurnActive != 0) {
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << faction_id)) != 0 || IsMultiplayerNet) {
            return;
        }
    }

    if (can_call_council(faction_id,
            (faction_id == ConsoleGlobal.faction_id_
             && (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << faction_id)) != 0) ? 1 : 0) == 0) {
        return;
    }

    faction_bit = 1 << faction_id;
    human_mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    if ((human_mask & faction_bit) == 0 && GameDataStateStorage.fields_.governor_faction_ > 0
            && ((GameDataStateStorage.fields_.turn_current_num_ + faction_id * 2) & 0xF) != 0) {
        return;
    }
    if ((human_mask & faction_bit) == 0 && (GameDataStateStorage.fields_.game_state_ & 1) != 0) {
        return;
    }
    if (IsMultiplayerNet && (GameDataStateStorage.fields_.net_turn_flags_ & 0x10) != 0
            && (human_mask & faction_bit) != 0 && GameDataStateStorage.fields_.net_turn_faction_ == faction_id) {
        NetMsgPrimary.pop("SIMULNO", 0x1388, 0, 0);
        return;
    }

    // If a human must convene the Council, no alien faction may still be
    // active.  The native path uses the saved low-byte mask for this walk.
    for (candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if ((human_mask & faction_bit) != 0
                || ((human_mask >> candidate) & 1) != 0) {
            if ((Players[candidate].rule_flags & RFLAG_ALIEN) != 0) {
                return;
            }
        }
    }

    offered_count = 0;
    last_proposal = -1;
    last_selected_faction = faction_id;
    max_wants = 0;
    rendered_count = 0;

    popup.start(PopupStartCaption, "COUNCILISSUES", -1, 0, 64, 0);
    if (Buffer::get_pcx_dimensions("council_sm.pcx", &picture_width,
                                   &picture_height) == 0) {
        sprite_initialized = council_sprite.init(
                                                  const_cast<char *>("council_sm.pcx"),
                                                  picture_width,
                                                  picture_height);
        // The original only uses this for initialization/lifetime side
        // effects.  Its pointer is never consumed by the dialog.
        (void)sprite_initialized;
    }

    proposal = 0;
proposal_loop:
    proposal_tech = Proposal[proposal].preq_tech;
    has_proposal_tech = has_tech(proposal_tech, faction_id);
    if (has_proposal_tech == 0) {
        if ((GameDataStateStorage.fields_.game_state_ & 0x80) == 0 || (GameDataStateStorage.fields_.game_state_ & 0x1000) == 0) {
            goto proposal_next;
        }
    }
    if (proposal_tech < -1) {
        goto proposal_next;
    }
    if (proposal > 9) {
        goto proposal_render;
    }

    switch (proposal) {
    case PROP_ELECT_PLANETARY_GOVERNOR:
        goto governor_proposal;
    case PROP_UNITE_SUPREME_LEADER:
        goto supreme_leader_proposal;
    case PROP_SALVAGE_UNITY_CORE:
    case PROP_LAUNCH_SOLAR_SHADE:
        goto one_resolution_proposal;
    case PROP_GLOBAL_TRADE_PACT:
        goto trade_pact_proposal;
    case PROP_REPEAL_GLOBAL_TRADE_PACT:
        goto repeal_trade_pact_proposal;
    case PROP_INCREASE_SOLAR_SHADE:
        goto increase_shade_proposal;
    case PROP_MELT_POLAR_CAPS:
        goto proposal_render;
    case PROP_REPEAL_UN_CHARTER:
        goto repeal_charter_proposal;
    case PROP_REINSTATE_UN_CHARTER:
        goto reinstate_charter_proposal;
    }
    goto proposal_render;

governor_proposal:
    if (ExpansionEnabled
            && (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
        goto governor_candidate_counted;
    }
    candidate = council_votes(faction_id);
    larger_vote_counts = 0;
    for (target = 1; target < MaxPlayerNum; ++target) {
        if (faction_id != target
                && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8)
                    & (1 << target)) != 0
                && council_votes(target) > candidate) {
            ++larger_vote_counts;
        }
    }
    if (larger_vote_counts >= 2) {
        goto governor_candidate_counted;
    }
    goto governor_vote_rule;

governor_candidate_counted:
    human_mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    if ((faction_bit & human_mask) == 0 && GameDataStateStorage.fields_.governor_faction_ > 0) {
        goto proposal_next;
    }
governor_vote_rule:
    local_faction = GameDataStateStorage.fields_.governor_faction_;
    human_mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    if (local_faction == faction_id
            && (human_mask & faction_bit) == 0) {
        goto proposal_next;
    }
    human_mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    if ((human_mask & faction_bit) != 0) {
        goto proposal_render;
    }
    if (local_faction <= 0) {
        goto governor_tally;
    }
    {
        const uint32_t relationship =
            PlayersData[faction_id].diplo_treaties[local_faction];
        if ((relationship & DTREATY_PACT) == 0) {
            goto governor_tally;
        }
        if (PlayersData[faction_id].ranking
                    < PlayersData[local_faction].ranking
                || (relationship & 0x02000000) != 0) {
            goto proposal_next;
        }
    }

governor_tally:
    total_votes = 0;
    pledged_votes = 0;
    for (candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        current_votes = council_votes(candidate);
        total_votes += current_votes;
        if (council_get_vote(candidate, PROP_ELECT_PLANETARY_GOVERNOR,
                             faction_id) == faction_id) {
            pledged_votes += council_votes(candidate);
        }
    }
    if (pledged_votes * 2 >= total_votes) {
        goto proposal_render;
    }
    local_faction = GameDataStateStorage.fields_.governor_faction_;
    if (local_faction <= 0) {
        goto proposal_render;
    }
    human_mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    if ((human_mask & (1 << local_faction)) != 0) {
        goto proposal_render;
    }
    goto proposal_next;

supreme_leader_proposal:
    if ((GameDataStateStorage.fields_.game_rules_ & 8) == 0 || (GameDataStateStorage.fields_.game_state_ & 1) != 0
            || GameDataStateStorage.fields_.council_resolution_count_[PROP_UNITE_SUPREME_LEADER] != 0) {
        goto proposal_next;
    }
    if (ExpansionEnabled
            && (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
        goto proposal_next;
    }
    candidate = council_votes(faction_id);
    larger_vote_counts = 0;
    for (target = 1; target < MaxPlayerNum; ++target) {
        if (faction_id != target
                && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8)
                    & (1 << target)) != 0
                && council_votes(target) > candidate) {
            ++larger_vote_counts;
        }
    }
    if (larger_vote_counts < 2) {
        goto proposal_next;
    }
    alternate_selection = 0;
    for (candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if ((Players[candidate].rule_flags & RFLAG_ALIEN) != 0
                && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8)
                    & (1 << candidate)) != 0) {
            alternate_selection = 1;
        }
    }
    goto proposal_render;

one_resolution_proposal:
    if (GameDataStateStorage.fields_.council_resolution_count_[proposal] != 0) {
        goto proposal_next;
    }
    goto proposal_render;

trade_pact_proposal:
    if (GameDataStateStorage.fields_.council_resolution_count_[3] > GameDataStateStorage.fields_.council_resolution_count_[4]) {
        goto proposal_next;
    }
    goto proposal_render;

repeal_trade_pact_proposal:
    if (GameDataStateStorage.fields_.council_resolution_count_[4] == GameDataStateStorage.fields_.council_resolution_count_[3]) {
        goto proposal_next;
    }
    goto proposal_render;

increase_shade_proposal:
    if (GameDataStateStorage.fields_.council_resolution_count_[5] == 0) {
        goto proposal_next;
    }
    goto proposal_render;

repeal_charter_proposal:
    if (GameDataStateStorage.fields_.council_resolution_count_[8] > GameDataStateStorage.fields_.council_resolution_count_[9]) {
        goto proposal_next;
    }
    goto proposal_render;

reinstate_charter_proposal:
    if (GameDataStateStorage.fields_.council_resolution_count_[9] >= GameDataStateStorage.fields_.council_resolution_count_[8]) {
        goto proposal_next;
    }
    goto proposal_render;

proposal_render:
    StringTemp[0] = 0;
    proposal_text_id = static_cast<int>(reinterpret_cast<intptr_t>(
        Proposal[proposal].name));
    strcat(StringTemp, StringTable.get(proposal_text_id));
    strcat(StringTemp, "|");
    proposal_description_id = static_cast<int>(reinterpret_cast<intptr_t>(
        Proposal[proposal].description));
    strcat(StringTemp, StringTable.get(proposal_description_id));
    popup.dialogs().item(StringTemp, rendered_count);
    ++rendered_count;

    // Source fields named unk_29/unk_30 are independently anchored at
    // PlayerData+0x400/+0x42C.  Their meanings remain provisional: this is the
    // original nonzero-entry/vote scan, expressed through those real arrays.
    human_mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    if ((human_mask & faction_bit) != 0) {
        goto proposal_next;
    }
    option = PlayersData[faction_id].unk_29[rendered_count - 1];
    if (option == 0) {
        goto no_vote_option;
    }
    if (rendered_count - 1 != 0) {
        goto proposal_next;
    }
    target = PlayersData[faction_id].unk_30[0];
    if (ExpansionEnabled
            && (Players[target].rule_flags & RFLAG_ALIEN) != 0) {
        goto no_vote_option;
    }
    candidate_votes = council_votes(target);
    larger_vote_counts = 0;
    for (candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if (target != candidate
                && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8)
                    & (1 << candidate)) != 0
                && council_votes(candidate) > candidate_votes) {
            ++larger_vote_counts;
        }
    }
    if (larger_vote_counts >= 2) {
        goto no_vote_option;
    }
    goto proposal_next;

no_vote_option:
    if (rendered_count != 1) {
        goto proposal_score;
    }
    candidate = 1;
    while (candidate != GameDataStateStorage.fields_.governor_faction_) {
        if (!(ExpansionEnabled
                && (Players[candidate].rule_flags & RFLAG_ALIEN) != 0)) {
            candidate_votes = council_votes(candidate);
            larger_vote_counts = 0;
            for (target = 1; target < MaxPlayerNum; ++target) {
                if (candidate != target
                        && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8)
                            & (1 << target)) != 0
                        && council_votes(target) > candidate_votes) {
                    ++larger_vote_counts;
                }
            }
            if (larger_vote_counts < 2) {
                option = wants_prop(faction_id, 0, candidate);
                if (option > max_wants) {
                    max_wants = option;
                    last_proposal = 0;
                    last_selected_faction = candidate;
                }
            }
        }
        ++candidate;
    }
    goto proposal_next;

proposal_score:
    // TODO: the seed's s14 stack slot is only initialized on the proposal-1
    // path but is later read for some recorded vote paths.  Do not silently
    // turn that image-dependent stack residue into a clean false default.
    if (alternate_selection != 0) {
        goto proposal_next;
    }
    option = wants_prop(faction_id, rendered_count - 1, faction_id);
    if (option <= max_wants) {
        goto proposal_next;
    }
    max_wants = option;
    last_proposal = rendered_count - 1;
    last_selected_faction = faction_id;
    goto proposal_next;

proposal_next:
    ++proposal;
    if (proposal < MaxProposalNum) {
        goto proposal_loop;
    }

    if (rendered_count == 0) {
        return;
    }
    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0) {
        selected_item = popup.exec(0, 0);
    } else {
        selected_item = last_proposal;
    }

    if (alternate_selection != 0 && selected_item == 1) {
        if (faction_id == ConsoleGlobal.faction_id_
                && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0) {
            NetMsgPrimary.pop("NOTWHILEALIENSLIVE", 0xFFFFEC78, 0,
                              "council_sm.pcx");
        }
        return;
    }
    if (selected_item < 0) {
        return;
    }

    turn = GameDataStateStorage.fields_.turn_current_num_;
    council_delay = static_cast<int>(Rules.min_turns_councils);
    if (selected_item != 0) {
        if ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0 && (GameDataStateStorage.fields_.game_state_ & 0x1000) != 0
                && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0) {
            goto proposal_resolved;
        }
        if (GameDataStateStorage.fields_.governor_faction_ == faction_id) {
            due_turn = council_delay / 2;
        } else {
            due_turn = council_delay;
        }
        if (turn - PlayersData[faction_id].council_call_turn >= due_turn) {
            goto proposal_resolved;
        }
        if (faction_id == ConsoleGlobal.faction_id_
                && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0) {
            parse_num(0, game_year(PlayersData[faction_id].council_call_turn));
            parse_num(1, council_delay);
            parse_num(2, council_delay / 2);
            NetMsgPrimary.pop("COUNCILRECENTCALL", 0xFFFFEC78, 0,
                              "council_sm.pcx");
        }
        return;
    }

proposal_resolved:
    last_resolution_turn = GameDataStateStorage.fields_.council_last_resolution_turn_[selected_item];
    if (turn - last_resolution_turn >= council_delay) {
        goto council_resolution;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0 && (GameDataStateStorage.fields_.game_state_ & 0x1000) != 0
            && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0) {
        goto council_resolution;
    }
    if (faction_id == ConsoleGlobal.faction_id_
            && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0) {
        parse_num(0, game_year(last_resolution_turn));
        parse_num(1, council_delay);
        NetMsgPrimary.pop("COUNCILRECENTPROP", 0xFFFFEC78, 0,
                          "council_sm.pcx");
    }
    return;

council_resolution:
    if (!IsMultiplayerNet) {
        council(faction_id, selected_item, last_selected_faction);
        return;
    }
    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) == 0) {
        CouncilPendingMessageArgs[0] = faction_id;
        CouncilPendingMessageArgs[1] = selected_item;
        CouncilPendingMessageArgs[2] = last_selected_faction;
        return;
    }

    NetMsgPrimary.pop("CONVENING", 0x0BB8, 0, 0);
    if (diplo_lock(0x32) == 0) {
        message_data(0x2600, 0, faction_id, selected_item,
                     last_selected_faction, 0);
        TurnRedrawPending = 1;
        while (ExitTurnLoop == 0) {
            wait_loop();
            if (TurnRedrawPending == 0) {
                break;
            }
        }
        diplo_unlock();
    } else {
        NetMsgPrimary.close();
        return;
    }
}

/*
Purpose: Say whether a faction may call the planetary council this turn:
         alien factions never, the scenario flag pair (GameState's two
         council bits) gates, an active sunspot blocks, the PBEM pair
         blocks, the currently-spying base's owner is always admitted, and
         otherwise some other live non-alien faction must share a
         treaty-or-vendetta status bit with the caller.

// ORIGINAL: 0x0052C670 ?can_call_council@@YAHHH@Z 0x0052C670-0x0052C87D
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x52C670..0x52C87D council caller and message paths", "source:src/council.h already declares can_call_council and src/council.cpp owns CouncilPbemGate", "0x009A64E9 is byte 1 of GameDataStateStorage.fields_.lock_masks_.enable_mask_; use its named alive_mask_ view", "0x00939284 is ConsoleGlobal.faction_id_ at member offset 0x23BD4", "pinned PE .data bounded literals including NUL: 0x68C020 solar_sm.pcx through 0x68C02D; 0x68C030 SUNSPOTSNO2 through 0x68C03B; 0x68C03C COUNCILHOT through 0x68C046; 0x68C048 council_sm.pcx through 0x68C056; 0x68C058 COUNCILNEEDALL through 0x68C066. No emitted C++ data symbol is asserted before root's fresh physical-symbol observation."], "reason": "council admission query in the existing council owner"}
// TRIED: MISMATCH #41 'mov' vs 'and', not chased further. The loop's triple early-exit (0x93f660==0 / two bit-mask misses) is a `goto blocked;` out of the for-loop rather than a `continue`, matching that all three targets jump clean out of the scan (not to its per-iteration increment).
// size      525 bytes
// prototype int (__cdecl ?can_call_council@@YAHHH@Z)(int32 factionID, int32 announce)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D9610
// notes     Seed provenance (TRIED): MISMATCH #41 'mov' vs 'and' was not
//           chased further; the scan's triple early-exit is a goto out of
//           the faction walk, whose exit test stays an address compare
//           against Players[8].rule_flags - not measured exact.
Return Value: 1 the council may be called, 0 not
Status: Coverage transcription - unclaimed body, seed
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0052c670.cpp retained.
*/
int __cdecl can_call_council(int faction_id, int announce) {
    if ((Players[faction_id].rule_flags & 0x80) != 0) {
        return 0;
    }

    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.net_turn_flags_) & 4) != 0) {
        const unsigned char state =
            static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
        if ((state & 0x40) == 0 || (state & 0x80) == 0) {
            return 0;
        }
    }

    if (GameDataStateStorage.fields_.sunspot_duration_ > 0) {
        if (announce != 0 && faction_id == ConsoleGlobal.faction_id_ &&
            ((1 << (faction_id & 0x1F)) &
             static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0) {
            NetMsgPrimary.pop(CouncilSunspotLabel, 0xffffec78, 0,
                             CouncilSunspotPcx);
        }
        return 0;
    }

    if (IsMultiplayerPBEM != 0 && CouncilPbemGate != 0) {
        if (announce != 0 && faction_id == ConsoleGlobal.faction_id_ &&
            ((1 << (faction_id & 0x1F)) &
             static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0) {
            NetMsgPrimary.pop(CouncilPbemLabel, 0xffffec78, 0,
                             static_cast<const char *>(0));
        }
        return 0;
    }

    const int spy_index = GameDataStateStorage.fields_.secret_project_.empath_guild();
    if (spy_index >= 0 &&
        Bases[spy_index].faction_id_current == faction_id) {
        return 1;
    }

    {
        const unsigned char *rec = reinterpret_cast<const unsigned char *>(
            &Players[1].rule_flags);
        int i = 1;
        for (;;) {
            if ((*rec & 0x80) == 0) {
                const unsigned int bit_i = 1u << (i & 0x1F);
                if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_) &
                     static_cast<unsigned char>(bit_i)) != 0 &&
                    i != faction_id) {
                    if ((PlayersData[faction_id].diplo_treaties[i] & 0x8000008) == 0) {
                        bool check_more = true;
                        if ((static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_) & 0x80) !=
                            0) {
                            const unsigned int bit_a1 = 1u << (faction_id & 0x1F);
                            if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                                 bit_a1) != 0) {
                                if (faction_id == ConsoleGlobal.faction_id_) {
                                    check_more = false;
                                }
                            }
                        }
                        if (check_more) {
                            if (IsMultiplayerNet == 0) {
                                goto blocked;
                            }
                            const unsigned int mask =
                                GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
                            const unsigned int bit_a1b =
                                1u << (faction_id & 0x1F);
                            if ((mask & bit_a1b) == 0) {
                                goto blocked;
                            }
                            if ((mask & bit_i) == 0) {
                                goto blocked;
                            }
                        }
                    }
                }
            }
            rec += 0x59C;
            ++i;
            if (rec >= reinterpret_cast<const unsigned char *>(
                           &Players[8].rule_flags)) {
                break;
            }
        }
        return 1;
    blocked:
        if (announce != 0 && faction_id == ConsoleGlobal.faction_id_ &&
            (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
             (1 << (faction_id & 0x1F))) != 0) {
            NetMsgPrimary.pop(CouncilBlockedLabel, 0xffffec78, 0,
                             CouncilBlockedPcx);
        }
        return 0;
    }
}

/*
Purpose: Score one council proposal for a faction: proposal 0 (propose
         anything) picks the option the faction wants most via wants_prop
         and keeps it unless the council's vote row is empty or the target
         faction's treaty word flags an obstruction; a numbered proposal
         adjusts wants_prop's base by the target faction's treaty bits and
         falls back to the seated vote row, a against/for verdict, or a
         deterministic turn-based abstention.

// ORIGINAL: 0x0052BE60 ?council_get_vote@@YAHHHH@Z 0x0052BE60-0x0052BF58
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x52BE60..0x52BF58 and callers from call_council/CouncWin/faction vote paths", "source:src/council.h already declares council_get_vote", "source:PlayerData exposes the three original vote/treaty rows; typed lookup already preserves the 0x833 index relation"], "reason": "council vote query in the existing council owner"}
// TRIED: `result < votes` for the loop-max compare (jle/jge polarity flip at #18) - flipped to `votes > result` to match, fixed. The surviving gap is a shared-epilogue layout choice, not semantics: the original falls through to `return a3` and jumps away to the shared tail for `return TableC[idx]`; every source order tried (plain if, inverted if, goto to a shared label) makes VC6 pick the opposite layout. Best reached is 96.3% mnemonic similarity (#32 je vs jne). 0x0096C9F8/0x0096CDE0/0x0096CE0C are indexed by `faction*2099+idx` (0x833 = 2099, matching spying_recovery.h's SpyingStatusTable stride at the same 0x0096C9F8 base) so they are `extern T name[]`, not the scaffold's folded `static T *const`, per the indexed-table-base lever.
// size      248 bytes
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052AEB0
// notes     Seed provenance (TRIED): `votes > result` (not `result < votes`)
//           for the loop-max compare, and the surviving gap is the shared
//           epilogue layout (je vs jne at #32, 96.3% similarity) - the
//           original falls through to `return a3` and jumps away for the
//           row read; every source order tried picks the opposite layout.
Return Value: the voting faction id (or -3 for none), -1 against, -2 for,
              or the seated vote row's faction
Status: Coverage transcription - unclaimed body; the seed
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0052be60.cpp is retained until this product
        definition measures fresh BYTE_EXACT.
*/
int __cdecl council_get_vote(int faction, int proposal, int target_faction) {
    int result;

    if (proposal == 0) {
        result = 0;
        target_faction = -3;
        int i = 1;
        do {
            int votes = wants_prop(faction, 0, i);
            if (votes > result) {
                result = votes;
                target_faction = i;
            }
            ++i;
        } while (i < 8);

        if (PlayersData[faction].unk_29[0] == 0) {
            return target_faction;
        }
        result = PlayersData[faction].unk_30[0];
        if ((PlayersData[faction].diplo_treaties[result] & 0x40c30) != 0) {
            return target_faction;
        }
    } else {
        result = wants_prop(faction, proposal, target_faction);
        unsigned int status = PlayersData[faction].diplo_treaties[target_faction];
        if ((status & 0x10) != 0) {
            --result;
        }
        if ((status & 0x20) != 0) {
            --result;
        }
        if ((status & 1) != 0) {
            ++result;
        }
        int direct = PlayersData[faction].unk_29[proposal];
        if (direct != 0) {
            return direct;
        }
        if (result > 0) {
            return -1;
        }
        if (result < 0) {
            return -2;
        }
        result = -1 - (GameDataStateStorage.fields_.turn_current_num_ + faction) % 3;
    }
    return result;
}

static int wants_prop_signed_difference(int left, uint32_t right) {
    const uint32_t bits = static_cast<uint32_t>(left) - right;
    int result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static int wants_prop_signed_sum(int left, uint32_t right) {
    const uint32_t bits = static_cast<uint32_t>(left) + right;
    int result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static int wants_prop_signed_product(int left, int right) {
    const uint32_t bits = static_cast<uint32_t>(left) *
                          static_cast<uint32_t>(right);
    int result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static int wants_prop_signed_add(int left, int right) {
    const uint32_t bits = static_cast<uint32_t>(left) +
                          static_cast<uint32_t>(right);
    int result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

// Score council proposal alternatives through the native ten-case dispatch.
// ORIGINAL: 0x0052AEB0 ?wants_prop@@YAHHHH@Z 0x0052AEB0-0x0052BE32
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0052aeb0.cpp original entry and complete span; pinned instruction review of all proposal blocks and common leader tail", "Native actor-row offset plus 96CA14 resolves to the low bytes of diplo_treaties[7..0], with an early break at the first unmarked rank; altitude threshold belongs to AltNatural[3]", "recovery/tests/test_wants_prop_behavior.py actual-source proposal controls; original seed preserved", "pinned original 0x52AEB0..0x52BE31 full dispatch and common leader tail; preserve case grouping and selector >10 behavior", "Pinned 0x52AEBC zeroes EDX; 0x52AEC1 stores it in the return local; unsigned 0x52AEC4 branches selectors >10 to 0x52BE28, whose common epilogue returns that local. Jump-table selector 10 alone targets 0x52BE21, stores -9, then joins the epilogue.", "At 0x52BA12 the climate loop cursor is 0x0097D042 (Bases+2, the y member); bitmask(26) supplies offset 3/mask 4, and 0x52BAAB reads cursor+0x8A+3 = record+0x8F, matching Base.facilities_built[3] at +0x8C plus index 3; current `(facilities_built[3] & 4)` is equivalent.", "source:src/council.cpp owns council vote, proposal, base, faction and map dependencies; councwin.h declares CouncilWindow", "the four static wants_prop_signed_* helpers move unchanged with their sole consumer", "retained TRIED text is historical and remains intact; fresh pinned clarification and original default/case-10 behavior are recorded here"], "reason": "source-owned council proposal evaluator with the original ten-case dispatch, shared scoring helpers and selector default"}
// TRIED: only case 0 (propose alliance) is transcribed with real fidelity, and only through the point the original reaches a shared continuation at 0x52B3C2 that this body does not chase further - the remaining ~2/3 of that case (great_satan rivalry, per-faction relationship-table gating via the `esi*65+esi`-stride tables seen in propose_treaty) is not modelled. Cases 1/2/3-4/5-7/8/9 are short plausible stand-ins using the same six callees (council_votes/wants_to_attack/great_satan/has_tech/bit_count/ bitmask) with invented scoring, not transcriptions - each real case is itself a dense multi-hundred-instruction block this pass did not read in full. Jump-table case grouping (3/4 and 5/6/7 share a target; case 10 and >10 both fall to the -9 default) is read from the image and correct. MISMATCH #2 (prologue, sub vs push - the SEH/frame shape here isn't modelled either).
int __cdecl wants_prop(int faction, int proposal, int option) {
    int score = 0;

    switch (proposal) {
    case 0: {
        // Native 0x52AED1..0x52AF5A rejects a dead option, applies the
        // expansion/alien filter, then rejects options below two or more
        // living factions by council votes. The live-byte mask is byte 1
        // of LockEnableMask, matching the native read at 0x009A64E9.
        const uint32_t living = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFFu;
        if ((living & (1u << option)) == 0) {
            return -9;
        }
        if (ExpansionEnabled != 0 &&
            (Players[option].rule_flags & RFLAG_ALIEN) != 0) {
            return -9;
        }

        const int option_votes = council_votes(option);
        int factions_above_option = 0;
        for (int other = 1; other < MaxPlayerNum; ++other) {
            if (other == option ||
                (((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFFu) & (1u << other)) == 0) {
                continue;
            }
            if (council_votes(other) > option_votes) {
                ++factions_above_option;
            }
        }
        if (factions_above_option >= 2) {
            return -9;
        }

        bool score_flag = false;
        bool bypass_governor_tail = false;
        int factions_above_actor = 0;
        for (int actor_peer = 1; actor_peer < MaxPlayerNum; ++actor_peer) {
            if (actor_peer != faction) {
                const int peer_votes = council_votes(actor_peer);
                const int actor_votes = council_votes(faction);
                if (actor_votes <= peer_votes) ++factions_above_actor;
            }
        }
        if (option == faction) {
            score = factions_above_actor > 1 ? 5 : 6;
            if (factions_above_actor == 0) {
                ++score;
            }
            if (GameDataStateStorage.fields_.governor_faction_ < 0) {
                score = score * 2 - 1;
            } else if (GameDataStateStorage.fields_.governor_faction_ == faction) {
                score += 2;
            } else if ((PlayersData[faction].diplo_treaties[GameDataStateStorage.fields_.governor_faction_] &
                        0x70u) != 0) {
                ++score;
            }
        } else {
            const uint32_t actor_to_option =
                PlayersData[faction].diplo_treaties[option];
            const bool direct_surrender =
                (actor_to_option & (DTREATY_PACT | DTREATY_HAVE_SURRENDERED)) ==
                (DTREATY_PACT | DTREATY_HAVE_SURRENDERED);
            if (direct_surrender) {
                score = 99;
                bypass_governor_tail = true;
            } else {
                for (int leader = 1; leader < MaxPlayerNum; ++leader) {
                    if (leader != faction &&
                        (PlayersData[faction].diplo_treaties[leader] &
                         (DTREATY_PACT | DTREATY_HAVE_SURRENDERED)) ==
                            (DTREATY_PACT | DTREATY_HAVE_SURRENDERED) &&
                        CouncilWindow.leader_label_kind_[leader] == option) {
                        score = 999;
                    }
                }
                if (GameDataStateStorage.fields_.governor_faction_ == faction) {
                    score -= 2;
                }
                if ((actor_to_option & DTREATY_VENDETTA) != 0) {
                    --score;
                }
                if ((actor_to_option & 0x60u) != 0) {
                    --score;
                }
                if ((actor_to_option & DTREATY_ATROCITY_VICTIM) != 0) {
                    --score;
                }

                if (score != 0) {
                    bypass_governor_tail = true;
                } else if ((actor_to_option & DTREATY_PACT) != 0) {
                    score = 1;
                    if (wants_to_attack(faction, option, 0) == 0 &&
                        wants_to_attack(option, faction, 0) == 0) {
                        score = 2;
                    }
                } else {
                    const int negated_option_fight = wants_prop_signed_difference(
                        0, static_cast<uint32_t>(PlayersData[option].ai_fight));
                    if (PlayersData[faction].ai_fight == negated_option_fight) {
                        --score;
                    }
                    if (Players[faction].soc_ideology_category >= 0) {
                        const int model = static_cast<int>(
                            PlayersData[option].soc_category_pending.models_[
                                Players[faction].soc_ideology_category]);
                        if (model != 0) {
                            if (model == Players[faction].soc_ideology_model) {
                                ++score;
                            } else {
                                --score;
                                score_flag = true;
                            }
                        }
                    }
                    if (Players[faction].soc_anti_ideology_category >= 0) {
                        const int model = static_cast<int>(
                            PlayersData[option].soc_category_pending.models_[
                                Players[faction].soc_anti_ideology_category]);
                        if (model != 0 &&
                            model == Players[faction].soc_anti_ideology_model) {
                            --score;
                            score_flag = true;
                        }
                    }
                    if (Players[faction].soc_ideology_category ==
                            Players[option].soc_anti_ideology_category &&
                        Players[faction].soc_ideology_model ==
                            Players[option].soc_anti_ideology_model) {
                        score -= 2;
                        score_flag = true;
                    }
                    if ((PlayersData[faction].diplo_treaties[option] & 2u) != 0 &&
                        wants_to_attack(faction, option, 0) == 0 &&
                        wants_to_attack(option, faction, 0) == 0) {
                        ++score;
                    }
                    if (great_satan(option, 0) != 0) {
                        --score;
                    } else {
                        const uint32_t humans = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
                        const uint32_t option_bit = 1u << option;
                        if ((humans & option_bit) != 0) {
                            if (IsMultiplayerNet != 0 &&
                                option == static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk1_) &&
                                static_cast<int>(PlayersData[option].ranking) >= 5) {
                                --score;
                            } else if (option != static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk1_) ||
                                       PlayersData[option].diff_level < 2) {
                                ++score;
                            }
                        } else {
                            int selected_rank = 7;
                            for (int rank = 7; rank >= 0; --rank) {
                                const int ranked_faction =
                                    static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[rank]);
                                const uint8_t relation_flags = static_cast<uint8_t>(
                                    PlayersData[faction].diplo_treaties[rank]);
                                if ((humans & (1u << ranked_faction)) == 0 &&
                                    (relation_flags & 0x70u) == 0) {
                                    break;
                                }
                                --selected_rank;
                            }
                            if (option == selected_rank) {
                                ++score;
                            }
                        }
                    }
                }
            }
        }

        // Native 0x52B2E4..0x52B3BF applies the elected-governor relation,
        // human/AI difficulty and treaty gates before the shared label tail.
        if (option != faction && !bypass_governor_tail && GameDataStateStorage.fields_.governor_faction_ >= 0 &&
            GameDataStateStorage.fields_.governor_faction_ != option && GameDataStateStorage.fields_.governor_faction_ != faction) {
            const uint32_t governor_treaty =
                PlayersData[faction].diplo_treaties[GameDataStateStorage.fields_.governor_faction_];
            const uint32_t actor_to_option = PlayersData[faction].diplo_treaties[option];
            if ((governor_treaty & DTREATY_PACT) != 0 &&
                (actor_to_option & DTREATY_PACT) == 0) {
                score -= 2;
            }
            if ((governor_treaty & 2u) != 0 &&
                (actor_to_option & 2u) == 0) {
                --score;
            }
            if ((governor_treaty & DTREATY_VENDETTA) != 0) {
                ++score;
            }
            if ((governor_treaty & 0x60u) != 0) {
                score += 2;
            }
        }
        if (option != faction && !bypass_governor_tail && score >= 0 &&
            factions_above_actor <= 1) {
            ++score;
        }
        if (option != faction && !bypass_governor_tail && score == 0 &&
            GameDataStateStorage.fields_.governor_faction_ == option) {
            score = 1;
        }
        if (option != faction && !bypass_governor_tail &&
            (PlayersData[faction].diplo_treaties[option] & DTREATY_PACT) == 0) {
            const uint32_t humans = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
            const uint32_t option_bit = 1u << option;
            if ((humans & option_bit) != 0 &&
                PlayersData[option].diff_level >= 3 && score > -2) {
                --score;
            }
            if ((humans & option_bit) != 0 && (GameDataStateStorage.fields_.game_rules_ & 0x40u) != 0 &&
                score > -2) {
                --score;
            }
        }

        // The common 0x52B3C2 tail first adjusts for a selected leader, then
        // handles the self-option floor and the flag set by ideology scoring.
        const int selected_leader = CouncilWindow.leader_label_kind_[option];
        if (selected_leader > 0 && selected_leader != option) {
            if (option == faction) {
                return score >= 1 ? score : 1;
            }
            --score;
            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << option)) == 0) {
                score /= 2;
            }
        }
        if (option == faction) {
            return score >= 1 ? score : 1;
        }
        if (score_flag &&
            (PlayersData[faction].diplo_treaties[option] & DTREATY_PACT) == 0 &&
            score > 0) {
            score = 0;
        }
        const uint32_t current_humans = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
        if ((current_humans & (1u << faction)) == 0 &&
            (current_humans & (1u << option)) == 0) {
            return score;
        }
        if ((PlayersData[faction].diplo_treaties[option] & 0x08000000u) != 0) {
            return score;
        }
        if (score > 0) {
            score = 0;
        }
        return score;
    }

    case PROP_SALVAGE_UNITY_CORE: {
        // Native 0x52B724 initializes the owner's score from the live player
        // record, then compares energy and maintenance/commercial positions
        // against every other numbered faction. The signed division at
        // 0x52B7D3..0x52B7D9 truncates toward zero.
        const PlayerData &owner = PlayersData[faction];
        if (owner.ai_power != 0) {
            ++score;
        }
        if (owner.ai_fight < 0) {
            --score;
        }
        if (owner.energy_reserves >= 500) {
            --score;
        }
        if (owner.energy_reserves < 100) {
            ++score;
        }
        if (owner.energy_reserves < 200) {
            ++score;
        }

        int relative_energy = 0;
        for (int other = 1; other < MaxPlayerNum; ++other) {
            if (other == faction) {
                continue;
            }
            const PlayerData &peer = PlayersData[other];
            if (peer.energy_reserves > owner.energy_reserves) {
                ++relative_energy;
            } else {
                --relative_energy;
            }

            const int peer_net = wants_prop_signed_difference(
                peer.unk_13, peer.maint_cost_total);
            const int owner_net = wants_prop_signed_difference(
                owner.unk_13, owner.maint_cost_total);
            if (peer_net > owner_net) {
                ++relative_energy;
            } else {
                --relative_energy;
            }
        }
        if (relative_energy <= 0) {
            return score;
        }
        score += relative_energy / 2;
        return score;
    }

    case PROP_UNITE_SUPREME_LEADER: {
        const uint32_t living = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFFu;
        if ((living & (1u << option)) == 0) {
            return -9;
        }
        if (ExpansionEnabled &&
            (Players[option].rule_flags & RFLAG_ALIEN) != 0) {
            return -9;
        }

        const int option_votes = council_votes(option);
        int higher_vote_factions = 0;
        for (int other = 1; other < MaxPlayerNum; ++other) {
            if (other == option ||
                (((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFFu) & (1u << other)) == 0) {
                continue;
            }
            if (council_votes(other) > option_votes) {
                ++higher_vote_factions;
            }
        }
        if (higher_vote_factions >= 2) {
            return -9;
        }

        if (option == faction) {
            return PlayersData[faction].ranking == 7 ? 5 : 0;
        }

        const uint32_t treaty = PlayersData[faction].diplo_treaties[option];
        if ((treaty & DTREATY_PACT) != 0) {
            if ((treaty & DTREATY_HAVE_SURRENDERED) != 0) {
                return 2;
            }
            if ((GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE) == 0) {
                return 0;
            }

            const PlayerData &actor = PlayersData[faction];
            const PlayerData &candidate = PlayersData[option];
            if (candidate.ranking > actor.ranking &&
                candidate.pop_total > actor.pop_total &&
                GameDataStateStorage.fields_.turn_current_num_ > 200) {
                return 1;
            }
            if (actor.ranking > candidate.ranking &&
                actor.pop_total > candidate.pop_total) {
                return -2;
            }
            return GameDataStateStorage.fields_.turn_current_num_ > 200 ? 0 : -1;
        }

        if ((treaty & DTREATY_ATROCITY_VICTIM) != 0) {
            return -5;
        }
        if ((treaty & (DTREATY_VENDETTA | DTREATY_WANT_REVENGE)) != 0) {
            return -4;
        }

        const PlayerData &actor = PlayersData[faction];
        const PlayerData &candidate = PlayersData[option];
        if (candidate.ranking != 7 || actor.ranking > 5 ||
            GameDataStateStorage.fields_.turn_current_num_ < 200 ||
            candidate.mil_strength_1 < wants_prop_signed_sum(
                actor.mil_strength_1,
                static_cast<uint32_t>(actor.mil_strength_1)) ||
            candidate.mil_strength_2 < wants_prop_signed_sum(
                actor.mil_strength_2,
                static_cast<uint32_t>(actor.mil_strength_2)) ||
            candidate.pop_total < wants_prop_signed_sum(
                actor.pop_total, static_cast<uint32_t>(actor.pop_total))) {
            return -4;
        }
        return -2;
    }

    case PROP_GLOBAL_TRADE_PACT:
    case PROP_REPEAL_GLOBAL_TRADE_PACT: {
        const PlayerData &owner = PlayersData[faction];
        for (int other = 1; other < MaxPlayerNum; ++other) {
            if (other == faction) {
                continue;
            }
            const PlayerData &peer = PlayersData[other];
            if (peer.tech_commerce_bonus > owner.tech_commerce_bonus) {
                --score;
            }
            score += bit_count(static_cast<int>(
                owner.diplo_treaties[other] & 3u));
        }

        if (owner.ai_wealth != 0) {
            ++score;
        }
        if (owner.ai_power != 0) {
            --score;
        }
        if (owner.ai_fight > 0) {
            --score;
        }
        if (owner.soc_effect_pending.values_[SocialEffect::ECONOMY] > 1) {
            ++score;
        }
        if (owner.soc_effect_pending.values_[SocialEffect::ECONOMY] < 0) {
            --score;
        }
        if (proposal == PROP_REPEAL_GLOBAL_TRADE_PACT) {
            score = -score;
        }
        return score;
    }

    case PROP_REPEAL_UN_CHARTER:
    case PROP_REINSTATE_UN_CHARTER: {
        const bool repeal = proposal == PROP_REPEAL_UN_CHARTER;
        const int chassis_technology = Chassis[8].preq_tech;
        const int weapon_technology = Weapon[16].preq_tech;
        const bool actor_has_chassis =
            has_tech(chassis_technology, faction) != 0;
        bool actor_has_weapon = false;
        if (actor_has_chassis) {
            actor_has_weapon = has_tech(weapon_technology, faction) != 0;
        }

        // A rival with either technology supplies the initial signed score;
        // it does not bypass the subsequent faction-policy adjustments.
        int charter_score = 0;
        if (!actor_has_chassis || !actor_has_weapon) {
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (has_tech(weapon_technology, other) ||
                    has_tech(chassis_technology, other)) {
                    charter_score = repeal ? -2 : 2;
                    break;
                }
            }
        }

        const PlayerData &owner = PlayersData[faction];
        if (owner.planet_busters == 0) {
            if ((repeal && charter_score < 0) ||
                (!repeal && charter_score > 0)) {
                charter_score += repeal ? -1 : 1;
            }
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (PlayersData[other].planet_busters != 0) {
                    charter_score += repeal ? -1 : 1;
                }
            }
        }

        if (repeal) {
            if (owner.ai_fight < 0) {
                --charter_score;
            }
            if (owner.ai_tech != 0 && owner.ai_growth != 0) {
                charter_score -= 2;
            }
            charter_score = wants_prop_signed_add(charter_score, owner.atrocities);
            if (charter_score < 0) {
                return charter_score;
            }
            if (owner.ai_fight > 0) {
                ++charter_score;
            }
            if (owner.ai_power == 0) {
                return charter_score;
            }
            return charter_score + 1;
        }

        if (owner.ai_fight < 0) {
            ++charter_score;
        }
        if (owner.ai_tech != 0 && owner.ai_growth != 0) {
            charter_score += 2;
        }
        charter_score = wants_prop_signed_difference(
            charter_score, static_cast<uint32_t>(owner.atrocities));
        if (charter_score > 0) {
            return charter_score;
        }
        if (owner.ai_fight > 0) {
            --charter_score;
        }
        if (owner.ai_power == 0) {
            return charter_score;
        }
        return charter_score - 1;
    }

    case PROP_LAUNCH_SOLAR_SHADE:
    case PROP_INCREASE_SOLAR_SHADE:
    case PROP_MELT_POLAR_CAPS: {
        const PlayerData &owner = PlayersData[faction];
        const int proposal_direction =
            proposal == PROP_MELT_POLAR_CAPS ? 1 : -1;
        int climate_score = 0;

        // Native 0x52B8FA..0x52B9F3 sets the initial proposal pressure from
        // current council sea-level direction, the faction's psi rule, and
        // the sign of its social-growth setting. These are ordered tests,
        // not a normalized policy score.
        if (owner.ai_growth != 0 || Players[faction].rule_psi != 0) {
            if (MapSeaLevelCouncil == 0 ||
                ((MapSeaLevelCouncil > 0) == (proposal_direction > 0))) {
                climate_score = -2;
                if (Players[faction].rule_psi != 0) {
                    climate_score = -4;
                }
            } else {
                climate_score = 2;
                if (Players[faction].rule_psi != 0) {
                    climate_score = 4;
                }
            }
        }

        if ((MapSeaLevelCouncil > 0) == (proposal_direction > 0) &&
            MapSeaLevelCouncil != 0) {
            climate_score -= 2;
        } else if (MapSeaLevelCouncil == 0) {
            // Native 0x52B99C goes straight to the decrement at 0x52B9EF.
            --climate_score;
        } else {
            if (MapSeaLevel >= 0) {
                ++climate_score;
            }
            if (owner.ai_power != 0) {
                climate_score += proposal_direction > 0 ? -1 : 1;
            }
            if (owner.ai_wealth != 0) {
                climate_score += proposal_direction > 0 ? 1 : -1;
            }
            if (owner.ai_tech != 0) {
                climate_score += proposal_direction > 0 ? 1 : -1;
            }
        }

        int base_weight_sum = 0;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            const Base &base = Bases[base_id];
            const Map &tile = map_tiles()[(base.x >> 1) +
                                          base.y * static_cast<int>(MapLongitude)];
            const int altitude = tile.climate & 0xE0;
            int terrain_weight = 0;
            const int depth = wants_prop_signed_difference(
                tile.contour, static_cast<uint32_t>(MapSeaLevel));

            if (altitude < 0x60) {
                if (proposal_direction < 0) {
                    const int cutoff = wants_prop_signed_difference(
                        static_cast<int>(AltNatural[3]),
                        WorldBuilder.sea_level_rises);
                    terrain_weight = depth < cutoff ? 1 : 2;
                } else {
                    terrain_weight = -1;
                }
            } else if (proposal_direction < 0) {
                if (altitude > 0x60) {
                    terrain_weight = -1;
                }
            } else if ((base.facilities_built[3] & 4) == 0 ||
                       altitude != 0x60) {
                const int cutoff = wants_prop_signed_sum(
                    static_cast<int>(AltNatural[3]),
                    WorldBuilder.sea_level_rises);
                if (depth <= cutoff) {
                    terrain_weight = -3;
                } else if (altitude == 0x60) {
                    terrain_weight = -1;
                } else {
                    terrain_weight = 1;
                }
            }

            int contribution = wants_prop_signed_product(
                static_cast<int>(base.population_size), terrain_weight);
            contribution = wants_prop_signed_product(contribution, 2);
            // Native loop cursor starts at Bases + 2 (the y member), so
            // byte [cursor + 2] is the base owner, not the map's val2.
            if (base.faction_id_current == faction) {
                base_weight_sum = wants_prop_signed_add(base_weight_sum,
                    wants_prop_signed_product(contribution, 2));
            } else {
                const int treaty_weight = bit_count(static_cast<int>(
                    owner.diplo_treaties[base.faction_id_current] & 0x00C0030u));
                const int penalty = wants_prop_signed_product(contribution,
                                                              treaty_weight);
                base_weight_sum = wants_prop_signed_difference(base_weight_sum,
                                                               static_cast<uint32_t>(penalty));
            }
        }

        int population_divisor = owner.pop_total;
        if (population_divisor < 1) {
            population_divisor = 1;
        } else if (population_divisor > 0x1869F) {
            population_divisor = 0x1869F;
        }
        climate_score = wants_prop_signed_add(
            climate_score, base_weight_sum / population_divisor);
        return climate_score;
    }

    case 10:
        // Jump-table entry 10 selects the explicit rejection block.
        return -9;
    default:
        // The unsigned selector guard returns the zero-initialized score.
        return 0;
    }
    return score;
}



/*
Purpose: Calculate faction's vote count. Used for Planetary Governor and Supreme Leader.
// ORIGINAL: 0x0052AD30 ?council_votes@@YAHH@Z 0x0052AD30-0x0052AE19 BYTE_EXACT
// LEVER: `Base *base = Bases; ...; base++` (walk, not index) for the
// population sum - `Bases[i]` plateaus. `votes` is a signed `int`, not
// `uint32_t` - the +50% arm needs the image's `cdq/sub/sar` signed-divide
// fixup, which an unsigned local turns into a plain `shr`. Each
// `has_project(...)` result is stored to a named `BOOL` before the `if`,
// matching the image's `sete`/`test`/`je` rather than a folded `cmp`/`jne`.
// The bonus loop walks `faction_bonus_val1` and reads
// `faction_bonus_id[i]` as `bonus_val1[-8]` (8 ints back, same registers
// the image keeps) rather than indexing both arrays separately. And that
// pointer is computed inside `if (bonus_count > 0) { ... }`, not
// unconditionally before the loop - without the guard VC6 schedules the
// `lea` ahead of the image's own `test`/`jle`.
// size      233 bytes
// prototype int (__cdecl ?council_votes@@YAHH@Z)(int factionID)
// callers   9   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Faction vote count
Status: Complete
*/
int __cdecl council_votes(int faction_id) {
    if (is_alien_faction(faction_id)) {
        return 0;
    }
    int votes = 0;
    Base *base = Bases;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++, base++) {
        if (base->faction_id_current == faction_id) {
            votes += base->population_size;
        }
    }
    BOOL has_empath_guild = has_project(SP_EMPATH_GUILD, faction_id);
    if (has_empath_guild) {
        votes += votes / 2; // +50% votes
    }
    BOOL has_clinical_immortality = has_project(SP_CLINICAL_IMMORTALITY, faction_id);
    if (has_clinical_immortality) {
        votes *= 2; // Doubles votes
    }
    int bonus_count = Players[faction_id].faction_bonus_count;
    if (bonus_count > 0) {
        int *bonus_val1 = Players[faction_id].faction_bonus_val1;
        for (i = 0; i < bonus_count; i++, bonus_val1++) {
            if (bonus_val1[-8] == RULE_VOTES) { // faction_bonus_id[i], 8 ints back
                int votes_bonus = *bonus_val1;
                if (votes_bonus >= 0) {
                    votes *= votes_bonus; // Peacekeeper bonus
                }
            }
        }
    }
    return votes;
}

/*
Purpose: Check whether a faction's leader is eligible to be a Planetary Governor candidate.
// ORIGINAL: 0x0052AE20 ?eligible@@YAHH@Z 0x0052AE20-0x0052AEA4 SEMANTIC
// LEVER: `faction_count` declared and zeroed BEFORE the `is_alien_faction`
// guard (the image's `mov [ebp-4],0` runs ahead of the alien test even
// though only the non-alien path reads it). `int`, not `uint32_t`, for
// `faction_count`/`votes`/the loop `i` - the image's `jle`/`jl`/`setl`
// tail is signed, an unsigned local turns it into `jbe`/`jb`/`sbb;neg`.
// TRIED: reordering is_alive's `FactionsStatus[1] & (1 << faction_id)` -
// no effect. Residual (53/54, SEMANTIC): `is_alive`'s inlined `test`
// encodes its two operands in the image's order (`test dl, al`) vs ours
// (`test al, dl`) - same flags, different byte, not chased further.
// size      132 bytes
// prototype 
// callers   6   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052AD30
Return Value: Is the leader eligible (top two vote totals)? true/false
Status: Complete
*/
BOOL __cdecl eligible(int faction_id) {
    int faction_count = 0;
    if (is_alien_faction(faction_id)) {
        return false;
    }
    int votes = council_votes(faction_id);
    for (int i = 1; i < MaxPlayerNum; i++) {
        if (faction_id != i && is_alive(i) && council_votes(i) > votes) {
            faction_count++;
        }
    }
    return faction_count < 2;
}

// The council proposal table, 0x009A6828 in the shipped image - MaxProposalNum
// entries of 0xc bytes. Was `RulesProposal *const Proposal =
// (RulesProposal *)0x009A6828` in council.h, naming terranx.exe data that is
// unmapped in a standalone build; see council.h. Zero at load there, as here:
// alpha.cpp's PROPOSALS text load fills it before anything reads it.
RulesProposal Proposal[MaxProposalNum];  // 0x009A6828, 0xc stride

// The dialog parser's negative gender/plurality arguments consume these
// defaults. Keep the leader and faction forms in the same order the native
// council announcement prepares them.
static void council_parse_leader(int faction_id) {
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(0, Players[faction_id].title_leader, -1, -1);

    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(1, Players[faction_id].name_leader, -1, -1);

    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(2, Players[faction_id].noun_faction, -1, -1);
}

/*
Purpose: Apply a selected council proposal to faction and world state.
The native dispatch's subtraction is only a dense jump-table bound check; the
proposal values themselves match CouncilProposals.
*/
// ORIGINAL: 0x0052BF60 ?council_action@@YAXHH@Z 0x0052BF60-0x0052C650
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0052bf60.cpp", "src/council.h", "src/game.cpp", "recovery/tests/test_council_action_behavior.py"], "reason": "Recovered complete proposal dispatch in its Council owner with genuine shared resolution arrays, Player and PlayerData members, Console lifetime references and shared climate state; preserves callback-sensitive faction reloads. Existing seed retains original extent/provenance."}
// TRIED: literal Ghidra transcription of the switch(a1){1,2,5,6,7} body, using the already-declared g_ADDR globals with pointer arithmetic scaled to match each DAT_ symbol's Ghidra-inferred width (uint*/byte* casts per site) compiles and preserves the case order and every call, but several popp() call sites show 4 explicit pushes in the raw disassembly against a 5-arg cdecl cleanup (`add esp,0x14`) - trusted Ghidra's resolved 5-arg call form over the raw push count, which is an unreconciled discrepancy. Diverges from the framed-prologue build around #2.
void __cdecl council_action(int proposal, int faction_id) {
    // Preserve the native proposal index before dispatch. Unsupported proposal
    // values still update their in-range resolution records, then fall through.
    GameDataStateStorage.fields_.council_resolution_count_[proposal]++;
    GameDataStateStorage.fields_.council_last_resolution_turn_[proposal] = GameDataStateStorage.fields_.turn_current_num_;

    switch (proposal) {
    case PROP_UNITE_SUPREME_LEADER: {
        GameDataStateStorage.fields_.game_state_ |= STATE_VICTORY_DIPLOMATIC;
        int local_faction = ConsoleGlobal.faction_id_;
        if (faction_id == local_faction) {
            if (!IsMultiplayerNet) {
                bool candidate_found = false;
                for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
                    if (candidate == faction_id || !is_alive(candidate)
                        || !(GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_CONQUEST)) {
                        continue;
                    }

                    const uint32_t candidate_to_target =
                        PlayersData[candidate].diplo_treaties[faction_id];
                    const uint32_t target_to_candidate =
                        PlayersData[faction_id].diplo_treaties[candidate];
                    const uint32_t candidate_agenda =
                        static_cast<uint32_t>(PlayersData[candidate].diplo_agenda[faction_id]);
                    if (!(candidate_to_target & 0x000C0000u)
                        && !(target_to_candidate & 0x00080000u)
                        && !(candidate_agenda & (0x00000800u | DAGENDA_FIGHT_TO_DEATH))) {
                        continue;
                    }

                    candidate_found = true;
                    council_parse_leader(candidate);
                    popp(ScriptPopupFile, "DEFIES", 0, "dipvic_sm.pcx", 0);
                    treaty_on(candidate, faction_id, DTREATY_VENDETTA);
                    agenda_on(candidate, faction_id, 0x2008);
                }

                if (candidate_found) {
                    for (int other = 1; other < MaxPlayerNum; ++other) {
                        if (other == faction_id || !is_alive(other)
                            || (PlayersData[other].diplo_agenda[faction_id]
                                & DAGENDA_FIGHT_TO_DEATH)) {
                            continue;
                        }
                        treaty_on(other, faction_id, 0x43);
                        treaty_off(other, faction_id, 0xC20);
                        agenda_off(other, faction_id, 0xC00);
                        set_treaty(other, faction_id, DTREATY_HAVE_SURRENDERED, TRUE);
                    }
                    return;
                }
            }

            EndgameExitReason = 6;
            PlayersData[faction_id].flags |= PFLAG_UNK_20000;
            GameDataStateStorage.fields_.game_state_ |= STATE_UNK_1;
            return;
        }

        council_parse_leader(faction_id);
        const bool cooperative_pact =
            (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE)
            && (PlayersData[local_faction].diplo_treaties[faction_id] & DTREATY_PACT);
        if (!cooperative_pact) {
            if (IsMultiplayerNet
                || !popp(ScriptPopupFile, "ACCEDE", 0, "dipvic_sm.pcx", 0)) {
                EndgameExitReason = 11;
                GameDataStateStorage.fields_.game_state_ |= STATE_UNK_1;
                PlayersData[faction_id].flags |= PFLAG_UNK_20000;
                return;
            }
        } else if (IsMultiplayerNet
                   || !popp(ScriptPopupFile, "ACCEDECOOP", 0, "dipvic_sm.pcx", 0)) {
            EndgameExitReason = 10;
            GameDataStateStorage.fields_.game_state_ |= STATE_UNK_1;
            PlayersData[faction_id].flags |= PFLAG_UNK_20000;
            return;
        }

        popp(ScriptPopupFile, "DEFY", 0, "council_sm.pcx", 0);
        local_faction = ConsoleGlobal.faction_id_;
        for (int other = 1; other < MaxPlayerNum; ++other) {
            if (other == local_faction || !is_alive(other)) {
                continue;
            }
            const uint32_t relation = PlayersData[other].diplo_treaties[local_faction];
            if ((relation & DTREATY_PACT) && (relation & DTREATY_HAVE_SURRENDERED)) {
                continue;
            }

            treaty_on(other, local_faction, 0x50);
            if (other != faction_id) {
                treaty_on(other, faction_id, DTREATY_PACT);
                agenda_on(other, faction_id, DAGENDA_DIPLO_PERMANENT);
            }
            set_agenda(other, ConsoleGlobal.faction_id_, DAGENDA_FIGHT_TO_DEATH, TRUE);
            local_faction = ConsoleGlobal.faction_id_;
        }
        return;
    }

    case PROP_SALVAGE_UNITY_CORE: {
        for (int player = 1; player < MaxPlayerNum; ++player) {
            if (!(Players[player].rule_flags & RFLAG_ALIEN)) {
                PlayersData[player].energy_reserves += 500;
            }
        }
        ConsoleGlobal.update_data(0);
        return;
    }

    case PROP_LAUNCH_SOLAR_SHADE:
    case PROP_INCREASE_SOLAR_SHADE:
        MapSeaLevelCouncil -= 3;
        GameDataStateStorage.fields_.climate_sea_level_delta_ -= static_cast<int>(WorldBuilder.sea_level_rises);
        GameDataStateStorage.fields_.climate_rise_step_ = GameDataStateStorage.fields_.climate_sea_level_delta_;
        if (GameDataStateStorage.fields_.climate_rise_step_ < 0) {
            GameDataStateStorage.fields_.climate_rise_step_ = -GameDataStateStorage.fields_.climate_rise_step_;
        }
        GameDataStateStorage.fields_.climate_rise_limit_ = 20;
        GameDataStateStorage.fields_.climate_rise_accum_ = 0;
        parse_num(0, 20);
        parse_num(1, (static_cast<int>(WorldBuilder.sea_level_rises) * 1000) / 15);
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) {
            popp(ScriptPopupFile, "WILLFALLALIEN", 0, "seafal_sm.pcx", 0);
        } else {
            popp(ScriptPopupFile, "WILLFALL", 0, "seafal_sm.pcx", 0);
        }
        return;

    case PROP_MELT_POLAR_CAPS:
        MapSeaLevelCouncil += 3;
        GameDataStateStorage.fields_.climate_sea_level_delta_ += static_cast<int>(WorldBuilder.sea_level_rises);
        GameDataStateStorage.fields_.climate_rise_step_ = GameDataStateStorage.fields_.climate_sea_level_delta_;
        if (GameDataStateStorage.fields_.climate_rise_step_ < 0) {
            GameDataStateStorage.fields_.climate_rise_step_ = -GameDataStateStorage.fields_.climate_rise_step_;
        }
        GameDataStateStorage.fields_.climate_rise_limit_ = 20;
        GameDataStateStorage.fields_.climate_rise_accum_ = 0;
        parse_num(0, 20);
        parse_num(1, (static_cast<int>(WorldBuilder.sea_level_rises) * 1000) / 15);
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) {
            popp(ScriptPopupFile, "WILLRISEALIEN", 0, "searis_sm.pcx", 0);
        } else {
            popp(ScriptPopupFile, "WILLRISE", 0, "searis_sm.pcx", 0);
        }
        return;
    }
}
