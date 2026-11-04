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
#include "map.h"
#include "console.h"

// Keep the mapped operation outside veh.cpp so its same-TU callsites retain
// the witnessed call boundary. The original object-file/TU assignment is
// unknown; this is only the current source ownership boundary.
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
Purpose: Sort a stack of units with the transports moved to the top.
// ORIGINAL: 0x005B8B60 ?stack_sort@@YAXH@Z 0x005B8B60-0x005B8C8D
// TRIED: and `stack_put(veh_id_put, x, y)` were plain calls; call_diff's count-based check missed it (6 calls either way, since {veh_top, stack_put} happened to replace the {2 extra veh_lift/veh_drop} the image gets from inlining stack_put), but the image's actual 6 callees are veh_cargo/has_abil/veh_lift x2/ veh_drop x2 - no veh_top or stack_put. Hand-inlining both, including stack_put's OWN internal veh_top climb (its outer `next_veh_id >= 0` check survives - the image keeps it as a second, separate test after veh_top's own), took 0.627 -> 0.758.
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
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
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

// Player admission is an order-service query. The current owner preserves
// the original external calls from vehicle sprite handlers; the original
// object-file/translation-unit assignment remains unproved.
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
    return veh_jail(veh_id) == 0;
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
// The order service owns this command; callers retain its external boundary.
// The historical object-file and translation-unit assignment remains unproved.
void __cdecl veh_skip(int veh_id) {
    // The original truncates speeds above 255 into the movement byte.
    Vehs[veh_id].moves_expended = (uint8_t)speed(veh_id, false);
}

/*
Purpose: Add a direction's tile offset to a vehicle's coordinates.
// ORIGINAL: 0x00506A60 ?battle_fight@@YAHHHHHPAH@Z 0x00506A60-0x00506AEB
// RECOVERY_BINDING: v1 {"reason":"Use the complete shared radius-offset owners for nonzero mode rather than truncated eight-element duplicates.","evidence":["Pinned506A70/506AB8 index66EFBC/66F440 directly; these are the existing289-dword RadiusOffsetX/Y owners in map.cpp.","Pinned action_destroy4CABEF..4CACFD supplies mode1 indices1..24; preserve unchecked direct indexing and the original single-wrap coordinate adjustment."]}
// size      139 bytes
// prototype int (__cdecl ?battle_fight@@YAHHHHHPAH@Z)(int vehID, int dir, int mode, int allow_interceptor, int *odds_out)
// callers   6   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00506AF0
// HOMED from src/unrecovered/00506a60.cpp, 2026-09-04. Its `g_00952828`
// short views are Vehs[].x (+0x00) and Vehs[].y (+0x02).
// `mode` is named from what it does here and in the seven-parameter body:
// a zero/nonzero flag selecting ordinary directions or full radius offsets. `allow_interceptor` and
// `odds_out` are named from the seven-parameter body - the first gates its
// call to interceptor (0x00506650), the second is the null-tested pointer it
// stores the odds word through at 0x005070A2.
Return Value: whatever the seven-parameter battle_fight returns.
Status: MISMATCH.
*/

// The genuine seven-parameter combat owner remains in veh.cpp.
// Its full original span and historical seed lessons remain source-owned there.
int __cdecl battle_fight(int veh_id, int dir, int x, int y, int src_mode,
                         int allow_interceptor, int *odds_out);

int __cdecl battle_fight(int veh_id, int dir, int mode, int allow_interceptor,
                         int *odds_out) {
    int x;
    if (mode != 0) {
        x = RadiusOffsetX[dir];
    } else {
        x = VehMoveDeltaX[dir];
    }
    const int val_x = wrap_map_x(Vehs[veh_id].x + x);

    const int y = mode != 0 ? RadiusOffsetY[dir] : VehMoveDeltaY[dir];
    const int val_y = Vehs[veh_id].y + y;

    return battle_fight(veh_id, dir, val_x, val_y, mode, allow_interceptor,
                        odds_out);
}
