/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "general.h"
#include "veh.h"

/*
Purpose: Check if the prototype can perform artillery combat. The 2nd parameter determines how sea
         units are treated.
// ORIGINAL: 0x005C0DB0 ?can_arty@@YAHHH@Z 0x005C0DB0-0x005C0E35 BYTE_EXACT
// TRIED 2026-09-03: the one remaining divergence is NOT the `mov bl/test bl`
// wall the first TRIED line below describes - that is gone (both sides now
// emit `mov bl, [edx+..]; test bl,bl`), and the Armor addressing of the
// second line went with it. What is left is ONE JUMP TARGET: the image's
// `dec eax / je 0x5c0dda` for case TRIAD_AIR against this tree's
// `je 0x5c0df9` - the image shares the FIRST `return false` epilogue, this
// tree the SECOND. Spelling the switch's `return false` as `return 0`,
// putting TRIAD_AIR before TRIAD_SEA in the case list, and adding an
// explicit `default: break;` each measured 53/54 identically. Which of two
// identical epilogues VC6's return-sharing picks is not reachable from the
// case order or from the constant's type; every other byte agrees.
// TRIED: the OR'd (offense<=0 || defense<0) guard into two sequential `if`s, one per term - matches the image's per-term SPORE_LAUNCHER exception check. Also: the TRIAD_SEA/TRIAD_AIR dispatch is a `switch`, not chained `if`s - the image's dec/je chain is switch codegen. 19/54 -> 50/54. One divergence remains: the defense_rating byte load folds into `cmp byte ptr [edx], 0` here where the image keeps `mov bl, ...; test bl, bl` - tried swapping check order, `0 > x`, a BOOL local, and `!(x >= 0)`; none moved it. Plateau.
// TRIED: same `Armor[armor_id].defense_rating` addressing defect as arm_strat (0x0057D270): the image computes `shl edx,4` then loads with a displaced `[edx+0x94F280]`, this tree's `get_proto_defense_rating` inline always compiles `add edx,0x94F28` (pre-adding the displaced/16 constant) then `shl edx,4` then a base-only load. Raw byte-array indexing forms tried at arm_strat produced the identical bytes there too, so not re-tried per-callsite here. Cross-function confirmation that this is a compiler addressing-mode choice, not something source-shape-local to either caller.
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
    if (get_proto_offense_rating(proto_id) > 0 || proto_id == BSC_SPORE_LAUNCHER) {
        if (get_proto_defense_rating(proto_id) >= 0 || proto_id == BSC_SPORE_LAUNCHER) {
            uint8_t triad = get_proto_triad(proto_id);
            switch (triad) {
              case TRIAD_SEA:
                return sea_triad_retn;
              case TRIAD_AIR:
                return false;
            }
            return has_abil(proto_id, ABL_ARTILLERY);
        }
        return false;
    }
    return false;
}

/*
Purpose: Calculate the cargo capacity of a unit. It seems Spore Launchers were considered to have
         cargo capacity at one time.
// ORIGINAL: 0x005C1760 ?veh_cargo@@YAHH@Z 0x005C1760-0x005C17C4
// TRIED: still MISMATCH (0.853 similar) - the image computes `proto_id*sizeof(VehPrototype)` ONCE and reuses it for both the `carry_capacity` and `weapon_id` field reads (same array element, two fields); this tree recomputes it for the second read under every flag set tried. A local `VehPrototype *proto = &VehPrototypes[proto_id]` used for both field reads did not change the codegen. Also see the `add`-before-`shl` base-folding note on arm_strat (0x0057D270) - same class of divergence on the `carry_capacity` read itself.
// TRIED: proto_id` (not `uint32_t`/`int`) matches the image, which keeps the 16-bit `proto_id` in `si` for the `< MaxVehProtoFactionNum` and `== BSC_SPORE_LAUNCHER` compares and only does `movsx eax,si` at the array-indexing use sites.
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
