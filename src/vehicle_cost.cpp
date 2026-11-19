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
#include "general.h"
#include "veh.h"
#include "technology.h"

// The prototype-cost unit: proto_cost 0x005A5A60, base_cost 0x005A5D00 and
// make_proto 0x005A5D40 form one address band after probe's unit (whose
// header-inline copies end at 0x005A5A52) and before proto_sort 0x005A6270.
// make_proto calls transport_val (0x0057D510, the veh unit) out of line:
// this unit sees only its declaration.

// RECOVERY_ASSOCIATED: v1 switch_targets 0x005A5A60 0x005A5CE4 0x005A5D00 0x005A5AF4
/*
Purpose: Calculates cost of the prototype based on various factors. Optimized logic flow from
         the original without any differences to the final calculation.
// ORIGINAL: 0x005A5A60 ?proto_cost@@YAHHHHHH@Z 0x005A5A60-0x005A5CE1 BYTE_EXACT
// symbol    ?proto_cost@@YAHHHHHH@Z
// size      641 bytes
// prototype int (__cdecl ?proto_cost@@YAHHHHHH@Z)(int chassisType, int weapType, int armorType, int ability, int reactorType)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// Rewritten from the image: one live weapon-cost variable that becomes combat_mod,
//   no zero-armor guard in the ratio case, separate land increment, final >> 2.
Return Value: Cost of prototype
Status: Complete
*/
int __cdecl proto_cost(int chassis_id, int weapon_id, int armor_id,
                            int ability, int reactor_id) {
    int combat_mod = Weapon[weapon_id].cost;
    int armor_cost = Armor[armor_id].cost;
    int speed_cost = Chassis[chassis_id].cost;
    int abil_modifier = 0;
    if (ability) {
        for (int i = 0; i < MaxAbilityNum; i++) {
            if ((1 << i) & ability) {
                if (abil_modifier) {
                    abil_modifier++; // Increased cost with more than one ability
                }
                int factor = Ability[i].cost_factor;
                if (factor >= 0) { // 1+ = Straight Cost; 25% increase per unit of cost
                    abil_modifier += factor;
                } else {
                    switch (factor) {
                      // Increases w/ ratio of weapon to armor: 0, 1, or 2. Rounded DOWN.
                      // Never higher than 2. The original has no zero-armor guard here.
                      case -1:
                        abil_modifier += range(combat_mod / armor_cost, 0, 2);
                        break;
                      case -2: // Increases w/ weapon value
                        abil_modifier += combat_mod - 1;
                        break;
                      case -3: // Increases w/ armor value
                        abil_modifier += armor_cost - 1;
                        break;
                      case -4: // Increases w/ speed value
                        abil_modifier += speed_cost - 1;
                        break;
                      case -5: // Increases w/ weapon+armor value
                        abil_modifier += combat_mod + armor_cost - 2;
                        break;
                      case -6: // Increases w/ weapon+speed value
                        abil_modifier += combat_mod + speed_cost - 2;
                        break;
                      case -7: // Increases w/ armor+speed value
                        abil_modifier += armor_cost + speed_cost - 2;
                        break;
                    }
                }
            }
        }
    }
    uint8_t triad = Chassis[chassis_id].triad;
    if (triad == TRIAD_SEA) {
        armor_cost /= 2;
        speed_cost += reactor_id;
    } else if (triad == TRIAD_AIR) {
        speed_cost += reactor_id * 2;
        if (armor_cost > 1) {
            armor_cost *= reactor_id * 2;
        }
    } else {
        // 010000000000 - Cost increased for land units; Deep Radar
        for (int i = 0; i < MaxAbilityNum; i++) {
            if ((1 << i) & ability) {
                if (Ability[i].flags & AFLAG_COST_INC_LAND_UNIT) {
                    abil_modifier++;
                }
            }
        }
    }
    if (Chassis[chassis_id].missile && Weapon[weapon_id].offense_rating >= 99) {
        return Weapon[weapon_id].cost;
    }
    if (armor_cost / 2 + 1 > combat_mod) { // which ever is greater
        combat_mod = armor_cost / 2 + 1;
    }
    // (2 << n) == 2^(n + 1) ; (2 << n) / 2 == 2 ^ n;
    int proto_mod = ((speed_cost + armor_cost) * combat_mod + ((2 << reactor_id) / 2))
        / (2 << reactor_id);
    if (speed_cost == 1) {
        proto_mod = (proto_mod / 2) + 1;
    }
    if (Weapon[weapon_id].cost > 1 && Armor[armor_id].cost > 1) {
        proto_mod++;
    }
    if (Chassis[chassis_id].triad == TRIAD_LAND && Weapon[weapon_id].cost > 1 && Armor[armor_id].cost > 1
        && speed_cost > 1) {
        proto_mod++;
    }
    // excludes sea probes
    if (triad == TRIAD_SEA) {
        if (Weapon[weapon_id].mode != WPN_MODE_INFOWAR) {
            proto_mod = (proto_mod + 1) / 2;
        }
    } else if (triad == TRIAD_AIR) {
        proto_mod /= (Weapon[weapon_id].mode > WPN_MODE_MISSILE) ? 2 : 4; // Non-combat : Combat
    }
    int reactor_mod = reactor_id * 2 - reactor_id / 2;
    if (proto_mod <= reactor_mod) { // which ever is greater
        proto_mod = reactor_mod;
    }
    if (combat_mod == 1 && armor_cost == 1 && speed_cost == 1 && reactor_id == RECT_FISSION) {
        proto_mod = 1;
    }
    return (proto_mod * (abil_modifier + 4) + 2) >> 2;
}

/*
Purpose: Calculates the base cost of the specified prototype.
// ORIGINAL: 0x005A5D00 ?base_cost@@YAHH@Z 0x005A5D00-0x005A5D3F BYTE_EXACT
// body      src/vehicle_cost.cpp
// size      63 bytes
// prototype int (__cdecl ?base_cost@@YAHH@Z)(int protoID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A5A60
Return Value: Base cost of the prototype
Status: Complete
*/
int __cdecl base_cost(int proto_id) {
    return proto_cost(VehPrototypes[proto_id].chassis_id, VehPrototypes[proto_id].weapon_id,
        VehPrototypes[proto_id].armor_id, 0, VehPrototypes[proto_id].reactor_id);
}

/*
Purpose: Create a new prototype. Sets initial values for everything except veh_name and preq_tech.
// ORIGINAL: 0x005A5D40 ?make_proto@@YAXHHHHHH@Z 0x005A5D40-0x005A6269 BYTE_EXACT
// size      1321 bytes
// prototype void (__cdecl ?make_proto@@YAXHHHHHH@Z)(int protoID, int chassisType, int weapType, int armorType, int ability, int reactorType)
// callers   4   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// Rewritten from the image: the land plan reads the stored prototype ids, the
//   speed >= 2 / >= 3 unk_1 tests are independent (no else), transport_val stays a call.
// calls     0x0057D510 0x005A5A60 0x005B9F20 0x005BF1F0
Return Value: n/a
Status: Complete
*/
void __cdecl make_proto(int proto_id, int chassis_id, int weapon_id, int armor_id,
                        int ability, int reactor_id) {
    int unk_local_1 = 0; // TODO: Identify
    BOOL cond3 = false;
    BOOL cond2 = false;
    BOOL cond1 = false;
    if (proto_id >= MaxVehProtoFactionNum) {
        int proto_id_loop;
        for (int i = 0; i < 128; i++) {
            proto_id_loop = i % MaxVehProtoFactionNum;
            if (proto_id_loop != i) {
                proto_id_loop += proto_id - proto_id % MaxVehProtoFactionNum;
            }
            int16_t flags_check = VehPrototypes[proto_id_loop].flags;
            if (!(flags_check & PROTO_ACTIVE)) {
                continue;
            }
            if (proto_id_loop < MaxVehProtoFactionNum) {
                if (!has_tech(VehPrototypes[proto_id_loop].preq_tech,
                        proto_id / MaxVehProtoFactionNum)) {
                    continue;
                }
            } else if (!(flags_check & PROTO_TYPED_COMPLETE)) {
                continue;
            }
            uint32_t loop_weapon_id = VehPrototypes[proto_id_loop].weapon_id;
            if (weapon_id == loop_weapon_id) {
                cond1 = true;
            }
            uint32_t loop_armor_id = VehPrototypes[proto_id_loop].armor_id;
            if (armor_id == loop_armor_id) {
                cond2 = true;
            }
            uint32_t loop_chassis_id = VehPrototypes[proto_id_loop].chassis_id;
            if (chassis_id == loop_chassis_id) {
                cond3 = true;
            }
            int8_t off_rating = Weapon[weapon_id].offense_rating;
            if (off_rating > 0 && Weapon[loop_weapon_id].offense_rating >= off_rating) {
                cond1 = true;
            }
            int8_t def_rating = Armor[armor_id].defense_rating;
            if (def_rating > 0 && Armor[loop_armor_id].defense_rating >= def_rating) {
                cond2 = true;
            }
            if (Chassis[chassis_id].triad == Chassis[loop_chassis_id].triad
                && Chassis[loop_chassis_id].speed >= Chassis[chassis_id].speed) {
                cond3 = true;
            }
        }
        if (cond1 && cond2 && cond3) {
            unk_local_1 = 1;
            if (proto_id_loop >= MaxVehProtoFactionNum
                && VehPrototypes[proto_id_loop].flags & 0x10) {
                unk_local_1 = 3;
            }
        }
    }
    VehPrototypes[proto_id].reactor_id = (uint8_t)reactor_id;
    VehPrototypes[proto_id].chassis_id = (uint8_t)chassis_id;
    VehPrototypes[proto_id].weapon_id = (uint8_t)weapon_id;
    VehPrototypes[proto_id].armor_id = (uint8_t)armor_id;
    VehPrototypes[proto_id].ability_flags = ability;
    VehPrototypes[proto_id].cost = (uint8_t)proto_cost(chassis_id, weapon_id, armor_id, ability,
        reactor_id);
    VehPrototypes[proto_id].carry_capacity = (Weapon[weapon_id].mode == WPN_MODE_TRANSPORT)
        ? transport_val(chassis_id, ability, reactor_id) : 0;
    // set plan & unk1
    if (Chassis[chassis_id].missile) {
        if (Weapon[weapon_id].offense_rating >= 99) {
            VehPrototypes[proto_id].plan = PLAN_PLANET_BUSTER;
            VehPrototypes[proto_id].unk_1 = 14;
        } else if (weapon_id == WPN_TECTONIC_PAYLOAD) {
            VehPrototypes[proto_id].plan = PLAN_TECTONIC_MISSILE;
            VehPrototypes[proto_id].unk_1 = 22;
        } else if (weapon_id == WPN_FUNGAL_PAYLOAD) {
            VehPrototypes[proto_id].plan = PLAN_FUNGAL_MISSILE;
            VehPrototypes[proto_id].unk_1 = 23;
        } else { // Conventional Payload
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
            VehPrototypes[proto_id].unk_1 = 13;
        }
    } else if (Weapon[weapon_id].mode >= WPN_MODE_TRANSPORT) { // Non-combat
        VehPrototypes[proto_id].plan = Weapon[weapon_id].mode;
        VehPrototypes[proto_id].unk_1 = Weapon[weapon_id].mode + 0x20;
    } else if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA) { // combat sea
        VehPrototypes[proto_id].plan = PLAN_NAVAL_SUPERIORITY;
        VehPrototypes[proto_id].unk_1 = 6; // same value as plan
    } else if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) { // combat air
        if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
            VehPrototypes[proto_id].plan = PLAN_AIR_SUPERIORITY;
            VehPrototypes[proto_id].unk_1 = 9;
        } else {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
            VehPrototypes[proto_id].unk_1 = 8;
        }
    } else { // TRIAD_LAND combat unit; reads go through the stored prototype ids
        VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        if (Armor[VehPrototypes[proto_id].armor_id].defense_rating > 1) {
            VehPrototypes[proto_id].plan = PLAN_DEFENSIVE;
        }
        if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating >= Armor[VehPrototypes[proto_id].armor_id].defense_rating
            && VehPrototypes[proto_id].plan == PLAN_DEFENSIVE) {
            VehPrototypes[proto_id].plan = PLAN_COMBAT;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].speed > 1
            && Weapon[VehPrototypes[proto_id].weapon_id].offense_rating > Armor[VehPrototypes[proto_id].armor_id].defense_rating) {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        }
        if (ability & (ABL_ARTILLERY | ABL_DROP_POD | ABL_AMPHIBIOUS)) {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        }
        VehPrototypes[proto_id].unk_1 = 3;
        if (Armor[VehPrototypes[proto_id].armor_id].defense_rating > Weapon[VehPrototypes[proto_id].weapon_id].offense_rating) {
            VehPrototypes[proto_id].unk_1 = 2;
        }
        if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating > Armor[VehPrototypes[proto_id].armor_id].defense_rating * 2) {
            VehPrototypes[proto_id].unk_1 = 1;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].speed >= 2) {
            VehPrototypes[proto_id].unk_1 = 4;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].speed >= 3) {
            VehPrototypes[proto_id].unk_1 = 5;
        }
        if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating == 1
            && Armor[VehPrototypes[proto_id].armor_id].defense_rating == 1) {
            VehPrototypes[proto_id].unk_1 = 10;
            if (Chassis[VehPrototypes[proto_id].chassis_id].speed > 1) {
                VehPrototypes[proto_id].plan = PLAN_RECONNAISANCE;
            }
            if (has_abil(proto_id, ABL_POLICE_2X)) {
                VehPrototypes[proto_id].plan = PLAN_DEFENSIVE;
            }
        }
    }
    // set final values
    VehPrototypes[proto_id].flags = PROTO_ACTIVE;
    if (unk_local_1) {
        VehPrototypes[proto_id].flags = 0x105;
        if (unk_local_1 & 2) {
            VehPrototypes[proto_id].flags = 0x115;
        }
    }
    VehPrototypes[proto_id].obsolete_factions = 0;
    VehPrototypes[proto_id].combat_factions = (proto_id < MaxVehProtoFactionNum) ?
        0xFF : 1 << (proto_id / MaxVehProtoFactionNum);
    VehPrototypes[proto_id].icon_offset = -1;
}



/*
Purpose: Rank the prototypes a faction can build, best first, into ProtoSortIds.
// ORIGINAL: 0x005A6270 ?proto_sort@@YAXH@Z 0x005A6270-0x005A63C9 FILE BYTE_EXACT
// size      345 bytes
// prototype void (__cdecl ?proto_sort@@YAXH@Z)(int factionID)
// callers   2   call targets   4
// calls     0x0057D360 0x0057D3F0 0x005B5690 0x005B9F20
// SYMBOL FACTS. ?proto_sort@@YAXH@Z is the catalogue's name; the two fixed
//        slots it shares with proto_sort_2 are bound here because nothing
//        else in the tree names them and the pair is one table - 0x00945B40
//        is how many entries the sort filled and 0x00945B44 the id list
//        itself, contiguous, which is why the image writes them 4 bytes
//        apart. The key's four terms are read straight off the layout:
//        Chassis[].triad (+0x49), Chassis[].speed (+0x48), and the
//        prototype's plan / chassis_id / armor_id / flags / preq_tech.
// symbol    ?proto_sort@@YAXH@Z
// TRIED: 0/109. `has_tech` is `MEASURED inline` in technology.h, so this body
//        INLINES it where the image calls it (136 compiled instructions
//        against 109); `has_tech_call` closes that (115 against 109). The
//        rest is the frame: the image keeps a frame pointer and allocates
//        0x204 for the 512-byte key array plus one dword, where this tree
//        allocates 0x20c and goes frame-less under both flag sets, so
//        everything from instruction 0 is shifted. Left as a compiling body.
Return Value: n/a
Status: Complete
*/

// RECOVERY_OBJECT: v2 0x00945B40 0x00945B44 ?ProtoSortCount@@3HA 0x005A63D0
int ProtoSortCount;

// RECOVERY_OBJECT: v2 0x00945B44 0x00945D44 ?ProtoSortIds@@3PAHA 0x005A63D0
int ProtoSortIds[128];

void __cdecl proto_sort(int faction_id) {
    int sort_keys[128];
    ProtoSortCount = 0;
    int i = 0;
    do {
        int proto_id = (i % 64) + (i >= MaxVehProtoFactionNum ? faction_id * 64 : 0);
        ProtoSortIds[i] = proto_id;
        int key;
        if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)
            || (proto_id < MaxVehProtoFactionNum
                && !has_tech(VehPrototypes[proto_id].preq_tech, faction_id))) {
            if (proto_id >= MaxVehProtoFactionNum) {
                key = 0x34FB5E38;
                ProtoSortCount++;
            } else {
                key = 999999999;
            }
        } else {
            key = (Chassis[VehPrototypes[proto_id].chassis_id].triad
                   + VehPrototypes[proto_id].plan * 16) << 18;
            key += (32 - weap_val(proto_id, faction_id)) << 9;
            key += (8 - Chassis[VehPrototypes[proto_id].chassis_id].speed) << 6;
            key += 8 - arm_val(VehPrototypes[proto_id].armor_id, faction_id);
            ProtoSortCount++;
        }
        sort_keys[i] = key;
        i++;
    } while (i < 128);
    sort(128, ProtoSortIds, sort_keys);
}

// Order design choices by availability and obsolete status. Original IDs stay
// in the output table; the added status bands are sorting keys only.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005A63D0 ?proto_sort_2@@YAXH@Z 0x005A63D0-0x005A650B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005A63D0-0x005A650B", "source:src/recovered/units/005a63d0.cpp", "source:src/faction.h:PlayerData.proto_id_active"], "reason": "Recover design-choice ordering with typed prototype, weapon and faction state and source-owned shared output; original IDs remain separate from status ranking keys."}
void __cdecl proto_sort_2(int faction_id) {
    ProtoSortCount = 0;
    int sort_keys[128];
    for (int index = 0; index < 128; ++index) {
        int prototype_id = (index >= MaxVehProtoFactionNum ? faction_id * MaxVehProtoFactionNum : 0)
            + index % MaxVehProtoFactionNum;
        ProtoSortIds[index] = prototype_id;
        if ((VehPrototypes[prototype_id].flags & PROTO_ACTIVE) != 0
            && (prototype_id >= MaxVehProtoFactionNum
                || (Weapon[VehPrototypes[prototype_id].weapon_id].offense_rating >= 0
                    && prototype_id != 15
                    && has_tech(VehPrototypes[prototype_id].preq_tech, faction_id)))) {
            if (VehPrototypes[prototype_id].obsolete_factions & (1 << faction_id)) {
                if (prototype_id >= MaxVehProtoFactionNum) {
                    prototype_id += PlayersData[faction_id].proto_id_active[prototype_id]
                        ? 0x800 : 0x1000;
                } else {
                    prototype_id += 0x2000;
                }
            }
            ++ProtoSortCount;
        } else {
            if (prototype_id >= MaxVehProtoFactionNum) {
                prototype_id += 0x400;
                ++ProtoSortCount;
            } else {
                prototype_id = 999999999;
            }
        }
        sort_keys[index] = prototype_id;
    }
    sort(128, ProtoSortIds, sort_keys);
}
