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
#include "veh.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "general.h" // range
#include "log.h"
#include "map.h"
#include "strings.h"
#include "technology.h"
#include "terraforming.h"


int VehCurrentCount;  // 0x009A64C8
int VehDropLiftVehID;  // 0x009B2280
int VehLiftX;  // 0x009B2278
int VehLiftY;  // 0x009B2284
BOOL VehBitError;  // 0x009B228C
int VehMoraleModifierCount; // only used by say_morale(), optimize to local var?
// Battle related globals
LPSTR VehBattleDisplayTerrain;

// has_fac_built_call() (real bitmask() call, general.h bitmask_call) is now
// declared in base.h, which this file already includes - no local
// redeclaration needed.

/*
Purpose: Generate an output string for the specified unit's morale.
// ORIGINAL: 0x004B3FD0 ?say_morale@@YAXPADHH@Z 0x004B3FD0-0x004B43BF
// LEVER: std::string -> strcat, matching the say_stats family (calls went
//        from 16, mostly basic_string internals, to 10; strcat count now
//        exactly 7, matching the image). RULED-OUT: crèche/brood-pit was
//        already the correct asymmetric if/else-if here, unlike
//        get_basic_offense - no change needed.
// LEVER: the ternary feeding the first strcat's `Strings::get()` argument
//        rewritten as an if/else with the strcat duplicated into each arm -
//        the image has 2 static call sites (one per branch) and the ternary
//        let the compiler merge them to 1. call_diff went from 10 (image 14)
//        to 11; best flag set's exact-instruction count nearly doubled,
//        13/313 -> 26/313 at /O2 /GR- /Oy- /GX.
// RULED-OUT: `bitmask_call` forwarder for the crèche/brood-pit
//        `has_fac_built()` sites (general.h, the same lever that helped
//        base.cpp's pop_goal) - tried on the sibling morale_veh (0x5C0E40)
//        first and reverted there, similarity fell at every flag set; not
//        retried here on that evidence. Still open: the same bitmask() x3
//        auto-inline wall (see get_basic_offense, 0x5015B0) accounts for the
//        remaining call-count gap (11 vs 14: 3x bitmask via has_fac_built).
// size      1007 bytes
// prototype void (__cdecl ?say_morale@@YAXPADHH@Z)(int8* moraleOutput, int vehID, int factionIDvsNative)
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050BA00 0x005C0E40 0x006169A0 0x00645470
Return Value: n/a
Status: Complete
*/
void __cdecl say_morale(LPSTR morale_output, int veh_id, int faction_id_vs_native) {
    // TODO: Look into inconsistencies related to morale display
    uint32_t morale = morale_veh(veh_id, false, faction_id_vs_native);
    uint32_t faction_id = Vehs[veh_id].faction_id;
    int proto_id = Vehs[veh_id].proto_id;
    // LEVER: std::string -> raw strcat() straight onto morale_output, per
    // the say_stats family above. The image's own `calls` list is 7 strcat
    // sites, not a string builder: this fill, the drone-riot " (-)", " "
    // and "(" as TWO separate strcat calls (not one " (" literal), the
    // "+" loop body, ")", and the designate-defender "(d)".
    if (proto_id < MaxVehProtoFactionNum &&
        (get_proto_offense_rating(proto_id) < 0 || proto_id == BSC_SPORE_LAUNCHER)) {
        strcat(morale_output, StringTable->get((int)Morale[morale].name_lifecycle));
    } else {
        strcat(morale_output, StringTable->get((int)Morale[morale].name));
    }
    if (VehPrototypes[proto_id].plan < PLAN_COLONIZATION) {
        uint32_t morale_penalty = 0;
        int home_base_id = Vehs[veh_id].home_base_id;
        if (home_base_id >= 0 && Bases[home_base_id].state & BSTATE_DRONE_RIOTS_ACTIVE && morale > 0
            && !(Players[faction_id].rule_flags & RFLAG_MORALE)) {
            strcat(morale_output, " (-)");
            morale_penalty = 1;
        }
        VehMoraleModifierCount = 0;
        int base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        if (base_id >= 0 && morale < 6) {
            if (has_fac_built(FAC_CHILDREN_CRECHE, base_id)) {
                VehMoraleModifierCount++;
                int morale_active = range(PlayersData[faction_id].soc_effect_active.morale, -4, 4);
                if (morale_active <= -2) {
                    morale_active++;
                }
                if (morale_active < 0) {
                    int morale_cap = morale + VehMoraleModifierCount;
                    do {
                        if (morale_cap >= 6) {
                            break;
                        }
                        VehMoraleModifierCount++;
                        morale_cap++;
                        morale_active++;
                    } while (morale_active < 0);
                }
                if(has_fac_built_call(FAC_BROOD_PIT, base_id) && proto_id < MaxVehProtoFactionNum
                    && (get_proto_offense_rating(proto_id) < 0 || proto_id == BSC_SPORE_LAUNCHER)) {
                    VehMoraleModifierCount++;
                }
            } else if (morale < 6 && has_fac_built_call(FAC_BROOD_PIT, base_id)
                && proto_id < MaxVehProtoFactionNum
                && (get_proto_offense_rating(proto_id) < 0 || proto_id == BSC_SPORE_LAUNCHER)) {
                VehMoraleModifierCount++;
                int morale_active = range(PlayersData[faction_id].soc_effect_active.morale, -4, 4);
                if (morale_active <= -2) {
                    morale_active++;
                }
                if (morale_active < 0) {
                    int morale_cap = morale + VehMoraleModifierCount;
                    do {
                        if (morale_cap >= 6) {
                            break;
                        }
                        VehMoraleModifierCount++;
                        morale_cap++;
                        morale_active++;
                    } while (morale_active < 0);
                }
            }
        }
        int morale_pending = PlayersData[faction_id].soc_effect_pending.morale;
        if (morale_pending == 2 || morale_pending == 3) {
            VehMoraleModifierCount++;
        }
        if (!morale && !VehMoraleModifierCount) {
            VehMoraleModifierCount = 1;
        }
        if(VehMoraleModifierCount) {
            strcat(morale_output, " ");
            strcat(morale_output, "(");
            for (int i = 0; i < VehMoraleModifierCount; i++) {
                strcat(morale_output, "+");
            }
            strcat(morale_output, ")");
        }
        VehMoraleModifierCount -= morale_penalty;
        if (Vehs[veh_id].state & VSTATE_DESIGNATE_DEFENDER) {
            strcat(morale_output, "(d)");
        }
    }
    // TODO: assumes at least 1032 char nulled buffer (stringTemp), eventually remove
}

/*
Purpose: Get the morale string for the specified unit and store it in the stringTemp buffer.
// ORIGINAL: 0x004B43C0 ?say_morale@@YAXHH@Z 0x004B43C0-0x004B43DA BYTE_EXACT
// size      26 bytes
// prototype void (__cdecl ?say_morale@@YAXHH@Z)(int vehID, int factionID)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004B3FD0
Return Value: n/a
Status: Complete
*/
void __cdecl say_morale(int veh_id, int faction_id_vs_native) {
    say_morale(StringTemp, veh_id, faction_id_vs_native);
}

/*
Purpose: Check whether the specified unit could ever reach the given tile as a patrol waypoint.
// ORIGINAL: 0x004CD6A0 ?valid_patrol@@YAHHHH@Z 0x004CD6A0-0x004CDA23
// size      899 bytes
// prototype int (__cdecl ?valid_patrol@@YAHHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050DE50 0x0050E160 0x005C1540 0x00644F3A
Return Value: Is the waypoint reachable? true/false
Status: Complete

Three answers, one per triad, and the shared part is only the four bounds checks and the refusal
to patrol to the tile the unit is already standing on.

LAND compares regions, which is the cheap standing-in for "there is a land route".

SEA asks the harbour question instead. A sea unit moves between ports, so what matters is whether
the two ends are connected by water: two bases go through port_to_port, a base and open water
through base_on_sea against the water's region, and open water at both ends falls back to the
same region comparison land uses.

AIR is a fuel calculation. A chassis with no range is unlimited and answers yes at once.
Otherwise both ends are classified as a landing place or not, and the test is `base_at() != 0 ||
airbase`. That is not `base_at() >= 0`: base_at answers -1 on an empty tile, so EMPTY GROUND
counts as a landing place and the one thing that does not is a tile holding base id zero with no
airbase on it. The fixture pins the distinction from both directions rather than assuming it is
a transcription slip. The reachable distance is

  (speed * (chassis range - terraforming_turns - 1) + moves left this turn) / move_rate_roads

with the turn's remaining movement clamped into 0..999 first. terraforming_turns is the field
air units keep their elapsed flight time in. The distance itself is the doubled vector_dist
halved once - (|dy| + wrapped |dx|) >> 1 - and it must be within the reachable distance when both
ends are landing places, or within half of it when either end is not, which is the fuel to get
there and back.

The x term wraps on a round map by testing bit 0 of MapIsFlat, the same reading reset_territory
and territory() use rather than x_dist's whole-int one.

Verification note: the sweep against recovery-gameplay-tests kills 46 of 52 valid mutants, and
the six survivors are dead code or equivalences rather than untested behaviour.

  - Both landing-place flags are set from a comparison and are therefore 0 or 1, so the `< 0`
    pair at 0x004CD900 that returns false when BOTH are negative cannot fire. The original tests
    them all the same and so does this. Three survivors.
  - `moves_left < 0` against `<= 0` or `< 1` differ only at zero, where the clamp writes the zero
    that is already there. Two survivors.
  - `moves_left > 999` against `>= 999` differ only at 999, where the clamp writes the 999 that
    is already there.
*/
int __cdecl valid_patrol(int veh_id, int x, int y) {
    Veh &veh = Vehs[veh_id];
    if (x == veh.x && y == veh.y) {
        return false;
    }
    if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
        return false;
    }
    uint32_t triad = Chassis[VehPrototypes[veh.proto_id].chassis_id].triad;
    if (triad == TRIAD_LAND) {
        return map_loc(x, y)->region == map_loc(veh.x, veh.y)->region;
    }
    if (triad == TRIAD_SEA) {
        int base_veh = base_at(veh.x, veh.y);
        int base_dst = base_at(x, y);
        if (base_veh >= 0) {
            if (base_dst >= 0) {
                return port_to_port(base_veh, base_dst) != 0;
            }
            return base_on_sea(base_veh, map_loc(x, y)->region) != 0;
        }
        if (base_dst >= 0) {
            return base_on_sea(base_dst, map_loc(veh.x, veh.y)->region) != 0;
        }
        return map_loc(x, y)->region == map_loc(veh.x, veh.y)->region;
    }
    if (triad != TRIAD_AIR) {
        return true;
    }
    uint32_t chassis_id = VehPrototypes[veh.proto_id].chassis_id;
    if (!Chassis[chassis_id].range) {
        return true;
    }
    int landing_veh;
    if (base_at(veh.x, veh.y)) {
        landing_veh = 1;
    } else {
        landing_veh = 0;
        if (map_loc(veh.x, veh.y)->bit & BIT_AIRBASE) {
            landing_veh = 1;
        }
    }
    int landing_dst;
    if (base_at(x, y)) {
        landing_dst = 1;
    } else {
        landing_dst = 0;
        if (map_loc(x, y)->bit & BIT_AIRBASE) {
            landing_dst = 1;
        }
    }
    if (landing_veh < 0 && landing_dst < 0) {
        return false;
    }
    int moves_left = (int)speed(veh_id, false) - veh.moves_expended;
    if (moves_left < 0) {
        moves_left = 0;
    } else if (moves_left > 999) {
        moves_left = 999;
    }
    int reach = ((int)speed(veh_id, false)
        * (Chassis[VehPrototypes[veh.proto_id].chassis_id].range - veh.terraforming_turns - 1)
        + moves_left) / (int)Rules->move_rate_roads;
    int x_delta = abs(x - veh.x);
    if (!(MapIsFlat & 1) && x_delta > (int)MapLongitude) {
        x_delta = MapLongitudeBounds - x_delta;
    }
    int dist = (abs(y - veh.y) + x_delta) >> 1;
    if (landing_veh && landing_dst) {
        return dist <= reach;
    }
    return dist <= reach / 2;
}

/*
Purpose: Calculate maximum range a faction's units can drop (air drops, Drop Pods).
// ORIGINAL: 0x00500320 ?drop_range@@YAHH@Z 0x00500320-0x0050037C
// LEVER: has_project(SP_SPACE_ELEVATOR, faction_id) written out by hand
//        (base_project's `*(&SecretProject->human_genome_project + id)` then
//        the owning base's faction) rather than called - at the `/Ob0` flag
//        set this needs (so has_tech does NOT auto-inline, matching the
//        image's real `call 0x5b9f20`), a call to the shared has_project()
//        inline does not fold back open and compiles a real `push 0x1b; call`
//        instead. Moved 10/36 (0.769) -> 17/36 (0.928): the SecretProject
//        constant (0x9a6580) now lands exactly where the image has it.
// size      92 bytes
// prototype int (__cdecl ?drop_range@@YAHH@Z)(int factionID)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Max range
Status: Complete
*/
int __cdecl drop_range(int faction_id) {
    // has_tech() is called for real here (the image's `call 0x5b9f20`), but
    // has_project() is written out rather than called: at `/Ob0` - needed so
    // has_tech does NOT auto-inline here, matching the image - a call to the
    // shared `has_project()` inline does not fold back open, so this repeats
    // its body (base_project() then the owning base's faction) directly.
    if (!has_tech(Rules->tech_orb_insert_sans_spc_elev, faction_id)) { // default 'Graviton Theory'
        int base_id = *(&SecretProject->human_genome_project + SP_SPACE_ELEVATOR);
        if (base_id < 0 || Bases[base_id].faction_id_current != faction_id) {
            return Rules->max_airdrop_sans_orb_insert;
        }
    }
    return (MapLongitudeBounds <= MapLatitudeBounds) ? MapLatitudeBounds : MapLongitudeBounds;
}

/*
Purpose: Check whether the specified prototype is a planet buster.
// ORIGINAL: 0x005004F0 ?planet_buster2@@YAHH@Z 0x005004F0-0x00500518 BYTE_EXACT
// symbol    ?planet_buster2@@YAIH@Z
// size      40 bytes
// prototype int (__cdecl ?planet_buster2@@YAHH@Z)(int protoID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Reactor id if planet buster, otherwise 0
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Check whether the specified unit is a planet buster.
// ORIGINAL: 0x00500520 ?planet_buster@@YAHH@Z 0x00500520-0x00500556 BYTE_EXACT
// symbol    ?planet_buster@@YAIH@Z
// size      54 bytes
// prototype int (__cdecl ?planet_buster@@YAHH@Z)(int vehID)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Reactor id if planet buster, otherwise 0
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Calculate how defensive the specified tile is to the defending unit based on the terrain.
// ORIGINAL: 0x005010C0 ?defense_value@@YAHHHHHH@Z 0x005010C0-0x0050134F
// LEVER: MORE (3 calls vs image's 0) - `label_get()` folds its array read
//        into a REAL `StringTable->get()` call, but the image never calls
//        anything here: `VehBattleDisplayTerrain` is read back at
//        battle_compute (0x501350-ish, line ~959) as `get(int(...))`, so
//        this site only needs to stash the raw id, not resolve it. Replaced
//        all three `label_get(n)` with the array read it wraps,
//        `*((LPSTR *)Labels->strings_ptr + n)`. Call count now agrees (0);
//        best flag set's similarity 0.391 -> 0.753 (/O2 /GR- /GX). Still
//        MISMATCH: the image writes an unidentified global (0x90f550, from
//        [0x9b90f8]+0x16c or +0x548 depending on branch) that this tree's
//        source has no equivalent for - not a call-count question, left open.
// size      655 bytes
// prototype int (__cdecl ?defense_value@@YAHHHHHH@Z)(int factionID, int xCoord, int yCoord, int vehIDDef, int vehIDAtk)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Defense terrain value
Status: Complete
*/
int __cdecl defense_value(int faction_id, int x, int y, int veh_id_def,
                          int veh_id_atk) {
    if (is_ocean(x, y) || base_who(x, y) >= 0) {
        return 2;
    }
    uint32_t bit = bit_at(x, y);
    if (bit & BIT_FUNGUS && altitude_at(x, y) >= ALT_BIT_OCEAN_SHELF && veh_id_atk >= 0
        && (!Vehs[veh_id_atk].faction_id 
            || has_project(SP_PHOLUS_MUTAGEN, Vehs[veh_id_atk].faction_id) 
            || (get_offense_rating(veh_id_atk) < 0
                && Vehs[veh_id_atk].proto_id < MaxVehProtoFactionNum))) {
                return 2;
    }
    uint32_t is_rocky = rocky_at(x, y) > TERRAIN_BIT_ROLLING;
    VehBattleDisplayTerrain = *((LPSTR *)Labels->strings_ptr + 91); // "Rocky"
    uint32_t defense = is_rocky;
    if (bit & BIT_FUNGUS && altitude_at(x, y) >= ALT_BIT_OCEAN_SHELF && !defense
        && get_triad(veh_id_def) != TRIAD_AIR) {
        if (veh_id_atk >= 0 && ((get_offense_rating(veh_id_atk) < 0
                && Vehs[veh_id_atk].proto_id < MaxVehProtoFactionNum)
            || has_project(SP_PHOLUS_MUTAGEN, Vehs[veh_id_atk].faction_id))) {
            defense = is_rocky;
        } else {
            VehBattleDisplayTerrain = *((LPSTR *)Labels->strings_ptr + 338); // "Fungus"
            defense = (has_project(SP_PHOLUS_MUTAGEN, faction_id) 
                || (Vehs[veh_id_def].proto_id < MaxVehProtoFactionNum 
                    && get_offense_rating(veh_id_def) < 0)) ? 2 : 1;
        }
    }
    if (bit & BIT_FOREST && !defense && (veh_id_atk < 0 || get_triad(veh_id_atk) == TRIAD_LAND)) {
        VehBattleDisplayTerrain = *((LPSTR *)Labels->strings_ptr + 291); // "Forest"
        defense = 1;
    }
    return defense + 2;
}

/*
Purpose: Calculate the lifecycle (morale) of the specified native life unit.
// ORIGINAL: 0x00501350 ?morale_alien@@YAHHH@Z 0x00501350-0x005014F1
// RULED-OUT: still MISMATCH (~0.72 similar, up from ~0.46) - most of the remaining divergence is register allocation the compiler picked differently around the atrocities/veh-flags block and the two `range()` call sites, which every flag set tried left in place.
// RULED-OUT: source had the Fungal Tower special case as the OUTER `if`, guarded by `veh_id >= 0`, with the turn-ladder as its `else` - the image runs the turn-ladder/atrocities/veh-flags computation FIRST, UNCONDITIONALLY, then reads `Vehs[veh_id].x/.y/.proto_id` with NO veh_id guard (an out-of-bounds `Vehs[-1]` read - reproduced deliberately, see the BUG comment at the site) and only THEN branches on proto_id, overwriting `morale` if it is the Fungal Tower. Each branch ends in its OWN `return range(morale, 0, 6);` (not a shared tail after the `if`) - `int x`/`int y` locals (not `int16_t`) match the image's `movsx` at the point of assignment.
// symbol    ?morale_alien@@YAIHH@Z
// size      417 bytes
// prototype int (__cdecl ?morale_alien@@YAHHH@Z)(int vehID, int factionIDvsNative)
// callers   9   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Lifecycle (morale) value
Status: Complete
*/
uint32_t __cdecl morale_alien(int veh_id, int faction_id_vs_native) {
    int morale;
    if (TurnCurrentNum < 45) {
        morale = 0;
    } else if (TurnCurrentNum < 90) {
        morale = 1;
    } else if (TurnCurrentNum < 170) {
        morale = 2;
    } else if (TurnCurrentNum < 250) {
        morale = 3;
    } else if (TurnCurrentNum < 330) {
        morale = 4;
    } else { // 330+
        morale = 6;
    }
    if (faction_id_vs_native > 0) {
        morale += (PlayersData[faction_id_vs_native].major_atrocities != 0)
            + (TectonicDetonationCount[faction_id_vs_native] != 0);
    }
    if (veh_id >= 0) {
        if (Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED) {
            morale++;
        }
        if (Vehs[veh_id].proto_id == BSC_LOCUSTS_OF_CHIRON) {
            morale++;
        }
        morale += (Vehs[veh_id].flags >> 3) & 3; // 0x8|0x10 > +1, +2, or +3; TODO: id unk flags
    }
    // BUG IN THE ORIGINAL: Vehs[veh_id].x/.y/.proto_id are read here with no
    // veh_id >= 0 guard, unlike the block just above - Vehs[-1] is read out
    // of bounds whenever the caller passes a negative veh_id. Reproduced
    // deliberately: the image has no such check either.
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    // Fungal Tower specific code: overrides the morale computed above entirely.
    if (Vehs[veh_id].proto_id == BSC_FUNGAL_TOWER) {
        morale = 0;
        // similar to is_coast() > except with fungus check + Ocean Shelf included
        for (int i = RadiusRange[0]; i < RadiusRange[1]; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius) && bit_at(x_radius, y_radius) & BIT_FUNGUS
                && altitude_at(x_radius, y_radius) >= ALT_BIT_OCEAN_SHELF) {
                morale++;
            }
        }
        morale -= 2;
        return range(morale, 0, 6);
    }
    return range(morale, 0, 6);
}

/*
Purpose: Calculate the psi combat factor for an attacking or defending unit.
// ORIGINAL: 0x00501500 ?psi_factor@@YAHHHHH@Z 0x00501500-0x005015AB BYTE_EXACT
// LEVER: each `if (has_project(...)) factor += factor/2;`/`if (is_fungal_twr) ...` stores its condition to a shared `add_half` local BEFORE the `if`, even the one used only once - the image tail-merges the attack and defense arms into one shared "if(add_half) factor+=factor/2; return factor;" block, and only materialising every condition into the same local (not just the ones that need to survive a branch) lets the compiler recognise the shared tail.
// size      171 bytes
// prototype int (__cdecl ?psi_factor@@YAHHHHH@Z)(int combatRatio, int factionID, BOOL isAttack, BOOL isFungalTower)
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Psi factor
Status: Complete
*/
int __cdecl psi_factor(int combat_ratio, int faction_id, BOOL is_attack, BOOL is_fungal_twr) {
    int rule_psi = Players[faction_id].rule_psi;
    int factor = rule_psi ? ((rule_psi + 100) * combat_ratio) / 100 : combat_ratio;
    BOOL add_half;
    if (is_attack) {
        add_half = has_project(SP_DREAM_TWISTER, faction_id); // Psi Attack +50%
    } else {
        add_half = has_project(SP_NEURAL_AMPLIFIER, faction_id);
        if (add_half) {
            factor += factor / 2; // Psi Defense +50%s
        }
        add_half = is_fungal_twr; // SMACX only: likely +50% Fungal Tower defense bonus
    }
    if (add_half) {
        factor += factor / 2;
    }
    return factor;
}

/*
Purpose: Get the basic offense value for an attacking unit with an optional defender unit parameter.
// ORIGINAL: 0x005015B0 ?get_basic_offense@@YAHHHHHH@Z 0x005015B0-0x00501940
// LEVER: crèche/brood-pit was `if/else if` with the SAME body on both arms;
//        the disassembly's 3rd bitmask() call proves the crèche-true arm
//        also runs a nested, BARE `morale++` brood-pit check (no range()/cap
//        loop) - only the `else if` arm has the full adjustment.
// LEVER: `bitmask_call` (general.h forwarder, base.cpp's own lever) on the
//        NESTED brood-pit check and the `else if` brood-pit check - both real
//        calls in the image - while the outer crèche check stays plain
//        `has_fac_built` (still auto-inlines there). All three real was
//        MORE (10 vs 9); all three inline was the old FEWER (7 vs 9); this
//        combination is the one that reads 0 disagree AND is the best-
//        similarity combination of the four tried. Best flag set's exact
//        count 5/307 (0.432 similar) -> 8/307 (0.494 similar).
// size      912 bytes
// prototype int (__cdecl ?get_basic_offense@@YAHHHHHH@Z)(int vehIDAtk, int vehIDDef, uint32_t psiCombatType, BOOL isBombardment, int isUnkTgl)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00501350 0x0050BA00 0x005BF1F0 0x005C0E40 0x005C1150
Return Value: Basic offense
Status: Complete
*/
int __cdecl get_basic_offense(int veh_id_atk, int veh_id_def, int psi_combat_type, 
                              BOOL is_bombard, BOOL unk_tgl) { // art/duel related? Is off of def?
    uint32_t faction_id_atk = Vehs[veh_id_atk].faction_id;
    uint32_t proto_id_atk = Vehs[veh_id_atk].proto_id;
    uint32_t morale = faction_id_atk ? morale_veh(veh_id_atk, true, 0) 
        : morale_alien(veh_id_atk, veh_id_def >= 0 ? Vehs[veh_id_def].faction_id : -1);
    int base_id_atk = base_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y);
    if (base_id_atk >= 0) {
        if (has_fac_built(FAC_CHILDREN_CRECHE, base_id_atk)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_atk].soc_effect_active.morale, -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
            // The disassembly calls bitmask() a 3rd time: on the crèche-true
            // fallthrough, a brood pit check ALSO runs, but only as a bare
            // `morale++` - the full range()/cap adjustment below it belongs
            // only to the crèche-false (`else if`) arm.
            if (has_fac_built_call(FAC_BROOD_PIT, base_id_atk) && proto_id_atk < MaxVehProtoFactionNum
                && (get_proto_offense_rating(proto_id_atk) < 0 || proto_id_atk == BSC_SPORE_LAUNCHER)) {
                morale++;
            }
        } else if (has_fac_built_call(FAC_BROOD_PIT, base_id_atk) && proto_id_atk < MaxVehProtoFactionNum
            && (get_proto_offense_rating(proto_id_atk) < 0 || proto_id_atk == BSC_SPORE_LAUNCHER)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_atk].soc_effect_active.morale, -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
        }
        if (unk_tgl) {
            int morale_pending = PlayersData[faction_id_atk].soc_effect_pending.morale;
            if (morale_pending >= 2 && morale_pending <= 3) {
                morale++;
            }
            if (veh_id_def >= 0) {
                if (Vehs[veh_id_def].faction_id) {
                    if ((proto_id_atk >= MaxVehProtoFactionNum
                        || (get_proto_offense_rating(proto_id_atk) >= 0
                            && proto_id_atk != BSC_SPORE_LAUNCHER))
                        && !has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE)
                        && has_abil(Vehs[veh_id_def].proto_id, ABL_SOPORIFIC_GAS)) {
                        morale -= 2;
                    }
                } else {
                    morale++;
                }
            }
        }
    }
    if (unk_tgl) {
        morale = range(morale, 1, 6);
    }
    VehBasicBattleMorale[unk_tgl != 0] = morale; // shifted up from original
    morale += 6;
    uint32_t offense = offense_proto(proto_id_atk, veh_id_def, is_bombard);
    if (psi_combat_type) {
        offense = psi_factor(offense, faction_id_atk, true, false);
    }
    return offense * morale * 4;
}

/*
Purpose: Get the basic defense value for a defending unit with an optional attacker unit parameter.
// ORIGINAL: 0x00501940 ?get_basic_defense@@YAHHHHH@Z 0x00501940-0x00501D26
// LEVER: same asymmetric crèche/brood-pit fix as get_basic_offense above -
//        see its comment for the disassembly evidence.
// LEVER: same `bitmask_call` split as get_basic_offense - nested and
//        `else if` brood-pit checks real, outer crèche check left plain
//        `has_fac_built`. Call count now agrees (was FEWER 7 vs 9); best
//        flag set's exact count 1/338 (0.149 similar) -> 32/338 (0.504
//        similar).
// size      998 bytes
// prototype int (__cdecl ?get_basic_defense@@YAHHHHH@Z)(int vehIDDef, int vehIDAtk, uint32_t psiCombatType, BOOL isBombardment)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00501350 0x0050BA00 0x005BF1F0 0x005C0E40 0x005C1290
Return Value: Basic defense
Status: Complete
*/
int __cdecl get_basic_defense(int veh_id_def, int veh_id_atk, int psi_combat_type,
                              BOOL is_bombard) {
    uint32_t faction_id_def = Vehs[veh_id_def].faction_id;
    uint32_t proto_id_def = Vehs[veh_id_def].proto_id;
    uint32_t morale = faction_id_def ? morale_veh(veh_id_def, true, 0)
        : morale_alien(veh_id_def, veh_id_atk >= 0 ? Vehs[veh_id_atk].faction_id : -1);
    int base_id_def = base_at(Vehs[veh_id_def].x, Vehs[veh_id_def].y);
    if (base_id_def >= 0) {
        if (has_fac_built(FAC_CHILDREN_CRECHE, base_id_def)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_def].soc_effect_active.morale, -4, 0);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
            // Asymmetric with the `else if` below - see get_basic_offense
            // above for the disassembly evidence: the crèche-true fallthrough
            // only bumps morale by 1 for a brood pit, the full range()/cap
            // adjustment belongs to the crèche-false arm only.
            if (has_fac_built_call(FAC_BROOD_PIT, base_id_def)
                &&  proto_id_def < MaxVehProtoFactionNum
                && (get_proto_offense_rating(proto_id_def) < 0 || proto_id_def == BSC_SPORE_LAUNCHER)) {
                morale++;
            }
        } else if (has_fac_built_call(FAC_BROOD_PIT, base_id_def)
            &&  proto_id_def < MaxVehProtoFactionNum
            && (get_proto_offense_rating(proto_id_def) < 0 || proto_id_def == BSC_SPORE_LAUNCHER)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_def].soc_effect_active.morale, -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
        }
        if (has_fac_built(FAC_HEADQUARTERS, base_id_def)) { // bug fix: per manual
            morale++; // "Units in a headquarters base automatically gain +1 Morale when defending."
        }
        int morale_pending = PlayersData[faction_id_def].soc_effect_pending.morale;
        if (morale_pending >= 2 && morale_pending <= 3) {
            morale++;
        }
        if (veh_id_atk >= 0 && !Vehs[veh_id_atk].faction_id) {
            morale++;
        }
    }
    if (veh_id_atk >= 0 && Vehs[veh_id_atk].faction_id && (proto_id_def >= MaxVehProtoFactionNum 
        || (get_proto_offense_rating(proto_id_def) >= 0 && proto_id_def != BSC_SPORE_LAUNCHER)) 
        && !has_abil(proto_id_def, ABL_DISSOCIATIVE_WAVE)
        && has_abil(Vehs[veh_id_atk].proto_id, ABL_SOPORIFIC_GAS)) {
        morale -= 2;
    }
    morale = range(morale, 1, 6);
    VehBasicBattleMorale[1] = morale;
    morale += 6;
    uint32_t plan_def = VehPrototypes[proto_id_def].plan;
    if (plan_def == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    if (plan_def == PLAN_INFO_WARFARE && get_proto_defense_rating(proto_id_def) == 1
        // bug fix: added veh_id_atk bounds check to prevent potential arbitrary memory read
        && (veh_id_atk < 0 || VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE)) {
        return 1;
    }
    uint32_t defense = armor_proto(proto_id_def, veh_id_atk, is_bombard);
    if (psi_combat_type) {
        defense = psi_factor(defense, faction_id_def, false, proto_id_def == BSC_FUNGAL_TOWER);
    }
    return defense * morale;
}

/*
Purpose: Initialize or reset the battle related global variables.
// ORIGINAL: 0x00501D30 ?battle_init@@YAXXZ 0x00501D30-0x00501D47 BYTE_EXACT
// size      23 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Add combat battle modifier for type (offense, defense).
// ORIGINAL: 0x00501D50 ?add_bat@@YAXHHPAD@Z 0x00501D50-0x00501D9A BYTE_EXACT
// LEVER: BYTE_EXACT 25/25, from 2/25, on four measured changes at once. (1) SPLIT THE CHAIN: `if (modifier && offset < 4)` shares one epilogue, and the image tests `modifier` FIRST and falls through to `pop ebp; ret` before it even loads the count - two guard clauses, in that order. Reading the count first instead scores 5/25. (2) `int offset`, not `uint32_t`: the image's `cmp eax, 4` is followed by `jge`, and unsigned gives `jae`. (3) `VehBattleModCount[type] = offset + 1` BEFORE the two stores, where this tree had the increment last - the image's `lea esi, [eax + 1]; mov [ecx*4 + 0x915614], esi` comes first. (4) ONE `int index = type * 4 + offset` used by both the modifier store and the display address: the image keeps that value in eax and addresses `[eax*4 + 0x9155f0]` and `(eax + eax*4) << 4`, which `type * 80 + offset * 20` spelled out twice does not reproduce (9/25 -> 13/25).
// LEVER: strcpy, not strcpy_s. The image's only callee here is 0x00645460, plain strcpy; the safe-CRT form does not exist in it.
// symbol    ?add_bat@@YAXHHPBD@Z
// size      74 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645460
Return Value: n/a
Status: Complete
*/
void __cdecl add_bat(int type, int modifier, LPCSTR display_str) {
    // TODO: Revise global offsets once all references are decompiled.
    if (!modifier) {
        return;
    }
    int offset = VehBattleModCount[type];
    if (offset >= 4) {
        return;
    }
    VehBattleModCount[type] = offset + 1;
    int index = type * 4 + offset;
    VehBattleModifier[index] = modifier;
    // FOLD THE BASE, do not read it. `&VehBattleDisplay[i]` needs the
    // binding's VALUE, and through the file-scope `LPSTR *const` VC6 emits
    // `mov esi, dword ptr [VehBattleDisplay]` then `add edx, esi` where the
    // image has `add edx, 0x90f554`. A local const initialised FROM THE
    // LITERAL folds to that immediate; one initialised from
    // VehBattleDisplay does not - measured, 13/25 either way.
    LPSTR *const display = (LPSTR *)0x0090F554;   // VehBattleDisplay
    strcpy((LPSTR)&display[index * 20], display_str);
}

/*
Purpose: Calculate the battle outcome between two units.
// ORIGINAL: 0x00501DA0 ?battle_compute@@YAXHHPAHPAHH@Z 0x00501DA0-0x005044C4
// symbol    ?battle_compute@@YAXHHPAH0H@Z
// LEVER: THE SEH FRAME WAS A std::string. The terrain modifier was built with
//        `std::string terrain_modifier = label_get(331); ... .c_str()`, and
//        basic_string's destructor made VC6 emit an unwind frame the image
//        does not pay for: the divergence started at instruction 2, this tree
//        emitting `push -1 / push <handler> / mov eax, fs:[0] / push eax`
//        where the image has `sub esp, 0x2c / mov eax, 8`. Rebuilt as
//        strcpy/strcat onto StringTemp - the image's own callee list here is
//        strcpy (0x00645460) and strcat (0x00645470), and add_bat copies what
//        it is handed - and the frame is gone: the prologue now agrees and the
//        compiled body drops from 2,290 instructions to 2,160.
// RULED-OUT: BYTE_EXACT, on budget rather than on a wall. 2,811 image
//        instructions against 2,160 compiled - 651 SHORT, meaning whole
//        blocks of the image are inlined work this tree still calls out to or
//        does not do - at 0.037 similarity, the best of all ten flag sets, and
//        the metric did not move when the SEH frame came out (0.039 before).
//        A body this size is not reachable by source-form search in one pass;
//        it wants the callee-by-callee treatment say_stats got. What IS
//        pinned: the prologue divergence is now only `sub esp, 0x30` against
//        the image's `sub esp, 0x2c`, one dword of extra locals, and
//        call_diff AGREES on call count, so nothing is being over- or
//        under-called.
// size      10020 bytes
// prototype void (__cdecl ?battle_compute@@YAXHHPAHPAHH@Z)(int vehIDAtk, int vehIDDef, int* offenseOutput, int* defenseOutput, int combatType)
// callers   2   call targets   15
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3B80 0x004E3EF0 0x005010C0 0x005015B0 0x00501940 0x0050BA00 0x00581170 0x005B9F20 0x005BF010 0x005BF1F0 0x005C13B0 0x006169A0 0x00645460 0x00645470
Return Value: n/a
Status: Complete - testing
*/
void __cdecl battle_compute(int veh_id_atk, int veh_id_def, int *offense_out, int *defense_out, 
                            int combat_type) {
    int offense = 8;
    int defense = 8;
    int proto_id_atk = -1;
    int proto_id_def = -1;
    uint32_t faction_id_atk = 0;
    uint32_t faction_id_def = 0;
    if (veh_id_atk >= 0) {
        proto_id_atk = Vehs[veh_id_atk].proto_id;
        faction_id_atk = Vehs[veh_id_atk].faction_id;
    }
    if (veh_id_def >= 0) {
        proto_id_def = Vehs[veh_id_def].proto_id;
        faction_id_def = Vehs[veh_id_def].faction_id;
    }
    battle_init();
    BOOL is_artillery = combat_type & 1;
    BOOL is_bombard = (is_artillery // added veh_id_atk bounds check
        || (veh_id_atk >= 0 && is_proto_missile(proto_id_atk))) ? true : false;
    uint32_t psi_combat_type = 0;
    if (!is_bombard || proto_id_atk == BSC_SPORE_LAUNCHER || proto_id_def == BSC_SPORE_LAUNCHER) {
        if (veh_id_atk >= 0 && get_proto_offense_rating(proto_id_atk) < 0) {
            psi_combat_type = 1; // PSI attacker
        }
        if (veh_id_def >= 0 && get_proto_defense_rating(proto_id_def) < 0) {
            psi_combat_type |= 2; // PSI defender
        }
        // NOTE: PSI bonuses below are only for display purposes, actual calculation is done in 
        //       combat morale functions.
        if (psi_combat_type) {
            // Calculate PSI attack bonus
            int rule_psi_atk = Players[faction_id_atk].rule_psi;
            if (rule_psi_atk) {
                add_bat(0, rule_psi_atk, label_get(342)); // "Gaian Psi Bonus"
            }
            if (has_project(SP_DREAM_TWISTER, faction_id_atk)) {
                add_bat(0, 50, label_get(343)); // "Dream Twist"
            }
            // PSI Defense?
            if (psi_combat_type) { // shouldn't this be & 2?
                int rule_psi_def = Players[faction_id_def].rule_psi;
                if (rule_psi_def) {
                    add_bat(1, rule_psi_def, label_get(342)); // "Gaian Psi Bonus"
                }
                if (has_project(SP_NEURAL_AMPLIFIER, faction_id_def)) {
                    add_bat(1, 50, label_get(344)); // "Neural Amp"
                }
                if (veh_id_def >= 0 && proto_id_def == BSC_FUNGAL_TOWER) {
                    add_bat(1, 50, VehPrototypes[BSC_FUNGAL_TOWER].veh_name); // "Fungal Tower"
                }
            }
        }
        if (veh_id_atk >= 0) {
            offense = get_basic_offense(veh_id_atk, veh_id_def, psi_combat_type, is_bombard, false);
            if (VehPrototypes[proto_id_atk].plan != PLAN_INFO_WARFARE) {
                if (veh_id_def >= 0 && !combat_type) { // checking if isn't wpn vs wpn ; air toggle
                    int x_def = Vehs[veh_id_def].x;
                    int y_def = Vehs[veh_id_def].y;
                    if (bit_at(x_def, y_def) & BIT_FUNGUS
                        && altitude_at(x_def, y_def) >= ALT_BIT_OCEAN_SHELF
                        && ((proto_id_atk < MaxVehProtoFactionNum
                            && get_proto_offense_rating(proto_id_atk) < 0)
                            || has_project(SP_PHOLUS_MUTAGEN, faction_id_atk))
                        && get_proto_offense_rating(proto_id_def) >= 0) {
                        offense += offense / 2;
                        add_bat(0, 50, label_get(338)); // "Fungus"
                    }
                    if (psi_combat_type & 2
                        && (VehPrototypes[proto_id_atk].weapon_id == WPN_RESONANCE_LASER
                            || VehPrototypes[proto_id_atk].weapon_id == WPN_RESONANCE_BOLT)) {
                        offense = (offense * 125) / 100;
                        add_bat(0, 25, label_get(1110)); // "Resonance Attack"
                    }
                    if (!has_abil(proto_id_def, ABL_DISSOCIATIVE_WAVE) && psi_combat_type 
                        && has_abil(proto_id_atk, ABL_EMPATHIC) 
                        && Rules->combat_pct_emp_song_atk_vs_psi) {
                        offense *= ((Rules->combat_pct_emp_song_atk_vs_psi + 100) / 100);
                        add_bat(0, Rules->combat_pct_emp_song_atk_vs_psi,
                            Ability[abil_index(ABL_EMPATHIC)].name);
                    }
                }
                if (Vehs[veh_id_atk].state & VSTATE_MADE_AIRDROP 
                    && has_abil(proto_id_atk, ABL_DROP_POD) && Rules->combat_pen_pct_atk_airdrop) {
                    offense = (100 - Rules->combat_pct_emp_song_atk_vs_psi) * offense / 100;
                    /*
                    uint32_t dropRange;
                    if (has_tech(Rules->tech_orb_insert_sans_spc_elev, faction_id_atk)
                        || has_project(SP_SPACE_ELEVATOR, faction_id_atk)) {
                        dropRange = (*MapHorizontalBounds <= MapLatitudeBounds)
                            ? MapLatitudeBounds : *MapHorizontalBounds;
                    } else {
                        dropRange = Rules->max_airdrop_sans_orb_insert;
                    }
                    */
                    add_bat(0, Rules->combat_pen_pct_atk_airdrop,
                        drop_range(faction_id_atk) <= Rules->max_airdrop_sans_orb_insert 
                        ? label_get(437) // "Air Drop"
                        : label_get(438)); // "Orbital Insertion"
                }
                if (Players[faction_id_atk].rule_flags & RFLAG_FANATIC
                    && Rules->combat_pct_fanatic_atk_bonus && !combat_type && !psi_combat_type) {
                    offense = (Rules->combat_pct_fanatic_atk_bonus + 100) * offense / 100;
                    add_bat(0, Rules->combat_pct_fanatic_atk_bonus, label_get(528));
                }
                int bonus_count = Players[faction_id_atk].faction_bonus_count;
                for (int i = 0; i < bonus_count; i++) {
                    if (Players[faction_id_atk].faction_bonus_id[i] == RULE_OFFENSE) {
                        int rule_off_bonus = Players[faction_id_atk].faction_bonus_val1[i];
                        offense *= (rule_off_bonus / 100);
                        add_bat(0, rule_off_bonus, label_get(1108)); // "Alien Offense"
                    }
                }
                if (psi_combat_type && faction_id_atk) {
                    int planet_atk = PlayersData[faction_id_atk].soc_effect_active.planet;
                    if (planet_atk && Rules->combat_pct_psi_atk_bonus_planet) {
                        int planet_modifier = planet_atk * Rules->combat_pct_psi_atk_bonus_planet;
                        add_bat(0, planet_modifier, label_get(625)); // "Planet"
                        offense = (planet_modifier + 100) * offense / 100;
                    }
                }
            }
        }
        if (!(combat_type & 2) && veh_id_def >= 0) {
            defense = get_basic_defense(veh_id_def, veh_id_atk, psi_combat_type, is_bombard);
            int bonus_count = Players[faction_id_def].faction_bonus_count;
            for (int i = 0; i < bonus_count; i++) {
                if (Players[faction_id_def].faction_bonus_id[i] == RULE_DEFENSE) {
                    int rule_def_bonus = Players[faction_id_def].faction_bonus_val1[i];
                    defense *= (rule_def_bonus / 100);
                    add_bat(1, rule_def_bonus, label_get(1109)); // "Alien Defense"
                }
            }
            if (veh_id_atk >= 0 && VehPrototypes[proto_id_def].plan == PLAN_INFO_WARFARE) {
                defense *= 4;
            } else {
                int x_def = Vehs[veh_id_def].x;
                int y_def = Vehs[veh_id_def].y;
                int base_id_def = base_at(x_def, y_def);
                if (psi_combat_type) {
                    if (base_id_def >= 0 && Rules->combat_pct_base_def) {
                        add_bat(1, Rules->combat_pct_base_def, label_get(332)); // "Base"
                        defense *= ((Rules->combat_pct_base_def + 100) / 100);
                    }
                    defense *= 4;
                } else {
                    uint32_t triad_def = get_proto_triad(proto_id_def);
                    uint32_t terrain_def = (triad_def == TRIAD_AIR) 
                        ? 2 : defense_value(faction_id_def, x_def, y_def, veh_id_def, veh_id_atk);
                    uint32_t alt_atk;
                    uint32_t alt_def;
                    if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_LAND) {
                        if (combat_type && Rules->combat_pct_art_bonus_lvl_alt 
                            && triad_def == TRIAD_LAND 
                            && (alt_atk = alt_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y),
                                alt_def = alt_at(x_def, y_def), alt_atk > alt_def)) {
                            offense *= ((Rules->combat_pct_art_bonus_lvl_alt 
                                * (alt_atk - alt_def) + 100) / 100);
                            // "Altitude"
                            add_bat(0, Rules->combat_pct_art_bonus_lvl_alt, label_get(576));
                        }
                        if (Rules->combat_pct_mobile_open_ground && !combat_type && base_id_def < 0
                            && terrain_def == 2 
                            && rocky_at(x_def, y_def) < TERRAIN_BIT_ROCKY) {
                            uint32_t speed_atk = speed_proto(proto_id_atk);
                            if (speed_atk > Rules->move_rate_roads 
                                && speed_proto(proto_id_def) < speed_atk) {
                                offense *= ((Rules->combat_pct_mobile_open_ground + 100) / 100);
                                // "Mobile in open"
                                add_bat(0, Rules->combat_pct_mobile_open_ground, label_get(611));
                            }
                        }
                        if (Rules->combat_pct_def_vs_mobile_rough && !combat_type 
                            && (terrain_def > 2 || base_id_def >= 0) 
                            && rocky_at(x_def, y_def) < TERRAIN_BIT_ROCKY
                            && speed_proto(proto_id_atk) > Rules->move_rate_roads) {
                            defense *= ((Rules->combat_pct_def_vs_mobile_rough + 100) / 100);
                            // "Rough vs. Mobile" : "Mobile vs. Base"
                            add_bat(1, Rules->combat_pct_def_vs_mobile_rough, base_id_def < 0
                                ? label_get(548) : label_get(612));
                        }
                        if (Rules->combat_pct_atk_road && !combat_type) {
                            // TODO: add check road/tube Combat % -> attacking along road
                            offense *= ((Rules->combat_pct_atk_road + 100) / 100);
                            add_bat(0, Rules->combat_pct_atk_road, label_get(606)); // "Road Attack"
                        }

                        if (Rules->combat_pen_pct_atk_lwr_elev && !combat_type 
                            && altitude_at(x_def, y_def) 
                            > altitude_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y)
                            && !has_abil(proto_id_atk, ABL_ANTIGRAV_STRUTS)) {
                            defense *= ((Rules->combat_pen_pct_atk_lwr_elev + 100) / 100);
                            // "Uphill"
                            add_bat(1, Rules->combat_pen_pct_atk_lwr_elev, label_get(441));
                        }
                        if (Rules->combat_pct_atk_higher_elev && !combat_type 
                            && altitude_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y) 
                            > altitude_at(x_def, y_def) 
                            && !has_abil(proto_id_def, ABL_ANTIGRAV_STRUTS)) {
                            offense *= ((Rules->combat_pct_atk_higher_elev + 100) / 100);
                            add_bat(0, Rules->combat_pct_atk_road, label_get(330)); // "Downhill"
                        }
                    }
                    defense *= terrain_def;
                    if (terrain_def > 2) {
                        // NO C++ LIBRARY. The image's own callee list for this
                        // body is strcpy (0x00645460) and strcat (0x00645470),
                        // and add_bat copies what it is handed, so the shared
                        // StringTemp buffer is safe here.
                        strcpy(StringTemp, label_get(331)); // "Terrain"
                        strcat(StringTemp, " (");
                        strcat(StringTemp, StringTable->get(int(VehBattleDisplayTerrain)));
                        strcat(StringTemp, ")");
                        add_bat(1, 10 * (5 * terrain_def - 10), StringTemp);
                    }
                    LPSTR display_def; // only one is displayed
                    uint32_t def_multi = 2;
                    if (bit_at(x_def, y_def) & BIT_BUNKER && (veh_id_atk < 0
                        || get_triad(veh_id_atk) != TRIAD_AIR)) {
                        def_multi = 3;
                        display_def = label_get(358); // "Bunker"
                    }
                    if (base_id_def) {
                        display_def = label_get(332); // "Base"
                        def_multi = 2;
                        if (veh_id_atk >= 0) {
                            uint32_t triad_atk = get_triad(veh_id_atk);
                            uint32_t fac_modifier = 0;
                            switch (triad_atk) {
                              case TRIAD_LAND:
                                if (has_fac_built(FAC_PERIMETER_DEFENSE, base_id_def)
                                    || has_project(SP_CITIZENS_DEFENSE_FORCE, faction_id_def)) {
                                    display_def = label_get(354); // "Perimeter"
                                    fac_modifier = 4;
                                }
                                break;
                              case TRIAD_SEA:
                                if (has_fac_built(FAC_NAVAL_YARD, base_id_def)
                                    || has_project(SP_MARITIME_CONTROL_CENTER, faction_id_def)) {
                                    display_def = Facility[FAC_NAVAL_YARD].name;
                                    fac_modifier = 4;
                                }
                                break;
                              case TRIAD_AIR:
                                if (has_fac_built(FAC_AEROSPACE_COMPLEX, base_id_def)
                                    || has_project(SP_CLOUDBASE_ACADEMY, faction_id_def)) {
                                    display_def = Facility[FAC_AEROSPACE_COMPLEX].name;
                                    fac_modifier = 4;
                                }
                                break;
                              default:
                                fac_modifier = def_multi;
                                break;
                            }
                            if (has_fac_built(FAC_TACHYON_FIELD, base_id_def)) {
                                fac_modifier += 2;
                                display_def = label_get(357); // "Tachyon"
                            }
                            if (fac_modifier < 2 || def_multi == 2
                                && (veh_id_atk < 0 || has_abil(proto_id_atk, ABL_BLINK_DISPLACER)
                                    && Rules->combat_pct_base_def)) {
                                add_bat(1, Rules->combat_pct_base_def, label_get(332)); // "Base"
                                defense *= ((Rules->combat_pct_base_def + 100) / 100);
                            }
                            if (veh_id_atk >= 0 && has_abil(proto_id_atk, ABL_BLINK_DISPLACER)) {
                                if (def_multi > 2) {
                                    def_multi = 2;
                                }
                                display_def = label_get(428); // "Base vs. Blink"
                            }
                            if (Rules->combat_pct_infantry_vs_base && !combat_type 
                                && base_id_def >= 0 && faction_id_atk 
                                && proto_id_atk > MaxVehProtoFactionNum
                                && get_proto_offense_rating(proto_id_atk) >= 0 
                                && proto_id_atk != BSC_SPORE_LAUNCHER 
                                && !get_proto_triad(proto_id_atk)
                                && Chassis[VehPrototypes[proto_id_atk].chassis_id].speed == 1) {
                                offense *= ((Rules->combat_pct_infantry_vs_base + 100) / 100);
                                // "Infantry vs. Base"
                                add_bat(0, Rules->combat_pct_infantry_vs_base, label_get(547));
                            }
                            uint32_t bit_def;
                            if (is_artillery && def_multi <= 2 && base_id_def < 0
                                && rocky_at(x_def, y_def) < TERRAIN_BIT_ROCKY
                                && (bit_def = bit_at(x_def, y_def),
                                    !(bit_def & BIT_FOREST)) && (bit_def & BIT_FUNGUS
                                        || altitude_at(x_def, y_def) 
                                        < ALT_BIT_OCEAN_SHELF)) {
                                def_multi = 3;
                                display_def = label_get(525); // "Open Ground"
                            }
                            defense *= def_multi;
                            if (def_multi > 2) {
                                add_bat(1, 10 * (5 * def_multi - 10), 
                                    StringTable->get(int(display_def)));
                            }
                        }
                    }
                }
                if (faction_id_def && veh_id_atk >= 0 && is_proto_missile(proto_id_atk)) {
                    for (int i = 0; i < 25; i++) {
                        int x_radius = xrange(x_def + RadiusOffsetX[i]);
                        int y_radius = y_def + RadiusOffsetY[i];
                        if (on_map(x_radius, y_radius)) {
                            int base_id_radius = base_at(x_radius, y_radius);
                            if (base_id_radius >= 0
                                && Bases[base_id_radius].faction_id_current == faction_id_def
                                && has_fac_built(FAC_FLECHETTE_DEFENSE_SYS, base_id_radius)) {
                                defense = 3 * defense / 2;
                                add_bat(1, 50, label_get(1113)); // "Flechette"
                            }
                        }
                    }
                }
                if (!is_ocean(x_def, y_def)) {
                    uint32_t sensor_def = 0;
                    if (faction_id_def) {
                        for (int i = 0; i < 25; i++) {
                            int x_radius = xrange(x_def + RadiusOffsetX[i]);
                            int y_radius = y_def + RadiusOffsetY[i];
                            uint32_t sensor_status;
                            if (on_map(x_radius, y_radius)
                                && (sensor_status = is_sensor(x_radius, y_radius), sensor_status)) {
                                BOOL has_sensor = false;
                                if (!is_ocean(x_radius, y_radius)) {
                                    int faction_id_terr = whose_territory(faction_id_def, x_radius, 
                                        y_radius, NULL, false);
                                    if (faction_id_terr < 0 
                                        || (uint32_t)faction_id_terr == faction_id_def) {
                                        has_sensor = true;
                                    }
                                } else {
                                    int base_id_find = base_find(x_radius, y_radius);
                                    // assumes will find a base?
                                    if (Bases[base_id_find].faction_id_current == faction_id_def) {
                                        has_sensor = true;
                                    }
                                }
                                if (has_sensor) {
                                    sensor_def |= 1;
                                    if (sensor_status > 1) {
                                        sensor_def |= 2;
                                    }
                                }
                            }
                        }
                    }
                    if (sensor_def & 1) {
                        defense *= ((Rules->combat_pct_def_range_sensor + 100) / 100);
                        add_bat(1, Rules->combat_pct_def_range_sensor, label_get(613)); // "Sensor"
                    }
                    if (sensor_def & 2) {
                        defense *= ((Rules->combat_pct_def_range_sensor + 100) / 100);
                        add_bat(1, Rules->combat_pct_def_range_sensor, label_get(1123)); // "GSP"
                    }
                }
                if (!has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE) 
                    && Rules->combat_pct_trance_def_vs_psi && veh_id_atk >= 0 && psi_combat_type & 1
                    && has_abil(proto_id_def, ABL_TRANCE)) {
                    defense *= ((Rules->combat_pct_trance_def_vs_psi + 100) / 100);
                    add_bat(1, Rules->combat_pct_trance_def_vs_psi, label_get(329)); // "Trance"
                }
                uint32_t armor_id_def = VehPrototypes[proto_id_def].armor_id;
                if (psi_combat_type & 1 && veh_id_atk >= 0 && (armor_id_def == ARM_RESONANCE_3_ARMOR
                    || armor_id_def == ARM_RESONANCE_8_ARMOR)) {
                    defense = 125 * defense / 100;
                    add_bat(1, 25, label_get(1111)); // "Resonance Def."
                }
                // add check veh_id_atk >= 0 to skip this entire code section? original jumps to end
                if (veh_id_atk >= 0 && get_proto_triad(proto_id_atk) == TRIAD_AIR
                    && has_abil(proto_id_atk, ABL_AIR_SUPERIORITY) && !psi_combat_type) {
                    if (get_proto_triad(proto_id_def) == TRIAD_AIR) {
                        int ground_strike_pen = Rules->combat_pen_pct_air_supr_vs_grnd;
                        if (ground_strike_pen) {
                            offense *= ((100 - ground_strike_pen) / 100);
                            add_bat(0, -ground_strike_pen, label_get(448)); // "Ground Strike"
                        }
                    } else {
                        int air_to_air = Rules->combat_pct_air_supr_vs_air;
                        if (air_to_air && !has_abil(proto_id_def, ABL_AIR_SUPERIORITY)) {
                            offense *= ((air_to_air + 100) / 100);
                            add_bat(0, air_to_air, label_get(449)); // "Air-to-Air"
                        }
                    }
                }
                // added in veh_id_atk bounds check
                if (veh_id_atk >= 0 && get_proto_triad(proto_id_atk) == TRIAD_AIR
                    && get_proto_triad(proto_id_def) == TRIAD_AIR
                    && has_abil(proto_id_def, ABL_AIR_SUPERIORITY)
                    && has_abil(proto_id_atk, ABL_AIR_SUPERIORITY) && !psi_combat_type
                    && !is_proto_missile(proto_id_atk) && !is_proto_missile(proto_id_def)
                    && Rules->combat_pct_air_supr_vs_air) {
                    defense *= ((Rules->combat_pct_air_supr_vs_air + 100) / 100);
                    add_bat(1, Rules->combat_pct_air_supr_vs_air, label_get(449)); // "Air-to-Air"
                }
                if (!get_proto_offense_rating(proto_id_def) 
                    && get_proto_defense_rating(proto_id_def) == 1 && (faction_id_atk 
                        || base_id_def < 0) && Rules->combat_pen_pct_non_cbt_def_vs_cbt) {
                    defense *= ((100 - Rules->combat_pen_pct_non_cbt_def_vs_cbt) / 100);
                    // "Non Combat"
                    add_bat(1, -Rules->combat_pen_pct_non_cbt_def_vs_cbt, label_get(439));
                }
                if (veh_id_atk >= 0 // added veh_id_atk check
                    && get_proto_triad(proto_id_atk) == TRIAD_SEA
                    && get_proto_triad(proto_id_def) == TRIAD_SEA && base_id_def >= 0 
                    && Rules->combat_pct_bonus_vs_ship_port) {
                    offense *= ((Rules->combat_pct_bonus_vs_ship_port + 100) / 100);
                    add_bat(0, Rules->combat_pct_bonus_vs_ship_port, label_get(335)); // "In Port"
                }
                if (armor_id_def == ARM_PULSE_3_ARMOR || armor_id_def == ARM_PULSE_8_ARMOR
                    && get_proto_triad(proto_id_atk) == TRIAD_LAND
                    && Chassis[VehPrototypes[proto_id_atk].chassis_id].speed > 1) {
                    defense = 125 * defense / 100;
                    add_bat(1, 25, label_get(1112)); // "Pulse Defense"
                }
                if (!has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE)
                    && has_abil(proto_id_def, ABL_COMM_JAMMER)
                    && get_proto_triad(proto_id_atk) == TRIAD_LAND
                    && Chassis[VehPrototypes[proto_id_atk].chassis_id].speed > 1 
                    && Rules->combat_pct_com_jam_def_vs_mobl) {
                    defense *= ((Rules->combat_pct_com_jam_def_vs_mobl + 100) / 100);
                    add_bat(1, Rules->combat_pct_com_jam_def_vs_mobl, label_get(336)); // "Jammer"
                }
                if (!has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE) 
                    && has_abil(proto_id_def, ABL_AAA) && get_proto_triad(proto_id_atk) == TRIAD_AIR
                    && Rules->combat_pct_aaa_bonus_vs_air) {
                    defense *= ((Rules->combat_pct_aaa_bonus_vs_air + 100) / 100);
                    add_bat(1, Rules->combat_pct_aaa_bonus_vs_air, label_get(337)); // "Tracking"
                }
            }
        } else if (veh_id_def >= 0) {
            defense = get_basic_offense(veh_id_def, veh_id_atk, psi_combat_type, is_bombard, true);
            if (!(combat_type & 0x18)) {
                uint32_t triad_atk = get_proto_triad(proto_id_atk);
                uint32_t triad_def = get_proto_triad(proto_id_def);
                if (triad_def == TRIAD_SEA && triad_atk == TRIAD_LAND) {
                    int gun_vs_ship_art = Rules->combat_pct_land_gun_vs_ship_art;
                    if (gun_vs_ship_art) {
                        offense = (gun_vs_ship_art + 100) * offense / 100;
                        add_bat(0, gun_vs_ship_art, label_get(435)); // "Land Based Guns"
                    }
                } else if (triad_atk == TRIAD_SEA && triad_def == TRIAD_LAND) {
                    int gun_vs_ship_art = Rules->combat_pct_land_gun_vs_ship_art;
                    if (gun_vs_ship_art) {
                        defense = (gun_vs_ship_art + 100) * defense / 100;
                        add_bat(1, gun_vs_ship_art, label_get(435)); // "Land Based Guns"
                    }
                } else if (Rules->combat_pct_art_bonus_lvl_alt) {
                    uint32_t alt_atk = alt_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y);
                    uint32_t alt_def = alt_at(Vehs[veh_id_def].x, Vehs[veh_id_def].y);
                    if (alt_def >= alt_atk) {
                        if (alt_def > alt_atk) { // ???
                            int alt_modifier_def = (alt_def - alt_atk) 
                                * Rules->combat_pct_art_bonus_lvl_alt;
                            defense *= ((alt_modifier_def + 100) / 100);
                            add_bat(1, alt_modifier_def, label_get(576)); // "Altitude"
                        }
                    } else {
                        int alt_mod_atk = (alt_atk - alt_def) * Rules->combat_pct_art_bonus_lvl_alt;
                        offense *= ((alt_mod_atk + 100) / 100);
                        add_bat(0, alt_mod_atk, label_get(576)); // "Altitude"
                    }
                }
            }
        }
    }
    if (offense_out) {
        *offense_out = offense;
    }
    if (defense_out) {
        *defense_out = defense;
    }
}

/*
Purpose: Determine the best defender in a stack.
// ORIGINAL: 0x005044D0 ?best_defender@@YAHHHH@Z 0x005044D0-0x00504A9B
// LEVER: MORE (14 calls vs image's 12) - per-callee counts matched the
//        image exactly (base_at 1, get_basic_offense 1, battle_compute 1,
//        stack_check 3, has_abil 3, can_arty 3 = 12) with two calls this
//        tree made that are not in the image's callee set at all:
//        veh_top() and proto_power(). veh_top() hand-inlined here, matching
//        spot_stack's precedent (its own record shows 0 real callers in the
//        whole image). proto_power() moved to veh.h as `MEASURED inline`
//        (see its own ORIGINAL marker) since it was a plain non-`inline`
//        function and so never an /Ob1 inline candidate at either of its
//        two callers. Call count now agrees (0 disagree); best flag set's
//        similarity 0.259 -> 0.275.
// symbol    ?best_defender@@YAIHHH@Z
// size      1483 bytes
// prototype int (__cdecl ?best_defender@@YAHHHH@Z)(int vehIDDef, int vehIDAtk, BOOL useArtillery)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x005015B0 0x00501DA0 0x005B9580 0x005BF1F0 0x005C0DB0
Return Value: Unit id of the best defender
Status: Complete - testing
*/
uint32_t __cdecl best_defender(int veh_id_def, int veh_id_atk, BOOL check_artillery) {
    int offense = veh_id_atk >= 0 
        ? get_basic_offense(veh_id_atk, veh_id_def, false, false, false) : 8;
    int x_def = Vehs[veh_id_def].x;
    int y_def = Vehs[veh_id_def].y;
    BOOL is_ocean_def = is_ocean(x_def, y_def);
    int base_id_def = base_at(x_def, y_def);
    int defender_search = -999;
    uint32_t best_def_veh_id = veh_id_def;
    // veh_top(), hand-inlined - matching spot_stack's precedent: image calls
    // it 0 times total (both its callers inline it).
    int top_veh_id;
    if (veh_id_def < 0) {
        top_veh_id = -1;
    } else {
        top_veh_id = veh_id_def;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
        uint32_t proto_id_def = Vehs[i].proto_id;
        if ((get_proto_triad(proto_id_def) != TRIAD_LAND || !is_ocean_def && base_id_def >= 0)
            // added veh_id_atk bounds check
            && (veh_id_atk < 0 || VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE
                || VehPrototypes[proto_id_def].plan == PLAN_INFO_WARFARE)) {
            uint32_t combat_type = (can_arty(proto_id_def, true) && (veh_id_atk 
                || can_arty(Vehs[veh_id_atk].proto_id, true))) ? 2 : 0; // added bounds?
            if (veh_id_atk >= 0 && get_triad(veh_id_atk) != TRIAD_AIR
                && has_abil(proto_id_def, ABL_AIR_SUPERIORITY)
                && !is_proto_missile(proto_id_def) && !is_missile(veh_id_atk)
                && get_offense_rating(veh_id_atk) > 0 && get_defense_rating(veh_id_atk) > 0
                && get_proto_offense_rating(proto_id_def) > 0) {
                combat_type |= 0xA; // air combat
                if (get_proto_triad(proto_id_def) == TRIAD_AIR) {
                    combat_type |= 0x10;
                }
            }
            int offense_output;
            int defense_output;
            battle_compute(veh_id_atk, i, &offense_output, &defense_output, combat_type);
            if (!offense_output) {
                break;
            }
            uint32_t proto_pwr_def = proto_power(i);
            int def_modifier = (((((range(proto_pwr_def - Vehs[i].dmg_incurred, 0, 9999)
                * defense_output) / proto_pwr_def) * offense) / offense_output) / 8)
                - get_proto_offense_rating(proto_id_def);
            uint32_t plan_def = VehPrototypes[proto_id_def].plan;
            if (plan_def < PLAN_COLONIZATION || plan_def == PLAN_TERRAFORMING) {
                def_modifier *= 16;
            }
            if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_AIR 
                && has_abil(proto_id_def, ABL_AIR_SUPERIORITY) 
                && get_proto_triad(proto_id_def) == TRIAD_AIR 
                && !is_proto_missile(proto_id_def) && !is_missile(veh_id_atk)) {
                def_modifier += 0x80000;
            } else if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_AIR
                && has_abil(Vehs[veh_id_atk].proto_id, ABL_AIR_SUPERIORITY)
                && get_proto_triad(proto_id_def) == TRIAD_AIR && base_id_def < 0
                && !(bit_at(x_def, y_def) & BIT_AIRBASE)
                && !stack_check(veh_id_def, 6, ABL_CARRIER, -1, -1)
                && !is_proto_missile(proto_id_def) && !is_missile(veh_id_atk)) {
                def_modifier += 0x80000;
            } else if (check_artillery) {
                if (can_arty(proto_id_def, true)) {
                    def_modifier += 0x80000;
                }
            } else if ((!stack_check(veh_id_def, 3, TRIAD_AIR, -1, -1) || base_id_def >= 0 
                || bit_at(x_def, y_def) & BIT_AIRBASE 
                || stack_check(veh_id_def, 6, ABL_CARRIER, -1, -1)) 
                && Vehs[i].state & VSTATE_DESIGNATE_DEFENDER) {
                def_modifier += 0x80000;
            }
            int defense = i + (def_modifier << 11);
            if (defense > defender_search) {
                defender_search = defense;
                best_def_veh_id = i;
            }
        }
    }
    return best_def_veh_id;
}

/*
Purpose: Determine whether any enemy naval transports are carrying land units within range to 
         attack the specified base. If so, set the units to move towards the base.
// ORIGINAL: 0x00506490 ?invasions@@YAXH@Z 0x00506490-0x00506646
// RULED-OUT: call structure already matches exactly - `osmx calls --all`
//            lists veh_cargo, stack_check, four abs() (vector_dist/x_dist
//            inlined) and speed, in the image's own order, so vector_dist's
//            4-arg overload IS inlining here despite being declared plain
//            (not `inline`) in map.h; the divergence is pure register
//            allocation/spill-slot choice across a big function (7 locals:
//            base_x, base_y, faction_id_base, i, veh_x, veh_y, proximity),
//            not a missing inline or a chained-condition shape. Best flag
//            set /O2 /Gy /GR- /Oy- /GX, 0.746 similar, 11/143 raw. map.h is
//            out of scope for this batch regardless.
// size      438 bytes
// prototype void (__cdecl ?invasions@@YAXH@Z)(int baseID)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9580 0x005C1540 0x005C1760 0x00644F3A
Return Value: n/a
Status: Complete
*/
void __cdecl invasions(int base_id) {
    uint32_t faction_id_base = Bases[base_id].faction_id_current;
    int16_t base_x = Bases[base_id].x;
    int16_t base_y = Bases[base_id].y;
    for (int i = 0; i < VehCurrentCount; i++) {
        uint32_t veh_faction_id = Vehs[i].faction_id;
        if (veh_faction_id && !is_human(veh_faction_id) && veh_faction_id != faction_id_base
            && !has_treaty(veh_faction_id, faction_id_base, DTREATY_TREATY)) {
            int veh_x = Vehs[i].x;
            int veh_y = Vehs[i].y;
            if (is_ocean(veh_x, veh_y) && veh_cargo(i) && stack_check(i, 3, TRIAD_LAND, -1, -1)) {
                int proximity = vector_dist(base_x, base_y, veh_x, veh_y);
                if (proximity <= (int)speed(i, false)) {
                    Vehs[i].order = ORDER_MOVE_TO;
                    Vehs[i].waypoint_x[0] = base_x;
                    Vehs[i].waypoint_y[0] = base_y;
                }
            }
        }
    }
}

/*
Purpose: Direct the unit to start moving automatically towards the specified tile.
// ORIGINAL: 0x00560AD0 ?go_to@@YAXHDHH@Z 0x00560AD0-0x00560B25 BYTE_EXACT
// size      85 bytes
// prototype void (__cdecl ?go_to@@YAXHDHH@Z)(int vehID, int8 type, int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: set_course's two calls to go_to are
// both inlined in the image (its own `calls` list has neither), while
// scenario.cpp's single call stays real - `osmx calls` on set_course names
// no go_to among its targets. LEVER: matches bitmask/battle_init above.

/*
Purpose: Wrap an x coordinate on a round map, as set_course does it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

xrange() at 0048BEE0 cannot stand in, for one bit: it tests the whole of MapIsFlat while
set_course reads the low BYTE and tests bit zero (`mov cl, byte ptr [94988Ch] / test cl, 1`),
the same reading valid_patrol above and reset_territory use. The two agree on the 0 and 1 the
game stores there and disagree on everything else.
*/
static inline int course_xrange(int x) {
    if (!(MapIsFlat & 1)) {
        if (x >= 0) {
            if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        } else {
            x += MapLongitudeBounds;
        }
    }
    return x;
}

/*
Purpose: Send the unit toward the given tile the long way round - by water - choosing the anchorage
         next to it that fronts the most of the destination's own landmass.
// ORIGINAL: 0x00564890 ?set_course@@YAXHDHH@Z 0x00564890-0x00564B82
// RULED-OUT: call structure already matches (`osmx calls --all` lists only
//            base_at and base_on_sea, same as the image's own `calls` list),
//            so the gap is register/control-flow noise across a 754-byte,
//            21-candidate scoring loop, not a missing inline or a wrong
//            callee. Best flag set /O2 /Ob0 /Gy /GR- /Oy- /GX, 0.558
//            similar. Not chased further in this pass - the semantic content
//            is already verified (mutant sweep kills 28/30, two equivalent).
// size      754 bytes
// prototype void (__cdecl ?set_course@@YAXHDHH@Z)(int vehID, int8 type, int xCoord, int yCoord)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x0050DE50
Return Value: n/a
Status: Complete

The direct answer comes first and is narrow: only when the destination is in the SAME region as
the unit AND the unit is itself afloat - its own tile below ALT_BIT_SHORE_LINE - does this hand
the coordinates straight to go_to. Anything else is a crossing, and the rest of the body picks
where to cross to.

The candidates are the 21 tiles of the destination's base radius, and each must be water. Which
water is decided by what the unit is sitting in: a unit inside a base asks base_on_sea whether
that base touches the candidate's sea region, and a unit at sea requires the candidate to be in
its own region. That is the reachability test, and it is the only one - nothing here checks
distance or fuel.

Each surviving candidate is then scored by FRONTAGE: how many of its eight neighbours are land,
belong to the DESTINATION's region, and hold no base. Land is the same ALT_BIT_SHORE_LINE line
read the other way round, the region is the destination's rather than the candidate's, and the
base test is base_who() - which answers -1 both for an empty tile and for a base tile whose owner
nibble is 0xF, so only a real faction-owned base takes a neighbour out of the count.

The best frontage wins, and the starting bar depends on the destination itself: zero when the
destination is land, minus one when it is water. A water destination therefore accepts an
anchorage that fronts NOTHING, which is what lets a unit be sent to open sea at all; a land
destination will not move the unit unless some anchorage fronts at least one tile of it.

Ties keep the earlier candidate, and every improvement re-issues the order, so the unit ends up
aimed at the last candidate that strictly beat everything before it.

The x wrap is course_xrange above rather than xrange(), for the one bit that separates them.

Verification note: the sweep against recovery-gameplay-tests kills 28 of 30 valid mutants twice
over, and both survivors are equivalences rather than untested behaviour.

  - Widening the neighbour loop to nine reaches RadiusBase entry 8, which is (0, 0) - the
    anchorage itself. Three lines earlier the anchorage was required to be below
    ALT_BIT_SHORE_LINE, and a neighbour is required to be at or above it, so that iteration can
    never reach the count no matter what the map holds.
  - Swapping `best_frontage = frontage;` with the go_to call behind it. go_to writes Veh fields
    and reads Vehs and VehPrototypes; the assignment reads a local the callee cannot name and
    writes a local the callee cannot see. Neither statement observes the other's effect, in
    either order.
*/
void __cdecl set_course(int veh_id, char type, int x, int y) {
    Veh &veh = Vehs[veh_id];
    int veh_region = map_loc(veh.x, veh.y)->region;
    int base_id = base_at(veh.x, veh.y);
    Map *destination = map_loc(x, y);
    int dst_region = destination->region;
    if (veh_region == dst_region
        && (map_loc(veh.x, veh.y)->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
        go_to(veh_id, type, x, y);
        return;
    }
    int best_frontage = ((destination->climate & 0xE0) >= ALT_BIT_SHORE_LINE) ? 0 : -1;
    for (int i = 0; i < 21; i++) {   // the destination's own base radius
        int x_radius = course_xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius)) {
            continue;
        }
        Map *anchorage = map_loc(x_radius, y_radius);
        if ((anchorage->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
            continue;
        }
        if (base_id >= 0) {
            if (!base_on_sea(base_id, anchorage->region)) {
                continue;
            }
        } else if (anchorage->region != veh_region) {
            continue;
        }
        int frontage = 0;
        for (int j = 0; j < 8; j++) {
            int x_adj = course_xrange(x_radius + RadiusBaseX[j]);
            int y_adj = y_radius + RadiusBaseY[j];
            if (!on_map(x_adj, y_adj)) {
                continue;
            }
            Map *shore = map_loc(x_adj, y_adj);
            if ((shore->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
                continue;
            }
            if (shore->region != dst_region) {
                continue;
            }
            if (base_who(x_adj, y_adj) >= 0) {
                continue;
            }
            frontage++;
        }
        if (frontage > best_frontage) {
            best_frontage = frontage;
            go_to(veh_id, type, x_radius, y_radius);
        }
    }
}

/*
Purpose: Get the unit on the top of the stack.
// ORIGINAL: 0x00579920 ?veh_top@@YAHH@Z 0x00579920-0x0057995A BYTE_EXACT
// LEVER: the climb loop's induction variable is int16_t, not int - the field
//   (prev_veh_id_stack) is int16_t, and declaring the loop variable int forces
//   an immediate movsx the image defers until the value is used as an index.
// size      58 bytes
// prototype int (__cdecl ?veh_top@@YAHH@Z)(int vehID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Unit id if found, otherwise -1
Status: Complete
*/
int __cdecl veh_top(int veh_id) {
    if (veh_id < 0) {
        return -1;
    }
    int top_veh_id = veh_id;
    for (int16_t i = Vehs[top_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
        top_veh_id = i;
    }
    return top_veh_id;
}

/*
Purpose: Get the current moves left for the specified unit.
// ORIGINAL: 0x00579960 ?veh_moves@@YAHH@Z 0x00579960-0x00579998 BYTE_EXACT
// size      56 bytes
// prototype int (__cdecl ?veh_moves@@YAHH@Z)(int vehID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540
Return Value: Remaining moves
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Get the specified unit's reactor power value from its prototype.
// ORIGINAL: 0x005799A0 ?proto_power@@YAHH@Z 0x005799A0-0x005799F9 BYTE_EXACT
// LEVER: neither of its two callers (best_defender 0x5044D0, action_home
//        0x4CBAA0) has it in their `calls` list - both inline it - but this
//        tree defined it as a plain (non-`inline`) function, so under /Ob1
//        (the default for every flag set but /Ob0) it was never a candidate
//        and both callers paid a real call the image does not make. `MEASURED
//        inline` keeps this address's own BYTE_EXACT claim measurable
//        (the /Ob0 flag sets still force it real for that purpose).
// size      89 bytes
// prototype int (__cdecl ?proto_power@@YAHH@Z)(int vehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Power
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Determine whether the specified unit is eligible for a monolith morale upgrade.
// ORIGINAL: 0x00579F80 ?want_monolith@@YAHH@Z 0x00579F80-0x00579FE6 BYTE_EXACT
// size      102 bytes
// prototype int (__cdecl ?want_monolith@@YAHH@Z)(int vehID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C0E40
Return Value: Is unit eligible for a monolith morale upgrade? true/false
Status: Complete
*/
BOOL __cdecl want_monolith(int veh_id) {
    if (!(Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED)
        && morale_veh(veh_id, true, 0) < MORALE_ELITE && Vehs[veh_id].morale < MORALE_ELITE
        && get_offense_rating(veh_id)) {
        return true;
    }
    return false;
}

/*
Purpose: Calculate the armor strategy for the specified armor id.
// ORIGINAL: 0x0057D270 ?arm_strat@@YAHHH@Z 0x0057D270-0x0057D2D5
// RULED-OUT: still 0.987 similar, one instruction short of exact - the image folds `armor_id*16 + 0x94F280` into `shl eax,4` plus a displaced `mov`; this tree pre-adds the base (`add eax,0x94F28` then `shl eax,4`) under every flag set tried. `(Armor+armor_id)->defense_rating`, a local `RulesArmor *`, and explicit pointer-arithmetic-plus-cast all produced the identical extra `add`.
// RULED-OUT: defense_rating` (not `int`) matches the image's lazy sign-extension - it tests the byte directly and only does `movsx eax, al` right before the final return; casting both `Rules->psi_combat_ratio_*` operands to `(int)` at the call site gets the `idiv`/`cdq` the image uses instead of unsigned `div` (both fields are `uint32_t` in the header, out of scope for this file).
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
        return psi_factor(((int)Rules->psi_combat_ratio_def[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_weapon_value)
            / (int)Rules->psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
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
        return psi_factor(((int)Rules->psi_combat_ratio_atk[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_armor_value)
            / (int)Rules->psi_combat_ratio_def[TRIAD_LAND], faction_id, true, false);
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
        return psi_factor(((int)Rules->psi_combat_ratio_atk[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_armor_value)
            / (int)Rules->psi_combat_ratio_def[TRIAD_LAND], faction_id, true, false);
    }
    return offense_rating;
}

/*
Purpose: Calculate the armor value for the specified armor id.
// ORIGINAL: 0x0057D3F0 ?arm_val@@YAHHH@Z 0x0057D3F0-0x0057D471
// size      129 bytes
// prototype int (__cdecl ?arm_val@@YAHHH@Z)(int armorID, int factionID)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00501500
// LEVER: WRONG CALLEE - arm_strat() (0x0057D270, own out-of-line function)
//        is hand-inlined at this site, matching the image's one call
//        (psi_factor), not arm_strat. Best similarity 0.980 (14->44 of 48).
// RULED-OUT: the residual 2-instruction gap is the SAME `Armor[armor_id]
//        .defense_rating` base-fold quirk already documented and left as-is
//        on arm_strat itself (0x0057D270) - the image folds
//        `armor_id*16 + 0x94F280` into `shl eax,4` + a displaced `mov`,
//        this tree's toolchain pre-adds the base under every flag set.
//        Carried into this hand-inlined copy for free; not re-chased here.
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
                defense = psi_factor(((int)Rules->psi_combat_ratio_def[TRIAD_LAND]
                    * PlayersData[faction_id].enemy_best_weapon_value)
                    / (int)Rules->psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
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
// ORIGINAL: 0x0057D480 ?armor_val@@YAHHH@Z 0x0057D480-0x0057D50D
// size      141 bytes
// prototype int (__cdecl ?armor_val@@YAHHH@Z)(int protoID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00501500
// LEVER: WRONG CALLEE - arm_val() (0x0057D3F0) and arm_strat() (0x0057D270)
//        are both hand-inlined at this site, matching the image's one call
//        (psi_factor), not arm_val. Best similarity 0.981.
// RULED-OUT: same `Armor[armor_id].defense_rating` base-fold quirk as
//        arm_strat/arm_val (see arm_val's own note above) - not re-chased
//        here either.
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
                defense = psi_factor(((int)Rules->psi_combat_ratio_def[TRIAD_LAND]
                    * PlayersData[faction_id].enemy_best_weapon_value)
                    / (int)Rules->psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
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
Purpose: Calculate the carrying/transport capacity for the specified chassis, abilities and reactor.
// ORIGINAL: 0x0057D510 ?transport_val@@YAHHHH@Z 0x0057D510-0x0057D556 BYTE_EXACT
// LEVER: uint32_t -> int; image uses signed division (cdq/sub/sar) for `/2`, not shr
// size      70 bytes
// prototype int (__cdecl ?transport_val@@YAHHHH@Z)(int chassis, int ability, int reactor)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Transport capacity
Status: Complete
*/
int __cdecl transport_val(int chassis_id, int ability, int reactor_id) {
    int transport = Chassis[chassis_id].cargo;
    if (Chassis[chassis_id].triad == TRIAD_SEA) {
        transport *= reactor_id;
    }
    if (ability & ABL_SLOW) {
        transport /= 2; // -50%, rounded down
    }
    if (ability & ABL_HEAVY_TRANSPORT) {
        transport = (3 * transport + 1) / 2; // +50%, rounded up
    }
    return transport;
}

/*
Purpose: Generate offensive display string for the specified prototype. Replaced existing non-safe
         strcat with string. Original function took a 2nd parameter with char buffer and didn't
         return anything.
// ORIGINAL: 0x0057D560 ?say_offense@@YAXPADH@Z 0x0057D560-0x0057D6C5 BYTE_EXACT
// size      357 bytes
// prototype void (__cdecl ?say_offense@@YAXPADH@Z)(int8*, int protoID)
// callers   3   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C0DB0 0x005C1150 0x00645470 0x0064FC88
Return Value: Offensive display string
Status: Complete
*/
void __cdecl say_offense(LPSTR stat, int proto_id) {
    char num_buf[80];
    if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
        strcat(stat, "<");
    }
    if (can_arty(proto_id, true)) {
        strcat(stat, "(");
    }
    int off_rating = offense_proto(proto_id, -1, 0);
    _itoa(off_rating, num_buf, 10);
    strcat(stat, num_buf);
    if (has_abil(proto_id, ABL_DROP_POD)) {
        strcat(stat, "^");
    }
    if (has_abil(proto_id, ABL_AMPHIBIOUS)) {
        strcat(stat, "~");
    }
    if (has_abil(proto_id, ABL_NERVE_GAS)) {
        strcat(stat, "x");
    }
    if (has_abil(proto_id, ABL_EMPATHIC)) {
        strcat(stat, "e");
    }
    if (has_abil(proto_id, ABL_BLINK_DISPLACER)) {
        strcat(stat, "!");
    }
    if (can_arty(proto_id, true)) {
        strcat(stat, ")");
    }
    if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
        strcat(stat, ">");
    }
    uint8_t weapon_id = VehPrototypes[proto_id].weapon_id;
    if (weapon_id == WPN_RESONANCE_LASER || weapon_id == WPN_RESONANCE_BOLT) {
        strcat(stat, "r");
    }
}

/*
Purpose: Generate defense display string for the specified prototype. Replaced existing non-safe 
         strcat with string. Original function took a 2nd parameter with char buffer and didn't 
         return anything.
// ORIGINAL: 0x0057D6D0 ?say_defense@@YAXPADH@Z 0x0057D6D0-0x0057D7C4 BYTE_EXACT
// size      244 bytes
// prototype void (__cdecl ?say_defense@@YAXPADH@Z)(int8*, int protoID)
// callers   3   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C1290 0x00645470 0x0064FC88
Return Value: Defense display string
Status: Complete
*/
void __cdecl say_defense(LPSTR stat, int proto_id) {
    char num_buf[80];
    if (has_abil(proto_id, ABL_AAA)) {
        strcat(stat, "<");
    }
    int def_rating = armor_proto(proto_id, -1, 0);
    _itoa(def_rating, num_buf, 10);
    strcat(stat, num_buf);
    if (has_abil(proto_id, ABL_COMM_JAMMER)) {
        strcat(stat, "+");
    }
    if (has_abil(proto_id, ABL_TRANCE)) {
        strcat(stat, "t");
    }
    if (has_abil(proto_id, ABL_AAA)) {
        strcat(stat, ">");
    }
    if (VehPrototypes[proto_id].armor_id == ARM_PULSE_3_ARMOR
        || VehPrototypes[proto_id].armor_id == ARM_PULSE_8_ARMOR) {
        strcat(stat, "p");
    }
    if (VehPrototypes[proto_id].armor_id == ARM_RESONANCE_3_ARMOR
        || VehPrototypes[proto_id].armor_id == ARM_RESONANCE_8_ARMOR) {
        strcat(stat, "r");
    }
}

/*
Purpose: Generate stats string for specified prototype. Replaced existing non-safe strcat with 
         string. Reworked to integrate with existing C code.
// ORIGINAL: 0x0057D7D0 ?say_stats_3@@YAXPADH@Z 0x0057D7D0-0x0057D8D3
// RULED-OUT: else-if branch order off_rating<0/off_rating<99/else folded the "?"
//            and "*" strcat tails together (29/92); reordering to
//            off_rating<0/off_rating>=99/else (matching the disasm's own branch
//            order) got to 51/92. def_rating stored to a named int8_t local, or
//            inlined as Armor[...].armor_id].defense_rating, made no further
//            difference; plateaus on the def_rating address computation
//            (shl 4 + disp vs add const + shl 4), 4 spellings tried.
// size      259 bytes
// prototype void (__cdecl ?say_stats_3@@YAXPADH@Z)(int8*, int protoID)
// callers   13   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats_3(LPSTR stat, int proto_id) {
    char num_buf[80];
    int8_t off_rating = get_proto_offense_rating(proto_id);
    if (off_rating < 0) {
        strcat(stat, "?"); // PSI
    } else if (off_rating >= 99) {
        strcat(stat, "*");
    } else {
        say_offense(stat, proto_id);
    }
    strcat(stat, "-");
    if (get_proto_defense_rating(proto_id) < 0) {
        strcat(stat, "?");
    } else {
        say_defense(stat, proto_id);
    }
    strcat(stat, "-");
    _itoa(speed_proto(proto_id) / Rules->move_rate_roads, num_buf, 10);
    strcat(stat, num_buf);
    uint8_t reactor = VehPrototypes[proto_id].reactor_id;
    if (reactor > RECT_FISSION) {
        strcat(stat, "*");
        _itoa(reactor, num_buf, 10);
        strcat(stat, num_buf);
    }
}

/*
Purpose: Generate prototype stats using stringTemp buffer.
// ORIGINAL: 0x0050B9A0 ?say_stats_3@@YAXH@Z 0x0050B9A0-0x0050B9B6 BYTE_EXACT
// size      22 bytes
// prototype void (__cdecl ?say_stats_3@@YAXH@Z)(int protoID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D7D0
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats_3(int proto_id) {
    say_stats_3(StringTemp, proto_id);
}

/*
Purpose: Generate stats string for specified prototype. List whether prototype is psi, sea or air.
         Replaced existing non-safe strcat with string. Reworked to integrate with existing C code.
// ORIGINAL: 0x0057D8E0 ?say_stats_2@@YAXPADH@Z 0x0057D8E0-0x0057DA94
// RULED-OUT: label_get(196/163/162) calls (25/140); open-coding
//            StringTable->get((int)*((LPSTR*)Labels->strings_ptr + N)) plus the
//            off_rating<0/>=99/else branch order got to 58/140. def_rating via
//            get_proto_defense_rating() vs inlined Armor[...] made no further
//            difference; plateaus on the same shl-vs-add-const address split
//            seen at 0x0057D7D0. Call count is 19/21 (call_diff): same
//            get(Labels[196])+strcat 'Psi' CSE across the off_rating<0 and
//            def_rating<0 arms as 0x0057DAA0 - the compiler shares the pair,
//            the image keeps two call sites.
// size      436 bytes
// prototype void (__cdecl ?say_stats_2@@YAXPADH@Z)(int8*, int protoID)
// callers   4   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x006169A0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats_2(LPSTR stat, int proto_id) {
    char num_buf[80];
    int8_t off_rating = get_proto_offense_rating(proto_id);
    if (off_rating < 0) {
        // INLINE: the image open-codes label_get(196) here rather than calling it.
        strcat(stat, StringTable->get((int)*((LPSTR *)Labels->strings_ptr + 196))); // 'Psi'
    } else if (off_rating >= 99) {
        strcat(stat, "*");
    } else {
        say_offense(stat, proto_id);
    }
    strcat(stat, "-");
    if (get_proto_defense_rating(proto_id) < 0) {
        strcat(stat, StringTable->get((int)*((LPSTR *)Labels->strings_ptr + 196))); // 'Psi'
    } else {
        say_defense(stat, proto_id);
    }
    strcat(stat, "-");
    _itoa(speed_proto(proto_id) / Rules->move_rate_roads, num_buf, 10);
    strcat(stat, num_buf);
    uint32_t triad = get_proto_triad(proto_id);
    if (triad == TRIAD_SEA) {
        strcat(stat, " ");
        strcat(stat, StringTable->get((int)*((LPSTR *)Labels->strings_ptr + 163))); // 'Sea'
    } else if (triad == TRIAD_AIR) {
        strcat(stat, " ");
        strcat(stat, StringTable->get((int)*((LPSTR *)Labels->strings_ptr + 162))); // 'Air'
    }
    uint32_t reactor = VehPrototypes[proto_id].reactor_id;
    if (reactor > 1) {
        strcat(stat, "*");
        _itoa(reactor, num_buf, 10);
        strcat(stat, num_buf);
    }
}

/*
Purpose: Generate verbose stats string for specified prototype. Used by Design Workshop and Military
         Command Nexus. Replaced existing non-safe strcat with string. Reworked to integrate with
         existing C code.
// ORIGINAL: 0x0057DAA0 ?say_stats@@YAXPADHPAD@Z 0x0057DAA0-0x0057DED8
// symbol    ?say_stats@@YAXPADH0@Z
// RULED-OUT: std::string body called 82 vs the image's 44 (call_diff); rebuilt
//            as char pointer + strcat/sprintf in the image's own branch shape (goto
//            shared "defense+spacer+speed" tail, matching the two physical
//            jumps into 0x57dd70) got the call count to 42/44. label_get(N)
//            calls had to become the open-coded
//            StringTable->get((int)*((LPSTR*)Labels->strings_ptr + N)) - the
//            image never calls a separate label helper here. The
//            `custom_spacer ? custom_spacer : "/"` after the offense append
//            is two strcat call sites in the image (if/else, not a ternary
//            merged into one call); the same spacer after the defense append
//            IS one call site (a shared "push ebx or literal, then push+call"
//            tail) - only the first needed splitting.
//            Still 2 calls short: the compiler CSEs the two identical
//            `StringTable->get(Labels[196])` + strcat 'Psi' pairs (one on the
//            off_rating<0 arm, one on the def_rating<0 arm) into one shared
//            block, where the image keeps them as two separate call sites.
//            No source spelling tried defeated that merge. Also plateaus on
//            prologue shape (push ebp/mov ebp,esp/sub esp,0x50 vs an
//            esp-relative frame) picked by the flag search's own similarity
//            metric; forcing /Oy- narrows but does not close it.
// size      1080 bytes
// prototype void (__cdecl ?say_stats@@YAXPADHPAD@Z)(int8*, int protoID, int8*)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x006169A0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats(LPSTR stat, int proto_id, LPSTR custom_spacer) {
    char num_buf[80];
    uint8_t plan = VehPrototypes[proto_id].plan;
    uint8_t chas = VehPrototypes[proto_id].chassis_id;
    uint8_t triad = Chassis[chas].triad;
    uint8_t mode = Weapon[VehPrototypes[proto_id].weapon_id].mode;
    int8_t off_rating = get_proto_offense_rating(proto_id);
    int8_t def_rating = get_proto_defense_rating(proto_id);
    if (plan == PLAN_RECONNAISANCE && triad == TRIAD_LAND && off_rating == 1 && def_rating == 1
        && !VehPrototypes[proto_id].ability_flags) {
        strcat(stat, StringTable->get((int)PlansFullName[3])); // 'Explore/Defense'
        strcat(stat, ", ");
    } else if (mode < 3) { // Projectile, energy, missile
        strcat(stat, StringTable->get((plan != PLAN_DEFENSIVE || (off_rating >= 0 && off_rating <= def_rating))
            ? (int)PlansShortName[plan] : (int)*((LPSTR *)Labels->strings_ptr + 312))); // 'Combat'
        strcat(stat, ", ");
    }
    if (off_rating < 0 || mode < 3) {
        if (off_rating < 0) {
            strcat(stat, StringTable->get((int)*((LPSTR *)Labels->strings_ptr + 196))); // 'Psi'
        } else {
            say_offense(stat, proto_id);
        }
        if (custom_spacer) {
            strcat(stat, custom_spacer);
        } else {
            strcat(stat, "/");
        }
        goto append_defense_tail;
    } else if (def_rating != 1 || VehPrototypes[proto_id].ability_flags || (Chassis[chas].speed != 1
        && (mode != WPN_MODE_TRANSPORT || chas != CHSI_FOIL))) {
        strcat(stat, StringTable->get(int(PlansShortName[mode])));
        if (plan == PLAN_NAVAL_TRANSPORT) {
            strcat(stat, "(");
            _itoa(VehPrototypes[proto_id].carry_capacity, num_buf, 10);
            strcat(stat, num_buf);
            strcat(stat, ")");
        }
        strcat(stat, ", ");
append_defense_tail:
        if (def_rating < 0) {
            strcat(stat, StringTable->get((int)*((LPSTR *)Labels->strings_ptr + 196))); // 'Psi'
        } else {
            say_defense(stat, proto_id);
        }
        strcat(stat, custom_spacer ? custom_spacer : "/");
        _itoa(speed_proto(proto_id) / Rules->move_rate_roads, num_buf, 10);
        strcat(stat, num_buf);
    } else {
        strcat(stat, StringTable->get((int)PlansFullName[mode]));
        if (plan == PLAN_NAVAL_TRANSPORT) {
            strcat(stat, "(");
            _itoa(VehPrototypes[proto_id].carry_capacity, num_buf, 10);
            strcat(stat, num_buf);
            strcat(stat, ")");
        }
        if (triad) { // sea, air
            strcat(stat, ","); // Bug fix: removed extra space
        }
    }
    if (triad == TRIAD_SEA) {
        strcat(stat, " ");
        strcat(stat, StringTable->get((int)*((LPSTR *)Labels->strings_ptr + 163))); // 'Sea'
    } else if (triad == TRIAD_AIR) {
        strcat(stat, " ");
        strcat(stat, StringTable->get((int)*((LPSTR *)Labels->strings_ptr + 162))); // 'Air'
    }
    uint32_t reactor = VehPrototypes[proto_id].reactor_id;
    if (reactor > 1) {
        strcat(stat, " ");
        strcat(stat, "(");
        strcat(stat, "*");
        _itoa(reactor, num_buf, 10);
        strcat(stat, num_buf);
        strcat(stat, ")");
    }
}

/*
Purpose: Check against faction's available tech for best available reactor.
// ORIGINAL: 0x0057EFA0 ?best_reactor@@YAHH@Z 0x0057EFA0-0x0057EFE2 BYTE_EXACT
// symbol    ?best_reactor@@YAIH@Z
// size      66 bytes
// prototype int (__cdecl ?best_reactor@@YAHH@Z)(int factionID)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best reactor available (1-4)
Status: Complete
*/
uint32_t __cdecl best_reactor(int faction_id) {
    for (int i = MaxReactorNum - 1; i >= 0; i--) {
        if (has_tech(Reactor[i].preq_tech, faction_id)) {
            return i + 1;
        }
    }
    return RECT_FISSION; // default
}

/*
Purpose: Check against faction's available tech for best available chassis meeting requirements.
// ORIGINAL: 0x0057EFF0 ?pick_chassis@@YAHHHH@Z 0x0057EFF0-0x0057F0A1 BYTE_EXACT
// LEVER: declaration/store order (best_speed before chassis_id); comparison operand
//   order (`Chassis[i].triad == triad_chk`, not the reverse - the `cmp` LHS follows
//   source LHS); swapped the `speed_chk > 0`/`speed_chk <= 0` if/else so the `>0`
//   arm is checked first, matching the image's fallthrough; `continue` in place of
//   each branch's own duplicated `if (speed_compare > best_speed)` so the check
//   merges into the ONE shared block the image jumps to from both arms; and the
//   `speed_chk <= 0` arm rewritten as the image's own fallback chain (TRIAD_AIR /
//   -1-needs-missile / -2-needs-range==1 / else-compute) instead of a flat `||` -
//   the image has no explicit `!speed_chk` test, it just falls through once -1 and
//   -2 are ruled out.
// size      177 bytes
// prototype int (__cdecl ?pick_chassis@@YAHHHH@Z)(int factionID, int triadChk, int speedChk)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best chassis available (0-8)
Status: Complete
*/
int __cdecl pick_chassis(int faction_id, int triad_chk, int speed_chk) {
    int best_speed = 0;
    int chassis_id = -1;
    for (int i = 0; i < MaxChassisNum; i++) {
        if (has_tech(Chassis[i].preq_tech, faction_id) && (triad_chk < 0
            || Chassis[i].triad == triad_chk) && (!Chassis[i].missile || speed_chk == -1)) {
            int speed_compare = 1;
            // The image reaches the best_speed check below through ONE shared block, from
            // either branch; where the source used to duplicate the check in each branch
            // (forcing the compiler to regenerate it twice, with different registers each
            // time), `continue` on the branch's own failure reproduces the image's direct
            // jump straight to the loop increment, letting the check merge into one copy.
            if (speed_chk > 0) { // 1, 2, 3
                if (Chassis[i].speed >= speed_chk) {
                    if (Chassis[i].speed == speed_chk) {
                        speed_compare = 2;
                    }
                } else {
                    continue;
                }
            } else { // 0, -1, -2
                // Written as the image's own fallback chain, not a flat `||`: when
                // triad_chk == TRIAD_AIR, speed_chk == -1 needs Chassis[i].missile and
                // speed_chk == -2 needs Chassis[i].range == 1; anything else (including
                // the assumed speed_chk == 0) falls through to compute unconditionally,
                // with no explicit `!speed_chk` test.
                if (triad_chk == TRIAD_AIR) {
                    if (speed_chk == -1) {
                        if (!Chassis[i].missile) {
                            continue;
                        }
                    } else if (speed_chk == -2) {
                        if (Chassis[i].range != 1) {
                            continue;
                        }
                    }
                }
                speed_compare = Chassis[i].speed + 1;
            }
            if (speed_compare > best_speed) {
                best_speed = speed_compare;
                chassis_id = i;
            }
        }
    }
    return chassis_id;
}

/*
Purpose: Check against faction's available tech for the best available weapon meeting requirements.
         The condition variable has a dual purpose of either max cost or search for 1st instance of
         a particular weapon mode.
// ORIGINAL: 0x0057F0B0 ?weapon_budget@@YAHHHH@Z 0x0057F0B0-0x0057F142 BYTE_EXACT
// LEVER: declaration/store order (best_offense before weapon_id, mirroring
//   pick_chassis's best_speed/chassis_id pair); swapped the `condition >= 0`/
//   `condition < 0` if/else so the `< 0` (break) arm is checked first, matching
//   the image's fallthrough; and `Weapon[i].mode < 2` spelled as `<= 1` - both
//   compile to the same `cmp byte, 1`, but only the `<=` spelling picks the `ja`
//   the image emits instead of `jae`.
// size      146 bytes
// prototype int (__cdecl ?weapon_budget@@YAHHHH@Z)(int factionID, int condition, BOOL checkMode)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best weapon available (0-25)
Status: Complete
*/
int __cdecl weapon_budget(int faction_id, int condition, BOOL check_mode) {
    int best_offense = -1;
    int weapon_id = 0;
    for (int i = 0; i < MaxWeaponNum; i++) {
        if (has_tech(Weapon[i].preq_tech, faction_id)) {
            if (condition < 0) {
                if (Weapon[i].mode == -condition) {
                    weapon_id = i;
                    break;
                }
            } else {
                if ((!check_mode || Weapon[i].mode <= 1) && Weapon[i].cost <= condition
                    && Weapon[i].mode < 3 && Weapon[i].offense_rating < 99
                    && i != 23) { // Exclude Conventional Payload
                    int offense_compare = Weapon[i].offense_rating * 2;
                    if (offense_compare >= best_offense) {
                        best_offense = offense_compare;
                        weapon_id = i;
                    }
                }
            }
        }
    }
    return weapon_id;
}

/*
Purpose: Check against the faction's available tech for the best available armor meeting the cost 
         requirement.
// ORIGINAL: 0x0057F150 ?armor_budget@@YAHHH@Z 0x0057F150-0x0057F1CF
// LEVER: store order - best_defense=-1 before armor_id=0, opposite of
//        declaration order (0.9524 -> 0.971 similar). RULED-OUT: remaining
//        divergence starts at the has_tech() call site (instr 17); not
//        chased further.
// size      127 bytes
// prototype int (__cdecl ?armor_budget@@YAHHH@Z)(int factionID, int maxCost)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best armor available (0-13)
Status: Complete
*/
int __cdecl armor_budget(int faction_id, int max_cost) {
    int best_defense = -1;
    int armor_id = 0;
    for (int i = 0; i < MaxArmorNum; i++) {
        if (has_tech(Armor[i].preq_tech, faction_id) && Armor[i].cost <= max_cost
            && Armor[i].defense_rating >= 0) { // excludes PSI
            int defense_compare = Armor[i].defense_rating;
            if (faction_id >= 0 && !ExpansionEnabled && i > 9) {
                defense_compare = 1; // Pulse + Resonance in non-expansion mode
            }                       // This really only affects Pulse 8 Armor (Super Tensile Solids)
            defense_compare *= 2;
            if (defense_compare >= best_defense) {
                armor_id = i;
                best_defense = defense_compare;
            }
        }
    }
    return armor_id;
}

/*
Purpose: Get the index value of a particular ability's bitfield.
// ORIGINAL: 0x00581170 ?abil_index@@YAHH@Z 0x00581170-0x00581187 BYTE_EXACT
// size      23 bytes
// prototype int (__cdecl ?abil_index@@YAHH@Z)(int abilityID)
// callers   7   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Ability index
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Calculate the movement penalty/cost.
// ORIGINAL: 0x00593510 ?hex_cost@@YAHHHHHHHH@Z 0x00593510-0x0059382F
// RULED-OUT: tree's `proto_id != BSC_SEALURK // Bug fix` term has no matching `cmp` in the image - only ONE proto_id compare (`cmp eax, 9`, BSC_ISLE_OF_THE_DEEP) exists on this path. Removing the extra term is a structural fix (matches the image's condition), not a byte win by itself: 7/308 -> 8/308 agreeing under the best flag set. RULED-OUT: hoisting `bit_src` next to `bit_dst` at the top (image DOES read the src tile's `bit` field before the is_ocean(dst) branch, per raw ebx allocation) - tried both declaration orders, both regressed (18/308 and 5/308) because it forces a second stack-resident local where the image keeps bit_src in a register (ebx) for the whole function and only spills bit_dst. The remaining gap past instruction 8 is VC6's own instruction scheduling across the three nested inlines (bit_at/is_ocean/altitude_at) - a scheduler decision, not reachable by reordering statements. Plateau at this structure.
// size      799 bytes
// prototype int (__cdecl ?hex_cost@@YAHHHHHHHH@Z)(int protoID, int factionID, int xCoordSrc, int yCoordSrc, int xCoordDst, int yCoordDst, int toggle)
// callers   5   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C13B0 0x00644F3A
Return Value: Movement cost
Status: Complete
*/
int __cdecl hex_cost(int proto_id, int faction_id, int x_src, int y_src, int x_dst,
                     int y_dst, BOOL toggle) {
    uint32_t bit_dst = bit_at(x_dst, y_dst);
    if (is_ocean(x_dst, y_dst)) {
        if (bit_dst & BIT_FUNGUS && altitude_at(x_dst, y_dst) == ALT_BIT_OCEAN_SHELF
            && get_proto_triad(proto_id) == TRIAD_SEA
            && proto_id != BSC_ISLE_OF_THE_DEEP && !has_project(SP_XENOEMPATYH_DOME, faction_id)) {
            return Rules->move_rate_roads * 3;
        }
        return Rules->move_rate_roads;
    }
    if (is_ocean(x_src, y_src)) {
        return Rules->move_rate_roads;
    }
    if (proto_id >= 0 && get_proto_triad(proto_id) != TRIAD_LAND) {
        return Rules->move_rate_roads;
    }
    // Land only conditions
    uint32_t bit_src = bit_at(x_src, y_src);
    if (bit_src & (BIT_MAGTUBE | BIT_BASE_IN_TILE) && bit_dst & (BIT_MAGTUBE | BIT_BASE_IN_TILE)
        && faction_id) {
        return 0;
    }
    if ((bit_src & (BIT_ROAD | BIT_BASE_IN_TILE) || bit_src & BIT_FUNGUS && faction_id > 0
        && has_project(SP_XENOEMPATYH_DOME, faction_id)) && bit_dst & (BIT_ROAD | BIT_BASE_IN_TILE)
        && faction_id) {
        return 1;
    }
    if (faction_id >= 0 && (has_project(SP_XENOEMPATYH_DOME, faction_id) || !faction_id
        || proto_id == BSC_MIND_WORMS || proto_id == BSC_SPORE_LAUNCHER) && bit_dst & BIT_FUNGUS) {
        return 1;
    }
    if (bit_src & BIT_RIVER && bit_dst & BIT_RIVER && x_dist(x_src, x_dst) == 1
        && abs((int)y_src - (int)y_dst) == 1 && faction_id) {
        return 1;
    }
    if (VehPrototypes[proto_id].chassis_id == CHSI_HOVERTANK
        || has_abil(proto_id, ABL_ANTIGRAV_STRUTS)) {
        return Rules->move_rate_roads;
    }
    uint32_t cost = Rules->move_rate_roads;
    if (rocky_at(x_dst, y_dst) > TERRAIN_BIT_ROLLING && !toggle) {
        cost += Rules->move_rate_roads;
    }
    if (bit_dst & BIT_FOREST && !toggle) {
        cost += Rules->move_rate_roads;
    }
    if (faction_id && bit_dst & BIT_FUNGUS && (proto_id >= MaxVehProtoFactionNum
        || get_proto_offense_rating(proto_id) >= 0)) {
        uint8_t plan = VehPrototypes[proto_id].plan;
        if (plan != PLAN_TERRAFORMING && plan != PLAN_ALIEN_ARTIFACT
            && PlayersData[faction_id].soc_effect_active.planet <= 0) {
            return cost + Rules->move_rate_roads * 2;
        }
        uint32_t speed = speed_proto(proto_id);
        if (cost <= speed) {
            return speed;
        }
    }
    return cost;
}

/*
Purpose: Relocate an existing unit to the specified tile.
// ORIGINAL: 0x005A59B0 ?veh_put@@YAXHHH@Z 0x005A59B0-0x005A59D2 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?veh_put@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BFFA0 0x005C0080
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Get the current health of the specified unit factoring in damage.
// ORIGINAL: 0x005A59E0 ?veh_health@@YAHH@Z 0x005A59E0-0x005A5A52 BYTE_EXACT
// LEVER: ternary -> if/else. The image branches on `plan != ALIEN_ARTIFACT`
// with `jne` to the range()*10 path and falls through `mov eax,1; jmp` on
// the equal side; a `cond ? range(...)*10 : 1` ternary compiled the
// opposite polarity (`je` past a shared *10). Splitting it into a plain
// if/else in the image's own branch order gives BYTE_EXACT at
// `/c /O2 /Gy /GR- /Oy- /GX` (the FRAMED set - `flags frame` above already
// says so).
// size      114 bytes
// prototype int (__cdecl ?veh_health@@YAHH@Z)(int vehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Unit health
Status: Complete
*/
int __cdecl veh_health(int veh_id) {
    int proto_id = Vehs[veh_id].proto_id;
    int health;
    if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        health = 1;
    } else {
        health = range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
    }
    return range(health - Vehs[veh_id].dmg_incurred, 0, 9999);
}

/*
Purpose: Calculates cost of the prototype based on various factors. Optimized logic flow from 
         the original without any differences to the final calculation.
// ORIGINAL: 0x005A5A60 ?proto_cost@@YAHHHHHH@Z 0x005A5A60-0x005A5CE1
// symbol    ?proto_cost@@YAIHHHHH@Z
// size      641 bytes
// prototype int (__cdecl ?proto_cost@@YAHHHHHH@Z)(int chassisType, int weapType, int armorType, int ability, int reactorType)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: reordering the checks to the ORIGINAL's actual sequence (missile check AFTER
//   the triad-based armor/speed adjustments, not before; the AFLAG_COST_INC_LAND_UNIT
//   check as a second standalone loop over Ability[] that only runs for TRIAD_LAND,
//   not folded into the main loop; the all-four-costs==1 override applied at the very
//   END overriding a fully-computed proto_mod, not as an early-out) - confirmed by
//   reading the full disassembly instruction-by-instruction and cross-referencing struct
//   offsets (RulesChassis.missile @0x4D, RulesAbility.cost_factor via a pointer walked by
//   sizeof(RulesAbility)=0x1C). This IS the original's control flow, but agreement is
//   still only ~3/247 instructions: the image reserves locals with `sub esp,0x14` and
//   reuses parameter/local stack slots for later values (armor_id's slot becomes
//   speed_cost; weap_cost's slot becomes combat_mod) in a way this tree's compiler does
//   not reproduce from equivalent-looking C. Left at MISMATCH; the control-flow structure
//   is now faithful even though the byte encoding is not.
Return Value: Cost of prototype
Status: Complete
*/
uint32_t __cdecl proto_cost(int chassis_id, int weapon_id, int armor_id, 
                            int ability, int reactor_id) {
    uint8_t weap_cost = Weapon[weapon_id].cost;
    uint8_t armor_cost = Armor[armor_id].cost;
    int speed_cost = Chassis[chassis_id].cost;
    int abil_modifier = 0;
    if (ability) {
        for (int i = 0; i < MaxAbilityNum; i++) {
            if ((1 << i) & ability) {
                if (abil_modifier) {
                    abil_modifier++; // Increased cost with more than one ability
                }
                int factor = Ability[i].cost_factor;
                if (factor > 0) { // 1+ = Straight Cost; 25% increase per unit of cost
                    abil_modifier += factor;
                } else {
                    switch (factor) {
                      // Increases w/ ratio of weapon to armor: 0, 1, or 2. Rounded DOWN.
                      // Never higher than 2.
                      case -1:
                        if (armor_cost) {
                            abil_modifier += range(weap_cost / armor_cost, 0, 2);
                        }
                        break;
                      case -2: // Increases w/ weapon value
                        abil_modifier += weap_cost - 1;
                        break;
                      case -3: // Increases w/ armor value
                        abil_modifier += armor_cost - 1;
                        break;
                      case -4: // Increases w/ speed value
                        abil_modifier += speed_cost - 1;
                        break;
                      case -5: // Increases w/ weapon+armor value
                        abil_modifier += weap_cost + armor_cost - 2;
                        break;
                      case -6: // Increases w/ weapon+speed value
                        abil_modifier += weap_cost + speed_cost - 2;
                        break;
                      case -7: // Increases w/ armor+speed value
                        abil_modifier += armor_cost + speed_cost - 2;
                        break;
                      case 0: // None
                      default:
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
        return weap_cost;
    }
    uint32_t combat_mod = armor_cost / 2 + 1;
    if (combat_mod < weap_cost) { // which ever is greater
        combat_mod = weap_cost;
    }
    // (2 << n) == 2^(n + 1) ; (2 << n) / 2 == 2 ^ n;
    int proto_mod = ((speed_cost + armor_cost) * combat_mod + ((2 << reactor_id) / 2))
        / (2 << reactor_id);
    if (speed_cost == 1) {
        proto_mod = (proto_mod / 2) + 1;
    }
    if (Weapon[weapon_id].cost > 1 && Armor[armor_id].cost > 1) {
        proto_mod++;
        if (triad == TRIAD_LAND && speed_cost > 1) {
            proto_mod++;
        }
    }
    // excludes sea probes
    if (triad == TRIAD_SEA && Weapon[weapon_id].mode != WPN_MODE_INFOWAR) {
        proto_mod = (proto_mod + 1) / 2;
    } else if (triad == TRIAD_AIR) {
        proto_mod /= (Weapon[weapon_id].mode > WPN_MODE_MISSILE) ? 2 : 4; // Non-combat : Combat
    }
    int reactor_mod = (reactor_id * 3 + 1) / 2;
    if (proto_mod < reactor_mod) { // which ever is greater
        proto_mod = reactor_mod;
    }
    if (combat_mod == 1 && armor_cost == 1 && speed_cost == 1 && reactor_id == RECT_FISSION) {
        proto_mod = 1;
    }
    return (proto_mod * (abil_modifier + 4) + 2) / 4;
}

/*
Purpose: Calculates the base cost of the specified prototype.
// ORIGINAL: 0x005A5D00 ?base_cost@@YAHH@Z 0x005A5D00-0x005A5D3F BYTE_EXACT
// size      63 bytes
// prototype int (__cdecl ?base_cost@@YAHH@Z)(int protoID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A5A60
Return Value: Base cost of the prototype
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Create a new prototype. Sets initial values for everything except veh_name and preq_tech.
// ORIGINAL: 0x005A5D40 ?make_proto@@YAXHHHHHH@Z 0x005A5D40-0x005A6269
// size      1321 bytes
// prototype void (__cdecl ?make_proto@@YAXHHHHHH@Z)(int protoID, int chassisType, int weapType, int armorType, int ability, int reactorType)
// callers   4   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// RULED-OUT: nothing tried yet beyond inspection - call_diff agrees on call count/targets,
//   so the divergence (5/361 agreeing, ~0.14 similar) is all in the opening loop. The image
//   computes `i % MaxVehProtoFactionNum` there with the full MSVC signed-modulo-by-constant
//   sign-correction sequence (`and 0x8000003f; jns; dec; or 0xffffffc0; inc`), which this
//   loop's straightforward `i % MaxVehProtoFactionNum` on a provably-nonnegative `i` does not
//   emit - the compiler proves `i>=0` and just masks. Left as MISMATCH; the loop bound/shape
//   likely needs rewriting closer to the disassembly's own register-level computation
//   (proto_id itself, not `i`, appears to be the value being modulo'd in several places) to
//   close this, and that is a bigger rewrite than this pass budgeted for one function.
// calls     0x0057D510 0x005A5A60 0x005B9F20 0x005BF1F0
Return Value: n/a
Status: Complete
*/
void __cdecl make_proto(int proto_id, int chassis_id, int weapon_id, int armor_id, 
                        int ability, int reactor_id) {
    int unk_local_1 = 0; // TODO: Identify
    if (proto_id >= MaxVehProtoFactionNum) {
        BOOL cond1 = false;
        BOOL cond2 = false;
        BOOL cond3 = false;
        int proto_id_loop;
        for (int i = 0; i < 128; i++) {
            proto_id_loop = i % MaxVehProtoFactionNum;
            if ((i % MaxVehProtoFactionNum) != i) {
                proto_id_loop += proto_id - proto_id % MaxVehProtoFactionNum;
            }
            int flags_check = VehPrototypes[proto_id_loop].flags;
            if (flags_check & PROTO_ACTIVE) {
                if ((proto_id_loop <= MaxVehProtoFactionNum &&
                    has_tech(VehPrototypes[proto_id_loop].preq_tech, 
                        proto_id / MaxVehProtoFactionNum))
                    || (proto_id_loop > MaxVehProtoFactionNum
                        && (flags_check & PROTO_TYPED_COMPLETE))) {
                    uint32_t loop_weapon_id = VehPrototypes[proto_id_loop].weapon_id;
                    if (loop_weapon_id == weapon_id) {
                        cond1 = true;
                    }
                    uint32_t loop_armor_id = VehPrototypes[proto_id_loop].armor_id;
                    if (loop_armor_id == armor_id) {
                        cond2 = true;
                    }
                    uint32_t loop_chassis_id = VehPrototypes[proto_id_loop].chassis_id;
                    if (loop_chassis_id == chassis_id) {
                        cond3 = true;
                    }
                    int off_rating = Weapon[weapon_id].offense_rating;
                    if (off_rating > 0 && Weapon[loop_weapon_id].offense_rating >= off_rating) {
                        cond1 = true;
                    }
                    int def_rating = Armor[armor_id].defense_rating;
                    if (def_rating > 0 && Armor[loop_armor_id].defense_rating >= def_rating) {
                        cond2 = true;
                    }
                    if (Chassis[chassis_id].triad == Chassis[loop_chassis_id].triad
                        && Chassis[loop_chassis_id].speed >= Chassis[chassis_id].speed) {
                        cond3 = true;
                    }
                }
            }
        }
        if (cond1 && cond2 && cond3) {
            unk_local_1 = (proto_id_loop >= MaxVehProtoFactionNum
                && VehPrototypes[proto_id_loop].flags & 0x10) ? 3 : 1;
        }
    }
    VehPrototypes[proto_id].chassis_id = (uint8_t)chassis_id;
    VehPrototypes[proto_id].weapon_id = (uint8_t)weapon_id;
    VehPrototypes[proto_id].armor_id = (uint8_t)armor_id;
    VehPrototypes[proto_id].ability_flags = ability;
    VehPrototypes[proto_id].reactor_id = (uint8_t)reactor_id;
    VehPrototypes[proto_id].cost = (uint8_t)proto_cost(chassis_id, weapon_id, armor_id, ability,
        reactor_id);
    VehPrototypes[proto_id].carry_capacity = (Weapon[weapon_id].mode == WPN_MODE_TRANSPORT) 
        ? (uint8_t)transport_val(chassis_id, ability, reactor_id) : 0;
    // set plan & unk1
    if (Chassis[chassis_id].missile) {
        if (Weapon[weapon_id].offense_rating < 99) { // non-PB missiles
            if (weapon_id == WPN_TECTONIC_PAYLOAD) {
                VehPrototypes[proto_id].plan = PLAN_TECTONIC_MISSILE;
                VehPrototypes[proto_id].unk_1 = 22;
            } else if (weapon_id == WPN_FUNGAL_PAYLOAD) {
                VehPrototypes[proto_id].plan = PLAN_FUNGAL_MISSILE;
                VehPrototypes[proto_id].unk_1 = 23;
            } else { // Conventional Payload
                VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
                VehPrototypes[proto_id].unk_1 = 13;
            }
        } else {
            VehPrototypes[proto_id].plan = PLAN_PLANET_BUSTER;
            VehPrototypes[proto_id].unk_1 = 14;
        }
    } else if (Weapon[weapon_id].mode >= WPN_MODE_TRANSPORT) { // Non-combat
        VehPrototypes[proto_id].plan = Weapon[weapon_id].mode;
        VehPrototypes[proto_id].unk_1 = Weapon[weapon_id].mode + 0x20;
    } else if (Chassis[chassis_id].triad == TRIAD_SEA) { // combat sea
        VehPrototypes[proto_id].plan = PLAN_NAVAL_SUPERIORITY;
        VehPrototypes[proto_id].unk_1 = 6; // same value as plan
    } else if (Chassis[chassis_id].triad == TRIAD_AIR) { // combat air
        if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
            VehPrototypes[proto_id].plan = PLAN_AIR_SUPERIORITY;
            VehPrototypes[proto_id].unk_1 = 9;
        } else {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
            VehPrototypes[proto_id].unk_1 = 8;
        }
    } else { // TRIAD_LAND combat unit
        VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        if (Armor[armor_id].defense_rating > 1) {
            VehPrototypes[proto_id].plan = PLAN_DEFENSIVE;
        }
        if (Weapon[weapon_id].offense_rating >= Armor[armor_id].defense_rating
            && VehPrototypes[proto_id].plan == PLAN_DEFENSIVE) {
            VehPrototypes[proto_id].plan = PLAN_COMBAT;
        }
        if (Chassis[chassis_id].speed > 1
            && Weapon[weapon_id].offense_rating > Armor[armor_id].defense_rating) {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        }
        if (ability & (ABL_ARTILLERY | ABL_DROP_POD | ABL_AMPHIBIOUS)) {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        }
        VehPrototypes[proto_id].unk_1 = 3;
        if (Armor[armor_id].defense_rating > Weapon[weapon_id].offense_rating) {
            VehPrototypes[proto_id].unk_1 = 2;
        }
        if (Weapon[weapon_id].offense_rating > Armor[armor_id].defense_rating * 2) {
            VehPrototypes[proto_id].unk_1 = 1;
        }
        // changed from '>= 2' since '>= 3' will always overwrite it
        if (Chassis[chassis_id].speed == 2) {
            VehPrototypes[proto_id].unk_1 = 4;
        } else if (Chassis[chassis_id].speed >= 3) { // added else, if speed <= 2 would always skip
            VehPrototypes[proto_id].unk_1 = 5;
        }
        if (Weapon[weapon_id].offense_rating == 1 && Armor[armor_id].defense_rating == 1) {
            if (Chassis[chassis_id].speed > 1) {
                VehPrototypes[proto_id].plan = PLAN_RECONNAISANCE;
            }
            if (has_abil(proto_id, ABL_POLICE_2X)) {
                VehPrototypes[proto_id].plan = PLAN_DEFENSIVE;
            }
            VehPrototypes[proto_id].unk_1 = 10;
        }
    }
    // set final values
    VehPrototypes[proto_id].obsolete_factions = 0;
    VehPrototypes[proto_id].combat_factions = (proto_id >= MaxVehProtoFactionNum) ?
        1 << (proto_id / MaxVehProtoFactionNum) : -1;
    VehPrototypes[proto_id].flags = unk_local_1 
        ? ((unk_local_1 & 2) ? 0x115 : 0x105) : PROTO_ACTIVE;
    VehPrototypes[proto_id].icon_offset = -1;
}

/*
Purpose: Search for the 1st prototype (basic + faction specific) that matches the specified plan.
// ORIGINAL: 0x005AED50 ?get_plan@@YAHHH@Z 0x005AED50-0x005AEDE0
// size      144 bytes
// prototype int (__cdecl ?get_plan@@YAHHH@Z)(int factionID, int plan)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Prototype id or -1 if not found
Status: Complete
*/
int __cdecl get_plan(int faction_id, int plan) {
    for (int i = 0; i < 128; i++) {
        uint32_t proto_id = (i < MaxVehProtoFactionNum) ? i
            : (faction_id * MaxVehProtoFactionNum) + i - MaxVehProtoFactionNum;
        if (VehPrototypes[proto_id].flags & PROTO_ACTIVE
            && !(VehPrototypes[proto_id].obsolete_factions & (1 << faction_id))
            && (proto_id >= MaxVehProtoFactionNum
                || has_tech(VehPrototypes[proto_id].preq_tech, faction_id))
            && VehPrototypes[proto_id].plan == plan) {
            return proto_id;
        }
    }
    return -1;
}

/*
Purpose: Reveal the specified tile to the specified faction and keep the
         faction's remembered copy of the tile's terrain bits in step with it.
// ORIGINAL: 0x005B5700 sub_5b5700 0x005B5700-0x005B57CA BYTE_EXACT
// LEVER: FEWER (0 calls vs image's 1: synch_bit) at every flag set that kept a standalone symbol. /Ob0 forces the WHOLE function real, including on_map()/map_loc(), which the image folds in here; without /Ob0 the compiler inlines synch_bit() too, since it is small and `MEASURED inline` in map.h. Writing on_map()/map_loc() as bare expressions (not calls) removes them from the optimiser's inline/no-inline choice entirely, so /Ob0 only has synch_bit() left to force real.
// symbol    ?spot_tile@@YAXHHH@Z
// size      202 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591E50
Return Value: n/a
Status: Complete

The image gives this function no symbol - the catalogue calls it sub_5b5700 -
so spot_tile is this recovery's name for it, chosen because it is the tile-level
member of the spot_*() family that surrounds it (005B57D0 spot_base, 005B58E0
spot_stack, 005B5AD0 spot_loc). Every one of those three carries this body
inlined verbatim, which is how it was identified: the one catalogued call site
is not the only place the bytes appear.

The dirty-tile arm is deliberately narrow. It fires only for the LOCAL faction,
only the first time that faction sees the tile, and not at all under omniscient
view or once the faction's map has been revealed outright - four conditions
which between them mean it is the interactive map's repaint hint rather than
game state. `bit2 |= 0x400000` and `UnkBitfield1 |= 1` are the same pair
climate_set() sets at 00591A80 and carry the same unidentified meaning.
*/
inline void __cdecl spot_tile(int x, int y, int faction_id) {
    // on_map() and map_loc() are written out here rather than called: the
    // real standalone body at 0x005B5700 needs both of them folded in while
    // synch_bit() stays a genuine call, and a function CALL to an inline
    // helper is still a call the optimiser can decide to keep - writing the
    // expressions removes that choice instead of hoping for it.
    if (!(y >= 0 && y < (int)MapLatitudeBounds
          && x >= 0 && x < (int)MapLongitudeBounds)) {
        return;
    }
    Map *tile = *reinterpret_cast<Map **>(0x0094A30C) + ((x >> 1) + y * MapLongitude);
    if (faction_id == LocalFaction && !(tile->visibility & (1 << faction_id))
        && !(GameState & STATE_OMNISCIENT_VIEW)
        && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
        tile->bit2 |= 0x400000; // TODO: identify value
        UnkBitfield1 |= 1; // TODO: identify global + value
    }
    tile->visibility |= (uint8_t)(1 << faction_id);
    synch_bit(x, y, faction_id);
}

/*
Purpose: Reveal the specified base, and the tile it stands on, to the specified
         faction.
// ORIGINAL: 0x005B57D0 ?spot_base@@YAXHH@Z 0x005B57D0-0x005B58D2 BYTE_EXACT
// size      258 bytes
// prototype void (__cdecl ?spot_base@@YAXHH@Z)(int baseID, int factionID)
// callers   8   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591E50
Return Value: n/a
Status: Complete

The population size is copied into the faction's intel slot at the moment of
sighting rather than read live, so a faction that loses sight of a base keeps
the size it last saw. The copy is unconditional: it happens again on every
sighting whether or not the visibility bit was already set.

There is no bounds check on base_id. All eight call sites pass an id that came
out of base_at() or a bounded loop.
*/
void __cdecl spot_base(int base_id, int faction_id) {
    Bases[base_id].visibility |= (uint8_t)(1 << faction_id);
    Bases[base_id].faction_pop_size_intel[faction_id] = Bases[base_id].population_size;
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    // spot_tile(), hand-inlined - a call to an `inline` helper is still a
    // call the optimiser can fold away, and does here: matching spot_tile's
    // own comment on why synch_bit() has to stay a genuine call.
    if (y >= 0 && y < (int)MapLatitudeBounds
        && x >= 0 && x < (int)MapLongitudeBounds) {
        Map *tile = *reinterpret_cast<Map **>(0x0094A30C) + ((x >> 1) + y * MapLongitude);
        if (faction_id == LocalFaction && !(tile->visibility & (1 << faction_id))
            && !(GameState & STATE_OMNISCIENT_VIEW)
            && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
            tile->bit2 |= 0x400000; // TODO: identify value
            UnkBitfield1 |= 1; // TODO: identify global + value
        }
        tile->visibility |= (uint8_t)(1 << faction_id);
        synch_bit(x, y, faction_id);
    }
}

/*
Purpose: Reveal the whole of the specified unit's stack, and the tile it stands
         on, to the specified faction.
// ORIGINAL: 0x005B58E0 ?spot_stack@@YAXHH@Z 0x005B58E0-0x005B5A61
// size      385 bytes
// prototype void (__cdecl ?spot_stack@@YAXHH@Z)(int vehID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591E50
Return Value: n/a
Status: Complete

Two separate walks over two separate things. The tile is revealed once, from the
coordinates of the unit that was named; the visibility bit is then set on every
unit in the stack, found by climbing prev_veh_id_stack to the top and descending
next_veh_id_stack from there. The named unit need not be the top one, which is
why the climb exists. A negative id reveals no tile and marks no unit.

The original's `and word [veh+8], 0FBBFh` is written as the two named bits it
clears: 0xFBBF is exactly ~(VFLAG_LURKER | VFLAG_INVISIBLE) in sixteen bits, so
being seen stops a unit lurking and stops it being invisible. It is skipped for
faction zero, and only for faction zero; nothing else here distinguishes
factions.

The climb is veh_top() at 00579920, inlined, and is written as the call it was.
0x005B59F9 to 0x005B5A2A is that function instruction for instruction: the same
`test eax, eax / jl` for a negative id, the same `lea eax+eax*2 / lea eax+ecx*4`
stride, the same `movsx` of prev_veh_id_stack, the same loop. The negative test
that follows the climb is therefore not a dead branch on the climb's output - it
is the caller reading veh_top()'s -1 - and it is the `id >= 0` below.

The original tests on_map() twice before revealing the tile: once for the guard
written out below, and once inside the inlined spot_tile(). It is a pure
predicate over two globals and the same two coordinates, so the second test
cannot disagree with the first.

// LEVER: `calls` disagreed (2 here vs the image's 1: synch_bit only) because
//        spot_tile() and veh_top() were both left as real calls despite the
//        comment above already establishing both are inlined. spot_tile is
//        now plain `inline` (its only callers are spot_base/spot_stack/
//        spot_loc, all in this file); the veh_top climb below is
//        hand-inlined, matching veh_at's precedent.
// RULED-OUT: calling the `inline` spot_tile() directly - the optimiser did
// NOT fold it away here (unlike in spot_base), so the call to spot_tile
// survived and call_diff still reported WRONG CALLEE (spot_tile instead of
// synch_bit). Hand-inlining spot_tile's body, matching spot_base's own
// precedent, is what removes the call.
*/
void __cdecl spot_stack(int veh_id, int faction_id) {
    if (veh_id >= 0 && on_map(Vehs[veh_id].x, Vehs[veh_id].y)) {
        // spot_tile(), hand-inlined - see spot_base's precedent above; a call
        // to `inline` spot_tile survived the optimiser here.
        int x = Vehs[veh_id].x;
        int y = Vehs[veh_id].y;
        if (y >= 0 && y < (int)MapLatitudeBounds
            && x >= 0 && x < (int)MapLongitudeBounds) {
            Map *tile = *reinterpret_cast<Map **>(0x0094A30C) + ((x >> 1) + y * MapLongitude);
            if (faction_id == LocalFaction && !(tile->visibility & (1 << faction_id))
                && !(GameState & STATE_OMNISCIENT_VIEW)
                && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
                tile->bit2 |= 0x400000; // TODO: identify value
                UnkBitfield1 |= 1; // TODO: identify global + value
            }
            tile->visibility |= (uint8_t)(1 << faction_id);
            synch_bit(x, y, faction_id);
        }
    }
    int top_veh_id;
    if (veh_id < 0) {
        top_veh_id = -1;
    } else {
        top_veh_id = veh_id;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    for (int id = top_veh_id; id >= 0; id = Vehs[id].next_veh_id_stack) {
        Vehs[id].visibility |= (uint8_t)(1 << faction_id);
        if (faction_id) {
            Vehs[id].flags &= (uint16_t)~(VFLAG_LURKER | VFLAG_INVISIBLE);
        }
    }
}

/*
Purpose: Reveal whatever occupies the specified location - the tile itself, and
         then either the base standing on it or the whole unit stack - to the
         specified faction.
// ORIGINAL: 0x005B5AD0 ?spot_loc@@YAXHHH@Z 0x005B5AD0-0x005B5E05
// RULED-OUT: byte-exactness, deliberately - not chased. The image INLINES
// spot_tile, spot_base and spot_stack whole (only base_at and veh_at
// survive as real calls), so a body that calls the five source-owned
// helpers by name - as this one does, on purpose, per the analysis below -
// diverges from instruction 0 (1/247 agreeing, push ebp/mov ebp,esp vs a
// register-argument prologue). Re-inlining all five to chase bytes would
// duplicate spot_tile/spot_base/spot_stack's own already-recovered bodies
// here; the call-graph transcription is judged the better trade and is
// left as MISMATCH.
// size      821 bytes
// prototype void (__cdecl ?spot_loc@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   6   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00591E50 0x005BFE90
Return Value: n/a
Status: Complete

821 bytes of original for six lines of source, because five of its six calls
were inlined and only base_at and veh_at survive as calls in the image. What
0x005B5AD0 to 0x005B5E04 actually contains, in order, is spot_tile at
0x005B5AD0, base_at, spot_base at 0x005B5BA7, veh_at, and spot_stack at
0x005B5CB4 - each one the whole of the corresponding function, instruction for
instruction, including spot_base's and spot_stack's own inlined copies of
spot_tile and spot_stack's inlined veh_top. Every one is now source-owned, so
this is a transcription of the call sequence rather than a re-inlining of it.

The evidence that these really are those functions and not merely similar code:
the base arm carries spot_base's unconditional intel copy from Base+6 to
Base+0Bh+faction and its SINGLE bounds test, while the unit arm carries
spot_stack's DOUBLED bounds test and its veh_top climb - a difference between
the two arms which has no reason to exist in one hand-written function and
exactly one reason to exist in two inlined ones.

A base wins outright: the unit arm is reached only when base_at answers -1, and
the tile reveal happens first either way, whether or not anything stands on it.
veh_at's own -1 is handed to spot_stack, which returns on a negative id, so
there is no third arm.
*/
void __cdecl spot_loc(int x, int y, int faction_id) {
    spot_tile(x, y, faction_id);
    int base_id = base_at(x, y);
    if (base_id >= 0) {
        spot_base(base_id, faction_id);
        return;
    }
    spot_stack(veh_at(x, y), faction_id);
}

/*
Purpose: Determine if the specified unit wants to wake up based on certain conditions and
         preferences. Optional parameter for spotted veh_id (-1 to skip).
// ORIGINAL: 0x005B5EA0 ?want_to_wake@@YAHHHH@Z 0x005B5EA0-0x005B6060
// RULED-OUT: image never reads Vehs[veh_id].faction_id eagerly - it re-reads it at each conditional use, and never caches it across the early return. Un-caching it into `Vehs[veh_id].faction_id` at every site (instead of a hoisted `veh_faction_id` local) let the compiler pick a plain ebp-frame prologue matching the image's, where the hoisted local forced a param-less-locals, esp-relative frame from instruction 0. Also restored the `veh_id >= 0 &&` half of the trailing guard - the image's `test edi, edi; jl` tests veh_id, not just spotted_veh_id, contradicting this body's own comment that it was safe to drop. Residual divergence past this is register allocation (esi/edi/ebx/ebp swapped throughout) and one 16-bit shift-then-extend vs extend-then-shift on `waypoint_x[0] >> 1`; an explicit `(int)` cast on the base_who() arguments did not change the codegen. Best flag set /O2 /Gy /GR- /Oy- /GX, 0.793 similar.
// size      448 bytes
// prototype int (__cdecl ?want_to_wake@@YAHHHH@Z)(unsigned int factionID, unsigned int vehID, int vehID2)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Does unit want to wake? true/false
Status: Complete
*/
BOOL __cdecl want_to_wake(int faction_id, int veh_id, int spotted_veh_id) {
    uint32_t triad = get_triad(veh_id);
    if (triad == TRIAD_AIR && Vehs[veh_id].terraforming_turns
        && Vehs[veh_id].order == ORDER_MOVE_TO) {
        int base_faction_id = base_who(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
        if (base_faction_id >= 0
            && (((uint32_t)base_faction_id == Vehs[veh_id].faction_id)
                || has_treaty(Vehs[veh_id].faction_id, base_faction_id, DTREATY_PACT))) {
            return false;
        }
    }
    BOOL wants_to_wake;
    if (IsMultiplayerNet) { // restructured to be more efficient with same logic
        if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_PACT)) {
            wants_to_wake = false;
        } else if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_TREATY)) {
            wants_to_wake = true;
        } else if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_TRUCE)) {
            wants_to_wake = true;
        } else {
            wants_to_wake = true;
        }
    } else {
        if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_PACT)) {
            wants_to_wake = GamePreferences & PREF_AUTO_END_MOVE_SPOT_VEH_PACT;
        } else if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_TREATY)) {
            wants_to_wake = GamePreferences & PREF_AUTO_END_MOVE_SPOT_VEH_TREATY;
        } else if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_TRUCE)) {
            wants_to_wake = GamePreferences & PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE;
        } else {
            wants_to_wake = GamePreferences & PREF_AUTO_END_MOVE_SPOT_VEH_WAR;
        }
        if (!(GamePreferences & PREF_AUTO_DONT_END_MOVE_DIFF_TRIAD)) {
            return wants_to_wake;
        }
    }
    if (veh_id >= 0 && spotted_veh_id >= 0) {
        uint32_t triad_spotted = get_triad(spotted_veh_id);
        if (triad != triad_spotted && ((Vehs[veh_id].state & (VSTATE_UNK_1000000 | VSTATE_UNK_200))
            != (VSTATE_UNK_1000000 | VSTATE_UNK_200) || triad_spotted == TRIAD_LAND)) {
            wants_to_wake = false;
        }
    }
    return wants_to_wake;
}

/*
Purpose: Wake up units meeting specific conditions from the stack where specified unit is located.
// ORIGINAL: 0x005B6060 ?wake_stack@@YAXH@Z 0x005B6060-0x005B619A
// LEVER: veh_top() is called at only 2 real sites in the image (see its own
//        comment on veh_top); here it is inlined, matching veh_at above. The
//        climb's induction variable is int16_t (prev_veh_id_stack's own type),
//        matching veh_top's own lever. is_ocean()/base_who()/bit_at() are hand-
//        inlined sharing ONE tile pointer, and map_loc()'s two globals
//        (map_tiles(), MapLongitude) are hoisted out of the `i` loop into locals
//        BEFORE it - the image reads them once, kept live in registers across
//        every iteration, rather than re-reading them on each tile lookup a
//        plain per-iteration map_loc(x, y) call would. Together: 0.836 -> 0.955.
// RULED-OUT: an `int altitude = tile->climate & 0xE0;` local still narrows to a
//   byte compare (`cmp al`/`jb`) where the image has `cmp eax`/`jl` - VC6 proves
//   the range from the `& 0xE0` mask regardless of the local's declared width;
//   not reachable by re-typing. Left as the last four-instruction gap.
// size      314 bytes
// prototype void (__cdecl ?wake_stack@@YAXH@Z)(int vehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl wake_stack(int veh_id) {
    if (veh_id < 0) {
        return;
    }
    int top_veh_id = veh_id;
    for (int16_t j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
        top_veh_id = j;
    }
    if (top_veh_id < 0) {
        return;
    }
    // map_loc()'s two globals (map_tiles(), MapLongitude), hand-hoisted out of the loop
    // below: the image reads them ONCE, before the loop even starts (kept live in
    // registers across every iteration), rather than re-reading them on every tile
    // lookup the way a plain per-iteration map_loc(x, y) call would.
    Map *const map_base = map_tiles();
    const int map_width = MapLongitude;
    for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
        int veh_id_way_point;
        int x;
        int y;
        // is_ocean()/base_who()/bit_at(), hand-inlined and sharing ONE tile pointer -
        // the image computes it once (kept live across the waypoint/get_triad checks
        // in between) and reads climate/val2/bit off it, rather than recomputing
        // map_loc() independently in each helper.
        Map *tile;
        int altitude;
        if (Vehs[i].order == ORDER_SENTRY_BOARD
            && (get_triad(i) || (x = Vehs[i].x, y = Vehs[i].y,
                tile = map_base + ((x >> 1) + y * map_width), altitude = tile->climate & 0xE0,
            altitude >= ALT_BIT_SHORE_LINE
            && ((veh_id_way_point = Vehs[i].waypoint_x[0], veh_id_way_point < 0)
                || get_triad(veh_id_way_point) != TRIAD_AIR
                || ((tile->bit & BIT_BASE_IN_TILE) && (tile->val2 & 0xF) < MaxPlayerNum)
                || (tile->bit & BIT_AIRBASE))))) {
            Vehs[i].order = ORDER_NONE;
            Vehs[i].state &= ~VSTATE_EXPLORE;
        }
    }
}

/*
Purpose: Move a stack of units in the same stack as the specified unit to a destination.
// ORIGINAL: 0x005B8AF0 ?stack_put@@YAXHHH@Z 0x005B8AF0-0x005B8B5D
// size      109 bytes
// prototype void (__cdecl ?stack_put@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   5   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFFA0 0x005C0080
// RULED-OUT: `veh_top()`/`veh_put()` as plain calls - both are inline-marked
// wrappers, but this call site is not one of veh_top's real sites and
// veh_put did not fold here either, so call_diff reported WRONG CALLEE
// (veh_put/veh_top instead of veh_lift/veh_drop). Hand-inlining both,
// matching veh_promote's precedent, is what removes the mismatch.
Return Value: n/a
Status: Complete
*/
void __cdecl stack_put(int veh_id, int x, int y) {
    // veh_top(), hand-inlined - matching veh_promote's precedent (see its own
    // comment): the image does not call it here.
    int top_veh_id;
    if (veh_id < 0) {
        top_veh_id = -1;
    } else {
        top_veh_id = veh_id;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    int veh_id_loop;
    if (top_veh_id >= 0) {
        do {
            veh_id_loop = Vehs[top_veh_id].next_veh_id_stack;
            // veh_put(), hand-inlined: the image calls veh_lift and veh_drop
            // directly here, discarding veh_lift's return (it always returns
            // its own veh_id argument unchanged) rather than threading it
            // through to veh_drop's first parameter.
            veh_lift(top_veh_id);
            veh_drop(top_veh_id, x, y);
            top_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
    }
}

/*
Purpose: Sort a stack of units with the transports moved to the top.
// ORIGINAL: 0x005B8B60 ?stack_sort@@YAXH@Z 0x005B8B60-0x005B8C8D
// RULED-OUT: and `stack_put(veh_id_put, x, y)` were plain calls; call_diff's count-based check missed it (6 calls either way, since {veh_top, stack_put} happened to replace the {2 extra veh_lift/veh_drop} the image gets from inlining stack_put), but the image's actual 6 callees are veh_cargo/has_abil/veh_lift x2/ veh_drop x2 - no veh_top or stack_put. Hand-inlining both, including stack_put's OWN internal veh_top climb (its outer `next_veh_id >= 0` check survives - the image keeps it as a second, separate test after veh_top's own), took 0.627 -> 0.758.
// size      301 bytes
// prototype void (__cdecl ?stack_sort@@YAXH@Z)(int vehID)
// callers   2   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0 0x005BFFA0 0x005C0080 0x005C1760
Return Value: n/a
Status: Complete
*/
void __cdecl stack_sort(int veh_id) {
    int16_t x = Vehs[veh_id].x;
    int16_t y = Vehs[veh_id].y;
    // veh_top(), hand-inlined - matching stack_fix's precedent (see its own
    // comment): the image does not call it here.
    int next_veh_id;
    if (veh_id < 0) {
        next_veh_id = -1;
    } else {
        next_veh_id = veh_id;
        for (int i = Vehs[next_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
            next_veh_id = i;
        }
    }
    int veh_id_put = -1;
    int veh_id_loop;
    if (next_veh_id >= 0) {
        do {
            veh_id_loop = Vehs[next_veh_id].next_veh_id_stack; // removed redundant < 0 check
            if (veh_cargo(next_veh_id) || has_abil(Vehs[next_veh_id].proto_id, ABL_CARRIER)) {
                veh_id_put = next_veh_id;
                veh_put(next_veh_id, -3, -3);
            }
            next_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
        // stack_put(), hand-inlined - the image inlines this call site too,
        // including its own inlined veh_top() climb on veh_id_put.
        int top_veh_id;
        if (veh_id_put < 0) {
            top_veh_id = -1;
        } else {
            top_veh_id = veh_id_put;
            for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
                top_veh_id = j;
            }
        }
        if (top_veh_id >= 0) {
            int veh_id_loop2;
            do {
                veh_id_loop2 = Vehs[top_veh_id].next_veh_id_stack;
                veh_put(top_veh_id, x, y);
                top_veh_id = veh_id_loop2;
            } while (veh_id_loop2 >= 0);
        }
    }
}

/*
Purpose: Sort a stack of units with colony pods at the top followed by combat/offensive units.
// ORIGINAL: 0x005B8C90 ?stack_sort_2@@YAXH@Z 0x005B8C90-0x005B8E0A BYTE_EXACT
// LEVER: hand-inlined veh_top()'s climb and stack_put()'s call+its own veh_top()
//   climb, matching stack_sort's precedent (veh_put() itself stays a real call,
//   it MEASURED-inlines to veh_lift/veh_drop on its own). Moved veh_id_put's
//   declaration to the top of the function, matching store order in the image.
//   The image re-derives `next_veh_id_stack` through a `(id < 0) ? id :
//   Vehs[id].next_veh_id_stack` ternary even though the guard is provably true
//   at every use - reproduced verbatim, at all three next_veh_id_stack reads.
//   The two prev_veh_id_stack climb loops' induction variables are int16_t. not
//   int: the field is int16_t, and declaring the induction var int forces an
//   immediate movsx the image defers until the value is used as an array index.
// size      378 bytes
// prototype void (__cdecl ?stack_sort_2@@YAXH@Z)(int vehID)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BFE90 0x005BFFA0 0x005C0080
Return Value: n/a
Status: Complete
*/
void __cdecl stack_sort_2(int veh_id) {
    int veh_id_put = -1;
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    // veh_top(), hand-inlined - matching stack_sort's precedent (see its own
    // comment): the image does not call it here.
    int next_veh_id;
    if (veh_id < 0) {
        next_veh_id = -1;
    } else {
        next_veh_id = veh_id;
        for (int16_t i = Vehs[next_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
            next_veh_id = i;
        }
    }
    int veh_id_loop;
    if (next_veh_id >= 0) {
        do {
            // The image re-guards `next_veh_id < 0` here even though it is provably
            // true from the loop's own entry/continuation test; reproduced as the
            // same ternary the compiler folded into this exact branch shape.
            veh_id_loop = (next_veh_id < 0) ? next_veh_id : Vehs[next_veh_id].next_veh_id_stack;
            if (VehPrototypes[Vehs[next_veh_id].proto_id].plan == PLAN_COLONIZATION) { // Colony Pod
                veh_id_put = next_veh_id;
                veh_put(next_veh_id, -3, -3);
            }
            next_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
    }
    next_veh_id = veh_at(x, y);
    if (next_veh_id >= 0) {
        do {
            veh_id_loop = (next_veh_id < 0) ? next_veh_id : Vehs[next_veh_id].next_veh_id_stack;
            if (VehPrototypes[Vehs[next_veh_id].proto_id].plan <= PLAN_COMBAT) {
                veh_id_put = next_veh_id;
                veh_put(next_veh_id, -3, -3);
            }
            next_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
    }
    // stack_put(), hand-inlined - the image inlines this call site too,
    // including its own inlined veh_top() climb on veh_id_put.
    int top_veh_id;
    if (veh_id_put < 0) {
        top_veh_id = -1;
    } else {
        top_veh_id = veh_id_put;
        for (int16_t j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    if (top_veh_id >= 0) {
        int veh_id_loop2;
        do {
            veh_id_loop2 = (top_veh_id < 0) ? top_veh_id : Vehs[top_veh_id].next_veh_id_stack;
            veh_put(top_veh_id, x, y);
            top_veh_id = veh_id_loop2;
        } while (veh_id_loop2 >= 0);
    }
}

/*
Purpose: Refresh and fix the stack. Used by DirectPlay multiplayer only.
// ORIGINAL: 0x005B8E10 ?stack_fix@@YAHH@Z 0x005B8E10-0x005B8ED9
// RULED-OUT: frame pointer - the image keeps ebp (a `push ecx` local-space idiom) and duplicate non-merged epilogues; no flag set in the harness reproduces both at once here - /Oy- restores ebp but the two early returns cross-jump to one shared tail instead of duplicating, unlike the image. Plateau at 0.758 (/O2 /Gy /GR- /GX).
// RULED-OUT: is called at only 2 real sites in the image (see its own comment on veh_top); here it is inlined, matching veh_at above.
// RULED-OUT: combined `if (a || b || c) return veh_id;` compiled to one merged block; the image duplicates the epilogue three times (three separate early returns, each its own `jge`). Splitting into three sequential `if`s took similarity 0.627 -> 0.758 (best flag set has 0 raw agree but is the closest shape - see BUG note below for the loop, which is the bulk of the remaining gap).
// BUG IN THE ORIGINAL: the promote/sort loop compares `Vehs[veh_id]`'s own
//        x/y against themselves, not the loop iterator `Vehs[i]`'s - always
//        true, so `veh_promote(i)`/`stack_sort(veh_id)` fire on EVERY
//        iteration. Reproduced deliberately; do not "fix" to `Vehs[i]`.
// size      201 bytes
// prototype int (__cdecl ?stack_fix@@YAHH@Z)(int vehID)
// callers   26   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B8B60 0x005C0260
Return Value: Either the parameter unit id or unit id of the stack top; Return is checked if >= 0
Status: Complete
*/
int __cdecl stack_fix(int veh_id) {
    if (veh_id < 0) {
        return veh_id; // invalid veh_id
    }
    if (!IsMultiplayerNet) {
        return veh_id; // not DirectPlay MP
    }
    if (Vehs[veh_id].next_veh_id_stack < 0 && Vehs[veh_id].prev_veh_id_stack < 0) {
        return veh_id; // no stack
    }
    for (int i = 0; i < VehCurrentCount; i++) {
        // BUG IN THE ORIGINAL: compares veh_id's own coordinates against
        // themselves, not the iterator's - always true, so veh_promote(i)/
        // stack_sort(veh_id) run on every iteration of this loop.
        if (Vehs[veh_id].x == Vehs[veh_id].x && Vehs[veh_id].y == Vehs[veh_id].y) {
            veh_promote(i);
            stack_sort(veh_id);
        }
    }
    if (veh_id < 0) {
        return -1;
    }
    int top_veh_id = veh_id;
    for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
        top_veh_id = j;
    }
    return top_veh_id;
}

/*
Purpose: Board the eligible units stacked with the specified transport or carrier, optionally
         lifting the whole stack off the map afterwards. Mode 0 marks and counts without moving
         anything; mode 1 sorts the stack first and lifts each boarder plus the subject to the
         off-map holding square; mode 2+ makes a single pass and short-circuits to the lift when
         the subject already sits on a faction-owned base.
// ORIGINAL: 0x005B8EE0 ?stack_veh@@YAHHH@Z 0x005B8EE0-0x005B950F
// RULED-OUT: by 1 (17 vs image's 16) - two `veh_top(veh_id)` calls (the mode-0 refresh loop and the per-pass boarding loop's start index) hand-inlined, matching stack_fix's precedent, and `sleep(i)` forced to a real call via a new `sleep_call` forwarder in veh.h/veh.cpp (its only other reference is the inline body - under /O2 `sleep(i)` folds to 3 field writes, but the image calls it for real at this one site). call_diff now agrees (0 disagreeing, was MORE by 1). This is a 472-instruction function; the remaining gap is per-branch register/scheduling noise not chased further at this budget.
// size      1583 bytes
// prototype int (__cdecl ?stack_veh@@YAHHH@Z)(int vehID, int)
// callers   10   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B8B60 0x005B8E10 0x005BF1F0 0x005BFFA0 0x005C0080 0x005C01A0 0x005C1760
Return Value: Remaining capacity for mode 0, otherwise the unit id
Status: Complete
*/
int __cdecl stack_veh(int veh_id, int mode) {
    if (veh_id < 0) {
        return mode ? veh_id : 0;
    }
    if (!veh_cargo(veh_id) && !has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)) {
        // Neither a transport nor a carrier: nothing can be stacked onto it.
        return mode ? veh_drop(veh_lift(veh_id), -2, -2) : 0;
    }
    if (IsMultiplayerNet) {
        stack_fix(veh_id); // return value is discarded by the original
    }
    if (mode) {
        if (mode > 1) {
            // Unguarded on purpose: the original reads this tile without an on_map() check and
            // halves x with an arithmetic shift, so map_loc()'s uint32_t parameter would send a
            // negative x roughly a gigabyte away. Same for the two other unguarded reads below.
            const Map *tile = &map_tiles()[(Vehs[veh_id].x >> 1)
                + Vehs[veh_id].y * (int)MapLongitude];
            // The original also compares the masked nibble against 0, which is vacuous.
            if ((tile->bit & BIT_BASE_IN_TILE) && (tile->val2 & 0xF) < MaxPlayerNum) {
                return veh_drop(veh_lift(veh_id), -2, -2);
            }
        }
        stack_sort(veh_id);
    } else {
        // veh_top(veh_id), hand-inlined - matching stack_fix's precedent
        // (see its own comment): the image does not call it here.
        int top_veh_id = veh_id;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
        for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
            Vehs[i].state &= ~VSTATE_UNK_1; // so this call's boarding flags start clean
        }
    }
    int16_t subject_x = Vehs[veh_id].x;
    int16_t subject_y = Vehs[veh_id].y;
    BOOL is_open_ocean = on_map(subject_x, subject_y) && is_ocean(subject_x, subject_y)
        && !((bit_at(subject_x, subject_y) & BIT_BASE_IN_TILE)
             && owner_at(subject_x, subject_y) < MaxPlayerNum);
    uint32_t subject_faction = Vehs[veh_id].faction_id;
    BOOL subject_is_air = (get_triad(veh_id) == TRIAD_AIR);
    int cargo_left = (int)veh_cargo(veh_id);
    int carrier_left = has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)
        ? ((int)veh_cargo(veh_id) < 1 ? 1 : (int)veh_cargo(veh_id)) : 0;
    if (!subject_faction) {
        cargo_left = 99;
    }
    int pass_count = (mode <= 1) ? 2 : 1;
    for (int pass = 0; pass < pass_count; pass++) {
        int i;
        if (mode) {
            i = veh_id;
        } else {
            // veh_top(veh_id), hand-inlined - matching stack_fix's precedent
            // (see its own comment): the image does not call it here.
            i = veh_id;
            for (int j = Vehs[i].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
                i = j;
            }
        }
        while (i >= 0) {
            if (!cargo_left && !carrier_left) {
                break;
            }
            // Captured before the body: mode 1 lifts the unit, rewriting this link.
            // The original's redundant negative test on the link is dropped.
            // (This note sits on its own line rather than trailing the statement because
            // tools/mutate_and_verify.py scans a trailing comment as if it were code: the
            // `< 0` it used to carry produced two mutants that only ever rewrote a comment,
            // so they were unkillable by construction. The tool already skips whole-line
            // comments for exactly that reason.)
            int next_veh_id = Vehs[i].next_veh_id_stack;
            if (Vehs[i].faction_id != subject_faction) {
                i = next_veh_id;
                continue;
            }
            BOOL eligible = (cargo_left && get_triad(i) == TRIAD_LAND && !veh_cargo(i));
            if (!eligible && (!carrier_left || get_triad(i) != TRIAD_AIR
                              || has_abil(Vehs[i].proto_id, ABL_CARRIER))) {
                i = next_veh_id;
                continue;
            }
            BOOL accept;
            if (!pass) {
                // First pass re-boards only what is already recorded as riding this transport.
                accept = (Vehs[i].order == ORDER_SENTRY_BOARD
                          && Vehs[i].waypoint_x[0] == veh_id);
            } else {
                accept = true;
                if (subject_is_air) {
                    const Map *tile = &map_tiles()[(Vehs[veh_id].x >> 1)
                        + Vehs[veh_id].y * (int)MapLongitude];
                    uint32_t owner = tile->val2 & 0xF;
                    // Note the > 0 here: unlike the two other owner tests, faction 0 is rejected.
                    if (!((tile->bit & BIT_BASE_IN_TILE) && owner < MaxPlayerNum && owner > 0)
                        && !(tile->bit & BIT_AIRBASE)) {
                        accept = false;
                    }
                }
                uint32_t unit_faction = Vehs[i].faction_id;
                BOOL both_unk_200 = ((Vehs[veh_id].state & VSTATE_UNK_200)
                                     && (Vehs[i].state & VSTATE_UNK_200));
                if (accept && ((1 << unit_faction) & *FactionsStatus) && !both_unk_200) {
                    accept = (Vehs[i].order == ORDER_SENTRY_BOARD)
                        ? (Vehs[i].waypoint_x[0] < 0) : is_open_ocean;
                } else if (accept) {
                    if (Vehs[i].state & VSTATE_UNK_40000) {
                        const Map *tile = &map_tiles()[(Vehs[i].x >> 1)
                            + Vehs[i].y * (int)MapLongitude];
                        accept = ((Vehs[i].state & VSTATE_UNK_20000)
                                  && PlayersData[unit_faction].region_base_plan[tile->region]
                                         == PLAN_NAVAL_TRANSPORT);
                    } else {
                        uint8_t plan = VehPrototypes[Vehs[i].proto_id].plan;
                        if (Vehs[i].order == ORDER_HOLD || plan == PLAN_ALIEN_ARTIFACT
                            || plan == PLAN_TERRAFORMING) {
                            accept = is_open_ocean;
                        } else if (!is_open_ocean) {
                            int16_t unit_x = Vehs[i].x;
                            int16_t unit_y = Vehs[i].y;
                            accept = !(on_map(unit_x, unit_y)
                                       && PlayersData[unit_faction]
                                              .region_base_plan[region_at(unit_x, unit_y)]
                                          == PLAN_DEFENSIVE
                                       && plan < PLAN_COLONIZATION);
                        }
                    }
                }
            }
            if (accept && !(!mode && (Vehs[i].state & VSTATE_UNK_1))) {
                sleep_call(i);
                Vehs[i].waypoint_x[0] = (int16_t)veh_id;
                if (mode) {
                    veh_drop(veh_lift(i), -2, -2);
                    Vehs[i].state &= ~VSTATE_UNK_10000;
                } else {
                    Vehs[i].state |= VSTATE_UNK_1;
                }
                if (carrier_left > 0) {
                    carrier_left--;
                }
                if (cargo_left > 0) {
                    cargo_left--;
                }
            }
            i = next_veh_id;
        }
    }
    if (mode) {
        return veh_drop(veh_lift(veh_id), -2, -2);
    }
    return veh_cargo(veh_id) ? cargo_left : carrier_left;
}

/*
Purpose: Various unit stack related calculations based on type parameter (0-19) and conditions.
// ORIGINAL: 0x005B9580 ?stack_check@@YAHHHHHH@Z 0x005B9580-0x005B9BB8
// RULED-OUT: call_diff still FEWER by one - case 12's `weap_strat(...)` call gets auto-inlined by this tree (weap_strat is small/BYTE_EXACT after this same pass) where the image keeps it a real `call`. `#pragma auto_inline(off)` around weap_strat's OWN definition would fix this call site but also stops weap_val (0x0057D360, not in this batch) from inlining it, which the image DOES do there - out of scope to touch a function outside this batch to fix this one. Still MISMATCH overall (~0.28 similar best) - a 20-case switch this size has per-case register allocation/instruction-scheduling noise that dwarfs the two structural fixes above.
// RULED-OUT: MORE - this tree called `veh_top(veh_id)` and (case 8) `veh_cargo(i)`, neither of which the image calls (both fully inlined here, matching veh_at's precedent on veh_top and veh_cargo's own inline expansion for case 8).
// size      1592 bytes
// prototype int (__cdecl ?stack_check@@YAHHHHHH@Z)(int vehID, int type, int cond1, int cond2, int cond3)
// callers   39   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D270 0x0057D2E0 0x005BF1F0 0x005C0DB0 0x005C1760
Return Value: Dependent on type parameter
Status: Complete
*/
int __cdecl stack_check(int veh_id, int type, int cond1, int cond2, int cond3) {
    int retn_val = 0;
    uint32_t plan;
    uint32_t chas;
    int top_veh_id = -1;
    if (veh_id >= 0) {
        top_veh_id = veh_id;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
        switch (type) {
          case 0:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) && Vehs[i].proto_id == cond1) {
                retn_val++;
            }
            break;
          case 1:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                retn_val++;
            }
            break;
          case 2:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) 
                && VehPrototypes[Vehs[i].proto_id].plan == cond1) {
                retn_val++;
            }
            break;
          case 3:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) && get_triad(i) == cond1) {
                retn_val++;
            }
            break;
          case 4:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                retn_val += get_offense_rating(i);
            }
            break;
          case 5:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                retn_val += get_defense_rating(i);
            }
            break;
          case 6:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) && has_abil(Vehs[i].proto_id, cond1)) {
                retn_val++;
            }
            break;
          case 7:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                retn_val += VehPrototypes[Vehs[i].proto_id].cost;
            }
            break;
          case 8:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                uint32_t triad = get_triad(i);
                if (triad == TRIAD_LAND) {
                    retn_val--;
                } else if (triad == TRIAD_SEA) {
                    // inlined veh_cargo(i): the image has no call here, see veh_cargo's own comment
                    int16_t proto_id_cargo = Vehs[i].proto_id;
                    uint32_t cargo = VehPrototypes[proto_id_cargo].carry_capacity;
                    retn_val += (cargo && proto_id_cargo < MaxVehProtoFactionNum
                        && (get_proto_offense_rating(proto_id_cargo) < 0
                            || proto_id_cargo == BSC_SPORE_LAUNCHER))
                        ? Vehs[i].morale + 1 : cargo;
                }
            }
            break;
          case 9:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) && Vehs[i].order == cond1) {
                retn_val++;
            }
            break;
          case 10:
            if (Vehs[i].faction_id == cond1) {
                retn_val++;
            }
            break;
          case 11:
            if ((cond3 < 0 || Vehs[i].faction_id == cond3)
                && (Vehs[i].state & cond1) == (uint32_t)cond2) {
                retn_val++;
            }
            break;
          case 12:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                uint32_t faction_id = Vehs[i].faction_id;
                int proto_id = Vehs[i].proto_id;
                if (arm_strat(VehPrototypes[proto_id].armor_id, faction_id) 
                    > weap_strat(VehPrototypes[proto_id].weapon_id, faction_id)) {
                    retn_val++;
                }
            }
            break;
          case 13:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1) && is_missile(i)) {
                retn_val++;
            }
            break;
          case 14:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1)
                && (plan = VehPrototypes[Vehs[i].proto_id].plan,
                    plan == PLAN_DEFENSIVE || plan == PLAN_COMBAT)) {
                retn_val++;
            }
            break;
          case 15:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1) && can_arty(Vehs[i].proto_id, TRUE)) {
                retn_val++;
            }
            break;
          case 16:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1)
                && (plan = VehPrototypes[Vehs[i].proto_id].plan, plan == PLAN_DEFENSIVE
                    || plan == PLAN_RECONNAISANCE || plan == PLAN_COMBAT)) {
                retn_val++;
            }
            break;
          case 17:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2)
                && VehPrototypes[Vehs[i].proto_id].unk_1 == cond1) {
                retn_val++;
            }
            break;
          case 18:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1)
                && (chas = VehPrototypes[Vehs[i].proto_id].chassis_id,
                    Chassis[chas].triad == TRIAD_AIR && Chassis[chas].range > 1)) {
                retn_val++;
            }
            break;
          case 19:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1) && !get_offense_rating(i)) {
                retn_val++;
            }
            break;
          default:
            break;
        }
    }
    return retn_val;
}

/*
Purpose: Check to see whether provided faction and base can build a specific prototype. Checks are
         included to prevent SMACX specific units from being built in SMAC mode.
// ORIGINAL: 0x005BA910 ?veh_avail@@YAHHHH@Z 0x005BA910-0x005BAB39
// LEVER: WRONG CALLEE - is_port(base_id, false) hand-inlined to
//   is_coast(Bases[base_id].x, Bases[base_id].y, false); the image never calls is_port.
//   Split every `||`-combined early return (PROTO_ACTIVE/obsolete_factions, and the
//   whole 18-way !ExpansionEnabled SMAC-mode block) into sequential `if (...) return
//   false;` statements - the image gives each condition its own duplicated epilogue
//   rather than sharing one merged branch. The ability_flags checks are bitwise
//   (`abil_flag & ABL_X`), not equality - the image emits `test`, not `cmp`. Together
//   0.538 -> 0.996 similar; RULED-OUT: the last ~6-instruction gap is register
//   allocation inside has_tech()/is_coast()'s own inlined bodies (shared MEASURED
//   inline helpers) - did not chase further to avoid touching their other callers.
// size      553 bytes
// prototype int (__cdecl ?veh_avail@@YAHHHH@Z)(int protoID, int factionID, int baseID)
// callers   7   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E49D0
Return Value: Is unit available to faction/base? true/false
Status: Complete
*/
BOOL __cdecl veh_avail(int proto_id, int faction_id, int base_id) {
    // The image gives PROTO_ACTIVE and obsolete_factions separate early returns, each
    // with its own epilogue, rather than one merged `||` - reproduced as sequential ifs.
    if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)) {
        return false;
    }
    if (VehPrototypes[proto_id].obsolete_factions & (1 << faction_id)) {
        return false;
    }
    if (proto_id < MaxVehProtoFactionNum) {
        if (!has_tech(VehPrototypes[proto_id].preq_tech, faction_id)) {
            return false;
        }
    }
    if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION
        && GameRules & RULES_SCN_NO_COLONY_PODS) {
        return false;
    }
    // is_port(), hand-inlined - the image calls is_coast() directly here, not is_port().
    if (base_id >= 0 && get_proto_triad(proto_id) == TRIAD_SEA
        && !is_coast(Bases[base_id].x, Bases[base_id].y, false)) {
        return false;
    }
    // The image splits this whole SMAC-mode block into eighteen separate early
    // returns, each with its own epilogue, rather than one merged `||` chain -
    // reproduced as sequential ifs. The ability_flags checks are bitwise (`test`
    // against a single flag bit), not equality - `abil_flag == ABL_X` would only
    // fire for a unit with exactly that one ability and nothing else.
    if (!ExpansionEnabled) {
        if (VehPrototypes[proto_id].armor_id > ARM_PSI_DEFENSE) {
            return false;
        }
        uint8_t weap_id = VehPrototypes[proto_id].weapon_id;
        if (weap_id == WPN_RESONANCE_LASER) {
            return false;
        }
        if (weap_id == WPN_RESONANCE_BOLT) {
            return false;
        }
        if (weap_id == WPN_STRING_DISRUPTOR) {
            return false;
        }
        if (weap_id == WPN_TECTONIC_PAYLOAD) {
            return false;
        }
        if (weap_id == WPN_FUNGAL_PAYLOAD) {
            return false;
        }
        uint32_t abil_flag = VehPrototypes[proto_id].ability_flags;
        if (abil_flag & ABL_SOPORIFIC_GAS) {
            return false;
        }
        if (abil_flag & ABL_DISSOCIATIVE_WAVE) {
            return false;
        }
        if (abil_flag & ABL_MARINE_DETACHMENT) {
            return false;
        }
        if (abil_flag & ABL_FUEL_NANOCELLS) {
            return false;
        }
        if (abil_flag & ABL_ALGO_ENHANCEMENT) {
            return false;
        }
        if (proto_id == BSC_SEALURK) {
            return false;
        }
        if (proto_id == BSC_SPORE_LAUNCHER) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK1) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK2) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK3) {
            return false;
        }
        if (proto_id == BSC_FUNGAL_TOWER) {
            return false;
        }
        if (proto_id == BSC_UNITY_MINING_LASER) {
            return false;
        }
    }
    if (proto_id < MaxVehProtoFactionNum) {
        return true;
    }
    return (proto_id / MaxVehProtoFactionNum) == faction_id;
}

/*
Purpose: Determine whether a faction wants the specified prototype based on the faction's current
         prototype designs.
// ORIGINAL: 0x005BE100 ?wants_prototype@@YAHHH@Z 0x005BE100-0x005BE29D
// RULED-OUT: nested ternary chains (`cond ? a : cond2 ? b : c`) used as a single `if (...)` condition compiled to SETcc-then-test where the image branches directly - splitting each into if/else-if with explicit `continue` (the loop's early-out target is one shared label in the image) recovered the direct jumps. Also split the combined `if (!A || !B) return false;` guard at the top into two separate ifs - the image has two full, separate epilogues, one per check, not a combined OR. And walking a `VehPrototype *proto_cmp` pointer (++proto_cmp each iteration) instead of indexing `VehPrototypes[proto_id_cmp]` per field matches the image's single incrementing esi and its negative-offset field reads (esi-1 for chassis_id, esi+5 for plan) - though the image anchors that pointer at the weapon_id field specifically (esi+0), where this tree's pointer anchors at the struct's own front, a difference not chased further. Moved 0.617 -> 0.865 similar (/O2 /Gy /GR- /Oy- /GX).
// size      413 bytes
// prototype int (__cdecl ?wants_prototype@@YAHHH@Z)(int protoID, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D480
Return Value: Does faction want prototype? true/false
Status: Complete
*/
BOOL __cdecl wants_prototype(int proto_id, int faction_id) {
    uint32_t flags = VehPrototypes[proto_id].flags;
    if (!(flags & PROTO_ACTIVE)) {
        return false;
    }
    if (!(flags & PROTO_TYPED_COMPLETE)) {
        return false;
    }
    uint32_t proto_offset = faction_id * MaxVehProtoFactionNum;
    uint32_t proto_id_cmp = proto_offset;
    VehPrototype *proto_cmp = &VehPrototypes[proto_offset];
    for (uint32_t i = 0; i < MaxVehProtoFactionNum; i++, proto_id_cmp++, proto_cmp++) {
        uint32_t flags_cmp = proto_cmp->flags;
        if (!(flags_cmp & PROTO_ACTIVE)) {
            continue;
        }
        if (!(flags_cmp & PROTO_TYPED_COMPLETE)) {
            continue;
        }
        if (VehPrototypes[proto_id].plan != proto_cmp->plan) {
            continue;
        }
        uint8_t chas_cmp = proto_cmp->chassis_id;
        uint8_t chas = VehPrototypes[proto_id].chassis_id;
        if (Chassis[chas].triad != Chassis[chas_cmp].triad) {
            continue;
        }
        uint8_t weap_id_cmp = proto_cmp->weapon_id;
        uint8_t mode_cmp = Weapon[weap_id_cmp].mode;
        if (mode_cmp > WPN_MODE_MISSILE) {
            if (mode_cmp != Weapon[VehPrototypes[proto_id].weapon_id].mode) {
                continue;
            }
        } else {
            int8_t off_rating_cmp = Weapon[weap_id_cmp].offense_rating;
            if (off_rating_cmp < 0) {
                if (off_rating_cmp != get_proto_offense_rating(proto_id)) {
                    continue;
                }
            } else if (off_rating_cmp < get_proto_offense_rating(proto_id)) {
                continue;
            }
        }
        uint8_t arm_id_cmp = proto_cmp->armor_id;
        if (Armor[arm_id_cmp].defense_rating <= 0) {
            if (arm_id_cmp != VehPrototypes[proto_id].armor_id) {
                continue;
            }
        } else if (armor_val(proto_id_cmp, faction_id) < armor_val(proto_id, faction_id)) {
            continue;
        }
        if (Chassis[chas_cmp].speed >= Chassis[chas].speed) {
            return false;
        }
    }
    return true;
}

/*
Purpose: Check the coordinates for units and if at least one is found return the top most id.
// ORIGINAL: 0x005BFE90 ?veh_at@@YAHHH@Z 0x005BFE90-0x005BFF9E
// RULED-OUT: 0.937 similar under every flag set tried - the control flow and
//        field reads already match; what's left is register allocation (the
//        image hoists `MapLatitudeBounds`/`MapLongitudeBounds` into ebx/edi
//        ONCE and reuses them for both `on_map()` call sites, this tree
//        re-reads them at the second site) plus esi/edi swaps throughout.
//        Left as-is per "do not chase register-allocation differences."
// size      270 bytes
// prototype int (__cdecl ?veh_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   61   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532A90 0x006262F0
Return Value: Unit id or -1 if nothing is found/error
Status: Complete
*/
// LEVER: veh_top() is called at only 2 real sites in the image (see its own comment);
//   here it is inlined - call_diff flagged the `call veh_top` this tree was making that
//   the image does not. Inlined the body by hand instead of `return veh_top(veh_id);`.
int __cdecl veh_at(int x, int y) {
    if (on_map(x, y) && !(bit_at(x, y) & BIT_VEH_IN_TILE)) {
        return -1; // not found
    }
    for (int veh_id = 0; veh_id < VehCurrentCount; veh_id++) {
        if (Vehs[veh_id].x == x && Vehs[veh_id].y == y) {
            if (veh_id < 0) {
                return -1;
            }
            int top_veh_id = veh_id;
            for (int i = Vehs[top_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
                top_veh_id = i;
            }
            return top_veh_id;
        }
    }
    if (!on_map(x, y)) {
        return -1;
    }
    if (!VehBitError) {
        log_say("Vehicle Bit Error  (x, y)", x, y, 0);
    }
    if (GameState & STATE_SCENARIO_EDITOR || GameState & STATE_DEBUG_MODE || IsMultiplayerNet) {
        if (VehBitError) {
            return -1;
        }
        VehBitError = true;
    }
    rebuild_vehicle_bits();
    return -1;
}

/*
Purpose: Check whether the prototype has a specific ability.
// ORIGINAL: 0x005BF1F0 ?has_abil@@YAHHH@Z 0x005BF1F0-0x005BF310
// RULED-OUT: a negative-proto_id guard clause (with its log_say call) that call_diff flagged as an extra call the image does not make; now 13/109 instructions agree, 0.825 similar (best flags), up from a structurally-unrelated function body before
// size      288 bytes
// prototype int (__cdecl ?has_abil@@YAHHH@Z)(int protoID, int abilityID)
// callers   54   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Does prototype have ability? true/false
Status: Complete
*/
BOOL __cdecl has_abil(int proto_id, int ability_id) {
    // BUG IN THE ORIGINAL: no guard against a negative proto_id here - it flows straight
    // into `proto_id / MaxVehProtoFactionNum` and an out-of-bounds VehPrototypes[] read.
    // A prior pass added a guard clause (with a log_say call) that is NOT in the image;
    // call_diff flagged the extra call, and the disassembly at 0x005BF1F0 confirms the
    // first instructions compute proto_id*sizeof(VehPrototype) directly, no branch first.
    if (VehPrototypes[proto_id].ability_flags & ability_id) {
        return true;
    }
    uint32_t faction_id = proto_id / MaxVehProtoFactionNum;
    if (!faction_id) {
        return false; // skip basic prototypes from #UNITS
    }
    if (is_alien_faction(faction_id) && ability_id == ABL_DEEP_RADAR) {
        return true; // Caretakers + Usurpers > "Deep Radar" ability for all units
    }
    for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
        if (Players[faction_id].faction_bonus_id[i] == RULE_FREEABIL) {
            int abil_bonus_id = Players[faction_id].faction_bonus_val1[i];
            if (has_tech(Ability[abil_bonus_id].preq_tech, faction_id) &&
                (ability_id & (1 << abil_bonus_id))) {
                // Pirates > "Marine Detachment" ability for combat sea units with Adaptive Doctrine
                return true;
            }
        }
    }
    if (VehPrototypes[proto_id].weapon_id == WPN_PROBE_TEAM && ability_id == ABL_ALGO_ENHANCEMENT
        && has_project(SP_NETHACK_TERMINUS, faction_id)) {
        return true; // All Probe Teams act as though they have the "Algorithmic Enhancement"
    }
    return false;
}

/*
Purpose: Temporarily remove the specified unit from its current square and stack in preparation for 
         another action such as interacting with the stack, moving or killing it.
// ORIGINAL: 0x005BFFA0 ?veh_lift@@YAXH@Z 0x005BFFA0-0x005C007F
// RULED-OUT: swapping the x/y local declaration order - the remaining gap is which register (eax/ecx/edx) the allocator picks for x, y and the -1 constant at the join point; both orders produced the identical register assignment, so it isn't source-order controlled here. Plateau at 49/60.
// RULED-OUT: were cached into locals BEFORE the prev/next stack-pointer fixup and the on_map/bit_set branch; the image re-reads `Vehs[veh_id].x`/`.y` fresh at each site (inside the branch, and again at the join point for VehLiftX/Y) instead of hoisting one read to the top. Also `VehDropLiftVehID = veh_id;` moved to AFTER the x/y reads (image assigns it between reading y and zeroing the fields, not before). 4/60 -> 49/60 (0.829 -> 0.983 similar).
// symbol    ?veh_lift@@YAHH@Z
// size      223 bytes
// prototype void (__cdecl ?veh_lift@@YAXH@Z)(int vehID)
// callers   18   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60
Return Value: Unit id
Status: Complete
*/
int __cdecl veh_lift(int veh_id) {
    BOOL prev_stack_exists = false;
    int16_t prev_veh_id = Vehs[veh_id].prev_veh_id_stack;
    if (prev_veh_id >= 0) {
        prev_stack_exists = true;
        Vehs[prev_veh_id].next_veh_id_stack = Vehs[veh_id].next_veh_id_stack;
    }
    int16_t next_veh_id = Vehs[veh_id].next_veh_id_stack;
    if (next_veh_id >= 0) {
        Vehs[next_veh_id].prev_veh_id_stack = Vehs[veh_id].prev_veh_id_stack;
    } else if (!prev_stack_exists && on_map(Vehs[veh_id].x, Vehs[veh_id].y)) {
        bit_set(Vehs[veh_id].x, Vehs[veh_id].y, BIT_VEH_IN_TILE, false);
    }
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    VehDropLiftVehID = veh_id;
    Vehs[veh_id].x = -1;
    Vehs[veh_id].y = -1;
    Vehs[veh_id].next_veh_id_stack = -1;
    Vehs[veh_id].prev_veh_id_stack = -1;
    VehLiftX = x;
    VehLiftY = y;
    return veh_id;
}

/*
Purpose: Move the specified unit to the provided coordinates.
// ORIGINAL: 0x005C0080 ?veh_drop@@YAXHHH@Z 0x005C0080-0x005C019C
// RULED-OUT: the last `mov ecx,[ebp+8]; mov eax,ecx` before the final `ret` - the image loads straight into eax there (ecx is dead, clobbered by the `bit_set` call just before), but this tree's register allocator prefers ecx as veh_id's home register for the whole function and copies to eax at the return. No source reshuffle around the final `return veh_id;` moved it; plateau.
// RULED-OUT: (veh_id_dest < 0) {...} else {...}` had the arms backwards - the image falls through the `>= 0` write first and jumps to the `< 0` block, so the source needs `if (veh_id_dest >= 0) {...} else {...}` to get that polarity. Also the flags ternary `(A && B) ? X|Y : Y` compiled an extra `mov` before the compare the image folds into a direct `cmp byte ptr [...], 2`; splitting it into `flags = 0/BIT_SUPPLY_REMOVE; flags |= BIT_VEH_IN_TILE;` (the image ORs BIT_VEH_IN_TILE in unconditionally at a shared tail) dropped that mov. 39/95 -> 80/95 (0.880 -> 0.973 similar).
// symbol    ?veh_drop@@YAHHHH@Z
// size      284 bytes
// prototype void (__cdecl ?veh_drop@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   17   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591B10 0x00591D60 0x005BFE90
Return Value: Unit id (1st param), doesn't seem to be used
Status: Complete
*/
int __cdecl veh_drop(int veh_id, int x, int y) {
    int veh_id_dest = veh_at(x, y);
    Vehs[veh_id].next_veh_id_stack = (int16_t)veh_id_dest;
    Vehs[veh_id].prev_veh_id_stack = -1;
    Vehs[veh_id].x = (int16_t)x;
    Vehs[veh_id].y = (int16_t)y;
    VehDropLiftVehID = -1;
    if (veh_id_dest >= 0) {
        Vehs[veh_id_dest].prev_veh_id_stack = (int16_t)veh_id;
    } else {
        if (y < 0) {
            return veh_id;
        }
        if (on_map(x, y) && !(bit_at(x, y) & BIT_BASE_IN_TILE)) {
            owner_set(x, y, Vehs[veh_id].faction_id);
        }
    }
    if (on_map(x, y)) {
        uint32_t flags;
        if (!Vehs[veh_id].faction_id || get_triad(veh_id) == TRIAD_AIR) {
            flags = 0;
        } else {
            flags = BIT_SUPPLY_REMOVE;
        }
        flags |= BIT_VEH_IN_TILE;
        bit_set(x, y, flags, true);
    }
    return veh_id;
}

/*
Purpose: Set the unit's status to sentry/board.
// ORIGINAL: 0x005C01A0 ?sleep@@YAXH@Z 0x005C01A0-0x005C01CA BYTE_EXACT
// size      42 bytes
// prototype void (__cdecl ?sleep@@YAXH@Z)(int vehID)
// callers   8   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// See veh.h: a real, out-of-line forwarder so stack_veh's one call site gets
// an actual `call` where `sleep(i)` directly would inline away under /O2.
void __cdecl sleep_call(int veh_id) {
    sleep(veh_id);
}

// See veh.h: a real, out-of-line forwarder so veh_cost's one call site gets
// an actual `call` where `base_cost(proto_id)` directly would inline away
// under /O2.
int __cdecl base_cost_call(int proto_id) {
    return base_cost(proto_id);
}

/*
Purpose: Move the specified unit to the bottom of the stack.
// ORIGINAL: 0x005C01D0 ?veh_demote@@YAXH@Z 0x005C01D0-0x005C0256
// RULED-OUT: byte-exactness. Best is 0.916 similar (4/40 in position) at
// `/c /O2 /Gy /GR- /Oy- /GX`, and every remaining edit is register
// allocation - `esi`/`edi`/`ebx` permuted against the image's fixed
// `ebx`=veh_id, `esi`=last_veh_id, `edi`=&Vehs[veh_id] - with no control
// flow or field-order difference. Measured two spellings: nested `if` and
// guard-clause early-return (identical 0.916, guard clause kept as closer
// to the image's flat `jl`/`jl`/`je` shape), and a cached `Veh *` for
// `&Vehs[veh_id]`/`&Vehs[last_veh_id]` (WORSE, 0.892 - an extra `lea`
// survives past the call where the image recomputes the address after
// `veh_lift` returns). Do not re-derive the pointer-cache attempt.
// size      134 bytes
// prototype void (__cdecl ?veh_demote@@YAXH@Z)(int vehID)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFFA0
Return Value: n/a
Status: Complete
*/
void __cdecl veh_demote(int veh_id) {
    if (veh_id < 0) {
        return;
    }
    int16_t next_veh_id = Vehs[veh_id].next_veh_id_stack;
    if (next_veh_id < 0) {
        return;
    }
    int16_t last_veh_id;
    do {
        last_veh_id = next_veh_id;
        next_veh_id = Vehs[last_veh_id].next_veh_id_stack;
    } while (next_veh_id >= 0);
    if (last_veh_id == veh_id) {
        return;
    }
    veh_lift(veh_id);
    Vehs[last_veh_id].next_veh_id_stack = (int16_t)veh_id;
    Vehs[veh_id].prev_veh_id_stack = last_veh_id;
    Vehs[veh_id].next_veh_id_stack = -1;
    Vehs[veh_id].x = Vehs[last_veh_id].x;
    Vehs[veh_id].y = Vehs[last_veh_id].y;
}

/*
Purpose: Move the specified unit to the top of the stack.
// ORIGINAL: 0x005C0260 ?veh_promote@@YAXH@Z 0x005C0260-0x005C02CE
// RULED-OUT: a `veh_id_top >= 0` guard after the hand-inlined veh_top - the image has none (see BUG note below). RULED-OUT: seeding `veh_id_top = veh_id` before vs. inside the `if` - same codegen either way. The remaining gap is the compiler proving the `prev_veh_id_stack < 0` (zero-iteration) case makes `veh_id_top != veh_id` tautologically false and skipping straight to the epilogue, where the image still materializes and compares it; not reachable by reordering statements. Plateau at 3/40.
// RULED-OUT: CALLEE - the tree called `veh_top()`/`veh_put()`, both inline-marked wrappers, but this call site is not one of veh_top's two real sites and veh_put did not inline here either. Hand- inlining both (veh_top's loop, and veh_put's `veh_lift(id); veh_drop(id, x, y)` - dropping veh_lift's unused return since it always returns its own argument) took call_diff from WRONG CALLEE to 0 disagreeing and 0.657 -> 0.773 similar.
// size      110 bytes
// prototype 
// callers   6   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFFA0 0x005C0080
Return Value: n/a
Status: Complete
*/
void __cdecl veh_promote(int veh_id) {
    // veh_top(), hand-inlined - matching stack_fix's precedent (see its own
    // comment): the image does not call it here.
    int veh_id_top = veh_id;
    if (veh_id >= 0) {
        for (int i = Vehs[veh_id_top].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
            veh_id_top = i;
        }
    } else {
        veh_id_top = -1;
    }
    // BUG IN THE ORIGINAL: no `veh_id_top >= 0` guard here - if veh_id is
    // itself negative, veh_id_top becomes -1 and this still proceeds to
    // index Vehs[-1] below, as long as veh_id != -1 exactly.
    if (veh_id_top != veh_id) {
        // veh_put(), hand-inlined: the image calls veh_lift and veh_drop
        // directly here, discarding veh_lift's return (it always returns
        // its own veh_id argument unchanged) rather than threading it
        // through to veh_drop's first parameter.
        veh_lift(veh_id);
        veh_drop(veh_id, Vehs[veh_id_top].x, Vehs[veh_id_top].y);
    }
}

/*
Purpose: Clear the specified unit.
// ORIGINAL: 0x005C02D0 ?veh_clear@@YAXHHH@Z 0x005C02D0-0x005C03C3
// RULED-OUT: `Players[faction_id].rule_morale` into its own statement, placed right after the `order_auto_type = 0;` store (where the image starts computing the address) and adding 1 at the later `morale = ...` store site instead of folding the `+1` into the read, moved this from 44/57 (0.895 similar, MISMATCH) to 55/57 (1.000 similar, MNEMONIC_ONLY). The remaining 2-instruction divergence is VC6 scheduling the actual byte LOAD (`mov dl, byte ptr [edx*4+...]`) between `move_to_ai_type` and `visibility` while this tree's load lands one store earlier; the address computation (lea/lea/shl/sub) already matches exactly. Moving the local's declaration later (to sit textually where the image's load falls) moved the ADDRESS computation too and lost the match (back to 44/57 or worse) - the image splits one C expression into an early address calc and a late load, which no single placement of one statement reproduces. Not chased further.
// size      243 bytes
// prototype void (__cdecl ?veh_clear@@YAXHHH@Z)(int vehID, int protoID, int factionID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl veh_clear(int veh_id, int proto_id, int faction_id) {
    Vehs[veh_id].x = -4;
    Vehs[veh_id].y = -4;
    Vehs[veh_id].year_end_lurking = 0;
    Vehs[veh_id].unk_1 = 0;
    Vehs[veh_id].flags = 0;
    Vehs[veh_id].faction_id = (uint8_t)faction_id;
    Vehs[veh_id].proto_id = (int16_t)proto_id;
    Vehs[veh_id].next_veh_id_stack = -1;
    Vehs[veh_id].prev_veh_id_stack = -1;
    Vehs[veh_id].waypoint_count = 0;
    Vehs[veh_id].patrol_current_point = 0;
    Vehs[veh_id].order = ORDER_NONE;
    for (uint32_t i = 0; i < 4; i++) {
        Vehs[veh_id].waypoint_x[i] = -1;
        Vehs[veh_id].waypoint_y[i] = -1;
    }
    Vehs[veh_id].state = 0;
    Vehs[veh_id].moves_expended = 0;
    Vehs[veh_id].dmg_incurred = 0;
    Vehs[veh_id].order_auto_type = 0;
    uint8_t rule_morale = (uint8_t)Players[faction_id].rule_morale;
    Vehs[veh_id].terraforming_turns = 0;
    Vehs[veh_id].unk_6 = 0;
    Vehs[veh_id].move_to_ai_type = 0;
    Vehs[veh_id].visibility = 0;
    Vehs[veh_id].home_base_id = -1;
    Vehs[veh_id].morale = rule_morale + 1;
    Vehs[veh_id].unk_5 = 2;
    Vehs[veh_id].probe_action = 0;
    Vehs[veh_id].probe_sabotage_id = 0;
}

/*
Purpose: Check if the prototype can perform artillery combat. The 2nd parameter determines how sea 
         units are treated.
// ORIGINAL: 0x005C0DB0 ?can_arty@@YAHHH@Z 0x005C0DB0-0x005C0E35
// RULED-OUT: the OR'd (offense<=0 || defense<0) guard into two sequential `if`s, one per term - matches the image's per-term SPORE_LAUNCHER exception check. Also: the TRIAD_SEA/TRIAD_AIR dispatch is a `switch`, not chained `if`s - the image's dec/je chain is switch codegen. 19/54 -> 50/54. One divergence remains: the defense_rating byte load folds into `cmp byte ptr [edx], 0` here where the image keeps `mov bl, ...; test bl, bl` - tried swapping check order, `0 > x`, a BOOL local, and `!(x >= 0)`; none moved it. Plateau.
// size      133 bytes
// prototype int (__cdecl ?can_arty@@YAHHH@Z)(int protoID, int triad_sea_retn)
// callers   14   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0
Return Value: Has artillery ability? true/false
Status: Complete
*/
BOOL __cdecl can_arty(int proto_id, BOOL sea_triad_retn) {
    if (get_proto_offense_rating(proto_id) <= 0 && proto_id != BSC_SPORE_LAUNCHER) {
        return false;
    }
    if (get_proto_defense_rating(proto_id) < 0 && proto_id != BSC_SPORE_LAUNCHER) {
        return false;
    }
    uint8_t triad = get_proto_triad(proto_id);
    switch (triad) {
      case TRIAD_SEA:
        return sea_triad_retn; // cursory check shows this value always being set to true
      case TRIAD_AIR:
        return false;
    }
    return has_abil(proto_id, ABL_ARTILLERY); // TRIAD_LAND
}

/*
Purpose: Calculate a unit's morale. TODO: Determine if 2nd param is a toggle for display vs actual 
         morale.
// ORIGINAL: 0x005C0E40 ?morale_veh@@YAHHHH@Z 0x005C0E40-0x005C1143
// RULED-OUT: call_diff FEWER (2 vs image's 4: morale_alien, has_tech, and
//        `bitmask` TWICE) - the two `has_fac_built(FAC_CHILDREN_CRECHE/
//        FAC_BROOD_PIT, home_base_id)` calls each keep `bitmask()` as a real
//        `call 0x50ba00` in the image, but this tree fully inlines it under
//        every flag set tried, same as the already-documented case on
//        get_basic_offense (0x5015B0). Bypassing `has_fac_built` and calling
//        `bitmask()` directly (with `int offset, mask;` locals) did not stop
//        it inlining either. Otherwise the control-flow order, field offsets
//        and short-circuit structure already match; the rest of the gap
//        looks like register allocation (a cached zero register the image
//        keeps in ebx that this tree re-materializes per comparison).
// RULED-OUT (2nd attempt): base.cpp's `bitmask_call` forwarder (general.h),
//        via a local `has_fac_built_call` matching base.cpp's own helper, at
//        BOTH call sites. Call count agreed (0 disagree, was FEWER 2v4), but
//        the best flag set's similarity fell from 0.484 (4/286 exact, /O1
//        /GR- /Oy- /GX) to 0.340 (2/286 exact, /O2 /Ob0 /GR- /GX). Reverted.
// LEVER (3rd attempt): `bitmask_call` on the CRECHE site ONLY, brood-pit left
//        plain `has_fac_built`, matching the "outer real, nested/nearby
//        inline" split get_basic_offense needed. Best flag set's similarity
//        0.484 -> 0.518 (3/286 exact, /O1 /GR- /Oy- /GX); call count 2v4 ->
//        3v4 (still FEWER by 1 - the brood-pit site stays a gap, same wall
//        as the 2nd attempt's failure mode when forced real).
// size      771 bytes
// prototype int (__cdecl ?morale_veh@@YAHHHH@Z)(int vehID, int checkDroneRiot, int factionIDvsNative)
// callers   16   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00501350 0x0050BA00 0x005B9F20
Return Value: Morale value
Status: Complete
*/
int __cdecl morale_veh(int veh_id, BOOL check_drone_riot, int faction_id_vs_native) {
    uint32_t faction_id = Vehs[veh_id].faction_id;
    if (!faction_id) {
        return morale_alien(veh_id, faction_id_vs_native);
    }
    int16_t proto_id = Vehs[veh_id].proto_id;
    if (VehPrototypes[proto_id].plan == PLAN_INFO_WARFARE) { // probe
        int probe_morale = range(PlayersData[faction_id].soc_effect_active.probe, 0, 3);
        probe_morale += has_project(SP_TELEPATHIC_MATRIX, faction_id) ? 2 : 0;
        for (int i = 0; i < MaxTechnologyNum; i++) {
            if (Technology[i].flags & TFLAG_IMPROVED_PROBES && has_tech(i, faction_id)) {
                probe_morale++;
            }
        }
        probe_morale += Vehs[veh_id].morale;
        return range(probe_morale, 2, 6);
    }
    int8_t offense_rating = get_proto_offense_rating(proto_id);
    if (proto_id < MaxVehProtoFactionNum && offense_rating < 0) {
        return range(Vehs[veh_id].morale, 0, 6); // Basic Psi Veh
    }
    // everything else
    int morale_modifier = range(PlayersData[faction_id].soc_effect_active.morale, -4, 4);
    if (morale_modifier <= -2) {
        morale_modifier++;
    } else if (morale_modifier >= 2) {
        morale_modifier--;
    }
    int rule_morale = Players[faction_id].rule_morale; // different from 'SOCIAL, MORALE'
    if (rule_morale < 0) { // negative effects 1st
        morale_modifier += rule_morale;
    }
    int home_base_id = Vehs[veh_id].home_base_id;
    if (home_base_id >= 0) { // home base countering negative effects
        if (has_fac_built_call(FAC_CHILDREN_CRECHE, home_base_id) && morale_modifier < 0) {
            morale_modifier /= 2;
        }
        if (has_fac_built(FAC_BROOD_PIT, home_base_id) && proto_id < MaxVehProtoFactionNum
            && (offense_rating < 0 || proto_id == BSC_SPORE_LAUNCHER) && morale_modifier < 0) {
            morale_modifier /= 2; // never reached due to above 'Basic Psi Veh' checks
        }
    }
    if (rule_morale > 0) {
        morale_modifier += rule_morale;
    }
    BOOL morale_flag = Players[faction_id].rule_flags & RFLAG_MORALE;
    if (morale_flag && morale_modifier < 0) {
        morale_modifier = 0;
    }
    if (check_drone_riot && home_base_id >= 0 
        && Bases[home_base_id].state & BSTATE_DRONE_RIOTS_ACTIVE && !morale_flag) {
        // bug fix: removed premature range bounding negating negative morale effects
        morale_modifier--;
    }
    return range(Vehs[veh_id].morale + morale_modifier, 0, 6);
}

/*
Purpose: Calculate the offense of the specified prototype. Optional param of the unit defending 
         against (-1 to ignore) as well as whether artillery or missile combat is being utilized.
// ORIGINAL: 0x005C1150 ?offense_proto@@YAHHHH@Z 0x005C1150-0x005C128F
// RULED-OUT: `abs()` for the negate - same similarity as the manual ternary. The remaining gap is the same defense_rating byte-load fold as can_arty's plateau (`cmp byte ptr [x], 0` here vs the image's `mov dl, ...; test dl, dl`) plus register-naming noise past that point; not moved by source reshuffling.
// RULED-OUT: `uint32_t weapon_id = VehPrototypes[proto_id].weapon_id;` local, read at its two use sites, made the compiler spill it back into proto_id's own stack slot (an extra store the image never makes) and renamed registers through the entire rest of the function. Writing `VehPrototypes[proto_id].weapon_id` at both call sites instead of caching it dropped that spill: 4/112 -> 47/112 (0.882 -> 0.918 similar). Also reproduced the missing `veh_id_def < 0` guard on the SECOND `Vehs[veh_id_def].proto_id != SPORE_LAUNCHER` check (see the BUG note below) and split `off_rating` into an `int8_t` read plus a widened ternary negate rather than negating an already-widened `int` - the image tests/negates the BYTE before the `movsx`. 47/112 -> 50/112 (0.932 similar).
// size      319 bytes
// prototype int (__cdecl ?offense_proto@@YAHHHH@Z)(int protoID, int vehIDDef, BOOL isBombardment)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Prototype's offense
Status: Complete
*/
int __cdecl offense_proto(int proto_id, int veh_id_def, BOOL is_bombard) {
    if (Weapon[VehPrototypes[proto_id].weapon_id].mode == WPN_MODE_INFOWAR && veh_id_def >= 0
        && VehPrototypes[Vehs[veh_id_def].proto_id].plan == PLAN_INFO_WARFARE) {
        return 16; // probe attacking another probe
    }
    // BUG IN THE ORIGINAL: Vehs[veh_id_def].proto_id is read unguarded here
    // - if veh_id_def is -1 (no defender), this reads Vehs[-1], an
    // arbitrary out-of-bounds (Reactor struct) memory read. The image has
    // no `veh_id_def < 0` guard on this specific comparison (unlike the
    // guarded one a few lines up), so it is left alone deliberately.
    if ((is_bombard || (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating >= 0
        && (veh_id_def < 0 || get_defense_rating(veh_id_def) >= 0)))
        && Vehs[veh_id_def].proto_id != BSC_SPORE_LAUNCHER
        && proto_id != BSC_SPORE_LAUNCHER) {
        int8_t off_rating_byte = get_proto_offense_rating(proto_id);
        int off_rating = (off_rating_byte < 0) ? -off_rating_byte : off_rating_byte;
        if (is_proto_missile(proto_id) && off_rating < 99) {
            off_rating = (off_rating * 3) / 2;
        }
        return (veh_id_def < 0) ? off_rating : off_rating * 8; // conventional
    }
    return (veh_id_def < 0) ? Rules->psi_combat_ratio_atk[TRIAD_LAND] : // PSI
        Rules->psi_combat_ratio_atk[get_triad(veh_id_def)] * 8;
}

/*
Purpose: Calculate the defense of the specified prototype. Optional param if unit is being attacked 
         (-1 to ignore) as well as whether artillery or missile combat is being utilized.
// ORIGINAL: 0x005C1290 ?armor_proto@@YAHHHH@Z 0x005C1290-0x005C13AD
// size      285 bytes
// prototype int (__cdecl ?armor_proto@@YAHHHH@Z)(int protoID, int vehIDAtk, int isBombardment)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: the first if() had a fabricated `veh_id_atk < 0 ||` guard the image never checks
//   before reading Vehs[veh_id_atk].proto_id (unlike the guarded get_offense_rating() check
//   further down); removing it moved 2/97 -> 35/97 agreeing at /Oy-. Remainder looks like
//   register allocation across the shared early-return tail, not a structural difference.
Return Value: Prototype's defense
Status: Complete
*/
int __cdecl armor_proto(int proto_id, int veh_id_atk, BOOL is_bombard) {
    if (Weapon[VehPrototypes[proto_id].weapon_id].mode == WPN_MODE_INFOWAR && veh_id_atk >= 0
        && VehPrototypes[Vehs[veh_id_atk].proto_id].plan == PLAN_INFO_WARFARE) {
        return 16; // probe defending against another probe
    }
    // BUG IN THE ORIGINAL: this branch reads Vehs[veh_id_atk].proto_id with no veh_id_atk >= 0
    // guard, unlike the get_offense_rating() check a few lines below which does guard it. When
    // is_bombard is set and veh_id_atk is -1 (no attacker), the image reads Vehs[-1], an
    // out-of-bounds array access. Left as shipped rather than adding the missing guard.
    if (is_bombard && Vehs[veh_id_atk].proto_id != BSC_SPORE_LAUNCHER
        && proto_id != BSC_SPORE_LAUNCHER || (get_proto_defense_rating(proto_id) >= 0
            && (veh_id_atk < 0 || get_offense_rating(veh_id_atk) >= 0))) {
        uint32_t def_rating = range(get_proto_defense_rating(proto_id), 1, 9999);
        return (veh_id_atk < 0) ? def_rating : def_rating * 8; // conventional
    }
    return (veh_id_atk < 0) ? Rules->psi_combat_ratio_def[TRIAD_LAND] : // PSI
        Rules->psi_combat_ratio_def[get_proto_triad(proto_id)] * 8;
}

/*
Purpose: Calculate the speed of the specified prototype on roads.
// ORIGINAL: 0x005C13B0 ?speed_proto@@YAHH@Z 0x005C13B0-0x005C1532
// size      386 bytes
// prototype int (__cdecl ?speed_proto@@YAHH@Z)(int protoID)
// callers   12   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0
// RULED-OUT: nothing changed - traced the full disassembly instruction-by-instruction
//   against this body (struct offsets, ABL_* constant values, WPN_MODE_TRANSPORT==7,
//   the TRIAD_AIR-only tail) and the control flow order already matches the image
//   exactly. Best flags give 0.81-0.82 similar; remainder looks like register
//   allocation / frame-pointer choice, not a structural difference.
Return Value: Prototype's speed on roads
Status: Complete
*/
int __cdecl speed_proto(int proto_id) {
    if (proto_id == BSC_FUNGAL_TOWER) {
        return 0; // cannot move
    }
    int chas_id = VehPrototypes[proto_id].chassis_id;
    int speed_val = Chassis[chas_id].speed;
    uint8_t triad = Chassis[chas_id].triad;
    uint32_t weap_id = VehPrototypes[proto_id].weapon_id;
    if (triad == TRIAD_AIR) {
        speed_val += VehPrototypes[proto_id].reactor_id * 2;
    }
    if (Weapon[weap_id].mode == WPN_MODE_TRANSPORT) {
        speed_val--;
    }
    if (has_abil(proto_id, ABL_SLOW)) {
        speed_val--;
    }
    if (has_abil(proto_id, ABL_ANTIGRAV_STRUTS)) {
        speed_val += (triad == TRIAD_AIR) ? VehPrototypes[proto_id].reactor_id * 2 : 1;
    }
    if (triad == TRIAD_AIR) {
        if (has_abil(proto_id, ABL_FUEL_NANOCELLS)) {
            speed_val += 2;
        }
        if (has_project(SP_CLOUDBASE_ACADEMY, proto_id / MaxVehProtoFactionNum)) {
            speed_val += 2; // bonus from Aerospace Complex
        }
        if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
            // generally -20% to -25%, in some cases higher due to lossy division rounding
            speed_val = (speed_val * 4) / 5;
        }
        if (Weapon[weap_id].mode == WPN_MODE_TRANSPORT) {
            speed_val /= 2; // 2nd penalty for air transports: -50%
        }
    }
    return range(speed_val, 1, 99) * Rules->move_rate_roads;
}

/*
Purpose: Calculate the speed of a unit on roads taking into consideration prototype speed, 
         elite morale, if the unit is damaged and other factors. The skip_morale parameter seems to 
         only be set to true for certain combat calculations in battle_fight().
// ORIGINAL: 0x005C1540 ?speed@@YAHHH@Z 0x005C1540-0x005C1752
// RULED-OUT: is called UNCONDITIONALLY in the image - `!skip_morale` is tested only AFTER the call and its MORALE_ELITE comparison, not before it (short-circuit order swapped: `morale_veh(...) == MORALE_ELITE && !skip_morale && (...)` instead of `!skip_morale && morale_veh(...) == MORALE_ELITE && (...)`). Confirmed against the disassembly: `call 0x5c0e40` at 0x5C15CB has no guard before it, and the skip_morale parameter isn't read until 0x5C15D8, after the MORALE_ELITE compare at 0x5C15D3.
// size      530 bytes
// prototype int (__cdecl ?speed@@YAHHH@Z)(int vehID, int toggle)
// callers   27   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C0E40 0x005C13B0
Return Value: Speed
Status: Complete
*/
int __cdecl speed(int veh_id, BOOL skip_morale) {
    uint32_t proto_id = Vehs[veh_id].proto_id;
    if (proto_id == BSC_FUNGAL_TOWER) { // moved this check to top vs bottom, same logic
        return 0; // cannot move
    }
    uint32_t speed_val = speed_proto(proto_id);
    uint8_t triad = get_proto_triad(proto_id);
    if (triad == TRIAD_SEA && has_project(SP_MARITIME_CONTROL_CENTER, Vehs[veh_id].faction_id)) {
        speed_val += Rules->move_rate_roads * 2;
    }
    if (morale_veh(veh_id, true, 0) == MORALE_ELITE && !skip_morale
        && (proto_id >= MaxVehProtoFactionNum || get_proto_offense_rating(proto_id) >= 0)) {
        speed_val += Rules->move_rate_roads;
    }
    if (Vehs[veh_id].dmg_incurred && triad != TRIAD_AIR) {
        // optimized code without changes to logic
        uint32_t moves = speed_val / Rules->move_rate_roads;
        uint32_t reactor_fac;
        if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
            speed_val = reactor_fac = 1;
        } else {
            reactor_fac = range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
            speed_val = range(reactor_fac, 1, 99);
        }
        speed_val = (moves 
            * range(reactor_fac - Vehs[veh_id].dmg_incurred, 0, 9999) + speed_val - 1) / speed_val;
        speed_val = range(speed_val, (triad == TRIAD_SEA) ? 2 : 1, 999) * Rules->move_rate_roads;
    }
    return speed_val;
}

/*
Purpose: Calculate the cargo capacity of a unit. It seems Spore Launchers were considered to have 
         cargo capacity at one time.
// ORIGINAL: 0x005C1760 ?veh_cargo@@YAHH@Z 0x005C1760-0x005C17C4
// RULED-OUT: still MISMATCH (0.853 similar) - the image computes `proto_id*sizeof(VehPrototype)` ONCE and reuses it for both the `carry_capacity` and `weapon_id` field reads (same array element, two fields); this tree recomputes it for the second read under every flag set tried. A local `VehPrototype *proto = &VehPrototypes[proto_id]` used for both field reads did not change the codegen. Also see the `add`-before-`shl` base-folding note on arm_strat (0x0057D270) - same class of divergence on the `carry_capacity` read itself.
// RULED-OUT: proto_id` (not `uint32_t`/`int`) matches the image, which keeps the 16-bit `proto_id` in `si` for the `< MaxVehProtoFactionNum` and `== BSC_SPORE_LAUNCHER` compares and only does `movsx eax,si` at the array-indexing use sites.
// size      100 bytes
// prototype int (__cdecl ?veh_cargo@@YAHH@Z)(int vehID)
// callers   19   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Cargo capacity
Status: Complete
*/
int __cdecl veh_cargo(int veh_id) {
    int16_t proto_id = Vehs[veh_id].proto_id;
    uint32_t cargo = VehPrototypes[proto_id].carry_capacity;
    return (cargo && proto_id < MaxVehProtoFactionNum && (get_proto_offense_rating(proto_id) < 0 
        || proto_id == BSC_SPORE_LAUNCHER)) ? Vehs[veh_id].morale + 1 : cargo;
}

/*
Purpose: Determine the extra percent cost for building a prototype. Includes a check if the faction 
         has the free prototype flag set or if the player is using one of the easier difficulties.
// ORIGINAL: 0x005C17D0 ?prototype_factor@@YAHH@Z 0x005C17D0-0x005C184F BYTE_EXACT
// LEVER: the `||` split into two separate guards - the image falls through to an inline `return 0` after the first test where a combined condition jumps to a shared epilogue. 18/48 -> BYTE_EXACT 48/48. Order measured: rule_flags first, `diff_level` first scores 5/48.
// size      127 bytes
// prototype int (__cdecl ?prototype_factor@@YAHH@Z)(int protoID)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Percent extra prototype cost
Status: Complete
*/
int __cdecl prototype_factor(int proto_id) {
    uint32_t faction_id = proto_id / MaxVehProtoFactionNum;
    // TWO GUARDS, NOT ONE `||`. Combined, VC6 short-circuits both tests into a
    // shared `return 0` epilogue and jumps to it; the image falls through to
    // its own inline `xor eax, eax; pop ebp; ret` after the first test, which
    // is what two separate statements produce. 18/48 -> 48/48.
    //
    // The ORDER is the image's too, and measured rather than assumed: testing
    // `diff_level` first scores 5 of 48.
    if (Players[faction_id].rule_flags & RFLAG_FREEPROTO) {
        return 0;
    }
    if (PlayersData[faction_id].diff_level <= DLVL_SPECIALIST) {
        return 0;
    }
    uint8_t triad = get_proto_triad(proto_id);
    switch (triad) {
      case TRIAD_SEA:
        return Rules->extra_pct_cost_proto_sea;
      case TRIAD_AIR:
        return Rules->extra_pct_cost_proto_air;
      case TRIAD_LAND:
      default:
        return Rules->extra_pct_cost_proto_land;
    }
}

/*
Purpose: Calculate the specified prototype's overall cost to build. Optional output parameter
         whether there is an associated 1st time prototype cost (true) or just the base (false).
// ORIGINAL: 0x005C1850 ?veh_cost@@YAHHHPAH@Z 0x005C1850-0x005C1A1D
// LEVER: three real structural fixes moved this 0.545 -> 0.854 similar
// (0/167 in position at the winning `/c /O2 /Gy /GR- /GX`):
//  (1) `prototype_factor(proto_id)` INLINED rather than called - call_diff
//      confirmed the image has no call to 0x005C17D0 here, it open-codes
//      the free-proto/difficulty/triad switch directly;
//  (2) a real out-of-line `base_cost_call` forwarder (veh.h/veh.cpp,
//      `sleep_call`'s own idiom) for the ONE call site where the image
//      keeps `base_cost` (0x005A5D00, "callers 1") as a genuine `call`
//      rather than the inline every other site gets - `base_cost(proto_id)`
//      written directly folded away under /O2, exposing the inner
//      `proto_cost` call call_diff flagged as WRONG CALLEE;
//  (3) `cost`/`proto_cost_first` as plain (signed) `int`, not `uint32_t` -
//      the image's `(cost*3)/4` and the `/100` scaling both carry the
//      `cdq`/sign-correction fixup a signed local earns and an unsigned one
//      does not, even though the values are never negative.
// RULED-OUT: not chased further. The residual gap is the boolean
// materialization in `has_fac_built_call`'s `(mask & field) != 0` - the
// image computes an actual 0/1 via `and/neg/sbb/neg` before testing it
// (0x005C18B8), where this tree's `&&`-chain folds the AND straight into a
// `test`. Same result, different instruction selection; not a control-flow
// difference.
// size      461 bytes
// prototype int (__cdecl ?veh_cost@@YAHHHPAH@Z)(int protoID, int baseID, int* hasProtoCost)
// callers   12   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x005A5D00
Return Value: Mineral cost
Status: Complete
*/
int __cdecl veh_cost(int proto_id, int base_id, BOOL *has_proto_cost) {
    // SIGNED, matching the image's `cdq; and edx,3; add eax,edx; sar eax,2`
    // divide-by-4 fixup below - `VehPrototypes[proto_id].cost` is a uint8_t
    // field, but the local the image computes with is a plain (signed) int.
    int cost = VehPrototypes[proto_id].cost;
    // BUG IN THE ORIGINAL: no `base_id >= 0` guard here, unlike the
    // FAC_SKUNKWORKS check below - `has_fac_built` reads `Bases[base_id]`
    // even when base_id is negative. The image's `test ebx,ebx; jl` guard
    // only appears on the second call site (0x005C19AE); this one
    // (0x005C1890) has none. Reproduced.
    if (proto_id < MaxVehProtoFactionNum
        && (get_proto_offense_rating(proto_id) < 0 || proto_id == BSC_SPORE_LAUNCHER)
        && has_fac_built_call(FAC_BROOD_PIT, base_id)) {
        cost = (cost * 3) / 4; // Decrease the cost of alien units by 25%
    }
    if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION && base_id >= 0) {
        cost = range(cost, 1, 999);
    }
    int proto_cost_first = 0;
    if (proto_id >= MaxVehProtoFactionNum
        && !(VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE)) {
        // `prototype_factor` INLINED, not called - the image has no call to
        // 0x005C17D0 here, it open-codes the free-proto/difficulty check and
        // the triad switch directly (0x005C1919-0x005C1986).
        int faction_id = proto_id / MaxVehProtoFactionNum;
        int factor;
        if (Players[faction_id].rule_flags & RFLAG_FREEPROTO
            || PlayersData[faction_id].diff_level <= DLVL_SPECIALIST) {
            factor = 0;
        } else {
            uint8_t triad = get_proto_triad(proto_id);
            switch (triad) {
              case TRIAD_SEA:
                factor = Rules->extra_pct_cost_proto_sea;
                break;
              case TRIAD_AIR:
                factor = Rules->extra_pct_cost_proto_air;
                break;
              case TRIAD_LAND:
              default:
                factor = Rules->extra_pct_cost_proto_land;
                break;
            }
        }
        // `base_cost` IS CALLED UNCONDITIONALLY here in the image, before the
        // FAC_SKUNKWORKS check - not short-circuited behind it the way a
        // `cond ? 0 : (factor * base_cost(...) + 50) / 100` ternary would.
        proto_cost_first = (factor * base_cost_call(proto_id) + 50) / 100;
        if (base_id >= 0 && has_fac_built_call(FAC_SKUNKWORKS, base_id)) {
            proto_cost_first = 0;
        }
        cost += proto_cost_first;
    }
    if (has_proto_cost) {
        *has_proto_cost = proto_cost_first != 0;
    }
    return cost;
}

/*
Purpose: Check if the land unit inside an air transport can disembark. The transport must be in 
         either a base or an airbase to do so.
// ORIGINAL: 0x005C1C40 ?veh_jail@@YAHH@Z 0x005C1C40-0x005C1D16
// size      214 bytes
// prototype int (__cdecl ?veh_jail@@YAHH@Z)(int vehID)
// callers   8   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: splitting the one big && chain into sequential early-return ifs; hoisting
//   Vehs[veh_id].x/y into locals; mixing split-return with the || form - all measured
//   worse or equal. Committed form already gets 6/63 agreeing, 0.800 similar; remaining
//   divergence is register naming only (ecx/edx swapped on otherwise identical opcodes)
Return Value: Can the specified unit disembark? true/false
Status: Complete
*/
BOOL __cdecl veh_jail(int veh_id) {
    if (get_triad(veh_id) == TRIAD_LAND && Vehs[veh_id].order == ORDER_SENTRY_BOARD
        && Vehs[veh_id].waypoint_x[0] >= 0 && get_triad(Vehs[veh_id].waypoint_x[0]) == TRIAD_AIR
        && (!(bit_at(Vehs[veh_id].x, Vehs[veh_id].y) & BIT_BASE_IN_TILE)
            || owner_at(Vehs[veh_id].x, Vehs[veh_id].y) >= 8
            || owner_at(Vehs[veh_id].x, Vehs[veh_id].y) <= 0)
        && !(bit_at(Vehs[veh_id].x, Vehs[veh_id].y) & BIT_AIRBASE)) {
        return true;
    }
    return false;
}

/*
Purpose: Sets all moves for the specified unit as expended.
// ORIGINAL: 0x005C1D20 ?veh_skip@@YAXH@Z 0x005C1D20-0x005C1D42 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?veh_skip@@YAXH@Z)(int vehID)
// callers   19   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Initialize/reset the fake unit id (2048) used as a placeholder for various UI elements.
// ORIGINAL: 0x005C1D50 ?veh_fake@@YAHHH@Z 0x005C1D50-0x005C1D6F BYTE_EXACT
// size      31 bytes
// prototype int (__cdecl ?veh_fake@@YAHHH@Z)(int protoID, int factionID)
// callers   13   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C02D0
Return Value: Fake unit id (2048)
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Activate the specified unit and clear the current action.
// ORIGINAL: 0x005C1D70 ?veh_wake@@YAHH@Z 0x005C1D70-0x005C1E1F
// size      175 bytes
// prototype int (__cdecl ?veh_wake@@YAHH@Z)(int vehID)
// callers   6   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C9A50 0x005C1540
// LEVER: `order` as int8_t, matching Vehs[].order's OWN (signed) field type -
//        `uint8_t` forces unsigned `jb`/`jae` on the range check below where
//        the image has signed `jl`/`jge`. Moved 0.903 -> 0.964.
Return Value: Unit id (doesn't look to be used on return)
Status: Complete
*/
int __cdecl veh_wake(int veh_id) {
    int8_t order = Vehs[veh_id].order;
    uint32_t state = Vehs[veh_id].state;
    if (order >= ORDER_FARM && order < ORDER_MOVE_TO && !(state & VSTATE_CRAWLING)) {
        // TODO bug fix: Issue with moves_expended size / speed return, see veh_skip()
        Vehs[veh_id].moves_expended = (uint8_t)(speed(veh_id, false) - Rules->move_rate_roads);
        int terra_turns = Vehs[veh_id].terraforming_turns;
        if (terra_turns) {
            terra_turns -= contribution(veh_id, order - 4);
            if (terra_turns < 0) {
                terra_turns = 0;
            }
            Vehs[veh_id].terraforming_turns = (uint8_t)terra_turns;
        }
    }
    if (state & VSTATE_UNK_200 && Vehs[veh_id].order_auto_type == ORDERA_ON_ALERT
        && !(state & VSTATE_UNK_4)) {
        Vehs[veh_id].moves_expended = 0;
    }
    Vehs[veh_id].order = ORDER_NONE;
    Vehs[veh_id].state &= ~(VSTATE_UNK_200 | VSTATE_EXPLORE | VSTATE_UNK_1000000 
        | VSTATE_UNK_2000000 | VSTATE_UNK_8000000); // TODO: Identify VSTATE flags
    return veh_id;
}









/*
Purpose: Send a unit home. Search the friendly and pacted bases, then - for an air unit - the
         friendly carriers and air supply convoys, then the airbases inside the search radius, for
         the closest place this unit could return to. Clear its orders if it is already standing
         there, otherwise point a move-to waypoint at it.
// ORIGINAL: 0x004CBAA0 ?action_home@@YAHHH@Z 0x004CBAA0-0x004CC35F
// LEVER: MORE (18 calls vs image's 17) - `proto_power()` was the extra one,
//        called once at the `chassis_range == 1` site with no equivalent in
//        the image's `calls` list. Same fix as best_defender (0x5044D0):
//        proto_power() moved to veh.h as `MEASURED inline`, since it was a
//        plain non-`inline` function in this tree. Call count now agrees
//        (0 disagree); best flag set's similarity 0.349 -> 0.503 (/O2 /Oi-
//        /GR- /Oy- /GX).
// size      2239 bytes
// prototype int (__cdecl ?action_home@@YAHHH@Z)(int vehID, int)
// callers   8   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3EF0 0x0050DE50 0x005BF1F0 0x005C1540 0x005C1D20 0x00644F3A
Return Value: Was a destination resolved? true/false
Status: Complete
*/
int __cdecl action_home(int veh_id, int flags) {
    Veh &veh = Vehs[veh_id];
    const int faction_id = veh.faction_id;
    const int veh_x = veh.x;
    const int veh_y = veh.y;
    const int veh_region = (int)region_at(veh_x, veh_y);
    const uint32_t proto_id = (uint32_t)veh.proto_id;
    const uint8_t chassis_id = VehPrototypes[proto_id].chassis_id;
    const int chassis_range = Chassis[chassis_id].range;
    const uint8_t triad = Chassis[chassis_id].triad;
    int best_dist = 9999;
    int best_x = -1;
    int best_y = -1;
    // How far the unit could still travel: what is left of this turn, plus - for a chassis with
    // endurance - the sorties it has not spent yet. A range 1 chassis additionally trades its
    // remaining hitpoints for distance. speed() is re-asked at each of the original's call sites
    // rather than hoisted, so the call count is unchanged.
    int moves_left = range((int)speed(veh_id, false) - veh.moves_expended, 0, 999);
    int reach = (int)speed(veh_id, false)
        * (chassis_range - veh.terraforming_turns - 1) + moves_left;
    if (chassis_range == 1) {
        int hp_left = range((int)proto_power(veh_id) - veh.dmg_incurred, 0, 9999);
        reach += (int)speed(veh_id, false)
            * ((hp_left - 1) / VehPrototypes[proto_id].reactor_id);
    }
    const int max_dist = reach / (int)Rules->move_rate_roads;
    // Set by either of the two "the unit is already on alert for this exact waypoint" shortcuts.
    // Both of them skip the remaining searches AND the endurance gate below, so this cannot be
    // collapsed into an ordinary break.
    bool alert_target = false;

    for (int base_id = 0; base_id < BaseCurrentCount; base_id++) {
        Base &base = Bases[base_id];
        const int base_faction = base.faction_id_current;
        if (base_faction != faction_id
            && !(PlayersData[faction_id].diplo_treaties[base_faction] & DTREATY_PACT)) {
            continue; // not ours and not a pact partner's
        }
        if (triad == TRIAD_LAND) {
            if ((int)region_at(base.x, base.y) != veh_region) {
                continue; // no land route
            }
        } else if (triad == TRIAD_SEA) {
            if ((base.x != veh_x || base.y != veh_y) && !base_on_sea(base_id, veh_region)) {
                continue; // not on this body of water
            }
        }
        if (flags == 2 && (base.state & BSTATE_UNK_200000)) {
            continue;
        }
        const int dist = (x_dist(veh_x, base.x) + abs(veh_y - base.y)) >> 1;
        if ((veh.state & VSTATE_UNK_200) && veh.order_auto_type == ORDERA_ON_ALERT
            && base.x == veh.waypoint_x[1] && base.y == veh.waypoint_y[1] && dist <= max_dist) {
            best_dist = dist;
            best_x = base.x;
            best_y = base.y;
            alert_target = true;
            break;
        }
        if (dist < best_dist) {
            best_dist = dist;
            best_y = base.y;
            best_x = base.x;
        }
    }

    if (!alert_target) {
        if (triad == TRIAD_AIR) {
            // An air unit can also come down on a friendly carrier or air supply convoy.
            for (int other_id = 0; other_id < VehCurrentCount; other_id++) {
                Veh &other = Vehs[other_id];
                if (other.faction_id != faction_id) {
                    continue;
                }
                if (other_id == veh_id) {
                    continue;
                }
                if (!has_abil(other.proto_id, ABL_CARRIER)) {
                    const uint8_t other_chassis = VehPrototypes[other.proto_id].chassis_id;
                    if (VehPrototypes[other.proto_id].plan != PLAN_SUPPLY_CONVOY
                        || Chassis[other_chassis].triad != TRIAD_AIR) {
                        continue;
                    }
                }
                int dist = (x_dist(veh_x, other.x) + abs(veh_y - other.y)) >> 1;
                if ((flags & 1) || flags == 4) {
                    if (!is_human(faction_id)) {
                        dist += (int)speed(other_id, false) / (int)Rules->move_rate_roads;
                    }
                    if (VehPrototypes[other.proto_id].plan == PLAN_SUPPLY_CONVOY) {
                        // Recomputed raw, without the bias just added above.
                        const int moves = range((int)speed(veh_id, false)
                                                - veh.moves_expended, 0, 999);
                        const int raw_dist =
                            (x_dist(veh_x, other.x) + abs(veh_y - other.y)) >> 1;
                        if (raw_dist > moves / (int)Rules->move_rate_roads) {
                            continue; // the convoy is not reachable this turn
                        }
                    }
                }
                if (dist < best_dist) {
                    best_dist = dist;
                    best_x = other.x;
                    best_y = other.y;
                }
            }
        }

        // Airbases inside the radius. The radius widens as the best answer so far gets worse, and
        // saturates at the largest ring when nothing has been found at all.
        const int radius_count = RadiusRange[best_dist < 8 ? best_dist : 8];
        for (int i = 0; i < radius_count; i++) {
            const int x_radius = xrange(veh_x + RadiusOffsetX[i]);
            const int y_radius = veh_y + RadiusOffsetY[i];
            if (!on_map(x_radius, y_radius)) {
                continue;
            }
            Map *tile = map_loc(x_radius, y_radius);
            if (!(tile->bit & BIT_AIRBASE)) {
                continue;
            }
            if (tile->bit & BIT_VEH_IN_TILE) {
                const int occupier = tile->val2 & 0xF;
                if (occupier < MaxPlayerNum && occupier != faction_id
                    && !(PlayersData[faction_id].diplo_treaties[occupier] & DTREATY_PACT)) {
                    continue; // someone else is sitting on it
                }
            }
            if (!is_human(faction_id)) {
                // The AI additionally insists the airbase is inside its own territory.
                if (whose_territory(faction_id, x_radius, y_radius, NULL, false) != faction_id) {
                    continue;
                }
                int owner = (int)owner_at(x_radius, y_radius);
                if (owner >= MaxPlayerNum) {
                    owner = -1;
                }
                if (owner != faction_id) {
                    continue;
                }
            }
            const int dist = (x_dist(veh_x, x_radius) + abs(veh_y - y_radius)) >> 1;
            if ((veh.state & VSTATE_UNK_200) && veh.order_auto_type == ORDERA_ON_ALERT
                && !veh.dmg_incurred && x_radius == veh.waypoint_x[1]
                && y_radius == veh.waypoint_y[1] && dist <= max_dist) {
                // best_dist is deliberately left alone here, unlike the base loop's shortcut.
                best_x = x_radius;
                best_y = y_radius;
                alert_target = true;
                break;
            }
            if (dist < best_dist) {
                best_dist = dist;
                best_x = x_radius;
                best_y = y_radius;
            }
        }

        if (!alert_target && ((flags & 1) || flags == 4) && !Chassis[chassis_id].range) {
            return false; // only an endurance chassis answers these queries
        }
    }

    if (!on_map(best_x, best_y)) {
        return false; // nothing found
    }
    if ((flags & 1) && best_dist + 1 <= max_dist - 1) {
        return false; // comfortably within reach, nothing to do
    }
    if (flags == 4) {
        if (best_dist > max_dist) {
            return false;
        }
    } else if (flags < 0) {
        return true; // query only, no state change
    }
    if (best_x == veh_x && best_y == veh_y) {
        veh.order = ORDER_NONE;
        if (!is_human(faction_id)) {
            veh_skip(veh_id);
        }
        return true;
    }
    veh.order = ORDER_MOVE_TO;
    veh.waypoint_x[0] = (int16_t)best_x;
    veh.waypoint_y[0] = (int16_t)best_y;
    if (Chassis[VehPrototypes[veh.proto_id].chassis_id].range && is_human(faction_id)
        && best_dist > max_dist) {
        // Out of reach for a human-controlled endurance unit: drop the order rather than fly it
        // into the ground.
        veh.order = ORDER_NONE;
        // 0xFCFFBDFF clears VSTATE_UNK_200, VSTATE_EXPLORE, VSTATE_UNK_1000000 and
        // VSTATE_UNK_2000000, and nothing else.
        veh.state &= 0xFCFFBDFFU;
    }
    return true;
}

/*
Purpose: Weight the raw odds of an attack by how badly the faction's AI wants that particular
         stack dead. The odds arrive already computed by battle_fight; everything here is a
         strategic multiplier on top of them.
// ORIGINAL: 0x00565F20 ?compute_odds@@YAHHHHHH@Z 0x00565F20-0x005665CC
// RULED-OUT: call structure already matches exactly (`osmx calls --all`'s 15
//            call sites resolve to the same 5 targets as the image's own
//            `calls` list: base_at, at_goal, stack_check x9, has_abil,
//            veh_at x2), so the 123/572 (0.540 best, /O2 /Gy /GR- /Oy- /GX)
//            gap is register/control-flow noise across a 1708-byte function,
//            not a missing inline or wrong callee. Not chased further in
//            this pass - too large for the budget available here.
// size      1708 bytes
// prototype int (__cdecl ?compute_odds@@YAHHHHHH@Z)(int, int factionID, int vehID1, int vehID2, int)
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00579CC0 0x005B9580 0x005BF1F0 0x005BFE90
Return Value: Weighted attack odds
Status: Complete

THE BASIS is what the target is worth divided by what the attacker costs. Out in the field the
defender's own prototype cost counts in full and the rest of its stack counts at ONE FIFTH, so a
stack of ten is worth roughly three of them; an Alien Artifact anywhere in that stack adds half
again. On a base tile the stack is ignored and only the defender's own cost is used, because the
garrison gets its own rule below. Either way the result is divided by the attacker's prototype
cost, which is what makes this a cost-effectiveness number rather than a value.

EVERYTHING AFTER THAT IS A DOUBLING, and they compose:

  x2   the defender's plan is PLAN_NAVAL_TRANSPORT or later, i.e. it cannot fight back
  x3/2 an offensive or combat LAND attacker, on a region this faction has no base plan for
  x2   the attacker's domain matches the tile - a sea unit over water or a land unit over land,
       never an air unit - but only for a base tile or a declared AI_GOAL_ATTACK goal
  x2   a base tile whose garrison is one real combat unit or fewer (total minus the ones with no
       offense rating); mutually exclusive with the domain bonus, which returns first
  x4   the unit carries VFLAG_IS_OBJECTIVE

then one of two branches on the attacker's triad.

AN AIR ATTACKER additionally gets x2 for a missile aimed at open water away from a base; and, if
the stack is TERRAFORMING, x2 for not being a missile, x2 over water when the faction carries
player flag 0x80000, and x2 again for each of the two terraform orders. On a base tile it looks
at the eight neighbours for a friendly stack containing LAND units - something already in place
to take the base - and doubles once for the first one it finds. Then, if the garrison is no
larger than a tenth of the faction's active count of this attacker's prototype plus one, it
scans this faction's sixty-four prototype slots for an active, typed, in-service design with
ABL_DROP_POD, and doubles for it - twice if the attacker is a missile.

ANY OTHER ATTACKER on a base tile counts LAND units across all eight neighbouring friendly
stacks and doubles once per whole garrison's worth of them, stopping at a million.

THE TAIL undoes some of it. An air-superiority attacker aimed at something that is neither
airborne nor armed is worth HALF; a plain PLAN_OFFENSIVE air unit that is not a missile is worth
a THIRD, or two thirds where the region's base plan is 2, because it is expected to be doing
something else.

Bug notes, reproduced rather than corrected:

  - veh_at() answers -1 for a tile with no unit, and both neighbour loops read
    Vehs[veh_id].faction_id from that -1 without checking. The address is fifty-two bytes below
    Vehs[0], four bytes inside Reactor, and it is compared against a live faction id. The read
    is preserved exactly - writing it through the same Vehs pointer lands on the same byte.
  - The division by the attacker's prototype cost is a bare idiv with no zero guard.
  - The two terraform-order tests are two independent ifs in the original, not an else-if. They
    are mutually exclusive in practice because `order` holds one value, so this is a shape
    difference only, and it is transcribed as written.
  - The `while` below terminates only because `defenders` is known nonzero; the guard is the
    original's, not an addition.

The x wrap is course_xrange() rather than xrange(), for the one bit that separates them: the
original reads the low BYTE of MapIsFlat and tests bit zero (`mov cl, byte ptr [94988Ch] /
test cl, 1`), exactly as set_course and reset_territory do.

Verification note: eighteen mutants survive the sweep and every one is an equivalence that can be
read off another recovered body.

SIXTEEN ARE ARGUMENTS stack_check DOES NOT LOOK AT. Its recovered switch takes cond1, cond2 and
cond3 but each arm reads only some of them, and the -1 in an unread slot is therefore free:
type 1, type 16 and type 19 read cond1 alone, so their cond2 and cond3 are dead - two survivors
each at the stack cost, the two thin-garrison censuses, the drop-pod censuses and the defender
count; type 2 and type 3 read cond1 and cond2, so their cond3 is dead - one survivor each at the
artifact test, the terraforming test and the two neighbour counts. The original passes -1 in all
of them and so does this, because the call is the same call.

THE OTHER TWO ARE THE NINTH RADIUSBASE ENTRY. Both neighbour loops stop at eight, and mutating
`i < 8` to `i <= 8` adds entry eight, which is (0,0) - the tile under attack. The stack there is
the defender's, and a tile in this game holds one faction's units, so
Vehs[veh_at(x_def, y_def)].faction_id is the defender's faction and the loop's
`!= faction_id` skips it for any caller that is attacking rather than reinforcing. Every other
tile of the ring is separately covered, including entry 0, which the first sweep found untested.

A NOTE ON THE SWEEP ITSELF, because it cost a pass to work out: the mutation harness perturbs
digits inside trailing comments as well as in code. A `// RadiusBase entry 8 is (0,0) ...` note
on the `for` line produced four unkillable survivors that read exactly like real coverage holes.
The comment is now on its own line and carries no digits.
*/
int __cdecl compute_odds(int odds, int faction_id, int veh_id_atk, int veh_id_def, int base_id) {
    const int x_def = Vehs[veh_id_def].x;
    const int y_def = Vehs[veh_id_def].y;
    const int triad_atk = Chassis[VehPrototypes[Vehs[veh_id_atk].proto_id].chassis_id].triad;
    const int plan_atk = VehPrototypes[Vehs[veh_id_atk].proto_id].plan;
    if (base_id < -1) {
        // Strictly below -1 asks for the lookup; -1 asserts there is no base here.
        base_id = base_at(x_def, y_def);
    }
    const int region_plan =
        PlayersData[faction_id].region_base_plan[map_loc(x_def, y_def)->region];

    int value;
    if (base_id < 0) {
        const int cost_def = VehPrototypes[Vehs[veh_id_def].proto_id].cost;
        // The rest of the stack counts at one fifth; the defender itself counts in full.
        value = ((stack_check(veh_id_def, 7, -1, -1, -1) - cost_def) / 5 + cost_def) * odds;
        if (stack_check(veh_id_def, 2, PLAN_ALIEN_ARTIFACT, -1, -1)) {
            value = value * 3 / 2;
        }
    } else {
        value = VehPrototypes[Vehs[veh_id_def].proto_id].cost * odds;
    }
    value /= VehPrototypes[Vehs[veh_id_atk].proto_id].cost;

    if (VehPrototypes[Vehs[veh_id_def].proto_id].plan >= PLAN_NAVAL_TRANSPORT) {
        value *= 2;   // it cannot fight back
    }
    if (plan_atk <= PLAN_COMBAT && !region_plan && triad_atk == TRIAD_LAND) {
        value = value * 3 / 2;
    }
    if (base_id >= 0 || at_goal(faction_id, AI_GOAL_ATTACK, x_def, y_def)) {
        if ((triad_atk == TRIAD_SEA) == (is_ocean(x_def, y_def) != 0) && triad_atk != TRIAD_AIR) {
            value *= 2;   // the attacker's own domain, and this returns before the next rule
        } else if (base_id >= 0) {
            const int noncombat = stack_check(veh_id_def, 19, -1, -1, -1);
            const int total = stack_check(veh_id_def, 1, -1, -1, -1);
            if (total - noncombat <= 1) {
                value *= 2;   // one real defender or none
            }
        }
    }
    if (Vehs[veh_id_def].flags & VFLAG_IS_OBJECTIVE) {
        value *= 4;
    }

    if (triad_atk == TRIAD_AIR) {
        if (is_proto_missile(Vehs[veh_id_atk].proto_id)
            && is_ocean(x_def, y_def) && base_id < 0) {
            value *= 2;
        }
        if (stack_check(veh_id_def, 2, PLAN_TERRAFORMING, -1, -1)) {
            if (!is_proto_missile(Vehs[veh_id_atk].proto_id)) {
                value *= 2;
            }
            if (is_ocean(x_def, y_def) && (PlayersData[faction_id].flags & 0x80000)) {
                value *= 2;   // unnamed player flag; PlayerFlagsBitfield skips this bit
            }
            // Two separate tests in the original, not an else-if; `order` cannot be both.
            if (Vehs[veh_id_def].order == ORDER_TERRAFORM_UP) {
                value *= 2;
            }
            if (Vehs[veh_id_def].order == ORDER_TERRAFORM_DOWN) {
                value *= 2;
            }
        }
        if (base_id >= 0) {
            for (int i = 0; i < 8; i++) {
                // The ninth RadiusBase entry is the tile itself and is never reached.
                const int x_radius = course_xrange(x_def + RadiusBaseX[i]);
                const int y_radius = y_def + RadiusBaseY[i];
                if (!on_map(x_radius, y_radius)) {
                    continue;
                }
                const int veh_id = veh_at(x_radius, y_radius);
                // Unchecked -1: see the bug note. Preserved deliberately.
                if (Vehs[veh_id].faction_id != faction_id) {
                    continue;
                }
                if (stack_check(veh_id, 3, TRIAD_LAND, -1, -1)) {
                    value *= 2;   // somebody is already on the ground next to it
                    break;
                }
            }
            const int noncombat = stack_check(veh_id_def, 19, -1, -1, -1);
            const int total = stack_check(veh_id_def, 1, -1, -1, -1);
            const int proto_atk = Vehs[veh_id_atk].proto_id;
            if (total - noncombat
                <= PlayersData[faction_id].proto_id_active[proto_atk] / 10 + 1) {
                for (int i = 0; i < MaxVehProtoFactionNum; i++) {
                    const int proto_id = faction_id * MaxVehProtoFactionNum + i;
                    if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)) {
                        continue;
                    }
                    if (!(VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE)) {
                        continue;
                    }
                    if (!PlayersData[faction_id].proto_id_active[proto_id]) {
                        continue;
                    }
                    if (!has_abil(proto_id, ABL_DROP_POD)) {
                        continue;
                    }
                    value *= 2;
                    if (is_proto_missile(Vehs[veh_id_atk].proto_id)) {
                        value *= 2;
                    }
                    break;
                }
            }
        }
    } else if (base_id >= 0) {
        int attackers = 0;
        for (int i = 0; i < 8; i++) {
            const int x_radius = course_xrange(x_def + RadiusBaseX[i]);
            const int y_radius = y_def + RadiusBaseY[i];
            if (!on_map(x_radius, y_radius)) {
                continue;
            }
            const int veh_id = veh_at(x_radius, y_radius);
            // Unchecked -1 here too.
            if (Vehs[veh_id].faction_id != faction_id) {
                continue;
            }
            attackers += stack_check(veh_id, 3, TRIAD_LAND, -1, -1);
        }
        const int defenders = stack_check(veh_id_def, 16, -1, -1, -1);
        if (defenders) {   // without this the subtraction below never terminates
            while (attackers > defenders && value < 1000000) {
                value *= 2;
                attackers -= defenders;
            }
        }
    }

    if (plan_atk == PLAN_AIR_SUPERIORITY) {
        const int proto_def = Vehs[veh_id_def].proto_id;
        if (Chassis[VehPrototypes[proto_def].chassis_id].triad == TRIAD_AIR) {
            return value;   // an interceptor doing its job
        }
        if (!Weapon[VehPrototypes[proto_def].weapon_id].offense_rating) {
            return value;   // nothing there to intercept anything
        }
        return value / 2;
    }
    if (triad_atk == TRIAD_AIR && plan_atk == PLAN_OFFENSIVE
        && !is_proto_missile(Vehs[veh_id_atk].proto_id)) {
        if (region_plan == 2) {
            return value * 2 / 3;
        }
        return value / 3;
    }
    return value;
}

/*
Purpose: Choose which base a native life form standing on the specified tile should head for.
// ORIGINAL: 0x005665D0 ?alien_base@@YAHHHH@Z 0x005665D0-0x00566802
// size      562 bytes
// prototype int (__cdecl ?alien_base@@YAHHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050DE50 0x005B9580 0x005BFE90 0x00644F3A
Return Value: Base id of the most attractive target, or -1 when there is none
Status: Complete

WHAT THE SCORE MEANS. The comparison variable is a cost, not a merit: the loop keeps the LOWEST
value, seeded at 9999, and the winner is published in BaseFindDist for the caller. The cost is the
radiating distance from the unit to the base, scaled by 32 and then divided by that base's own
mineral plus energy intake plus 32, so a productive base pulls a worm from further away. Three
adjustments follow, each of which the original applies as a whole doubling or halving:

  - an Alien Artifact standing in the base halves the cost. stack_check type 2 counts units in the
    tile's stack whose prototype PLAN is PLAN_ALIEN_ARTIFACT, so this is literally "something is
    sitting there that Planet wants back".
  - the unit's own home base halves it again. Native life gets a home base when a faction breeds
    it, and this is what makes a bred worm drift back toward the base that made it.
  - a base whose owner runs a positive PLANET rating and has no eco damage DOUBLES it. That is the
    in-game promise that a green faction is left alone, expressed as a cost the worm has to pay.

Verification note: the third test compares Veh+0x26 against the base's owner. veh.h names that
byte order_auto_type, and on this path the original is reading it as a faction id rather than as
an order type - base.cpp reads the same byte as a resource id for convoys, so the field is already
known to be multi-purpose. Transcribed as the original reads it rather than corrected.

REACHABILITY. The region test only applies to a native standing on LAND. Water bodies are numbered
from MaxRegionLandNum upward (see the region notes in map.h), so a unit already at sea reaches
every base and the test is skipped - which is the whole reason it is written as a bound and not as
an equality. On land, the base has to share the unit's region, except for a Sealurk - prototype
14, the only native the loop names - which may also take a base that merely touches the region it
is standing in, and base_on_sea answers that.

Verification note: stack_check's fifth argument survives a mutation sweep and always will. Type 2
reads cond1 and cond2 and never looks at cond3, so the -1 the original pushes for it cannot be
observed by any fixture. The fourth argument is observable and is covered - the artifact unit in
the fixture carries a non-zero faction precisely so that turning that -1 into 0 changes the count.

The original inlines region_at (0x00500220), x_dist (0x00579790) and the two-argument vector_dist
(0x004F8090); each inlined copy is instruction-for-instruction the standalone body, so they are
called here. One consequence is visible in the disassembly and worth recording: the inlined
map_loc halves x with SAR, while the exported map_loc that region_at reaches takes uint32_t and
would halve it with SHR. The two agree on every non-negative x, and alien_move at 0x005668A0 -
the only caller - passes a unit's own sign-extended coordinates.

// LEVER: FEWER (6 calls vs image's 7) - the image calls `abs()` FOUR times
//        here and never calls vector_dist(int,int) itself (0x4F8090 is not
//        in its `calls` list, despite being a real, `callers 5`-elsewhere
//        function). Open-coded the `vector_dist(x_dist(...), abs(...))`
//        call as the abs()/largest/smallest expansion vector_dist's own
//        body performs - the same pattern base.cpp's black_market and
//        base_find already use for the 4-arg form. Call count now agrees
//        (0 disagree); best flag set's similarity 0.782 -> 0.883
//        (/O2 /Oi- /GR- /Oy- /GX).
*/
int __cdecl alien_base(int veh_id, int x, int y) {
    int best_value = 9999;
    BaseFindDist = 9999;
    int base_id_best = -1;
    const int region = region_at(x, y);
    for (int base_id = 0; base_id < BaseCurrentCount; base_id++) {
        Base &base = Bases[base_id];
        if (Vehs[veh_id].proto_id == BSC_SEALURK) {
            if (region < MaxRegionLandNum && region != (int)region_at(base.x, base.y)
                && !base_on_sea(base_id, region)) {
                continue;
            }
        } else if (region < MaxRegionLandNum && region != (int)region_at(base.x, base.y)) {
            continue;
        }
        // Open-coded vector_dist(x_dist(...), abs(...)): the image calls
        // abs() four times, never vector_dist itself - the same expansion
        // as black_market (base.cpp, 0x4E4D00-ish) and base_find.
        int dx = x_dist(x, base.x);
        int dy = abs(y - base.y);
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
        int value = (largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1)) * 32
            / (base.mineral_intake_2 + base.energy_intake_2 + 32);
        if (stack_check(veh_at(base.x, base.y), 2, PLAN_ALIEN_ARTIFACT, -1, -1)) {
            value /= 2;
        }
        if (Vehs[veh_id].home_base_id == base_id) {
            value /= 2;
        }
        if (Vehs[veh_id].order_auto_type != base.faction_id_current
            && PlayersData[base.faction_id_current].soc_effect_active.planet > 0
            && !base.eco_damage) {
            value *= 2;
        }
        if (value <= best_value) {
            best_value = value;
            base_id_best = base_id;
        }
    }
    if (base_id_best < 0) {
        return base_id_best;
    }
    BaseFindDist = best_value;
    return base_id_best;
}
