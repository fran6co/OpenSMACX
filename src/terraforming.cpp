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
// ORIGINAL: 0x004C9420 ?terraform_cost@@YAHHHH@Z 0x004C9420-0x004C96D6
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
        && static_cast<int>(climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
        cost *= 3;
    }
    cost *= 2;
    if (alt < ALT_SHORE_LINE) {
        cost *= 2;
        if (!GameTechAchieved[TECH_DOCAIR]) {
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
        int cursor_distance = cursor_dist(x, y, Bases[base_id].x, Bases[base_id].y);
        cost *= range(cursor_distance, 1, 100);
        if (base_id_prox >= 0 
            && !has_treaty(faction_id, Bases[base_id_prox].faction_id_current, DTREATY_PACT)) {
            int num_prox = (cursor_distance * (Bases[base_id_prox].population_size + 2)) / 3;
            int denom_prox = (cursor_dist(x, y, Bases[base_id_prox].x, Bases[base_id_prox].y)
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

/*
Purpose: Calculate the Former rate to perform terrain enhancements.
// ORIGINAL: 0x004C9A50 ?contribution@@YAHHH@Z 0x004C9A50-0x004C9AFB
// size      171 bytes
// prototype int (__cdecl ?contribution@@YAHHH@Z)(int vehID, int terraformID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0
// LEVER: `faction_id` cached in a local (the image loads it once alongside
//        `proto_id` and reuses it for both `has_project` calls, this body
//        was re-reading `Vehs[veh_id].faction_id` at each site).
// TRIED: `if/else` in place of the `? 4 : 2` ternary for the has_abil
//        result - VC6 compiles BOTH forms to the same branchless
//        `neg/sbb/and/add` idiom (any zero/nonzero boolean qualifies, not
//        just exactly-0-or-1), where the image keeps two real branches
//        with separate `mov edi, N` stores. Not a source-shape question.
//        The two inlined `has_project` sites also keep a `sete`+`test`+`je`
//        shape the image uses that this body's direct `cmp`/`jne` does not
//        reproduce. Best reached this pass: 3/66, MISMATCH.
Return Value: Terraforming speed
Status: Complete
*/
int __cdecl contribution(int veh_id, int terraform_id) {
    // CACHED, NOT RE-READ: the image loads Vehs[veh_id].faction_id ONCE,
    // alongside proto_id, and reuses it for both has_project() calls below.
    uint8_t faction_id = Vehs[veh_id].faction_id;
    int rate;
    // AN IF, NOT A TERNARY: the image branches to two separate `mov edi, N`
    // stores rather than computing 2/4 branchlessly.
    if (has_abil(Vehs[veh_id].proto_id, ABL_SUPER_TERRAFORMER)) {
        rate = 4;
    } else {
        rate = 2;
    }
    if (terraform_id == (ORDER_REMOVE_FUNGUS - 4) || terraform_id == (ORDER_PLANT_FUNGUS - 4)) {
        if (has_project(SP_XENOEMPATYH_DOME, faction_id)) {
            rate *= 2; // Doubled
        }
    } else if (has_project(SP_WEATHER_PARADIGM, faction_id)) {
        rate = (rate * 3) / 2; // +50%
    }
    return rate;
}

/*
Purpose: Check to see whether the specified faction can construct a specific terrain enhancement.
// ORIGINAL: 0x005BAB40 ?terrain_avail@@YAHHHH@Z 0x005BAB40-0x005BAC13
// size      211 bytes
// prototype int (__cdecl ?terrain_avail@@YAHHHH@Z)(int terraformID, BOOL isSea, int factionID)
// callers   5   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: the base-address split for `&Terraforming[terraform_id]
//        .preq_tech + is_sea` - the image computes `(is_sea + id*8)` as an
//        INDEX first, then a separate `[index*4 + 0x691880]` load; this
//        tree's `lea` folds the base INTO the index before the implied
//        `*4` (0x1a4620*4 == 0x691880, so it is the same address, just
//        computed with the constant added before the scale rather than
//        after). Tried the equivalent `reinterpret_cast<int*>(...)
//        [is_sea]` array-subscript spelling - VC6 canonicalises both to
//        the identical folded `lea`. Not a source-shape question found
//        this pass; best 3/86.
Return Value: Is terrain enhancement available to faction? true/false
Status: Complete
*/
BOOL __cdecl terrain_avail(int terraform_id, BOOL is_sea, int faction_id) {
    int preq_tech = *(&Terraforming[terraform_id].preq_tech + is_sea);
    if (preq_tech < TechNone || ((terraform_id == TERRA_RAISE_LAND
        || terraform_id == TERRA_LOWER_LAND) && GameRules & RULES_SCN_NO_TERRAFORMING)) {
        return false;
    }
    if (terraform_id >= TERRA_CONDENSER && terraform_id <= TERRA_LEVEL_TERRAIN
        && has_project(SP_WEATHER_PARADIGM, faction_id)) {
        return true;
    }
    return has_tech(preq_tech, faction_id);
}

/*
Purpose: Wrap an x coordinate on a round map, as can_terraform() wraps it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

The third private copy of this, after map.cpp's site_xrange() and game.cpp's
territory_xrange(), and for the same reason: the exported xrange() at 0048BEE0
tests the whole of MapIsFlat while can_terraform's base-radius census reads the
low BYTE and tests bit zero (`mov ebx, [94988Ch] / and ebx, 1`). The two agree
on the 0 and 1 the game stores there and disagree on everything else, and the
difference is in the original rather than in the transcription.
*/
static int terraform_xrange(int x) {
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
Purpose: Decide which Former order, if any, should be issued on the specified tile.
// ORIGINAL: 0x00565320 ?can_terraform@@YAHHHHHH@Z 0x00565320-0x00565F1E
// LEVER: real-bitmask-calls the two facility checks in the forest-value block
//   are `has_fac_built_call` (base.h), not `has_fac_built`. The image CALLS
//   `bitmask` at 0x0050BA00 twice, from 0x00565449 and 0x00565487; the plain
//   spelling folds `MEASURED inline bitmask` into a shift/and and this body
//   then made 26 calls against the image's 28 - `call_diff` FEWER. With the
//   forwarder the counts agree, 28 against 28, and best similarity moved
//   0.168 -> 0.207 at `/c /O2 /Gy /GR- /Oy- /GX`. That is a call the image
//   really makes, so the fix stands on its own regardless of the tier.
// TRIED: not attempted to byte-exactness this pass - out of budget, and
//   said so rather than half-ground. Measured 2026-08-22 over all ten flag
//   sets: best 0.214 at `/c /O2 /Ob0 /Gy /GR- /Oy- /GX`, 15 of 1113
//   instructions. The gap is SIZE, not one lever: this tree compiles 1,566
//   instructions against the image's 1,113, a 40% excess, across 51 differing
//   runs, and that shape says the image leaves out-of-line what this tree
//   inlines somewhere in the accessor chain rather than that any one
//   expression is spelled wrong. The divergence opens at instruction 3, on
//   the `map_loc` index: the image reads MapLongitude at the fixed 0x0068FAF0
//   while this tree reads it as a linker symbol, and the base-in-tile test
//   arrives as `mov ecx, 1; test cl, bl` instead of a folded memory-operand
//   test. Anyone picking this up should start from `osmx calls` plus
//   `inline_candidates.py` on the accessors this body reaches, not from the
//   listing: with 453 instructions of excess, the first divergence is not
//   where the cause is.
// size      3070 bytes
// prototype int (__cdecl ?can_terraform@@YAHHHHHH@Z)(int factionID, int xCoord, int yCoord, int, int baseID)
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0 0x004E6E50 0x0050BA00 0x00592030 0x005B9F20 0x005BAB40
Return Value: The ORDER_* to issue, or ORDER_NONE for none
Status: Complete

THE NAME IS A PREDICATE AND THE FUNCTION IS NOT ONE. It returns an OrderType id from veh.h -
ORDER_FARM, ORDER_MINE, ORDER_SOLAR_COLLECTOR, ORDER_PLANT_FOREST, ORDER_REMOVE_FUNGUS,
ORDER_PLANT_FUNGUS, ORDER_TERRAFORM_UP - or ORDER_NONE. ORDER_x is TERRA_x + 4, the same offset
contribution() above already uses, and every returned constant is guarded by the matching
Terraforming[] prerequisite: 4 by TERRA_FARM's, 6 by TERRA_MINE's, 7 by TERRA_SOLAR_TIDAL's,
8 by TERRA_FOREST's, 14 by TERRA_REMOVE_FUNGUS's, 15 by TERRA_PLANT_FUNGUS's, and 20 by
terrain_avail(TERRA_RAISE_LAND). Console::give_hints reads the answer as a value rather than a
truth, comparing it against 4 at 00513311.

ORDER_FARM covers the soil enricher as well as the farm: in this game the enricher is built by
issuing "build farm" on a tile that already has one, so the two share order 4.

WHERE IT SPLITS. A tile that already holds somebody's base is refused outright. Everything after
that divides on the tile's own altitude at ALT_BIT_SHORE_LINE.

BELOW THE SHORE LINE. Deep water - anything under ALT_BIT_OCEAN_SHELF - is only ever raised.
On the shelf, fungus is removed if the faction's fungus yield is worth less than 4 and the human
player has asked for automatic fungus removal; bare shelf gets a kelp farm, and failing that a
mining platform or a tidal harness chosen by censusing the owning base's own 21-tile radius for
mines and harnesses already standing on water. A shelf tile that already has a farm and not yet
an enricher gets order 4 again.

ABOVE IT. Foreign territory without a pact refuses. Fungus is removed unless the base's
eco-damage-adjusted terrain value says the fungus is worth more. A monolith refuses; so does an
existing forest that is worth more than the terrain. What is left runs a cascade of
solar / mine / farm / enricher rules over crop yield, rockiness, rainfall, elevation, resource
bonus and the owning base's three planning bits, and then asks two closing questions: is a
forest worth more than the terrain, and after that is fungus worth more than the forest.

force_improve is the fourth parameter and it is the AI's "I already want this tile improved"
flag - base_terraform passes 1, Console::give_hints passes 0, and enemy_move passes
at_goal(faction_id, AI_GOAL_TERRAFORM_LAND, x, y). Each of its four reads turns a marginal tile
into a definite order.

Verification note: five kinds of unreachable code are transcribed as written rather than
deleted, and a mutation sweep will report them as survivors.

  1. elev CANNOT BE ZERO here. The land tail is entered only when (climate & 0xE0) >=
     ALT_BIT_SHORE_LINE, which is alt_at(x, y) >= 3, and elev is alt_at(x, y) - 2 floored at
     zero, so elev >= 1 always. That makes `!elev` at the solar rule and at the rolling-mine
     rule, `elev == 0` in the plant_value adjustment, and the `elev != 0` forest rejection all
     constant. The last one is the one that matters: it means a tile that already has a farm can
     never be forested, and the rainfall and is_human tests behind it are unreachable. True of
     the original at 00565A23, 00565ADC, 00565B64, 00565DC5 and 00565E4C, not just of this
     transcription.

  2. `rockiness < ROCKINESS_ROCKY` inside the BSTATE_UNK_2000 mine rule is reached only after
     `rockiness > ROCKINESS_ROLLING` has already been excluded (00565BB2), so it is constant.

  3. The owner nibble is masked to 0xF before `owner >= 0` is asked (0056535A), so that half of
     the base guard is constant.

  4. A DEAD COMPARISON, collapsed rather than transcribed. At 00565A7B the original computes
     `(crop >= has_farm) + 2` and branches on the result being zero; the value is 2 or 3, so the
     branch is never taken and both arms of the enclosing `if (crop)` reach order = ORDER_MINE.
     The block therefore is `if (bonus == 2 || rockiness == ROCKINESS_ROCKY) order = ORDER_MINE`,
     which is what is written below. The comparison has no side effect - only flags - so unlike
     a dead call there is nothing to preserve. Its shape shows the intent: has_farm is the RAW
     `bit & BIT_FARM`, 0 or 0x8000, so `crop >= has_farm` was meant to be the
     `crop >= (has_farm ? 2 : 1)` used three times elsewhere in this function.

  5. Two calls whose results the original throws away, kept because they are what it executes
     and because deleting one is a purity claim about another translation unit:
     bonus_at(x, y, 0) on the sea path (005655FD, EAX overwritten at 00565609) and
     has_tech(Terraforming[TERRA_FARM].preq_tech, faction_id) on the land path (005659F3, EAX
     overwritten at 005659F8 and the same question asked again for real at 00565BF1).

Bug note: both fungus-removal paths read Terraforming[TERRA_REMOVE_FUNGUS].preq_tech_SEA
(006919C4). On the sea path that is right. On the land path (0056596A) it should be preq_tech at
006919C0, and every other land/sea pair in the function is correctly discriminated. Stock
terraform.txt gives Remove Fungus the same technology for both, so it is invisible in a default
game and visible in a mod that splits them. Transcribed as written.
*/
int __cdecl can_terraform(int faction_id, int x, int y, int force_improve, int base_id) {
    Map *tile = map_loc(x, y);
    if (tile->bit & BIT_BASE_IN_TILE) {
        int owner = (int)owner_at(x, y);
        if (owner < 8 && owner >= 0) {
            return ORDER_NONE;
        }
    }
    uint32_t bit = tile->bit;
    // Fungus in deep water does not count: the altitude is the tile's own.
    BOOL has_fungus = (bit & BIT_FUNGUS)
        && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF;
    uint32_t has_mine = bit & BIT_MINE;
    uint32_t has_solar = bit & BIT_SOLAR_TIDAL;
    uint32_t has_farm = bit & BIT_FARM;
    uint32_t has_soil_enricher = bit & BIT_SOIL_ENRICHER;
    uint32_t has_forest = bit & BIT_FOREST;
    BOOL is_special = (bit & (BIT_CONDENSER | BIT_ECH_MIRROR | BIT_THERMAL_BORE))
        || (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO)
            && !(tile->bit2 & 0xFF000000));  // landmark code zero, ie. the volcano's own tile
    int fungus_value = PlayersData[faction_id].tech_fungus_nutrient
        + PlayersData[faction_id].tech_fungus_mineral
        + PlayersData[faction_id].tech_fungus_energy;
    int forest_value = ResourceInfo[RSCINFO_FOREST_SQ].nutrients
        + ResourceInfo[RSCINFO_FOREST_SQ].minerals
        + ResourceInfo[RSCINFO_FOREST_SQ].energy;
    if (base_id >= 0) {
        // REAL bitmask() CALLS, not the folded shift/and: the image calls
        // 0x0050BA00 twice, at 0x00565449 and 0x00565487, which is what
        // `has_fac_built_call` (base.h) is for. Plain `has_fac_built` inlines
        // `MEASURED inline bitmask` and this body then makes 26 calls against
        // the image's 28.
        if (has_fac_built_call(FAC_TREE_FARM, base_id)) {
            forest_value++;
        }
        if (has_fac_built_call(FAC_HYBRID_FOREST, base_id)) {
            forest_value += 2;
        }
    }
    if (PlayersData[faction_id].ai_growth && !PlayersData[faction_id].ai_power
        && !PlayersData[faction_id].ai_wealth && !PlayersData[faction_id].ai_tech
        && PlayersData[faction_id].ai_fight < 0 && !is_human(faction_id)) {
        forest_value++;   // a growth-only AI plan likes forest one point more
    }
    int altitude = altitude_at(x, y);
    if (altitude < ALT_BIT_SHORE_LINE) {
        // ---- water ---------------------------------------------------------
        if (has_fungus) {
            if (fungus_value >= 4) {
                return ORDER_NONE;
            }
            if (is_human(faction_id)
                && !(GameMorePreferences & MPREF_AUTO_FORMER_REMOVE_FUNGUS)) {
                return ORDER_NONE;
            }
            return has_tech(Terraforming[TERRA_REMOVE_FUNGUS].preq_tech_sea, faction_id)
                ? ORDER_REMOVE_FUNGUS : ORDER_NONE;
        }
        if (altitude < ALT_BIT_OCEAN_SHELF) {
            return terrain_avail(TERRA_RAISE_LAND, true, faction_id)
                ? ORDER_TERRAFORM_UP : ORDER_NONE;
        }
        if (bit & BIT_THERMAL_BORE) {
            return ORDER_NONE;
        }
        if (!has_farm
            && has_tech(Terraforming[TERRA_FARM].preq_tech_sea, faction_id)) {
            return ORDER_FARM;   // kelp farm
        }
        if (!has_solar && !has_mine && !is_special) {
            bonus_at(x, y, 0);   // result discarded by the original, see the note above
            if (base_id >= 0) {
                if ((Bases[base_id].state & BSTATE_UNK_2000)
                    && has_tech(Terraforming[TERRA_MINE].preq_tech_sea, faction_id)) {
                    return ORDER_MINE;
                }
                if ((Bases[base_id].state & BSTATE_UNK_1000)
                    && has_tech(Terraforming[TERRA_SOLAR_TIDAL].preq_tech_sea, faction_id)) {
                    return ORDER_SOLAR_COLLECTOR;
                }
                if (((Bases[base_id].state & BSTATE_UNK_4000)
                        || Bases[base_id].nutrient_surplus < 2)
                    && has_tech(Terraforming[TERRA_SOLAR_TIDAL].preq_tech_sea, faction_id)
                    && Rules->tgl_nutrient_effect_with_mine < 0) {
                    return ORDER_SOLAR_COLLECTOR;
                }
                // Census the base's own radius for water improvements already standing.
                int mine_count = (Rules->tgl_nutrient_effect_with_mine < 0) ? 1 : 0;
                int solar_count = 0;
                int base_x = Bases[base_id].x;
                int base_y = Bases[base_id].y;
                for (int i = 0; i < 21; i++) {
                    int x_radius = terraform_xrange(base_x + RadiusOffsetX[i]);
                    int y_radius = base_y + RadiusOffsetY[i];
                    if (!on_map(x_radius, y_radius)) {
                        continue;
                    }
                    Map *radius_tile = map_loc(x_radius, y_radius);
                    if ((radius_tile->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
                        continue;   // land
                    }
                    if (radius_tile->bit & BIT_MINE) {
                        mine_count++;
                    }
                    if (radius_tile->bit & BIT_SOLAR_TIDAL) {
                        solar_count++;
                    }
                }
                if (mine_count
                        + (has_tech(Terraforming[TERRA_MINE].preq_tech_sea, faction_id) ? 0 : 1)
                        > solar_count
                    && has_tech(Terraforming[TERRA_SOLAR_TIDAL].preq_tech_sea, faction_id)) {
                    return ORDER_SOLAR_COLLECTOR;
                }
            }
            if (has_tech(Terraforming[TERRA_MINE].preq_tech_sea, faction_id)
                && (Rules->tgl_nutrient_effect_with_mine >= 0
                    || (base_id >= 0 && Bases[base_id].nutrient_surplus > 1
                        && !(Bases[base_id].state & BSTATE_UNK_4000)))) {
                return ORDER_MINE;
            }
        }
        if (has_farm && !has_soil_enricher
            && has_tech(Terraforming[TERRA_SOIL_ENR].preq_tech_sea, faction_id)) {
            return ORDER_FARM;   // upgrade the kelp farm to an enricher
        }
        return ORDER_NONE;
    }
    // ---- land --------------------------------------------------------------
    // Rainfall runs arid, moist, rainy.
    int rainfall = (tile->climate >> 3) & 3;
    int rockiness = tile->val3 >> 6;           // ROCKINESS_FLAT / ROLLING / ROCKY
    int order = ORDER_NONE;
    int crop = crop_yield(faction_id, -1, x, y, 0);
    int elev = alt_at(x, y) - 2;               // levels above the shore line; never zero
    if (elev < 0) {
        elev = 0;
    }
    // The bonus runs none, nutrient, mineral, energy.
    int bonus = bonus_at(x, y, 0);
    int value = elev + rainfall + rockiness + 1;
    if (has_tech(Terraforming[TERRA_SOIL_ENR].preq_tech, faction_id) && has_farm) {
        value++;
    }
    int territory = whose_territory(faction_id, x, y, NULL, false);
    if (territory >= 0 && territory != faction_id
        && !(PlayersData[faction_id].diplo_treaties[territory] & DTREATY_PACT)) {
        return ORDER_NONE;
    }
    if (has_fungus) {
        if (base_id >= 0 && Bases[base_id].eco_damage / 2 + value <= fungus_value) {
            return ORDER_NONE;   // the fungus is worth more than the terrain under it
        }
        if (is_human(faction_id)
            && !(GameMorePreferences & MPREF_AUTO_FORMER_REMOVE_FUNGUS)) {
            return ORDER_NONE;
        }
        return has_tech(Terraforming[TERRA_REMOVE_FUNGUS].preq_tech_sea, faction_id)
            ? ORDER_REMOVE_FUNGUS : ORDER_NONE;
    }
    if (bit & BIT_MONOLITH) {
        return ORDER_NONE;
    }
    if (has_forest && value <= forest_value) {
        return ORDER_NONE;
    }
    BOOL mine_tech = has_tech(Terraforming[TERRA_MINE].preq_tech, faction_id);
    BOOL solar_tech = has_tech(Terraforming[TERRA_SOLAR_TIDAL].preq_tech, faction_id);
    has_tech(Terraforming[TERRA_FARM].preq_tech, faction_id);   // result discarded, see above
    if (!has_mine && !has_solar && !is_special) {
        if (solar_tech && elev
            && (((rockiness == ROCKINESS_ROLLING || elev >= 2)
                    && crop >= (has_farm ? 2 : 1))
                || force_improve)) {
            order = ORDER_SOLAR_COLLECTOR;
        }
        if (mine_tech) {
            if (bonus == 2 /* mineral */ || rockiness == ROCKINESS_ROCKY) {
                order = ORDER_MINE;
            }
            BOOL rolling_rule = false;
            BOOL farm_rule = false;
            if (bonus == 2 && rockiness == ROCKINESS_ROCKY) {
                farm_rule = true;
            } else if (rockiness == ROCKINESS_ROCKY
                       || Rules->tgl_nutrient_effect_with_mine >= 0) {
                rolling_rule = true;
            } else {
                int worked = crop + (has_farm ? 0 : 1);
                if (worked <= 1) {
                    rolling_rule = true;
                } else if (rockiness == ROCKINESS_FLAT && worked > 2) {
                    farm_rule = true;
                }
            }
            if (rolling_rule) {
                if (rockiness == ROCKINESS_ROLLING && !elev) {
                    order = ORDER_MINE;
                }
                if (bonus == 3 /* energy */) {
                    order = ORDER_MINE;
                }
                farm_rule = true;
            }
            if (farm_rule && force_improve && has_farm && !order) {
                order = ORDER_MINE;
            }
        }
        if (force_improve && has_farm && !order && elev && solar_tech) {
            order = ORDER_SOLAR_COLLECTOR;
        }
        if (base_id >= 0) {
            uint32_t state = Bases[base_id].state;
            if ((state & BSTATE_UNK_1000) && solar_tech && elev
                && (crop >= (has_farm ? 2 : 1) || elev > 1)) {
                order = ORDER_SOLAR_COLLECTOR;
            }
            if ((state & BSTATE_UNK_2000) && mine_tech) {
                if (crop >= (has_farm ? 3 : 2)) {
                    order = ORDER_MINE;
                }
                BOOL wants_mine = rockiness > ROCKINESS_ROLLING || bonus == 2
                    || (rainfall != 0 && rockiness < ROCKINESS_ROCKY && elev < 2);
                if (wants_mine && (has_farm || crop >= 2)) {
                    order = ORDER_MINE;
                }
            }
        }
    }
    if (has_tech(Terraforming[TERRA_FARM].preq_tech, faction_id) && !has_farm
        && rockiness < ROCKINESS_ROCKY
        && !(bit & BIT_THERMAL_BORE)
        && (tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) != BIT2_VOLCANO
        && !(bit & BIT_ECH_MIRROR)
        && (crop < 2 || has_tech(Rules->tech_three_nutrients_sqr, faction_id)
            || bonus == 1 /* nutrient */)) {
        if (has_solar || has_mine) {
            order = ORDER_FARM;
        }
        BOOL forced_farm = false;
        if (base_id >= 0 && (Bases[base_id].state & BSTATE_UNK_4000)) {
            order = ORDER_FARM;
        } else if (order == ORDER_SOLAR_COLLECTOR) {
            if (crop < 2) {
                order = ORDER_FARM;
            }
        } else if (order == ORDER_NONE) {
            forced_farm = true;
        }
        if (forced_farm || rainfall < 2) {
            order = ORDER_FARM;
        }
    }
    if (has_tech(Terraforming[TERRA_SOIL_ENR].preq_tech, faction_id) && has_farm
        && !has_soil_enricher) {
        BOOL enrich = false;
        if (crop > 1
            && (crop != 2 || has_tech(Rules->tech_three_nutrients_sqr, faction_id))) {
            if (has_solar || has_mine) {
                order = ORDER_FARM;
            }
            if (base_id >= 0 && (Bases[base_id].state & BSTATE_UNK_4000)) {
                enrich = true;
            }
        }
        if (!enrich && order == ORDER_NONE && (force_improve || crop > 1)) {
            enrich = true;
        }
        if (enrich) {
            order = ORDER_FARM;
        }
    }
    // What the terrain would be worth if it were planted, scored separately from `value`.
    int plant_value = elev + rainfall + rockiness + 1;
    if (has_tech(Terraforming[TERRA_SOIL_ENR].preq_tech, faction_id) && has_farm) {
        plant_value++;
    }
    if (base_id >= 0) {
        uint32_t state = Bases[base_id].state;
        if (state & BSTATE_UNK_4000) {
            if (rainfall > 1) {
                plant_value++;
            }
            if (rainfall == 0) {
                plant_value--;
            }
        }
        if (state & BSTATE_UNK_1000) {
            if (elev > 1) {
                plant_value++;
            }
            if (elev == 0) {
                plant_value--;
            }
        }
        if (state & BSTATE_UNK_2000) {
            if (rockiness == ROCKINESS_FLAT) {
                plant_value--;
            }
        }
        if (!(state & (BSTATE_UNK_2000 | BSTATE_UNK_4000))) {
            if (has_solar) {
                plant_value++;
            }
        }
    }
    if (rainfall && elev && rockiness == ROCKINESS_ROLLING && TurnCurrentNum < 50) {
        plant_value++;
    }
    int result = order;
    if (has_tech(Terraforming[TERRA_FOREST].preq_tech, faction_id)
        && rockiness < ROCKINESS_ROCKY && !has_mine && !has_forest
        && (!has_farm || (!elev && rainfall < 2 && !is_human(faction_id)))
        && !is_special
        && (tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) != BIT2_VOLCANO
        && plant_value < forest_value
        && (!is_human(faction_id)
            || (faction_id == LocalFaction
                && (GamePreferences & PREF_AUTO_FORMER_PLANT_FORESTS)))) {
        result = ORDER_PLANT_FOREST;
    }
    if (has_tech(Terraforming[TERRA_PLANT_FUNGUS].preq_tech, faction_id)
        && !is_human(faction_id)) {
        if (rockiness == ROCKINESS_ROCKY) {
            plant_value += has_mine ? 2 : 1;
        }
        if (!is_special) {
            plant_value++;
            if (plant_value < fungus_value) {
                result = ORDER_PLANT_FUNGUS;
            }
        }
    }
    return result;
}
