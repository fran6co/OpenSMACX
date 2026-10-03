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

// council_action increments a proposal's resolution count and records the
// current turn before dispatching its effect. setup_game's native paired
// reset loop (005B3A7A..3A90) independently establishes eleven signed entries
// in each table; its owning implementation remains unfinished.
// RECOVERY_OBJECT: v2 0x009A6618 0x009A6644 ?CouncilResolutionCount@@3PAHA 0x005B3920
int CouncilResolutionCount[MaxProposalNum];
// RECOVERY_OBJECT: v2 0x009A6644 0x009A6670 ?CouncilLastResolutionTurn@@3PAHA 0x005B3920
int CouncilLastResolutionTurn[MaxProposalNum];

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
// initial image bytes are twelve FF bytes. Keep their meanings neutral until
// the unfinished council producer is recovered, rather than inventing fields.
// RECOVERY_OBJECT: v1 0x0068BF38 ?CouncilPendingMessageArgs@@3PAHA
int CouncilPendingMessageArgs[3] = {-1, -1, -1};

// The council dialog remains an explicit dependency of MultiWin's button
// callback. Its full historical body is retained in the seed, not discarded.
// ORIGINAL: 0x0052C880 ?call_council@@YAXH@Z 0x0052C880-0x0052DA98;0x0065DB70-0x0065DF45
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0052c880.cpp", "src/council.h", "src/multiwin.cpp"], "reason": "The original MultiWin council button calls this faction-taking council dialog. This linkable placeholder preserves the dependency until the council owner recovers its complex dialog body."}
void __cdecl call_council(int) {
    // BODY GOES HERE.
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
    for (int i = 0; i < BaseCurrentCount; i++, base++) {
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
    CouncilResolutionCount[proposal]++;
    CouncilLastResolutionTurn[proposal] = TurnCurrentNum;

    switch (proposal) {
    case PROP_UNITE_SUPREME_LEADER: {
        GameState |= STATE_VICTORY_DIPLOMATIC;
        int local_faction = ConsoleGlobal.faction_id_;
        if (faction_id == local_faction) {
            if (!IsMultiplayerNet) {
                bool candidate_found = false;
                for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
                    if (candidate == faction_id || !is_alive(candidate)
                        || !(GameRules & RULES_VICTORY_CONQUEST)) {
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
            GameState |= STATE_UNK_1;
            return;
        }

        council_parse_leader(faction_id);
        const bool cooperative_pact =
            (GameRules & RULES_VICTORY_COOPERATIVE)
            && (PlayersData[local_faction].diplo_treaties[faction_id] & DTREATY_PACT);
        if (!cooperative_pact) {
            if (IsMultiplayerNet
                || !popp(ScriptPopupFile, "ACCEDE", 0, "dipvic_sm.pcx", 0)) {
                EndgameExitReason = 11;
                GameState |= STATE_UNK_1;
                PlayersData[faction_id].flags |= PFLAG_UNK_20000;
                return;
            }
        } else if (IsMultiplayerNet
                   || !popp(ScriptPopupFile, "ACCEDECOOP", 0, "dipvic_sm.pcx", 0)) {
            EndgameExitReason = 10;
            GameState |= STATE_UNK_1;
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
        ClimateSeaLevelDelta -= static_cast<int>(WorldBuilder.sea_level_rises);
        ClimateRiseStep = ClimateSeaLevelDelta;
        if (ClimateRiseStep < 0) {
            ClimateRiseStep = -ClimateRiseStep;
        }
        ClimateRiseLimit = 20;
        ClimateRiseAccum = 0;
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
        ClimateSeaLevelDelta += static_cast<int>(WorldBuilder.sea_level_rises);
        ClimateRiseStep = ClimateSeaLevelDelta;
        if (ClimateRiseStep < 0) {
            ClimateRiseStep = -ClimateRiseStep;
        }
        ClimateRiseLimit = 20;
        ClimateRiseAccum = 0;
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
