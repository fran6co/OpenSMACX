/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// Vehicle record functions, 0x005BFE90-0x005C1E1F in the image (veh_at
// through veh_wake), grouped by address order: kill inlines veh_cargo here,
// while the stack functions (veh_stack.cpp) call it out of line.
#include "stdafx.h"
#include "general.h"
#include "veh.h"
#include "map.h"
#include "game.h"
#include "log.h"
#include "faction.h"
#include "base.h"
#include "alpha.h"
#include "technology.h"
#include "crt_random_inline.h"
#include "deletionlist.h"
#include "netdaemon.h"
#include "tutwin.h"
#include "console.h"
#include "statuswin.h"
#include "terraforming.h"


/*
Purpose: Check the coordinates for units and if at least one is found return the top most id.
// ORIGINAL: 0x005BFE90 ?veh_at@@YAHHH@Z 0x005BFE90-0x005BFF9E
// TRIED: 0.937 similar under every flag set tried - the control flow and
//        field reads already match; what's left is register allocation (the
//        image hoists `MapLatitudeBounds`/`MapLongitudeBounds` into ebx/edi
//        ONCE and reuses them for both `on_map()` call sites, this tree
//        re-reads them at the second site) plus esi/edi swaps throughout.
//        Left as-is per "do not chase register-allocation differences."
// LEVER 2026-09-03: hoisting the two bounds into locals before the FIRST
//        on_map() test is what earns the image's single load - a dllexport
//        inline re-reads its globals at every expansion, so the two sites
//        were two pairs of loads. Both on_map() calls are now written out
//        (the image inlines it at both) against those locals: 0.937 -> 0.938
//        at /Oy-, 0.942 at /GX, agreeing 5/96 -> 44/96.
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
    const int lat_bounds = MapLatitudeBounds;
    if (y >= 0 && y < lat_bounds && x >= 0 && x < MapLongitudeBounds && !(bit_at(x, y) & BIT_VEH_IN_TILE)) {
        return -1; // not found
    }
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        if (Vehs[veh_id].x == x && Vehs[veh_id].y == y) {
            return veh_top(veh_id);
        }
    }
    if (y < 0 || y >= lat_bounds || x < 0 || x >= MapLongitudeBounds) {
        return -1;
    }
    if (!VehBitError) {
        log_say("Vehicle Bit Error  (x, y)", x, y, 0);
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR || GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE || IsMultiplayerNet) {
        if (VehBitError) {
            return -1;
        }
        VehBitError = true;
    }
    rebuild_vehicle_bits();
    return -1;}


/*
Purpose: Temporarily remove the specified unit from its current square and stack in preparation for
         another action such as interacting with the stack, moving or killing it.
// ORIGINAL: 0x005BFFA0 ?veh_lift@@YAXH@Z 0x005BFFA0-0x005C007F BYTE_EXACT
// TRIED: swapping the x/y local declaration order - the remaining gap is which register (eax/ecx/edx) the allocator picks for x, y and the -1 constant at the join point; both orders produced the identical register assignment, so it isn't source-order controlled here. Plateau at 49/60.
// TRIED 2026-09-03: NO x/y locals at all - `VehLiftX = Vehs[veh_id].x;` after the four -1 stores. The loads then sit AFTER the stores (VC6 will not hoist a load above a store to the same field), where the image hoists them BEFORE the stores, which needs the locals. 54/60 - better than 49/60, kept; the remaining 6 are the two movsx loads plus which register holds the -1 (ours edx, image eax).
// TRIED 2026-09-03: `VehDropLiftVehID = veh_id;` hoisted ABOVE the x/y reads (so VC6 sees the global store early) - regressed to 49/60, matching the original note's finding that the image assigns it between reading y and zeroing the fields.
// TRIED: were cached into locals BEFORE the prev/next stack-pointer fixup and the on_map/bit_set branch; the image re-reads `Vehs[veh_id].x`/`.y` fresh at each site (inside the branch, and again at the join point for VehLiftX/Y) instead of hoisting one read to the top. Also `VehDropLiftVehID = veh_id;` moved to AFTER the x/y reads (image assigns it between reading y and zeroing the fields, not before). 4/60 -> 49/60 (0.829 -> 0.983 similar).
// Faithful order: after the optional bit_set call, native 005C003A/0041
// captures the then-current signed coordinates; it stores VehDropLiftVehID
// at 005C004B, clears the record at 005C0051..0066, then publishes the saved
// coordinates to VehLiftX/Y at 005C0070/0076. The old matching candidate read
// those fields only after clearing them and therefore published -1/-1.
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
    VehLiftX = Vehs[veh_id].x;
    VehLiftY = Vehs[veh_id].y;
    VehDropLiftVehID = veh_id;
    Vehs[veh_id].x = -1;
    Vehs[veh_id].y = -1;
    Vehs[veh_id].next_veh_id_stack = -1;
    Vehs[veh_id].prev_veh_id_stack = -1;
    return veh_id;
}


/*
Purpose: Move the specified unit to the provided coordinates.
// ORIGINAL: 0x005C0080 ?veh_drop@@YAXHHH@Z 0x005C0080-0x005C019C BYTE_EXACT
// TRIED: the last `mov ecx,[ebp+8]; mov eax,ecx` before the final `ret` - the image loads straight into eax there (ecx is dead, clobbered by the `bit_set` call just before), but this tree's register allocator prefers ecx as veh_id's home register for the whole function and copies to eax at the return. No source reshuffle around the final `return veh_id;` moved it; plateau.
// TRIED: (veh_id_dest < 0) {...} else {...}` had the arms backwards - the image falls through the `>= 0` write first and jumps to the `< 0` block, so the source needs `if (veh_id_dest >= 0) {...} else {...}` to get that polarity. Also the flags ternary `(A && B) ? X|Y : Y` compiled an extra `mov` before the compare the image folds into a direct `cmp byte ptr [...], 2`; splitting it into `flags = 0/BIT_SUPPLY_REMOVE; flags |= BIT_VEH_IN_TILE;` (the image ORs BIT_VEH_IN_TILE in unconditionally at a shared tail) dropped that mov. 39/95 -> 80/95 (0.880 -> 0.973 similar).
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
        return veh_id;
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
void __cdecl sleep(int veh_id) {
    Vehs[veh_id].order = ORDER_SENTRY_BOARD;
    Vehs[veh_id].waypoint_x[0] = -1;
    Vehs[veh_id].waypoint_y[0] = 0;
}


/*
Purpose: Move the specified unit to the bottom of the stack.
// ORIGINAL: 0x005C01D0 ?veh_demote@@YAXH@Z 0x005C01D0-0x005C0256 BYTE_EXACT
// TRIED: byte-exactness. Best is 0.916 similar (4/40 in position) at
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
    int last_veh_id;
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
// ORIGINAL: 0x005C0260 ?veh_promote@@YAXH@Z 0x005C0260-0x005C02CE BYTE_EXACT
// TRIED: a `veh_id_top >= 0` guard after the hand-inlined veh_top - the image has none (see BUG note below). TRIED: seeding `veh_id_top = veh_id` before vs. inside the `if` - same codegen either way. The remaining gap is the compiler proving the `prev_veh_id_stack < 0` (zero-iteration) case makes `veh_id_top != veh_id` tautologically false and skipping straight to the epilogue, where the image still materializes and compares it; not reachable by reordering statements. Plateau at 3/40.
// TRIED: CALLEE - the tree called `veh_top()`/`veh_put()`, both inline-marked wrappers, but this call site is not one of veh_top's two real sites and veh_put did not inline here either. Hand- inlining both (veh_top's loop, and veh_put's `veh_lift(id); veh_drop(id, x, y)` - dropping veh_lift's unused return since it always returns its own argument) took call_diff from WRONG CALLEE to 0 disagreeing and 0.657 -> 0.773 similar.
// size      110 bytes
// prototype
// callers   6   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFFA0 0x005C0080
Return Value: n/a
Status: Complete
*/
// stack_fix calls this out of line.
#pragma auto_inline(off)
void __cdecl veh_promote(int veh_id) {
    // The image expands veh_top's signed-word climb here.
    int veh_id_top = veh_top(veh_id);
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
#pragma auto_inline(on)


/*
Purpose: Clear the specified unit.
// ORIGINAL: 0x005C02D0 ?veh_clear@@YAXHHH@Z 0x005C02D0-0x005C03C3 BYTE_EXACT
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
    Vehs[veh_id].facing = 2;
    Vehs[veh_id].probe_action = 0;
    Vehs[veh_id].probe_sabotage_id = 0;
}



// The superseded raw-address scaffold was deleted after full owner recovery.
// Its seed path below is historical Git provenance, not a source dependency.
// ORIGINAL: 0x005C03D0 ?veh_init@@YAHHHHH@Z 0x005C03D0-0x005C08C0
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005c03d0.cpp; recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_base_colony_production.py", "Pinned01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf full389instructions5C03D0..5C08C0.", "Native5C04C6/5C04D0 invalidate BaseComputedID, not BaseIDCurrentSelected;5C05D6 scores the current faction region count, not the rival count.", "Native5C06E8 uses signed minimum after wrapped threshold doubling/division;5C07B0 immediately returns when the probe-allocation bit is already set."], "reason": "Recover complete typed Vehicle creation, home-base assignment and target-region allocation, replacing only its executable-address forwarder. Preserve original seed and historical obligations; no new byte, linked-preservation or runtime acceptance claim."}
// symbol    ?veh_init@@YAHHHHH@Z
// TRIED: direct Ghidra transcription with own extern DAT_ table arrays (byte/dword typed per bracket-index width) instead of the given g_ address pointers; diverges at insn #5, plausibly register-allocation shape of the ~200-line body
int __cdecl veh_init(int prototype, int faction, int x, int y) {
    if (GameDataStateStorage.fields_.vehicle_current_count_ >= 0x800) {
        return -1;
    }
    const int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_++;
    log_say("Adding Vehicle (v,who,v2)", vehicle_id, faction, prototype);
    if (VehPrototypes[prototype].plan < PLAN_COLONIZATION) {
        PlayersData[faction].total_mil_veh++;
    }
    PlayersData[faction].proto_id_active[prototype]++;
    veh_clear(vehicle_id, prototype, faction);
    const int base_id = base_find(x, y);
    if (VehPrototypes[prototype].plan == PLAN_ALIEN_ARTIFACT) {
        if (GameDataStateStorage.fields_.game_state_ & 0x40000) {
            Vehs[vehicle_id].flags |= VFLAG_IS_OBJECTIVE;
        }
    } else if (base_id >= 0 && Bases[base_id].faction_id_current == faction) {
        Vehs[vehicle_id].home_base_id = static_cast<int16_t>(base_id);
        if (BaseComputedID == base_id) {
            BaseComputedID = -1;
        }
    }
    veh_drop(vehicle_id, x, y);
    spot_all(vehicle_id, 1);

    // AI land units: decide whether to dedicate the unit to the faction's
    // target region.
    if (PlayersData[faction].x_target < 0) {
        return vehicle_id;
    }
    Map *tile = map_loc(x, y);
    if ((tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
        return vehicle_id;
    }
    const uint8_t plan = VehPrototypes[Vehs[vehicle_id].proto_id].plan;
    if (plan > 2 && plan != PLAN_INFO_WARFARE) {
        return vehicle_id;
    }
    int region = tile->region;
    if (map_loc(PlayersData[faction].x_target, PlayersData[faction].y_target)->region != region) {
        return vehicle_id;
    }
    int enemy_faction = -1;
    int enemy_bases = 0;
    for (int i = 1; i < MaxPlayerNum; i++) {
        // The original scores this faction's own region count, not the rival's.
        if (i != faction && PlayersData[faction].diplo_treaties[i] & DTREATY_VENDETTA
            && PlayersData[faction].region_total_bases[region] > enemy_bases) {
            enemy_bases = PlayersData[faction].region_total_bases[region];
            enemy_faction = i;
        }
    }
    if (enemy_faction >= 0) {
        base_find(x, y, enemy_faction, region, -1, -1);
        if (vector_dist(x, y, PlayersData[faction].x_target, PlayersData[faction].y_target)
            > BaseFindDist) {
            return vehicle_id;
        }
    }
    int limit = range(PlayersData[faction].region_total_bases[region] * 2 / 3, 3, 8);
    if (PlayersData[faction].region_base_plan[region] == 7) {
        const int sea_limit = PlayersData[faction].unk_70 * 2 / 0x400000;
        if (limit >= sea_limit) {
            limit = sea_limit;
        }
    }
    if (PlayersData[faction].unk_71 - bit_count(PlayersData[faction].unk_72) >= limit) {
        return vehicle_id;
    }
    const int weapon_value =
        weap_strat(VehPrototypes[Vehs[vehicle_id].proto_id].weapon_id, faction);
    const int armor_value =
        arm_strat(VehPrototypes[Vehs[vehicle_id].proto_id].armor_id, faction);
    if (Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].triad != TRIAD_LAND || has_abil(Vehs[vehicle_id].proto_id, 0x10)) {
        return vehicle_id;
    }
    if (VehPrototypes[Vehs[vehicle_id].proto_id].plan == PLAN_INFO_WARFARE) {
        if (!(PlayersData[faction].unk_72 & 2)) {
            Vehs[vehicle_id].state |= VSTATE_UNK_40000;
            PlayersData[faction].unk_71++;
            PlayersData[faction].unk_72 |= 2;
        }
        return vehicle_id;
    }
    if (weapon_value < armor_value) {
        if (PlayersData[faction].unk_72 & 1
            || PlayersData[faction].region_base_plan[region] == 7) {
            return vehicle_id;
        }
    } else if (PlayersData[faction].region_base_plan[region] != 7
               && !bounded_crt_random(
                   range(PlayersData[faction].region_total_bases[region] * 2 / 3, 3, 8) + 2)) {
        Vehs[vehicle_id].state |= VSTATE_UNK_20000;
        return vehicle_id;
    }
    Vehs[vehicle_id].state |= VSTATE_UNK_40000;
    PlayersData[faction].unk_71++;
    if (weapon_value < armor_value) {
        PlayersData[faction].unk_72 |= 1;
    }
    return vehicle_id;
}


// ===== veh.cpp, P2 wave 2: the vehicle removal / query verbs =====

// ---- the .data slots veh_kill compacts when a vehicle id disappears ----
// 0x008C6B2C is the vehicle id the map cursor is hovering: MainInterface::
// on_iface_mouse_move (0x0045D000) writes it and clears the base id beside it
// before handing the pair to StatusWin::draw_active. The active vehicle at
// 0x008C6B34 is ConsoleStatusWin.field_15CC_: native reset_active4B6544
// reads receiver+15CC, and veh_kill5C0A71 updates that same source-owned word.

// Original0x0093E31C is NetDaemonNet.deletion_list_, not a separate global.
// veh_kill passes this exact embedded receiver to add_deletion52DE70.

// 0x0093E908. A dword * holding the address of one of the vehicle-id slots
// above: the interface keeps it pointing at whichever selection it wants to
// follow, and veh_kill walks the dereference so a deletion the pointer names is
// re-pointed rather than left dangling. It skips three slots by identity -
// CursorVehIndex, FocusVehId and VehSelectionFollowLimit - which is how the
// image says "the ones already handled above".
// The borrowed selection pointer is NetDaemonNet's channel1 payload, not a
// second global overlapping that daemon-owned word.

// 0x009A64DC. Another vehicle-id slot, and one with its own extra rule: it is
// also decremented when it EQUALS the deleted id, which the three above do not
// do. action_airdrop stores/reloads it across capture_base; veh_kill updates
// the same signed slot, and synch transports its four-byte representation.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.


// 0x00691DC0, the log_say title. The image logs every vehicle removal with the
// vehicle id, its owner and its prototype as the three substitutions.
// RECOVERY_OBJECT: v2 0x00691DC0 0x00691DDB ?VehLogKillingVehicle@@3PADA 0x005C08C0
char VehLogKillingVehicle[] = "Killing vehicle (v,who,v2)";

// 0x009B227C..0x009B2280 is veh_find's four-byte distance output. Its pinned
// initial bytes are zero; the function resets it to 9999 at 0x005C0CBF and
// stores the winning distance at 0x005C0D9E only when a vehicle qualifies. The
// pinned reference scan identifies those two writes, while indirect readers,
// aliases and the full allocation / lifetime remain unproved. Neighboring
// storage is not part of this binding.
// RECOVERY_OBJECT: v2 0x009B227C 0x009B2280 ?VehFindBestDist@@3HA 0x005C0CB0
int VehFindBestDist = 0;

// PlayersData owns both faction counters: total_mil_veh is the dword at +0x1058,
// and proto_id_active is the byte row at +0x0858. veh_kill's native index
// scales faction*2099 by four before addressing that row, yielding the 0x20CC
// PlayerData stride.

/*
Purpose: Remove a vehicle from the game entirely.
// ORIGINAL: 0x005C08C0 ?veh_kill@@YAXH@Z 0x005C08C0-0x005C0AF4 BYTE_EXACT
// size      564 bytes
// prototype void (__cdecl ?veh_kill@@YAXH@Z)(int vehID)
// callers   15   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A7A0 0x0041A7E0 0x0052DE70 0x005BFFA0 0x006262F0 0x00645930
// HOMED from src/recovered/units/005c08c0.cpp, 2026-09-04, which carried 14
// anonymous address bindings; every one is named or modelled above, and the
// pending_bodies.cpp forwarder of the same name is gone (this definition
// replaces it). stack_kill reads Vehs[].next_veh_id_stack at 0x952858 and
// prev_veh_id_stack at 0x95285A. In veh_kill's compaction loop, eax starts at
// Vehs + 0x30 and advances 0x34; `word [eax]`/`word [eax+2]` are those same
// fields. In that same veh_kill loop, `byte [eax - 0x1f]` and `word
// [eax - 0x1c]` are the previous record's order and waypoint_x[0].
Return Value: n/a
Status: MISMATCH #15 - the `xor edx,edx` zero-extending faction_id for log_say
        lands one slot away from the image's; every call, table stride and
        indirect target matches (0.79 past it).
*/
// The image calls memcpy out of line here.
#pragma function(memcpy)
void __cdecl veh_kill(int veh_id) {
    if (veh_id < 0) {
        return;
    }

    if (IsMultiplayerNet != 0) {
        NetDaemonNet.deletion_list_.add_deletion(veh_id);
    }

    log_say(VehLogKillingVehicle, veh_id, Vehs[veh_id].faction_id,
            Vehs[veh_id].proto_id);

    const int faction_id = Vehs[veh_id].faction_id;
    const int x = Vehs[veh_id].x;
    const int y = Vehs[veh_id].y;
    const int home_base_id = Vehs[veh_id].home_base_id;
    const int proto_id = Vehs[veh_id].proto_id;

    if (VehPrototypes[proto_id].plan <= 7) {
        --PlayersData[faction_id].total_mil_veh;
    }
    --PlayersData[faction_id].proto_id_active[proto_id];

    veh_lift(veh_id);

    if (veh_id < GameDataStateStorage.fields_.vehicle_current_count_ - 1) {
        memcpy(&Vehs[veh_id], &Vehs[veh_id + 1],
               (GameDataStateStorage.fields_.vehicle_current_count_ - veh_id - 1) * sizeof(Veh));
    }
    if (veh_id < 0x800) {
        --GameDataStateStorage.fields_.vehicle_current_count_;
    }
    for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
        if (Vehs[i].prev_veh_id_stack > veh_id) {
            --Vehs[i].prev_veh_id_stack;
        }
        if (Vehs[i].next_veh_id_stack > veh_id) {
            --Vehs[i].next_veh_id_stack;
        }
        if (Vehs[i].order == 1) {
            const short transport = Vehs[i].waypoint_x[0];
            if (transport == veh_id) {
                Vehs[i].order = 0;
            } else if (transport > veh_id) {
                Vehs[i].waypoint_x[0] = transport - 1;
            }
        }
    }

    if (veh_id == ConsoleGlobal.selected_vehicle_) {
        ConsoleGlobal.selected_vehicle_ = -1;
    } else if (ConsoleGlobal.selected_vehicle_ > veh_id) {
        --ConsoleGlobal.selected_vehicle_;
    }
    if (veh_id == ConsoleGlobal.focus_vehicle_) {
        ConsoleGlobal.focus_vehicle_ = -1;
    } else if (ConsoleGlobal.focus_vehicle_ > veh_id) {
        --ConsoleGlobal.focus_vehicle_;
    }
    if (veh_id == ConsoleStatusWin.hover_vehicle_id_) {
        ConsoleStatusWin.hover_vehicle_id_ = -1;
    } else if (ConsoleStatusWin.hover_vehicle_id_ > veh_id) {
        --ConsoleStatusWin.hover_vehicle_id_;
    }
    if (veh_id == ConsoleStatusWin.field_15CC_) {
        ConsoleStatusWin.field_15CC_ = -1;
    } else if (ConsoleStatusWin.field_15CC_ > veh_id) {
        --ConsoleStatusWin.field_15CC_;
    }
    if (GameDataStateStorage.fields_.vehicle_selection_follow_limit_ != 0 && GameDataStateStorage.fields_.vehicle_selection_follow_limit_ >= veh_id) {
        --GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
    }
    if (IsMultiplayerNet != 0) {
        int *slot = NetDaemonNet.vehicle_selection_follow();
        if (slot != 0 && slot != &ConsoleGlobal.selected_vehicle_ && slot != &GameDataStateStorage.fields_.vehicle_selection_follow_limit_ &&
            slot != &ConsoleGlobal.focus_vehicle_) {
            if (*slot >= veh_id) {
                --*slot;
            }
        }
    }

    TutWinBaseWindow.check_loc(x, y, 0);
    TutWinBaseWindow.check_base(home_base_id);
}
#pragma intrinsic(memcpy)


/*
Purpose: Kill a vehicle, or strip it down to a transport's cargo first.
// ORIGINAL: 0x005C0B00 ?kill@@YAXH@Z 0x005C0B00-0x005C0CB0 BYTE_EXACT
// size      432 bytes
// prototype void (__cdecl ?kill@@YAXH@Z)(int vehID)
// callers   25   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A7A0 0x0046AEF0 0x0046AF40 0x005B8EE0 0x005B9510 0x005B9580 0x005BF1F0 0x005BFE90 0x005C08C0
// HOMED from src/recovered/units/005c0b00.cpp, 2026-09-04. Its `g_tbl1` /
// `g_tbl2` / `g_tbl3` scaffolding is Vehs[] / VehPrototypes[] / Weapon[]:
// `base2[0]` is VehPrototypes[p].weapon_id (0x9AB88D = the array front +
// 0x25) and `base2[3]` is VehPrototypes[p].carry_capacity (+0x28).
Return Value: n/a
Status: Complete
*/
void __cdecl kill(int veh_id) {
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    int faction_id = Vehs[veh_id].faction_id;
    BOOL worm = has_abil(Vehs[veh_id].proto_id, 2);
    int home_base_id = Vehs[veh_id].home_base_id;
    if (veh_cargo(veh_id)) {
        BOOL ocean;
        if (on_map(x, y)) {
            ocean = is_ocean(x, y);
        } else {
            if (x == -2) {
                stack_kill(veh_id);
                return;
            }
            ocean = TRUE;
        }
        if (ocean) {
            stack_veh(veh_id, 2);
            stack_kill(veh_id);
            goto killed;
        }
    }
    veh_kill(veh_id);
killed:
    TutWinBaseWindow.check_base(home_base_id);
    if (!VehBattleUnkTgl && on_map(x, y)) {
        if (faction_id == ConsoleGlobal.faction_id_
            && (!stack_check(veh_at(x, y), 1, faction_id, -1, -1) || worm)) {
            draw_radius(x, y, 2, 2);
            return;
        }
        draw_tile(x, y, 2);
    }
}


/*
Purpose: Report the vehicle the map cursor is nearest to.
// ORIGINAL: 0x005C0CB0 ?veh_find@@YAHHHHH@Z 0x005C0CB0-0x005C0DA9 BYTE_EXACT
// size      249 bytes
// prototype int (__cdecl ?veh_find@@YAHHHHH@Z)(int xCoord, int yCoord, int factionID, int skipVehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// HOMED from src/unrecovered/005c0cb0.cpp, 2026-09-04. Its mangled name
// decodes as return int + FOUR int parameters - the return type is the first H
// - and the body reads exactly four stack slots, so the archive's
// four-argument body was right and its five-argument prototype line was not.
// TRIED: `extern "C" int abs(int);` + a local `#pragma function(abs)` (the
//        archive's device for a real `call _abs`). stdafx.h's pragma line
//        already carries abs, so the plain `abs()` here compiles to the same
//        call; the extra local pragma is dead weight and is not carried.
// TRIED: the archive's own spelling - `best_idx` initialised in a declaration
//        and `or eax,0FFFFFFFFh` vs `mov eax,-1` left as the residual
//        MISMATCH; named `int best_idx = -1;` here and the divergence is the
//        same one instruction.
Return Value: the nearest vehicle's id, -1 when nothing qualified.
Status: MISMATCH.
*/
int __cdecl veh_find(int x, int y, int faction_id, int skip_veh_id) {
    int best_idx = -1;
    int best_dist = 9999;
    VehFindBestDist = best_dist;
    int i = 0;
    if (GameDataStateStorage.fields_.vehicle_current_count_ > 0) {
        do {
            if ((faction_id < 0 || Vehs[i].faction_id == (unsigned)faction_id) &&
                i != skip_veh_id) {
                // vector_dist(x, y, veh x, veh y), expanded inline.
                const int veh_y = Vehs[i].y;
                const int veh_x = Vehs[i].x;
                const int dist = vector_dist(x_dist(x, veh_x), abs(y - veh_y));
                if (dist <= best_dist) {
                    best_dist = dist;
                    best_idx = i;
                }
            }
            ++i;
        } while (i < GameDataStateStorage.fields_.vehicle_current_count_);
        if (best_idx >= 0) {
            VehFindBestDist = best_dist;
        }
    }
    return best_idx;
}


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
Purpose: Calculate a unit's morale. TODO: Determine if 2nd param is a toggle for display vs actual
         morale.
// ORIGINAL: 0x005C0E40 ?morale_veh@@YAHHHH@Z 0x005C0E40-0x005C1143 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:5C10DF-5C1100 checks riot flag, faction/home base and morale exemption", "pinned:5C1101-5C1112 decrements/clamps modifier0..6;5C1114-5C1142 then adds vehicle morale and clamps total0..6"], "reason": "Restore the original drone-riot modifier clamp before adding stored vehicle morale, retaining the separate final clamp. Noneligible and morale-exempt paths do not gain this intermediate clamp; complete matching and runtime remain obligations."}
// TRIED: call_diff FEWER (2 vs image's 4: morale_alien, has_tech, and
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
// TRIED (2nd attempt): base.cpp's `bitmask_call` forwarder (general.h),
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
        int probe_morale = range(PlayersData[faction_id].soc_effect_active.values_[SocialEffect::PROBE], 0, 3);
        probe_morale += has_project(SP_TELEPATHIC_MATRIX, faction_id) ? 2 : 0;
        for (int i = 0; i < MaxTechnologyNum; i++) {
            if (Technology[i].flags & TFLAG_IMPROVED_PROBES && has_tech(i, faction_id)) {
                probe_morale++;
            }
        }
        return range(Vehs[veh_id].morale + probe_morale, 2, 6);
    }
    if (proto_id < MaxVehProtoFactionNum && get_proto_offense_rating(proto_id) < 0) {
        return range(Vehs[veh_id].morale, 0, 6); // Basic Psi Veh
    }
    // everything else
    int morale_modifier = range(PlayersData[faction_id].soc_effect_active.values_[SocialEffect::MORALE], -4, 4);
    if (morale_modifier <= -2) {
        morale_modifier++;
    }
    if (morale_modifier >= 2) {
        morale_modifier--;
    }
    if (Players[faction_id].rule_morale < 0) { // negative effects 1st; not 'SOCIAL, MORALE'
        morale_modifier += Players[faction_id].rule_morale;
    }
    int home_base_id = Vehs[veh_id].home_base_id;
    if (home_base_id >= 0) { // home base countering negative effects
        if (has_fac(FAC_CHILDREN_CRECHE, home_base_id, 0) && morale_modifier < 0) {
            morale_modifier /= 2;
        }
        if (has_fac(FAC_BROOD_PIT, home_base_id, 0) && Vehs[veh_id].proto_id < MaxVehProtoFactionNum
            && (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0
                || Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER) && morale_modifier < 0) {
            morale_modifier /= 2; // never reached due to above 'Basic Psi Veh' checks
        }
    }
    if (Players[faction_id].rule_morale > 0) {
        morale_modifier += Players[faction_id].rule_morale;
    }
    BOOL morale_flag = Players[faction_id].rule_flags & RFLAG_MORALE;
    if (morale_flag && morale_modifier < 0) {
        morale_modifier = 0;
    }
    if (check_drone_riot && home_base_id >= 0
        && Bases[home_base_id].state & BSTATE_DRONE_RIOTS_ACTIVE && !morale_flag) {
        morale_modifier = range(morale_modifier - 1, 0, 6);
    }
    morale_modifier += Vehs[veh_id].morale;
    return range(morale_modifier, 0, 6);
}


/*
Purpose: Calculate the offense of the specified prototype. Optional param of the unit defending
         against (-1 to ignore) as well as whether artillery or missile combat is being utilized.
// ORIGINAL: 0x005C1150 ?offense_proto@@YAHHHH@Z 0x005C1150-0x005C128F BYTE_EXACT
// TRIED: `abs()` for the negate - same similarity as the manual ternary. The remaining gap is the same defense_rating byte-load fold as can_arty's plateau (`cmp byte ptr [x], 0` here vs the image's `mov dl, ...; test dl, dl`) plus register-naming noise past that point; not moved by source reshuffling.
// TRIED: `uint32_t weapon_id = VehPrototypes[proto_id].weapon_id;` local, read at its two use sites, made the compiler spill it back into proto_id's own stack slot (an extra store the image never makes) and renamed registers through the entire rest of the function. Writing `VehPrototypes[proto_id].weapon_id` at both call sites instead of caching it dropped that spill: 4/112 -> 47/112 (0.882 -> 0.918 similar). Also reproduced the missing `veh_id_def < 0` guard on the SECOND `Vehs[veh_id_def].proto_id != SPORE_LAUNCHER` check (see the BUG note below) and split `off_rating` into an `int8_t` read plus a widened ternary negate rather than negating an already-widened `int` - the image tests/negates the BYTE before the `movsx`. 47/112 -> 50/112 (0.932 similar).
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
    // Preserve the original unchecked index at this short-circuit phase.
    // The native storage owner places four Reactor records immediately before
    // Vehs, so id -1 reads signed WORD0x9527FE: bits16..31 of the live
    // Reactor[0].name_short pointer. This relies on the verified VC6 layout,
    // not portable array bounds; other invalid indices are not normalized.
    if ((is_bombard || (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating >= 0
        && (veh_id_def < 0 || get_defense_rating(veh_id_def) >= 0)))
        && Vehs[veh_id_def].proto_id != BSC_SPORE_LAUNCHER
        && proto_id != BSC_SPORE_LAUNCHER) {
        int8_t off_rating_byte = get_proto_offense_rating(proto_id);
        int off_rating = (off_rating_byte < 0) ? -off_rating_byte : off_rating_byte;
        // Native 5C1229 tests the signed raw byte; scaling uses its magnitude.
        if (is_proto_missile(proto_id) && off_rating_byte < 99) {
            off_rating = (off_rating * 3) / 2;
        }
        if (veh_id_def >= 0) {
            off_rating *= 8;
        }
        return off_rating; // conventional
    }
    return (veh_id_def < 0) ? Rules.psi_combat_ratio_atk[TRIAD_LAND] : // PSI
        Rules.psi_combat_ratio_atk[get_triad(veh_id_def)] * 8;
}


/*
Purpose: Calculate the defense of the specified prototype. Optional param if unit is being attacked
         (-1 to ignore) as well as whether artillery or missile combat is being utilized.
// ORIGINAL: 0x005C1290 ?armor_proto@@YAHHHH@Z 0x005C1290-0x005C13AD BYTE_EXACT
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
    // Keep the unchecked read after the original bombardment gate. With the
    // verified native Reactor-before-Vehs storage, id -1 observes signed
    // WORD0x9527FE, the upper word of the live Reactor[0].name_short pointer.
    // This is the original layout-dependent access, not a portable bound.
    if (is_bombard && Vehs[veh_id_atk].proto_id != BSC_SPORE_LAUNCHER
        && proto_id != BSC_SPORE_LAUNCHER || (get_proto_defense_rating(proto_id) >= 0
            && (veh_id_atk < 0 || get_offense_rating(veh_id_atk) >= 0))) {
        uint32_t def_rating = range(get_proto_defense_rating(proto_id), 1, 9999);
        if (veh_id_atk >= 0) {
            def_rating *= 8;
        }
        return def_rating; // conventional
    }
    return (veh_id_atk < 0) ? Rules.psi_combat_ratio_def[TRIAD_LAND] : // PSI
        Rules.psi_combat_ratio_def[get_proto_triad(proto_id)] * 8;
}


/*
Purpose: Calculate the speed of the specified prototype on roads.
// ORIGINAL: 0x005C13B0 ?speed_proto@@YAHH@Z 0x005C13B0-0x005C1532 BYTE_EXACT
// size      386 bytes
// prototype int (__cdecl ?speed_proto@@YAHH@Z)(int protoID)
// callers   12   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0
// TRIED: nothing changed - traced the full disassembly instruction-by-instruction
//   against this body (struct offsets, ABL_* constant values, WPN_MODE_TRANSPORT==7,
//   the TRIAD_AIR-only tail) and the control flow order already matches the image
//   exactly. Best flags give 0.81-0.82 similar; remainder looks like register
//   allocation / frame-pointer choice, not a structural difference.
Return Value: Prototype's speed on roads
Status: Complete
*/
int __cdecl speed_proto(int proto_id) {
    int chas_id = VehPrototypes[proto_id].chassis_id;
    int speed_val = Chassis[chas_id].speed;
    if (proto_id == BSC_FUNGAL_TOWER) {
        return 0; // cannot move
    }
    if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
        speed_val += VehPrototypes[proto_id].reactor_id * 2;
    }
    if (Weapon[VehPrototypes[proto_id].weapon_id].mode == WPN_MODE_TRANSPORT) {
        speed_val--;
    }
    if (has_abil(proto_id, ABL_SLOW)) {
        speed_val--;
    }
    if (has_abil(proto_id, ABL_ANTIGRAV_STRUTS)) {
        if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
            speed_val += VehPrototypes[proto_id].reactor_id * 2;
        } else {
            speed_val++;
        }
    }
    if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
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
        if (Weapon[VehPrototypes[proto_id].weapon_id].mode == WPN_MODE_TRANSPORT) {
            speed_val /= 2; // 2nd penalty for air transports: -50%
        }
    }
    return range(speed_val, 1, 99) * Rules.move_rate_roads;
}


/*
Purpose: Calculate the speed of a unit on roads taking into consideration prototype speed,
         elite morale, if the unit is damaged and other factors. The skip_morale parameter seems to
         only be set to true for certain combat calculations in battle_fight().
// ORIGINAL: 0x005C1540 ?speed@@YAHHH@Z 0x005C1540-0x005C1752 BYTE_EXACT
// TRIED: is called UNCONDITIONALLY in the image - `!skip_morale` is tested only AFTER the call and its MORALE_ELITE comparison, not before it (short-circuit order swapped: `morale_veh(...) == MORALE_ELITE && !skip_morale && (...)` instead of `!skip_morale && morale_veh(...) == MORALE_ELITE && (...)`). Confirmed against the disassembly: `call 0x5c0e40` at 0x5C15CB has no guard before it, and the skip_morale parameter isn't read until 0x5C15D8, after the MORALE_ELITE compare at 0x5C15D3.
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
    int speed_val = speed_proto(Vehs[veh_id].proto_id);
    if (get_triad(veh_id) == TRIAD_SEA && has_project(SP_MARITIME_CONTROL_CENTER, Vehs[veh_id].faction_id)) {
        speed_val += Rules.move_rate_roads * 2;
    }
    if (morale_veh(veh_id, true, 0) == MORALE_ELITE && !skip_morale
        && (Vehs[veh_id].proto_id >= MaxVehProtoFactionNum || get_offense_rating(veh_id) >= 0)) {
        speed_val += Rules.move_rate_roads;
    }
    if (Vehs[veh_id].dmg_incurred) {
        int proto_id = Vehs[veh_id].proto_id;
        uint8_t triad = Chassis[VehPrototypes[proto_id].chassis_id].triad;
        if (triad != TRIAD_AIR) {
            // proto_power()'s body, written against this block's proto_id
            // twice: the image shares one prototype address across the
            // triad, plan and reactor reads, which proto_power(veh_id) breaks.
            int power = range((VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT)
                ? 1 : range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10, 1, 99);
            int moves = speed_val / Rules.move_rate_roads;
            speed_val = (moves * range(((VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT)
                ? 1 : range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10)
                - Vehs[veh_id].dmg_incurred, 0, 9999) + power - 1) / power;
            speed_val = range(speed_val, (triad == TRIAD_SEA) ? 2 : 1, 999) * Rules.move_rate_roads;
        }
    }
    return (Vehs[veh_id].proto_id != BSC_FUNGAL_TOWER) ? speed_val : 0;
}


/*
Purpose: Calculate the cargo capacity of a unit. It seems Spore Launchers were considered to have
         cargo capacity at one time.
// ORIGINAL: 0x005C1760 ?veh_cargo@@YAHH@Z 0x005C1760-0x005C17C4 BYTE_EXACT
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
    if (VehPrototypes[proto_id].carry_capacity && proto_id < MaxVehProtoFactionNum && (get_proto_offense_rating(proto_id) < 0
        || proto_id == BSC_SPORE_LAUNCHER)) {
        return Vehs[veh_id].morale + 1;
    }
    return VehPrototypes[proto_id].carry_capacity;
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
        return Rules.extra_pct_cost_proto_sea;
      case TRIAD_AIR:
        return Rules.extra_pct_cost_proto_air;
      case TRIAD_LAND:
      default:
        return Rules.extra_pct_cost_proto_land;
    }
}


/*
Purpose: Calculate the specified prototype's overall cost to build. Optional output parameter
         whether there is an associated 1st time prototype cost (true) or just the base (false).
// ORIGINAL: 0x005C1850 ?veh_cost@@YAHHHPAH@Z 0x005C1850-0x005C1A1D BYTE_EXACT
// LEVER: three real structural fixes moved this 0.545 -> 0.854 similar
// (0/167 in position at the winning `/c /O2 /Gy /GR- /GX`):
//  (1) prototype_factor(proto_id) is a direct same-TU call that /Ob2
//      inlines (it was previously hand-copied);
//  (2) the image has one direct call to base_cost (0x005A5D00, callers 1).
//      The old same-TU `base_cost_call` workaround introduced a synthetic
//      callee; the mapped owner now lives in vehicle_cost.cpp.
//      This current TU boundary is not evidence of the original object file;
//      no separate inline witness for base_cost has been established;
//  (3) `cost`/`proto_cost_first` as plain (signed) `int`, not `uint32_t` -
//      the image's `(cost*3)/4` and the `/100` scaling both carry the
//      `cdq`/sign-correction fixup a signed local earns and an unsigned one
//      does not, even though the values are never negative.
// LEVER: the prototype branch returns on its own with its own
//   has_proto_cost store; the fall-through stores false and returns.
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
        && has_fac(FAC_BROOD_PIT, base_id, 0)) {
        cost = (cost * 3) / 4; // Decrease the cost of alien units by 25%
    }
    if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION && base_id >= 0) {
        cost = range(cost, 1, 999);
    }
    if (proto_id >= MaxVehProtoFactionNum
        && !(VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE)) {
        int factor = prototype_factor(proto_id);
        // `base_cost` IS CALLED UNCONDITIONALLY here in the image, before the
        // FAC_SKUNKWORKS check.
        int proto_cost_first = (factor * base_cost(proto_id) + 50) / 100;
        if (base_id >= 0 && has_fac(FAC_SKUNKWORKS, base_id, 0)) {
            proto_cost_first = 0;
        }
        cost += proto_cost_first;
        if (has_proto_cost) {
            *has_proto_cost = proto_cost_first != 0;
        }
        return cost;
    }
    if (has_proto_cost) {
        *has_proto_cost = false;
    }
    return cost;
}


/*
Purpose: May the player select this vehicle?
// ORIGINAL: 0x005C1A20 ?veh_selectable@@YAHH@Z 0x005C1A20-0x005C1AAA BYTE_EXACT
// size      138 bytes
// prototype int (__cdecl ?veh_selectable@@YAHH@Z)(int vehID)
// callers   11   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1C40
// HOMED from src/unrecovered/005c1a20.cpp, 2026-09-04.
// LEVER: moved beside veh_unmoved/veh_ready (contiguous block; both inline
//        it). The sprite click handlers call it out of line under
//        `#pragma inline_depth(0)`.
// TRIED: nesting the veh_id < 0 guard as `if (veh_id >= 0) { ... }` with a
//        single fall-through return - same jl polarity either way, /O2
//        canonicalises both to the same CFG.
Return Value: 1 when the vehicle is on the map, ours, and not jailed.
Status: MISMATCH.
*/
int __cdecl veh_selectable(int veh_id) {
    if (veh_id < 0) {
        return 0;
    }
    if (!on_map(Vehs[veh_id].x, Vehs[veh_id].y)) {
        return 0;
    }
    if (Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_) {
        return 0;
    }
    if ((Vehs[veh_id].flags & 0x40) != 0 && static_cast<int16_t>(Vehs[veh_id].flags & 0x400) != 0) {
        return 0;
    }
    if (veh_jail(veh_id)) {
        return 0;
    }
    return 1;
}


/*
Purpose: Has this vehicle any moves left?
// ORIGINAL: 0x005C1AB0 ?veh_unmoved@@YAHH@Z 0x005C1AB0-0x005C1B6F BYTE_EXACT
// size      191 bytes
// prototype int (__cdecl ?veh_unmoved@@YAHH@Z)(int vehID)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540 0x005C1C40
// HOMED from src/unrecovered/005c1ab0.cpp, 2026-09-04.
// LEVER: veh_selectable() and veh_moves() inline here (veh_selectable moved
//        into this file beside it).
Return Value: 1 when the vehicle cannot move again this turn.
Status: Complete
*/
int __cdecl veh_unmoved(int veh_id) {
    if (!veh_selectable(veh_id)) {
        return 0;
    }
    return ((Vehs[veh_id].state & 0x4000000) == 0 && Vehs[veh_id].order >= 4 && Vehs[veh_id].order < 0x18) || veh_moves(veh_id);
}


/*
Purpose: Can this vehicle act this turn?
// ORIGINAL: 0x005C1B70 ?veh_ready@@YAHH@Z 0x005C1B70-0x005C1C38 BYTE_EXACT
// size      200 bytes
// prototype int (__cdecl ?veh_ready@@YAHH@Z)(int vehID)
// callers   14   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540 0x005C1C40
// HOMED from src/unrecovered/005c1b70.cpp, 2026-09-04, and its
// pending_bodies.cpp forwarder removed - this definition replaces it. The
// archive's `extern char g_952828_arr[]` scaffolding is Vehs[].
// LEVER: veh_selectable() and veh_moves() inline here; the image's separate
//        `return 0` blocks come from those inline boundaries, not a chain.
Return Value: 1 when the vehicle has moves left and is not jailed.
Status: Complete
*/
// action_arty calls this out of line.
#pragma auto_inline(off)
int __cdecl veh_ready(int veh_id) {
    if (!veh_selectable(veh_id)) {
        return 0;
    }
    if (!(Vehs[veh_id].state & 0x4200)
        && (Vehs[veh_id].order == 1 || Vehs[veh_id].order == 2 || Vehs[veh_id].order == 3)) {
        return 0;
    }
    // A logical-or normalizes the moves count with branches, as the image
    // does (`!= 0` or a ternary gives setne).
    return veh_moves(veh_id) || false;
}
#pragma auto_inline(on)


/*
Purpose: Check if the land unit inside an air transport can disembark. The transport must be in
         either a base or an airbase to do so.
// ORIGINAL: 0x005C1C40 ?veh_jail@@YAHH@Z 0x005C1C40-0x005C1D16 BYTE_EXACT
// size      214 bytes
// prototype int (__cdecl ?veh_jail@@YAHH@Z)(int vehID)
// callers   8   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: the owner test is base_who() <= 0 (inline), and the transport id is an
//   int local (the image sign-extends before testing it).
Return Value: Can the specified unit disembark? true/false
Status: Complete
*/
// Every caller in the image calls this out of line.
#pragma auto_inline(off)
BOOL __cdecl veh_jail(int veh_id) {
    if (get_triad(veh_id) == TRIAD_LAND && Vehs[veh_id].order == ORDER_SENTRY_BOARD) {
        int transport_id = Vehs[veh_id].waypoint_x[0]; // sentry/board stores the transport id
        if (transport_id >= 0 && get_triad(transport_id) == TRIAD_AIR
            && base_who(Vehs[veh_id].x, Vehs[veh_id].y) <= 0
            && !(bit_at(Vehs[veh_id].x, Vehs[veh_id].y) & BIT_AIRBASE)) {
            return true;
        }
    }
    return false;
}
#pragma auto_inline(on)


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
// The order service owns this command; callers retain its external boundary.
// The historical object-file and translation-unit assignment remains unproved.
void __cdecl veh_skip(int veh_id) {
    // The original truncates speeds above 255 into the movement byte.
    Vehs[veh_id].moves_expended = (uint8_t)speed(veh_id, false);
}


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
// Keep this implementation out of line: the pinned callers branch to the
// standalone helper before veh_clear and use its returned fake ID.
int __cdecl veh_fake(int proto_id, int faction_id) {
    veh_clear(2048, proto_id, faction_id);
    return 2048;
}


/*
Purpose: Activate the specified unit and clear the current action.
// ORIGINAL: 0x005C1D70 ?veh_wake@@YAHH@Z 0x005C1D70-0x005C1E1F BYTE_EXACT
// size      175 bytes
// prototype int (__cdecl ?veh_wake@@YAHH@Z)(int vehID)
// callers   6   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C9A50 0x005C1540
// LEVER: `order` as int8_t, matching Vehs[].order's OWN (signed) field type -
//        `uint8_t` forces unsigned `jb`/`jae` on the range check below where
//        the image has signed `jl`/`jge`. Moved 0.903 -> 0.964.
// LEVER: no `state` local - the image re-reads Vehs[].state from memory at
//        each use (`test dword ptr [esi+..], 0x4000000`, then
//        `mov eax,[esi+..] / test ah,2 / test al,4`, then again for the
//        trailing `and`), and caching it in a local cost a spill slot that
//        pushed the whole frame onto `push ecx` instead of the image's
//        `push ebp` frame. Byte-exact 55/55 on removing it alone.
Return Value: Unit id (doesn't look to be used on return)
Status: Complete
*/
int __cdecl veh_wake(int veh_id) {
    if (Vehs[veh_id].order >= ORDER_FARM && Vehs[veh_id].order < ORDER_MOVE_TO && !(Vehs[veh_id].state & VSTATE_CRAWLING)) {
        // TODO bug fix: Issue with moves_expended size / speed return, see veh_skip()
        Vehs[veh_id].moves_expended = (uint8_t)(speed(veh_id, false) - Rules.move_rate_roads);
        int terra_turns = Vehs[veh_id].terraforming_turns;
        if (terra_turns) {
            terra_turns -= contribution(veh_id, Vehs[veh_id].order - 4);
            if (terra_turns < 0) {
                terra_turns = 0;
            }
            Vehs[veh_id].terraforming_turns = (uint8_t)terra_turns;
        }
    }
    if (Vehs[veh_id].state & VSTATE_UNK_200 && Vehs[veh_id].order_auto_type == ORDERA_ON_ALERT
        && !(Vehs[veh_id].state & VSTATE_UNK_4)) {
        Vehs[veh_id].moves_expended = 0;
    }
    Vehs[veh_id].order = ORDER_NONE;
    Vehs[veh_id].state &= ~(VSTATE_UNK_200 | VSTATE_EXPLORE | VSTATE_UNK_1000000
        | VSTATE_UNK_2000000 | VSTATE_UNK_8000000); // TODO: Identify VSTATE flags
    return veh_id;
}
