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
#include "fx.h"
#include "wave.h"
#include "console.h"
#include "netmsg.h"
#include "netdaemon.h"
#include "tutwin.h"
#include "sprite.h"


/*
Purpose: Select the live road or magtube terraforming rate from tile mask 0x4,
         then add the contributions selected by the tile flags, climate
         and upper val3 bits. The final gameplay purpose remains unresolved.

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C99D0 sub_4c99d0 0x004C99D0-0x004C9A43 FILE
// symbol    _sub_4c99d0
// size      115 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the selected terraforming rate plus tile-derived contributions
Status: Coverage transcription - unclaimed body
*/
extern "C" int __cdecl sub_4c99d0(int x_coord, int y_coord) {
    const Map *tile = map_tiles() + MapLongitude * y_coord + (x_coord >> 1);
    const uint32_t bits = tile->bit;
    const int selected = (bits & BIT_ROAD) ? TERRA_MAGTUBE : TERRA_ROAD;
    int result = Terraforming[selected].rate;
    if ((bits & 0x80) != 0) {
        result++;
    }
    if ((bits & 0x20) != 0 && (tile->climate & ~0x1F) >= 0x40) {
        result += 2;
    }
    if ((bits & 0x200000) != 0) {
        return result + 2;
    }
    return result + (tile->val3 >> 6);
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
    const int faction_id = Vehs[veh_id].faction_id;
    int rate = 2;
    // AN IF, NOT A TERNARY: the image branches to two separate `mov edi, N`
    // stores rather than computing 2/4 branchlessly.
    if (has_abil(Vehs[veh_id].proto_id, ABL_SUPER_TERRAFORMER)) {
        rate = 4;
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

// ===== bindings the terraform verb needs =====

// 0x009A64B4, two dwords: the TERRA_* bits the local player has already been
// shown a first-time popup for, [0] land and [1] sea - indexed by the same
// not-ocean test that picks Terraforming[]'s preq_tech from its _sea twin.
// 0x009A64B4

// 0x009B22DC and 0x009B22E0 close the run map.cpp binds as
// MapBaseIdClosestSubmergedVeh[8] (0x009B22BC) and hold two ids the terraform
// epilogue reads: the faction whose territory the tile sits in, and the base
// that lost land to the raise/lower. Both are -1 when nobody is involved.
int TerraformTileFaction;    // 0x009B22DC
int TerraformDrownedBaseId;  // 0x009B22E0

// 0x0090DB84. A latch: the first terraform a unit completes with no order
// queued raises it and plays the unit-improvement sting, and nothing here
// clears it.
uint32_t VehTerraformStingLatch;  // 0x0090DB84

// The native base-id reads at 0x009A651C, 0x009A6534 and 0x009A6580 are slots
// 2/8/27 of the 64-dword SecretProject owner at 0x009A6514. The real nested
// owner is GameDataStateStorage.fields_.secret_project_; its existing named
// accessors preserve those exact slots without executable-address pointers.

// The phrase keys the terraform popups and the tutorial map call are titled
// with. All are file-backed .data, all read only here.
// RECOVERY_OBJECT: v2 0x00687B80 0x00687B8D ?VehLabelBOREHOLEMINE@@3PADA 0x004C9B00
char VehLabelBOREHOLEMINE[] = "BOREHOLEMINE";
// RECOVERY_OBJECT: v2 0x00687B90 0x00687B9E ?VehLabelHARVESTFOREST@@3PADA 0x004C9B00
char VehLabelHARVESTFOREST[] = "HARVESTFOREST";
// RECOVERY_OBJECT: v2 0x00687BA0 0x00687BAA ?VehLabelTERRAMINE@@3PADA 0x004C9B00
char VehLabelTERRAMINE[] = "TERRAMINE";
// RECOVERY_OBJECT: v2 0x00687BAC 0x00687BB7 ?VehLabelTERRAYOURS@@3PADA 0x004C9B00
char VehLabelTERRAYOURS[] = "TERRAYOURS";
// RECOVERY_OBJECT: v2 0x00687BB8 0x00687BC2 ?VehLabelDROWNMINE@@3PADA 0x004C9B00
char VehLabelDROWNMINE[] = "DROWNMINE";
// RECOVERY_OBJECT: v2 0x00687BC4 0x00687BCF ?VehLabelDROWNYOURS@@3PADA 0x004C9B00
char VehLabelDROWNYOURS[] = "DROWNYOURS";
// RECOVERY_OBJECT: v2 0x00687BD0 0x00687BD9 ?VehLabelMADEFARM@@3PADA 0x004C9B00
char VehLabelMADEFARM[] = "MADEFARM";
// RECOVERY_OBJECT: v2 0x00687BDC 0x00687BE5 ?VehLabelMADESOIL@@3PADA 0x004C9B00
char VehLabelMADESOIL[] = "MADESOIL";
// RECOVERY_OBJECT: v2 0x00687BE8 0x00687BF1 ?VehLabelMADEMINE@@3PADA 0x004C9B00
char VehLabelMADEMINE[] = "MADEMINE";
// RECOVERY_OBJECT: v2 0x00687BF4 0x00687BFE ?VehLabelMADESOLAR@@3PADA 0x004C9B00
char VehLabelMADESOLAR[] = "MADESOLAR";
// RECOVERY_OBJECT: v2 0x00687C00 0x00687C0B ?VehLabelMADEFOREST@@3PADA 0x004C9B00
char VehLabelMADEFOREST[] = "MADEFOREST";
// RECOVERY_OBJECT: v2 0x00687C0C 0x00687C15 ?VehLabelMADEROAD@@3PADA 0x004C9B00
char VehLabelMADEROAD[] = "MADEROAD";
// RECOVERY_OBJECT: v2 0x00687C18 0x00687C21 ?VehLabelMADETUBE@@3PADA 0x004C9B00
char VehLabelMADETUBE[] = "MADETUBE";
// RECOVERY_OBJECT: v2 0x00687C24 0x00687C2F ?VehLabelMADESENSOR@@3PADA 0x004C9B00
char VehLabelMADESENSOR[] = "MADESENSOR";
// RECOVERY_OBJECT: v2 0x00687C30 0x00687C3C ?VehLabelMADEAQUIFER@@3PADA 0x004C9B00
char VehLabelMADEAQUIFER[] = "MADEAQUIFER";
// RECOVERY_OBJECT: v2 0x00687C3C 0x00687C4A ?VehLabelMADECONDENSER@@3PADA 0x004C9B00
char VehLabelMADECONDENSER[] = "MADECONDENSER";
// RECOVERY_OBJECT: v2 0x00687C4C 0x00687C59 ?VehLabelMADEBOREHOLE@@3PADA 0x004C9B00
char VehLabelMADEBOREHOLE[] = "MADEBOREHOLE";
// RECOVERY_OBJECT: v2 0x00687C5C 0x00687C68 ?VehLabelMADESEAFARM@@3PADA 0x004C9B00
char VehLabelMADESEAFARM[] = "MADESEAFARM";
// RECOVERY_OBJECT: v2 0x00687C68 0x00687C74 ?VehLabelMADESEAMINE@@3PADA 0x004C9B00
char VehLabelMADESEAMINE[] = "MADESEAMINE";
// RECOVERY_OBJECT: v2 0x00687C74 0x00687C81 ?VehLabelMADESEASOLAR@@3PADA 0x004C9B00
char VehLabelMADESEASOLAR[] = "MADESEASOLAR";

// The four facility sprites the first-time tutorial popup draws beside its
// text. sprite.cpp owns the objects: the farm and soil enricher runs are five
// 0x2C-stride Sprites and the image hands the tutorial the COMPLETED tile,
// which is element [3] of each run (0x00791FC8 / 0x0079A710 + 3 * 0x2C); the
// mine and solar collector are single Sprites of their own.
extern Sprite g_TER1_FARM_SPRITES[5];             // 0x00791FC8, sprite.cpp
extern Sprite g_TER1_SOIL_ENRICHER_SPRITES[5];    // 0x0079A710, sprite.cpp
extern Sprite g_TER1_MINE_SPRITE;                 // 0x00776A50, sprite.cpp
extern Sprite g_TER1_SOLAR_COLLECTOR_SPRITE;      // 0x007991F8, sprite.cpp
static Sprite *const VehSpriteFARM = &g_TER1_FARM_SPRITES[3];
static Sprite *const VehSpriteSOIL = &g_TER1_SOIL_ENRICHER_SPRITES[3];
static Sprite *const VehSpriteMINE = &g_TER1_MINE_SPRITE;
static Sprite *const VehSpriteSOLAR = &g_TER1_SOLAR_COLLECTOR_SPRITE;

/*
Purpose: Perform one terraforming action on a unit's tile, over its stack.
// ORIGINAL: 0x004C9B00 ?action_terraform@@YAHHHH@Z 0x004C9B00-0x004CA76B
// size      3179 bytes
// prototype int (__cdecl ?action_terraform@@YAHHHH@Z)(int veh_id, int action, int do_it)
// callers   8   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// HOMED from src/unrecovered/004c9b00.cpp, 2026-09-04. `action` is the TERRA_*
// index (the order byte stores action + 4); `do_it` distinguishes the query
// pass, which only totals the stack's progress, from the pass that spends it.
// Its `g_0095284d` byte is Vehs[].terraforming_turns (+0x25), `g_00952850`
// Vehs[].moves_expended (+0x28), `g_00952856` Vehs[].home_base_id (+0x2E) and
// `g_0095285a`/`g_00952858` prev/next_veh_id_stack (+0x32/+0x30). Its
// `g_0069188x` triple is Terraforming[].bit / .bit_incompatible / .rate
// (0x10/0x14/0x18, 0x20 stride off 0x00691878).
//
// BUG IN THE ORIGINAL: the drowned-base epilogue gates on
// MapBaseIdClosestSubmergedVeh[LocalFaction] (0x004CA466) but then reads
// MapBaseIdClosestSubmergedVeh[faction] (0x004CA47E). For the local player the
// two are the same slot and the slip is invisible; for any other faction the
// guard answers a different slot than the one used. Transcribed as written -
// both sides are the image's.
Return Value: 1 when the action ran, 0 when it only reported progress.
Status: MISMATCH.
*/
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C9B00 0x004CA76C 0x004CA78C 0x004C9C0D
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C9B00 0x004CA78C 0x004CA7B0 0x004CA0E6
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C9B00 0x004CA7B0 0x004CA7F0 0x004CA5A9
int __cdecl action_terraform(int veh_id, int action, int do_it) {
    const int veh_count_before = GameDataStateStorage.fields_.vehicle_current_count_;
    const int y = Vehs[veh_id].y;
    const int x = Vehs[veh_id].x;
    const uint8_t faction = Vehs[veh_id].faction_id;
    Map *tile = map_tiles()
        + ((x >> 1) + y * MapLongitude);
    const int not_ocean = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;

    if (action == TERRA_ROAD) {
        if (tile->bit & BIT_ROAD) {
            if (has_tech(not_ocean != 0
                                  ? Terraforming[TERRA_MAGTUBE].prerequisite_tech[1]
                                  : Terraforming[TERRA_MAGTUBE].prerequisite_tech[0],
                              faction)) {
                action = TERRA_MAGTUBE;
            }
        }
    } else if (action == TERRA_FARM && (tile->bit & BIT_FARM) != 0
               && (tile->climate & 0xE0) > 0x5F) {
        if (has_tech(not_ocean != 0
                              ? Terraforming[TERRA_FARM].prerequisite_tech[1]
                              : Terraforming[TERRA_FARM].prerequisite_tech[0],
                          faction)) {
            action = TERRA_SOIL_ENR;
        }
    }

    if (do_it != 0) {
        Vehs[veh_id].order = (uint8_t)(action + 4);
    }

    int base_time = Terraforming[action].rate;
    switch (action) {
    case TERRA_MINE: {
        const uint32_t bonus = ((uint32_t)(tile->val3 >> 6) * base_time) / 2;
        base_time = base_time + bonus;
        break;
    }
    case TERRA_ROAD:
    case TERRA_MAGTUBE:
        base_time = sub_4c99d0(x, y);
        break;
    case TERRA_REMOVE_FUNGUS:
        if (has_abil(Vehs[veh_id].proto_id, 0x2000)) {
            base_time = base_time / 2;
        }
        break;
    default:
        break;
    }

    const uint8_t faction_bit = (uint8_t)(1 << (faction & 0x1F));
    if (!(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) && GameDataStateStorage.fields_.diff_level_current_ > 3
        && base_time > 3) {
        base_time--;
    }

    int turns_accum = 0;
    int progress_accum = 0;
    if (veh_id >= 0) {
        int stack_bottom = veh_id;
        int16_t prev = Vehs[stack_bottom].prev_veh_id_stack;
        while (prev >= 0) {
            stack_bottom = prev;
            prev = Vehs[stack_bottom].prev_veh_id_stack;
        }
        for (int v = stack_bottom; v >= 0; v = Vehs[v].next_veh_id_stack) {
            if (v != veh_id && (int8_t)Vehs[v].order == (int8_t)(action + 4)) {
                if (do_it == 0) {
                    turns_accum += contribution(v, action);
                    progress_accum += Vehs[v].terraforming_turns;
                } else {
                    Vehs[veh_id].terraforming_turns =
                        Vehs[veh_id].terraforming_turns
                        + Vehs[v].terraforming_turns;
                    Vehs[v].terraforming_turns = 0;
                }
            }
        }
    }

    {
        turns_accum += contribution(veh_id, action);
        progress_accum += Vehs[veh_id].terraforming_turns;
    }

    if (do_it == 0) {
        const int cur_speed = speed(veh_id, 0);
        const int since_move = cur_speed - Vehs[veh_id].moves_expended;
        if (since_move >= 0
            && (since_move > 999 || cur_speed != Vehs[veh_id].moves_expended)) {
            progress_accum += turns_accum;
        }
        progress_accum = base_time * 2 - progress_accum;
        if (turns_accum < 1) {
            turns_accum = 1;
        } else if (turns_accum > 0xFF) {
            return (progress_accum + 0xFE) / 0xFF;
        }
        return (progress_accum - 1 + turns_accum) / turns_accum;
    }

    veh_skip(veh_id);
    const uint8_t progress_before = Vehs[veh_id].terraforming_turns;
    Vehs[veh_id].terraforming_turns = progress_before + (uint8_t)turns_accum;
    if ((tile->bit & BIT_BASE_IN_TILE) != 0 && (tile->val2 & 0xF) < 8) {
        Vehs[veh_id].order = ORDER_NONE;
        return 0;
    }
    if ((int)(uint8_t)(progress_before + (uint8_t)turns_accum) < base_time * 2) {
        draw_tile(x, y, 2);
        TutWinBaseWindow.check_veh(veh_id);
        return 0;
    }
    Vehs[veh_id].terraforming_turns = 0;
    if (veh_id >= 0) {
        int stack_bottom = veh_id;
        int16_t prev = Vehs[stack_bottom].prev_veh_id_stack;
        while (prev >= 0) {
            stack_bottom = prev;
            prev = Vehs[stack_bottom].prev_veh_id_stack;
        }
        for (int v = stack_bottom; v >= 0; v = Vehs[v].next_veh_id_stack) {
            if ((int8_t)Vehs[v].order == (int8_t)(action + 4)) {
                Vehs[v].order = ORDER_NONE;
            }
        }
    }

    const uint32_t tile_flags = tile->bit;
    bit_set(x, y, Terraforming[action].bit_incompatible, 0);
    bit_set(x, y, Terraforming[action].bit, 1);
    synch_bit(x, y, faction);
    switch (action) {
    case TERRA_REMOVE_FUNGUS:
    case TERRA_PLANT_FUNGUS:
        site_radius(x, y, 0);
        break;
    case TERRA_CONDENSER:
    case TERRA_THERMAL_BORE:
        site_radius(x, y, 0);
        world_climate();
        draw_map(1);
        break;
    case TERRA_AQUIFER:
        bit_set(x, y, 0x100, 1);
        world_climate();
        draw_map(1);
        break;
    case TERRA_RAISE_LAND:
        world_raise_alt(x, y);
        world_climate();
        draw_map(1);
        break;
    case TERRA_LOWER_LAND:
        world_lower_alt(x, y);
        world_climate();
        draw_map(1);
        break;
    case TERRA_LEVEL_TERRAIN: {
        uint8_t rocky = tile->val3;
        if (rocky & 0xC0) {
            rocky_set(x, y, (rocky >> 6) - 1);
            site_radius(x, y, 0);
        }
        break;
    }
    default:
        break;
    }

    int base_idx = -1;
    if ((Terraforming[action].bit & 0x1208050) != 0) {
        base_idx = base_find(x, y, faction);
        if (base_idx >= 0 && BaseFindDist < 3) {
            Bases[base_idx].state |= 0x100;
        }
        if (action == TERRA_THERMAL_BORE && faction == ConsoleGlobal.faction_id_
            && base_idx >= 0 && Bases[base_idx].faction_id_current == faction) {
            parse_says(0, Bases[base_idx].name_string, -1, -1);
            NetMsgPrimary.pop(VehLabelBOREHOLEMINE, 5000, 0, 0);
        }
    }

    if ((tile_flags & BIT_FOREST) == 0
        || ((map_tiles()
             + ((x >> 1) + y * MapLongitude))
                ->bit
            & BIT_FOREST)
            != 0
        || ((map_tiles()
             + ((x >> 1) + y * MapLongitude))
                ->climate
            & 0xE0)
            < ALT_BIT_SHORE_LINE) {
        if (action == TERRA_RAISE_LAND || action == TERRA_LOWER_LAND
            || action == TERRA_CONDENSER) {
            goto forest_shared;
        }
    } else if (action != TERRA_RAISE_LAND) {
        int near_base = Vehs[veh_id].home_base_id;
        if (near_base < 0) {
            near_base = base_find(x, y, faction);
        }
        if (near_base >= 0) {
            const int forest_val = Bases[near_base].minerals_accumulated
                + Rules.minerals_harvesting_forest;
            Bases[near_base].minerals_accumulated = forest_val;
            Bases[near_base].minerals_accumulated_2 = forest_val;
            Bases[near_base].production_id_last =
                Bases[near_base].queue_production_id[0];
            if (faction == ConsoleGlobal.faction_id_ && (faction_bit & GameDataStateStorage.fields_.lock_masks_.enable_mask_) != 0
                && Rules.minerals_harvesting_forest != 0) {
                parse_says(0, Bases[near_base].name_string, -1, -1);
                parse_num(0, Rules.minerals_harvesting_forest);
                NetMsgPrimary.pop(VehLabelHARVESTFOREST, 5000, 0, 0);
            }
            TutWinBaseWindow.check_base(near_base);
        }
        goto forest_shared;
    } else {
        if (TerraformTileFaction >= 0
            && !(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << (TerraformTileFaction & 0x1F)))
            && (faction_bit & GameDataStateStorage.fields_.lock_masks_.enable_mask_) != 0
            && (PlayersData[TerraformTileFaction].diplo_treaties[faction]
                & 0x11)
                == 0) {
            if ((PlayersData[faction].diplo_treaties[TerraformTileFaction]
                 & 0x20)
                == 0) {
                cause_friction(TerraformTileFaction, faction, 5);
            } else {
                set_treaty(TerraformTileFaction, faction, 0x400, 1);
            }
        }
        if (TerraformDrownedBaseId >= 0) {
            parse_says(0, Bases[TerraformDrownedBaseId].name_string, -1, -1);
            parse_says(1, Players[faction].adj_name_faction, -1, -1);
            const char *msg = faction == ConsoleGlobal.faction_id_ ? VehLabelTERRAMINE
                                                      : VehLabelTERRAYOURS;
            NetMsgPrimary.pop(msg, 5000, 0, 0);
        }
        if (MapBaseIdClosestSubmergedVeh[ConsoleGlobal.faction_id_] >= 0) {
            const int drown_base = MapBaseIdClosestSubmergedVeh[faction];
            parse_says(0, Bases[drown_base].name_string, -1, -1);
            parse_says(1, Players[faction].adj_name_faction, -1, -1);
            const char *msg = faction == ConsoleGlobal.faction_id_ ? VehLabelDROWNMINE
                                                      : VehLabelDROWNYOURS;
            NetMsgPrimary.pop(msg, 5000, 0, 0);
        }
    }

forest_shared:
    synch_bit(x, y, faction);
    draw_tiles(x, y, 2);
    if (veh_count_before == GameDataStateStorage.fields_.vehicle_current_count_) {
        TutWinBaseWindow.check_veh(veh_id);
        TutWinBaseWindow.check_loc(x, y, 2);
    } else {
        TutWinBaseWindow.GraphicWin::redraw();
    }

    if (faction != ConsoleGlobal.faction_id_) {
        return 1;
    }

    if ((Vehs[veh_id].state & VSTATE_UNK_200) != 0
        && (GameDataStateStorage.fields_.game_preferences_ & 0x20) != 0 && IsMultiplayerNet == 0) {
        const uint32_t bit = 1 << (action & 0x1F);
        uint32_t seen = GameDataStateStorage.fields_.terraform_announced_mask_[not_ocean];
        if ((bit & seen) == 0) {
            const uint32_t new_mask = seen | bit;
            GameDataStateStorage.fields_.terraform_announced_mask_[not_ocean] = new_mask;
            Sprite *sprite = 0;
            const char *msg = 0;
            int noun = veh_id;
            bool do_popup = true;
            if (not_ocean == 0) {
                switch (action) {
                case TERRA_FARM:
                    sprite = VehSpriteFARM;
                    noun = -1;
                    msg = VehLabelMADEFARM;
                    break;
                case TERRA_SOIL_ENR:
                    sprite = VehSpriteSOIL;
                    noun = -1;
                    msg = VehLabelMADESOIL;
                    break;
                case TERRA_MINE:
                    sprite = VehSpriteMINE;
                    noun = -1;
                    msg = VehLabelMADEMINE;
                    break;
                case TERRA_SOLAR_TIDAL:
                    sprite = VehSpriteSOLAR;
                    noun = -1;
                    msg = VehLabelMADESOLAR;
                    break;
                case TERRA_FOREST:
                    msg = VehLabelMADEFOREST;
                    break;
                case TERRA_ROAD:
                    msg = VehLabelMADEROAD;
                    break;
                case TERRA_MAGTUBE:
                    msg = VehLabelMADETUBE;
                    break;
                case TERRA_SENSOR:
                    msg = VehLabelMADESENSOR;
                    break;
                case TERRA_CONDENSER:
                    msg = VehLabelMADECONDENSER;
                    break;
                case TERRA_THERMAL_BORE:
                    msg = VehLabelMADEBOREHOLE;
                    break;
                case TERRA_AQUIFER:
                    msg = VehLabelMADEAQUIFER;
                    break;
                default:
                    GameDataStateStorage.fields_.terraform_announced_mask_[not_ocean] = ~bit & new_mask;
                    do_popup = false;
                    break;
                }
            } else if (action == TERRA_FARM) {
                sprite = VehSpriteFARM;
                msg = VehLabelMADESEAFARM;
                noun = -1;
            } else if (action == TERRA_MINE) {
                sprite = VehSpriteMINE;
                msg = VehLabelMADESEAMINE;
                noun = -1;
            } else if (action == TERRA_SOLAR_TIDAL) {
                sprite = VehSpriteSOLAR;
                msg = VehLabelMADESEASOLAR;
                noun = -1;
            } else {
                GameDataStateStorage.fields_.terraform_announced_mask_[not_ocean] = ~bit & new_mask;
                do_popup = false;
            }
            if (do_popup) {
                TutorialWindow.tut_map(msg, x, y, noun, sprite, 1, -1, -1);
            }
        }
    }

    if (faction == ConsoleGlobal.faction_id_ && Vehs[veh_id].order == ORDER_NONE
        && !(Vehs[veh_id].state & VSTATE_UNK_200)
        && VehTerraformStingLatch == 0) {
        VehTerraformStingLatch = 1;
        wave_it(0x14);
        ambience(0xCF);
    }
    return 1;
}
