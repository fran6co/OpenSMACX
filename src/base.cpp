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
#include "base.h"
#include "alpha.h"
#include "faction.h"
#include "game.h"
#include "log.h"
#include "map.h"
#include "text.h"
#include "technology.h"
#include "veh.h"

RulesFacility *Facility = (RulesFacility *)0x009A4B68;
RulesCitizen *Citizen = (RulesCitizen *)0x00946020;
Base *Bases = (Base *)0x0097D040; // [512]
BaseSecretProject *SecretProject = (BaseSecretProject *)0x009A6514; // [64]
int *BaseIDCurrentSelected = (int *)0x00689370;
int *BaseCurrentCount = (int *)0x009A64CC;
int *BaseFindDist = (int *)0x0090EA04;
Base **BaseCurrent = (Base **)0x0090EA30;
uint32_t *ScnVictFacilityObj = (uint32_t *)0x009A6814;
uint32_t *BaseUpkeepStage = (uint32_t *)0x0090EA34;
uint32_t *BaseCurrentConvoyFrom = (uint32_t *)0x0090E904; // [4]
uint32_t *BaseCurrentConvoyTo = (uint32_t *)0x0090E93C; // [4]
int *BaseCurrentGrowthRate = (int *)0x0090E918;
uint32_t *BaseCurrentVehPacifismCount = (uint32_t *)0x0090E980;
uint32_t *BaseCurrentForcesSupported = (uint32_t *)0x0090E8FC;
uint32_t *BaseCurrentForcesMaintCost = (uint32_t *)0x0090E91C;

/*
 * How many of the base's supported units are past its free allowance, counted
 * alongside BaseCurrentForcesMaintCost. base_support() (0x004E9550) is the only
 * function in the executable that touches this address at all - it zeroes it,
 * reads it back and stores the increment, and nothing ever reads the total - so
 * the count is written and then dropped. Kept because it is observable state.
 */
uint32_t *BaseCurrentForcesMaintCount = (uint32_t *)0x0090EA08;

/*
 * Out-parameter of the three terrain yield functions: how much of a resource
 * the square lost to the "three per square needs a technology" restriction, and
 * to mine_yield's roadless-mine mineral limit. Callers zero it, ask for a
 * yield, then read it back - BaseWin::draw_farm (0x0040F0F0) does exactly that
 * for each of the three resources, and resource_yield zeroes it for RSC_PSI.
 * Purpose is observed from those call sites; the executable does not name it.
 */
int *TileYieldRestricted = (int *)0x0090E998;

/*
 * energy_yield publishes the clamped energy of a square that contains a base
 * here. Its only other reader in the image is base_terraform (0x004ECCBD).
 */
int *BaseSquareEnergy = (int *)0x0090E914;

/*
 * The faction elected Planetary Governor: written by CouncWin::make_resolution
 * (0x004246E5), persisted by game_data (0x005A6B61), read by call_council,
 * wants_prop, num_objectives and the council windows. energy_yield grants it a
 * point of energy on a base square.
 *
 * NOTE: src/spying_recovery.cpp already binds this same address as
 * `SpyingObserverFaction` - the Governor is the faction that may view any other
 * faction's bases, so the two names describe one datum. They are deliberately
 * NOT unified here: spying_recovery.cpp belongs to OPENSMACX_LEAF_SRC_CLOSURE,
 * which does not link base.cpp, so neither file can host the other's
 * definition without moving a translation unit between suites. Two pointer
 * variables onto one address are correct at run time; the hazard is that a test
 * rebinding one does not rebind the other.
 */
int *GovernorFaction = (int *)0x009A6614;

/*
 * A two-turn state advanced by random_events (0x0051F2F4 steps it 1 -> 2 -> 0
 * at turn start and arms it at 0x00520529). While the selected bit is set,
 * energy_yield triples its result.
 */
int *GlobalEnergyEventState = (int *)0x009A6818;

/*
 * Selects which bit of GlobalEnergyEventState energy_yield consults. Read by
 * Console::on_key_click, faction_upkeep, NetDaemon::lock_base, wait_loop,
 * check_spock, base_yield and base_doctors; sits immediately below
 * ConsoleControlTurnActive (0x0093A938) and near IsMultiplayerPBEM (0x0093A95C).
 * TODO: identify global - its writer was not located, so it is not named.
 */
int *UnkGlobal0093A934 = (int *)0x0093A934;

/*
Purpose: Check if the base already has a particular facility built or if it's in the queue.
Original Offset: 00421670
Return Value: Does base already have or planning on building facility? true/false
Status: Complete
*/
BOOL __cdecl has_fac(int facility_id, uint32_t base_id, int queue_count) {
    if (facility_id >= FacilityRepStart) {
        return false;
    }
    BOOL is_built = has_fac_built(facility_id, base_id);
    if (is_built || !queue_count) {
        return is_built;
    }
    if (queue_count <= 0 || queue_count > 10) { // added upper bounds check for queueCount
        return false;
    }
    for (int i = 0; i < queue_count; i++) {
        if (Bases[base_id].queue_production_id[i] == -facility_id) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Set the current base globals.
Original Offset: 004E39D0
Return Value: n/a
Status: Complete
*/
void __cdecl set_base(uint32_t base_id) {
    *BaseIDCurrentSelected = base_id;
    *BaseCurrent = &Bases[base_id];
}

/*
Purpose: Get base name string from base id and store it in base_str. If base id is -1, use 'NONE'.
Original Offset: 004E3A00
Return Value: n/a
Status: Complete
*/
void __cdecl say_base(LPSTR base_str, int base_id) {
    std::string output = (base_id >= 0) ? Bases[base_id].name_string : label_get(25); // 'NONE'
    // assumes 1032 char buffer via stringTemp, eventually remove
    strcat_s(base_str, 1032, output.c_str());
}

/*
Purpose: Return the base at the specified map coordinates and repair stale base map bits.
Original Offset: 004E3A50
Return Value: Base id or -1 when no base is present
Status: Complete - testing

The original also displays a debug-only BASEBIT popup and focuses the internal console when the map
bit has no matching base. The state repair behavior is retained without those UI diagnostics.
*/
int __cdecl base_at(int x, int y) {
    if (!on_map(x, y) || !(bit_at(x, y) & BIT_BASE_IN_TILE)) {
        return -1;
    }
    for (int base_id = 0; base_id < *BaseCurrentCount; base_id++) {
        if (Bases[base_id].x == x && Bases[base_id].y == y) {
            return base_id;
        }
    }
    log_say("Base Bits Error  (x, y)", x, y, 0);
    rebuild_base_bits();
    return -1;
}

/*
Purpose: Find the base id closest to the specified coordinates.
Original Offset: 004E3B80
Return Value: Base id or -1 if not found 
Status: Complete
*/
int __cdecl base_find(int x, int y) {
    if (*BaseCurrentCount <= 0) {
        return -1;
    }
    int proximity = 9999;
    int base_id = -1;
    for (int i = 0; i < *BaseCurrentCount; i++) {
        int dist = vector_dist(x, y, Bases[i].x, Bases[i].y);
        if (dist <= proximity) {
            proximity = dist;
            base_id = i;
        }
    }
    if (base_id >= 0) {
        *BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Find the base id closest to the specified coordinates owned by faction.
Original Offset: 004E3C60
Return Value: Base id or -1 if not found
Status: Complete
*/
int __cdecl base_find(int x, int y, uint32_t faction_id) {
    if (*BaseCurrentCount <= 0) {
        return -1;
    }
    int proximity = 9999;
    int base_id = -1;
    for (int i = 0; i < *BaseCurrentCount; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            int dist = vector_dist(x, y, Bases[i].x, Bases[i].y);
            if (dist <= proximity) {
                proximity = dist;
                base_id = i;
            }
        }
    }
    if (base_id >= 0) {
        *BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Find the base id closest to the specified coordinates meeting various conditional checks.
Original Offset: 004E3D50
Return Value: Base id or -1 if not found
Status: Complete
*/
int __cdecl base_find(int x, int y, int faction_id, int region, int faction_id_2,
                      int faction_id_3) {
    int proximity = 9999;
    int base_id = -1;
    *BaseFindDist = 9999; // difference from other two functions where this is reset at start
    if (*BaseCurrentCount <= 0) {
        return -1;
    }
    for (int i = 0; i < *BaseCurrentCount; i++) {
        if (region < 0 || region_at(Bases[i].x, Bases[i].y) == (uint32_t)region) {
            if (faction_id < 0 ? (faction_id_2 < 0 || Bases[i].faction_id_current != faction_id_2)
                : (faction_id == Bases[i].faction_id_current || (faction_id_2 == -2
                    ? has_treaty(faction_id, Bases[i].faction_id_current, DTREATY_PACT)
                    : (faction_id_2 >= 0 && faction_id_2 == Bases[i].faction_id_current)))) {
                if (faction_id_3 < 0 || Bases[i].faction_id_current == faction_id_3
                    || ((1 << faction_id_3) & Bases[i].visibility)) {
                    int dist = vector_dist(x, y, Bases[i].x, Bases[i].y);
                    if (dist <= proximity) {
                        proximity = dist;
                        base_id = i;
                    }
                }
            }
        }
    }
    if (base_id >= 0) {
        *BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Find the best specialist available to the current base with more weight placed on psych.
Original Offset: 004E4020
Return Value: Best citizen id (always going to be 1, 4, or 6 based on default weights)
Status: Complete
*/
uint32_t  __cdecl best_specialist() {
    int current_bonus = -999;
    uint32_t citizen_id = 0;
    for (int i = 0; i < MaxSpecialistNum; i++) {
        if (has_tech(Citizen[i].preq_tech, (*BaseCurrent)->faction_id_current)) {
            uint32_t bonus = Citizen[i].psych_bonus * 3;
            if ((*BaseCurrent)->population_size >= (int)Rules->min_base_size_specialists) {
                bonus += Citizen[i].ops_bonus + Citizen[i].research_bonus;
            }
            if ((int)bonus > current_bonus) {
                current_bonus = bonus;
                citizen_id = i;
            }
        }
    }
    return citizen_id;
}

/*
Purpose: Generate a base name. Added some minor randomization for sea bases and ensuring offset gets
         increased for default base name. TODO: Eventually improve base name shuffling.
Original Offset: 004E4090
Return Value: n/a
Status: Complete
*/
void __cdecl name_base(uint32_t faction_id, LPSTR name_out, BOOL is_final, BOOL is_sea_base) {
    if (is_sea_base && !text_open(Players[faction_id].filename, "WATERBASES")) {
        uint32_t offset_sea = PlayersData[faction_id].base_sea_name_offset + 1;
        if (offset_sea > 1) {
            uint32_t total;
            for (total = 0; ; total++) {
                text_get();
                if (!strlen(*TextBufferGetPtr) || !_strnicmp(*TextBufferGetPtr, "#END", 4)) {
                    break;
                }
            }
            if (offset_sea <= total) {
                int seed = ((*MapRandSeed + faction_id) & 0xFE) | 1;
                uint32_t loop = 1;
                do {
                    if (seed & 1) {
                        seed ^= 0x170;
                    }
                    seed >>= 1;
                } while (seed >= (int)total || ++loop != offset_sea);
                offset_sea = seed + 1;
            }
        }
        if (!text_open(Players[faction_id].filename, "WATERBASES")) {
            uint32_t count;
            for (count = 0; count < offset_sea; count++) {
                text_get();
                if (!strlen(*TextBufferGetPtr) || !_strnicmp(*TextBufferGetPtr, "#END", 4)) {
                    break;
                }
            }
            if (count == offset_sea) {
                // water base name available
                strncpy_s(name_out, 25, text_item(), 24);
                name_out[23] = 0;
                if (is_final) {
                    PlayersData[faction_id].base_sea_name_offset++;
                }
                text_close();
                return;
            }
        }
        text_close();
    }
    // Land base names or generic
    uint32_t offset = PlayersData[faction_id].base_name_offset + 1;
    sprintf_s(name_out, 25, "%s %d", get_noun(faction_id), offset); // default if names exhausted
    if (is_final) {
        PlayersData[faction_id].base_name_offset++;
    }
    if (!text_open(Players[faction_id].filename, "BASES")) {
        if (offset > 1) {
            uint32_t total;
            for (total = 0; ; total++) {
                text_get();
                if (!strlen(*TextBufferGetPtr) || !_strnicmp(*TextBufferGetPtr, "#END", 4)) {
                    break;
                }
            }
            if (offset <= total) {
                int seed = ((*MapRandSeed + faction_id) & 0xFE) | 1;
                uint32_t loop = 1;
                do {
                    if (seed & 1) {
                        seed ^= 0x170;
                    }
                    seed >>= 1;
                } while (seed >= (int)total || ++loop != offset);
                offset = seed + 1;
            }
        }
        if (!text_open(Players[faction_id].filename, "BASES")) {
            uint32_t count;
            for (count = 0; count < offset; count++) {
                text_get();
                if (!strlen(*TextBufferGetPtr) || !_strnicmp(*TextBufferGetPtr, "#END", 4)) {
                    break;
                }
            }
            if (count != offset) {
                if (text_open("BASENAME", "GENERIC")) {
                    return;
                }
                while (count < offset) {
                    text_get();
                    if (!strlen(*TextBufferGetPtr) || !_strnicmp(*TextBufferGetPtr, "#END", 4)) {
                        return;
                    }
                    count++;
                }
            }
            strncpy_s(name_out, 25, text_item(), 24);
            name_out[23] = 0;
        }
        text_close();
    }
}

/*
Purpose: TBD
Original Offset: 004E4350
Return Value: n/a
Status: Complete - testing
*/
void __cdecl base_mark(uint32_t base_id) {
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    int x_radius = 0;
    int y_radius = 0;
    uint32_t faction_id = Bases[base_id].faction_id_current;
    for (int i = 0; i < RadiusRange[3]; i++) {
        x_radius = xrange(x + RadiusOffsetX[i]);
        y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            if (i < 21) {
                bit_set(x_radius, y_radius, BIT_BASE_RADIUS, true);
                using_set(x_radius, y_radius, faction_id); // rolled this into one conditional check
            } else {
                site_set(x_radius, y_radius, 0);
            }
        }
    }
    for (int f = 0; f < MaxPlayerNum; f++) {
        // TODO: verify if using last radius coordinates is best logic, seems odd
        // It would seem using the Base coordinates would be better suited versus radius
        del_site(f, AI_GOAL_COLONIZE, x_radius, y_radius, 3);
    }
}

/*
Purpose: Calculate the cost factor for the specified faction and resource type. Optional base param.
Original Offset: 004E4430
Return Value: Cost factor
Status: Complete
*/
int __cdecl cost_factor(uint32_t faction_id, uint32_t rsc_type, int base_id) {
    const uint32_t diff_cost_base[] = { 13, 12, 11, 10, 8, 7 };
    uint32_t factor = is_human(faction_id) ? 10 : diff_cost_base[*DiffLevelCurrent]
        - great_satan(FactionRankings[7], false)
        - (!*IsMultiplayerNet && is_human(FactionRankings[7]));
    uint32_t cost_multiplier = rsc_type ? Rules->mineral_cost_multi : Rules->nutrient_cost_multi;
    if (cost_multiplier != 10) {
        factor = (factor * cost_multiplier) / 10;
    }
    if (*MapSizePlanet == 1) {
        factor = (factor * 9) / 10;
    } else if (!*MapSizePlanet) {
        factor = (factor * 8) / 10;
    }
    if (rsc_type) {
        if (rsc_type == RSC_MINERALS) {
            switch (PlayersData[faction_id].soc_effect_pending.industry) {
              case -7:
              case -6:
              case -5:
              case -4:
              case -3:
                return (factor * 13 + 9) / 10;
              case -2:
                return (factor * 6 + 4) / 5;
              case -1:
                return (factor * 11 + 9) / 10;
              case 0:
                return factor;
              case 1:
                return (factor * 9 + 9) / 10;
              case 2:
                return (factor * 4 + 4) / 5;
              case 3:
                return (factor * 7 + 9) / 10;
              case 4:
                return (factor * 3 + 4) / 5;
              default:
                return (factor + 1) / 2;
            }
        } else {
            return factor; // Energy/PSI? Legacy code logic in case cost used these?
        }
    } else { // nutrient
        int growth_factor = PlayersData[faction_id].soc_effect_pending.growth;
        if (base_id >= 0) {
            if (has_fac_built(FAC_CHILDREN_CRECHE, base_id)) {
                growth_factor += 2; // +2 on growth scale
            }
            if (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE) {
                growth_factor += 2;
            }
        }
        growth_factor = range(growth_factor, -2, 5);
        return (factor * (10 - growth_factor) + 9) / 10;
    }
}

/*
Purpose: Determine if the specified base has any restrictions around production item retooling.
Original Offset: 004E4700
Return Value: Fixed value (-1, 0, 1, 2, 3, -70) or productionID
Status: Complete
*/
int __cdecl base_making(int production_id, uint32_t base_id) {
    uint32_t retool = Rules->retool_strictness;
    int skn_off = facility_offset("Skunkworks");
    if ((has_fac_built(FAC_SKUNKWORKS, base_id) // has Skunkworks
        || (Players[Bases[base_id].faction_id_current].rule_flags & RFLAG_FREEPROTO // bug fix
            && skn_off >= 0 
            && has_tech(Facility[skn_off].preq_tech, Bases[base_id].faction_id_current)))
        && retool >= 1) { // don't override if retool strictness is already set to always free (0)
        retool = 1; // Skunkworks or FREEPROTO + prerequisite tech > 'Free in Category'
    }
    if (production_id < 0) { // facility or SP to build
        int queue_id = Bases[base_id].queue_production_id[0]; // current production item
        if (queue_id < 0) { // non-Veh
            queue_id = -queue_id;
            if (queue_id < FacilityRepStart && has_fac_built(queue_id, base_id)) {
                return -1; // facility completed outside normal process, no retool penalty to change
            }
        }
    }
    switch (retool) { // converted into switch to improve readability and performance
      case 0: // Always Free
        return 0;
      case 1: // Free in Category
        if (production_id >= 0) {
            return 0; // Veh
        }
        // SP (1), repeatable facility (2), non-repeatable facility (3)
        return (production_id > -FacilitySPStart) ? (production_id > -FacilityRepStart) + 2 : 1;
      case 2: // Free switching between SPs (default behavior)
        return (production_id <= -FacilitySPStart) ? -FacilitySPStart : production_id;
      case 3: // Never Free
        return production_id;
      default:
        return 1; // should never be reached unless problem with retool value
    }
}

/*
Purpose: Calculate the mineral loss if production were to changed at the specified base.
Original Offset: 004E4810
Return Value: Minerals that would be lost or 0 if not applicable.
Status: Complete
*/
int __cdecl base_lose_minerals(uint32_t base_id, int UNUSED(production_id)) {
    int min_accum;
    if (Rules->retool_pct_pen_prod_chg && is_human(Bases[base_id].faction_id_current)
        && base_making(Bases[base_id].production_id_last, base_id)
        != base_making(Bases[base_id].queue_production_id[0], base_id)
        && (min_accum = Bases[base_id].minerals_accumulated_2, 
            min_accum > (int)Rules->retool_exemption)) {
        return min_accum - (100 - Rules->retool_pct_pen_prod_chg)
            * (min_accum - Rules->retool_exemption) / 100 - Rules->retool_exemption;
    }
    return 0;
}

/*
Purpose: Set or unset the specified base's facility.
Original Offset: 004E48B0
Return Value: n/a
Status: Complete
*/
void __cdecl set_fac(uint32_t facility_id, uint32_t base_id, BOOL set) {
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    if (set) {
        Bases[base_id].facilities_built[offset] |= mask;
    } else {
        Bases[base_id].facilities_built[offset] &= ~mask;
    }
}

/*
Purpose: Check whether the facility audio blurb announcement has played for the faction.
Original Offset: 004E4900
Return Value: Has the facility audio blurb played? true/false
Status: Complete
*/
BOOL __cdecl has_fac_announced(uint32_t faction_id, uint32_t facility_id) {
    if (facility_id > FacilitySPStart) {
        return true;
    }
    int offset, mask;
    bitmask(facility_id, &offset, &mask);
    return (PlayersData[faction_id].facility_announced[offset] & mask) != 0;
}

/*
Purpose: Set or unset whether a particular facility audio blurb has played for specified faction.
Original Offset: 004E4960
Return Value: n/a
Status: Complete
*/
void __cdecl set_fac_announced(uint32_t faction_id, uint32_t facility_id, BOOL set) {
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    if (set) {
        PlayersData[faction_id].facility_announced[offset] |= mask;
    } else {
        PlayersData[faction_id].facility_announced[offset] &= ~mask;
    }
}

/*
Purpose: Determine what unit the specified base should start building 1st then add it to the queue.
Original Offset: 004E4AA0
Return Value: n/a
Status: Complete
*/
void __cdecl base_first(uint32_t base_id) {
    int priority = -1;
    uint32_t proto_id = BSC_SCOUT_PATROL;
    uint32_t faction_id = Bases[base_id].faction_id_current;
    for (int i = 0; i < MaxVehProtoNum; i++) {
        if (veh_avail(i, faction_id, base_id)) {
            if (i < MaxVehProtoFactionNum || VehPrototypes[i].flags & PROTO_TYPED_COMPLETE) {
                int compare = get_proto_defense_rating(i) * 32;
                uint32_t plan = VehPrototypes[i].plan;
                if (plan == PLAN_DEFENSIVE) {
                    compare *= 4;
                } else if (plan == PLAN_COMBAT) {
                    compare *= 3;
                } else if (plan == PLAN_RECONNAISANCE) {
                    compare *= 2;
                }
                if (get_proto_triad(i) != TRIAD_LAND) {
                    compare /= 4;
                }
                compare -= VehPrototypes[i].cost;
                if (compare >= priority) {
                    priority = compare;
                    proto_id = i;
                }
            }
        }
    }
    Bases[base_id].queue_production_id[0] = proto_id;
}

/*
Purpose: Calculate the new unit morale bonus modifier provided by the base and faction for a triad.
Original Offset: 004E6400
Return Value: Morale bonus modifier
Status: Complete
*/
uint32_t __cdecl morale_mod(uint32_t base_id, uint32_t faction_id, uint32_t triad) {
    uint32_t morale_modifier = 0;
    if (triad == TRIAD_LAND) {
        if (has_fac_built(FAC_COMMAND_CENTER, base_id) 
            || has_project(SP_COMMAND_NEXUS, faction_id)) {
            morale_modifier = 2;
        }
    } else if (triad == TRIAD_SEA) {
        if (has_fac_built(FAC_NAVAL_YARD, base_id)
            || has_project(SP_MARITIME_CONTROL_CENTER, faction_id)) {
            morale_modifier = 2;
        }
    } else if (triad == TRIAD_AIR) {
        if (has_fac_built(FAC_AEROSPACE_COMPLEX, base_id)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)) {
            morale_modifier = 2;
        }
    }
    if (has_fac_built(FAC_BIOENHANCEMENT_CENTER, base_id)
        || has_project(SP_CYBORG_FACTORY, faction_id)) {
        morale_modifier += 2;
    }
    if (PlayersData[faction_id].soc_effect_pending.morale < -1) {
        morale_modifier /= 2;
    }
    return morale_modifier;
}

/*
Purpose: Calculate the new native unit lifecycle bonus modifier provided by a base and faction.
Original Offset: 004E65C0
Return Value: Lifecycle bonus
Status: Complete
*/
uint32_t __cdecl breed_mod(uint32_t base_id, uint32_t faction_id) {
    uint32_t lifecycle_modifier = has_project(SP_XENOEMPATYH_DOME, faction_id) ? 1 : 0;
    if (has_project(SP_PHOLUS_MUTAGEN, faction_id)) {
        lifecycle_modifier++;
    }
    if (has_project(SP_VOICE_OF_PLANET, faction_id)) {
        lifecycle_modifier++;
    }
    if (has_fac_built(FAC_CENTAURI_PRESERVE, base_id)) {
        lifecycle_modifier++;
    }
    if (has_fac_built(FAC_TEMPLE_OF_PLANET, base_id)) {
        lifecycle_modifier++;
    }
    if (has_fac_built(FAC_BIOLOGY_LAB, base_id)) {
        lifecycle_modifier++;
    }
    if (has_fac_built(FAC_BIOENHANCEMENT_CENTER, base_id)
        || has_project(SP_CYBORG_FACTORY, faction_id)) {
        lifecycle_modifier++;
    }
    return lifecycle_modifier;
}

/*
Purpose: Calculate the count of lifecycle/psi bonuses that are provided by a base and faction.
Original Offset: 004E6740
Return Value: Native life modifier count
Status: Complete
*/
uint32_t __cdecl worm_mod(uint32_t base_id, uint32_t faction_id) {
    uint32_t worm_modifier_count = breed_mod(base_id, faction_id);
    if (Players[faction_id].rule_psi) {
        worm_modifier_count++;
    }
    if (has_project(SP_DREAM_TWISTER, faction_id)) {
        worm_modifier_count++;
    }
    if (has_project(SP_NEURAL_AMPLIFIER, faction_id)) {
        worm_modifier_count++;
    }
    return worm_modifier_count;
}


/*
Purpose: Address the map tile the terrain yield functions price.
Original Offset: n/a
Return Value: Pointer to map tile
Status: Complete

The three yield functions each inline this, and they inline it with signed
arithmetic - `imul` against y and `sar esi, 1` on x. map_loc() takes unsigned
coordinates and therefore shifts x logically, which differs for a negative x.
Neither form bounds-checks, exactly as the original does not.
*/
static Map *yield_tile(int x, int y) {
    return &(*MapTiles)[(x >> 1) + y * (int)*MapLongitude];
}

/*
Purpose: Identify the faction owning a base standing on the specified tile.
Original Offset: n/a
Return Value: Faction id of the base in the tile, or -1 for none/unowned
Status: Complete

All three yield functions inline this same sequence: the tile's low `bit` flag
selects it, the owner nibble of val2 supplies the id, and an id at or above
MaxPlayerNum - 0xF is the "unoccupied" marker - is rejected.
*/
static int yield_tile_owner(const Map *tile) {
    if (!(tile->bit & BIT_BASE_IN_TILE)) {
        return -1;
    }
    int owner = tile->val2 & 0xF;
    return (owner < MaxPlayerNum) ? owner : -1;
}

/*
Purpose: Calculate the nutrients produced by a single map square.
Original Offset: 004E6E50
Return Value: Nutrients from the square
Status: Complete

`assume_improved` prices the square as if it had already been terraformed; the
name is inferred from the call sites, where StatusWin::draw_status asks for the
same square twice and base_terraform and the AI previews pass 1. Here it
suppresses the fungus path, an improved square having had its fungus cleared.
*/
int __cdecl crop_yield(int faction_id, int base_id, int x, int y,
                       BOOL assume_improved) {
    Map *tile = yield_tile(x, y);
    uint32_t bit = tile->bit;
    BOOL is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;
    BOOL has_base_in_tile = yield_tile_owner(tile) >= 0;
    // Bonus id 1 is the nutrient bonus.
    BOOL has_nutrient_bonus = bonus_at(x, y, 0) == 1;
    uint32_t bit2 = tile->bit2;
    // Nutrient landmarks: Jungle counts on land, Freshwater Sea on ocean.
    BOOL landmark_bonus =
        (((bit2 & (BIT2_UNK_80000000 | BIT2_JUNGLE)) == BIT2_JUNGLE)
            && !is_ocean_tile)
        || ((bit2 & BIT2_FRESH) && is_ocean_tile);
    int crop;

    if (has_base_in_tile) {
        crop = ResourceInfo[RSCINFO_BASE_SQ].nutrients;
        if (has_nutrient_bonus) {
            crop *= 2;
        }
        if (landmark_bonus) {
            crop++;
        }
        if (base_id < 0) {
            return crop; // the original returns here rather than reaching the tail
        }
        if (has_fac_built(FAC_RECYCLING_TANKS, base_id)
            || has_fac_built(FAC_PRESSURE_DOME, base_id)) {
            crop += ResourceInfo[RSCINFO_RECYCLING_TANKS].nutrients;
        }
    } else if (bit & BIT_THERMAL_BORE) {
        crop = ResourceInfo[RSCINFO_BOREHOLE_SQ].nutrients;
        // Faithful: 0x004E6FCB is `test esi, esi` on the borehole value that was
        // just loaded, NOT on the resource bonus flag. mine_yield's structurally
        // parallel arm tests the flag. Transcribed literally.
        if (crop) {
            crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
        }
    } else if (bit & BIT_MONOLITH) {
        crop = ResourceInfo[RSCINFO_MONOLITH].nutrients;
        if (has_nutrient_bonus) {
            crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
        }
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)
            && PlayersData[faction_id].soc_effect_pending.planet > 0) {
            crop++;
        }
    } else if ((bit & BIT_FUNGUS) && !assume_improved
               && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
        int planet = PlayersData[faction_id].soc_effect_pending.planet;
        int planet_mod = (planet < -3) ? -3 : ((planet > 0) ? 0 : planet);
        crop = range(PlayersData[faction_id].tech_fungus_nutrient + planet_mod,
                     0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id) && planet > 0) {
            crop++;
        }
    } else {
        if (is_ocean_tile) {
            crop = ResourceInfo[RSCINFO_OCEAN_SQ].nutrients;
            if (has_nutrient_bonus) {
                crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
            }
            if (landmark_bonus) {
                crop++;
            }
            if ((tile->climate & 0xE0) == ALT_BIT_OCEAN_SHELF
                || *ExpansionEnabled) {
                // A kelp farm.
                if (bit & BIT_FARM) {
                    crop += ResourceInfo[RSCINFO_IMPROVED_SEA].nutrients;
                    // Legacy: unguarded for base_id < 0, exactly as 0x004E7150.
                    // Bases[(uint32_t)-1] and Bases[-1] are the same address on
                    // 32-bit x86, so this reproduces the original's read.
                    if (has_fac_built(FAC_AQUAFARM, (uint32_t)base_id)) {
                        crop++;
                    }
                }
                if ((bit & BIT_MINE) && crop > 1) {
                    crop += Rules->tgl_nutrient_effect_with_mine;
                }
            }
        } else if (bit & BIT_FOREST) {
            crop = ResourceInfo[RSCINFO_FOREST_SQ].nutrients;
            if (has_nutrient_bonus) {
                crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
            }
            if (base_id >= 0) {
                if (has_fac_built(FAC_HYBRID_FOREST, base_id)) {
                    crop++;
                }
                if (has_fac_built(FAC_TREE_FARM, base_id)) {
                    crop++;
                }
            }
            if (landmark_bonus) {
                crop++;
            }
        } else {
            int rockiness = tile->val3 >> 6;
            // Rainfall level: arid / moist / rainy.
            crop = (tile->climate >> 3) & 3;
            if (rockiness > ROCKINESS_ROLLING) {
                crop = 0;
            }
            if (has_nutrient_bonus) {
                crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
            }
            if (landmark_bonus) {
                crop++;
            }
            if (crop < 0) {
                crop = 0;
            }
            if ((bit & BIT_FARM) && rockiness <= ROCKINESS_ROLLING) {
                crop += ResourceInfo[RSCINFO_IMPROVED_LAND].nutrients;
            }
            if ((bit & BIT_MINE) && crop > 1) {
                crop += Rules->tgl_nutrient_effect_with_mine;
            }
        }
        if (bit & BIT_SOIL_ENRICHER) {
            crop += crop / 2;
        }
        BOOL has_condenser = (bit & BIT_CONDENSER) != 0;
        if (has_condenser) {
            crop += crop / 2;
        }
        // Only the ocean and land arms reach the restriction; the base,
        // borehole, monolith and fungus arms jump past it to the event tail.
        if (crop > 2 && !has_tech(Rules->tech_three_nutrients_sqr, faction_id)
            && !has_nutrient_bonus && !has_condenser) {
            *TileYieldRestricted += crop - 2;
            crop = 2;
        }
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_BUMPER) {
            crop++;
        }
        if ((event & BEVENT_FAMINE) && crop) {
            crop--;
        }
    }
    return crop;
}

/*
Purpose: Calculate the minerals produced by a single map square.
Original Offset: 004E7310
Return Value: Minerals from the square
Status: Complete

`assume_improved` stands in for a mine or borehole on land and a mining platform
at sea, and suppresses the roadless-mine mineral limit.
*/
int __cdecl mine_yield(int faction_id, int base_id, int x, int y,
                       BOOL assume_improved) {
    Map *tile = yield_tile(x, y);
    uint32_t bit = tile->bit;
    uint32_t bit2 = tile->bit2;
    // Landmark sequence code, signed exactly as `sar edi, 0x18`. Bit 31 of bit2
    // is required clear by every test below, so the sign only decides branches
    // that are not taken.
    int code = (int)bit2 >> 24;
    BOOL is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;
    // Bonus id 2 is the mineral bonus.
    BOOL has_mineral_bonus = bonus_at(x, y, 0) == 2;
    BOOL landmark_bonus =
        (((bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO)
            && code < 9)
        || (((bit2 & (BIT2_UNK_80000000 | BIT2_CRATER)) == BIT2_CRATER)
            && code < 9)
        || (((bit2 & (BIT2_UNK_80000000 | BIT2_FOSSIL)) == BIT2_FOSSIL)
            && code < 6)
        || ((bit2 & (BIT2_UNK_80000000 | BIT2_CANYON)) == BIT2_CANYON);
    int mineral = (has_mineral_bonus ? ResourceInfo[RSCINFO_BONUS_SQ].minerals
                                     : 0)
                  + (landmark_bonus ? 1 : 0);
    BOOL restrict_yield = false;

    if (yield_tile_owner(tile) >= 0) {
        mineral += ResourceInfo[RSCINFO_BASE_SQ].minerals;
        if (base_id < 0) {
            return mineral; // the original returns here rather than reaching the tail
        }
        if (has_fac_built(FAC_RECYCLING_TANKS, base_id)
            || has_fac_built(FAC_PRESSURE_DOME, base_id)) {
            mineral += ResourceInfo[RSCINFO_RECYCLING_TANKS].minerals;
        }
    } else if (bit & BIT_MONOLITH) {
        // Assignment, not addition: the bonus and landmark prefix is discarded.
        mineral = ResourceInfo[RSCINFO_MONOLITH].minerals;
        if (has_mineral_bonus) {
            mineral += ResourceInfo[RSCINFO_BONUS_SQ].minerals;
        }
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)
            && PlayersData[faction_id].soc_effect_pending.planet > 1) {
            mineral++;
        }
    } else if (bit & BIT_THERMAL_BORE) {
        mineral = ResourceInfo[RSCINFO_BOREHOLE_SQ].minerals;
        if (has_mineral_bonus) {
            mineral += ResourceInfo[RSCINFO_BONUS_SQ].minerals;
        }
        // Unlike crop_yield's borehole arm, this one does reach the restriction.
        restrict_yield = true;
    } else if ((bit & BIT_FUNGUS)
               && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
        // Note there is no assume_improved suppressor here; crop_yield has one.
        int planet = PlayersData[faction_id].soc_effect_pending.planet;
        int planet_mod = (planet < -3) ? -3 : ((planet > 0) ? 0 : planet);
        mineral = range(PlayersData[faction_id].tech_fungus_mineral + planet_mod,
                        0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id) && planet > 1) {
            mineral++;
        }
    } else if (is_ocean_tile) {
        restrict_yield = true;
        mineral += ResourceInfo[RSCINFO_OCEAN_SQ].minerals;
        // Legacy: the aquatic bonus keys off the faction owning BASE_ID, not off
        // faction_id, and is unguarded for base_id < 0 (0x004E75E2).
        int base_owner = Bases[base_id].faction_id_current;
        if ((Players[base_owner].rule_flags & RFLAG_AQUATIC)
            && (tile->climate & 0xE0) == ALT_BIT_OCEAN_SHELF) {
            mineral++;
        }
        if ((tile->climate & 0xE0) == ALT_BIT_OCEAN_SHELF || *ExpansionEnabled) {
            if ((bit & BIT_MINE) || assume_improved) {
                mineral += ResourceInfo[RSCINFO_IMPROVED_SEA].minerals;
                if (has_tech(Rules->tech_mining_platform_bonus, faction_id)) {
                    mineral++;
                }
                // Legacy: unguarded for base_id < 0, exactly as 0x004E765F.
                if (has_fac_built(FAC_SUBSEA_TRUNKLINE, (uint32_t)base_id)) {
                    mineral++;
                }
            }
        }
    } else {
        restrict_yield = true;
        int rockiness = tile->val3 >> 6;
        if (bit & BIT_FOREST) {
            mineral += ResourceInfo[RSCINFO_FOREST_SQ].minerals;
        } else if ((bit & (BIT_MINE | BIT_THERMAL_BORE)) || assume_improved) {
            mineral += rockiness;
            int extra = rockiness ? rockiness : 1;
            if (has_mineral_bonus || landmark_bonus) {
                extra++;
            }
            if (!(bit & BIT_ROAD) && !assume_improved) {
                int limit = (int)Rules->limit_mineral_mine_sans_road;
                if (extra > limit) {
                    *TileYieldRestricted += extra - limit;
                    extra = limit;
                }
            }
            mineral += extra;
        } else if (rockiness) {
            mineral++;
        }
    }

    if (restrict_yield && mineral > 2
        && !has_tech(Rules->tech_three_minerals_sqr, faction_id)
        && !has_mineral_bonus) {
        *TileYieldRestricted += mineral - 2;
        mineral = 2;
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_INDUSTRY) {
            mineral++;
        }
        if ((event & BEVENT_BUST) && mineral) {
            mineral--;
        }
    }
    return mineral;
}

/*
Purpose: Calculate the energy produced by a single map square.
Original Offset: 004E7750
Return Value: Energy from the square
Status: Complete

`assume_improved` stands in for a solar collector on land and a tidal harness at
sea.

The fungus arm deliberately skips the whole shared tail - river, resource bonus,
landmark, Merchant Exchange, economy and the clip to 2 - by jumping from
0x004E7AD2 straight to the base event tail at 0x004E7D4B. That is what the bytes
do, not a transcription slip.
*/
int __cdecl energy_yield(int faction_id, int base_id, int x, int y,
                         BOOL assume_improved) {
    Map *tile = yield_tile(x, y);
    uint32_t bit = tile->bit;
    uint32_t bit2 = tile->bit2;
    int code = (int)bit2 >> 24;
    BOOL is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;
    BOOL has_base_in_tile = yield_tile_owner(tile) >= 0;
    // Bonus id 3 is the energy bonus.
    BOOL has_energy_bonus = bonus_at(x, y, 0) == 3;
    BOOL landmark_bonus =
        (((bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO)
            && code < 9)
        || ((bit2 & (BIT2_UNK_80000000 | BIT2_URANIUM)) == BIT2_URANIUM)
        || ((bit2 & (BIT2_UNK_80000000 | BIT2_GEOTHERMAL)) == BIT2_GEOTHERMAL)
        || ((bit2 & (BIT2_UNK_80000000 | BIT2_RIDGE)) == BIT2_RIDGE);
    int energy = 0;
    BOOL skip_shared_tail = false;

    if (has_base_in_tile) {
        energy = ResourceInfo[RSCINFO_BASE_SQ].energy;
        // Legacy: read BEFORE the base_id sign check at 0x004E7896, so a base_id
        // of -1 reads one Base entry below the array, exactly as the original.
        if (has_fac_built(FAC_HEADQUARTERS, (uint32_t)base_id)) {
            energy++;
        }
        int golden_age = 0;
        if (base_id >= 0) {
            if (has_fac_built(FAC_RECYCLING_TANKS, base_id)
                || has_fac_built(FAC_PRESSURE_DOME, base_id)) {
                energy += ResourceInfo[RSCINFO_RECYCLING_TANKS].energy;
            }
            if (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE) {
                golden_age = 1;
            }
        }
        int economy = PlayersData[faction_id].soc_effect_pending.economy
                      + golden_age;
        if (economy < 0) {
            // The original asks for Headquarters again here rather than reusing
            // the answer above.
            if (!has_fac_built(FAC_HEADQUARTERS, (uint32_t)base_id)
                || economy < -1
                || PlayersData[faction_id].current_num_bases == 1u) {
                economy++;
            }
        } else if (economy > 1) {
            if (economy > 4) {
                economy = 4;
            }
            economy = economy * 2 - 4;
        }
        energy += economy;
        if (*GovernorFaction == faction_id) {
            energy++;
        }
    } else if (bit & BIT_MONOLITH) {
        energy = ResourceInfo[RSCINFO_MONOLITH].energy;
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)) {
            int planet = PlayersData[faction_id].soc_effect_pending.planet;
            if (planet >= 0) {
                energy++;
            }
            if (planet >= 3) {
                energy++;
            }
        }
    } else if (bit & BIT_THERMAL_BORE) {
        energy = ResourceInfo[RSCINFO_BOREHOLE_SQ].energy;
    } else if ((bit & BIT_FUNGUS)
               && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
        int planet = PlayersData[faction_id].soc_effect_pending.planet;
        int planet_mod = (planet < -3) ? -3 : ((planet > 0) ? 0 : planet);
        energy = range(PlayersData[faction_id].tech_fungus_energy + planet_mod,
                       0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)) {
            if (planet >= 0) {
                energy++;
            }
            if (planet >= 3) {
                energy++;
            }
        }
        skip_shared_tail = true;
    } else if (is_ocean_tile) {
        if ((tile->climate & 0xE0) == ALT_BIT_OCEAN_SHELF || *ExpansionEnabled) {
            energy = ResourceInfo[RSCINFO_OCEAN_SQ].energy;
            if ((bit & BIT_SOLAR_TIDAL) || assume_improved) {
                energy += ResourceInfo[RSCINFO_IMPROVED_SEA].energy;
                // Legacy: unguarded for base_id < 0, exactly as 0x004E7B19.
                if (has_fac_built(FAC_THERMOCLINE_TRANSDUCER,
                                  (uint32_t)base_id)) {
                    energy++;
                }
            }
        }
        if (bit & BIT_FOREST) {
            energy--;
        }
    } else if (bit & BIT_FOREST) {
        energy = ResourceInfo[RSCINFO_FOREST_SQ].energy;
        if (base_id >= 0 && has_fac_built(FAC_HYBRID_FOREST, base_id)) {
            energy++;
        }
    } else {
        // Altitude levels above 2.
        int solar = (tile->climate >> 5) - 2;
        if (solar < 0) {
            solar = 0;
        }
        if ((bit & (BIT_SOLAR_TIDAL | BIT_ECH_MIRROR | BIT_THERMAL_BORE))
            || assume_improved) {
            energy = solar;
        }
        if (bit & BIT_SOLAR_TIDAL) {
            // Each adjacent echelon mirror belonging to the current base's
            // faction reflects one more energy onto this collector. The x wrap
            // is xrange() inlined, except that the original masks MapIsFlat with
            // 1 rather than testing it for nonzero.
            BOOL is_flat = *MapIsFlat & 1;
            int longitude_bounds = *MapLongitudeBounds;
            for (int i = 0; i < 8; i++) {
                int x_adj = x + RadiusBaseX[i];
                if (!is_flat) {
                    if (x_adj < 0) {
                        x_adj += longitude_bounds;
                    } else if (x_adj >= longitude_bounds) {
                        x_adj -= longitude_bounds;
                    }
                }
                int y_adj = y + RadiusBaseY[i];
                if (y_adj < 0 || y_adj >= *MapLatitudeBounds) {
                    continue;
                }
                if (x_adj < 0 || x_adj >= longitude_bounds) {
                    continue;
                }
                Map *adjacent = yield_tile(x_adj, y_adj);
                if (!(adjacent->bit & BIT_ECH_MIRROR)) {
                    continue;
                }
                if (adjacent->territory
                    != (int)(*BaseCurrent)->faction_id_current) {
                    continue;
                }
                energy++;
            }
        }
    }

    if (!skip_shared_tail) {
        if ((bit & BIT_RIVER) && !is_ocean_tile) {
            energy++;
        }
        if (has_energy_bonus) {
            energy += ResourceInfo[RSCINFO_BONUS_SQ].energy;
        }
        if (landmark_bonus) {
            energy++;
        }
        int golden_age = 0;
        if (base_id >= 0) {
            if (SecretProject->merchant_exchange == base_id) {
                energy++;
            }
            if (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE) {
                golden_age = 1;
            }
        }
        if (PlayersData[faction_id].soc_effect_pending.economy + golden_age
            >= 2) {
            energy++;
        }
        if (has_base_in_tile) {
            energy = range(energy, 0, 99);
            *BaseSquareEnergy = energy;
        } else if (energy > 2
                   && !has_tech(Rules->tech_three_energy_sqr, faction_id)
                   && !has_energy_bonus) {
            *TileYieldRestricted += energy - 2;
            energy = 2;
        }
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_HEAT_WAVE) {
            energy++;
        }
        if ((event & BEVENT_CLOUD_COVER) && energy) {
            energy--;
        }
    }
    if (*DustCloudDuration && energy) {
        energy--;
    }
    int event_active = *UnkGlobal0093A934 ? (*GlobalEnergyEventState & 2)
                                          : (*GlobalEnergyEventState & 1);
    return event_active ? energy * 3 : energy;
}

/*
Purpose: Tally what the current base's forces cost it: the resources its supply
         convoys move in and out, the units it supports, the minerals their
         maintenance takes, and the pacifism drones they cause.
Original Offset: 004E9550
Return Value: n/a
Status: Complete

One pass over every Veh of the base's faction, splitting on whether the unit is
homed here. The five accumulators this zeroes are read by base_nutrient
(0x004E9B70), base_minerals (0x004E9CB0) and base_psych, which is why the whole
tally has to be finished before any of them run.

Two of the eight `free_support` entries are not `support + 2`: -4 and -3 both
give nothing, and +3 gives max(4, population), so the table is built rather than
computed. It is indexed by the SUPPORT rating offset by four, -4 at [0].

The original recomputes the Brood Pit police adjustment a second time in the
`police != -3` arm, with a second inlined bitmask() over the same facility and
the same base. Nothing between the two writes facilities_built, so it is written
once here. The three has_abil(ABL_CLEAN_REACTOR) calls are NOT collapsed: the
first one gates the whole support section, so the second and third can only ever
see false, but has_abil() logs on a negative prototype id and the call count is
therefore observable.
*/
void __cdecl base_support() {
    int faction_id = (*BaseCurrent)->faction_id_current;
    for (int rsc = 0; rsc < 4; rsc++) {
        BaseCurrentConvoyTo[rsc] = 0;
        BaseCurrentConvoyFrom[rsc] = 0;
    }
    *BaseCurrentVehPacifismCount = 0;
    *BaseCurrentForcesMaintCount = 0;
    *BaseCurrentForcesSupported = 0;
    *BaseCurrentForcesMaintCost = 0;
    int free_support[8]; // units supported for free, by SUPPORT rating + 4
    for (int support = -4; support <= 3; support++) {
        int free_units;
        if (support >= 3) {
            int pop_size = (*BaseCurrent)->population_size;
            free_units = (pop_size > 4) ? pop_size : 4;
        } else if (support < -2) {
            free_units = 0;
        } else if (support == -2) {
            free_units = 1;
        } else {
            free_units = support + 2;
        }
        free_support[support + 4] = free_units;
    }
    for (int veh_id = 0; veh_id < *VehCurrentCount; veh_id++) {
        Veh *veh = &Vehs[veh_id];
        if (veh->faction_id != faction_id) {
            continue;
        }
        int proto_id = veh->proto_id;
        if (veh->home_base_id != *BaseIDCurrentSelected) {
            /*
            * A convoy homed at another base but standing on this base's own
            * tile counts one, not a yield; base_nutrient() adds
            * BaseCurrentConvoyTo to the base's intake all the same.
            */
            if (VehPrototypes[proto_id].plan == PLAN_SUPPLY_CONVOY
                && veh->x == (*BaseCurrent)->x && veh->y == (*BaseCurrent)->y
                && veh->order == ORDER_CONVOY && veh->home_base_id >= 0) {
                BaseCurrentConvoyTo[veh->order_auto_type]++;
            }
            continue;
        }
        if (VehPrototypes[proto_id].plan == PLAN_SUPPLY_CONVOY
            && yield_tile_owner(yield_tile(veh->x, veh->y)) < 0
            && veh->order == ORDER_CONVOY) {
            int convoyed;
            switch (veh->order_auto_type) {
              case RSC_NUTRIENTS:
                convoyed = crop_yield(faction_id, *BaseIDCurrentSelected, veh->x, veh->y,
                                      false);
                break;
              case RSC_MINERALS:
                convoyed = mine_yield(faction_id, *BaseIDCurrentSelected, veh->x, veh->y,
                                      false);
                break;
              case RSC_ENERGY:
                convoyed = energy_yield(faction_id, *BaseIDCurrentSelected, veh->x, veh->y,
                                        false);
                break;
              case RSC_PSI:
                *TileYieldRestricted = 0;
                convoyed = 0;
                break;
              default:
                convoyed = 0;
                break;
            }
            BaseCurrentConvoyTo[veh->order_auto_type] += convoyed;
        }
        /*
        * The AI's offensive strength tally. A negative offense rating is a psi
        * weapon: those count only on a custom prototype, never on one of the
        * predefined native life forms.
        */
        int offense = Weapon[VehPrototypes[proto_id].weapon_id].offense_rating;
        if (offense && (offense > 0 || proto_id >= MaxVehProtoFactionNum)) {
            uint32_t tally_plan = VehPrototypes[proto_id].plan;
            if (tally_plan != PLAN_RECONNAISANCE && tally_plan != PLAN_PLANET_BUSTER) {
                PlayersData[faction_id].unk_48++;
                if (tally_plan == PLAN_OFFENSIVE || tally_plan == PLAN_COMBAT) {
                    PlayersData[faction_id].unk_48++;
                }
            }
        }
        veh->state &= ~(VSTATE_REQUIRES_SUPPORT | VSTATE_PACIFISM_DRONE
                        | VSTATE_PACIFISM_FREE_SKIP);
        uint32_t plan = VehPrototypes[proto_id].plan;
        // A convoy of ours standing in any base is charged as consumption.
        if (plan == PLAN_SUPPLY_CONVOY && veh->order == ORDER_CONVOY
            && yield_tile_owner(yield_tile(veh->x, veh->y)) >= 0) {
            BaseCurrentConvoyFrom[veh->order_auto_type]++;
        }
        if (plan > PLAN_TERRAFORMING) {
            continue; // convoys and above are never supported
        }
        if (!has_abil(proto_id, ABL_CLEAN_REACTOR)) {
            /*
            * Native life in fungus is supported for free. The prototype test is
            * a psi weapon on a predefined prototype, or prototype 15 - the Sea
            * Lurk, whose weapon is not psi.
            */
            BOOL free_in_fungus = false;
            if (proto_id < MaxVehProtoFactionNum) {
                if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0
                    || proto_id == 15) {
                    Map *tile = yield_tile(veh->x, veh->y);
                    if ((tile->bit & BIT_FUNGUS)
                        && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
                        free_in_fungus = true;
                    }
                }
            }
            if (!free_in_fungus) {
                (*BaseCurrentForcesSupported)++;
                int support = PlayersData[faction_id].soc_effect_pending.support;
                if ((int)*BaseCurrentForcesSupported
                        > free_support[range(support + 4, 0, 7)]
                    && !has_abil(proto_id, ABL_CLEAN_REACTOR)) {
                    (*BaseCurrentForcesMaintCount)++;
                    veh->state |= VSTATE_REQUIRES_SUPPORT;
                    *BaseCurrentForcesMaintCost += (support <= -4) ? 2 : 1;
                }
            }
            /*
            * What this unit would cost at each of the eight SUPPORT ratings,
            * for social_ai() to price a social engineering change with. The
            * marginal cost triples once it passes the base's mineral intake and
            * doubles once it passes half of it.
            */
            if (*BaseUpkeepStage == 1) {
                for (int support = -4; support <= 3; support++) {
                    if ((int)*BaseCurrentForcesSupported <= free_support[support + 4]) {
                        continue;
                    }
                    int cost = has_abil(proto_id, ABL_CLEAN_REACTOR)
                        ? 0 : ((support <= -4) ? 2 : 1);
                    int minerals = (*BaseCurrent)->mineral_intake_2;
                    int half_minerals = minerals / 2;
                    if (cost >= minerals) {
                        cost += (cost - half_minerals) * 2;
                    } else if (cost > half_minerals) {
                        cost = cost * 2 - half_minerals;
                    }
                    PlayersData[faction_id].unk_38[support + 4] += cost;
                }
            }
        }
        /*
        * Pacifism. Only armed units count, and only away from home - except for
        * air units, which count anywhere unless they are interceptors.
        */
        if (!Weapon[VehPrototypes[proto_id].weapon_id].offense_rating) {
            continue;
        }
        if (yield_tile_owner(yield_tile(veh->x, veh->y)) >= 0
            || whose_territory(faction_id, veh->x, veh->y, NULL, false) == faction_id) {
            if (Chassis[VehPrototypes[proto_id].chassis_id].triad != TRIAD_AIR
                || VehPrototypes[proto_id].plan == PLAN_AIR_SUPERIORITY) {
                continue;
            }
        }
        (*BaseCurrentVehPacifismCount)++;
        int police = PlayersData[faction_id].soc_effect_pending.police
            + (has_fac_built(FAC_BROOD_PIT, *BaseIDCurrentSelected) ? 2 : 0);
        if (police == -3) {
            veh->state |= (*BaseCurrentVehPacifismCount == 1) ? VSTATE_PACIFISM_FREE_SKIP
                                                              : VSTATE_PACIFISM_DRONE;
        } else if (police < -3) {
            veh->state |= VSTATE_PACIFISM_DRONE;
        }
    }
}

/*
Purpose: Calculate nutrients and growth for the current base.
Original Offset: 004E9B70
Return Value: n/a
Status: Complete
*/
void __cdecl base_nutrient() {
    uint32_t faction_id = (*BaseCurrent)->faction_id_current;
    *BaseCurrentGrowthRate = PlayersData[faction_id].soc_effect_pending.growth;
    if (has_fac_built(FAC_CHILDREN_CRECHE, *BaseIDCurrentSelected)) {
        *BaseCurrentGrowthRate += 2; // +2 on growth scale
    }
    if ((*BaseCurrent)->state & BSTATE_GOLDEN_AGE_ACTIVE) {
        *BaseCurrentGrowthRate += 2;
    }
    (*BaseCurrent)->nutrient_intake_2 += BaseCurrentConvoyTo[RSC_NUTRIENTS];
    (*BaseCurrent)->nutrient_consumption = BaseCurrentConvoyFrom[RSC_NUTRIENTS]
        + (*BaseCurrent)->population_size * Rules->nutrient_req_citizen;
    (*BaseCurrent)->nutrient_surplus = (*BaseCurrent)->nutrient_intake_2
        - (*BaseCurrent)->nutrient_consumption;
    if ((*BaseCurrent)->nutrient_surplus >= 0) {
        if ((*BaseCurrent)->nutrients_accumulated < 0) {
            (*BaseCurrent)->nutrients_accumulated = 0;
        }
    } else if (!((*BaseCurrent)->nutrients_accumulated)) {
        (*BaseCurrent)->nutrients_accumulated = -1;
    }
    if (*BaseUpkeepStage == 1) {
        PlayersData[faction_id].nutrient_surplus_total
            += range((*BaseCurrent)->nutrient_surplus, 0, 99);
    }
}

/*
Purpose: Calculate minerals and ecological damage for the current base.
Original Offset: 004E9CB0
Return Value: n/a
Status: Complete
*/
void __cdecl base_minerals() {
    uint32_t faction_id = (*BaseCurrent)->faction_id_current;
    (*BaseCurrent)->mineral_intake_2 += BaseCurrentConvoyTo[RSC_MINERALS];
    uint32_t mineral_bonus = (has_fac_built(FAC_QUANTUM_CONVERTER, *BaseIDCurrentSelected)
        || has_project(SP_SINGULARITY_INDUCTOR, faction_id)) ? 1 : 0;
    if (has_fac_built(FAC_ROBOTIC_ASSEMBLY_PLANT, *BaseIDCurrentSelected)) {
        mineral_bonus++;
    }
    if (has_fac_built(FAC_GENEJACK_FACTORY, *BaseIDCurrentSelected)) {
        mineral_bonus++;
    }
    if (has_fac_built(FAC_NANOREPLICATOR, *BaseIDCurrentSelected)) {
        mineral_bonus++;
    }
    if (has_project(SP_BULK_MATTER_TRANSMITTER, faction_id)) {
        mineral_bonus++;
    }
    (*BaseCurrent)->mineral_intake_2 = ((*BaseCurrent)->mineral_intake_2 * (mineral_bonus + 2)) / 2;
    (*BaseCurrent)->mineral_consumption = *BaseCurrentForcesMaintCost
        + BaseCurrentConvoyFrom[RSC_MINERALS];
    (*BaseCurrent)->mineral_surplus = (*BaseCurrent)->mineral_intake_2
        - (*BaseCurrent)->mineral_consumption;
    (*BaseCurrent)->mineral_inefficiency = 0; // ?
    (*BaseCurrent)->mineral_surplus -= (*BaseCurrent)->mineral_inefficiency; // ?
    (*BaseCurrent)->mineral_surplus_final = (*BaseCurrent)->mineral_surplus;
    (*BaseCurrent)->eco_damage /= 8;
    int planet_eco_factor = PlayersData[faction_id].planet_ecology + 16;
    if ((*BaseCurrent)->eco_damage > 0) {
        int planet_modifier = (*BaseCurrent)->eco_damage;
        if ((*BaseCurrent)->eco_damage >= planet_eco_factor) {
            planet_modifier = planet_eco_factor;
        }
        planet_eco_factor -= planet_modifier;
        (*BaseCurrent)->eco_damage -= planet_modifier;
    }
    int eco_dmg_reduction = (has_fac_built(FAC_NANOREPLICATOR, *BaseIDCurrentSelected)
        || has_project(SP_SINGULARITY_INDUCTOR, faction_id)) ? 2 : 1;
    if (has_fac_built(FAC_CENTAURI_PRESERVE, *BaseIDCurrentSelected)) {
        eco_dmg_reduction++;
    }
    if (has_fac_built(FAC_TEMPLE_OF_PLANET, *BaseIDCurrentSelected)) {
        eco_dmg_reduction++;
    }
    if (has_project(SP_PHOLUS_MUTAGEN, faction_id)) {
        eco_dmg_reduction++;
    }
    (*BaseCurrent)->eco_damage += ((*BaseCurrent)->mineral_intake_2 - planet_eco_factor
        - range(PlayersData[faction_id].satellites_mineral, 0, (*BaseCurrent)->population_size))
        / eco_dmg_reduction;
    if (is_human(faction_id)) {
        (*BaseCurrent)->eco_damage += ((PlayersData[faction_id].major_atrocities
            + TectonicDetonationCount[faction_id]) * 5) / (range(*MapSeaLevel, 0, 100)
                / range(WorldBuilder->sea_level_rises, 1, 100) + eco_dmg_reduction);
    }
    if ((*BaseCurrent)->eco_damage < 0) {
        (*BaseCurrent)->eco_damage = 0;
    }
    if (ascending(faction_id) && *GameRules & RULES_VICTORY_TRANSCENDENCE) {
        (*BaseCurrent)->eco_damage *= 2;
    }
    if (*GameState & STATE_PERIHELION_ACTIVE) {
        (*BaseCurrent)->eco_damage *= 2;
    }
    uint32_t diff_factor;
    if (is_human(faction_id)) {
        int diff_lvl = PlayersData[faction_id].diff_level;
        diff_factor = !diff_lvl ? DLVL_TALENT
            : ((diff_lvl <= DLVL_LIBRARIAN) ? DLVL_LIBRARIAN : DLVL_TRANSCEND);
    } else {
        diff_factor = range(6 - *DiffLevelCurrent, DLVL_SPECIALIST, DLVL_LIBRARIAN);
    }
    (*BaseCurrent)->eco_damage = ((PlayersData[faction_id].tech_ranking
        - PlayersData[faction_id].theory_of_everything)
        * (3 - range(PlayersData[faction_id].soc_effect_pending.planet, -3, 2))
        * (*MapNativeLifeForms + 1) * (*BaseCurrent)->eco_damage * diff_factor) / 6;
    (*BaseCurrent)->eco_damage = ((*BaseCurrent)->eco_damage + 50) / 100;
    int queue_id;
    if (has_project(SP_SPACE_ELEVATOR, faction_id) // orbital facilities double mineral prod rate
        && (queue_id = (*BaseCurrent)->queue_production_id[0], queue_id == -FAC_SKY_HYDRO_LAB
            || queue_id == -FAC_NESSUS_MINING_STATION || queue_id == -FAC_ORBITAL_POWER_TRANS
            || queue_id == -FAC_ORBITAL_DEFENSE_POD)) {
        (*BaseCurrent)->mineral_intake_2 *= 2;
        (*BaseCurrent)->mineral_surplus = // doesn't update mineral_surplus_final?
            (*BaseCurrent)->mineral_intake_2 - (*BaseCurrent)->mineral_consumption;
    }
}

/*
Purpose: Calculate the current base's energy loss/inefficiency for an amount of energy.
Original Offset: 004EA1F0
Return Value: Amount of energy inefficiency
Status: Complete
*/
uint32_t __cdecl black_market(int energy) {
    if (energy <= 0) {
        return 0;
    }
    uint32_t faction_id = (*BaseCurrent)->faction_id_current;
    int x = (*BaseCurrent)->x;
    int y = (*BaseCurrent)->y;
    int dist_hq = 999;
    for (int i = 0; i < *BaseCurrentCount; i++) { // modified version of vulnerable()
        if (Bases[i].faction_id_current == faction_id && has_fac_built(FAC_HEADQUARTERS, i)) {
            int dist = vector_dist(Bases[i].x, Bases[i].y, x, y);
            if (dist < dist_hq) {
                dist_hq = dist;
            }
        }
    }
    if (dist_hq == 999) {
        dist_hq = 16;
    } else if (dist_hq == 0) {
        return 0;
    }
    BOOL has_creche = has_fac_built(FAC_CHILDREN_CRECHE, *BaseIDCurrentSelected);
    if (*BaseUpkeepStage == 1) {
        for (int i = 0, j = 0; i >= -64; i -= 8, j++) {
            int ineff;
            int factor;
            if (has_creche) {
                ineff = j - 2; // +2 on efficiency scale
                factor = i + 16; // ?
            } else {
                ineff = j;
                factor = i;
            }
            if (ineff >= 8) {
                PlayersData[faction_id].unk_46[j] += energy;
            } else {
                PlayersData[faction_id].unk_46[j] += energy * dist_hq / (factor + 64);
            }
        }
    }
    int ineffciency = 4 - PlayersData[faction_id].soc_effect_pending.efficiency;
    if (has_creche) {
        ineffciency -= 2; // +2 on efficiency scale
    }
    if (ineffciency >= 8) {
        return energy;
    }
    return range(energy * dist_hq / ((8 - ineffciency) * 8), 0, energy);
}

/*
Purpose: Calculate the current base's psych values.
Original Offset: 004EA540
Return Value: n/a
Status: WIP
*/
void __cdecl base_psych() {
    uint32_t faction_id = (*BaseCurrent)->faction_id_current;
    int pop_size = (*BaseCurrent)->population_size;
    int drones_base;
    int talents_base;
    int diff_lvl = is_human(faction_id) ? PlayersData[faction_id].diff_level : DLVL_LIBRARIAN;
    psych_check(faction_id, &drones_base, &talents_base);
    if (talents_base) {
        talents_base = range((*BaseIDCurrentSelected % talents_base
            + PlayersData[faction_id].current_num_bases - talents_base) 
            / talents_base, 0, pop_size);
    }
    int psych_val = range(pop_size - drones_base, 0, pop_size);
    psych_val += range(((*BaseCurrent)->assimilation_turns_left + 9) / 10,
        0, (pop_size + diff_lvl - 2) / 4);
    if (Players[faction_id].rule_drone) {
        psych_val += pop_size / Players[faction_id].rule_drone;
    }
    int psych_val_2 = 0;
    if (Players[faction_id].rule_talent) {
        psych_val_2 += (Players[faction_id].rule_talent + pop_size - 1) 
            / Players[faction_id].rule_talent;
    }
    for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
        if (Players[faction_id].faction_bonus_id[i] == RULE_NODRONE) {
            psych_val -= Players[faction_id].faction_bonus_val1[i];
            break;
        }
    }
    int drones = has_fac_built(FAC_GENEJACK_FACTORY) ? Rules->drones_genejack_factory : 0;
    if (has_fac_built(FAC_CHILDREN_CRECHE)) {
        drones -= 2;
    }
    if ((has_fac_built(FAC_NETWORK_NODE) && has_project(SP_VIRTUAL_WORLD, faction_id))
        || has_fac_built(FAC_HOLOGRAM_THEATRE)) {
        drones -= 2;
    }
    if (has_project(SP_PLANETARY_TRANS_SYS, faction_id) && pop_size <= 3) {
        drones--;
    }
    int talents_fac = has_fac_built(FAC_PARADISE_GARDEN) ? 2 : 0;
    if (has_fac_built(FAC_RESEARCH_HOSPITAL)) {
        drones--;
    }
    if (has_fac_built(FAC_NANOHOSPITAL)) {
        drones--;
    }
    int talents_sp = has_project(SP_HUMAN_GENOME_PROJ, faction_id) ? 1 : 0;
    if (has_project(SP_CLINICAL_IMMORTALITY, faction_id)) {
        talents_sp += 2; // TODO: eval - bug fix per manual: one extra talent at every base
    }
    BOOL has_punishment_sphere = has_fac_built(FAC_PUNISHMENT_SPHERE);
    for (int i = 8; i >= 0; i--) {
        int val;
        if (!i) {
            val = PlayersData[faction_id].soc_effect_pending.talent;
        } else if(*BaseUpkeepStage != 1) {
            break;
        } else {
            val = i - 4;
        }
        for (int j = 9; j >= 0; j--) {
            int val_2;// , k;
            if (!j) {
                val_2 = PlayersData[faction_id].soc_effect_pending.police
                    + has_fac_built(FAC_BROOD_PIT) * 2;
                if (i) {
                    break;
                }
            } else if (*BaseUpkeepStage != 1 || !i) {
                break;
            } else {
                val_2 = j - 6;
                //val2 = val;
            }
            val_2 = range(val_2 + 2, 0, 4);
            if (val_2 > 1) {
                val_2--;
            }
            (*BaseCurrent)->drone_total = drones_base;
            (*BaseCurrent)->talent_total = talents_base;
            if (val_2 >= 0) {
                (*BaseCurrent)->talent_total += val_2;
            } else {
                (*BaseCurrent)->drone_total -= val_2;
            }
        }
    }
}

/*
Purpose: Determine the faction's base with the specified position sorted by the most energy output.
Original Offset: 004EB490
Return Value: Base id for the specified rank position or -1 for invalid requests
Status: Complete
*/
int __cdecl base_rank(uint32_t faction_id, int rank_position) {
    if (rank_position < 0) { // Added parameter bounds check
        return -1;
    }
    int base_id[MaxBaseNum];
    int base_energy[MaxBaseNum];
    int bases_found = 0;
    for (int i = 0; i < *BaseCurrentCount; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            base_id[bases_found] = i;
            base_energy[bases_found++] = (Bases[i].energy_intake_1 * 512) + i;
        }
    }
    if (!bases_found || rank_position >= bases_found) { // Added check to skip if no bases are found
        return -1;
    }
    sort_descending(bases_found, base_id, base_energy);
    return base_id[rank_position];
}

/*
Purpose: Check what facility (if any) a base needs for additional population growth. Stand alone
         function unused in original game and likely optimized out.
Original Offset: 004EEF80
Return Value: Facility id needed for pop growth or zero if base already has Hab Complex and Dome.
Status: Complete
*/
uint32_t __cdecl pop_goal_fac(uint32_t base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    uint32_t limit_mod = has_project(SP_ASCETIC_VIRTUES, faction_id) ? 2 : 0;
    int pop = Bases[base_id].population_size - limit_mod + Players[faction_id].rule_population;
    if (pop >= (int)Rules->pop_limit_sans_hab_complex && !has_fac_built(FAC_HAB_COMPLEX, base_id)) {
        return FAC_HAB_COMPLEX;
    }
    if (pop >= (int)Rules->pop_limit_sans_hab_dome 
        && !has_fac_built(FAC_HABITATION_DOME, base_id)) {
        return FAC_HABITATION_DOME;
    }
    return 0; // Pop hasn't reached capacity or Base already has Hab Complex and Dome
}

/*
Purpose: Calculate population goal for a base.
Original Offset: 004EF090
Return Value: Goal population
Status: Complete
*/
uint32_t __cdecl pop_goal(uint32_t base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    uint32_t limit_mod = has_project(SP_ASCETIC_VIRTUES, faction_id) ? 2 : 0;
    int goal = (36 - Bases[base_id].population_size) / 6 + Bases[base_id].population_size;
    if (goal <= 6) {
        goal = 6;
    }
    if (!has_fac_built(FAC_HAB_COMPLEX, base_id)) {
        int compare = Rules->pop_limit_sans_hab_complex - Players[faction_id].rule_population 
            + limit_mod;
        if (goal >= compare) {
            goal = compare;
        }
    }
    if (!has_fac_built(FAC_HABITATION_DOME, base_id)) {
        int compare = Rules->pop_limit_sans_hab_dome - Players[faction_id].rule_population 
            + limit_mod;
        if (goal >= compare) {
            goal = compare;
        }
    }
    return goal;
}

/*
Purpose: Move the specified base's production queue forward if applicable.
Original Offset: 004F06E0
Return Value: Is there a valid item in queue to be built? true/false
Status: Complete
*/
BOOL __cdecl base_queue(uint32_t base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    while (Bases[base_id].queue_size) {
        int queue_prod_id = Bases[base_id].queue_production_id[0];
        if (queue_prod_id >= 0) {
            PlayersData[faction_id].proto_id_queue[queue_prod_id]--;
        }
        for (uint32_t i = 0; i < Bases[base_id].queue_size; i++) {
            Bases[base_id].queue_production_id[i] = Bases[base_id].queue_production_id[i + 1];
        }
        Bases[base_id].queue_size--;
        queue_prod_id = Bases[base_id].queue_production_id[0];
        if((queue_prod_id >= 0) ? veh_avail(queue_prod_id, faction_id, base_id) 
            : facility_avail(-queue_prod_id, faction_id, base_id, 0)) {
            if (queue_prod_id <= -FacilitySPStart) {
                wave_it(36); // CPU project initiated
            }
            draw_radius(Bases[base_id].x, Bases[base_id].y, 2, 2);
            return true;
        }
    }
    return false;
}

/*
Purpose: Check if current base has had an energy shortfall. If so, reset all existing energy convoy
         orders for the faction. TODO: Revisit and find a way to only reset specific base convoys.
Original Offset: 004F4DC0
Return Value: n/a
Status: Complete
*/
void __cdecl base_energy_costs() {
    if ((*BaseCurrent)->energy_surplus >= 0 || *VehCurrentCount <= 0) {
        return;
    }
    uint32_t faction_id = (*BaseCurrent)->faction_id_current;
    for (int i = 0; i < *VehCurrentCount; i++) {
        if (Vehs[i].faction_id == faction_id 
            && VehPrototypes[Vehs[i].proto_id].plan == PLAN_SUPPLY_CONVOY
            && Vehs[i].order == ORDER_CONVOY && Vehs[i].order_auto_type == RSC_ENERGY
            && base_who(Vehs[i].x, Vehs[i].y) >= 0) {
            Vehs[i].order = ORDER_NONE;
        }
    }
}

/*
Purpose: Calculate facility maintenance cost for specified faction.
Original Offset: 004F6510
Return Value: Facility maintenance cost
Status: Complete
*/
uint32_t __cdecl fac_maint(uint32_t facility_id, uint32_t faction_id) {
    if (facility_id == FAC_COMMAND_CENTER) {
        int reactor = best_reactor(faction_id);
        int diff_factor = (PlayersData[faction_id].diff_level + 1) / 2;
        if (reactor < 0 || diff_factor < 0) {
            return 0;
        }
        return (reactor > diff_factor) ? diff_factor : reactor;
    }
    int bonus_count = Players[faction_id].faction_bonus_count;
    for (int i = 0; i < bonus_count; i++) {
        int bonus_id = Players[faction_id].faction_bonus_id[i];
        if ((bonus_id == RULE_FACILITY || (bonus_id == RULE_FREEFAC
            && has_tech(Facility[Players[faction_id].faction_bonus_val1[i]].preq_tech, faction_id)))
            && Players[faction_id].faction_bonus_val1[i] == (int)facility_id) {
            return 0;
        }
    }
    return Facility[facility_id].maint;
}

/*
Purpose: Calculate overall maintenance cost for the currently selected base.
Original Offset: 004F65F0
Return Value: Base maintenance cost
Status: Complete
*/
void __cdecl base_maint() {
    uint32_t faction_id = (*BaseCurrent)->faction_id_current;
    for (int fac = 1; fac < FacilitySPStart; fac++) {
        if (has_fac_built(fac)) {
            uint32_t maint = fac_maint(fac, faction_id);
            if (has_project(SP_SELF_AWARE_COLONY, faction_id)) {
                if (PlayersData[faction_id].flags & PFLAG_SELF_AWARE_COLONY_LOST_MAINT) {
                    maint++; // attempt to even out maintenance costs from lossy integer division
                }
                if (maint & 1) {
                    PlayersData[faction_id].flags |= PFLAG_SELF_AWARE_COLONY_LOST_MAINT;
                } else {
                    PlayersData[faction_id].flags &= ~PFLAG_SELF_AWARE_COLONY_LOST_MAINT;
                }
                maint /= 2;
            }
            PlayersData[faction_id].energy_reserves -= maint;
            PlayersData[faction_id].maint_cost_total += maint;
            if (PlayersData[faction_id].energy_reserves < 0) {
                if (PlayersData[faction_id].diff_level <= DLVL_SPECIALIST
                    || (*BaseCurrent)->queue_production_id[0] == -fac) {
                    PlayersData[faction_id].energy_reserves = 0;
                } else {
                    set_fac(fac, *BaseIDCurrentSelected, false);
                    PlayersData[faction_id].energy_reserves 
                        = cost_factor(faction_id, RSC_MINERALS, -1) * Facility[fac].cost;
                    parse_say(1, (int)*(&Facility[fac].name), -1, -1);
                    popb("POWERSHORT", 0x10000, 14, "genwarning_sm.pcx", NULL);
                }
            }
        }
    }
}

/*
Purpose: Check if the initial base name is unique. If not, append a sequential number after it.
         The function has been rewritten to work as intended.
Original Offset: 004F7FE0
Return Value: n/a
Status: Complete
*/
void __cdecl make_base_unique(uint32_t base_id) {
    purge_spaces(Bases[base_id].name_string); // added to remove extraneous whitespace
    uint32_t found = 0;
    std::string core_unique_name;
    for (int i = 0; i < *BaseCurrentCount; i++) {
        if (base_id != (uint32_t)i && !strcmp(Bases[i].name_string, Bases[base_id].name_string)) {
            if (!found) { // only create core part of base name on first collision
                Bases[base_id].name_string[21] = 0;
                core_unique_name = Bases[base_id].name_string;
            }
            std::string unique_name = core_unique_name;
            unique_name += "-" + std::to_string(++found + 1); // start with appending "-2"
            strcpy_s(Bases[base_id].name_string, 25, unique_name.c_str());
            i = 0; // reset loop to verify base name is still unique
        }
    }
}

/*
Purpose: Check if a faction has the specified secret project built in a base they control.
Original Offset: 004F80D0
Return Value: Does faction have Secret Project? true/false
Status: Complete
*/
BOOL __cdecl has_project(uint32_t project_id, uint32_t faction_id) {
    int base_id = base_project(project_id);
    return (base_id < 0) ? false : (Bases[base_id].faction_id_current == faction_id);
}

/*
Purpose: Checks whether the facility (non-SP) has been build in the currently selected base.
Original Offset: 00500290
Return Value: Does current base have facility? true/false
Status: Complete
*/
BOOL __cdecl has_fac_built(uint32_t facility_id) {
    return (facility_id >= FacilityRepStart) ? false
        : has_fac_built(facility_id, *BaseIDCurrentSelected);
}

/*
Purpose: Check if the base already has a particular facility built.
Original Offset: n/a
Return Value: Does base have facility built? true/false
Status: Complete
*/
BOOL __cdecl has_fac_built(uint32_t facility_id, uint32_t base_id) {
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    return (Bases[base_id].facilities_built[offset] & mask) != 0;
}

/*
Purpose: Get the current status of the specified project.
Original Offset: 005002E0
Return Value: Base id, if not built (-1) or destroyed (-2)
Status: Complete
*/
int __cdecl base_project(uint32_t project_id) {
    return *(&SecretProject->human_genome_project + project_id);
}

/*
Purpose: Determine the faction's best base to attack the specified base from.
Original Offset: 0054AFA0
Return Value: Base id to attack from or 0 
Status: Complete
*/
int __cdecl attack_from(uint32_t base_id_to_atk, uint32_t faction_id) {
    uint32_t region_src = region_at(Bases[base_id_to_atk].x, Bases[base_id_to_atk].y);
    int base_id_atk_from = 0; // TODO: eventually change this to -1 and add handling to the return
    int search = 9999;        // value to prevent the edge case of base id 0 being incorrect target
    for (int i = 0; i < *BaseCurrentCount; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            int x = Bases[i].x;
            int y = Bases[i].y;
            int compare = (x == PlayersData[faction_id].x_target
                && y == PlayersData[faction_id].y_target) 
                ? 0 : vector_dist(x, y, Bases[base_id_to_atk].x, Bases[base_id_to_atk].y);
            if (region_at(x, y) != region_src) {
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

/*
Purpose: Determine the value of the specified base between the requester and the respondent faction.
         This valuation could be triggered either from a gift, a threat or a base swap.
Original Offset: 0054CB50
Return Value: Value of base or -1 for invalid requests
Status: Complete - testing
*/
int __cdecl value_of_base(int base_id, uint32_t faction_id_req, uint32_t faction_id_res, 
                          uint32_t overmatch_deg, BOOL tgl) {
    if (base_id < 0) {
        return -1;
    }
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    int dist_factor = vulnerable(faction_id_req, x, y);
    if (dist_factor <= 0) {
        return -1;
    }
    uint32_t region_base = region_at(x, y);
    for (int i = 1; i < RadiusRange[6]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            int base_id_found = base_at(x_radius, y_radius);
            if (base_id_found >= 0) {
                uint32_t faction_id_base = Bases[base_id_found].faction_id_current;
                uint32_t region_found = region_at(x_radius, y_radius);
                if (faction_id_base != faction_id_req) {
                    if (region_base == region_found) {
                        if (faction_id_base == faction_id_res) {
                            dist_factor++;
                        } else {
                            break;
                        }
                    }
                } else {
                    if (region_base == region_found) {
                        dist_factor--;
                    }
                    break;
                }
            }
        }
    }
    int veh_id = stack_fix(veh_at(x, y)); // reason to define here rather than below?
    int most_reserves = PlayersData[faction_id_res].energy_reserves;
    if (PlayersData[faction_id_req].energy_reserves > most_reserves) {
        most_reserves = PlayersData[faction_id_req].energy_reserves;
    }
    uint32_t base_pop_factor = Bases[base_id].population_size;
    if (base_pop_factor < 3) {
        base_pop_factor = 3;
    }
    // Bug fix: treat value as unsigned, original uses signed which could cause incorrect valuation
    uint32_t value = ((most_reserves + 1000) / (dist_factor + 4)) * base_pop_factor;
    if (value < 100) {
        value = 100;
    }
    if (veh_who(x, y) < 0) {
        value /= 2;
    }
    uint32_t res_base_count_region = PlayersData[faction_id_res].region_total_bases[region_base];
    if (!res_base_count_region) {
        value *= 2;
    }
    uint32_t req_base_count_region = PlayersData[faction_id_req].region_total_bases[region_base];
    if (req_base_count_region == 1) {
        if (!res_base_count_region) {
            value *= 2;
        }
        if (res_base_count_region == 1) {
            value *= 2;
        }
    }
    if (tgl) {
        value *= 4;
    }
    if (res_base_count_region && req_base_count_region) {
        if (res_base_count_region >= req_base_count_region * 5) {
            value /= 2;
        }
        if (overmatch_deg) {
            value /= 2;
        }
    }
    uint32_t facil_count = 0;
    for (uint32_t fac = 1; fac < FacilitySPStart; fac++) {
        if (fac < FacilityRepStart && has_fac_built(fac, base_id)) {
            facil_count++;
            value += (Facility[fac].cost * facil_count);
        }
    }
    for (uint32_t proj = 0; proj < MaxSecretProjectNum; proj++) {
        if (base_project(proj) == base_id) {
            value += (Facility[FacilitySPStart + proj].cost * 25);
        }
    }
    for (int i = veh_top(veh_id); i >= 0; i = Vehs[i].next_veh_id_stack) {
        if (Vehs[i].faction_id == faction_id_req) {
            value += (VehPrototypes[Vehs[i].proto_id].cost * 2);
        }
    }
    if (!_stricmp(Players[faction_id_req].filename, "BELIEVE")) {
        value *= 2;
    }
    if (is_objective(base_id)) {
        value *= 4;
    }
    return value;
}

/*
Purpose: Determine ideal non-offense (defense, combat, recon) unit count for the specified base.
Original Offset: 00560B30
Return Value: Amount of non-offensive units needed (1-10)
Status: Complete
*/
uint32_t __cdecl garrison_check(uint32_t base_id) {
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    uint32_t faction_id = Bases[base_id].faction_id_current;
    uint32_t sp_count = 0;
    for (uint32_t i = 0; i < MaxSecretProjectNum; i++) {
        if (base_project(i) == (int)base_id) {
            sp_count++;
        }
    }
    int garrison = (sp_count + 2) / 3 + (Bases[base_id].population_size + 1) / 4 + 1;
    BOOL is_obj = is_objective(base_id);
    if (has_fac_built(FAC_HEADQUARTERS, base_id) || bit_at(x, y) & BIT_UNK_40000000 || is_obj) {
        garrison++;
    }
    if (is_obj && PlayersData[faction_id].flags & PFLAG_STRAT_DEF_OBJECTIVES) {
        garrison++;
    }
    if (PlayersData[faction_id].region_base_plan[region_at(x, y)] == PLAN_COLONIZATION) {
        garrison--;
    }
    int faction_id_sea = zoc_sea(x, y, faction_id) - 1;
    if (faction_id_sea > 0
        && (has_treaty(faction_id, faction_id_sea, DTREATY_VENDETTA | DTREATY_WANT_REVENGE)
        || has_treaty(faction_id_sea, faction_id, DTREATY_WANT_REVENGE)
        || PlayersData[faction_id_sea].integrity_blemishes >= 4)) {
        garrison++;
    }
    int support = PlayersData[faction_id].soc_effect_pending.support;
    if (support <= -4) {
        garrison--;
    }
    if (support <= -3 && garrison > 2) {
        garrison--;
    }
    return range(garrison, 1, 10);
}

/*
Purpose: Determine the number of defensive units needed to protect the specified base.
Original Offset: 00560D30
Return Value: Amount of defensive units needed (1-8)
Status: Complete
*/
uint32_t __cdecl defensive_check(uint32_t base_id) {
    uint32_t defenses = garrison_check(base_id);
    if (defenses > 5) {
        defenses--;
    }
    if (defenses > 2) {
        defenses--;
    }
    return defenses;
}

/*
Purpose: Determine if base is a port.
Original Offset: 00579A00
Return Value: Is base a port? true/false
Status: Complete
*/
BOOL __cdecl is_port(uint32_t base_id, BOOL is_base_radius) {
    return is_coast(Bases[base_id].x, Bases[base_id].y, is_base_radius);
}

/*
Purpose: Calculate how vulnerable the coordinates are for the specified faction based on how far
         away this tile is from the faction's Headquarters.
Original Offset: 0059E980
Return Value: Radial distance between coordinates and faction's HQ or 12 if no HQ/bases
Status: Complete
*/
int __cdecl vulnerable(uint32_t faction_id, int x, int y) {
    int dist = 12; // default value for no bases or no HQ
    for (int i = 0; i < *BaseCurrentCount; i++) {
        if (Bases[i].faction_id_current == faction_id && has_fac_built(FAC_HEADQUARTERS, i)) {
            dist = vector_dist(x, y, Bases[i].x, Bases[i].y);
            break;
        }
    }
    return dist;
}

/*
Purpose: Determine whether the specified base is considered an objective.
Original Offset: 005AC060
Return Value: Is base an objective? true/false
Status: Complete
*/
BOOL __cdecl is_objective(uint32_t base_id) {
    if (*GameRules & RULES_SCN_VICT_ALL_BASE_COUNT_OBJ || Bases[base_id].event & BEVENT_OBJECTIVE) {
        return true;
    }
    if (*GameRules & RULES_SCN_VICT_SP_COUNT_OBJ) {
        for (int i = 0; i < MaxSecretProjectNum; i++) {
            if (base_project(i) == (int)base_id) {
                return true;
            }
        }
    }
    if (*GameState & STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ
        && has_fac(*ScnVictFacilityObj, base_id, 0)) {
        return true;
    }
    return false;
}

/*
Purpose: Check if specified faction is currently building Ascent to Transcendence. This code isn't
         used by original game. There was also a bug where it compares to a non-negative queue id.
Original Offset: 005AC630
Return Value: Is faction transcending? true/false
Status: Complete
*/
BOOL __cdecl transcending(int faction_id) {
    if (!ascending(faction_id)) {
        return false;
    }
    for (int i = 0; i < *BaseCurrentCount; i++) {
        if (Bases[i].faction_id_current == faction_id
            && Bases[i].queue_production_id[0] == -FAC_ASCENT_TO_TRANSCENDENCE) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Check if Voice of Planet has been built that starts the Ascent to Transcendence sequence.
Original Offset: 005AC680
Return Value: Has Voice of Planet been built? true/false
Status: Complete
*/
BOOL __cdecl ascending(int UNUSED(faction_id)) {
    return base_project(SP_VOICE_OF_PLANET) != SP_Unbuilt;
}

/*
Purpose: Determine if the facility is redundant due to a Secret Project counting as that facility.
Original Offset: 005BA030
Return Value: Is facility redundant? true/false
Status: Complete
*/
BOOL __cdecl redundant(int facility_id, uint32_t faction_id) {
    uint32_t project_id;
    switch (facility_id) {
      case FAC_NAVAL_YARD:
        project_id = SP_MARITIME_CONTROL_CENTER;
        break;
      case FAC_PERIMETER_DEFENSE:
        project_id = SP_CITIZENS_DEFENSE_FORCE;
        break;
      case FAC_COMMAND_CENTER:
        project_id = SP_COMMAND_NEXUS;
        break;
      case FAC_BIOENHANCEMENT_CENTER:
        project_id = SP_CYBORG_FACTORY;
        break;
      case FAC_QUANTUM_CONVERTER:
        project_id = SP_SINGULARITY_INDUCTOR;
        break;
      case FAC_AEROSPACE_COMPLEX:
        project_id = SP_CLOUDBASE_ACADEMY;
        break;
      case FAC_ENERGY_BANK:
        project_id = SP_PLANETARY_ENERGY_GRID;
        break;
      default:
        return false;
    }
    return has_project(project_id, faction_id);
}

/*
Purpose: Determine if the provided faction can build a specific facility or Secret Project in the
         specified base. Checks are included to prevent SMACX specific facilities from being built
         in SMAC mode.
Original Offset: 005BA0E0
Return Value: Is facility or Secret Project available to faction, base and game mode? true/false
Status: Complete
*/
BOOL __cdecl facility_avail(uint32_t facility_id, uint32_t faction_id, int base_id, 
                            int queue_count) {
    // initial checks
    if (!facility_id || (facility_id == FAC_SKUNKWORKS && *DiffLevelCurrent <= DLVL_SPECIALIST)
        || (facility_id >= FAC_HUMAN_GENOME_PROJ && *GameRules & RULES_SCN_NO_BUILDING_SP)) {
        return false; // Skunkworks removed if there are no prototype costs
    }
    if (facility_id == FAC_ASCENT_TO_TRANSCENDENCE) { // at top since anyone can build it
        return ascending(faction_id) && *GameRules & RULES_VICTORY_TRANSCENDENCE
            && _stricmp(Players[faction_id].filename, "CARETAKE"); // bug fix for Caretakers
    }
    if (!has_tech(Facility[facility_id].preq_tech, faction_id)) { // Check tech for facility + SP
        return false;
    }
    // Secret Project availability
    if (!*ExpansionEnabled && (facility_id == FAC_MANIFOLD_HARMONICS
        || facility_id == FAC_NETHACK_TERMINUS || facility_id == FAC_CLOUDBASE_ACADEMY
        || facility_id == FAC_PLANETARY_ENERGY_GRID)) {
        return false;
    }
    if (facility_id == FAC_VOICE_OF_PLANET && !_stricmp(Players[faction_id].filename, "CARETAKE")) {
        return false; // shifted Caretaker Ascent check to top (never reached here)
    }
    if (facility_id >= FAC_HUMAN_GENOME_PROJ) {
        return base_project(facility_id - FAC_HUMAN_GENOME_PROJ) == SP_Unbuilt;
    }
    // Facility availability
    if (base_id < 0) {
        return true;
    }
    if (has_fac(facility_id, base_id, queue_count)) {
        return false; // already built or in queue
    }
    if (redundant(facility_id, faction_id)) {
        return false; // has SP that counts as facility
    }
    switch (facility_id) { // consolidated into switch to improve readability and performance
      case FAC_RECYCLING_TANKS:
        return !has_fac(FAC_PRESSURE_DOME, base_id, queue_count); // count as Recycling Tank, skip
      case FAC_TACHYON_FIELD:
        return has_fac(FAC_PERIMETER_DEFENSE, base_id, queue_count)
            || has_project(SP_CITIZENS_DEFENSE_FORCE, faction_id); // Cumulative
      case FAC_SKUNKWORKS:
        return !(Players[faction_id].rule_flags & RFLAG_FREEPROTO); // no prototype costs? skip
      case FAC_HOLOGRAM_THEATRE:
        return has_fac(FAC_RECREATION_COMMONS, base_id, queue_count) // not documented in manual
            && !has_project(SP_VIRTUAL_WORLD, faction_id); // Network Nodes replaces Theater
      case FAC_HYBRID_FOREST:
        return has_fac(FAC_TREE_FARM, base_id, queue_count); // Cumulative
      case FAC_QUANTUM_LAB:
        return has_fac(FAC_FUSION_LAB, base_id, queue_count); // Cumulative
      case FAC_NANOHOSPITAL:
        return has_fac(FAC_RESEARCH_HOSPITAL, base_id, queue_count); // Cumulative
      case FAC_PARADISE_GARDEN: // bug fix: added check
        return !has_fac(FAC_PUNISHMENT_SPHERE, base_id, queue_count); // antithetical
      case FAC_PUNISHMENT_SPHERE:
        return !has_fac(FAC_PARADISE_GARDEN, base_id, queue_count); // antithetical
      case FAC_NANOREPLICATOR:
        return has_fac(FAC_ROBOTIC_ASSEMBLY_PLANT, base_id, queue_count) // Cumulative
            || has_fac(FAC_GENEJACK_FACTORY, base_id, queue_count);
      case FAC_HABITATION_DOME:
        return has_fac(FAC_HAB_COMPLEX, base_id, queue_count); // must have Complex
      case FAC_TEMPLE_OF_PLANET:
        return has_fac(FAC_CENTAURI_PRESERVE, base_id, queue_count); // must have Preserve
      case FAC_QUANTUM_CONVERTER:
        return has_fac(FAC_ROBOTIC_ASSEMBLY_PLANT, base_id, queue_count); // Cumulative
      case FAC_NAVAL_YARD:
        return is_coast(Bases[base_id].x, Bases[base_id].y, false); // needs ocean
      case FAC_AQUAFARM:
      case FAC_SUBSEA_TRUNKLINE:
      case FAC_THERMOCLINE_TRANSDUCER:
        return *ExpansionEnabled && is_coast(Bases[base_id].x, Bases[base_id].y, false);
      case FAC_COVERT_OPS_CENTER:
      case FAC_BROOD_PIT:
      case FAC_FLECHETTE_DEFENSE_SYS:
        return *ExpansionEnabled;
      case FAC_GEOSYNC_SURVEY_POD: // SMACX only & must have Aerospace Complex
        return *ExpansionEnabled && (has_fac(FAC_AEROSPACE_COMPLEX, base_id, queue_count)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)
            || has_project(SP_SPACE_ELEVATOR, faction_id));
      case FAC_SKY_HYDRO_LAB:
      case FAC_NESSUS_MINING_STATION:
      case FAC_ORBITAL_POWER_TRANS:
      case FAC_ORBITAL_DEFENSE_POD:  // must have Aerospace Complex
        return has_fac(FAC_AEROSPACE_COMPLEX, base_id, queue_count)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)
            || has_project(SP_SPACE_ELEVATOR, faction_id);
      case FAC_SUBSPACE_GENERATOR: // Progenitor factions only
        return is_alien_faction(faction_id);
      default:
        break;
    }
    return true;
}

/*
Purpose: To assist with locating a facility based on a name rather than using hardcoded offsets.
Original Offset: n/a
Return Value: Offset value otherwise -1 if not found
Status: Complete
*/
int __cdecl facility_offset(LPCSTR facil_search) {
    int offset = -1;
    for (uint32_t i = 0; i < MaxFacilityNum; i++) {
        LPSTR fac_name = Facility[i].name;
        if (fac_name && !_stricmp(facil_search, fac_name)) {
            offset = i;
            break;
        }
    }
    return offset;
}
