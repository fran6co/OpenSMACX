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
#include "time.h"
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
// Extent: compiled sizeof 0xc; int[3]; net_upkeep resets the three; later hits are not references.
// RECOVERY_OBJECT: v2 0x0068BF38 0x0068BF44 ?CouncilPendingMessageArgs@@3PAHA 0x005294D3
int CouncilPendingMessageArgs[3] = {-1, -1, -1};


/*
Purpose: Say whether a faction may call the planetary council this turn:
         alien factions never, the scenario flag pair (GameState's two
         council bits) gates, an active sunspot blocks, the PBEM pair
         blocks, the currently-spying base's owner is always admitted, and
         otherwise some other live non-alien faction must share a
         treaty-or-vendetta status bit with the caller.

// ORIGINAL: 0x0052C670 ?can_call_council@@YAHHH@Z 0x0052C670-0x0052C87D BYTE_EXACT
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
        git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0052c670.cpp retained.
*/
#pragma optimize("y", off)
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
            ((1 << faction_id) &
             GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) != 0) {
            NetMsgPrimary.pop("SUNSPOTSNO2", "solar_sm.pcx");
        }
        return 0;
    }

    if (IsMultiplayerPBEM != 0 && CouncilPbemGate != 0) {
        if (announce != 0 && faction_id == ConsoleGlobal.faction_id_ &&
            ((1 << faction_id) &
             GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) != 0) {
            NetMsgPrimary.pop("COUNCILHOT", static_cast<const char *>(0));
        }
        return 0;
    }

    const int spy_index = GameDataStateStorage.fields_.secret_project_.empath_guild();
    int has_empath = 0;
    if (spy_index >= 0) {
        has_empath = (Bases[spy_index].faction_id_current == faction_id);
    }
    if (has_empath) {
        return 1;
    }

    {
        for (int i = 1; i < 8; ++i) {
            if ((Players[i].rule_flags & 0x80) == 0) {
                if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &
                     (1 << i)) != 0 &&
                    i != faction_id) {
                    if ((PlayersData[faction_id].diplo_treaties[i] & 0x8000008) == 0) {
                        bool check_more = true;
                        if ((static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_) & 0x80) !=
                            0) {
                            const unsigned int bit_a1 = 1u << faction_id;
                            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &
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
                                1u << faction_id;
                            if ((mask & bit_a1b) == 0) {
                                goto blocked;
                            }
                            if ((mask & (1 << i)) == 0) {
                                goto blocked;
                            }
                        }
                    }
                }
            }
        }
        return 1;
    blocked:
        if (announce != 0 && faction_id == ConsoleGlobal.faction_id_ &&
            (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &
             (1 << faction_id)) != 0) {
            NetMsgPrimary.pop("COUNCILNEEDALL", "council_sm.pcx");
        }
        return 0;
    }
}
#pragma optimize("", on)

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
        git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0052be60.cpp is retained until this product
        definition measures fresh BYTE_EXACT.
*/
#pragma optimize("y", off)
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

        if (PlayersData[faction].unk_29[proposal] == 0 ||
            (PlayersData[faction].diplo_treaties[result = PlayersData[faction].unk_30[proposal]] & 0x40c30) != 0) {
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
#pragma optimize("", on)

// The leader proposals share this opening: a living, non-alien option that
// fewer than two living factions outvote. The image keeps the count in the
// caller's frame (a dead parameter slot) and materialises the final test.
static inline int wants_prop_candidate(int option, int &count) {
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << option))) {
        return 0;
    }
    count = 0;
    if (ExpansionEnabled && (Players[option].rule_flags & RFLAG_ALIEN)) {
        return 0;
    }
    const int votes = council_votes(option);
    for (int other = 1; other < MaxPlayerNum; ++other) {
        if (option != other
            && (GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << other))
            && council_votes(other) > votes) {
            count++;
        }
    }
    return count < 2;
}

// WORKAROUND(inline): has_fac's tail (is_built / queue_count test) with the
// bitmask call left at the site. Expanding has_fac (or any inline that
// contains the bitmask call) turns wants_prop's Bases loop cursor into a
// 0-based offset plus an absolute Bases displacement; the image walks a
// pointer at &Bases[0].y (0x52BA12 `mov [ebp+0x10], 0x97D042`). Writing the
// call at the site keeps the pointer cursor; this tail keeps the image's
// materialised, dead-stored flag ([ebp-0x1c], neg/sbb/neg).
inline BOOL wants_prop_built(int bits, int queue_count) {
    BOOL is_built = bits != 0;
    if (is_built || !queue_count) {
        return is_built;
    }
    return false;
}

// Score council proposal alternatives through the native ten-case dispatch.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0052AEB0 0x0052BE34 0x0052BE60 0x0052AECA
// ORIGINAL: 0x0052AEB0 ?wants_prop@@YAHHHH@Z 0x0052AEB0-0x0052BE32 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0052aeb0.cpp original entry and complete span; pinned instruction review of all proposal blocks and common leader tail", "Native actor-row offset plus 96CA14 resolves to the low bytes of diplo_treaties[7..0], with an early break at the first unmarked rank; altitude threshold belongs to AltNatural[3]", "recovery/tests/test_wants_prop_behavior.py actual-source proposal controls; original seed preserved", "pinned original 0x52AEB0..0x52BE31 full dispatch and common leader tail; preserve case grouping and selector >10 behavior", "Pinned 0x52AEBC zeroes EDX; 0x52AEC1 stores it in the return local; unsigned 0x52AEC4 branches selectors >10 to 0x52BE28, whose common epilogue returns that local. Jump-table selector 10 alone targets 0x52BE21, stores -9, then joins the epilogue.", "At 0x52BA12 the climate loop cursor is 0x0097D042 (Bases+2, the y member); bitmask(26) supplies offset 3/mask 4, and 0x52BAAB reads cursor+0x8A+3 = record+0x8F, matching Base.facilities_built[3] at +0x8C plus index 3; current `(facilities_built[3] & 4)` is equivalent.", "source:src/council.cpp owns council vote, proposal, base, faction and map dependencies; councwin.h declares CouncilWindow", "the four static wants_prop_signed_* helpers move unchanged with their sole consumer", "retained TRIED text is historical and remains intact; fresh pinned clarification and original default/case-10 behavior are recorded here"], "reason": "source-owned council proposal evaluator with the original ten-case dispatch, shared scoring helpers and selector default"}
// TRIED: only case 0 (propose alliance) is transcribed with real fidelity, and only through the point the original reaches a shared continuation at 0x52B3C2 that this body does not chase further - the remaining ~2/3 of that case (great_satan rivalry, per-faction relationship-table gating via the `esi*65+esi`-stride tables seen in propose_treaty) is not modelled. Cases 1/2/3-4/5-7/8/9 are short plausible stand-ins using the same six callees (council_votes/wants_to_attack/great_satan/has_tech/bit_count/ bitmask) with invented scoring, not transcriptions - each real case is itself a dense multi-hundred-instruction block this pass did not read in full. Jump-table case grouping (3/4 and 5/6/7 share a target; case 10 and >10 both fall to the -9 default) is read from the image and correct. MISMATCH #2 (prologue, sub vs push - the SEH/frame shape here isn't modelled either).
int __cdecl wants_prop(int faction, int proposal, int option) {
    int score = 0;

    switch (proposal) {
    case 0: {
        int i;
        // Kept for the later human-player tests.
        int option_bit = 1 << option;
        int count;
        if (!wants_prop_candidate(option, count)) {
            score = -9;
            break;
        }
        int flag = 0;  // [ebp-8]
        count = 0;
        for (i = 1; i < MaxPlayerNum; i++) {
            if (i != faction) {
                if (council_votes(i) >= council_votes(faction)) {
                    count++;
                }
            }
        }

        if (option == faction) {
            score = 5;
            if (count <= 1) {
                score = 6;
            }
            if (count == 0) {
                score++;
            }
            if (GameDataStateStorage.fields_.governor_faction_ < 0) {
                score = score * 2 - 1;
            }
            // Not an else-if: 0x52AFC9 compares governor == faction even after the <0 arm.
            if (GameDataStateStorage.fields_.governor_faction_ == faction) {
                score += 2;
            } else if (GameDataStateStorage.fields_.governor_faction_ >= 0 &&
                       (PlayersData[faction].diplo_treaties[
                            GameDataStateStorage.fields_.governor_faction_] & 0x70u) != 0) {
                score++;
            }
        } else {
            // [ebp+0x10] holds `diplo_treaties[option] & 1` until 0x52B0CF; it may be
            // a named local (`int pact`) or a VC6 CSE temp. Written as a local here.
            int pact = PlayersData[faction].diplo_treaties[option] & DTREATY_PACT;
            if (pact && (PlayersData[faction].diplo_treaties[option] & DTREATY_HAVE_SURRENDERED) != 0) {
                score = 99;
            } else {
                for (i = 1; i < MaxPlayerNum; i++) {
                    if (i != faction &&
                        (PlayersData[faction].diplo_treaties[i] & DTREATY_PACT) != 0 &&
                        (PlayersData[faction].diplo_treaties[i] & DTREATY_HAVE_SURRENDERED) != 0 &&
                        CouncilWindow.leader_label_kind_[i] == static_cast<uint32_t>(option)) {
                        score = 999;
                    }
                }
                if (GameDataStateStorage.fields_.governor_faction_ == faction) {
                    score -= 2;
                }
                if (PlayersData[faction].diplo_treaties[option] & DTREATY_VENDETTA) {
                    score--;
                }
                if (PlayersData[faction].diplo_treaties[option] & 0x60u) {
                    score--;
                }
                if (PlayersData[faction].diplo_treaties[option] & DTREATY_ATROCITY_VICTIM) {
                    score--;
                }
                if (score == 0) {
                    if (pact) {
                        score = 1;
                        if (!wants_to_attack(faction, option, 0) &&
                            !wants_to_attack(option, faction, 0)) {
                            score = 2;
                        }
                    } else {
                        // 0x52B13B stores the constant -1 (score is 0 here); `score--` would be `dec [mem]`.
                        if (PlayersData[faction].ai_fight == -PlayersData[option].ai_fight) {
                            score = -1;
                        }
                        if (Players[faction].soc_ideology_category >= 0) {
                            int model = static_cast<int>(PlayersData[option].soc_category_pending.models_[
                                Players[faction].soc_ideology_category]);
                            if (model != 0) {
                                if (model != Players[faction].soc_ideology_model) {
                                    score--;
                                    flag = 1;
                                } else {
                                    score++;
                                }
                            }
                        }
                        if (Players[faction].soc_anti_ideology_category >= 0) {
                            int model = static_cast<int>(PlayersData[option].soc_category_pending.models_[
                                Players[faction].soc_anti_ideology_category]);
                            if (model != 0 && model == Players[faction].soc_anti_ideology_model) {
                                score--;
                                flag = 1;
                            }
                        }
                        if (Players[faction].soc_ideology_category == Players[option].soc_anti_ideology_category &&
                            Players[faction].soc_ideology_model == Players[option].soc_anti_ideology_model) {
                            score -= 2;
                            flag = 2;
                        }
                        if ((PlayersData[faction].diplo_treaties[option] & 2u) != 0 &&
                            !wants_to_attack(faction, option, 0) &&
                            !wants_to_attack(option, faction, 0)) {
                            score++;
                        }
                        if (great_satan(option, 0)) {
                            score--;
                        } else {
                            int humans = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;  // [ebp-0x14]
                            if (humans & option_bit) {
                                if (IsMultiplayerNet != 0 &&
                                    option == static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk1_) &&
                                    PlayersData[option].ranking >= 5) {
                                    score--;
                                } else if (option != static_cast<int>(GameDataStateStorage.fields_.ranking_faction_id_unk1_) ||
                                           PlayersData[option].diff_level < 2) {
                                    score++;
                                }
                            } else {
                                // counter lives in [ebp+0x10]; ends at -1 when no rank breaks
                                for (i = 7; i >= 0; i--) {
                                    if ((humans & (1 << GameDataStateStorage.fields_.faction_rankings_[i])) == 0 &&
                                        (PlayersData[faction].diplo_treaties[i] & 0x70u) == 0) {
                                        break;
                                    }
                                }
                                if (option == i) {
                                    score++;
                                }
                            }
                        }
                    }

                    // 0x52B2E4: elected-governor relation adjustments.
                    if (GameDataStateStorage.fields_.governor_faction_ >= 0 &&
                        option != GameDataStateStorage.fields_.governor_faction_ &&
                        faction != GameDataStateStorage.fields_.governor_faction_) {
                        if ((PlayersData[faction].diplo_treaties[GameDataStateStorage.fields_.governor_faction_] & DTREATY_PACT) != 0 &&
                            (PlayersData[faction].diplo_treaties[option] & DTREATY_PACT) == 0) {
                            score -= 2;
                        }
                        if ((PlayersData[faction].diplo_treaties[GameDataStateStorage.fields_.governor_faction_] & 2u) != 0 &&
                            (PlayersData[faction].diplo_treaties[option] & 2u) == 0) {
                            score--;
                        }
                        if (PlayersData[faction].diplo_treaties[GameDataStateStorage.fields_.governor_faction_] & DTREATY_VENDETTA) {
                            score++;
                        }
                        if (PlayersData[faction].diplo_treaties[GameDataStateStorage.fields_.governor_faction_] & 0x60u) {
                            score += 2;
                        }
                    }
                    if (score >= 0) {
                        if (count <= 1) {
                            score++;
                        }
                        if (score == 0 && option == GameDataStateStorage.fields_.governor_faction_) {
                            score = 1;
                        }
                    }
                    if ((PlayersData[faction].diplo_treaties[option] & DTREATY_PACT) == 0) {
                        if (((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & option_bit)
                            && PlayersData[option].diff_level >= 3 && score > -2) {
                            score--;
                        }
                        if (((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & option_bit)
                            && (GameDataStateStorage.fields_.game_rules_ & 0x40) != 0 && score > -2) {
                            score--;
                        }
                    }
                }
            }
        }

        // 0x52B3C2 shared tail.
        // The `option != faction` term jumps straight to 0x52B492 (threaded into the
        // next test). If that does not reproduce, try the nested form
        // `if (option == faction) { if (score < 1) score = 1; break; }` inside.
        if (static_cast<int>(CouncilWindow.leader_label_kind_[option]) > 0 &&  // signed jle at 0x52B3CB
            CouncilWindow.leader_label_kind_[option] != static_cast<uint32_t>(option) &&
            option != faction) {
            score--;
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & option_bit) == 0) {
                score /= 2;
            }
        }
        if (faction != option) {
            if (flag && (PlayersData[faction].diplo_treaties[option] & DTREATY_PACT) == 0) {
                score = score > 0 ? 0 : score;  // setg/dec/and
            }
            if (!((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 1 << faction) && !((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & option_bit)) {
                break;
            }
            if (PlayersData[faction].diplo_treaties[option] & 0x08000000u) {
                break;
            }
            score = score > 0 ? 0 : score;  // setg/dec/and
        } else {
            if (score < 1) {
                score = 1;
            }
        }
        break;
    }

    case PROP_UNITE_SUPREME_LEADER: {
        int higher;
        if (!wants_prop_candidate(option, higher)) {
            score = -9;
            break;
        }
        if (option == faction) {
            score = PlayersData[faction].ranking == 7 ? 5 : 0;
            break;
        }
        const uint32_t treaty = PlayersData[faction].diplo_treaties[option];
        if ((treaty & DTREATY_PACT) && (treaty & DTREATY_HAVE_SURRENDERED)) {
            score = 2;
            break;
        }
        if (treaty & DTREATY_PACT) {
            if (!(GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE)) {
                break;
            }
            if (PlayersData[option].ranking > PlayersData[faction].ranking
                && PlayersData[option].pop_total > PlayersData[faction].pop_total
                && GameDataStateStorage.fields_.turn_current_num_ > 200) {
                score = 1;
                break;
            }
            if (PlayersData[faction].ranking > PlayersData[option].ranking
                && PlayersData[faction].pop_total > PlayersData[option].pop_total) {
                score = -2;
                break;
            }
            score = GameDataStateStorage.fields_.turn_current_num_ > 200 ? 0 : -1;
            break;
        }
        if (treaty & DTREATY_ATROCITY_VICTIM) {
            score = -5;
            break;
        }
        score = -4;
        if (treaty & (DTREATY_VENDETTA | DTREATY_WANT_REVENGE)) {
            break;
        }
        if (PlayersData[option].ranking != 7 || PlayersData[faction].ranking > 5
            || GameDataStateStorage.fields_.turn_current_num_ < 200
            || PlayersData[option].mil_strength_1 < PlayersData[faction].mil_strength_1 * 2
            || PlayersData[option].mil_strength_2 < PlayersData[faction].mil_strength_2 * 2
            || PlayersData[option].pop_total < PlayersData[faction].pop_total * 2) {
            break;
        }
        score = -2;
        break;
    }

    case PROP_SALVAGE_UNITY_CORE: {
        if (PlayersData[faction].ai_power) {
            score = 1;
        }
        if (PlayersData[faction].ai_fight < 0) {
            score--;
        }
        if (PlayersData[faction].energy_reserves >= 500) {
            score--;
        }
        if (PlayersData[faction].energy_reserves < 100) {
            score++;
        }
        if (PlayersData[faction].energy_reserves < 200) {
            score++;
        }
        int relative = 0;
        for (int other = 1; other < MaxPlayerNum; ++other) {
            if (other != faction) {
                if (PlayersData[other].energy_reserves > PlayersData[faction].energy_reserves) {
                    relative++;
                } else {
                    relative--;
                }
                if (PlayersData[other].unk_13 - PlayersData[other].maint_cost_total
                    > PlayersData[faction].unk_13 - PlayersData[faction].maint_cost_total) {
                    relative++;
                } else {
                    relative--;
                }
            }
        }
        if (relative > 0) {
            score += relative / 2;
        }
        break;
    }

    case PROP_GLOBAL_TRADE_PACT:
    case PROP_REPEAL_GLOBAL_TRADE_PACT: {
        int trade = 0;
        for (int other = 1; other < MaxPlayerNum; ++other) {
            if (other != faction) {
                if (PlayersData[other].tech_commerce_bonus > PlayersData[faction].tech_commerce_bonus) {
                    trade--;
                }
                trade += bit_count(PlayersData[faction].diplo_treaties[other] & 3);
            }
        }
        score = trade;
        if (PlayersData[faction].ai_wealth) {
            score++;
        }
        if (PlayersData[faction].ai_power) {
            score--;
        }
        if (PlayersData[faction].ai_fight > 0) {
            score--;
        }
        if (PlayersData[faction].soc_effect_pending.values_[SocialEffect::ECONOMY] > 1) {
            score++;
        }
        if (PlayersData[faction].soc_effect_pending.values_[SocialEffect::ECONOMY] < 0) {
            score--;
        }
        if (proposal == PROP_REPEAL_GLOBAL_TRADE_PACT) {
            score = -score;
        }
        break;
    }

    case PROP_LAUNCH_SOLAR_SHADE:
    case PROP_INCREASE_SOLAR_SHADE:
    case PROP_MELT_POLAR_CAPS: {
        // [ebp-0xc] (edi): `sub 7; neg; sbb; and -2; inc`
        int dir = (proposal == PROP_MELT_POLAR_CAPS) ? 1 : -1;
        // [ebp-0x10] is only the spilled CSE of faction * 0x20CC.
        if (PlayersData[faction].ai_growth || Players[faction].rule_psi) {
            // `jle; mov 1 | xor; setge; dec` = sign(council). First compare is
            // `cmp edi, ecx` (dir on the left), the second `cmp ecx, edi`.
            if (MapSeaLevelCouncil
                && dir != (MapSeaLevelCouncil > 0 ? 1 : (MapSeaLevelCouncil < 0 ? -1 : 0))) {
                score = 2;
                if (Players[faction].rule_psi) {
                    score = 4;
                }
            } else {
                score = -2;
                if (Players[faction].rule_psi) {
                    score = -4;
                }
            }
        }
        if ((MapSeaLevelCouncil > 0 ? 1 : (MapSeaLevelCouncil < 0 ? -1 : 0)) == dir) {
            score -= 2;
        } else if (!MapSeaLevelCouncil) {
            score--;
        } else {
            if (MapSeaLevel >= 0) {
                score++;
            }
            if (PlayersData[faction].ai_power) {
                if (dir > 0) {
                    score--;
                } else {
                    score++;
                }
            }
            if (PlayersData[faction].ai_wealth) {
                if (dir > 0) {
                    score++;
                } else {
                    score--;
                }
            }
            if (PlayersData[faction].ai_tech) {
                if (dir > 0) {
                    score++;
                } else {
                    score--;
                }
            }
        }
        // sum -> [ebp+0xc] (dead proposal slot), i -> [ebp-8],
        // biased cursor &Bases[i].y -> [ebp+0x10] (compiler strength reduction).
        int sum = 0;
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
            int weight = 0; // [ebp+8] (dead faction slot; faction lives in esi)
            // x (edi) / y (ebx) must be real locals: they survive the bitmask
            // call while MapTiles is reloaded. The image loads y before x.
            int x = Bases[i].x;
            int y = Bases[i].y;
            if ((map_loc(x, y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
                if (dir < 0) {
                    weight = 1;
                    // Signed `jl`: AltNatural and sea_level_rises are ints
                    // (map.h, alpha.h).
                    if (map_loc(x, y)->contour - MapSeaLevel
                        >= AltNatural[ALT_SHORE_LINE] - WorldBuilder.sea_level_rises) {
                        weight = 2;
                    }
                } else {
                    weight = -1;
                }
            } else if (dir > 0) {
                // has_fac(FAC_PRESSURE_DOME, i, 0) written out (see
                // wants_prop_built): [ebp-0x18] offset, [ebp-0x14] mask,
                // [ebp-0x1c] materialized is_built (neg/sbb/neg).
                int offset;
                int mask;
                bitmask(FAC_PRESSURE_DOME, &offset, &mask);
                if (!wants_prop_built(Bases[i].facilities_built[offset] & mask, 0)
                    || (map_loc(x, y)->climate & 0xE0) != ALT_BIT_SHORE_LINE) {
                    // Image loads AltNatural[3] (edi) first, then rises (ebx),
                    // `add ebx, edi` -> rises + AltNatural[3] (uncertain order).
                    // Signed `jg`, see above.
                    if (map_loc(x, y)->contour - MapSeaLevel
                        <= WorldBuilder.sea_level_rises + AltNatural[ALT_SHORE_LINE]) {
                        weight = -3;
                    } else if ((map_loc(x, y)->climate & ~0x1F) == ALT_BIT_SHORE_LINE) {
                        weight = -1;
                    } else if ((map_loc(x, y)->climate & ~0x1F) > ALT_BIT_SHORE_LINE) {
                        weight = 1;
                    }
                }
            } else if ((map_loc(x, y)->climate & ~0x1F) > ALT_BIT_SHORE_LINE) {
                weight = -1;
            }
            // `imul edi, [weight]; shl edi, 1` -- `<< 1` per the shl rule
            // (uncertain: could be `* 2`).
            int value = Bases[i].population_size * weight << 1;
            if (Bases[i].faction_id_current == faction) {
                sum += value * 2;
            } else {
                sum -= bit_count(PlayersData[faction].diplo_treaties[
                    Bases[i].faction_id_current] & 0xC0030) * value;
            }
        }
        // range() inline (general.h): <1 -> 1, >99999 -> 99999; the three
        // idiv/return tails are the tail-duplicated `score += ...; break;`.
        score += sum / range(PlayersData[faction].pop_total, 1, 99999);
        break;
    }

    case PROP_REPEAL_UN_CHARTER:
        if (!has_tech(Chassis[8].preq_tech, faction) || !has_tech(Weapon[16].preq_tech, faction)) {
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (has_tech(Weapon[16].preq_tech, other) || has_tech(Chassis[8].preq_tech, other)) {
                    score = -2;
                    break;
                }
            }
        }
        if (!PlayersData[faction].planet_busters) {
            if (score < 0) {
                score--;
            }
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (PlayersData[other].planet_busters) {
                    score--;
                }
            }
        }
        if (PlayersData[faction].ai_fight < 0) {
            score--;
        }
        if (PlayersData[faction].ai_tech && PlayersData[faction].ai_growth) {
            score -= 2;
        }
        score += PlayersData[faction].atrocities;
        if (score < 0) {
            break;
        }
        if (PlayersData[faction].ai_fight > 0) {
            score++;
        }
        if (!PlayersData[faction].ai_power) {
            break;
        }
        score++;
        break;

    case PROP_REINSTATE_UN_CHARTER:
        if (!has_tech(Chassis[8].preq_tech, faction) || !has_tech(Weapon[16].preq_tech, faction)) {
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (has_tech(Weapon[16].preq_tech, other) || has_tech(Chassis[8].preq_tech, other)) {
                    score = 2;
                    break;
                }
            }
        }
        if (!PlayersData[faction].planet_busters) {
            if (score > 0) {
                score++;
            }
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (PlayersData[other].planet_busters) {
                    score++;
                }
            }
        }
        if (PlayersData[faction].ai_fight < 0) {
            score++;
        }
        if (PlayersData[faction].ai_tech && PlayersData[faction].ai_growth) {
            score += 2;
        }
        score -= PlayersData[faction].atrocities;
        if (score > 0) {
            break;
        }
        if (PlayersData[faction].ai_fight > 0) {
            score--;
        }
        if (!PlayersData[faction].ai_power) {
            break;
        }
        score--;
        break;

    case 10:
        score = -9;
        break;
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
// ORIGINAL: 0x0052AE20 ?eligible@@YAHH@Z 0x0052AE20-0x0052AEA4 BYTE_EXACT
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
// RECOVERY_OBJECT: v2 0x009A6828 0x009A68AC ?Proposal@@3PAURulesProposal@@A 0x005873C0
RulesProposal Proposal[MaxProposalNum];  // 0x009A6828, 0xc stride

// The dialog parser's negative gender/plurality arguments consume these
// defaults. Keep the leader and faction forms in the same order the native
// council announcement prepares them.
static void council_parse_leader(int faction_id) {
    parse_set(Players[faction_id].is_leader_female, FALSE);
    parse_says(0, Players[faction_id].title_leader, -1, -1);

    parse_set(Players[faction_id].is_leader_female, FALSE);
    parse_says(1, Players[faction_id].name_leader, -1, -1);

    parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
    parse_says(2, Players[faction_id].noun_faction, -1, -1);
}

/*
Purpose: Apply a selected council proposal to faction and world state.
The native dispatch's subtraction is only a dense jump-table bound check; the
proposal values themselves match CouncilProposals.
*/
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0052BF60 0x0052C650 0x0052C66C 0x0052BF90
// ORIGINAL: 0x0052BF60 ?council_action@@YAXHH@Z 0x0052BF60-0x0052C650 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0052bf60.cpp", "src/council.h", "src/game.cpp", "recovery/tests/test_council_action_behavior.py"], "reason": "Recovered complete proposal dispatch in its Council owner with genuine shared resolution arrays, Player and PlayerData members, Console lifetime references and shared climate state; preserves callback-sensitive faction reloads. Existing seed retains original extent/provenance."}
// TRIED: literal Ghidra transcription of the switch(a1){1,2,5,6,7} body, using the already-declared g_ADDR globals with pointer arithmetic scaled to match each DAT_ symbol's Ghidra-inferred width (uint*/byte* casts per site) compiles and preserves the case order and every call, but several popp() call sites show 4 explicit pushes in the raw disassembly against a 5-arg cdecl cleanup (`add esp,0x14`) - trusted Ghidra's resolved 5-arg call form over the raw push count, which is an unreconciled discrepancy. Diverges from the framed-prologue build around #2.
// RECOVERY_LITERAL: v1 0x0068BF44 0x0068BF52 $SG57882 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BF54 0x0068BF62 $SG57883 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BF64 0x0068BF72 $SG57884 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BF74 0x0068BF7D $SG57885 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BF80 0x0068BF8E $SG57892 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BF90 0x0068BF9E $SG57893 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BFA0 0x0068BFAE $SG57894 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BFB0 0x0068BFB9 $SG57895 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BFBC 0x0068BFCA $SG57907 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BFCC 0x0068BFD3 $SG57908 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068C008 0x0068C017 $SG57928 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068C018 0x0068C01D $SG57929 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BFD4 0x0068BFE2 $SG57919 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BFE4 0x0068BFEF $SG57920 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068BFF0 0x0068BFFE $SG57924 0x0052BF60
// RECOVERY_LITERAL: v1 0x0068C000 0x0068C007 $SG57925 0x0052BF60
void __cdecl council_action(int proposal, int faction_id) {
    // Preserve the native proposal index before dispatch. Unsupported proposal
    // values still update their in-range resolution records, then fall through.
    GameDataStateStorage.fields_.council_resolution_count_[proposal]++;
    GameDataStateStorage.fields_.council_last_resolution_turn_[proposal] = GameDataStateStorage.fields_.turn_current_num_;

    switch (proposal) {
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
        {
            int step = GameDataStateStorage.fields_.climate_sea_level_delta_ -=
                static_cast<int>(WorldBuilder.sea_level_rises);
            if (step < 0) {
                step = -step;
            }
            GameDataStateStorage.fields_.climate_rise_step_ = step;
        }
        GameDataStateStorage.fields_.climate_rise_limit_ = 20;
        GameDataStateStorage.fields_.climate_rise_accum_ = 0;
        parse_num(0, 20);
        parse_num(1, (static_cast<int>(WorldBuilder.sea_level_rises) * 1000) / 15);
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) {
            popp(ScriptPopupFile, "WILLFALLALIEN", 0, "seafal_sm.pcx", 0);
            return;
        }
        popp(ScriptPopupFile, "WILLFALL", 0, "seafal_sm.pcx", 0);
        return;

    case PROP_MELT_POLAR_CAPS:
        MapSeaLevelCouncil += 3;
        {
            int step = GameDataStateStorage.fields_.climate_sea_level_delta_ +=
                static_cast<int>(WorldBuilder.sea_level_rises);
            if (step < 0) {
                step = -step;
            }
            GameDataStateStorage.fields_.climate_rise_step_ = step;
        }
        GameDataStateStorage.fields_.climate_rise_limit_ = 20;
        GameDataStateStorage.fields_.climate_rise_accum_ = 0;
        parse_num(0, 20);
        parse_num(1, (static_cast<int>(WorldBuilder.sea_level_rises) * 1000) / 15);
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) {
            popp(ScriptPopupFile, "WILLRISEALIEN", 0, "searis_sm.pcx", 0);
            return;
        }
        popp(ScriptPopupFile, "WILLRISE", 0, "searis_sm.pcx", 0);
        return;

    case PROP_UNITE_SUPREME_LEADER: {
        GameDataStateStorage.fields_.game_state_ |= STATE_VICTORY_DIPLOMATIC;
        int candidate_found = 0;
        if (faction_id == ConsoleGlobal.faction_id_) {
            if (!IsMultiplayerNet) {
                for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
                    if (candidate == faction_id || !is_alive(candidate)
                        || !(GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_CONQUEST)) {
                        continue;
                    }
                    if (!(PlayersData[candidate].diplo_treaties[faction_id] & 0x000C0000u)
                        && !(PlayersData[faction_id].diplo_treaties[candidate] & 0x00080000u)
                        && !(PlayersData[candidate].diplo_agenda[faction_id] & 0x00000800u)
                        && !(PlayersData[candidate].diplo_agenda[faction_id] & DAGENDA_FIGHT_TO_DEATH)) {
                        continue;
                    }

                    candidate_found = 1;
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
        } else {
            if ((GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE)
                && (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction_id] & DTREATY_PACT)) {
                council_parse_leader(faction_id);
                if (!IsMultiplayerNet
                    && popp(ScriptPopupFile, "ACCEDECOOP", 0, "dipvic_sm.pcx", 0)) {
                    candidate_found = 1;
                } else {
                    GameDataStateStorage.fields_.game_state_ |= STATE_UNK_1;
                    EndgameExitReason = 10;
                    PlayersData[faction_id].flags |= PFLAG_UNK_20000;

                }
            } else {
                council_parse_leader(faction_id);
                if (!IsMultiplayerNet
                    && popp(ScriptPopupFile, "ACCEDE", 0, "dipvic_sm.pcx", 0)) {
                    candidate_found = 1;
                } else {
                    GameDataStateStorage.fields_.game_state_ |= STATE_UNK_1;
                    EndgameExitReason = 11;
                    PlayersData[faction_id].flags |= PFLAG_UNK_20000;

                }
            }
        }
        if (!candidate_found) {
            return;
        }
        popp(ScriptPopupFile, "DEFY", 0, "council_sm.pcx", 0);
        {

            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (other == ConsoleGlobal.faction_id_ || !is_alive(other)) {
                    continue;
                }
                int player = ConsoleGlobal.faction_id_;
                if ((PlayersData[other].diplo_treaties[player] & DTREATY_PACT)
                    && (PlayersData[other].diplo_treaties[player] & DTREATY_HAVE_SURRENDERED)) {
                    continue;
                }

                treaty_on(other, ConsoleGlobal.faction_id_, 0x50);
                if (other != faction_id) {
                    treaty_on(other, faction_id, DTREATY_PACT);
                    agenda_on(other, faction_id, DAGENDA_DIPLO_PERMANENT);
                }
                set_agenda(other, ConsoleGlobal.faction_id_, DAGENDA_FIGHT_TO_DEATH, TRUE);

            }
        }
        return;
    }
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0052C880 0x0052DA98 0x0052DAC0 0x0052CE00
// ORIGINAL: 0x0052C880 ?call_council@@YAXH@Z 0x0052C880-0x0052DA98;0x0065DB70-0x0065DF45
// RECOVERY_BINDING: v1 {"reason":"Recover the complete faction-taking council dialog in its existing owner with genuine automatic Popup and Sprite lifetimes, typed game/faction storage and live post-modal reads.","evidence":["Pinned52C880 and its65DB70 lifetime tail establish the proposal selection, cooldown and network dispatch paths.","Existing Rules.min_turns_councils and Players/PlayersData members account for the observed storage; no executable-address aliases or manual vtable writes.","The original s14 local is initialized only on eligible proposal1 paths; alternate_selection retains that unresolved stack-residue obligation rather than inventing a default."]}
// TRIED: literal register/goto transcription of all ~85 branches and the 10-entry switch resolved from the image; Popup+embedded Scroll+ 2 embedded FlatButton locals (BasePop+0x3230=Scroll start, Scroll+0xAAC/0x15F8=flat_button_left_/right_, matching the SEH thunks at 0x65DB99/0x65DBA7); vtable-reassert writes before every close() and the per-site EH state bytes are compiler artifacts, not transcribed. #18 first divergence still open.
void __cdecl call_council(int faction_id) {
    Popup popup;

    if (is_alien_faction(faction_id)) {
        return;
    }
    if (!faction_id) {
        return;
    }
    if (ConsoleControlTurnActive && ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id)) || IsMultiplayerNet)) {
        return;
    }
    if (!can_call_council(faction_id,
            (faction_id == ConsoleGlobal.faction_id_ && is_human(faction_id)) ? 1 : 0)) {
        return;
    }

    int faction_bit = 1 << faction_id;
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
            && GameDataStateStorage.fields_.governor_faction_ > 0
            && ((GameDataStateStorage.fields_.turn_current_num_ + faction_id * 2) & 0xF)) {
        return;
    }
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
            && (GameDataStateStorage.fields_.game_state_ & 1)) {
        return;
    }
    if (IsMultiplayerNet && (GameDataStateStorage.fields_.net_turn_flags_ & 0x10)
            && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
            && GameDataStateStorage.fields_.net_turn_faction_ != faction_id) {
        NetMsgPrimary.pop("SIMULNO", 0x1388, 0, 0);
        return;
    }
    // An AI faction may not convene the Council while a human alien
    // faction is in the game.
    int i;
    for (i = 1; i < MaxPlayerNum; i++) {
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
                && is_human(i) && (Players[i].rule_flags & RFLAG_ALIEN)) {
            return;
        }
    }

    int max_wants = 0;
    int selected_proposal = -1;
    int selected_faction = faction_id;
    int rendered_count = 0;
    int alien_alive;
    popup.start(PopupStartCaption, "COUNCILISSUES", -1, 0, 64, 0);
    int picture_width;
    int picture_height;
    if (!Buffer::get_pcx_dimensions("council_sm.pcx", &picture_width,
                                    &picture_height)
            && !popup.owned_picture().init(const_cast<char *>("council_sm.pcx"),
                                           picture_width, picture_height)) {
        popup.select_owned_picture();
    }

    for (int proposal = 0; proposal < MaxProposalNum; proposal++) {
        if (!has_tech(Proposal[proposal].preq_tech, faction_id)
                && (!(GameDataStateStorage.fields_.game_state_ & 0x80)
                    || !(GameDataStateStorage.fields_.game_state_ & 0x1000))) {
            continue;
        }
        if (Proposal[proposal].preq_tech < -1) {
            continue;
        }
        switch (proposal) {
        case PROP_ELECT_PLANETARY_GOVERNOR: {
            if (!eligible(faction_id)
                    && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
                    && GameDataStateStorage.fields_.governor_faction_ > 0) {
                continue;
            }
            int governor = GameDataStateStorage.fields_.governor_faction_;
            if (governor == faction_id
                    && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)) {
                continue;
            }
            if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)) {
                if (governor > 0) {
                    uint32_t treaty = PlayersData[faction_id].diplo_treaties[governor];
                    if (treaty & DTREATY_PACT) {
                        if (PlayersData[faction_id].ranking < PlayersData[governor].ranking
                                || (treaty & 0x02000000)) {
                            continue;
                        }
                    }
                }
                int total_votes = 0;
                int pledged_votes = 0;
                for (i = 1; i < MaxPlayerNum; i++) {
                    total_votes += council_votes(i);
                    if (council_get_vote(i, PROP_ELECT_PLANETARY_GOVERNOR,
                                         selected_faction) == faction_id) {
                        pledged_votes += council_votes(i);
                    }
                }
                if (pledged_votes * 2 < total_votes) {
                    governor = GameDataStateStorage.fields_.governor_faction_;
                    if (governor > 0 && !is_human(governor)) {
                        continue;
                    }
                }
            }
            break;
        }
        case PROP_SALVAGE_UNITY_CORE:
        case PROP_LAUNCH_SOLAR_SHADE:
            if (GameDataStateStorage.fields_.council_resolution_count_[proposal]) {
                continue;
            }
            break;
        case PROP_UNITE_SUPREME_LEADER:
            if (!(GameDataStateStorage.fields_.game_rules_ & 8)
                    || (GameDataStateStorage.fields_.game_state_ & 1)
                    || GameDataStateStorage.fields_.council_resolution_count_[PROP_UNITE_SUPREME_LEADER]) {
                continue;
            }
            if (!eligible(faction_id)) {
                continue;
            }
            alien_alive = 0;
            for (i = 1; i < MaxPlayerNum; i++) {
                if ((Players[i].rule_flags & RFLAG_ALIEN) && is_alive(i)) {
                    alien_alive = 1;
                }
            }
            break;
        case PROP_REPEAL_UN_CHARTER:
            if (GameDataStateStorage.fields_.council_resolution_count_[8]
                    > GameDataStateStorage.fields_.council_resolution_count_[9]) {
                continue;
            }
            break;
        case PROP_REINSTATE_UN_CHARTER:
            if (GameDataStateStorage.fields_.council_resolution_count_[9]
                    >= GameDataStateStorage.fields_.council_resolution_count_[8]) {
                continue;
            }
            break;
        case PROP_GLOBAL_TRADE_PACT:
            if (GameDataStateStorage.fields_.council_resolution_count_[3]
                    > GameDataStateStorage.fields_.council_resolution_count_[4]) {
                continue;
            }
            break;
        case PROP_REPEAL_GLOBAL_TRADE_PACT:
            if (GameDataStateStorage.fields_.council_resolution_count_[4]
                    == GameDataStateStorage.fields_.council_resolution_count_[3]) {
                continue;
            }
            break;
        case PROP_INCREASE_SOLAR_SHADE:
            if (!GameDataStateStorage.fields_.council_resolution_count_[5]) {
                continue;
            }
            break;
        }

        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Proposal[proposal].name));
        strcat(StringTemp, "|");
        strcat(StringTemp, StringTable.get(Proposal[proposal].description));
        popup.dialogs().item(StringTemp, proposal);
        rendered_count++;
        if (is_human(faction_id)) {
            continue;
        }
        // Source fields named unk_29/unk_30 are independently anchored at
        // PlayerData+0x400/+0x42C; their meanings remain provisional.
        if (PlayersData[faction_id].unk_29[proposal]) {
            if (proposal || eligible(PlayersData[faction_id].unk_30[0])) {
                continue;
            }
        }
        if (!proposal) {
            for (i = 1; i < MaxPlayerNum; i++) {
                if (i != GameDataStateStorage.fields_.governor_faction_ && eligible(i)) {
                    int wants = wants_prop(faction_id, PROP_ELECT_PLANETARY_GOVERNOR, i);
                    if (wants > max_wants) {
                        max_wants = wants;
                        selected_proposal = PROP_ELECT_PLANETARY_GOVERNOR;
                        selected_faction = i;
                    }
                }
            }
        } else {
            // alien_alive is set only by the Supreme Leader case above;
            // the original reads it uninitialized otherwise.
            int wants = wants_prop(faction_id, proposal, faction_id);
            if (!alien_alive && wants > max_wants) {
                max_wants = wants;
                selected_proposal = proposal;
                selected_faction = faction_id;
            }
        }
    }

    if (!rendered_count) {
        return;
    }
    int selection;
    if (is_human(faction_id)) {
        selection = popup.exec(0, 0);
    } else {
        selection = selected_proposal;
    }
    if (alien_alive && selection == PROP_UNITE_SUPREME_LEADER) {
        if (faction_id == ConsoleGlobal.faction_id_
                && is_human(faction_id)) {
            NetMsgPrimary.pop("NOTWHILEALIENSLIVE", "council_sm.pcx");
        }
        return;
    }
    if (selection < 0) {
        return;
    }

    int turn = GameDataStateStorage.fields_.turn_current_num_;
    int delay = Rules.min_turns_councils;
    if (selection
            && !((GameDataStateStorage.fields_.game_state_ & 0x80)
                 && (GameDataStateStorage.fields_.game_state_ & 0x1000)
                 && is_human(faction_id))) {
        int due = (faction_id == GameDataStateStorage.fields_.governor_faction_) ? delay / 2 : delay;
        if (turn - PlayersData[faction_id].council_call_turn < due) {
            if (faction_id == ConsoleGlobal.faction_id_
                    && is_human(faction_id)) {
                parse_num(0, game_year(PlayersData[ConsoleGlobal.faction_id_].council_call_turn));
                parse_num(1, Rules.min_turns_councils);
                parse_num(2, Rules.min_turns_councils / 2);
                NetMsgPrimary.pop("COUNCILRECENTCALL", "council_sm.pcx");
            }
            return;
        }
    }
    int last_turn = GameDataStateStorage.fields_.council_last_resolution_turn_[selection];
    if (turn - last_turn < delay
            && !((GameDataStateStorage.fields_.game_state_ & 0x80)
                 && (GameDataStateStorage.fields_.game_state_ & 0x1000)
                 && is_human(faction_id))) {
        if (faction_id == ConsoleGlobal.faction_id_
                && is_human(faction_id)) {
            parse_num(0, game_year(last_turn));
            parse_num(1, Rules.min_turns_councils);
            NetMsgPrimary.pop("COUNCILRECENTPROP", "council_sm.pcx");
        }
        return;
    }

    if (IsMultiplayerNet) {
        if (!is_human(faction_id)) {
            CouncilPendingMessageArgs[0] = faction_id;
            CouncilPendingMessageArgs[1] = selection;
            CouncilPendingMessageArgs[2] = selected_faction;
            return;
        }
        NetMsgPrimary.pop("CONVENING", 0x0BB8, 0, 0);
        if (diplo_lock(0x32)) {
            NetMsgPrimary.close();
            return;
        }
        message_data(0x2600, 0, faction_id, selection, selected_faction, 0);
        TurnRedrawPending = 1;
        while (1) {
            if (ExitTurnLoop) {
                break;
            }
            wait_loop();
            if (!TurnRedrawPending) {
                break;
            }
        }
        diplo_unlock();
    } else {
        council(faction_id, selection, selected_faction);
    }
}

// ORIGINAL: 0x004243D0 ?clear_council_notify@@YAXH@Z 0x004243D0-0x004243E8 BYTE_EXACT
// size      24 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x004243D0  ?clear_council_notify@@YAXH@Z  ->  ?clear_council_notify@@YAXH@Z
//
// Historical byte-match context: this overload was first recorded before it
// had a compiled source owner. Its implementation and BYTE_EXACT record are
// now retained here in the Council owner.


void __cdecl clear_council_notify(int a1) {
    CouncilNotifyFormSlots[a1] = 0;
    CouncilNotifyPluralSlots[a1] = 0;
}

// ORIGINAL: 0x004243F0 ?clear_council_notify@@YAXXZ 0x004243F0-0x0042440D FILE BYTE_EXACT
// size      29 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Historical context: the source-form observations below came from earlier
// compiler-matching work on this preserved unit. They describe useful
// compiler-sensitive forms, not instructions to regenerate a translation
// unit or verification artifact.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

void __cdecl clear_council_notify() {
    memset(CouncilNotifyPluralSlots, 0, 8 * sizeof(int));
    memset(CouncilNotifyFormSlots, 0, 8 * sizeof(int));
}

