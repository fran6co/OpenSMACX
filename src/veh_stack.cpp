/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// Vehicle visibility and stack functions, 0x005B5700-0x005B9BB8 in the image.
// Split from veh.cpp by address order: the image calls veh_cargo/veh_lift/
// veh_drop from here out of line while kill inlines veh_cargo.
#include "stdafx.h"
#include "general.h"
#include "veh.h"
#include "map.h"
#include "game.h"
#include "log.h"
#include "faction.h"
#include "base.h"
#include "alpha.h"
#include "console.h"
#include "interlude.h"
#include "worldwin.h"

/*
Purpose: Reveal the specified tile to the specified faction and keep the
         faction's remembered copy of the tile's terrain bits in step with it.
// ORIGINAL: 0x005B5700 sub_5b5700 0x005B5700-0x005B57CA BYTE_EXACT
// LEVER: Historical synch_bit_call spelling retained a real call. The image
//        CALLS 0x591E50 as this body's one genuine call, and at plain /O2 the
//        optimiser open-coded the `MEASURED inline` from map.h into the tail
//        (3/66 at /O2 /Oy- before the switch), which is also where the extra register
//        pressure came from. on_map() and map_loc() stay written out: the
//        image folds both in here.
// LEVER: FEWER (0 calls vs image's 1: synch_bit) at every flag set that kept a standalone symbol. /Ob0 forces the WHOLE function real, including on_map()/map_loc(), which the image folds in here; without /Ob0 the compiler inlines synch_bit() too, because the former map.h definition was `MEASURED inline`. Writing on_map()/map_loc() as bare expressions (not calls) removes them from the optimiser's inline/no-inline choice entirely, so /Ob0 only has synch_bit() left to force real.
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
game state. `bit2 |= 0x400000` and `WorldClimateWorldWin.field_239BC_ |= 1` are the same pair
climate_set() sets at 00591A80 (WorldWin::on_redraw consumes that bit).
*/

void __cdecl spot_tile(int x, int y, int faction_id) {
    // on_map() is a real call: where spot_all expands this body it keeps
    // on_map out of line (0x5B6D2C, 0x5B7200, 0x5B750A, ...).
    if (!on_map(x, y)) {
        return;
    }
    Map *tile = map_loc(x, y);
    if (faction_id == ConsoleGlobal.faction_id_ && !(tile->visibility & (1 << faction_id))
        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
        && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
        tile->bit2 |= 0x400000; // TODO: identify value
        WorldClimateWorldWin.field_239BC_ |= 1;
    }
    tile->visibility |= (uint8_t)(1 << faction_id);
    synch_bit(x, y, faction_id);
}


/*
Purpose: Reveal the specified base, and the tile it stands on, to the specified
         faction.
// ORIGINAL: 0x005B57D0 ?spot_base@@YAXHH@Z 0x005B57D0-0x005B58D2 BYTE_EXACT
// LEVER: Historical synch_bit_call spelling retained a real call. The
//        image CALLS 0x591E50 as this body's one genuine call, and open-coding
//        the `MEASURED inline` from map.h here cost the frame its spare
//        registers (5/78 at /O2 /Oy- before the switch). spot_tile() is now a
//        direct same-TU call that inlines; with the hand copy gone spot_loc
//        inlines this function as the image does.
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
    spot_tile(Bases[base_id].x, Bases[base_id].y, faction_id);
}


/*
Purpose: Reveal the whole of the specified unit's stack, and the tile it stands
         on, to the specified faction.
// ORIGINAL: 0x005B58E0 ?spot_stack@@YAXHH@Z 0x005B58E0-0x005B5A61 BYTE_EXACT
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

// LEVER: on_map(), spot_tile() and veh_top() are plain same-TU calls that
//        /Ob2 inlines once each; the earlier hand-copied bodies (and the
//        written-out on_map compares) were the mismatch.
*/
void __cdecl spot_stack(int veh_id, int faction_id) {
    if (veh_id >= 0 && on_map(Vehs[veh_id].x, Vehs[veh_id].y)) {
        spot_tile(Vehs[veh_id].x, Vehs[veh_id].y, faction_id);
    }
    for (int id = veh_top(veh_id); id >= 0; id = Vehs[id].next_veh_id_stack) {
        Vehs[id].visibility |= (uint8_t)(1 << faction_id);
        if (faction_id) {
            Vehs[id].flags &= ~(VFLAG_LURKER | VFLAG_INVISIBLE);
        }
    }
}


/*
Purpose: Hide a whole stack from the faction that spotted it.
// ORIGINAL: 0x005B5A70 ?unspot_stack@@YAXH@Z 0x005B5A70-0x005B5AC5 BYTE_EXACT
// size      85 bytes
// prototype void (__cdecl ?unspot_stack@@YAXH@Z)(int vehID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// HOMED from src/recovered/005b5a70.cpp, 2026-09-04. The archive's
// `g_0095284f[a1 * 0x34] = 0` is Vehs[id].visibility (+0x27) and its two short
// walks are Vehs[].prev_veh_id_stack (+0x32) and Vehs[].next_veh_id_stack
// (+0x30) - the same climb-and-descend spot_stack does.
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl unspot_stack(int veh_id) {
    if (veh_id >= 0) {
        short next = Vehs[veh_id].prev_veh_id_stack;
        while (next >= 0) {
            veh_id = next;
            next = Vehs[veh_id].prev_veh_id_stack;
        }
        for (; veh_id >= 0; veh_id = Vehs[veh_id].next_veh_id_stack) {
            Vehs[veh_id].visibility = 0;
        }
    }
}


/*
Purpose: Reveal whatever occupies the specified location - the tile itself, and
         then either the base standing on it or the whole unit stack - to the
         specified faction.
// ORIGINAL: 0x005B5AD0 ?spot_loc@@YAXHHH@Z 0x005B5AD0-0x005B5E05 BYTE_EXACT
// LEVER: once spot_base/spot_stack call spot_tile/veh_top instead of
//   hand copies, /Ob2 inlines all three here as the image does (0.51 ->
//   0.998). Left: the inlined veh_top climb swaps eax/ecx for top and the
//   induction variable; a named veh_at local or an else arm did not change it.
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
Purpose: Clear a vehicle's move state at the end of its stack walk.
// ORIGINAL: 0x005B5E10 sub_5b5e10 0x005B5E10-0x005B5EA0 BYTE_EXACT
// size      144 bytes
// prototype void (__cdecl sub_5b5e10)(int vehID)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// symbol    ?veh_move_end@@YAXH@Z
//           The image's catalogue name is an uninformative `sub_`; this tree
//           names what the body does. Renames update this fact, not the bytes.
// SUPERSEDED OFFSET INTERPRETATION (retained seed history, not original behavior):
// The following HOMED explanation mistook the state address for the Veh array base.
// Its raw +0xD/+0xE/+0xF implementation consequently accessed the wrong fields.
// HOMED from src/unrecovered/005b5e10.cpp, 2026-09-04. Its `g_95282c` byte
// view is Vehs[]: +0x04 state, +0x0D nothing in the tree's Veh (see the note
// below), +0x0E faction_id, +0x0F year_end_lurking, +0x11 order.
// Former explanation: BUG IN THE ORIGINAL: the two bytes the body tests against each other,
// 0x95282c + veh_id * 0x34 + 0xD and + 0xE, are `unk_1` and `faction_id` in
// the tree's Veh - the low half of `flags` and the state dword's high byte, so
// `+ 0xD` is one byte *below* the field the comparison reads. The arithmetic
// is reproduced exactly as shipped; a reader who "fixes" the offset changes
// the program.
// Pinned 5B5E35/5B5E3C compare 95283B/95283A, and 5B5E84/5B5E8F
// read/clear 952839. Vehs begins 952828, not 95282C: these are +13/+12/+11,
// the existing patrol_current_point, waypoint_count and order members.
// Preserve the unchecked index, unsigned counter comparison and state masks.
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl veh_move_end(int veh_id) {
    if ((Vehs[veh_id].state & 0x2000200) == 0x2000200 &&
        Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count) {
        Vehs[veh_id].state &= 0xfdfffdff;
    }
    if ((Vehs[veh_id].state & 0x1000200) == 0x1000200) {
        Vehs[veh_id].state &= 0xfefffdff;
    }
    Vehs[veh_id].state &= 0xffffbfff;
    if (Vehs[veh_id].order == 0x18) {
        Vehs[veh_id].order = 0;
    }
}


/*
Purpose: Determine if the specified unit wants to wake up based on certain conditions and
         preferences. Optional parameter for spotted veh_id (-1 to skip).
// ORIGINAL: 0x005B5EA0 ?want_to_wake@@YAHHHH@Z 0x005B5EA0-0x005B6060 BYTE_EXACT
// TRIED: image never reads Vehs[veh_id].faction_id eagerly - it re-reads it at each conditional use, and never caches it across the early return. Un-caching it into `Vehs[veh_id].faction_id` at every site (instead of a hoisted `veh_faction_id` local) let the compiler pick a plain ebp-frame prologue matching the image's, where the hoisted local forced a param-less-locals, esp-relative frame from instruction 0. Also restored the `veh_id >= 0 &&` half of the trailing guard - the image's `test edi, edi; jl` tests veh_id, not just spotted_veh_id, contradicting this body's own comment that it was safe to drop. Residual divergence past this is register allocation (esi/edi/ebx/ebp swapped throughout) and one 16-bit shift-then-extend vs extend-then-shift on `waypoint_x[0] >> 1`; an explicit `(int)` cast on the base_who() arguments did not change the codegen. Best flag set /O2 /Gy /GR- /Oy- /GX, 0.793 similar.
// TRIED 2026-09-03: `uint8_t triad = get_triad(veh_id);` (the field's own
//        width, against the usual width lever) - the image keeps the byte in
//        `al` and compares `cmp al, 2`, but the narrower local measured WORSE:
//        0.790 against 0.793, because the byte local forces a second
//        zero-extension at the `triad != triad_spotted` compare further down.
//        Reverted.
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
    uint8_t triad = get_triad(veh_id);
    if (triad == TRIAD_AIR && Vehs[veh_id].terraforming_turns
        && Vehs[veh_id].order == ORDER_MOVE_TO) {
        int base_faction_id = base_who(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
        if (base_faction_id >= 0
            && (((uint32_t)base_faction_id == Vehs[veh_id].faction_id)
                || (PlayersData[Vehs[veh_id].faction_id].diplo_treaties[base_faction_id] & DTREATY_PACT))) {
            return false;
        }
    }
    BOOL wants_to_wake;
    uint32_t preferences;
    const uint32_t treaty =
        PlayersData[Vehs[veh_id].faction_id].diplo_treaties[faction_id];
    if (treaty & DTREATY_PACT) {
        if (IsMultiplayerNet) {
            wants_to_wake = false;
            goto check_spotted;
        }
        preferences = GameDataStateStorage.fields_.game_preferences_;
        wants_to_wake = preferences & PREF_AUTO_END_MOVE_SPOT_VEH_PACT;
    } else if (treaty & DTREATY_TREATY) {
        if (IsMultiplayerNet) {
            wants_to_wake = true;
            goto check_spotted;
        }
        preferences = GameDataStateStorage.fields_.game_preferences_;
        wants_to_wake = preferences & PREF_AUTO_END_MOVE_SPOT_VEH_TREATY;
    } else if (treaty & DTREATY_TRUCE) {
        if (IsMultiplayerNet) {
            wants_to_wake = true;
            goto check_spotted;
        }
        preferences = GameDataStateStorage.fields_.game_preferences_;
        wants_to_wake = preferences & PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE;
    } else {
        if (IsMultiplayerNet) {
            wants_to_wake = true;
            goto check_spotted;
        }
        preferences = GameDataStateStorage.fields_.game_preferences_;
        wants_to_wake = preferences & PREF_AUTO_END_MOVE_SPOT_VEH_WAR;
    }
    if (!(preferences & PREF_AUTO_DONT_END_MOVE_DIFF_TRIAD)) {
        return wants_to_wake;
    }
check_spotted:
    if (veh_id >= 0 && spotted_veh_id >= 0) {
        uint8_t triad_spotted = get_triad(spotted_veh_id);
        if (triad != triad_spotted && ((Vehs[veh_id].state & (VSTATE_UNK_1000000 | VSTATE_UNK_200))
            != (VSTATE_UNK_1000000 | VSTATE_UNK_200) || triad_spotted == TRIAD_LAND)) {
            wants_to_wake = false;
        }
    }
    return wants_to_wake;
}

// The original keeps one tile across these checks. Use the same owner
// operation as base_who with the cached map inputs, allowing the compiler
// to reuse the existing tile address. This local factoring does not claim
// an original helper name or an additional original function.
static inline int wake_stack_base_owner(Map *tile) {
    if (tile->bit & BIT_BASE_IN_TILE) {
        int owner = tile->val2 & 0xF;
        if (owner < MaxPlayerNum) {
            return owner;
        }
    }
    return -1;
}


/*
Purpose: Wake up units meeting specific conditions from the stack where specified unit is located.
// ORIGINAL: 0x005B6060 ?wake_stack@@YAXH@Z 0x005B6060-0x005B619A BYTE_EXACT
// LEVER: veh_top() is called at only 2 real sites in the image (see its own
//        comment on veh_top); here it is inlined, matching veh_at above. The
//        climb's induction variable is int16_t (prev_veh_id_stack's own type),
//        matching veh_top's own lever. is_ocean()/base_who()/bit_at() are hand-
//        inlined sharing ONE tile pointer, and map_loc()'s two globals
//        (map_tiles(), MapLongitude) are hoisted out of the `i` loop into locals
//        BEFORE it - the image reads them once, kept live in registers across
//        every iteration, rather than re-reading them on each tile lookup a
//        plain per-iteration map_loc(x, y) call would. Together: 0.836 -> 0.955.
// Exact: nested ifs with one Map* from the hoisted globals; the airbase
// test respells the tile so VC6 rereads tile->bit as the image does.
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
    Map *const map_base = map_tiles();
    const int map_width = MapLongitude;
    for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
        if (Vehs[i].order == ORDER_SENTRY_BOARD) {
            if (get_triad(i) == TRIAD_LAND) {
                Map *tile = map_base + ((Vehs[i].x >> 1) + Vehs[i].y * map_width);
                if ((tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
                    continue;
                }
                int veh_id_way_point = Vehs[i].waypoint_x[0];
                if (veh_id_way_point >= 0 && get_triad(veh_id_way_point) == TRIAD_AIR
                    && wake_stack_base_owner(tile) < 0
                    && !(map_base[(Vehs[i].x >> 1) + Vehs[i].y * map_width].bit & BIT_AIRBASE)) {
                    continue;
                }
            }
            Vehs[i].order = ORDER_NONE;
            Vehs[i].state &= ~VSTATE_EXPLORE;
        }
    }
}


/*
Purpose: Can this vehicle leave the stack on its own?
// ORIGINAL: 0x005B61A0 sub_5b61a0 0x005B61A0-0x005B6259 BYTE_EXACT
// size      185 bytes
// prototype int (__cdecl sub_5B61A0)(int vehID)
// symbol    ?veh_can_leave_stack@@YAHH@Z
//           The image's catalogue name is an uninformative `sub_`; this tree
//           names what the body does. Renames update this fact, not the bytes.
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0
// HOMED from src/unrecovered/005b61a0.cpp, 2026-09-04.
Return Value: 0 no, 1 yes by ability or order, 2 yes - sea/peach tile.
Status: MISMATCH.
*/
// WORKAROUND(inline): spot_all expands this body at all three of its sites
// (0x5B62D3, 0x5B7785, 0x5B81AB) while 0x5B61A0 keeps the standalone copy;
// as a plain function our spot_all calls it instead.
MEASURED inline int __cdecl veh_can_leave_stack(int veh_id) {
    if (has_abil(Vehs[veh_id].proto_id, 0x40) != 0) {
        return 1;
    }
    if (has_abil(Vehs[veh_id].proto_id, 4) != 0) {
        return 1;
    }
    if (static_cast<int16_t>(Vehs[veh_id].flags & 0x400) != 0) {
        return 1;
    }
    const Map *tile = map_loc(Vehs[veh_id].x, Vehs[veh_id].y);
    if (tile->bit & 0x20u) {
        if ((tile->climate & ~0x1F) >= 0x40 && Vehs[veh_id].proto_id != 0x13) {
            return 2;
        }
    }
    return 0;
}



// WORKAROUND(noinline): spot_stack as spot_all expands it at its first three
// sites, where the image keeps spot_tile a real call (0x5B6E6E, 0x5B6FF5,
// 0x5B730F) and inlines veh_top; the later sites inline spot_tile instead.
inline void __cdecl spot_stack_tile_call(int veh_id, int faction_id) {
    void (__cdecl *const tile_fn)(int, int, int) = spot_tile;
    if (veh_id >= 0 && on_map(Vehs[veh_id].x, Vehs[veh_id].y)) {
        tile_fn(Vehs[veh_id].x, Vehs[veh_id].y, faction_id);
    }
    for (int id = veh_top(veh_id); id >= 0; id = Vehs[id].next_veh_id_stack) {
        Vehs[id].visibility |= (uint8_t)(1 << faction_id);
        if (faction_id) {
            Vehs[id].flags &= ~(VFLAG_LURKER | VFLAG_INVISIBLE);
        }
    }
}


// Transcribed from the image in its block order (2026-11-12): the faction's
// pre-scan of the 49/25 sensor ring, the 25/9 visibility ring, the outer
// ring's edge reveal, the stack's own spot_stack pass, then two separate
// contact loops (offsets 0..8 and 9..24) and the presentation tail. The
// remaining distance is mostly /Ob2 decisions inside the spot_stack and
// spot_base expansions (spot_tile and veh_top called or expanded per site).
// ORIGINAL: 0x005B6260 ?spot_all@@YAXHH@Z 0x005B6260-0x005B8AE2
// RECOVERY_BINDING: v1 {"evidence": ["task:vehicle-visibility-owner", "pinned:5B6260..5B8AE2 complete sensing, terrain memory, contact/wake/goals and presentation span; original prototype and seed findings preserved.", "pinned:70F5 own Base joins common73E1 tile/stack path;7858 contact visibility differs from824E selected-source visibility.", "pinned:76FB..7707 target fallback contactLow || !sourceLow;7A69..7A7B source fallback sourceLow || !contactLow || triad2; distinct lock/callback routes.", "pinned:8A43 fixed WorldClimateWorldWin receiver8E9F60 and8A48 direct GraphicWin::redraw5D5A70 before interlude/encounters; source:worldwin.h owns the receiver.", "historical seed:src/unrecovered/005b6260.cpp native mapping and unsuccessful mechanical transliteration retained as source evidence."], "reason": "Recover the complete original visibility operation with genuine vehicle, faction, base, map and climate-window owners; retain original identity/spans and seed findings without image calls or raw self-offset models."}
// TRIED: mechanical register/stack-slot goto transliteration (234 labels, 5638-line body): compiles, MISMATCH sim up to 0.73 on /O2 frameless. Superseded by the 2026-11-12 block-order transcription.
// TRIED: spot_tile spellings (if-block, mask temp, no tile local) and writing the coordinate wrap out instead of wrap_map_x: none matches the image's per-site inline pattern; the hand-written wrap doubled the diff.
void __cdecl spot_all(int veh_id, int toggle) {
    int count = 0;
    int enc_a = 0;
    int enc_b = 0;
    int i;
    int x;
    int y;
    if (!IsMultiplayerNet) {
        toggle = 1;
    }
    int y0 = Vehs[veh_id].y;
    int x0 = Vehs[veh_id].x;
    int triad = get_proto_triad(Vehs[veh_id].proto_id);
    int leave = veh_can_leave_stack(veh_id);
    int faction_id = Vehs[veh_id].faction_id;
    int faction_mask = 1 << faction_id;
    int sensor = has_abil(Vehs[veh_id].proto_id, ABL_DEEP_RADAR)
        || (is_sensor(x0, y0) && whose_territory(faction_id, x0, y0, 0, 0) == faction_id);
    int low_terrain = is_ocean(x0, y0);
    int encounter_mask = 0;
    int radius_tier = 0;
    if (!faction_id) {
        Vehs[veh_id].visibility &= map_loc(x0, y0)->visibility;
    }
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 1)
        && faction_id == ConsoleGlobal.faction_id_
        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
        for (i = 0; i < (sensor ? 49 : 25); i++) {
            x = wrap_map_x(x0 + RadiusOffsetX[i]);
            y = y0 + RadiusOffsetY[i];
            if (!(y >= 0 && y < (int)MapLatitudeBounds && x >= 0 && x < (int)MapLongitudeBounds)) {
                continue;
            }
            if (i >= (sensor ? 25 : 9) && (map_loc(x, y)->bit2 & 0x200000)) {
                continue;
            }
            int match;
            if (i < (sensor ? 25 : 9)) {
                match = 1;
            } else {
                match = 0;
                for (int j = 0; j < 25; j++) {
                    int x2 = wrap_map_x(x + RadiusOffsetX[j]);
                    int y2 = y + RadiusOffsetY[j];
                    if (!on_map(x2, y2)) {
                        continue;
                    }
                    if (j < 9) {
                        int owner = anything_at(x2, y2);
                        if (owner == faction_id) {
                            match = 1;
                            break;
                        }
                        if (PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT) {
                            for (int v = veh_at(x2, y2); v >= 0; v = Vehs[v].next_veh_id_stack) {
                                if (Vehs[v].faction_id == faction_id) {
                                    match = 1;
                                    break;
                                }
                            }
                            if (match) {
                                break;
                            }
                        }
                    }
                    if (is_sensor(x2, y2)) {
                        if (map_loc(x2, y2)->territory == faction_id) {
                            match = 1;
                            break;
                        }
                        int base_id = base_find(x0, y0);
                        if (base_id != -1) {
                            if ((x_dist(x0, Bases[base_id].x) == 0 || x_dist(x0, Bases[base_id].x) == 2)
                                && (y0 - Bases[base_id].y == 0
                                    || (y0 - Bases[base_id].y >= 0 ? y0 - Bases[base_id].y
                                                                    : Bases[base_id].y - y0) == 2)
                                && has_fac(FAC_GEOSYNC_SURVEY_POD, base_id, 0)) {
                                match = 1;
                                break;
                            }
                        }
                    }
                    if (base_who(x2, y2) == faction_id) {
                        match = 1;
                        break;
                    }
                    int owner = veh_who(x2, y2);
                    if (owner != faction_id && !(PlayersData[owner].diplo_treaties[faction_id] & DTREATY_PACT)) {
                        continue;
                    }
                    for (int v = veh_top(veh_at(x2, y2)); v >= 0; v = Vehs[v].next_veh_id_stack) {
                        if (Vehs[v].faction_id == faction_id && has_abil(Vehs[v].proto_id, ABL_DEEP_RADAR)) {
                            int triad2 = get_proto_triad(Vehs[v].proto_id);
                            if (triad2 == TRIAD_AIR || is_ocean(x, y) == (triad2 == TRIAD_SEA)) {
                                match = 1;
                                break;
                            }
                        }
                    }
                }
            }
            if (map_loc(x, y)->bit2 & 0x200000) {
                if (match) {
                    radius_tier = max(radius_tier, i < 9 ? 1 : (i < 25 ? 2 : 3));
                }
            } else if (!match) {
                radius_tier = max(radius_tier, i < 9 ? 1 : (i < 25 ? 2 : 3));
            }
        }
    }
    for (i = 0; i < (sensor ? 25 : 9); i++) {
        x = wrap_map_x(x0 + RadiusOffsetX[i]);
        y = y0 + RadiusOffsetY[i];
        if (!on_map(x, y)) {
            continue;
        }
        Map *tile = map_loc(x, y);
        if ((tile->bit2 & 0x80002000) == 0x2000) {
            count++;
        }
        if (9 <= i && triad != TRIAD_AIR && is_ocean(x, y) != (triad == TRIAD_SEA)) {
            continue;
        }
        if (faction_id == ConsoleGlobal.faction_id_
            && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
            if (!(tile->visibility & faction_mask)) {
                tile->bit2 |= 0x400000;
                WorldClimateWorldWin.field_239BC_ |= 1;
            }
            if (((tile->bit_visible[faction_id - 1] ^ tile->bit) & 0xFFFFFFFC)
                || !(tile->visibility & faction_mask)) {
                radius_tier = max(radius_tier, i < 9 ? 2 : 3);
            }
        }
        if (faction_id) {
            tile->bit_visible[faction_id - 1] = tile->bit;
        }
        tile->visibility |= faction_mask;
    }
    for (i = sensor ? 25 : 9; i < (sensor ? 49 : 25); i++) {
        x = wrap_map_x(x0 + RadiusOffsetX[i]);
        y = y0 + RadiusOffsetY[i];
        if (!on_map(x, y)) {
            continue;
        }
        Map *tile = map_loc(x, y);
        if (tile->visibility & faction_mask) {
            continue;
        }
        int hidden = 0;
        for (int k = 0; k < 8; k++) {
            int x2 = wrap_map_x(x + VehMoveDeltaX[k]);
            int y2 = y + VehMoveDeltaY[k];
            if (!on_map(x2, y2)) {
                continue;
            }
            if (map_loc(x2, y2)->visibility & faction_mask) {
                continue;
            }
            hidden++;
            if (!VehMoveDeltaX[k] || !VehMoveDeltaY[k]) {
                hidden++;
            }
        }
        int limit = 4;
        if (sensor || is_ocean(x, y)) {
            limit = 6;
        }
        if (hidden >= limit) {
            continue;
        }
        if (faction_id == ConsoleGlobal.faction_id_
            && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
            tile->bit2 |= 0x400000;
            WorldClimateWorldWin.field_239BC_ |= 1;
        }
        if (faction_id) {
            tile->bit_visible[faction_id - 1] = tile->bit;
        }
        tile->visibility |= faction_mask;
        int base_id = base_at(x, y);
        if (base_id >= 0) {
            spot_base(base_id, faction_id);
        }
        radius_tier = max(radius_tier, sensor ? 3 : 2);
    }
    for (int v = veh_top(veh_id); v >= 0; v = Vehs[v].next_veh_id_stack) {
        int owner = Vehs[v].faction_id;
        if (owner != faction_id) {
            spot_stack_tile_call(v, owner);
        }
    }
    // spot_all first contact loop, image 0x5B6EE2 (i = 0) .. 0x5B7CEC (inc/test).
    // Shared names per CONTEXT.md. Block locals:
    //   [ebp-0x30] base_id             base_at(x, y)
    //   [ebp-0x48] contact_low_terrain is_ocean(x, y)   (setl at 0x5B741A)
    //   [ebp-0x2c] stack_id            stack_fix(veh_at(x, y))
    //   [ebp-0x10] stack_faction       Vehs[stack_id].faction_id
    //   esi        sensor_faction      map_loc(x, y)->territory (register only)
    // Compiler temps (no source variable):
    //   [ebp-0x1]  byte (uint8_t)(1 << faction_id) of the inlined spot_base, spilled
    //              across its out-of-line on_map call
    //   [ebp-0x50] inlined spot_stack's faction_id param at 0x5B72DE; later the
    //              PlayersData[faction_id] row CSE (2099*faction_id, then +stack_faction
    //              at 0x5B7B57)
    for (i = 0; i < 9; i++) {
        x = wrap_map_x(x0 + RadiusOffsetX[i]);
        y = y0 + RadiusOffsetY[i];
        if (!on_map(x, y)) {
            continue;
        }
        // 0x5B6F5F
        if (is_sensor(x, y)) {
            int sensor_faction = map_loc(x, y)->territory; // esi; ? may be spelled inline in the condition
            if (sensor_faction > 0 && sensor_faction != faction_id && leave != 1
                && base_who(x, y) < 0) {
                // 0x5B6FC2: spot_stack inlined (spot_tile call 0x5B5700, veh_top inlined;
                // the `if (faction_id)` guard folded away because sensor_faction > 0)
                spot_stack_tile_call(veh_id, sensor_faction);
                // 0x5B7056
                if (!(PlayersData[faction_id].diplo_treaties[sensor_faction] & 7) && toggle) {
                    if (triad == TRIAD_LAND) {
                        add_goal(sensor_faction, AI_GOAL_ATTACK, 2, x, y, -1);
                        add_goal(sensor_faction, AI_GOAL_DEFEND, 1, x, y, -1);
                    } else {
                        add_goal(sensor_faction, AI_GOAL_UNK_1, 2, x, y, -1);
                    }
                }
            }
        }
        // 0x5B70B9
        if (!i) {
            continue;
        }
        int base_id = base_at(x, y); // [ebp-0x30]
        if (base_id >= 0 && Bases[base_id].faction_id_current != faction_id) {
            // 0x5B70FD
            if (triad != TRIAD_AIR
                || !Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range) {
                // ? 0x5B7134: two copies of the or-body (0x5B7162, 0x5B716E) => if/else, not one ||
                if (low_terrain || leave) {
                    if (!(PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current] & 8)) {
                        encounter_mask |= 1 << Bases[base_id].faction_id_current;
                    }
                } else {
                    encounter_mask |= 1 << Bases[base_id].faction_id_current;
                }
            }
            // 0x5B7180
            if (faction_id == ConsoleGlobal.faction_id_
                && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
                if (!(Bases[base_id].visibility & faction_mask)) {
                    radius_tier = max(radius_tier, 2);
                } else if (Bases[base_id].faction_pop_size_intel[faction_id]
                           != Bases[base_id].population_size) {
                    radius_tier = max(radius_tier, 1);
                }
            }
            // 0x5B71CB: spot_base inlined; its spot_tile inlined with on_map left as a call
            spot_base(base_id, faction_id);
            // 0x5B729C
            if (!leave
                || !(PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current] & 8)) {
                // 0x5B72CE: spot_stack inlined (spot_tile call, veh_top inlined)
                spot_stack_tile_call(veh_id, Bases[base_id].faction_id_current);
            }
            // 0x5B737F
            if (triad == TRIAD_LAND
                && !(PlayersData[Bases[base_id].faction_id_current].diplo_treaties[faction_id] & 7)
                && toggle) {
                add_goal(Bases[base_id].faction_id_current, AI_GOAL_ATTACK, 2, x, y, -1);
                add_goal(Bases[base_id].faction_id_current, AI_GOAL_DEFEND, 1, x, y, -1);
            }
        }
        // 0x5B73E1
        int contact_low_terrain = is_ocean(x, y); // [ebp-0x48]
        if (has_abil(Vehs[veh_id].proto_id, ABL_DEEP_PRESSURE_HULL) && !contact_low_terrain) {
            continue;
        }
        // 0x5B7435
        int stack_id = stack_fix(veh_at(x, y)); // [ebp-0x2c]
        if (stack_id < 0) {
            continue;
        }
        int stack_faction = Vehs[stack_id].faction_id; // [ebp-0x10]
        if (stack_faction == faction_id) {
            continue;
        }
        // 0x5B7471
        if (!leave || has_abil(Vehs[stack_id].proto_id, ABL_DEEP_RADAR)
            || !(PlayersData[faction_id].diplo_treaties[stack_faction] & 8)) {
            // 0x5B74B9: spot_stack inlined (veh_id >= 0 / on_map inline, spot_tile inlined
            // with on_map call 0x5B750A, veh_top CALL 0x5B75B4)
            if (base_id < 0) {
                spot_stack(veh_id, stack_faction);
            }
            // 0x5B75F9
            if (!(PlayersData[faction_id].diplo_treaties[stack_faction] & 1) && toggle) {
                wake_stack(stack_id);
                if (is_human(stack_faction)) {
                    // 0x5B764F
                    if (low_terrain == contact_low_terrain
                        && want_to_wake(faction_id, stack_id, veh_id)) {
                        veh_move_end(stack_id); // 0x5B7679 inlined (order tail shared at 0x5B7774)
                    }
                } else if (!(PlayersData[faction_id].diplo_treaties[stack_faction] & 7)
                           && (contact_low_terrain || !low_terrain)) {
                    veh_move_end(stack_id); // 0x5B7709 inlined
                }
            }
        }
        // 0x5B777B: veh_can_leave_stack(stack_id) inlined as a jump chain
        // ? (could be the expanded predicate spelled out; the int16_t flags&0x400 test at
        //   0x5B77A7 is that function's exact spelling)
        if (veh_can_leave_stack(stack_id) && !sensor
            && stack_check(stack_id, 1, -1, -1, -1) == 1
            && (PlayersData[faction_id].diplo_treaties[stack_faction] & 8)) {
            continue;
        }
        // 0x5B7838
        if (base_id < 0) {
            if (faction_id == ConsoleGlobal.faction_id_
                && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
                && !(Vehs[stack_id].visibility & faction_mask)) {
                radius_tier = max(radius_tier, 1);
            }
            // 0x5B7874: spot_stack inlined (stack_id >= 0 test folded; spot_tile inlined with
            // on_map call 0x5B78AC; veh_top CALL 0x5B7950)
            spot_stack(stack_id, faction_id);
        }
        // 0x5B7995: is_human(faction_id) CSE'd with faction_mask
        if (is_human(faction_id)) {
            if ((low_terrain == contact_low_terrain || triad == TRIAD_AIR)
                && want_to_wake(stack_faction, veh_id, stack_id)) {
                veh_move_end(veh_id); // 0x5B79D5 inlined
            }
        } else if (!(PlayersData[faction_id].diplo_treaties[stack_faction] & 7)
                   && (low_terrain || !contact_low_terrain || triad == TRIAD_AIR)) {
            veh_move_end(veh_id); // 0x5B7A7D inlined
        }
        // 0x5B7AF5
        if (contact_low_terrain && stack_faction
            && !(PlayersData[faction_id].diplo_treaties[stack_faction] & 7)) {
            add_goal(faction_id, AI_GOAL_UNK_1,
                     has_abil(Vehs[stack_id].proto_id, ABL_DEEP_PRESSURE_HULL | ABL_CLOAKED) ? 3 : 2,
                     x, y, -1); // ? spelling of the 0x44 mask and of `? 3 : 2` (neg/sbb/neg/add 2)
        }
        // 0x5B7B4E
        if (!(PlayersData[faction_id].diplo_treaties[stack_faction] & 7)) {
            add_goal(faction_id, AI_GOAL_ATTACK, faction_id ? 3 : 1, x, y, -1);
        }
        // 0x5B7B81
        if (VehPrototypes[Vehs[veh_id].proto_id].plan >= PLAN_COLONIZATION
            && !(PlayersData[faction_id].diplo_treaties[stack_faction] & 1)
            && whose_territory(stack_faction, x, y, 0, 0) == stack_faction
            && VehPrototypes[Vehs[stack_id].proto_id].plan >= PLAN_COLONIZATION) {
            add_goal(stack_faction, AI_GOAL_DEFEND, 2, x, y, -1);
            add_goal(stack_faction, AI_GOAL_ATTACK, 1, x, y, -1);
            add_goal(stack_faction, AI_GOAL_SCOUT, 1, x, y, -1);
        }
        // 0x5B7C0E
        if (triad == TRIAD_AIR && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range) {
            continue;
        }
        // 0x5B7C43: proto_id held in dx and compared as 16-bit below
        if (get_proto_triad(Vehs[stack_id].proto_id) == TRIAD_AIR
            && Chassis[VehPrototypes[Vehs[stack_id].proto_id].chassis_id].range) {
            continue;
        }
        // 0x5B7C77
        if (low_terrain != contact_low_terrain && (base_id < 0 || low_terrain)
            && (PlayersData[faction_id].diplo_treaties[stack_faction] & 8)) {
            continue;
        }
        // 0x5B7C99
        if (Vehs[stack_id].proto_id == BSC_FUNGAL_TOWER) {
            enc_a = 1;
        } else if (Vehs[stack_id].proto_id < MaxVehProtoFactionNum // ? 0x40 spelling
                   && (get_proto_offense_rating(Vehs[stack_id].proto_id) < 0
                       || Vehs[stack_id].proto_id == BSC_SPORE_LAUNCHER)) {
            enc_b = 1;
        }
        // 0x5B7CD0
        encounter_mask |= 1 << stack_faction;
    }
    // Second contact loop, 0x5B7CF2..0x5B89FA (i = 9 is set at 0x5B7CF2).
    for (i = 9; i < 25; i++) {
        x = wrap_map_x(x0 + RadiusOffsetX[i]);              // 5B7CFA..5B7D26
        y = y0 + RadiusOffsetY[i];                          // 5B7D28..5B7D37
        if (!on_map(x, y)) {                                // 5B7D3A..5B7D5A
            continue;
        }
        // 5B7D60..5B7F74: a hostile sensor's owner sees the moving stack.
        if (is_sensor(x, y)) {
            int owner = map_loc(x, y)->territory;           // register esi, no slot
            if (owner > 0 && owner != faction_id && leave != 1
                && base_who(x, y) < 0) {                    // 5B7DAD: base_who on the same tile ptr
                spot_stack(veh_id, owner);                  // 5B7DC6..5B7F0C inlined
                if (!(PlayersData[faction_id].diplo_treaties[owner]
                      & (DTREATY_PACT | DTREATY_TREATY | DTREATY_TRUCE))
                    && toggle) {                            // 5B7F0E..5B7F32
                    if (triad == TRIAD_LAND) {              // 5B7F34: [ebp-0x3c]
                        add_goal(owner, AI_GOAL_ATTACK, 2, x, y, -1);
                        add_goal(owner, AI_GOAL_DEFEND, 1, x, y, -1);
                    } else {
                        add_goal(owner, AI_GOAL_UNK_1, 2, x, y, -1);
                    }
                }
            }
        }
        // 5B7F77: [ebp-0x50] is the CSE temp x >> 1 for map_loc(x, y) below.
        int occupant = anything_at(x, y);                   // [ebp-0x10]
        if (occupant == faction_id) {                       // 5B7FB7
            continue;
        }
        int spotted_veh_id = stack_fix(veh_at(x, y));       // [ebp-0x2c]
        int base_id = base_at(x, y);                        // [ebp-0x30] (ebx)
        // 5B7FDB..5B802B (sensor false -> 5B8400, terrain miss -> 5B8403)
        if (sensor && (triad == TRIAD_AIR
                       || is_ocean(x, y) == (triad == TRIAD_SEA))) {
            if (base_id >= 0) {                             // 5B802E
                if (faction_id == ConsoleGlobal.faction_id_
                    && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
                    if (!(Bases[base_id].visibility & faction_mask)) {
                        radius_tier = max(radius_tier, 3);  // 5B805E
                    } else if (Bases[base_id].faction_pop_size_intel[faction_id]
                               != Bases[base_id].population_size) {
                        radius_tier = max(radius_tier, 2);  // 5B8080
                    }
                }
                spot_base(base_id, faction_id);             // 5B808F..5B8180 inlined
            }
            // ? 5B8183/5B818E: separate `if`, not `else`: the base arm falls
            // into the veh test and veh < 0 jumps straight to 5B86A6.
            if (spotted_veh_id >= 0 && base_id < 0) {
                if (!veh_can_leave_stack(spotted_veh_id)) { // 5B8196..5B8233 inlined
                    if (faction_id == ConsoleGlobal.faction_id_
                        && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
                        && !(Vehs[veh_id].visibility & faction_mask)) {   // source unit, not spotted
                        radius_tier = max(radius_tier, 2);  // 5B825C
                    }
                    spot_stack(spotted_veh_id, faction_id); // 5B826B..5B8394 inlined
                    if (is_ocean(x, y) && occupant
                        && !(PlayersData[faction_id].diplo_treaties[occupant] & DTREATY_TREATY)) {
                        add_goal(faction_id, AI_GOAL_UNK_1, 2, x, y, -1);   // 5B83F5
                    }
                }
            }
        }
        // 5B8403: a deep-radar stack sees the moving unit.
        if (spotted_veh_id >= 0 && !leave
            && has_abil(Vehs[spotted_veh_id].proto_id, ABL_DEEP_RADAR)
            // ? 5B843D: one triad load kept in al; two get_triad() calls CSE'd
            && (get_triad(spotted_veh_id) == TRIAD_AIR
                || low_terrain == (get_triad(spotted_veh_id) == TRIAD_SEA))) {
            if (!(PlayersData[faction_id].diplo_treaties[occupant] & DTREATY_PACT)
                && toggle) {
                wake_stack(spotted_veh_id);                 // 5B84A0 (call)
            }
            if (base_who(x0, y0) < 0) {                     // 5B84A8 ([ebp-0x54] = x0 >> 1)
                spot_stack(veh_id, occupant);               // 5B84DE..5B863D inlined
            }
            if (!is_ocean(x, y) && occupant
                && !(PlayersData[occupant].diplo_treaties[faction_id] & DTREATY_TREATY)) {
                add_goal(occupant, AI_GOAL_UNK_1, 2, x0, y0, -1);   // 5B869B
            }
        }
        // 5B86A6: a base's owner sees the moving unit.
        // [ebp-0x30] is reused here as the CSE byte offset of PlayersData[occupant]
        // and [ebp-0x48] as the CSE of its flags & PFLAG_MAP_REVEALED (reread by
        // the inlined spot_tile's dirty arm at 5B8812).
        if (base_id >= 0
            && ((PlayersData[occupant].flags & PFLAG_MAP_REVEALED)
                || (map_loc(x0, y0)->visibility & (1 << occupant)))
            && !leave
            // ? 5B8744: region compared as bytes (cmp dl, al), tile0 on the left
            && (low_terrain != is_ocean(x, y)
                || region_at(x0, y0) == region_at(x, y))) {
            spot_stack(veh_id, occupant);                   // 5B8764..5B88B6 inlined
            if (get_triad(veh_id) == TRIAD_LAND             // 5B88BD: recomputed, not [ebp-0x3c]
                && region_at(x0, y0) == region_at(x, y)     // ? 5B891D byte compare
                && x == PlayersData[occupant].x_target
                && y == PlayersData[occupant].y_target) {
                // ? 5B8938: y_target is stored before x_target (chained assignment).
                PlayersData[occupant].x_target = PlayersData[occupant].y_target = -1;
                for (int v = veh_at(x, y); v >= 0; v = Vehs[v].next_veh_id_stack) {   // register
                    if (Vehs[v].state & VSTATE_UNK_40000) {
                        Vehs[v].state |= VSTATE_UNK_40000 | VSTATE_UNK_20000;
                    }
                }
            }
            if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_INFO_WARFARE
                && !(PlayersData[faction_id].diplo_treaties[occupant] & DTREATY_PACT)) {
                add_goal(occupant, AI_GOAL_ATTACK, 2, x0, y0, -1);   // 5B89CC
                add_goal(occupant, AI_GOAL_DEFEND, 1, x0, y0, -1);
                add_goal(occupant, AI_GOAL_SCOUT, 1, x0, y0, -1);
            }
        }
    }

    if (GameDataStateStorage.fields_.turn_current_num_) {
        draw_radius(x0, y0, faction_id == ConsoleGlobal.faction_id_ ? radius_tier : 0, 2);
        if (faction_id == ConsoleGlobal.faction_id_ && radius_tier) {
            WorldClimateWorldWin.GraphicWin::redraw();
        }
        if (count > 8 && faction_id == ConsoleGlobal.faction_id_
            && (Players[faction_id].rule_flags & 0x80)) {
            interlude(0x29, 0, 1, 0);
        }
        if (encounter_mask & 1) {
            encounter(faction_id, 0, x0, y0, enc_a, enc_b);
        }
        if (faction_id && toggle) {
            for (int f = 1; f < MaxPlayerNum; f++) {
                if (f != faction_id && (encounter_mask & (1 << f))) {
                    encounter(faction_id, f, x0, y0, 0, 0);
                }
            }
        }
    }
}


/*
Purpose: Move a stack of units in the same stack as the specified unit to a destination.
// ORIGINAL: 0x005B8AF0 ?stack_put@@YAXHHH@Z 0x005B8AF0-0x005B8B5D BYTE_EXACT
// size      109 bytes
// prototype void (__cdecl ?stack_put@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   5   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFFA0 0x005C0080
// LEVER: veh_top()/veh_put() are direct calls that inline here; veh_top's
//   sign test must precede its `top = veh_id` copy (see veh_top).
Return Value: n/a
Status: Complete
*/
void __cdecl stack_put(int veh_id, int x, int y) {
    int top_veh_id = veh_top(veh_id);
    int veh_id_loop;
    if (top_veh_id >= 0) {
        do {
            veh_id_loop = Vehs[top_veh_id].next_veh_id_stack;
            veh_put(top_veh_id, x, y);
            top_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
    }
}


/*
Purpose: Sort a stack of units with the transports moved to the top.
// ORIGINAL: 0x005B8B60 ?stack_sort@@YAXH@Z 0x005B8B60-0x005B8C8D BYTE_EXACT
// TRIED: and `stack_put(veh_id_put, x, y)` were plain calls; call_diff's count-based check missed it (6 calls either way, since {veh_top, stack_put} happened to replace the {2 extra veh_lift/veh_drop} the image gets from inlining stack_put), but the image's actual 6 callees are veh_cargo/has_abil/veh_lift x2/ veh_drop x2 - no veh_top or stack_put. Hand-inlining both, including stack_put's OWN internal veh_top climb (its outer `next_veh_id >= 0` check survives - the image keeps it as a second, separate test after veh_top's own), took 0.627 -> 0.758.
// LEVER: moved beside stack_put (same TU: veh_top inlines). The first veh_top()
//   call inlines; the final walk is written out like stack_sort_2's because the
//   image keeps the dead `id < 0 ? id : next` test that a stack_put() call
//   lacks, and its climb tests the sign before copying (veh_top copies first).
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
    int veh_id_put = -1;
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    int next_veh_id = veh_top(veh_id);
    int veh_id_loop;
    if (next_veh_id >= 0) {
        do {
            // The image keeps this dead sign test inside the walk.
            veh_id_loop = next_veh_id < 0 ? next_veh_id : Vehs[next_veh_id].next_veh_id_stack;
            if (veh_cargo(next_veh_id) || has_abil(Vehs[next_veh_id].proto_id, ABL_CARRIER)) {
                veh_id_put = next_veh_id;
                veh_put(next_veh_id, -3, -3);
            }
            next_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
        // veh_top()'s climb written out sign-test first; a veh_top() call
        // schedules the `top = veh_id_put` copy ahead of the test.
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
                veh_id_loop2 = top_veh_id < 0 ? top_veh_id : Vehs[top_veh_id].next_veh_id_stack;
                veh_put(top_veh_id, x, y);
                top_veh_id = veh_id_loop2;
            } while (veh_id_loop2 >= 0);
        }
    }
}


/*
Purpose: Sort a stack of units with colony pods at the top followed by combat/offensive units.
// ORIGINAL: 0x005B8C90 ?stack_sort_2@@YAXH@Z 0x005B8C90-0x005B8E0A BYTE_EXACT
// LEVER: the first veh_top() call inlines (same TU). The final walk is written
//   out, not a stack_put() call: the image re-derives `next_veh_id_stack` through a
//   `(id < 0) ? id : Vehs[id].next_veh_id_stack` ternary at all three reads,
//   which stack_put's own body does not have. Moved veh_id_put's declaration
//   to the top of the function, matching store order in the image.
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
    int next_veh_id = veh_top(veh_id);
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
    // veh_top()'s climb written out sign-test first; a veh_top() call
    // schedules the `top = veh_id_put` copy ahead of the test.
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
// ORIGINAL: 0x005B8E10 ?stack_fix@@YAHH@Z 0x005B8E10-0x005B8ED9 BYTE_EXACT
// TRIED: frame pointer - the image keeps ebp (a `push ecx` local-space idiom) and duplicate non-merged epilogues; no flag set in the harness reproduces both at once here - /Oy- restores ebp but the two early returns cross-jump to one shared tail instead of duplicating, unlike the image. Plateau at 0.758 (/O2 /Gy /GR- /GX).
// TRIED: is called at only 2 real sites in the image (see its own comment on veh_top); here it is inlined, matching veh_at above.
// TRIED: combined `if (a || b || c) return veh_id;` compiled to one merged block; the image duplicates the epilogue three times (three separate early returns, each its own `jge`). Splitting into three sequential `if`s took similarity 0.627 -> 0.758 (best flag set has 0 raw agree but is the closest shape - see BUG note below for the loop, which is the bulk of the remaining gap).
// BUG IN THE ORIGINAL: the loop compares the subject Vehs[veh_id]'s current
//        x/y to copies captured at loop entry, rather than checking iterator
//        Vehs[i]. Preserve that wrong-subject comparison; it can become false
//        after a callback changes the subject's coordinates.
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
    const int entry_x = Vehs[veh_id].x;
    const int entry_y = Vehs[veh_id].y;
    for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
        // Re-read the subject and compare to loop-entry values (not Vehs[i]);
        // a preceding callback may have changed the subject's coordinates.
        if (Vehs[veh_id].x == entry_x && Vehs[veh_id].y == entry_y) {
            veh_promote(i);
            stack_sort(veh_id);
        }
    }
    return veh_top(veh_id);
}


/*
Purpose: Board the eligible units stacked with the specified transport or carrier, optionally
         lifting the whole stack off the map afterwards. Mode 0 marks and counts without moving
         anything; mode 1 sorts the stack first and lifts each boarder plus the subject to the
         off-map holding square; mode 2+ makes a single pass and short-circuits to the lift when
         the subject already sits on a faction-owned base.
// ORIGINAL: 0x005B8EE0 ?stack_veh@@YAHHH@Z 0x005B8EE0-0x005B950F
// TRIED: by 1 (17 vs image's 16) - two `veh_top(veh_id)` calls (the mode-0 refresh loop and the per-pass boarding loop's start index) hand-inlined, matching stack_fix's precedent, and `sleep(i)` forced to a real call via a new `sleep_call` forwarder in veh.h/veh.cpp (its only other reference is the inline body - under /O2 `sleep(i)` folds to 3 field writes, but the image calls it for real at this one site). call_diff now agrees (0 disagreeing, was MORE by 1). This is a 472-instruction function; the remaining gap is per-branch register/scheduling noise not chased further at this budget.
// SUPERSEDED: sleep now has a separate ordinary mapped owner in
// vehicle_orders.cpp; stack_veh calls it directly.
// SUPERSEDED: the two veh_top() climbs are plain calls again; they inline
// identically to the hand copies.
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
    int carrier_left;
    int cargo_left;
    BOOL open_ocean;
    int i;
    int pass;
    int next_veh_id;
    int faction_id;
    BOOL subject_is_air;
    int pass_count;
    BOOL board;

    if (veh_id < 0) {
        return mode ? veh_id : 0;
    }
    if (!veh_cargo(veh_id) && !has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)) {
        // Neither a transport nor a carrier: nothing can be stacked onto it.
        if (mode) {
            goto lift;
        }
        return 0;
    }
    if (IsMultiplayerNet) {
        stack_fix(veh_id); // return value is discarded by the original
    }
    if (mode) {
        if (mode > 1 && base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
            goto lift;
        }
        stack_sort(veh_id);
    } else {
        for (i = veh_top(veh_id); i >= 0; i = Vehs[i].next_veh_id_stack) {
            Vehs[i].state &= ~VSTATE_UNK_1; // so this call's boarding flags start clean
        }
    }
    open_ocean = on_map(Vehs[veh_id].x, Vehs[veh_id].y)
        && is_ocean(Vehs[veh_id].x, Vehs[veh_id].y)
        && base_who(Vehs[veh_id].x, Vehs[veh_id].y) < 0;
    faction_id = Vehs[veh_id].faction_id;
    subject_is_air = (get_triad(veh_id) == TRIAD_AIR);
    cargo_left = veh_cargo(veh_id);
    carrier_left = has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)
        ? (veh_cargo(veh_id) < 1 ? 1 : veh_cargo(veh_id)) : 0;
    if (!faction_id) {
        cargo_left = 99;
    }
    pass_count = (mode <= 1) ? 2 : 1;
    for (pass = 0; pass < pass_count; pass++) {
        for (i = mode ? veh_id : veh_top(veh_id); i >= 0; i = next_veh_id) {
            if (!cargo_left && !carrier_left) {
                break;
            }
            // Captured before the body: mode 1 lifts the unit, rewriting this link.
            next_veh_id = (i < 0) ? i : Vehs[i].next_veh_id_stack;
            if (Vehs[i].faction_id != faction_id) {
                continue;
            }
            if (!((cargo_left && get_triad(i) == TRIAD_LAND && !veh_cargo(i))
                  || (carrier_left && get_triad(i) == TRIAD_AIR
                      && !has_abil(Vehs[i].proto_id, ABL_CARRIER)))) {
                continue;
            }
            switch (pass) {
            case 0:
                // First pass re-boards only what is already recorded as riding this transport.
                board = Vehs[i].order == ORDER_SENTRY_BOARD
                    && Vehs[i].waypoint_x[0] == veh_id;
                break;
            case 1:
                // Note the > 0 here: unlike the two other owner tests, faction 0 is rejected.
                if (subject_is_air
                    && base_who(Vehs[veh_id].x, Vehs[veh_id].y) <= 0
                    && !(bit_at(Vehs[veh_id].x, Vehs[veh_id].y) & BIT_AIRBASE)) {
                    board = FALSE;
                } else if (is_human(Vehs[i].faction_id)
                           && !((Vehs[veh_id].state & VSTATE_UNK_200)
                                && (Vehs[i].state & VSTATE_UNK_200))) {
                    if (Vehs[i].order == ORDER_SENTRY_BOARD) {
                        board = Vehs[i].waypoint_x[0] < 0;
                    } else {
                        board = open_ocean;
                    }
                } else if (Vehs[i].state & VSTATE_UNK_40000) {
                    board = (Vehs[i].state & VSTATE_UNK_20000)
                        && PlayersData[Vehs[i].faction_id]
                               .region_base_plan[region_at(Vehs[i].x, Vehs[i].y)]
                           == PLAN_NAVAL_TRANSPORT;
                } else if (Vehs[i].order == ORDER_HOLD
                           || VehPrototypes[Vehs[i].proto_id].plan == PLAN_ALIEN_ARTIFACT
                           || VehPrototypes[Vehs[i].proto_id].plan == PLAN_TERRAFORMING) {
                    board = open_ocean;
                } else {
                    board = open_ocean
                        || !(on_map(Vehs[i].x, Vehs[i].y)
                             && PlayersData[Vehs[i].faction_id]
                                    .region_base_plan[region_at(Vehs[i].x, Vehs[i].y)]
                                == PLAN_DEFENSIVE
                             && VehPrototypes[Vehs[i].proto_id].plan < PLAN_COLONIZATION);
                }
                break;
            default:
                continue;
            }
            if (board && (mode || !(Vehs[i].state & VSTATE_UNK_1))) {
                sleep(i);
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
        }
    }
    if (mode) {
lift:
        return veh_drop(veh_lift(veh_id), -2, -2);
    }
    return veh_cargo(veh_id) ? cargo_left : carrier_left;
}


/*
Purpose: Kill a vehicle stack by walking the link list to its tail, then
         unwinding it leaf-first (each kill can free the next link, so the
         walk takes the low side whenever the branch points upward).
// ORIGINAL: 0x005B9510 ?stack_kill@@YAXH@Z 0x005B9510-0x005B9577 BYTE_EXACT
// size      103 bytes
// prototype void (__cdecl ?stack_kill@@YAXH@Z)(int vehID)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C08C0
// HOMED from src/recovered/units/005b9510.cpp, 2026-09-03: the claim was
//            archive-only while a PENDING_BODY forwarder named it - the
//            address-index landmine. The artifact's two dead g_009528xx
//            bindings were not carried; the body never reads them.
Return Value: n/a
Status: Complete
*/
// Declared, never defined here (a definition would be inlined); the editor
// command family calls it by name.
void __cdecl veh_kill(int vehID);

void __cdecl stack_kill(int a1) {
    if (a1 >= 0) {
        short s = Vehs[a1].prev_veh_id_stack;
        while (s >= 0) {
            a1 = s;
            s = Vehs[a1].prev_veh_id_stack;
        }
    }
    if (a1 >= 0) {
        do {
            int next;
            if (a1 < 0) {
                next = a1;
            } else {
                next = Vehs[a1].next_veh_id_stack;
            }
            veh_kill(a1);
            if (next > a1) {
                --next;
            }
            a1 = next;
        } while (a1 >= 0);
    }
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x005B9580 0x005B9BB8 0x005B9C08 0x005B95D7
/*
Purpose: Various unit stack related calculations based on type parameter (0-19) and conditions.
// ORIGINAL: 0x005B9580 ?stack_check@@YAHHHHHH@Z 0x005B9580-0x005B9BB8 BYTE_EXACT
// TRIED: call_diff still FEWER by one - case 12's `weap_strat(...)` call gets auto-inlined by this tree (weap_strat is small/BYTE_EXACT after this same pass) where the image keeps it a real `call`. `#pragma auto_inline(off)` around weap_strat's OWN definition would fix this call site but also stops weap_val (0x0057D360, not in this batch) from inlining it, which the image DOES do there - out of scope to touch a function outside this batch to fix this one. Still MISMATCH overall (~0.28 similar best) - a 20-case switch this size has per-case register allocation/instruction-scheduling noise that dwarfs the two structural fixes above.
// The original inlines veh_top, but case 8 calls veh_cargo at 0x005B984D.
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
    uint8_t plan;
    uint32_t chas;
    for (int i = veh_top(veh_id); i >= 0; i = Vehs[i].next_veh_id_stack) {
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
                switch (get_triad(i)) {
                  case TRIAD_LAND:
                    retn_val--;
                    break;
                  case TRIAD_SEA:
                    // The pinned body calls veh_cargo(i) at 0x005B984D.
                    retn_val += veh_cargo(i);
                    break;
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
                if (arm_strat(VehPrototypes[Vehs[i].proto_id].armor_id, Vehs[i].faction_id)
                    > weap_strat(VehPrototypes[Vehs[i].proto_id].weapon_id, Vehs[i].faction_id)) {
                    retn_val++;
                }
            }
            break;
          case 16:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1)
                && (plan = VehPrototypes[Vehs[i].proto_id].plan, plan == PLAN_DEFENSIVE
                    || plan == PLAN_RECONNAISANCE || plan == PLAN_COMBAT)) {
                retn_val++;
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
