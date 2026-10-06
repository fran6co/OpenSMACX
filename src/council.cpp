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
#include <string.h>

int __cdecl wants_prop(int, int, int);
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
