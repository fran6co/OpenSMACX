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
#include "terraforming.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "general.h"
#include "map.h"
#include "technology.h"
#include "veh.h"


/*
Purpose: Calculate the credit cost to lower or raise the tile's terrain for the specified faction.
// ORIGINAL: 0x004C9420 ?terraform_cost@@YAHHHH@Z 0x004C9420-0x004C96D6 BYTE_EXACT
// LEVER: both-base_finds-run the second `base_find` is NOT nested inside
//   `if (base_id >= 0)`. The image pushes the first call's three arguments and
//   the second's six back to back and cleans both up with one `add esp, 0x24`
//   at 0x004C94BD, BEFORE `test edi, edi` at 0x004C94C3. Nested, this tree made
//   7 calls against the image's 8 and `call_diff` reported FEWER; hoisted it is
//   8 against 8, and the body went 8 of 236 to 27 of 236, similarity
//   0.174 -> 0.515 at `/c /O2 /Gy /GR- /Oy- /GX`. This is a BEHAVIOUR fix, not
//   an encoding one: the image searches for the nearest foreign base on every
//   call, including the ones with no friendly base on the tile.
// LEVER: one-tile-pointer `Map *const tile = map_loc(x, y);` with the climate
//   byte read ONCE into a local, instead of `alt_at` + `bit_at` +
//   `altitude_at`. The image computes the tile address once into edx
//   (0x004C9449), reads `[edx]` for the climate and folds the fungus test into
//   the memory operand, `test byte ptr [edx + 8], 0x20`. Three accessor calls
//   leave a separate byte load before the test.
// LEVER: abs-is-a-branch-here `int cost = alt - 3; if (cost < 0) cost = 3 - alt;`
//   is what the image emits - `lea ecx, [eax-3]; test ecx, ecx; jge; mov ecx, 3;
//   sub ecx, eax`. `abs()` compiles to VC6's branchless intrinsic
//   `cdq; xor eax, edx; sub eax, edx`, which is three different instructions in
//   the same place. Note the image DOES call `_abs` at 0x004C94F0, inside the
//   inlined `cursor_dist`, so this is per-site and not the `/Oi-` axis.
// LEVER: climate-unsigned-alt-signed `uint32_t climate` gives the image's
//   `shr eax, 5`; as `int` it is `sar`. The two comparisons ON the result are
//   signed - `cmp edx, 0x40; jl` and `cmp eax, 3; jge` - so `alt` and the
//   masked altitude are `int`.
// TRIED: parameter-register-assignment what is left is allocation. The
//   image loads y into ebx and x into esi (0x004C942C, 0x004C9430); this tree
//   loads y into esi and x into ebx, in the same ORDER, and every instruction
//   naming either register disagrees from there on. It spills the climate byte
//   into the dead `y` parameter slot, `mov [ebp+0xc], eax`, where this tree
//   keeps it in edi - which is free here precisely BECAUSE of the register
//   swap. Nothing in the body's spelling chooses between them; `map_loc`'s
//   `(x >> 1) + y * MapLongitude` is in map.h and shared with every caller.
// TRIED: mask-encoding the image's altitude mask is `83 E2 E0`, the
//   sign-extended imm8 form of 0xFFFFFFE0, where `& 0xE0` needs the six-byte
//   `81 /4 id`. Writing `& ~0x1Fu` DOES emit the short form and still costs an
//   agreeing instruction (27 -> 26), because the register the mask applies to
//   differs anyway and the three saved bytes shift every later offset.
//   Measured and reverted; the tree's `altitude_at` spelling stands.
// size      694 bytes
// prototype int (__cdecl ?terraform_cost@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   3   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3C60 0x004E3D50 0x00644F3A
Return Value: Credit cost
Status: Complete
*/
#pragma optimize("y", off)
int __cdecl terraform_cost(int x, int y, int faction_id) {
    Map *const tile = map_loc(x, y);
    const uint32_t climate = tile->climate;
    const int alt = static_cast<int>(climate >> 5);
    int cost = alt - 3;
    if (cost < 0) {
        cost = 3 - alt;
    }
    cost += 2;
    cost *= cost;
    if (tile->bit & BIT_FUNGUS
        && static_cast<int>(climate & ~0x1Fu) >= ALT_BIT_OCEAN_SHELF) {
        cost *= 3;
    }
    cost *= 2;
    if (alt < ALT_SHORE_LINE) {
        cost *= 2;
        if (!GameDataStateStorage.fields_.game_tech_achieved_[TECH_DOCAIR]) {
            cost *= 2;
        }
    }
    int base_id = base_find(x, y, faction_id);
    // BOTH SEARCHES RUN, always: the image pushes the three arguments of the
    // first and the six of the second back to back and cleans up once,
    // `add esp, 0x24` at 0x004C94BD, BEFORE it tests base_id at 0x004C94C3.
    // Nesting the second inside `if (base_id >= 0)` makes this tree skip a
    // call the image makes - `call_diff` reports FEWER.
    int base_id_prox = base_find(x, y, -1, -1, faction_id, -1);
    if (base_id >= 0) {
        const int base_y = Bases[base_id].y;
        const int base_x = Bases[base_id].x;
        int cursor_distance = (x_dist(x, base_x) + abs(y - base_y)) >> 1;
        cost *= range(cursor_distance, 1, 100);
        if (base_id_prox >= 0
            && (PlayersData[faction_id].diplo_treaties[
                    Bases[base_id_prox].faction_id_current] & DTREATY_PACT) == 0) {
            const int numerator_y = Bases[base_id].y;
            const int numerator_x = Bases[base_id].x;
            int num_prox = (((x_dist(x, numerator_x) + abs(y - numerator_y)) >> 1)
                * (Bases[base_id_prox].population_size + 2)) / 3;
            const int denominator_y = Bases[base_id_prox].y;
            const int denominator_x = Bases[base_id_prox].x;
            int denom_prox = (((x_dist(x, denominator_x) + abs(y - denominator_y)) >> 1)
                * (Bases[base_id].population_size + 2)) / 3;
            if (denom_prox && num_prox && denom_prox < num_prox) {
                cost = (cost * num_prox) / denom_prox; // increase cost based on proximity ratio
            }
        }
    }
    if (Players[faction_id].rule_flags & RFLAG_TERRAFORM) {
        cost /= 2;
    }
    cost /= 2;
    return range(cost, 1, 30000);
}
#pragma optimize("", on)

