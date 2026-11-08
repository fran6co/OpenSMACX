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
#include "alpha.h"
#include "veh.h"

/*
Purpose: Calculate the armor strategy for the specified armor id.
// ORIGINAL: 0x0057D270 ?arm_strat@@YAHHH@Z 0x0057D270-0x0057D2D5 BYTE_EXACT
// size      101 bytes
// prototype int (__cdecl ?arm_strat@@YAHHH@Z)(int armorID, int factionID)
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00501500
Return Value: Armor strategy
Status: Complete
*/
int __cdecl arm_strat(int armor_id, int faction_id) {
    if (!ExpansionEnabled && armor_id > ARM_PSI_DEFENSE) {
        return 1;
    }
    int8_t defense_rating = Armor[armor_id].defense_rating;
    if (defense_rating < 0) {
        return psi_factor((Rules.psi_combat_ratio_def[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_weapon_value)
            / Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
    }
    return defense_rating;
}

/*
Purpose: Calculate the weapon strategy for the specified weapon id.
// ORIGINAL: 0x0057D2E0 ?weap_strat@@YAHHH@Z 0x0057D2E0-0x0057D35D BYTE_EXACT
// LEVER: the `!ExpansionEnabled && (a || b || c)` guard was three SEPARATE early-return `if`s under one outer `if (!ExpansionEnabled)`, not one `||` chain - the image has THREE distinct `mov eax,1; pop ebp; ret` blocks (no shared tail), which VC6 only emits for three genuinely separate `return` statements. Same `int8_t` / `(int)` cast pair as arm_strat for the byte read and the signed division.
// size      125 bytes
// prototype int (__cdecl ?weap_strat@@YAHHH@Z)(int weaponID, int factionID)
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00501500
Return Value: Weapon strategy
Status: Complete
*/
int __cdecl weap_strat(int weapon_id, int faction_id) {
    if (!ExpansionEnabled) {
        if (weapon_id == WPN_RESONANCE_LASER) {
            return 1;
        }
        if (weapon_id == WPN_RESONANCE_BOLT) {
            return 1;
        }
        if (weapon_id == WPN_STRING_DISRUPTOR) {
            return 1;
        }
    }
    int8_t offense_rating = Weapon[weapon_id].offense_rating;
    if (offense_rating < 0) {
        return psi_factor((Rules.psi_combat_ratio_atk[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_armor_value)
            / Rules.psi_combat_ratio_def[TRIAD_LAND], faction_id, true, false);
    }
    return offense_rating;
}

/*
Purpose: Calculate the weapon value for the specified prototype.
// ORIGINAL: 0x0057D360 ?weap_val@@YAHHH@Z 0x0057D360-0x0057D3ED BYTE_EXACT
// size      141 bytes
// prototype int (__cdecl ?weap_val@@YAHHH@Z)(int protoID, int factionID)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00501500
Return Value: Weapon value
Status: Complete
*/
int __cdecl weap_val(int proto_id, int faction_id) {
    // weap_strat() (0x0057D2E0, BYTE_EXACT as its own out-of-line function)
    // is hand-inlined here: the image writes its whole body out at this
    // call site (psi_factor() is the only call it keeps), rather than
    // calling 0x0057D2E0.
    int weapon_id = VehPrototypes[proto_id].weapon_id;
    if (!ExpansionEnabled) {
        if (weapon_id == WPN_RESONANCE_LASER) {
            return 1;
        }
        if (weapon_id == WPN_RESONANCE_BOLT) {
            return 1;
        }
        if (weapon_id == WPN_STRING_DISRUPTOR) {
            return 1;
        }
    }
    int8_t offense_rating = Weapon[weapon_id].offense_rating;
    if (offense_rating < 0) {
        return psi_factor((Rules.psi_combat_ratio_atk[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_armor_value)
            / Rules.psi_combat_ratio_def[TRIAD_LAND], faction_id, true, false);
    }
    return offense_rating;
}

/*
Purpose: Calculate the armor value for the specified armor id.
// ORIGINAL: 0x0057D3F0 ?arm_val@@YAHHH@Z 0x0057D3F0-0x0057D471 BYTE_EXACT
// size      129 bytes
// prototype int (__cdecl ?arm_val@@YAHHH@Z)(int armorID, int factionID)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00501500
// LEVER: WRONG CALLEE - arm_strat() (0x0057D270, own out-of-line function)
//        is hand-inlined at this site, matching the image's one call
//        (psi_factor), not arm_strat. Best similarity 0.980 (14->44 of 48).
Return Value: Armor value
Status: Complete
*/
int __cdecl arm_val(int armor_id, int faction_id) {
    // arm_strat() (0x0057D270, own out-of-line function) is hand-inlined
    // here: the image writes its whole body out at this call site
    // (psi_factor() is the only call it keeps), rather than calling
    // 0x0057D270.
    int defense;
    if (faction_id >= 0) {
        if (!ExpansionEnabled && armor_id > ARM_PSI_DEFENSE) {
            defense = 1;
        } else {
            int8_t defense_rating = Armor[armor_id].defense_rating;
            if (defense_rating < 0) {
                defense = psi_factor((Rules.psi_combat_ratio_def[TRIAD_LAND]
                    * PlayersData[faction_id].enemy_best_weapon_value)
                    / Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
            } else {
                defense = defense_rating;
            }
        }
    } else {
        defense = Armor[armor_id].defense_rating;
    }
    return defense * 2;
}

/*
Purpose: Calculate the armor value for the specified prototype.
// ORIGINAL: 0x0057D480 ?armor_val@@YAHHH@Z 0x0057D480-0x0057D50D BYTE_EXACT
// size      141 bytes
// prototype int (__cdecl ?armor_val@@YAHHH@Z)(int protoID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00501500
// LEVER: WRONG CALLEE - arm_val() (0x0057D3F0) and arm_strat() (0x0057D270)
//        are both hand-inlined at this site, matching the image's one call
//        (psi_factor), not arm_val. Best similarity 0.981.
Return Value: Armor value
Status: Complete
*/
int __cdecl armor_val(int proto_id, int faction_id) {
    // arm_val() (0x0057D3F0) and arm_strat() (0x0057D270) are both
    // hand-inlined here: the image writes the whole chain out at this call
    // site (psi_factor() is the only call it keeps), rather than calling
    // 0x0057D3F0.
    int armor_id = VehPrototypes[proto_id].armor_id;
    int defense;
    if (faction_id >= 0) {
        if (!ExpansionEnabled && armor_id > ARM_PSI_DEFENSE) {
            defense = 1;
        } else {
            int8_t defense_rating = Armor[armor_id].defense_rating;
            if (defense_rating < 0) {
                defense = psi_factor((Rules.psi_combat_ratio_def[TRIAD_LAND]
                    * PlayersData[faction_id].enemy_best_weapon_value)
                    / Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
            } else {
                defense = defense_rating;
            }
        }
    } else {
        defense = Armor[armor_id].defense_rating;
    }
    return defense * 2;
}
