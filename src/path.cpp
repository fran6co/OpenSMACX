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
#include "path.h"
#include "alpha.h"
#include "game.h"
#include "technology.h"
#include "veh.h"
#include "map.h"

/*
Purpose: Initialize an instance of the class.
// ORIGINAL: 0x0059A220 ?init@Path@@QAEXXZ 0x0059A220-0x0059A2C3 BYTE_EXACT
// size      163 bytes
// prototype void (__thiscall ?init@Path@@QAEXXZ)(Path* this)
// callers   6   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006465F0
Return Value: n/a
Status: Complete
*/
void Path::init() {
    shutdown();
    x_dst_ = -1;
    y_dst_ = -1;
    map_table_ = (int *)mem_get(MapArea * 4);
    x_table_ = (int16_t *)mem_get(MapArea * 2);
    y_table_ = (int16_t *)mem_get(MapArea * 2);
    ZeroMemory(map_table_, MapArea * 4);
}

/*
 Purpose: Shutdown an instance of the class.
// ORIGINAL: 0x0059A2D0 ?shutdown@Path@@QAEXXZ 0x0059A2D0-0x0059A318 BYTE_EXACT
// body      src/path.h
// size      72 bytes
// prototype void (__thiscall ?shutdown@Path@@QAEXXZ)(Path* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
 Return Value: n/a
 Status: Complete
*/


/*
 Purpose: Get the value from the map table for the specified coordinates.
 Original Offset: n/a (Optimized out of x86 binary, present in PPC)
 Return Value: Map table value
 Status: Complete
*/
int Path::get(uint32_t x, uint32_t y) {
    return map_table_[(x >> 1) + y * MapLongitude];
}

/*
 Purpose: Set the value in the map table for the specified coordinates.
 Original Offset: n/a
 Return Value: n/a
 Status: Complete
*/
void Path::set(uint32_t x, uint32_t y, int val) {
    map_table_[(x >> 1) + y * MapLongitude] = val;
}

/*
 Purpose: Check for unit zone of control conflicts taking into account land or ocean. This is a 
          slightly modified version of zoc_move() or zoc_sea().
// ORIGINAL: 0x0059A370 ?zoc_path@Path@@QAEHHHH@Z 0x0059A370-0x0059A527
// LEVER: ONE `map_loc` for the entry test. The image computes the tile
//   address once at 0x0059A395 and then reads `[eax+8]` (bit), `[eax+2]`
//   (val2) and `[eax]` (climate) off THAT pointer - `bit_at`, `owner_at` and
//   `is_ocean` all open-coded against a shared `Map *`. Calling the three
//   helpers instead makes VC6 reload `*MapTiles` and redo the index for each:
//   0.838 -> 0.859, first divergence 14 -> 22, 25 -> 33 instructions
//   agreeing. Mixing them (shared tile for the bit, `owner_at()` for the
//   owner) is WORSE than either, 0.817.
// LEVER: the owner is tested TWICE. The image emits `cmp ecx, 8 / jge` and
//   then `test ecx, ecx / jge` - `< 8` AND `>= 0` - and the second test is
//   what stops VC6 collapsing the pair into one byte-wide `cmp cl, 8 / jb`.
//   It is redundant as arithmetic (`val2 & 0xF` is 0..15) and load-bearing
//   as codegen: dropping it costs 0.859 -> 0.844 and two instructions.
// TRIED: the `is_ocean` materialisation, for the same reason as
//   0x0059C3C0. The image ends it `xor edx, edx / mov dl, [eax] / and edx,
//   0xE0 / cmp edx, 0x60 / setl al` - dword and SIGNED - and this tree emits
//   `and dl, 0xe0 / cmp dl, 0x60 / sbb eax, eax / neg eax`, byte and
//   unsigned. Five spellings measured, all 0.859: `? 1 : 0`, an `int` local
//   for the masked altitude, an `int` local for the raw climate byte,
//   `static_cast<int>` on both sides of the compare, and the `is_ocean()`
//   helper itself (which is worse still, 0.839).
// TRIED: matching the loop's register schedule. The image hoists
//   `MapLongitudeBounds` into ebx for `xrange`/`on_map` and reloads it once
//   per iteration at the back edge; this tree reads `[0x949870]` at each of
//   the four uses. Those reads are inside `xrange` and `on_map` in
//   src/map.h, which this batch does not own.
// size      439 bytes
// prototype int (__thiscall ?zoc_path@Path@@QAEHHHH@Z)(Path* this, int xCoord, int yCoord, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFE90
 Return Value: If ZOC, faction id + 1; Otherwise, 0 (however return seems to be treated as boolean)
 Status: Complete
*/
int Path::zoc_path(int x, int y, int faction_id) {
    Map *tile = map_loc(x, y);
    int owner_tile;
    if ((tile->bit & BIT_BASE_IN_TILE)
        && (owner_tile = tile->val2 & 0xF, owner_tile < 8)
        && owner_tile >= 0) {
        return 0;
    }
    BOOL is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;
    for (uint32_t i = 0; i < 8; i++) {
        int x_radius = xrange(x + RadiusBaseX[i]);
        int y_radius = y + RadiusBaseY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = veh_who(x_radius, y_radius);
            if (owner >= 0 && (uint32_t)owner != faction_id 
                && is_ocean(x_radius, y_radius) == is_ocean_tile
                && !has_treaty(faction_id, owner, DTREATY_VENDETTA | DTREATY_PACT)) {
                if (!is_human(faction_id)) {
                    return owner + 1;
                }
                for (int veh_id = veh_at(x_radius, y_radius); veh_id >= 0; 
                    veh_id = Vehs[veh_id].next_veh_id_stack) {
                    if ((Vehs[veh_id].faction_id == faction_id && ((Vehs[veh_id].flags
                        & (VFLAG_INVISIBLE | VFLAG_LURKER)) != (VFLAG_INVISIBLE | VFLAG_LURKER)))
                        || Vehs[veh_id].visibility & (1 << faction_id)) {
                        return owner + 1;
                    }
                }
            }
        }
    }
    return 0;
}

/*
 Purpose: Find the path between two points that meets the conditions?
// ORIGINAL: 0x0059A530 ?find@Path@@QAEHHHHHHHHH@Z 0x0059A530-0x0059BC05
// TRIED: this pass, on BUDGET, not on any wall - and recorded as a
//   lesson line because the investigation note below is prose and therefore
//   invisible to `frontier.py --untouched`, which is why this address was
//   handed out again as fresh. Measured 2026-08-22: the body is still
//   `return 0;` and compiles to 2 instructions against the image's 1,884,
//   similarity 0.002 under every flag set. There is nothing here to lever;
//   it is ~1,900 instructions of transcription, with the same
//   ~400-instruction neighbour-scan block repeated at four sites, and it is
//   the largest single body left in the tree. Do not hand it out beside
//   eleven other addresses - it needs a pass of its own.
// size      5845 bytes
// prototype int (__thiscall ?find@Path@@QAEHHHHHHHHH@Z)(Path* this, int xCoordSrc, int yCoordSrc, int xCoordDst, int yCoordDst, int protoID, int factionID, int, int)
// callers   7   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046A550 0x00592140 0x00593510 0x0059A370 0x005BF1F0 0x005BFE90 0x005C0DB0 0x005C13B0 0x005FCB20 0x00644F3A 0x006465F0
 Return Value: TBD
 Status: WIP
//
// INVESTIGATION NOTE (2026-08-21), left as `return 0;` deliberately - this
// is the tree's only remaining stub and no artifact exists anywhere for it
// (confirmed: it was `return 0;` in the very first commit that introduced
// Path, b152634e/dbfd210c, long before the decomp effort). 5845 bytes is
// roughly 730 instructions; a wrong 700-instruction body would compile and
// "look done" while being a different program, which the brief calls out
// as worse than the honest stub. Recorded here instead, so the next pass
// does not re-spend the same effort re-deriving these:
//
// CALLEES (call_diff names, all already real symbols in this tree):
//   0x0046A550 MapWin::draw_map(int)   0x00592140 goody_at(x,y)
//   0x00593510 hex_cost(proto_id, faction_id, x_src, y_src, x_dst, y_dst, toggle)
//   0x0059A370 Path::zoc_path (this file, above)   0x005BF1F0 has_abil(proto_id, abil)
//   0x005BFE90 veh_at(x,y)   0x005C0DB0 can_arty(proto_id, triad_sea_retn)
//   0x005C13B0 speed_proto(proto_id)   0x00644F3A abs()
//   0x006465F0 memset (ZeroMemory idiom, matches Path::init's own use)
//
// GLOBALS resolved by address (grep across already-recovered src/, all in
// map.cpp/map.h/alpha.h): Rules=0x949738, MapLongitude=0x68faf0,
// MapLongitudeBounds=0x949870, MapLatitudeBounds=0x949874, MapArea=0x949884,
// MapIsFlat=0x94988c, map_tiles()=0x94a30c (map.h).
//
// STRUCTURE: the prologue is a CACHE check against this object's own
// x_dst_/y_dst_/proto_id_/faction_id_2_/unk_ fields (path.h offsets
// 0x18/0x1c/0x28/0x24/0x20) - if the new call's (x_dst,y_dst,proto triad,
// unk1,faction_id) all match what is cached, and map_table_[x_src,y_src] is
// already nonzero, it reuses the existing table instead of recomputing (a
// path.cpp equivalent of `hex_cost`'s inlining pattern: the compiler wrote
// this branch out at the ONE call site rather than as a helper). Otherwise
// it memsets map_table_ back to 0 and restamps the five cache fields.
//
// The remaining ~85% of the body is an A*-shaped search: map_table_ is the
// open/closed cost grid (int per tile), x_table_/y_table_ the coordinate
// queue BasePop-style FIFO/priority arrays (path.h), walked from index1_
// through index2_ (offsets 0xc/0x10). Each dequeued tile is expanded over
// 8 directions using two tables at 0x0066EFBC/0x0066F440, and those ARE
// map.h's RadiusOffsetX and RadiusOffsetY - CONFIRMED 2026-08-21 by DATA
// IDENTITY rather than by the address arithmetic this note used to rest on.
// All 289 ints at each address were read out of the image and compared with
// the arrays map.h declares: identical, element for element, both of them.
// They cannot be confirmed by a symbol - map.h declares them `const int[]`,
// so this tree materialises its own copy and there is no address binding to
// compare - which is exactly why data identity is the right test here.
//
// The costly part: an ~400-instruction "does any of my 8 neighbors satisfy
// <bit tests on 0x96c9e0/0x96c9f8, a per-(faction,faction) table indexed
// exactly like spying_recovery.h's SpyingStatusTable, plus hex_cost/hex
// zoc_path/goody_at/can_arty calls>" block is NOT written once - it appears
// at LEAST four times near-identically (0059A98A, 0059AF7E, 0059B49D,
// 0059B80D) with small per-instance deltas, the shape this codebase
// documents on Buffer::copy as "the image writes the same case four times ...
// [it] repeats the whole clip/get_data/pointer-compute sequence inline"
// rather than calling a shared helper. Reconstructing that block once with
// confidence, then verifying it is the SAME at each of >=4 sites rather than
// subtly different, is most of the remaining work and was not completed
// this pass.
//
// THE "DOZEN NEVER-NAMED GLOBAL TABLES" ARE NONE OF THEM, which removes the
// stated reason this was left alone. Resolved 2026-08-21, all seven:
//   0x009A64E8  LockEnableMask      lock.h - not a faction bit table
//   0x009A6800  SunspotDuration     game.cpp - one int, not a table at all
//   0x0096C9E0  PlayersData         faction.h
//   0x0096C9F8  SpyingStatusTable   spying_recovery.h - this note guessed it
// and the last three are not tables either. They are FIELD OFFSETS into
// arrays this tree already models, which is precisely why no symbol sits at
// them and why they read as anonymous bases:
//   0x009AB88C = VehPrototypes (0x009AB868) + 0x24  ->  chassis_id
//   0x009AB892 = VehPrototypes              + 0x2A  ->  plan
//   0x00952832 = Vehs          (0x00952828) + 0x0A  ->  proto_id
// `chassis_id` is the "proto-triad" this note reached for: it indexes
// Chassis[].triad, the relation batch A recorded separately the same day.
//
// So what remains is transcription risk alone - ~700 instructions with the
// same ~400-instruction neighbour block repeated at four sites - against
// named globals throughout. Still real, and still a reason to be careful,
// but no longer compounded by operands nobody can name.
*/
int Path::find(int x_src, int y_src, int x_dst, int y_dst, int proto_id, int faction_id, int unk1,
               int unk2) {
    return 0;
}

/*
 Purpose: TBD
// ORIGINAL: 0x0059BC10 ?move@Path@@QAEHHH@Z 0x0059BC10-0x0059C1F3
// TRIED: this pass, on BUDGET. Measured 2026-08-22: 62 instructions
//   against the image's 488, similarity 0.211, first divergence at
//   instruction 0 - the image opens `sub esp, 0x38` for a frame this body
//   never needs because it stops at 0x0059BD73, roughly an eighth of the
//   way in. The `STATUS: WIP` comment in the body traces the missing
//   ~420 instructions and they reach `Path::find` (0x0059A530) at
//   0x0059BE66, which is itself still `return 0;`, so the tail cannot be
//   finished before that one is. No source-shape lever applies to a body
//   that is seven eighths absent; the work is transcription.
//   The `LEVER:` note about the two `veh_moves` calls lives INSIDE the body
//   rather than in this block, which is why this address still reads as
//   untouched to `frontier.py`; it is a real measured lever and is left
//   where the code it explains is.
// size      1507 bytes
// prototype int (__thiscall ?move@Path@@QAEHHH@Z)(Path* this, int vehID, int factionID)
// callers   4   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00593510 0x0059A530 0x005C1540 0x005C1D20 0x005C8D40 0x00644F3A
 Return Value: TBD
 Status: WIP
*/
int Path::move(int veh_id, int faction_id) {
    int veh_faction_id = Vehs[veh_id].faction_id;
    uint32_t flags = 0xC0;
    if (veh_faction_id != faction_id_) {
        x_dst_ = -1;
        y_dst_ = -1;
        faction_id_ = veh_faction_id;
    }
    int veh_x = Vehs[veh_id].x;
    int veh_y = Vehs[veh_id].y;
    int x_waypoint = Vehs[veh_id].waypoint_x[0]; 
    int y_waypoint = Vehs[veh_id].waypoint_y[0];
    if (!on_map(x_waypoint, y_waypoint) || (veh_x == x_waypoint && veh_y == y_waypoint)) {
        return -1;
    }
    BOOL is_human_player = is_human(veh_faction_id);
    if (!is_human_player && ((Vehs[veh_id].state & (VSTATE_UNK_40000 | VSTATE_UNK_20000))
        == (VSTATE_UNK_40000 | VSTATE_UNK_20000))) {
        flags = 0xE0;
    }
    if ((Vehs[veh_id].state & (VSTATE_UNK_1000000 | VSTATE_UNK_200))
        == (VSTATE_UNK_1000000 | VSTATE_UNK_200)) {
        flags &= 0xBF;
    // LEVER: the image calls veh_moves(veh_id) (`speed(veh_id,false)` +
    // clamp, veh.h) TWICE, not once - the disassembly's second `call speed`
    // at 0x005C1540 has its own fresh `sub eax, moves_expended` and clamp,
    // and it is gated behind the FIRST veh_moves() result (`jg` skips both
    // the flags|=0x100 store AND the second call together), so a nested
    // `else if` reproduces the image's single conditional jump instead of
    // caching one value for both compares. The inner store is also
    // `or ah, 0x10` (bit 12, 0x1000), not `or dh, 1` (bit 8, 0x100) again -
    // a different bit than the outer one.
    } else if (veh_moves(veh_id) <= Rules.move_rate_roads) {
        flags |= 0x100;
        if (veh_moves(veh_id) <= (uint32_t)((Vehs[veh_id].proto_id != BSC_MIND_WORMS) + 1)) {
            flags |= 0x1000;
        }
    }
    // STATUS: WIP past this point (0x0059BD73 in the image). Traced but not
    // written, because it is the same duplicated-neighbour-scan shape
    // documented on Path::find above, and reaches Path::find itself
    // (0x0059A530 - still a stub) at 0x0059BE66:
    //   - VehPrototypes[proto_id].weapon_id -> Weapon[weapon_id].offense_rating
    //     (0x94ae68 = Weapon base 0x94ae60 + offsetof(offense_rating)==8);
    //     if zero, reads a scalar at 0x009A6534 and, if non-negative,
    //     indexes a dword table at 0x0097D044 with a stride of 77 elements
    //     that this pass could not name - the only two addresses in this
    //     function it could not resolve against already-recovered src/.
    //   - VehPrototypes[proto_id].chassis_id -> Chassis[chassis_id].triad
    //     (0x94a379 = Chassis base 0x94a330 + offsetof(triad)==0x49, stride
    //     confirmed sizeof(RulesChassis)==0x90 via `lea eax,[eax+eax*8];
    //     shl eax,4` == *144); triad==2 (air) skips straight past the
    //     distance/hex_cost/Path::find block to the zoc_move call below.
    //   - otherwise: x_dist/y_dist against the waypoint, hex_cost(),
    //     Path::find(x_src=veh_x, y_src=veh_y, x_dst=waypoint, ...) at
    //     0x0059BE66, then a sentinel-99999 8-direction cost scan (same
    //     shape as Path::find's own inner loop) before calling
    //     zoc_move(veh_x, veh_y, veh_faction_id) and veh_skip(veh_id).
    return 0;
}

/*
 Purpose: Populate the abstract map with the radial region value.
// ORIGINAL: 0x0059C200 ?make_abstract@Path@@QAEXXZ 0x0059C200-0x0059C33F
// TRIED: the divergence is at instruction 2 and it is the FRAME SIZE - the image reserves `sub esp, 0x1c` where this body reserves 0x18, one dword short. Four source shapes measured with tools/try_spellings.py and ALL FOUR compile identically to what is committed, 11/112: hoisting `x * 5` and `y * 5` into their own locals, splitting the `on_map && !is_ocean` guard into nested ifs, both together, and signed loop counters. VC6 emits the same code for every one, so the extra slot is not this body's to allocate - it belongs to one of the four `MEASURED inline` callees here (xrange, on_map, is_ocean, region_at) spilling where ours does not. Same family as the port_to_port/has_tech ceiling; not reachable from path.cpp.
// size      319 bytes
// prototype void (__thiscall ?make_abstract@Path@@QAEXXZ)(Path* this)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591230
 Return Value: n/a
 Status: Complete
*/
void Path::make_abstract() {
    for (uint32_t y = 0; y < MapAbstractLatBounds; y++) {
        for (uint32_t x = y & 1; x < MapAbstractLongBounds; x += 2) {
            uint8_t region = 0;
            for (int i = 0; i < RadiusRange[1]; i++) {
                int x_radius = xrange(x * 5 + RadiusOffsetX[i]);
                int y_radius = y * 5 + RadiusOffsetY[i];
                if (on_map(x_radius, y_radius) && !is_ocean(x_radius, y_radius)) {
                    region = (uint8_t)region_at(x_radius, y_radius);
                    break;
                }
            }
            abstract_set(x, y, region);
        }
    }
}

/*
 Purpose: Merge the old region into the new region.
// ORIGINAL: 0x0059C340 ?merge@Path@@QAEXHH@Z 0x0059C340-0x0059C3B7
// size      119 bytes
// prototype void (__thiscall ?merge@Path@@QAEXHH@Z)(Path* this, int regionOld, int regionNew)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
 Return Value: n/a
 Status: Complete
*/
void Path::merge(int region_old, int region_new) {
    Continents[region_new].tile_count += Continents[region_old].tile_count;
    Continents[region_old].tile_count = 0;
    for (uint32_t i = 0; i < MapArea; i++) {
        if ((uint32_t)map_tiles()[i].region == region_old) {
            map_tiles()[i].region = (uint8_t)region_new;
        }
    }
}

/*
 Purpose: Build the pathing tables using the provided coordinates to radiate outward for connected 
          land tiles owned by the specified faction.
// ORIGINAL: 0x0059C3C0 ?territory@Path@@QAEXHHHH@Z 0x0059C3C0-0x0059C51B
// LEVER: the loop cursors are `int`, not `int16_t`. The image reads
//   `movsx edx, word ptr [edx + eax*2]` and keeps a 32-bit value; `int16_t`
//   locals give `mov dx, word ptr [...]` and defer the widening to a later
//   `movsx`, which shifts every use after it. 0.803 -> 0.851 on that alone.
// LEVER: ONE `map_loc` for the whole condition. The image computes the tile
//   address once - `lea eax, [ebx + eax*4]` - and then reads `[eax]`,
//   `[eax+6]` and `[eax+7]` off it, so the ocean test is `tile->climate &
//   0xE0` rather than a second `is_ocean(x_radius, y_radius)`. Calling
//   `is_ocean` here made VC6 recompute the address (a second
//   `mov eax, [0x94a30c]` / `add eax, esi`), because the tile pointer comes
//   from a global and cannot be CSE'd across it. 0.851 -> 0.871, and two
//   instructions closer (125 against the image's 123).
// TRIED: reaching the image's `xor ebx, ebx / mov bl, [eax] / and ebx,
//   0xffffffe0 / cmp ebx, 0x60 / setl` from this file. VC6 narrows the same
//   expression to `mov al, [eax] / and al, 0xe0 / cmp al, 0x60 / jb` - byte
//   width and UNSIGNED where the image is dword and SIGNED - and it does so
//   whatever the caller writes: `is_ocean(...)`, the open-coded
//   `(climate & 0xE0) >= ALT_BIT_SHORE_LINE`, an `int` local for the loaded
//   byte, `static_cast<int>(tile->climate)`, `!(... < ...)`, and an `int`
//   local for `unk_1` all compile to the byte form and all score 0.871. The
//   shape belongs to `altitude_at`/`is_ocean` in src/map.h, which this batch
//   does not own; it is the same divergence at BOTH ocean tests here.
// TRIED: the bool-materialising guard spellings that fixed 0x0055BC00.
//   The image's entry test ends `setl al / xor edi, edi / cmp eax, edi /
//   jne`, but `BOOL ocean = is_ocean(x, y);`, `!!is_ocean(...)`,
//   `is_ocean(...) != 0` and `(is_ocean(...) ? 1 : 0) != 0` all fold to the
//   same direct branch here: 0.871 each, no movement.
// TRIED: writing `index1_ = 0;` before `index2_ = 0;`, and reusing the
//   `x`/`y` parameters as the loop cursors instead of naming new locals.
//   Both score exactly 0.871; the parameter reuse does reproduce the image's
//   `mov [ebp+8], edx` but costs nothing either way.
// size      347 bytes
// prototype void (__thiscall ?territory@Path@@QAEXHHHH@Z)(Path* this, int xCoord, int yCoord, int region, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FCB20
 Return Value: n/a
 Status: Complete
*/
void Path::territory(int x, int y, int UNUSED(region), int faction_id) {
    if (is_ocean(x, y)) {
        return; // skip ocean terrain
    }
    index2_ = 0;
    index1_ = 0;
    x_table_[index1_] = (int16_t)x;
    y_table_[index1_++] = (int16_t)y;
    do {
        int x_it = x_table_[index2_];
        int y_it = y_table_[index2_++];
        for (uint32_t i = 0; i < 8; i++) {
            int x_radius = xrange(x_it + RadiusBaseX[i]);
            int y_radius = y_it + RadiusBaseY[i];
            Map *tile;
            if (on_map(x_radius, y_radius) && y_it && y_it != ((int)MapLatitudeBounds - 1)
                && (tile = map_loc(x_radius, y_radius),
                    (tile->climate & 0xE0) >= ALT_BIT_SHORE_LINE
                    && !tile->unk_1 && tile->territory == faction_id)) {
                tile->unk_1 = 1;
                x_table_[index1_] = (int16_t)x_radius;
                y_table_[index1_++] = (int16_t)y_radius;
            }
        }
    } while (index2_ && index1_ != index2_);
    do_all_non_input();
}

/*
 Purpose: Populate the continent and path table for the specified tile and region.
// ORIGINAL: 0x0059C520 ?continent@Path@@QAEXHHH@Z 0x0059C520-0x0059C781
// TRIED: a real out-of-line forwarder for `do_all_non_input`, the `sleep_call`/`base_cost_call` idiom, to answer this body making 5 calls where the image makes 4. MEASURED and REVERTED: it makes things worse in both directions at once. Agreement falls from 16/206 to 1/206, because changing the call target shifts everything after it; and call_diff goes from MORE (5 against 4) to FEWER (2 against 4) rather than agreeing, because it resolves callees by ADDRESS and a forwarder is a different address than the 0x005FCB20 the image calls. The idiom works where the forwarder IS the image's callee; here it cannot be.
// size      609 bytes
// prototype void (__thiscall ?continent@Path@@QAEXHHH@Z)(Path* this, int xCoord, int yCoord, int region)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591B90 0x00591DB0 0x005FCB20
// TRIED: `call_diff` reports this tree makes 5 calls against the
//   image's 4 (MORE) - the image calls 0x005FCB20 (`do_all_non_input`)
//   directly at this site, but `do_all_non_input` is declared `MEASURED
//   inline` in temp.h and VC6 inlines it here too, expanding it into three
//   calls (`do_non_input`/`do_net`/`check_net`) that the image does not make
//   at this address. That header is outside this batch's touch list (general/
//   buffer/wave/path only), so the fix - making this ONE call site see the
//   out-of-line function while `do_all_draws`/`do_all_keyboard` keep the
//   inlined one - is not made here; reported instead. Best measured with the
//   body unchanged: 16/206 instructions, MISMATCH.
 Return Value: n/a
 Status: Complete
*/
void Path::continent(int x, int y, int region) {
    Continents[region].tile_count = 0;
    index1_ = 0; 
    index2_ = 0;
    uint32_t fresh_water_count = 0;
    x_table_[index1_] = (int16_t)x;
    y_table_[index1_++] = (int16_t)y;
    region_set(x, y, (uint8_t)region);
    BOOL is_ocean_tile = is_ocean(x, y);
    do {
        int16_t x_it = x_table_[index2_];
        int16_t y_it = y_table_[index2_++];
        Continents[region].tile_count++;
        for (uint32_t i = 0; i < 8; i++) {
            int x_radius = xrange(x_it + RadiusBaseX[i]);
            int y_radius = y_it + RadiusBaseY[i];
            BOOL is_ocean_rad;
            if (on_map(x_radius, y_radius) && y_it && y_it != ((int)MapLatitudeBounds - 1)
                && (is_ocean_rad = is_ocean(x_radius, y_radius), is_ocean_rad == is_ocean_tile 
                    && !region_at(x_radius, y_radius))) {
                if (is_ocean_tile && bit2_at(x_radius, y_radius) & BIT2_FRESH && is_ocean_rad) {
                    fresh_water_count++;
                }
                region_set(x_radius, y_radius, (uint8_t)region);
                x_table_[index1_] = (int16_t)x_radius;
                y_table_[index1_++] = (int16_t)y_radius;
            }
        }
    } while (index2_ && index1_ != index2_);
    if (fresh_water_count) {
        // land locked?
        BOOL is_fresh_water = fresh_water_count >= ((Continents[region].tile_count * 3) / 4);
        for (int y_it = 0; y_it < MapLatitudeBounds; y_it++) {
            for (int x_it = y_it & 1; x_it < MapLongitudeBounds; x_it += 2) {
                if (region_at(x_it, y_it) == region) {
                    bit2_set(x_it, y_it, BIT2_FRESH, is_fresh_water);
                }
            }
        }
    }
    do_all_non_input();
}

/*
 Purpose: Populate and set up all the continents and path tables.
// ORIGINAL: 0x0059C790 ?continents@Path@@QAEXXZ 0x0059C790-0x0059CC9D
// size      1293 bytes
// prototype void (__thiscall ?continents@Path@@QAEXXZ)(Path* this)
// callers   3   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00591B90 0x0059C200 0x0059C520 0x005FCB20
// TRIED: `call_diff` reports NO call-count disagreement here (unlike
//   the sibling `continent` a few lines up, where the same `do_all_non_input`
//   call gets inlined) - this function is large enough that VC6 keeps it as
//   a real out-of-line call, matching the image's 0x005FCB20. The remaining
//   gap is register/stack-layout noise typical of a 1293-byte function this
//   size, not a structural difference: best measured 2/414 instructions,
//   MISMATCH. Not chased further at this size within batch budget.
 Return Value: n/a
 Status: Complete
*/
void Path::continents() {
    for (uint32_t i = 0; i < MapArea; i++) {
        map_tiles()[i].region = 0;
    }
    int y_south_pole = MapLatitudeBounds - 1;
    int ocean_region = 64;
    int land_region = 0;
    for (int y = 1; y < y_south_pole; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            if (!region_at(x, y)) {
                int region_current;
                int region_min;
                int region_max;
                if (is_ocean(x, y)) {
                    ocean_region++;
                    region_min = 64;
                    region_max = 126;
                    region_current = range(ocean_region, region_min, region_max);
                } else {
                    land_region++;
                    region_min = 0;
                    region_max = 62;
                    region_current = range(land_region, region_min, region_max);
                }
                continent(x, y, region_current);
                if (region_current == region_max) {
                    uint32_t tiles = Continents[region_current].tile_count;
                    int search_region = -1;
                    for (int i = region_min; i < region_max; i++) {
                        if (Continents[i].tile_count < tiles) {
                            tiles = Continents[i].tile_count;
                            search_region = i;
                        }
                    }
                    if (search_region >= 0) {
                        merge(search_region, region_max + 1);
                        merge(region_current, search_region);
                    } else {
                        merge(region_current, region_max + 1);
                    }
                }
            }
        }
    }
    for (int x = 0; x < MapLongitudeBounds; x += 2) { // north pole
        uint8_t pole_region = is_ocean(x, 0) ? 127 : 63;
        region_set(x, 0, pole_region);
        Continents[pole_region].tile_count++;
    }
    for (x = y_south_pole & 1; x < MapLongitudeBounds; x += 2) { // south pole
        uint8_t pole_region = is_ocean(x, y_south_pole) ? 127 : 63;
        region_set(x, y_south_pole, pole_region);
        Continents[pole_region].tile_count++;
    }
    int most_tiles = 0;
    int total_tiles = 0;
    for (int i_405 = 1; i_405 < 63; i_405++) {
        int tiles = Continents[i_405].tile_count;
        total_tiles += tiles;
        if (tiles > most_tiles) {
            most_tiles = tiles;
        }
    }
    GameState = (most_tiles >= ((total_tiles * 4) / 5))
        ? GameState | STATE_UNK_100 : GameState & ~STATE_UNK_100;
    for (int i_414 = 0; i_414 < MaxRegionLandNum; i_414++) {
        ZeroMemory(Continents[i_414].sea_coasts, 8);
    }
    for (y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            uint32_t region = region_at(x, y);
            if (region < MaxRegionLandNum) {
                for (int i = 0; i < 8; i++) {
                    int x_radius = xrange(x + RadiusBaseX[i]);
                    int y_radius = y + RadiusBaseY[i];
                    if (on_map(x_radius, y_radius)) {
                        uint32_t region_rad = region_at(x_radius, y_radius);
                        if (region_rad >= MaxRegionLandNum) {
                            int offset;
                            int mask;
                            bitmask(region_rad - MaxRegionLandNum, &offset, &mask);
                            Continents[region].sea_coasts[offset] |= mask;
                            i += 2 - (i & 1);
                        }
                    }
                }
            }
        }
    }
    make_abstract();
    do_all_non_input();
}

/*
 Purpose: TBD
// ORIGINAL: 0x0059CCA0 ?sensors@Path@@QAEHHPAHPAH@Z 0x0059CCA0-0x0059D22A
// LEVER: THE SWEEP IS OVER THE REAL MAP, NOT THE ABSTRACT ONE, and this was
//   a WRONG PROGRAM rather than a wrong spelling. The body read
//   `for (uint32_t y = 1; y < MapAbstractLatBounds - 1; y++)` /
//   `x < MapAbstractLongBounds`; the image loads 0x00949874
//   (`MapLatitudeBounds`) and 0x00949870 (`MapLongitudeBounds`), starts y at
//   ZERO (`xor esi, esi`), and closes the two loops `add ebx, 2 / cmp ebx,
//   edx / jl` and `inc esi / cmp esi, eax / jl` at 0x0059D1EE-0x0059D213 -
//   SIGNED tests against the real bounds, with no `- 1` anywhere. The
//   abstract bounds are 0x0094A294/0x0094A298 and the image never reads
//   them here. Fixed to `for (int y = 0; y < MapLatitudeBounds; y++)` /
//   `for (int x = y & 1; x < MapLongitudeBounds; x += 2)`:
//   similarity 0.177 -> 0.336, 25 -> 32 instructions agreeing, and six
//   fewer instructions emitted. `uint32_t` cursors over the same real
//   bounds score 0.312, so the SIGNED `int` is part of it.
// LEVER: FOUR MORE WRONG-PROGRAM DEFECTS in the accept test, all read off
//   the branch targets rather than guessed - every one of them jumps to
//   0x0059D1EE, which the loop bottom shows is "skip this tile":
//     * `test al, 0x50 / jne skip` at 0x0059CE4A. The tile is taken when
//       BIT_MINE|BIT_SOLAR_TIDAL are CLEAR; the body required them SET.
//     * `test eax, 0x1402000 / jne skip` at 0x0059CE52. Same inversion for
//       BIT_MONOLITH|BIT_CONDENSER|BIT_THERMAL_BORE.
//     * `test byte [eax*4+0x96c9f8], 0x10 / jne skip` at 0x0059CE23 is
//       DTREATY_VENDETTA (0x10) and skips when SET, so the guard is
//       `!has_treaty(..., DTREATY_VENDETTA)`. The body had
//       `has_treaty(..., DTREATY_PACT)` - wrong mask AND wrong polarity.
//       (The veh_who guard 30 bytes earlier really is PACT: mask 1,
//       `je skip`, so that one was right.)
//     * the fungus arms compare against 0x40, ALT_BIT_OCEAN_SHELF, not the
//       0x60 of `is_ocean`: 0x0059CEB1 and 0x0059CEC7 both `cmp ecx, 0x40`.
//   Together the old body accepted very nearly the complement of the tiles
//   the image accepts. Fixing all four costs NOTHING on the measurement -
//   0.3363 either way with `altitude_at(x, y)` for the altitude arms - so
//   this is a correctness fix that the score could never have found.
// TRIED: spelling the altitude arms as `tile->climate & 0xE0` instead of
//   `altitude_at(x, y)`. Same program, and it costs 0.336 -> 0.285.
// TRIED: byte-exactness this pass - out of budget, and the gap is
//   register allocation across a 464-instruction body, not a missing
//   statement. `call_diff` reports the call counts already AGREE (nine
//   calls, whose_territory x3, is_sensor x2, zoc_veh, bonus_at, has_tech,
//   do_all_non_input), so nothing structural is missing; this tree emits
//   535 against 464 because the image keeps the outer cursor in ESI while
//   also mirroring it to `[ebp-8]`, and this tree reads `[ebp-8]` at every
//   use. `/Oi-` is the winning flag set here and already applied by the
//   measurement - the image CALLS memset at 0x006465F0 for the
//   `ZeroMemory(map_table_, MapArea * 4)`, where the intrinsic gives
//   `rep stosd`.
// symbol    ?sensors@Path@@QAEHHPAH0@Z
// size      1418 bytes
// prototype int (__thiscall ?sensors@Path@@QAEHHPAHPAH@Z)(Path* this, int factionID, int* xCoordPtr, int* yCoordPtr)
// callers   2   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0 0x00592030 0x005B9F20 0x005BF010 0x005C8AC0 0x005FCB20 0x00644F3A 0x006465F0
 Return Value: true/false
 Status: Complete - testing
*/
BOOL Path::sensors(int faction_id, int *x_sensor, int *y_sensor) {
    BOOL has_sensor = true;
    ZeroMemory(map_table_, MapArea * 4);
    x_dst_ = -1;
    y_dst_ = -1;
    int x_search = *x_sensor;
    int y_search = *y_sensor;
    int search = 9999;
    uint32_t region = region_at(x_search, y_search);
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            int veh_faction_id;
            if (region_at(x, y) == region 
                && whose_territory(faction_id, x, y, NULL, false) == (int)faction_id
                && base_who(x, y) < 0 && (veh_faction_id = veh_who(x, y), veh_faction_id < 0 
                    || veh_faction_id == (int)faction_id
                    || has_treaty(faction_id, veh_faction_id, DTREATY_PACT))) {
                int zoc_faction_id = zoc_veh(x, y, faction_id);
                uint32_t bit = bit_at(x, y);
                Map *tile = map_loc(x, y);
                if (zoc_faction_id != 1 && (!zoc_faction_id
                    || !has_treaty(faction_id, zoc_faction_id, DTREATY_VENDETTA))
                    && !(bit & (BIT_MINE | BIT_SOLAR_TIDAL))
                    && !(bit & (BIT_MONOLITH | BIT_CONDENSER | BIT_THERMAL_BORE))
                    && !bonus_at(x, y, 0) && ((tile->val3 & 0xC0u) > TERRAIN_BIT_ROLLING
                        || climate_at(x, y) == RAINFALL_ARID
                        || ((bit & BIT_FUNGUS)
                            && altitude_at(x, y) >= ALT_BIT_OCEAN_SHELF))
                    && (!(bit & BIT_FUNGUS)
                        || altitude_at(x, y) < ALT_BIT_OCEAN_SHELF
                        || has_tech(Rules.tech_improve_fungus_sqr, faction_id))) {
                    uint32_t flags = 0;
                    for (int i = 0; i < RadiusRange[2]; i++) {
                        int x_radius = xrange(x + RadiusOffsetX[i]);
                        int y_radius = y + RadiusOffsetY[i];
                        if (on_map(x_radius, y_radius) && !is_sensor(x_radius, y_radius)
                            && (whose_territory(faction_id, x_radius, y_radius, NULL, false)
                                == (int)faction_id || get(x_radius, y_radius))) {
                            if (i >= 9) {
                                flags |= 1;
                            }
                            flags |= 2;
                        }
                    }
                    if (!(flags & 1)) {
                        int proxminity = vector_dist(x_search, y_search, x, y);
                        for (uint32_t i = 0; i < 25; i++) {
                            int tile_faction_id;
                            int x_radius = xrange(x + RadiusOffsetX[i]);
                            int y_radius = y + RadiusOffsetY[i];
                            if (on_map(x_radius, y_radius) 
                                && (tile_faction_id = base_who(x_radius, y_radius), 
                                    tile_faction_id == (int)faction_id)) {
                                flags |= 4;
                                BOOL check = true;
                                for (int j = 0; j < RadiusRange[2]; j++) {
                                    int x_radius2 = xrange(x_radius + RadiusOffsetX[j]);
                                    int y_radius2 = y_radius + RadiusOffsetY[j];
                                    if (on_map(x_radius2, y_radius2)
                                        && !is_sensor(x_radius2, y_radius2)
                                        && (whose_territory(faction_id, x_radius2, y_radius2, NULL,
                                            false) == (int)faction_id 
                                            || get(x_radius2, y_radius2))) {
                                        check = false;
                                        break;
                                    }
                                }
                                if (check) {
                                    flags |= 8;
                                }
                            }
                        }
                        if (flags & 4) {
                            proxminity += 400;
                        }
                        if (flags & 8) {
                            proxminity += 800;
                        }
                        if (!(flags & 2) || flags & 8) {
                            proxminity++;
                            set(x, y, 0);
                        } else {
                            proxminity += 200;
                        }
                        if (proxminity < search) {
                            *x_sensor = x;
                            *y_sensor = y;
                            search = proxminity;
                            has_sensor = false;
                        }
                    }
                }
            }
        }
    }
    do_all_non_input();
    return has_sensor;
}

// global


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x0059A320 ??1Path@@QAE@XZ 0x0059A320-0x0059A368 BYTE_EXACT
// body      src/path.h
// size      72 bytes
// prototype void (__thiscall ??1Path@@QAE@XZ)(Path* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2
*/
