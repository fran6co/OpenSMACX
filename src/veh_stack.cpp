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
    // on_map() and map_loc() are written out here rather than called: the
    // image folds both of them in, and writing the expressions removes the
    // optimiser's inline/no-inline choice instead of hoping for it.
    // Keep the real synch_bit API call boundary.
    if (!(y >= 0 && y < (int)MapLatitudeBounds
          && x >= 0 && x < (int)MapLongitudeBounds)) {
        return;
    }
    Map *tile = map_tiles() + ((x >> 1) + y * MapLongitude);
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
int __cdecl veh_can_leave_stack(int veh_id) {
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



// Base history remains inline rather than calling spot_base(), preserving the
// contact path's treaty-sensitive dirty-map gate. The contact passes scan
// offsets [0,9), then [9,25); the 25/49 outer sensor-radius bound belongs to
// earlier visibility phases, not this tactical-radius work.
//
// The final notification path remains separate from sensing: the first
// encounter uses the neutral faction slot and selected base/tile context;
// remaining factions are visited in ascending id order only when their bit
// was accumulated by the sensing passes and the caller requested wakeups.
// ORIGINAL: 0x005B6260 ?spot_all@@YAXHH@Z 0x005B6260-0x005B8AE2
// RECOVERY_BINDING: v1 {"evidence": ["task:vehicle-visibility-owner", "pinned:5B6260..5B8AE2 complete sensing, terrain memory, contact/wake/goals and presentation span; original prototype and seed findings preserved.", "pinned:70F5 own Base joins common73E1 tile/stack path;7858 contact visibility differs from824E selected-source visibility.", "pinned:76FB..7707 target fallback contactLow || !sourceLow;7A69..7A7B source fallback sourceLow || !contactLow || triad2; distinct lock/callback routes.", "pinned:8A43 fixed WorldClimateWorldWin receiver8E9F60 and8A48 direct GraphicWin::redraw5D5A70 before interlude/encounters; source:worldwin.h owns the receiver.", "historical seed:src/unrecovered/005b6260.cpp native mapping and unsuccessful mechanical transliteration retained as source evidence."], "reason": "Recover the complete original visibility operation with genuine vehicle, faction, base, map and climate-window owners; retain original identity/spans and seed findings without image calls or raw self-offset models."}
// TRIED: mechanical register/stack-slot goto transliteration (234 labels, 5638-line body): compiles, MISMATCH sim up to 0.73 on /O2 frameless. Not restructured into idiomatic control flow - next pass should re-derive loop/if shape per block before chasing byte-exactness.
void __cdecl spot_all(int source_vehicle_id, int toggle) {
    const Veh &selected_source = Vehs[source_vehicle_id];
    const int source_faction_id = selected_source.faction_id;
    const int source_prototype_id = selected_source.proto_id;
    const int source_vehicle_triad = get_proto_triad(selected_source.proto_id);
    const int source_x = selected_source.x;
    const int source_y = selected_source.y;
    const unsigned char source_faction_mask =
        (unsigned char)(1u << source_faction_id);
    int special_terrain_mode;
    int source_controls_sensor_radius;
    int offset_limit;
    int visibility_offset_limit;
    int effective_toggle;
    int early_radius_tier;
    int source_low_terrain;
    int neutral_event_flag_a;
    int neutral_event_flag_b;

    // Offline play forces the reveal toggle before scan policy is selected.
    if (!IsMultiplayerNet) {
        toggle = 1;
    }
    effective_toggle = toggle;
    early_radius_tier = 0;
    source_low_terrain = 0;
    neutral_event_flag_a = 0;
    neutral_event_flag_b = 0;
    if (on_map(source_x, source_y)) {
        source_low_terrain =
            (map_loc(source_x, source_y)->climate & 0xE0u) < 0x60u;
    }

    if (has_abil(source_prototype_id, 0x40) ||
        has_abil(source_prototype_id, 4) ||
        (selected_source.flags & 0x400u) != 0) {
        special_terrain_mode = 1;
    } else {
        special_terrain_mode = 0;
        if (on_map(source_x, source_y)) {
            const Map *tile = map_loc(source_x, source_y);
            if ((tile->bit & 0x20u) != 0 &&
                (tile->climate & 0xE0u) >= 0x40u &&
                source_prototype_id != 0x13) {
                special_terrain_mode = 2;
            }
        }
    }

    source_controls_sensor_radius = 0;
    if (has_abil(source_prototype_id, 2)) {
        source_controls_sensor_radius = 1;
    } else if (is_sensor(source_x, source_y) &&
               whose_territory(source_faction_id, source_x, source_y, 0, 0) ==
                   source_faction_id) {
        source_controls_sensor_radius = 1;
    }
    offset_limit = 25 + (source_controls_sensor_radius ? 24 : 0);
    visibility_offset_limit =
        9 + (source_controls_sensor_radius ? 16 : 0);
    int encounter_mask = 0;

    // The source's faction-zero cleanup is the byte AND at 0x5B640E. It
    // applies to the selected Veh only and is distinct from the later stack
    // walks which expose a stack to a sensor owner.
    if (source_faction_id == 0 && on_map(source_x, source_y)) {
        Vehs[source_vehicle_id].visibility &=
            map_loc(source_x, source_y)->visibility;
    }

    if ((GameDataStateStorage.fields_.game_more_preferences_ & 1u) != 0 &&
        source_faction_id == ConsoleGlobal.faction_id_ &&
        (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0) {
        const int inner_scan_start =
            source_controls_sensor_radius ? 25 : 9;
        for (int offset = 0; offset < offset_limit; ++offset) {
            int x = source_x + RadiusOffsetX[offset];
            const int y = source_y + RadiusOffsetY[offset];
            if (!(MapIsFlat & 1)) {
                if (x < 0) x += MapLongitudeBounds;
                else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
            }
            if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
                x >= MapLongitudeBounds) {
                continue;
            }

            Map *tile = map_loc(x, y);
            int inner_match = 1;
            if (offset >= inner_scan_start) {
                inner_match = 0;
                for (int inner_index = 0; inner_index < 25; ++inner_index) {
                    int inner_x = x + RadiusOffsetX[inner_index];
                    const int inner_y = y + RadiusOffsetY[inner_index];
                    if (!(MapIsFlat & 1)) {
                        if (inner_x < 0) inner_x += MapLongitudeBounds;
                        else if (inner_x >= MapLongitudeBounds)
                            inner_x -= MapLongitudeBounds;
                    }
                    if (!on_map(inner_x, inner_y)) continue;
                    const Map *inner_tile = map_loc(inner_x, inner_y);
                    int tile_owner = -1;
                    if ((inner_tile->bit & 3u) != 0 &&
                        (inner_tile->val2 & 0x0Fu) < 8) {
                        tile_owner = inner_tile->val2 & 0x0Fu;
                    }
                    if (inner_index < 9) {
                        if (tile_owner == source_faction_id) {
                            inner_match = 1;
                            break;
                        }
                        const int treaty_vehicle_path = tile_owner >= 0 &&
                            (PlayersData[source_faction_id]
                                 .diplo_treaties[tile_owner] & 1u) != 0;
                        if (treaty_vehicle_path) {
                            const int vehicle_id = veh_at(inner_x, inner_y);
                            if (vehicle_id >= 0) {
                                const int top_vehicle_id = veh_top(vehicle_id);
                                for (int member_id = top_vehicle_id;
                                     member_id >= 0;
                                     member_id = Vehs[member_id].next_veh_id_stack) {
                                    if (Vehs[member_id].faction_id ==
                                        source_faction_id) {
                                        inner_match = 1;
                                        break;
                                    }
                                }
                                if (inner_match) break;
                            }
                        }
                    }
                    if (is_sensor(inner_x, inner_y)) {
                        int facility_offset;
                        int facility_mask;
                        bitmask(0x29, &facility_offset, &facility_mask);
                        const int sensor_owner =
                            map_loc(inner_x, inner_y)->territory;
                        if (sensor_owner == source_faction_id) {
                            inner_match = 1;
                            break;
                        }
                        const int base_id = base_find(source_x, source_y);
                        if (base_id >= 0) {
                            int delta_x = source_x - Bases[base_id].x;
                            if (delta_x < 0) delta_x = -delta_x;
                            if (!(MapIsFlat & 1) && delta_x > MapLongitude)
                                delta_x = MapLongitudeBounds - delta_x;
                            if (delta_x == 0 || delta_x == 2) {
                                int delta_y = source_y - Bases[base_id].y;
                                if (delta_y < 0) delta_y = -delta_y;
                                if ((delta_y == 0 || delta_y == 2) &&
                                    (Bases[base_id].facilities_built[
                                         facility_offset] & facility_mask) != 0) {
                                    inner_match = 1;
                                    break;
                                }
                            }
                        }
                    }
                    const int reverse_treaty_sensor_path = tile_owner >= 0 &&
                        (inner_tile->bit & 2u) != 0 &&
                        (PlayersData[tile_owner]
                             .diplo_treaties[source_faction_id] & 1u) != 0;
                    if (reverse_treaty_sensor_path) {
                        const int vehicle_id = veh_at(inner_x, inner_y);
                        if (vehicle_id >= 0) {
                            const int top_vehicle_id = veh_top(vehicle_id);
                            const Map *center_tile = map_loc(x, y);
                            const int low_terrain =
                                (center_tile->climate & 0xE0u) < 0x60u;
                            for (int member_id = top_vehicle_id;
                                 member_id >= 0;
                                 member_id = Vehs[member_id].next_veh_id_stack) {
                                const Veh &member = Vehs[member_id];
                                if (member.faction_id != source_faction_id ||
                                    !has_abil(member.proto_id, 2)) {
                                    continue;
                                }
                                const int prototype_category =
                                    get_proto_triad(member.proto_id);
                                if (prototype_category == 2 ||
                                    ((prototype_category == 1) == low_terrain)) {
                                    inner_match = 1;
                                    break;
                                }
                            }
                            if (inner_match) break;
                        }
                    }
                }
            }
            if ((tile->bit2 & 0x200000u) == (inner_match ? 0x200000u : 0u)) {
                const int candidate_tier = offset < 9 ? 1
                    : (offset < 25 ? 2 : 3);
                if (early_radius_tier <= candidate_tier)
                    early_radius_tier = candidate_tier;
            }
        }
    }
    int terrain_visibility_count = 0;
    for (int offset = 0; offset < visibility_offset_limit; ++offset) {
        int x = source_x + RadiusOffsetX[offset];
        const int y = source_y + RadiusOffsetY[offset];
        if (!(MapIsFlat & 1)) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds) {
            continue;
        }
        Map *tile = map_loc(x, y);
        if ((tile->bit2 & 0x80002000u) == 0x2000u) {
            ++terrain_visibility_count;
        }
        if (offset >= 9 && source_vehicle_triad != 2) {
            const int is_low_terrain = (tile->climate & 0xE0u) < 0x60u;
            if (is_low_terrain != (source_vehicle_triad == 1)) {
                continue;
            }
        }
        if (source_faction_id == ConsoleGlobal.faction_id_ &&
            (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0 &&
            (PlayersData[source_faction_id].flags &
             PFLAG_MAP_REVEALED) == 0) {
            const int unseen = (tile->visibility & source_faction_mask) == 0;
            const uint32_t remembered_bits = source_faction_id != 0
                ? tile->bit_visible[source_faction_id - 1] : tile->bit2;
            const int memory_changed =
                ((remembered_bits ^ tile->bit) & 0xFFFFFFFCu) != 0;
            if (unseen) {
                tile->bit2 |= 0x400000u;
                WorldClimateWorldWin.field_239BC_ |= 1u;
            }
            const int minimum_tier = (offset >= 9) ? 3 : 2;
            if ((unseen || memory_changed) &&
                early_radius_tier <= minimum_tier) {
                early_radius_tier = minimum_tier;
            }
        }
        if (source_faction_id != 0) {
            tile->bit_visible[source_faction_id - 1] = tile->bit;
        }
        tile->visibility |= source_faction_mask;
    }
    for (int outer_offset = visibility_offset_limit;
         outer_offset < offset_limit; ++outer_offset) {
        int x = source_x + RadiusOffsetX[outer_offset];
        const int y = source_y + RadiusOffsetY[outer_offset];
        if (!(MapIsFlat & 1)) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        if (!on_map(x, y)) continue;
        Map *tile = map_loc(x, y);
        if ((tile->visibility & source_faction_mask) != 0) continue;

        const int maximum_hidden_neighbors =
            source_controls_sensor_radius != 0 ||
            (tile->climate & 0xE0u) < 0x60u ? 4 : 6;
        int hidden_count = 0;
        for (int direction = 0; direction < 8; ++direction) {
            int neighbor_x = x + VehMoveDeltaX[direction];
            const int neighbor_y = y + VehMoveDeltaY[direction];
            if (!(MapIsFlat & 1)) {
                if (neighbor_x < 0) neighbor_x += MapLongitudeBounds;
                else if (neighbor_x >= MapLongitudeBounds)
                    neighbor_x -= MapLongitudeBounds;
            }
            if (!on_map(neighbor_x, neighbor_y)) continue;
            if ((map_loc(neighbor_x, neighbor_y)->visibility &
                 source_faction_mask) == 0) {
                hidden_count += (VehMoveDeltaX[direction] == 0 ||
                                 VehMoveDeltaY[direction] == 0) ? 2 : 1;
            }
        }
        if (hidden_count >= maximum_hidden_neighbors) continue;

        const int unseen = (tile->visibility & source_faction_mask) == 0;
        if (source_faction_id == ConsoleGlobal.faction_id_ &&
            (GameDataStateStorage.fields_.game_state_ &
             STATE_OMNISCIENT_VIEW) == 0 &&
            (PlayersData[source_faction_id].flags &
             PFLAG_MAP_REVEALED) == 0 && unseen) {
            tile->bit2 |= 0x400000u;
            WorldClimateWorldWin.field_239BC_ |= 1u;
        }
        if (source_faction_id != 0)
            tile->bit_visible[source_faction_id - 1] = tile->bit;
        tile->visibility |= source_faction_mask;

        const int base_id = base_at(x, y);
        if (base_id >= 0) {
            Base &base = Bases[base_id];
            base.visibility |= source_faction_mask;
            base.faction_pop_size_intel[source_faction_id] =
                (uint8_t)base.population_size;
            if (on_map(base.x, base.y)) {
                Map *base_tile = map_loc(base.x, base.y);
                if (source_faction_id == ConsoleGlobal.faction_id_ &&
                    (GameDataStateStorage.fields_.game_state_ &
                     STATE_OMNISCIENT_VIEW) == 0 &&
                    (base_tile->visibility & source_faction_mask) == 0 &&
                    (PlayersData[source_faction_id].flags &
                     PFLAG_MAP_REVEALED) == 0) {
                    base_tile->bit2 |= 0x400000u;
                    WorldClimateWorldWin.field_239BC_ |= 1u;
                }
                if (source_faction_id != 0)
                    base_tile->bit_visible[source_faction_id - 1] =
                        base_tile->bit;
                base_tile->visibility |= source_faction_mask;
                synch_bit(base.x, base.y, source_faction_id);
            }
        }

        const int minimum_tier =
            2 + (source_controls_sensor_radius != 0);
        if (early_radius_tier <= minimum_tier)
            early_radius_tier = minimum_tier;
    }
    // 0x5B6DED refreshes only stack members whose owner differs from the
    // selected vehicle's faction, through each member's existing owners.
    {
        const int top_vehicle_id = veh_top(source_vehicle_id);
        if (top_vehicle_id >= 0) {
            const Veh &top_vehicle = Vehs[top_vehicle_id];
            const int top_owner = top_vehicle.faction_id;
            if (top_owner != source_faction_id) {
                if (on_map(top_vehicle.x, top_vehicle.y))
                    spot_tile(top_vehicle.x, top_vehicle.y, top_owner);
                const int seen_top_vehicle_id = veh_top(top_vehicle_id);
                if (seen_top_vehicle_id >= 0) {
                    const uint8_t seen_mask = (uint8_t)(1u << top_owner);
                    for (int member_id = seen_top_vehicle_id; member_id >= 0;
                         member_id = Vehs[member_id].next_veh_id_stack) {
                        Veh &member = Vehs[member_id];
                        member.visibility |= seen_mask;
                        if (top_owner != 0)
                            member.flags &= (uint16_t)~(VFLAG_LURKER | VFLAG_INVISIBLE);
                    }
                }
            }
        }
    }

// Original 0x5B6EE9..0x5B89FA contact work runs in two adjacent passes.
// Keep their shared source snapshot and encounter mask live across both.
for (int contact_pass = 0; contact_pass != 2; ++contact_pass) {
    const int begin_offset = contact_pass == 0 ? 0 : 9;
    const int end_offset = contact_pass == 0 ? 9 : 25;
    const int allow_ai_goals = effective_toggle;
    int contacts = 0;

    for (int offset = begin_offset; offset < end_offset; ++offset) {
        int x = source_x + RadiusOffsetX[offset];
        const int y = source_y + RadiusOffsetY[offset];
        if (!(MapIsFlat & 1)) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        if (!on_map(x, y) || !is_sensor(x, y)) {
            continue;
        }

        const Map *sensor_tile = map_loc(x, y);
        const int sensor_faction = sensor_tile->territory;
        const int contact_low_terrain =
            (sensor_tile->climate & 0xE0u) < 0x60u;
        if (sensor_faction <= 0 || sensor_faction == source_faction_id ||
            special_terrain_mode == 1) {
            continue;
        }
        if ((sensor_tile->bit & 1u) != 0 &&
            (sensor_tile->val2 & 0x0Fu) < 8) {
            continue;
        }

        if (source_vehicle_id >= 0 && on_map(source_x, source_y)) {
            spot_tile(source_x, source_y, sensor_faction);
            const int seen_top_vehicle_id = veh_top(source_vehicle_id);
            if (seen_top_vehicle_id >= 0) {
                const uint8_t seen_mask = (uint8_t)(1u << sensor_faction);
                for (int member_id = seen_top_vehicle_id; member_id >= 0;
                     member_id = Vehs[member_id].next_veh_id_stack) {
                    Veh &member = Vehs[member_id];
                    member.visibility |= seen_mask;
                    if (sensor_faction != 0)
                        member.flags &= (uint16_t)~(VFLAG_LURKER | VFLAG_INVISIBLE);
                }
            }
        }

        if (allow_ai_goals &&
            (PlayersData[source_faction_id].diplo_treaties[sensor_faction] &
             7u) == 0) {
            if (special_terrain_mode == 0) {
                add_goal(sensor_faction, AI_GOAL_ATTACK, 2, x, y, -1);
                add_goal(sensor_faction, AI_GOAL_DEFEND, 1, x, y, -1);
            } else {
                add_goal(sensor_faction, AI_GOAL_UNK_1, 2, x, y, -1);
            }
        }

        if (offset != 0) {
            const int detected_base_id = base_at(x, y);
            if (detected_base_id >= 0 &&
                Bases[detected_base_id].faction_id_current !=
                    source_faction_id) {
                Base &base = Bases[detected_base_id];

                if (begin_offset < 9 &&
                    base.faction_id_current != source_faction_id &&
                    base.faction_id_current < 8) {
                    const int chassis_id =
                        VehPrototypes[source_prototype_id].chassis_id;
                    if (!(source_vehicle_triad == 2 &&
                          Chassis[chassis_id].range != 0)) {
                        const int direct_base_path =
                            source_low_terrain == 0 &&
                            special_terrain_mode == 0;
                        const int treaty_path_allowed =
                            (PlayersData[source_faction_id]
                                 .diplo_treaties[base.faction_id_current] &
                             8u) == 0;
                        if (direct_base_path || treaty_path_allowed) {
                            encounter_mask |= 1 << base.faction_id_current;
                        }
                    }
                }

                if (source_faction_id == ConsoleGlobal.faction_id_ &&
                    (GameDataStateStorage.fields_.game_state_ &
                     STATE_OMNISCIENT_VIEW) == 0) {
                    const int had_base_memory =
                        (base.visibility & source_faction_mask) != 0;
                    const int changed_population =
                        base.faction_pop_size_intel[source_faction_id] !=
                        (uint8_t)base.population_size;
                    if (begin_offset < 9) {
                        if (!had_base_memory &&
                            early_radius_tier <= 2) {
                            early_radius_tier = 2;
                        } else if (had_base_memory && changed_population &&
                                   early_radius_tier <= 1) {
                            early_radius_tier = 1;
                        }
                    } else if (source_controls_sensor_radius != 0 &&
                               (special_terrain_mode == 2 ||
                                contact_low_terrain ==
                                    (source_vehicle_triad == 1))) {
                        if (!had_base_memory &&
                            early_radius_tier <= 3) {
                            early_radius_tier = 3;
                        } else if (had_base_memory && changed_population &&
                                   early_radius_tier <= 2) {
                            early_radius_tier = 2;
                        }
                    }
                }

                base.visibility |= source_faction_mask;
                base.faction_pop_size_intel[source_faction_id] =
                    (uint8_t)base.population_size;
                if (on_map(base.x, base.y)) {
                    Map *tile = map_loc(base.x, base.y);
                    if (source_faction_id == ConsoleGlobal.faction_id_ &&
                        (GameDataStateStorage.fields_.game_state_ &
                         STATE_OMNISCIENT_VIEW) == 0 &&
                        (tile->visibility & source_faction_mask) == 0 &&
                        (PlayersData[source_faction_id].flags &
                         PFLAG_MAP_REVEALED) == 0) {
                        tile->bit2 |= 0x400000u;
                        WorldClimateWorldWin.field_239BC_ |= 1u;
                    }
                    tile->visibility |= source_faction_mask;
                    synch_bit(base.x, base.y, source_faction_id);
                }
            }

            const int detected_stack_id = stack_fix(veh_at(x, y));
            if (detected_stack_id >= 0 &&
                detected_stack_id != source_vehicle_id &&
                Vehs[detected_stack_id].faction_id != source_faction_id) {
                const int detected_faction =
                    Vehs[detected_stack_id].faction_id;

                if (detected_base_id < 0 && source_vehicle_id >= 0 &&
                    detected_faction >= 0 && detected_faction < 8) {
                    if (on_map(source_x, source_y))
                        spot_tile(source_x, source_y, detected_faction);
                    const int seen_top_vehicle_id = veh_top(source_vehicle_id);
                    if (seen_top_vehicle_id >= 0) {
                        const uint8_t seen_mask =
                            (uint8_t)(1u << detected_faction);
                        for (int member_id = seen_top_vehicle_id; member_id >= 0;
                             member_id = Vehs[member_id].next_veh_id_stack) {
                            Veh &member = Vehs[member_id];
                            member.visibility |= seen_mask;
                            if (detected_faction != 0)
                                member.flags &=
                                    (uint16_t)~(VFLAG_LURKER | VFLAG_INVISIBLE);
                        }
                    }
                }

                const uint32_t pair_flags = PlayersData[source_faction_id]
                                                .diplo_treaties[detected_faction];
                int clear_target_stack_wake = 0;
                if (begin_offset < 9) {
                    if (allow_ai_goals) wake_stack(detected_stack_id);
                    if ((pair_flags & 1u) == 0 && allow_ai_goals &&
                        source_vehicle_id >= 0 && detected_faction >= 0 &&
                        detected_faction < 8) {
                        const uint32_t sensor_bit =
                            (uint32_t)(1u << detected_faction);
                        if ((GameDataStateStorage.fields_.lock_masks_
                                 .enable_mask_ & sensor_bit) != 0 &&
                            source_low_terrain ==
                                contact_low_terrain) {
                            clear_target_stack_wake = want_to_wake(
                                source_faction_id, detected_stack_id,
                                source_vehicle_id) != 0;
                        } else if (source_vehicle_id >= 0 &&
                                   detected_faction >= 0 &&
                                   detected_faction < 8) {
                            const uint32_t lock_bit =
                                (uint32_t)(1u << detected_faction);
                            if ((GameDataStateStorage.fields_.lock_masks_
                                     .enable_mask_ & lock_bit) == 0 &&
                                (PlayersData[source_faction_id]
                                     .diplo_treaties[detected_faction] & 7u) == 0 &&
                                (contact_low_terrain != 0 ||
                                 source_low_terrain == 0)) {
                                clear_target_stack_wake = 1;
                            }
                        }
                    } else if (allow_ai_goals && source_vehicle_id >= 0 &&
                               detected_faction >= 0 && detected_faction < 8) {
                        const uint32_t lock_bit =
                            (uint32_t)(1u << detected_faction);
                        if ((GameDataStateStorage.fields_.lock_masks_
                                 .enable_mask_ & lock_bit) == 0 &&
                            (PlayersData[source_faction_id]
                                 .diplo_treaties[detected_faction] & 7u) == 0 &&
                            (contact_low_terrain != 0 ||
                             source_low_terrain == 0)) {
                            clear_target_stack_wake = 1;
                        }
                    }
                }
                if (clear_target_stack_wake) {
                    const int top_vehicle_id = veh_top(detected_stack_id);
                    for (int member_id = top_vehicle_id; member_id >= 0;
                         member_id = Vehs[member_id].next_veh_id_stack) {
                        Veh &member = Vehs[member_id];
                        if ((member.state & 0x02000200u) == 0x02000200u &&
                            member.patrol_current_point < member.waypoint_count) {
                            member.state &= ~0x02000200u;
                        }
                        if ((member.state & 0x01000200u) == 0x01000200u)
                            member.state &= ~0x01000200u;
                        member.state &= ~0x00004000u;
                        if (member.order == 0x18) member.order = 0;
                    }
                }

                int skip_contact_history = 0;
                if (source_controls_sensor_radius == 0 &&
                    detected_stack_id >= 0 && detected_faction >= 0 &&
                    detected_faction < 8) {
                    const Veh &target = Vehs[detected_stack_id];
                    int native_stack_exception =
                        has_abil(target.proto_id, 0x40) ||
                        has_abil(target.proto_id, 4) ||
                        (target.flags & 0x400u) != 0;
                    if (!native_stack_exception && on_map(target.x, target.y)) {
                        const Map *tile = map_loc(target.x, target.y);
                        native_stack_exception =
                            (tile->bit & 0x20u) != 0 &&
                            (tile->climate & 0xE0u) >= 0x40u &&
                            target.proto_id != 0x13;
                    }
                    if (native_stack_exception &&
                        stack_check(detected_stack_id, 1, -1, -1, -1) == 1 &&
                        (PlayersData[source_faction_id]
                             .diplo_treaties[detected_faction] & 8u) != 0) {
                        skip_contact_history = 1;
                    }
                }
                if (begin_offset < 9 && skip_contact_history) {
                    ++contacts;
                    continue;
                }

                if (begin_offset < 9 && detected_base_id < 0 &&
                    source_faction_id == ConsoleGlobal.faction_id_ &&
                    (GameDataStateStorage.fields_.game_state_ &
                     STATE_OMNISCIENT_VIEW) == 0 &&
                    (Vehs[detected_stack_id].visibility &
                     source_faction_mask) == 0 &&
                    early_radius_tier <= 1) {
                    early_radius_tier = 1;
                }
                const Veh &detected = Vehs[detected_stack_id];
                if (!has_abil(detected.proto_id, 0x40) &&
                    !has_abil(detected.proto_id, 4) &&
                    (detected.flags & 0x400u) == 0 &&
                    on_map(detected.x, detected.y)) {
                    const Map *detected_tile =
                        map_loc(detected.x, detected.y);
                    const int exceptional_highland_unit =
                        (detected_tile->bit & 0x20u) != 0 &&
                        (detected_tile->climate & 0xE0u) >= 0x40u &&
                        detected.proto_id != 0x13;
                    if (begin_offset >= 9 && !exceptional_highland_unit &&
                        source_faction_id == ConsoleGlobal.faction_id_ &&
                        (GameDataStateStorage.fields_.game_state_ &
                         STATE_OMNISCIENT_VIEW) == 0 &&
                        (Vehs[source_vehicle_id].visibility &
                         source_faction_mask) == 0 &&
                        early_radius_tier <= 2) {
                        early_radius_tier = 2;
                    }
                }

                int clear_source_vehicle_wake = 0;
                if (begin_offset < 9 && allow_ai_goals) {
                    // The original call evaluates this ignored argument
                    // before may_query_sensor_owner_wake_policy rejects its
                    // conditions. Retain the source live-read site/order.
                    const int detected_prototype_for_policy =
                        Vehs[detected_stack_id].proto_id;
                    (void)detected_prototype_for_policy;
                    if (source_vehicle_id >= 0 && detected_faction >= 0 &&
                        detected_faction < 8) {
                        const uint32_t source_bit =
                            (uint32_t)(1u << source_faction_id);
                        const int can_query_sensor_owner_policy =
                            (GameDataStateStorage.fields_.lock_masks_
                                 .enable_mask_ & source_bit) != 0 &&
                            (source_low_terrain == contact_low_terrain ||
                             source_vehicle_triad == 2);
                        if (can_query_sensor_owner_policy) {
                            clear_source_vehicle_wake = want_to_wake(
                                detected_faction, source_vehicle_id,
                                detected_stack_id) != 0;
                        } else {
                            const uint32_t source_bit_for_fallback =
                                (uint32_t)(1u << source_faction_id);
                            if ((GameDataStateStorage.fields_.lock_masks_
                                     .enable_mask_ & source_bit_for_fallback) == 0 &&
                                (PlayersData[source_faction_id]
                                     .diplo_treaties[detected_faction] & 7u) == 0 &&
                                (source_low_terrain != 0 ||
                                 contact_low_terrain == 0 ||
                                 source_vehicle_triad == 2)) {
                                clear_source_vehicle_wake = 1;
                            }
                        }
                    }
                }
                if (clear_source_vehicle_wake) {
                    Veh &vehicle = Vehs[source_vehicle_id];
                    if ((vehicle.state & 0x02000200u) == 0x02000200u &&
                        vehicle.patrol_current_point < vehicle.waypoint_count) {
                        vehicle.state &= ~0x02000200u;
                    }
                    if ((vehicle.state & 0x01000200u) == 0x01000200u)
                        vehicle.state &= ~0x01000200u;
                    vehicle.state &= ~0x00004000u;
                    if (vehicle.order == 0x18) vehicle.order = 0;
                }

                if (allow_ai_goals &&
                    (PlayersData[source_faction_id]
                         .diplo_treaties[detected_faction] & 7u) == 0) {
                    const int goal_type =
                        has_abil(Vehs[detected_stack_id].proto_id, 0x44) ? 1 : 2;
                    add_goal(source_faction_id, AI_GOAL_UNK_1, goal_type,
                             x, y, -1);
                    add_goal(source_faction_id, AI_GOAL_ATTACK,
                             source_faction_id == 0 ? 1 : 3,
                             x, y, -1);
                }
                if (begin_offset < 9 && (pair_flags & 7u) == 0) {
                    add_goal(source_faction_id, AI_GOAL_ATTACK,
                             source_faction_id == 0 ? 1 : 2, x, y, -1);
                }
                if (begin_offset < 9 &&
                    VehPrototypes[source_prototype_id].plan >= 8 &&
                    (PlayersData[source_faction_id]
                         .diplo_treaties[detected_faction] & 1u) == 0 &&
                    whose_territory(detected_faction, x, y, 0, 0) ==
                        detected_faction &&
                    VehPrototypes[Vehs[detected_stack_id].proto_id].plan >= 8) {
                    add_goal(detected_faction, AI_GOAL_DEFEND, 2, x, y, -1);
                    add_goal(detected_faction, AI_GOAL_ATTACK, 1, x, y, -1);
                    add_goal(detected_faction, AI_GOAL_SCOUT, 1, x, y, -1);
                }

                // apply_spotted_location_effects is called with all three
                // optional wake/cleanup arguments zero at this callsite.
                spot_tile(x, y, source_faction_id);
                const int seen_top_vehicle_id = veh_top(detected_stack_id);
                if (seen_top_vehicle_id >= 0) {
                    const uint8_t seen_mask = (uint8_t)(1u << source_faction_id);
                    for (int member_id = seen_top_vehicle_id; member_id >= 0;
                         member_id = Vehs[member_id].next_veh_id_stack) {
                        Veh &member = Vehs[member_id];
                        member.visibility |= seen_mask;
                        if (source_faction_id != 0)
                            member.flags &=
                                (uint16_t)~(VFLAG_LURKER | VFLAG_INVISIBLE);
                    }
                }

                if (begin_offset >= 9 && allow_ai_goals &&
                    detected_stack_id >= 0 &&
                    special_terrain_mode == 0) {
                    const int top_vehicle_id = veh_top(detected_stack_id);
                    if (top_vehicle_id >= 0) {
                        const Veh &sensor_vehicle = Vehs[top_vehicle_id];
                        if (has_abil(sensor_vehicle.proto_id, 2)) {
                            const int triad =
                                get_proto_triad(sensor_vehicle.proto_id);
                            if ((triad == 2 ||
                                 source_low_terrain == (triad == 1)) &&
                                detected_faction >= 0 &&
                                detected_faction < 8 &&
                                (PlayersData[source_faction_id]
                                     .diplo_treaties[detected_faction] & 1u) == 0) {
                                wake_stack(detected_stack_id);
                            }
                        }
                    }
                }

                if (begin_offset < 9) {
                    const int detected_prototype =
                        Vehs[detected_stack_id].proto_id;
                    int event_path_qualified = 1;
                    if (source_low_terrain != contact_low_terrain &&
                        (detected_base_id < 0 ||
                         source_low_terrain != 0) &&
                        sensor_faction >= 0 && sensor_faction < 8 &&
                        (PlayersData[source_faction_id]
                             .diplo_treaties[sensor_faction] & 8u) != 0) {
                        event_path_qualified = 0;
                    }
                    if (event_path_qualified) {
                        if (detected_prototype == 0x13) {
                            neutral_event_flag_a = 1;
                        } else if (detected_prototype >= 0 &&
                                   detected_prototype < 0x40) {
                            const int weapon_id =
                                VehPrototypes[detected_prototype].weapon_id;
                            if (detected_prototype == 0x0F ||
                                (weapon_id >= 0 && weapon_id < 26 &&
                                 Weapon[weapon_id].offense_rating < 0)) {
                                neutral_event_flag_b = 1;
                            }
                        }
                    }
                    if (event_path_qualified)
                        encounter_mask |= 1 << detected_faction;
                }
            }

            if (begin_offset >= 9 && sensor_faction >= 0 &&
                sensor_faction < 8 && source_vehicle_triad == 0) {
                const Map *sensor_target_tile = map_loc(x, y);
                const Map *source_target_tile = map_loc(source_x, source_y);
                if (source_target_tile->region == sensor_target_tile->region) {
                    PlayerData &sensor_player = PlayersData[sensor_faction];
                    if (sensor_player.x_target == x &&
                        sensor_player.y_target == y) {
                        sensor_player.x_target = -1;
                        sensor_player.y_target = -1;
                        const int stack_id = veh_at(x, y);
                        for (int member_id = stack_id; member_id >= 0;
                             member_id = Vehs[member_id].next_veh_id_stack) {
                            Veh &member = Vehs[member_id];
                            if ((member.state & 0x40000u) != 0)
                                member.state |= 0x60000u;
                        }
                    }
                }
            }
            if (begin_offset >= 9 &&
                VehPrototypes[source_prototype_id].plan == 0x0B &&
                (PlayersData[source_faction_id]
                     .diplo_treaties[sensor_faction] & 1u) == 0) {
                add_goal(sensor_faction, AI_GOAL_ATTACK, 2,
                         source_x, source_y, -1);
                add_goal(sensor_faction, AI_GOAL_DEFEND, 1,
                         source_x, source_y, -1);
                add_goal(sensor_faction, AI_GOAL_SCOUT, 1,
                         source_x, source_y, -1);
            }
        }

        ++contacts;
    }
    (void)contacts; // Both original helper return values are discarded.
}

    // 0x5B8A00 onward presents the radius/interlude first and publishes
    // encounters only after that presentation path has returned.
    if (GameDataStateStorage.fields_.turn_current_num_ != 0) {
        const int radius_indicator = early_radius_tier;
        draw_radius(source_x, source_y,
                    source_faction_id == ConsoleGlobal.faction_id_
                        ? radius_indicator : 0,
                    2);
        if (source_faction_id == ConsoleGlobal.faction_id_ &&
            radius_indicator != 0) {
            WorldClimateWorldWin.GraphicWin::redraw();
        }
        if (terrain_visibility_count > 8 &&
            source_faction_id == ConsoleGlobal.faction_id_ &&
            (Players[source_faction_id].rule_flags & 0x80u) != 0) {
            interlude(0x29, 0, 1, 0);
        }

        if ((encounter_mask & 1) != 0) {
            encounter(source_faction_id, 0, source_x, source_y,
                      neutral_event_flag_a,
                      neutral_event_flag_b);
        }
        if (source_faction_id != 0 && effective_toggle) {
            for (int faction_id = 1; faction_id < 8; ++faction_id) {
                if (faction_id != source_faction_id &&
                    (encounter_mask & (1 << faction_id)) != 0) {
                    encounter(source_faction_id, faction_id, source_x,
                              source_y, 0, 0);
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
                + Vehs[veh_id].y * MapLongitude];
            // The original also compares the masked nibble against 0, which is vacuous.
            if ((tile->bit & BIT_BASE_IN_TILE) && (tile->val2 & 0xF) < MaxPlayerNum) {
                return veh_drop(veh_lift(veh_id), -2, -2);
            }
        }
        stack_sort(veh_id);
    } else {
        for (int i = veh_top(veh_id); i >= 0; i = Vehs[i].next_veh_id_stack) {
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
            i = veh_top(veh_id);
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
                        + Vehs[veh_id].y * MapLongitude];
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
                if (accept && (((1 << unit_faction) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) && !both_unk_200) {
                    accept = (Vehs[i].order == ORDER_SENTRY_BOARD)
                        ? (Vehs[i].waypoint_x[0] < 0) : is_open_ocean;
                } else if (accept) {
                    if (Vehs[i].state & VSTATE_UNK_40000) {
                        const Map *tile = &map_tiles()[(Vehs[i].x >> 1)
                            + Vehs[i].y * MapLongitude];
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
            i = next_veh_id;
        }
    }
    if (mode) {
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

// Every caller in the image calls this out of line.
#pragma auto_inline(off)
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
#pragma auto_inline(on)


// RECOVERY_ASSOCIATED: v1 switch_targets 0x005B9580 0x005B9BB8 0x005B9C08 0x005B95D7
/*
Purpose: Various unit stack related calculations based on type parameter (0-19) and conditions.
// ORIGINAL: 0x005B9580 ?stack_check@@YAHHHHHH@Z 0x005B9580-0x005B9BB8
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
                    // The pinned body calls veh_cargo(i) at 0x005B984D.
                    retn_val += veh_cargo(i);
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
