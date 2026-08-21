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
#include "faction.h"
#include "alpha.h"
#include "base.h"
#include "council.h"
#include "game.h"
#include "map.h"
#include "probe.h"
#include "technology.h"
#include "veh.h"

Player *Players = (Player *)0x00946A50;
PlayerData *PlayersData = (PlayerData *)0x0096C9E0;
RulesSocialCategory *SocialCategories = (RulesSocialCategory *)0x0094B000;
uint32_t *FactionRankings = (uint32_t *)0x009A64EC; // [8]
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
// The faction whose voice the next popup speaks in. Popup::start (0x00406380)
// and popp (0x0048C0A0) are the only readers: each bounds it to 1..7 and, when
// Players[it].rule_flags has RFLAG_ALIEN, reads the dialogue out of
// alienuscript.txt instead of the ordinary script. Thirty-odd diplomacy entry
// points publish their counterpart faction here before opening a popup, which
// is why suggest_plan writes it unconditionally rather than only on the paths
// that go on to show one.
int PopupDialogFactionID;  // 0x0093F7CC

/*
Purpose: Determine if the specified faction is a Progenitor alien faction (Caretakers / Usurpers).
Original Offset: n/a
Return Value: Is faction a Progenitor? true/false
Status: Complete
*/
BOOL __cdecl is_alien_faction(uint32_t faction_id) {
    return ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_ALIEN);
}



/*
Purpose: Calculate the base amount of talents and drones for the specified faction.
// ORIGINAL: 0x004EA4A0 ?psych_check@@YAXHPAHPAH@Z 0x004EA4A0-0x004EA533
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
    *drones = 6 - (is_human(faction_id) ? PlayersData[faction_id].diff_level : DLVL_LIBRARIAN);
    *talents = (((*drones + 2) * (PlayersData[faction_id].soc_effect_pending.efficiency < 0 ? 4
        : PlayersData[faction_id].soc_effect_pending.efficiency + 4) * MapAreaSqRoot) / 56) / 2;
}

/*
Purpose: Check whether the primary faction has at least one of the specified treaties (bitfield) 
         with the secondary faction.
// ORIGINAL: 0x005002F0 ?has_treaty@@YAHHHH@Z 0x005002F0-0x00500319
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
// ORIGINAL: 0x0050B910 ?get_adjective@@YAHH@Z 0x0050B910-0x0050B92A
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
// ORIGINAL: 0x0050B930 ?get_noun@@YAHH@Z 0x0050B930-0x0050B964
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
LPSTR __cdecl get_noun(int faction_id) {
    parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
    return Players[faction_id].noun_faction;
}

/*
Purpose: Determine whether automatic contact is enabled for net or PBEM games.
// ORIGINAL: 0x00539160 ?auto_contact@@YAHXZ 0x00539160-0x00539193
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
    return (IsMultiplayerNet && Rules->tgl_human_always_contact_net) ? true
        : IsMultiplayerPBEM && Rules->tgl_humans_always_contact_pbem;
}

/*
Purpose: Determine if the overall dominant human faction is a minor threat based on base count.
// ORIGINAL: 0x00539B70 ?great_beelzebub@@YAHHH@Z 0x00539B70-0x00539BF2
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
        uint32_t bases_threat = (TurnCurrentNum + 25) / 50;
        if (bases_threat < 4) {
            bases_threat = 4;
        }
        if (PlayersData[faction_id].current_num_bases > bases_threat
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
    if (great_beelzebub(faction_id, is_aggressive)) {
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
    return false;
}

/*
Purpose: Check whether the specified faction is nearing the diplomatic victory requirements to be 
         able to call a Supreme Leader vote. Optional 2nd parameter (0/-1 to disable) that specifies
         a faction to skip if they have a pact with faction from the 1st parameter.
// ORIGINAL: 0x00539D40 ?aah_ooga@@YAHHH@Z 0x00539D40-0x00539E3E
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
    uint32_t votes_total = 0;
    for (uint32_t i = 1; i < MaxPlayerNum; i++) {
        votes_total += council_votes(i);
    }
    uint32_t faction_id_ret = 0;
    for (int player = 1; player < MaxPlayerNum; player++) {
        if (player != pact_faction_id
            && (pact_faction_id <= 0 || !has_treaty(player, pact_faction_id, DTREATY_PACT)
                || !(GameRules & RULES_VICTORY_COOPERATIVE))) {
            int proposal_preq = Proposal[PROP_UNITE_SUPREME_LEADER].preq_tech;
            if ((has_tech(proposal_preq, faction_id)
                || (proposal_preq >= 0 
                    && (has_tech(Technology[proposal_preq].preq_tech_1, faction_id)
                    || has_tech(Technology[proposal_preq].preq_tech_2, faction_id))))
                && council_votes(player) * 2 >= votes_total && (!faction_id_ret || player == faction_id)) {
                faction_id_ret = player;
            }
        }
    }
    return faction_id_ret;
}

/*
Purpose: Check if the human controlled player is nearing the endgame.
// ORIGINAL: 0x00539E40 ?climactic_battle@@YAHXZ 0x00539E40-0x00539EE4
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
    if (ascending(0)) {
        for (uint32_t i = 1; i < MaxPlayerNum; i++) {
            if (is_human(i) && (has_tech(Facility[FAC_PSI_GATE].preq_tech, i)
                || has_tech(Facility[FAC_VOICE_OF_PLANET].preq_tech, i))) {
                return true; // Human controlled player has tech to build PSI Gates or VoP
            }
        }
    }
    return false;
}

/*
Purpose: Determine if the specified AI faction is at the game climax based on certain conditions.
// ORIGINAL: 0x00539EF0 ?at_climax@@YAHH@Z 0x00539EF0-0x0053A022
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
        if (i != faction_id && PlayersData[faction_id].corner_market_turn > TurnCurrentNum
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
            if (ascending(i) && !ascending(faction_id)) { // both return same so irrelevant check
                return true;
            }
        }
    }
    return trans_most_min_them && trans_most_min_them > trans_most_min_us;
}

/*
Purpose: Add friction between the two specified factions.
// ORIGINAL: 0x0053A030 ?cause_friction@@YAXHHH@Z 0x0053A030-0x0053A08D
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
    uint32_t *diplo_friction = &PlayersData[faction_id].diplo_friction[faction_id_with];
    *diplo_friction = range(*diplo_friction + friction, 0, 20);
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
// ORIGINAL: 0x0053A100 ?reputation@@YAHHH@Z 0x0053A100-0x0053A14D
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
// ORIGINAL: 0x0053A150 ?get_patience@@YAHHH@Z 0x0053A150-0x0053A1B8
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
Purpose: Set or unset the diplomatic treaty for the specified faction with another faction.
// ORIGINAL: 0x0055BB30 ?set_treaty@@YAXHHHH@Z 0x0055BB30-0x0055BB98
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
        PlayersData[faction_id].diplo_treaties[faction_id_with] |= treaty;
        if (treaty & DTREATY_UNK_40) {
            PlayersData[faction_id].diplo_merc[faction_id_with] = 50;
        }
    } else {
        PlayersData[faction_id].diplo_treaties[faction_id_with] &= ~treaty;
    }
}

/*
Purpose: Set or unset the diplomatic agenda for the specified faction with another faction.
// ORIGINAL: 0x0055BBA0 ?set_agenda@@YAXHHHH@Z 0x0055BBA0-0x0055BBF1
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
// ORIGINAL: 0x005591E0 ?has_agenda@@YAHHHH@Z 0x005591E0-0x00559209
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
    uint32_t region_top_base_count[8] = { 0 }; // bug fix: initialize to zero, original doesn't and
    for (int region = 1; region < MaxRegionLandNum; region++) { // compares arbitrary data on stack
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
// ORIGINAL: 0x00560D50 ?guard_check@@YAHHH@Z 0x00560D50-0x00560DC3
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
    uint32_t plan_region = PlayersData[faction_id].region_base_plan[region];
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
        Goal &goals = PlayersData[faction_id].goals[i];
        if (goals.x == x && goals.y == y && goals.type == type) {
            if (goals.priority <= priority) {
                goals.priority = (int16_t)priority;
            }
            return;
        }
    }
    int priority_search = 0;
    int goal_id = -1;
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
// ORIGINAL: 0x00579CC0 ?at_goal@@YAHHHHH@Z 0x00579CC0-0x00579D16
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
    for (int i = 0; i < MaxGoalsNum; i++) {
        Goal &goals = PlayersData[faction_id].goals[i];
        if (goals.x == x && goals.y == y && goals.type == type) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Check if a site exists at the tile for the specified faction and type.
// ORIGINAL: 0x00579D20 ?at_site@@YAHHHHH@Z 0x00579D20-0x00579D76
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
    for (int i = 0; i < MaxSitesNum; i++) {
        Goal &sites = PlayersData[faction_id].sites[i];
        if (sites.x == x && sites.y == y && sites.type == type) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Wipe all goals for the specified faction then recreate any active site related goals.
// ORIGINAL: 0x00579D80 ?wipe_goals@@YAXH@Z 0x00579D80-0x00579DFC
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
// ORIGINAL: 0x00579E00 ?init_goals@@YAXH@Z 0x00579E00-0x00579E66
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
        goals.type = -1;
        goals.priority = 0;
        goals.x = 0;
        goals.y = 0;
        goals.base_id = 0;
    }
    for (i = 0; i < MaxSitesNum; i++) {
        Goal &sites = PlayersData[faction_id].sites[i];
        sites.type = -1;
        sites.priority = 0;
        sites.x = 0;
        sites.y = 0;
        sites.base_id = 0;
    }
}

/*
Purpose: Delete sites of the specified type within proximity of the tile along with related goals.
// ORIGINAL: 0x00579E70 ?del_site@@YAXHHHHH@Z 0x00579E70-0x00579F73
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
    for (int i = 0; i < MaxSitesNum; i++) {
        Goal &sites = PlayersData[faction_id].sites[i];
        if (sites.type == type) {
            int dist = vector_dist(x, y, sites.x, sites.y);
            if (dist <= proximity) {
                sites.type = AI_GOAL_UNUSED;
                sites.priority = 0;
                for (int j = 0; j < MaxGoalsNum; j++) {
                    Goal &goal_compare = PlayersData[faction_id].goals[j];
                    if (goal_compare.x == sites.x && goal_compare.y == sites.y &&
                        goal_compare.type == type) {
                        goal_compare.type = AI_GOAL_UNUSED;
                    }
                }
            }
        }
    }
}

/*
Purpose: Calculate the cost for the faction to corner the Global Energy Market (Economic Victory).
// ORIGINAL: 0x0059EE50 ?corner_market@@YAHH@Z 0x0059EE50-0x0059EEDD
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
Purpose: Validate whether each faction meets the requirements to have the Map revealed. Added some
         minor tweaks to improve performance without changing the logic.
// ORIGINAL: 0x005A96D0 ?see_map_check@@YAXXZ 0x005A96D0-0x005A9753
// size      131 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: n/a
Status: Complete
*/
void __cdecl see_map_check() {
    for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
        PlayersData[faction_id].flags &= ~PFLAG_MAP_REVEALED;
        uint32_t *satellites = &PlayersData[faction_id].satellites_nutrient;
        for (int i = 0; i < 4; i++, satellites++) {
            if (*satellites) {
                PlayersData[faction_id].flags |= PFLAG_MAP_REVEALED;
                break; // end satellite loop early once set
            }
        }
        if (!(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) { // skip Tech check if set
            for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
                if (Technology[tech_id].flags & TFLAG_REVEALS_MAP 
                    && has_tech(tech_id, faction_id)) {
                    PlayersData[faction_id].flags |= PFLAG_MAP_REVEALED;
                }
            }
        }
    }
}

/*
Purpose: Rescore every faction's power, and optionally publish the ranking order that scoring
         implies plus the betrayals the new order provokes.
// ORIGINAL: 0x005AC690 ?rankings@@YAXH@Z 0x005AC690-0x005ACBD6
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
                if (has_tech(tech_id, faction_id)) {
                    score += (tech_id < MaxTechnologyNum)
                        ? Technology[tech_id].growth_value + Technology[tech_id].tech_value
                            + Technology[tech_id].wealth_value + Technology[tech_id].power_value
                        : 4;
                }
            }
            const int *projects = reinterpret_cast<const int *>(SecretProject);
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
    uint32_t humans = FactionsStatus[0];
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
    for (int i = 0; i < count; i++) {
        if (Players[faction_id].faction_bonus_id[i] == RULE_SOCIAL) {
            *(&PlayersData[faction_id].soc_effect_base.economy
                + Players[faction_id].faction_bonus_val1[i]) 
                += Players[faction_id].faction_bonus_val2[i];
        }
    }
}

/*
Purpose: Calculate the social engineering effect modifiers for the specified faction.
// ORIGINAL: 0x005B4210 ?social_calc@@YAXPAHPAHHHH@Z 0x005B4210-0x005B44C9
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
        uint32_t model = *(&category->politics + cat);
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
// ORIGINAL: 0x005B44D0 ?social_upkeep@@YAXH@Z 0x005B44D0-0x005B4550
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
        *(&PlayersData[faction_id].soc_category_active.politics + i) =
            *(&PlayersData[faction_id].soc_category_pending.politics + i);
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
uint32_t __cdecl social_upheaval(int faction_id, SocialCategory *category_new) {
    uint32_t change_count = 0;
    for (int i = 0; i < MaxSocialCatNum; i++) {
        if (*(&category_new->politics + i) != 
            *(&PlayersData[faction_id].soc_category_active.politics + i)) {
            change_count++;
        }
    }
    if (!change_count) {
        return 0;
    }
    change_count++;
    uint32_t diff_lvl = is_human(faction_id) ? PlayersData[faction_id].diff_level : DLVL_LIBRARIAN;
    uint32_t cost = change_count * change_count * change_count * diff_lvl;
    if (is_alien_faction(faction_id)) {
        cost += cost / 2;
    }
    return cost;
}

/*
Purpose: Check to see whether the faction can utilize a specific social category and model.
// ORIGINAL: 0x005B4730 ?society_avail@@YAHHHH@Z 0x005B4730-0x005B4786
// size      86 bytes
// prototype int (__cdecl ?society_avail@@YAHHHH@Z)(int socCategory, int socModel, int factionID)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Is social category/model available to faction? true/false
Status: Complete
*/
BOOL __cdecl society_avail(int soc_category, int soc_model, int faction_id) {
    if (Players[faction_id].soc_anti_ideology_category == soc_category
        && Players[faction_id].soc_anti_ideology_model == soc_model) {
        return false;
    }
    return has_tech(SocialCategories[soc_category].preq_tech[soc_model], faction_id);
}

/*
Purpose: Calculate an AI faction's social engineering.
// ORIGINAL: 0x005B4790 ?social_ai@@YAXHHHHHPAH@Z 0x005B4790-0x005B5612
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
    // purpose?? loop to set unk_val2 to 256
    uint32_t unk_val2 = 4;
    for (int i = 3; i >= 0; i--) {
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
        for (int j = 0; j < MaxSocialCatNum; j++) {
            int model = k & 3;
            *(&soc_cat.politics + player) = model;
            if (model) {
                if (Players[faction_id].soc_anti_ideology_category == j
                    || Players[faction_id].soc_anti_ideology_model == model
                    || !has_tech(SocialCategories[model].preq_tech[j], faction_id)) {
                    break;
                }
            }
            k >>= 2;
        }
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
                int unk_val7 = *(&soc_cat.politics + ideology_cat);
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
                *(&output->politics + i)
                    = *(&PlayersData[faction_id].soc_category_pending.politics + i);
            }
        }
    } else {
        if (!output) {
            BOOL no_cat_change = true;
            for (int i = 0; i < MaxSocialCatNum; i++) {
                uint32_t cat_bit = soc_cat_bits & 3;
                if (cat_bit != *(&PlayersData[faction_id].soc_category_pending.politics + i)) {
                    *(&PlayersData[faction_id].soc_category_pending.politics + i) = cat_bit;
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
                *(&output->politics + i) = soc_cat_bits & 3;
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
    BOOL has_worms = veh_avail(BSC_MIND_WORMS, faction_id, -1);
    PlayersData[faction_id].best_psi_offense = has_worms 
        ? weap_strat(WPN_PSI_ATTACK, faction_id) : 0;
    PlayersData[faction_id].best_weapon_value = 1;
    for (int i = 0; i < MaxWeaponNum; i++) {
        if (has_tech(Weapon[i].preq_tech, faction_id) && Weapon[i].offense_rating < 99) {
            int weap_val = weap_strat(i, faction_id);
            if (Weapon[i].offense_rating < 0 && weap_val 
                > PlayersData[faction_id].best_psi_offense) {
                PlayersData[faction_id].best_psi_offense = weap_val;
            }
            if (weap_val > PlayersData[faction_id].best_weapon_value) {
                PlayersData[faction_id].best_weapon_value = weap_val;
            }
        }
    }
    PlayersData[faction_id].best_psi_defense = has_worms 
        ? arm_strat(ARM_PSI_DEFENSE, faction_id) : 0;
    PlayersData[faction_id].best_armor_value = 1;
    for (i = 0; i < MaxArmorNum; i++) {
        if (has_tech(Armor[i].preq_tech, faction_id)) {
            int arm_val = arm_strat(i, faction_id);
            if (Armor[i].defense_rating < 0 && arm_val > PlayersData[faction_id].best_psi_defense) {
                PlayersData[faction_id].best_psi_defense = arm_val;
            }
            if (arm_val > PlayersData[faction_id].best_armor_value) {
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
    PlayersData[faction_id].enemy_best_psi_offense = 0;
    PlayersData[faction_id].enemy_best_psi_defense = 0;
    for (i = 0; i < 4 && !PlayersData[faction_id].enemy_best_weapon_value; i++) {
        // 1st pass: vendetta, no treaty, has commlink
        // 2nd pass: no treaty, has commlink
        // 3rd pass: has commlink
        // 4th pass: any non-pact faction
        for (uint32_t j = 1; j < MaxPlayerNum; j++) {
            if (j != faction_id
                && !has_treaty(i, j, DTREATY_PACT)
                && ((!i && has_treaty(i, j, DTREATY_VENDETTA) && !has_treaty(i, j, DTREATY_TREATY)
                    && has_treaty(i, j, DTREATY_COMMLINK)) 
                    || (i == 1 && !has_treaty(i, j, DTREATY_TREATY) 
                        && has_treaty(i, j, DTREATY_COMMLINK))
                    || (i == 2 && has_treaty(i, j, DTREATY_COMMLINK)) || (i == 3))) {
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
