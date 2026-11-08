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
#include "popup.h"
#include "basepop.h"
#include "text.h"
#include "basewin.h"
#include "buffer.h"
#include "sprite.h"
#include "random.h"
#include "interlude.h"
#include "time.h"
#include "win.h"
#include "veh.h"
#include "enemy_pb_policy.h"
#include "council.h"
#include "lock.h"  // map_tile_bytes() - the map-record table pointer
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "interface_palette.h"
#include "general.h" // range
#include "flic.h"
#include "log.h"
#include "map.h"
#include "strings.h"
#include "palette.h"  // Reactor-zero naming observes the existing palette slot.
#include "technology.h"
#include "terraforming.h"
#include "deletionlist.h"  // NetDaemonNet owns the embedded deletion queue
#include "tutwin.h"  // TutWinBaseWindow, TutorialWindow
#include "mapwin.h"  // draw_tile - the free single-tile redraw the removals end on
#include "netmsg.h"  // NetMsgPrimary - the promote family reports through it
#include "xpops.h"  // X_pop - transferred-unit single-player notifications.
#include "netdaemon.h"  // NetDaemonNet, synch_veh / order_veh - the action family's net half
#include "path.h"  // Paths - the move cost table the go-to verbs consult
#include "console.h"  // ConsoleGlobal - action_gate re-focuses the Console
#include "statuswin.h"  // ConsoleStatusWin owns its hovered vehicle selection.
#include "caviar.h"
#include "font.h"
#include "font_queue.h"
#include "crt_random_inline.h"
#include "infowin.h"
#include "worldwin.h"
#include "reportif.h"
#include "maininterface.h"

#include "wave.h"
#include "fx.h"
#include <mmsystem.h>
#include "designwin.h" // Crawler animation advances while the design window is visible.
#include "vector.h"  // vector_add operates on actual three-float arrays.

extern void __cdecl unspot_stack(int veh_id);


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

// Neutral source captions for the initial bytes at 0x687B60 and 0x687B6C: original `push` references at 0x4C9713/0x4C972B and terminating NULs bound 12/17 bytes. The original .data section is writable; original symbols and wider allocation extents are unproved.
// RECOVERY_OBJECT: v2 0x00687B60 0x00687B6C ?ActionBuildLogCaption@@3PADA 0x004C9713
char ActionBuildLogCaption[12] = "Action Base";
// RECOVERY_OBJECT: v2 0x00687B6C 0x00687B7D ?ActionBuildTurnLogCaption@@3PADA 0x004C972B
char ActionBuildTurnLogCaption[17] = "Action Base Turn";

/*
Purpose: Found a base from the built colony pod: log it, clear rock under
         the site, focus the view for the owning faction, settle the
         territory and treaty fallout, run base_init, copy the pod's home
         base's governor settings, name the base, kill the pod and record
         the colony-founded monument trigger.
// HISTORICAL SCAFFOLD ROLE: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x004C96E0 ?action_build@@YAHHPAD@Z 0x004C96E0-0x004C99C5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/map.cpp typed Vehs/Bases/PlayersData and base/map service declarations", "source callers:Console::new_base, enemy_move, NetDaemon::process_message; each owner includes map.h", "pinned original data at 0x687B60/0x687B6C has bounded NUL-terminated initial spans; former capacity unproved"], "reason": "Own base construction in the map/game-data API because its typed reads and writes use Vehs, Bases, PlayersData and existing map/base services. Keep the two caption arrays bounded to independently evidenced initial bytes."}
// symbol    ?action_build@@YAHHPAD@Z
// Rewritten from the image: base_init failure path first, double_cross on the
// treaty-2 arm, shift-form rocky test.
// size      741 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004c96e0.cpp, plain FILE). The veh record reads
        are the typed Vehs[] members (x/y, state, flags, faction_id,
        visibility); the base writes are Bases[] (governor, name_string,
        visibility, faction_id_current); the diplomacy words are
        PlayersData[].last_turn_new_base and .diplo_treaties. Two seeds
        readings are corrected to the pinned layouts: the home-base read
        is Vehs[a1].home_base_id (0x2E, stride 0x34) where the seed's
        unmeasured note carried a 0x1A stride, and the is-human tests are
        faction.h's is_human over LockEnableMask. The original +0x105C
        faction word is PlayerData::current_num_bases. base_init and
        mon_colony_founded use their genuine source owners. veh_skip
        expands from its MEASURED inline
        where the image calls 0x005C1D20 - the accepted house form.
*/

int __cdecl action_build(int veh_id, char *base_name) {
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    int faction_id = Vehs[veh_id].faction_id;
    log_say(ActionBuildLogCaption, x, y, faction_id);
    log_say(ActionBuildTurnLogCaption, GameDataStateStorage.fields_.turn_current_num_, 0, 0);
    if ((map_loc(x, y)->val3 >> 6) > 1) {
        rocky_set(x, y, 1);
    }
    if (faction_id == ConsoleGlobal.faction_id_
        || (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_
            && (Vehs[veh_id].flags & 0x440) != 0x440)
        || Vehs[veh_id].visibility & (1 << ConsoleGlobal.faction_id_)
        || GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) {
        ConsoleGlobal.focus(x, y, faction_id);
    }
    int faction_id_target = whose_territory(faction_id, x, y, NULL, false);
    if (PlayersData[faction_id].current_num_bases && faction_id_target >= 0
        && faction_id_target != faction_id) {
        uint32_t treaties = PlayersData[faction_id].diplo_treaties[faction_id_target];
        if (!(treaties & 0x11) && (is_human(faction_id) || is_human(faction_id_target))) {
            if (treaties & 2) {
                double_cross(faction_id, faction_id_target, -1);
            } else {
                treaty_on(faction_id, faction_id_target, 0x10);
            }
        }
    }
    int base_id = base_init(faction_id, x, y);
    if (base_id < 0) {
        veh_skip(veh_id);
        return base_id;
    }
    if (is_human(faction_id)) {
        int home_base_id = Vehs[veh_id].home_base_id;
        if (home_base_id >= 0) {
            Bases[base_id].governor = Bases[home_base_id].governor;
        }
        if (Vehs[veh_id].state & 0x200) {
            Bases[base_id].governor |= 0x80000000;
        }
    }
    PlayersData[faction_id].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
    if (base_name && strlen(base_name)) {
        strcpy(Bases[base_id].name_string, base_name);
        make_base_unique(base_id);
    }
    veh_kill(veh_id);
    if (faction_id == ConsoleGlobal.faction_id_
        || Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
        || Bases[base_id].visibility & (1 << ConsoleGlobal.faction_id_)
        || GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) {
        draw_map(1);
    }
    for (int i = 1; i < 8; i++) {
        del_site(i, 8, x, y, 2);
    }
    mon_colony_founded(faction_id, Bases[base_id].name_string);
    return base_id;
}


/*
Purpose: Select the live road or magtube terraforming rate from tile mask 0x4,
         then add the contributions selected by the tile flags, climate
         and upper val3 bits. The final gameplay purpose remains unresolved.

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C99D0 sub_4c99d0 0x004C99D0-0x004C9A43 FILE BYTE_EXACT
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
        result += 2;
    } else {
        result += tile->val3 >> 6;
    }
    return result;
}

/*
Purpose: Calculate the Former rate to perform terrain enhancements.
// ORIGINAL: 0x004C9A50 ?contribution@@YAHHH@Z 0x004C9A50-0x004C9AFB BYTE_EXACT
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

namespace {
// Separate writable objects preserve the original popup operands at each use.
// RECOVERY_OBJECT: v1 0x00687C84 ?staple_fail_alien_picture@?%src/terraforming_actions.cpp@@3PADA
char staple_fail_alien_picture[] = "ALdrone_sm.pcx";
// RECOVERY_OBJECT: v1 0x00687C94 ?staple_fail_alien_label@?%src/terraforming_actions.cpp@@3PADA
char staple_fail_alien_label[] = "NERVESTAPLEFAIL";
// RECOVERY_OBJECT: v1 0x00687CA4 ?staple_fail_human_picture@?%src/terraforming_actions.cpp@@3PADA
char staple_fail_human_picture[] = "drone_sm.pcx";
// RECOVERY_OBJECT: v1 0x00687CB4 ?staple_fail_human_label@?%src/terraforming_actions.cpp@@3PADA
char staple_fail_human_label[] = "NERVESTAPLEFAIL";
// RECOVERY_OBJECT: v1 0x00687CC4 ?staple_done_alien_picture@?%src/terraforming_actions.cpp@@3PADA
char staple_done_alien_picture[] = "ALdrone_sm.pcx";
// RECOVERY_OBJECT: v1 0x00687CD4 ?staple_done_alien_label@?%src/terraforming_actions.cpp@@3PADA
char staple_done_alien_label[] = "NERVESTAPLEDONE";
// RECOVERY_OBJECT: v1 0x00687CE4 ?staple_done_human_picture@?%src/terraforming_actions.cpp@@3PADA
char staple_done_human_picture[] = "drone_sm.pcx";
// RECOVERY_OBJECT: v1 0x00687CF4 ?staple_done_human_label@?%src/terraforming_actions.cpp@@3PADA
char staple_done_human_label[] = "NERVESTAPLEDONE";
// RECOVERY_OBJECT: v1 0x00687D04 ?staple_foreign_label@?%src/terraforming_actions.cpp@@3PADA
char staple_foreign_label[] = "NERVESTAPLETHEM";
}

// Current code uses PlayerData::proto_id_active for the byte table historically
// viewed as veh.cpp's FactionProtoSeenCount; the original LEVER annotations below
// retain their task-base wording as provenance.

// The UI, network and AI callers share the same base-domain operation.
// ORIGINAL: 0x004CA7F0 ?action_staple@@YAXH@Z 0x004CA7F0-0x004CAA4B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bca75cb0:src/recovered/units/004ca7f0.cpp", "image:original complete function instructions", "source:src/base.cpp"], "reason": "Home the original behavioral dependency in its base/audio domain using typed state and existing named callees; retained seed provenance and original spans remain restoration evidence, not runtime acceptance."}
void __cdecl action_staple(int base_id) {
    const int attempts = ++Bases[base_id].nerve_staple_count;
    const unsigned int faction = Bases[base_id].faction_id_current;
    bool success = false;
    if (attempts <= 2) {
        success = true;
    } else if (attempts <= 8 && rand() % 2 != 0) {
        success = true;
    }
    if (!success) {
        if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            if ((Players[faction].rule_flags & RFLAG_ALIEN) != 0) {
                popp(ScriptPopupFile, staple_fail_alien_label, 0, staple_fail_alien_picture, 0);
            } else {
                popp(ScriptPopupFile, staple_fail_human_label, 0, staple_fail_human_picture, 0);
            }
        }
    } else {
        Bases[base_id].state &= ~BSTATE_DRONE_RIOTS_ACTIVE;
        Bases[base_id].nerve_staple_turns_left += 10;
        draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
        if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            // The original retains this faction across draw_tile and does
            // not add an ExpansionEnabled test to the alien rule check.
            if ((Players[faction].rule_flags & RFLAG_ALIEN) != 0) {
                popp(ScriptPopupFile, staple_done_alien_label, 0, staple_done_alien_picture, 0);
            } else {
                popp(ScriptPopupFile, staple_done_human_label, 0, staple_done_human_picture, 0);
            }
        } else {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            PluralityDefault = 0;
            GenderDefault = Players[Bases[base_id].faction_id_current].is_leader_female;
            parse_says(1, Players[Bases[base_id].faction_id_current].title_leader, -1, -1);
            PluralityDefault = 0;
            GenderDefault = Players[Bases[base_id].faction_id_current].is_leader_female;
            parse_says(2, Players[Bases[base_id].faction_id_current].name_leader, -1, -1);
            NetMsgPrimary.pop(staple_foreign_label, 5000, 0, 0);
        }
    }
    // Failed attempts and foreign bases still report the atrocity.
    atrocity(Bases[base_id].faction_id_current, Bases[base_id].faction_id_former,
             Bases[base_id].faction_id_former != Bases[base_id].faction_id_current, 1);
}

// Inlined: action_destroy's halved neighbour reach.
static inline int destroy_dist(int x_a, int y_a, int x_b, int y_b) {
    return (x_dist(x_a, x_b) + abs(y_a - y_b)) >> 1;
}

// ORIGINAL: 0x004CAA50 ?action_destroy@@YAXHHHH@Z 0x004CAA50-0x004CB302
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004caa50.cpp", "recovery/tasks/mapwin-owner-coverage-wave.json", "Map byte+2 is val2; diplomatic treaty direction and callback rereads independently decoded from the pinned original."], "reason": "Recover the artillery and bombing destruction paths in their real vehicle owner, replacing the explicitly unfinished dependency while preserving its complete original span and all seed evidence."}
// TRIED: Ghidra-structure translation with NetMsg::pop/Strings::get/ Console::focus receiver fixes (Ghidra flattened those three thiscalls away), boom()/whose_territory()/battle_fight()/ stack_check()/add_goal()/base_find() argument order verified against raw disasm one by one; compiles, MISMATCH #6 (early - likely the isAssault/flag18 bool-store or veh_skip's arg push order relative to the OR-mask store).
void __cdecl action_destroy(int veh_id, int bits, int x, int y) {
    const int faction_id = Vehs[veh_id].faction_id;
    BOOL is_assault = false;
    BOOL is_forest = false;
    veh_skip(veh_id);
    Vehs[veh_id].state |= 0x40;
    if (on_map(x, y)) {
        if (base_who(x, y) >= 0) {
            if (faction_id == ConsoleGlobal.faction_id_) {
                boom(x, y, 0x80);
            }
            return;
        }
        if (rand() % 2) {
            if (faction_id == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop("ARTYBOMBFAILED", 5000, 0, 0);
            }
            return;
        }
        is_assault = true;
        if (faction_id == ConsoleGlobal.faction_id_
            || whose_territory(ConsoleGlobal.faction_id_, x, y, 0, 0) == ConsoleGlobal.faction_id_) {
            if (faction_id && faction_id != ConsoleGlobal.faction_id_) {
                boom(x, y, 0x80);
                boom(Vehs[veh_id].x, Vehs[veh_id].y, 0x80);
            }
            boom(x, y, 0x80);
        }
        int owner = whose_territory(faction_id, x, y, 0, 0);
        if (owner != faction_id) {
            for (int i = 1; i < 25; i++) {
                int x_radius = Vehs[veh_id].x + RadiusOffsetX[i];
                if (!(MapIsFlat & 1)) {
                    if (x_radius < 0) {
                        x_radius += MapLongitudeBounds;
                    } else if (x_radius >= MapLongitudeBounds) {
                        x_radius -= MapLongitudeBounds;
                    }
                }
                int y_radius = Vehs[veh_id].y + RadiusOffsetY[i];
                if (on_map(x_radius, y_radius) && destroy_dist(x_radius, y_radius, x, y) <= 2) {
                    int veh_id_stack = stack_fix(veh_at(x_radius, y_radius));
                    if (veh_id_stack >= 0 && stack_check(veh_id_stack, 15, owner, -1, -1)) {
                        battle_fight(veh_id, i, 1, 1, 0);
                        return;
                    }
                }
            }
        }
    } else {
        x = Vehs[veh_id].x;
        y = Vehs[veh_id].y;
    }
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR
        && !Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile
        && !(rand() % 4)) {
        if (faction_id == ConsoleGlobal.faction_id_) {
            NetMsgPrimary.pop("BOMBFAILED", 5000, 0, 0);
        }
    } else {
        uint32_t target_bits;
        if (bits > 0) {
            bits &= map_loc(x, y)->bit;
            target_bits = bits;
            if (!bits) {
                goto finish_destroy;
            }
            bit_set(x, y, bits, 0);
        } else {
            target_bits = map_loc(x, y)->bit;
        }
        if (!has_abil(Vehs[veh_id].proto_id, ABL_FUNGICIDAL)) {
            target_bits &= ~0x20;
        }
        int terra_id;
        for (int i = -1; i < 19; i++) {
            terra_id = i < 0 ? 9 : i;
            if (Terraforming[terra_id].bit & target_bits) {
                goto found_terra;
            }
        }
        log_say("Arty Failed The Hard Way", bits, target_bits, 0);
        if (is_assault) {
            if (faction_id == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop("ARTYBOMBFAILED", 5000, 0, 0);
            }
        } else if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR
            && faction_id == ConsoleGlobal.faction_id_) {
            NetMsgPrimary.pop("BOMBFAILED", 5000, 0, 0);
        }
        goto finish_destroy;
found_terra:
        {
            bits |= Terraforming[terra_id].bit;
            bit_set(x, y, Terraforming[terra_id].bit, 0);
            parse_says(2, StringTable.get((&Terraforming[terra_id].name)[is_ocean(x, y)]), -1, -1);
            if (terra_id == TERRA_FOREST) {
                is_forest = true;
            }
            if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR
                && faction_id == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop("BOMBSUCCEEDED", 5000, 0, 0);
            }
            synch_bit(x, y, faction_id);
            int owner = whose_territory(faction_id ? faction_id : ConsoleGlobal.faction_id_, x, y, 0, 0);
            synch_bit(x, y, owner);
            if (bits & 0x1C00000) {
                world_climate();
                draw_map(1);
            } else if (bits & 0x80000000 && owner == ConsoleGlobal.faction_id_) {
                draw_radius(x, y, 2, 2);
            } else {
                draw_tiles(x, y, 2);
            }
            if (faction_id) {
                if (is_assault && faction_id == ConsoleGlobal.faction_id_) {
                    NetMsgPrimary.pop("BOMBARDSUCCEEDED", 5000, 0, 0);
                }
                if (owner >= 0 && owner != faction_id
                    && PlayersData[faction_id].diplo_treaties[owner] & DTREATY_COMMLINK
                    && (PlayersData[owner].flags & PFLAG_MAP_REVEALED
                        || map_loc(x, y)->visibility & (1 << owner))) {
                    synch_bit(x, y, owner);
                    add_goal(owner, 0, 5, Vehs[veh_id].x, Vehs[veh_id].y, -1);
                    if (owner == ConsoleGlobal.faction_id_) {
                        int base_id = base_find(x, y, -1, -1, -1, ConsoleGlobal.faction_id_);
                        parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
                        parse_says(0, Players[faction_id].noun_faction, -1, -1);
                        parse_says(1, Bases[base_id].name_string, -1, -1);
                        parse_says(3, Players[faction_id].adj_name_faction, -1, -1);
                        NetMsgPrimary.pop(is_assault ? "HAVEDESTROYED1" : "HAVEDESTROYED", 5000, 0, 0);
                    }
                }
            } else if ((PlayersData[ConsoleGlobal.faction_id_].flags & PFLAG_MAP_REVEALED
                    || map_loc(x, y)->visibility & (1 << ConsoleGlobal.faction_id_))
                && owner == ConsoleGlobal.faction_id_) {
                int base_id = base_find(x, y, -1, -1, -1, ConsoleGlobal.faction_id_);
                if (base_id >= 0 && Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
                    parse_says(0, Bases[base_id].name_string, -1, -1);
                    parse_says(1, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
                    ConsoleGlobal.focus(x, y, 0);
                    if (is_forest) {
                        if (Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER) {
                            popp(ScriptPopupFile, "SPOREFOREST", 0, "sporlnch_sm.pcx", 0);
                        } else {
                            NetMsgPrimary.pop("ATEFOREST", 5000, 0, 0);
                        }
                    } else if (Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER) {
                        popp(ScriptPopupFile, "SPORESLAUNCHED", 0, "sporlnch_sm.pcx", 0);
                    } else {
                        NetMsgPrimary.pop("ATESTUFF", 5000, 0, 0);
                    }
                }
            }
        }
    }
finish_destroy:
    order_veh(veh_id, -1, 3);
}

// ===== veh.cpp, P2 wave 3: the action family =====
//
// The order verbs the unit's `order` byte dispatches to. All of them start the
// same way: under multiplayer, and while this client is not the one playing
// the turn out, they publish the address of their own veh_id parameter into
// VehSelectionFollow so the daemon's lock bookkeeping names the unit they are
// about to move - which is why every one of them reloads veh_id off the stack
// after its first call, and why the source spells the publish as `&veh_id`.

/*
Purpose: Move a unit along its path, one ORDER_MOVE / ORDER_MOVE_TO leg.
// ORIGINAL: 0x004CB310 ?action_go_to@@YAXH@Z 0x004CB310-0x004CB57F BYTE_EXACT
// size      623 bytes
// prototype void (__cdecl ?action_go_to@@YAXH@Z)(int veh_id)
// callers   2   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005310F0 0x00531300 0x00531480 0x00531780 0x00593220 0x005947C0 0x0059BC10 0x005C1D20
// HOMED from src/recovered/units/004cb310.cpp, 2026-09-04. Its `g_00952839`
// byte is Vehs[].order (+0x11), `g_0095283c` the waypoint_x[] run (+0x14),
// `g_00952836` faction_id (+0x0E) and `g_009ab88c` VehPrototypes[].chassis_id
// (+0x24); `g_0094a37b` is Chassis[].cargo (+0x4B, 0x90 stride) - the unit
// has to be a transport for the arrival skip to apply.
// symbol    the free order_veh (0x005947C0) is the host-side move; the
//           member NetDaemon::order_veh (0x00531780) is the client's lock
//           request. Both appear here, spelled by their own names.
// TRIED: 2/175, first divergence at instruction 2. The image opens
//        `push esi / mov esi,[0x93f660] / push edi / mov edi,[0x93a938]` - it
//        loads BOTH net globals into callee-saved registers before testing
//        either, then tests `esi` and `edi`. Spelled as two `const int` locals
//        ahead of the `&&`, this tree still sinks the second load behind the
//        first test (`mov eax,[0x93f660] / test / je / mov eax,[0x93a938]`), so
//        it reads the transport flag at all three guard sites instead of two
//        loads plus the one reload after Paths::move. The globals-read-at-each-
//        site spelling scores the same. Reverting to the plain spelling.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_go_to(int veh_id) {
    const int net = IsMultiplayerNet;
    const int turn_active = ConsoleControlTurnActive;
    if (net != 0 && turn_active == 0) {
        NetDaemonNet.set_vehicle_selection_follow(&veh_id);
    }

    int move_result;
    {
        const uint8_t order = Vehs[veh_id].order;
        if (order == ORDER_MOVE) {
            move_result = Vehs[veh_id].waypoint_x[0];
        } else {
            move_result = Paths.move(
                veh_id, (order & 0x40) != 0 ? -1 : Vehs[veh_id].faction_id);
        }
    }

    if (move_result >= 0) {
        if (IsMultiplayerNet != 0 && ConsoleControlTurnActive == 0) {
            if (NetDaemonNet.order_veh(veh_id, move_result, 1) != 0) {
                NetDaemonNet.await_exec(1);
            }
            return;
        }

        if (order_veh(veh_id, move_result, 3) == 0) {
            return;
        }

        const uint8_t order = Vehs[veh_id].order;
        if (order == ORDER_MOVE) {
            if (Vehs[veh_id].x == Vehs[veh_id].waypoint_x[1]
                && Vehs[veh_id].y == Vehs[veh_id].waypoint_y[1]) {
                Vehs[veh_id].order = ORDER_NONE;
                return;
            }
        } else if (order == ORDER_MOVE_TO) {
            if (Vehs[veh_id].x == Vehs[veh_id].waypoint_x[0]
                && Vehs[veh_id].y == Vehs[veh_id].waypoint_y[0]) {
                Vehs[veh_id].order = ORDER_NONE;

                if ((Vehs[veh_id].state & 0x2000200) == 0x2000200
                    && Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count) {
                    for (int i = 0; i < Vehs[veh_id].waypoint_count; ++i) {
                        Vehs[veh_id].waypoint_x[i] = Vehs[veh_id].waypoint_x[i + 1];
                        Vehs[veh_id].waypoint_y[i] = Vehs[veh_id].waypoint_y[i + 1];
                    }
                    Vehs[veh_id].order = ORDER_MOVE_TO;
                    Vehs[veh_id].waypoint_count -= 1;
                    if (Vehs[veh_id].waypoint_count == 0) {
                        Vehs[veh_id].state &= 0xfdfefdff;
                    }
                }

                if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << Vehs[veh_id].faction_id))
                    && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].cargo != 0) {
                    Map *tile = map_tiles()
                        + ((Vehs[veh_id].x >> 1) + Vehs[veh_id].y * MapLongitude);
                    if (tile->bit & BIT_BASE_IN_TILE) {
                        const int rockiness = tile->val2 & 0xf;
                        if (rockiness < 8 && rockiness >= 0) {
                            veh_skip(veh_id);
                            return;
                        }
                    }
                }
            }
        }
    } else {
        Vehs[veh_id].order = ORDER_NONE;
        if (IsMultiplayerNet != 0 && ConsoleControlTurnActive == 0) {
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
            NetDaemonNet.unlock_veh();
        }
    }
}

// 0x0094977C is Rules.tech_build_roads_fungus (Rules + 0x44): the tech that
// lets a unit road a fungus tile instead of clearing it first.
// 0x006919C0 is Terraforming[TERRA_REMOVE_FUNGUS].preq_tech and 0x00691940
// Terraforming[TERRA_MAGTUBE].preq_tech (base 0x00691878, 0x20 stride).
// 0x00687E20 / 0x00687E2C are the two phrase keys the arrival popup is titled
// with, "ROADTODONE" and "TUBETODONE" - selected by the unit's order.
// RECOVERY_OBJECT: v2 0x00687E20 0x00687E2B ?VehLabelROAD@@3PADA 0x004CB580
char VehLabelROAD[] = "ROADTODONE";
// RECOVERY_OBJECT: v2 0x00687E2C 0x00687E37 ?VehLabelTUBE@@3PADA 0x004CB580
char VehLabelTUBE[] = "TUBETODONE";

/*
Purpose: Walk a unit toward its goal, roading or tubing the tiles it crosses.
// ORIGINAL: 0x004CB580 ?action_road_to@@YAXH@Z 0x004CB580-0x004CBA98
// size      1304 bytes
// prototype void (__cdecl ?action_road_to@@YAXH@Z)(int veh_id)
// callers   2   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005310F0 0x00531300 0x00531480 0x00531780 0x00592250 0x005947C0 0x0059A530 0x005B9F20 0x00592EE0 0x00593220 0x004C9B00 0x0047A890 0x00625EC0
// HOMED from src/recovered/units/004cb580.cpp, 2026-09-04. Its `g_00952828`
// short views are Vehs[].x / .y / .waypoint_x[] / .waypoint_y[], its byte
// views Vehs[].faction_id (+0x0E) and Vehs[].order (+0x11) and its dword view
// Vehs[].state (+0x04).
//
// BUG IN THE ORIGINAL (and left as written): the archive this came from put
// the "is there a base on the fungus tile and it is rockiness <= 7" case on
// the SKIP side of the fungus clearing, which would drop the tile walk
// entirely. The image jumps from that test into the tile walk (0x004CB6C0
// `test eax,eax / jge 0x004CB7DF`, and eax is 0..7 after `and eax,0xf`, so
// the branch is always taken). The guard is dead on its <= 7 side - the
// compiler emitted `test eax,eax` after a mask that cannot set the sign flag -
// and the source below spells the fall-through the image actually takes.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_road_to(int veh_id) {
    if (IsMultiplayerNet && !ConsoleControlTurnActive) {
        NetDaemonNet.set_vehicle_selection_follow(&veh_id);
    }
    const int faction_id = Vehs[veh_id].faction_id;
    const int x_goal = Vehs[veh_id].waypoint_x[0];
    const int y = Vehs[veh_id].y;
    const int y_goal = Vehs[veh_id].waypoint_y[0];
    const int x = Vehs[veh_id].x;
    Map *tile = map_loc(x, y);
    const int altitude = tile->climate & ~0x1F;
    if (altitude < ALT_BIT_SHORE_LINE || !on_map(x_goal, y_goal)) {
        Vehs[veh_id].order = ORDER_NONE;
    } else {
        // The image's altitude bound here is the ocean shelf, not the shoreline.
        if (tile->bit & BIT_FUNGUS && altitude >= ALT_BIT_OCEAN_SHELF
            && !has_tech(Rules.tech_build_roads_fungus, faction_id)
            && has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[0], faction_id)
            && base_who(x, y) < 0) {
            if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                message_data(0x2405, 0, veh_id, ORDER_REMOVE_FUNGUS, Vehs[veh_id].order, 0);
                NetDaemonNet.await_exec(1);
            } else {
                int order = Vehs[veh_id].order;
                action_terraform(veh_id, TERRA_REMOVE_FUNGUS, 1);
                Vehs[veh_id].order = (int8_t)order;
            }
            return;
        }
        if ((!(map_loc(x, y)->bit & BIT_FUNGUS) || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF
                || has_tech(Rules.tech_build_roads_fungus, faction_id))
            && base_who(x, y) < 0) {
            uint32_t bits = map_loc(x, y)->bit;
            if (!(bits & BIT_ROAD)) {
                if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                    message_data(0x2405, 0, veh_id, ORDER_ROAD, Vehs[veh_id].order, 0);
                    NetDaemonNet.await_exec(1);
                } else {
                    int order = Vehs[veh_id].order;
                    action_terraform(veh_id, TERRA_ROAD, 1);
                    Vehs[veh_id].order = (int8_t)order;
                }
                return;
            }
            if (Vehs[veh_id].order == ORDER_MAGTUBE_TO && !(bits & BIT_MAGTUBE)
                && has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0], faction_id)) {
                if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                    message_data(0x2405, 0, veh_id, ORDER_ROAD, ORDER_MAGTUBE_TO, 0);
                    NetDaemonNet.await_exec(1);
                } else {
                    action_terraform(veh_id, TERRA_ROAD, 1);
                    Vehs[veh_id].order = ORDER_MAGTUBE_TO;
                }
                return;
            }
        }
        if (x == x_goal && y == y_goal) {
            int order = Vehs[veh_id].order;
            Vehs[veh_id].order = ORDER_NONE;
            if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
                NetDaemonNet.unlock_veh();
            }
            if (!(Vehs[veh_id].state & VSTATE_UNK_200) && faction_id == ConsoleGlobal.faction_id_) {
                StringTemp[0] = 0;
                say_loc(StringTemp, x_goal, y_goal, 0, 2, 0);
                parse_says(0, StringTemp, -1, -1);
                if (order == ORDER_ROAD_TO) {
                    NetMsgPrimary.pop(VehLabelROAD, 5000, 0, 0);
                } else {
                    NetMsgPrimary.pop(VehLabelTUBE, 5000, 0, 0);
                }
            }
            return;
        }
        int dir = Paths.find(x, y, x_goal, y_goal, 0, -1,
            has_tech(Rules.tech_build_roads_fungus, faction_id) ? 0 : 0x10, -1);
        if (dir >= 0 && dir < 8) {
            if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                if (NetDaemonNet.order_veh(veh_id, dir, 1)) {
                    NetDaemonNet.await_exec(1);
                }
            } else {
                order_veh(veh_id, dir, 3);
            }
            return;
        }
        Vehs[veh_id].order = ORDER_NONE;
    }
    if (IsMultiplayerNet && !ConsoleControlTurnActive) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
        NetDaemonNet.unlock_veh();
    }
}

// action_home's distance: arguments evaluate before the body, as the image's
// early second-coordinate load shows.
static inline int home_dist(int x_a, int y_a, int x_b, int y_b) {
    return (x_dist(x_a, x_b) + abs(y_a - y_b)) >> 1;
}

// action_home's airbase-ring wrap: the image tests the flat bit first.
static inline int home_wrap_x(int x) {
    return (MapIsFlat & 1) ? x
        : (x < 0 ? x + (int)MapLongitudeBounds
                 : (x >= (int)MapLongitudeBounds ? x - (int)MapLongitudeBounds : x));
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
    int best_dist = 9999;
    const int veh_x = veh.x;
    const int veh_y = veh.y;
    const int veh_region = (int)region_at(veh_x, veh_y);
    int best_x = -1;
    int best_y = -1;
    // How far the unit could still travel: what is left of this turn, plus - for a chassis with
    // endurance - the sorties it has not spent yet. A range 1 chassis additionally trades its
    // remaining hitpoints for distance.
    int moves_left = range((int)speed(veh_id, false) - veh.moves_expended, 0, 999);
    const int proto_id = veh.proto_id;
    const uint8_t chassis_range = Chassis[VehPrototypes[proto_id].chassis_id].range;
    int reach = (int)speed(veh_id, false)
        * (chassis_range - veh.terraforming_turns - 1) + moves_left;
    if (chassis_range == 1) {
        const int power = (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT)
            ? 1 : range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
        int hp_left = range(power - veh.dmg_incurred, 0, 9999);
        reach += (int)speed(veh_id, false)
            * ((hp_left - 1) / VehPrototypes[veh.proto_id].reactor_id);
    }
    const int max_dist = reach / (int)Rules.move_rate_roads;

    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        const int base_faction = Bases[base_id].faction_id_current;
        if (base_faction != faction_id
            && !(PlayersData[faction_id].diplo_treaties[base_faction] & DTREATY_PACT)) {
            continue; // not ours and not a pact partner's
        }
        switch (get_triad(veh_id)) {
        case TRIAD_LAND:
            if ((int)region_at(Bases[base_id].x, Bases[base_id].y) != veh_region) {
                continue; // no land route
            }
            break;
        case TRIAD_SEA:
            if ((Bases[base_id].x != veh_x || Bases[base_id].y != veh_y)
                && !base_on_sea(base_id, veh_region)) {
                continue; // not on this body of water
            }
            break;
        }
        if (flags == 2 && (Bases[base_id].state & BSTATE_UNK_200000)) {
            continue;
        }
        const int dist = home_dist(veh_x, veh_y, Bases[base_id].x, Bases[base_id].y);
        if ((veh.state & VSTATE_UNK_200)
            && veh.order_auto_type == ORDERA_ON_ALERT
            && Bases[base_id].x == veh.waypoint_x[1]
            && Bases[base_id].y == veh.waypoint_y[1] && dist <= max_dist) {
            // Already on alert for this exact base: take it and skip the remaining searches.
            best_dist = dist;
            best_x = Bases[base_id].x;
            best_y = Bases[base_id].y;
            goto found;
        }
        if (dist < best_dist) {
            best_dist = dist;
            best_x = Bases[base_id].x;
            best_y = Bases[base_id].y;
        }
    }

    if (get_triad(veh_id) == TRIAD_AIR) {
        // An air unit can also come down on a friendly carrier or air supply convoy.
        for (int other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
            if (Vehs[other_id].faction_id != faction_id) {
                continue;
            }
            if (other_id == veh_id) {
                continue;
            }
            if (!has_abil(Vehs[other_id].proto_id, ABL_CARRIER)) {
                if (VehPrototypes[Vehs[other_id].proto_id].plan != PLAN_SUPPLY_CONVOY) {
                    continue;
                }
                if (get_triad(other_id) != TRIAD_AIR) {
                    continue;
                }
            }
            int dist = home_dist(veh_x, veh_y, Vehs[other_id].x, Vehs[other_id].y);
            if ((flags & 1) || flags == 4) {
                if (!is_human(faction_id)) {
                    dist += (int)speed(other_id, false) / (int)Rules.move_rate_roads;
                }
                if (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_SUPPLY_CONVOY) {
                    // Recomputed raw, without the bias just added above.
                    const int other_y = Vehs[other_id].y;
                    const int x_part = x_dist(veh_x, Vehs[other_id].x);
                    const int moves = range((int)speed(veh_id, false)
                                            - veh.moves_expended, 0, 999);
                    if ((x_part + abs(veh_y - other_y)) >> 1 > moves / (int)Rules.move_rate_roads) {
                        continue; // the convoy is not reachable this turn
                    }
                }
            }
            if (dist < best_dist) {
                best_dist = dist;
                best_x = Vehs[other_id].x;
                best_y = Vehs[other_id].y;
            }
        }

        // Airbases inside the radius. The radius widens as the best answer so far gets worse, and
        // saturates at the largest ring when nothing has been found at all.
        const int radius_count = RadiusRange[best_dist < 8 ? best_dist : 8];
        for (int i = 0; i < radius_count; i++) {
            const int x_radius = home_wrap_x(RadiusOffsetX[i] + veh_x);
            const int y_radius = RadiusOffsetY[i] + veh_y;
            if (!on_map(x_radius, y_radius)) {
                continue;
            }
            Map *tile = map_loc(x_radius, y_radius);
            if (!(tile->bit & BIT_AIRBASE)) {
                continue;
            }
            const int occupier = veh_who(x_radius, y_radius);
            if (occupier >= 0 && occupier != faction_id
                && !(PlayersData[faction_id].diplo_treaties[occupier] & DTREATY_PACT)) {
                continue; // someone else is sitting on it
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
            const int dist = home_dist(veh_x, veh_y, x_radius, y_radius);
            if ((veh.state & VSTATE_UNK_200) && veh.order_auto_type == ORDERA_ON_ALERT
                && !veh.dmg_incurred && x_radius == veh.waypoint_x[1]
                && y_radius == veh.waypoint_y[1] && dist <= max_dist) {
                // best_dist is deliberately left alone here, unlike the base loop's shortcut.
                best_x = x_radius;
                best_y = y_radius;
                goto found;
            }
            if (dist < best_dist) {
                best_dist = dist;
                best_x = x_radius;
                best_y = y_radius;
            }
        }
    }
    if (((flags & 1) || flags == 4)
        && !Chassis[VehPrototypes[veh.proto_id].chassis_id].range) {
        return false; // only an endurance chassis answers these queries
    }

found:
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

// Airdrop a vehicle, including range, interception, landing-site, diplomatic
// and post-landing processing.  The four modes are the caller's network/local
// context flags; mode 1 is the network execution path, mode 3 the local path.
// Inlined: the halved airdrop reach, computed before drop_range is called.
static inline int airdrop_dist(int x_a, int y_a, int x_b, int y_b) {
    int x_distance = x_dist(x_a, x_b);
    int y_distance = abs(y_a - y_b);
    int distance = (x_distance + y_distance) >> 1;
    return distance;
}

// ORIGINAL: 0x004CC360 ?action_airdrop@@YAHHHHH@Z 0x004CC360-0x004CD051 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004cc360.cpp retained signature/extents and complete seed; no historical claim copied", "Coordinator full pinned original instruction decode 004CC360..004CD051, typed source owner fields and corrected original call/branch evidence", "recovery/tests/test_airdrop_network.py actual-body directional treaty and network reentrancy controls; not linked/runtime/game acceptance"], "reason": "Recover complete typed airdrop admission/interception, landing selection, stack movement, capture and post-drop updates. Preserve unsigned damage storage with integer thresholds and directional treaty evidence; referenced unfinished dependencies remain explicit, not game acceptance."}
// TRIED: direct field-for-field transcription (validation chain of terrain/ZOC/treaty/AA checks, each guarding a NetMsg::pop error-return; airdrop-site scoring loop; base-capture tail) diverges at instr #7 (push vs xor), early register-allocation choice; not chased further given the 3313-byte scope.
int __cdecl action_airdrop(int veh_id, int x, int y, int mode) {
    NetDaemonNet.channels_[0].leading_words_[3] = mode;
    const int faction_id = Vehs[veh_id].faction_id;
    if (airdrop_dist(Vehs[veh_id].x, Vehs[veh_id].y, x, y) > drop_range(faction_id)) {
        if (NetDaemonNet.channels_[0].leading_words_[3] & 1 && faction_id == ConsoleGlobal.faction_id_) {
            parse_num(0, Rules.max_airdrop_sans_orb_insert);
            if (Players[faction_id].rule_flags & 0x80) {
                NetMsgPrimary.pop("DROPRANGE", -5000, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("DROPRANGE", -5000, 0, "airdp_sm.pcx");
        }
        return 0;
    }
    if ((map_loc(x, y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
        if (NetDaemonNet.channels_[0].leading_words_[3] & 1 && faction_id == ConsoleGlobal.faction_id_) {
            if (Players[faction_id].rule_flags & 0x80) {
                NetMsgPrimary.pop("DROPSEA", -5000, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("DROPSEA", -5000, 0, "airdp_sm.pcx");
        }
        return 0;
    }
    int owner = veh_who(x, y);
    if (owner >= 0 && owner != faction_id
        && !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)) {
        if (NetDaemonNet.channels_[0].leading_words_[3] & 1 && faction_id == ConsoleGlobal.faction_id_) {
            if (Players[faction_id].rule_flags & 0x80) {
                NetMsgPrimary.pop("DROPUNIT", -5000, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("DROPUNIT", -5000, 0, "airdp_sm.pcx");
        }
        return 0;
    }
    if (!get_proto_offense_rating(Vehs[veh_id].proto_id) && zoc_move(x, y, faction_id)) {
        if (NetDaemonNet.channels_[0].leading_words_[3] & 1 && faction_id == ConsoleGlobal.faction_id_) {
            if (Players[faction_id].rule_flags & 0x80) {
                NetMsgPrimary.pop("NONCOMDROP", -5000, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("NONCOMDROP", -5000, 0, "airdp_sm.pcx");
        }
        return 0;
    }
    owner = base_who(x, y);
    if (owner >= 0 && owner != faction_id
        && !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)) {
        if (!get_proto_offense_rating(Vehs[veh_id].proto_id)) {
            if (NetDaemonNet.channels_[0].leading_words_[3] & 1 && faction_id == ConsoleGlobal.faction_id_) {
                if (Players[faction_id].rule_flags & 0x80) {
                    NetMsgPrimary.pop("NONCOMDROP", -5000, 0, "al_drop_sm.pcx");
                    return 0;
                }
                NetMsgPrimary.pop("NONCOMDROP", -5000, 0, "airdp_sm.pcx");
            }
            return 0;
        }
        if (NetDaemonNet.channels_[0].leading_words_[3] & 1 && faction_id == ConsoleGlobal.faction_id_
            && break_treaty(faction_id, owner, 7)) {
            return 0;
        }
    }
    for (int i = 0; i < 25; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            int owner_radius = base_who(x_radius, y_radius);
            if (owner_radius > 0 && owner_radius != faction_id
                && PlayersData[faction_id].diplo_treaties[owner_radius] & DTREATY_VENDETTA) {
                int base_id = base_at(x_radius, y_radius);
                if (base_id >= 0
                    && (has_fac(FAC_AEROSPACE_COMPLEX, base_id, 0)
                        || has_project(SP_CLOUDBASE_ACADEMY, owner_radius)
                        || stack_check(veh_at(x_radius, y_radius), 2, PLAN_AIR_SUPERIORITY, -1, -1))) {
                    if (NetDaemonNet.channels_[0].leading_words_[3] & 1 && faction_id == ConsoleGlobal.faction_id_) {
                        parse_says(0, Bases[base_id].name_string, -1, -1);
                        if (Players[faction_id].rule_flags & 0x80) {
                            NetMsgPrimary.pop("AIRDEFENSES", -5000, 0, "alair_sm.pcx");
                            return 0;
                        }
                        NetMsgPrimary.pop("AIRDEFENSES", -5000, 0, "air_sm.pcx");
                    }
                    return 0;
                }
            }
        }
    }
    if (NetDaemonNet.channels_[0].leading_words_[3] & 2) {
        int goody = goody_at(x, y);
        int is_seen = faction_id == ConsoleGlobal.faction_id_
            || anything_at(x, y) == ConsoleGlobal.faction_id_
            || GameDataStateStorage.fields_.game_state_ & 0x80;
        if (!is_seen) {
            for (int i = 0; i < 8; i++) {
                int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
                int y_radius = y + VehMoveDeltaY[i];
                if (on_map(x_radius, y_radius) && anything_at(x_radius, y_radius) == ConsoleGlobal.faction_id_) {
                    is_seen = true;
                    break;
                }
            }
        }
        int is_shown;
        if (is_seen && (faction_id == ConsoleGlobal.faction_id_
                || (PlayersData[faction_id].diplo_treaties[ConsoleGlobal.faction_id_] & DTREATY_PACT
                    ? GameDataStateStorage.fields_.game_preferences_ & 0x8000
                    : GameDataStateStorage.fields_.game_preferences_ & 4))) {
            is_shown = true;
        } else {
            is_shown = false;
        }
        int best_score = 999999;
        int best_dir = -1;
        for (int dir = 0; dir < 8; dir++) {
            int x_radius = wrap_map_x(x + VehMoveDeltaX[dir]);
            int y_radius = y + VehMoveDeltaY[dir];
            if (on_map(x_radius, y_radius)) {
                int score = rand() % 10 + 50;
                if (dir >= 6 || dir <= 0) {
                    score -= 25;
                }
                if (dir == 5 || dir == 1) {
                    score -= 10;
                }
                int owner_radius = anything_at(x_radius, y_radius);
                if (owner_radius >= 0) {
                    score += 100;
                    if (owner_radius != faction_id) {
                        score += 1000;
                        if (!(PlayersData[faction_id].diplo_treaties[owner_radius] & DTREATY_PACT)) {
                            score += 10000;
                        }
                    }
                }
                if (score < best_score) {
                    best_score = score;
                    best_dir = dir;
                }
            }
        }
        if (best_dir < 0) {
            return 0;
        }
        if (is_seen && faction_id != ConsoleGlobal.faction_id_) {
            parse_says(0, Bases[base_find(x, y, -1, -1, -1, 1)].name_string, -1, -1);
            parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
            parse_says(1, Players[faction_id].noun_faction, -1, -1);
            if (Players[faction_id].rule_flags & 0x80) {
                popp(ScriptPopupFile, drop_range(faction_id) > Rules.max_airdrop_sans_orb_insert
                    ? "MADEORBITAL" : "MADEAIRDROP", 0, "al_drop_sm.pcx", 0);
            } else {
                popp(ScriptPopupFile, drop_range(faction_id) > Rules.max_airdrop_sans_orb_insert
                    ? "MADEORBITAL" : "MADEAIRDROP", 0, "airdp_sm.pcx", 0);
            }
        }
        Vehs[veh_id].state |= VSTATE_MADE_AIRDROP;
        if (!get_proto_offense_rating(Vehs[veh_id].proto_id)) {
            veh_skip(veh_id);
        }
        int x_old = Vehs[veh_id].x;
        int y_old = Vehs[veh_id].y;
        int x_dir = wrap_map_x(x + VehMoveDeltaX[best_dir]);
        int y_dir = y + VehMoveDeltaY[best_dir];
        stack_veh(veh_id, 1);
        draw_tile(x_old, y_old, 2);
        if (is_seen && is_shown) {
            veh_scoot(veh_id, x_dir, y_dir, best_dir ^ 4, 0);
        }
        unspot_stack(veh_id);
        for (int veh_id_stack = veh_top(veh_id); veh_id_stack >= 0;
             veh_id_stack = Vehs[veh_id_stack].next_veh_id_stack) {
            if (veh_id_stack != veh_id) {
                veh_skip(veh_id_stack);
            }
        }
        if (veh_id == ConsoleGlobal.selected_vehicle_ && ConsoleGlobal.cursor_active_) {
            ConsoleGlobal.cursor_next(x, y);
        }
        stack_put(veh_id, x, y);
        int base_id = base_at(x, y);
        if (base_id >= 0) {
            if (faction_id != Bases[base_id].faction_id_current
                && !(PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current] & DTREATY_PACT)) {
                if (!((1 << faction_id) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
                    veh_skip(veh_id);
                }
                GameDataStateStorage.fields_.vehicle_selection_follow_limit_ = veh_id;
                Vehs[veh_id].dmg_incurred = 0;
                act_of_aggression(faction_id, Bases[base_id].faction_id_current);
                capture_base(base_id, faction_id, 0);
                veh_id = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
            }
        } else {
            Vehs[veh_id].dmg_incurred += VehPrototypes[Vehs[veh_id].proto_id].reactor_id * 2;
            while (!range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999)) {
                Vehs[veh_id].dmg_incurred--;
            }
        }
        spot_all(veh_id, 1);
        ConsoleGlobal.update_data(0);
        if (goody) {
            goody_box(veh_id, goody);
        }
        if (faction_id != ConsoleGlobal.faction_id_ && !(GameDataStateStorage.fields_.game_state_ & 0x80)
            && is_seen && !ConsoleGlobal.field_23BF0_ && !HIBYTE(GetAsyncKeyState(VK_SHIFT))
            && !IsMultiplayerNet) {
            clock_wait(is_shown && !ConsoleGlobal.field_23BF0_ ? 200 : 50);
        }
    }
    return 1;
}

// 0x00687FDC, the NetMsg label action_feature pops: the image's .data holds
// the phrase key the message table looks up.
// RECOVERY_OBJECT: v2 0x00687FDC 0x00687FE4 ?VehLabelFEATURE@@3PADA 0x004CD060
char VehLabelFEATURE[] = "FEATURE";

/*
Purpose: Report the feature at a site, multiplayer only.
// ORIGINAL: 0x004CD060 ?action_feature@@YAXXZ 0x004CD060-0x004CD082 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?action_feature@@YAXXZ)()
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047A890
// HOMED from src/recovered/004cd060.cpp, 2026-09-04 - its archive-only
// BYTE_EXACT claim re-earned in the TU that owns the rest of the action
// family. The scaffold's `g_0093f660` is IsMultiplayerNet and
// `g_00805338` is netmsg.h's NetMsgPrimary.
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl action_feature() {
    if (IsMultiplayerNet) {
        NetMsgPrimary.pop(VehLabelFEATURE, 0x1388, 0, 0);
    }
}

// Artillery command: source-owned vehicle/rules/treaty state and actual
// network lock protocol. The empty-target destruction dependency below
// remains explicitly unfinished; build continuity does not prove that path.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004CD090 ?action_arty@@YAXHHH@Z 0x004CD090-0x004CD2ED
// RECOVERY_BINDING: v1 {"evidence": ["Original4CD090..4CD2ED: readiness and wrapped-distance gates; stack/treaty protection; Rules.artillery_max_rng indexes existing RadiusRange; mode3 offline versus mode1 network locking and message dispatch.", "Retained src/recovered/units/004cd090.cpp is evidence, not the source owner. Fresh original disassembly confirms imports and call order; no new byte or game-acceptance claim."], "reason": "Recover the artillery command through genuine vehicle, faction, rules and network owners; destruction of an empty target remains an unfinished dependency."}
void __cdecl action_arty(int veh_id, int x, int y) {
    if (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_ &&
        veh_ready(veh_id) == 0) {
        NetMsgPrimary.pop("UNITMOVED", 5000, 0, nullptr);
        return;
    }
    const int vehicle_y = Vehs[veh_id].y;
    const int absolute_dx = abs(Vehs[veh_id].x - x);
    int dx;
    if ((MapIsFlat & 1) == 0 && absolute_dx > static_cast<int>(MapLongitude)) {
        dx = static_cast<int>(MapLongitudeBounds) - absolute_dx;
    } else {
        dx = absolute_dx;
    }
    if (((abs(vehicle_y - y) + dx) & ~1) > 4) {
        NetMsgPrimary.pop("OUTOFRANGE", 5000, 0, nullptr);
        return;
    }

    const int target = stack_fix(veh_at(x, y));
    if (target < 0) {
        if (IsMultiplayerNet == 0) {
            action_destroy(veh_id, 0, x, y);
        } else if (NetDaemonNet.lock_veh(&veh_id, 4, x, y, 0) == 0) {
            message_data(0x2413, 0, veh_id, x, y, 0);
            NetDaemonNet.await_exec(1);
        }
        return;
    }
    if (Vehs[target].faction_id == Vehs[veh_id].faction_id
        || PlayersData[Vehs[veh_id].faction_id].diplo_treaties[Vehs[target].faction_id] & DTREATY_PACT) {
        return;
    }
    const int direction = radius_move(Vehs[veh_id].x, Vehs[veh_id].y,
                                     x, y, RadiusRange[Rules.artillery_max_rng]);
    if (direction < 0) return;

    if (IsMultiplayerNet == 0) {
        NetDaemonNet.channels_[0].leading_words_[3] = 3;
        battle_fight(veh_id, direction, 1, 1, nullptr);
        return;
    }
    if (NetDaemonNet.maybe_lock(&veh_id, 0, 0, x, y, 4) != 0) return;
    NetDaemonNet.channels_[0].leading_words_[3] = 1;
    if (battle_fight(veh_id, direction, 1, 1, nullptr) != 0) {
        NetDaemonNet.unlock_veh();
    } else {
        message_veh(0x2402, veh_id, direction, 0);
        NetDaemonNet.await_exec(1);
    }
}

// ORIGINAL: 0x004CD2F0 ?action_destruct@@YAXH@Z 0x004CD2F0-0x004CD4D1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/recovered/units/004cd2f0.cpp", "recovery/tasks/mapwin-owner-coverage-wave.json", "Pinned instructions 4CD365 and 4CD44E identify reactor_id and PLAN_ALIEN_ARTIFACT, correcting seed cost/class labels."], "reason": "Recover vehicle self-destruction through typed reactor and plan fields, nine radius offsets and real stack lifecycle; original rereads the vehicle prototype after weap_val. Preserve seed identity and original extent without granting a byte claim."}
// LEVER: (0.84 -> 0.994) range()/wrap_map_x()/on_map() inlines, stack_kill
//   kept out of line (auto_inline off at its definition), and the kill pass
//   restarts the stack walk from an outer loop. Left: the post-break `< 0`
//   test the image threads straight into the kill block.
void __cdecl action_destruct(int veh_id) {
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    boom(x, y, 0);
    int damage = VehPrototypes[Vehs[veh_id].proto_id].reactor_id
        * range(weap_val(Vehs[veh_id].proto_id, Vehs[veh_id].faction_id), 1, 20) / 2;
    kill(veh_id);
    for (int i = 0; i < 9; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius) || base_at(x_radius, y_radius) >= 0) {
            continue;
        }
        for (int veh_id_dmg = veh_at(x_radius, y_radius); veh_id_dmg >= 0;
             veh_id_dmg = Vehs[veh_id_dmg].next_veh_id_stack) {
            Vehs[veh_id_dmg].dmg_incurred += (uint8_t)damage;
        }
        // A kill reshapes the stack, so the walk restarts from its new top.
        int veh_id_kill;
    restart_kill_walk:
        for (veh_id_kill = stack_fix(veh_at(x_radius, y_radius)); veh_id_kill >= 0;) {
            int threshold = (VehPrototypes[Vehs[veh_id_kill].proto_id].plan == PLAN_ALIEN_ARTIFACT)
                ? 1 : range(VehPrototypes[Vehs[veh_id_kill].proto_id].reactor_id, 1, 100) * 10;
            if (Vehs[veh_id_kill].dmg_incurred >= threshold) {
                stack_veh(veh_id_kill, 1);
                stack_kill(veh_id_kill);
                goto restart_kill_walk;
            }
            veh_id_kill = Vehs[veh_id_kill].next_veh_id_stack;
            if (veh_id_kill < 0) {
                break;
            }
        }
    }
}

// 0x00691B0C holds the address of the popup title the obliteration report
// shares with the climate popup - game.h owns ScriptPopupFile for that slot;
// the former ClimatePopupTitle facade no longer mediates this read.
// The other four are the phrase keys and the portrait the two report popups
// take: the obliterator's own faction gets the plain pair, everyone else the
// "2" pair the game uses for a faction hearing about another faction's act.
// RECOVERY_OBJECT: v2 0x00687FFC 0x0068800B ?VehIconBASEOBL_SM_PCX@@3PADA 0x004CD4E0
char VehIconBASEOBL_SM_PCX[] = "baseobl_sm.pcx";
// RECOVERY_OBJECT: v2 0x0068800C 0x00688015 ?VehLabelOBLITTED@@3PADA 0x004CD4E0
char VehLabelOBLITTED[] = "OBLITTED";
// RECOVERY_OBJECT: v2 0x00688018 0x00688027 ?VehIconBASEOBL2_SM_PCX@@3PADA 0x004CD4E0
char VehIconBASEOBL2_SM_PCX[] = "baseobl_sm.pcx";
// RECOVERY_OBJECT: v2 0x00688028 0x00688032 ?VehLabelOBLITTED2@@3PADA 0x004CD4E0
char VehLabelOBLITTED2[] = "OBLITTED2";

/*
Purpose: Obliterate a base and report it to every faction that can hear.
// ORIGINAL: 0x004CD4E0 ?action_oblit@@YAXHH@Z 0x004CD4E0-0x004CD69D BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The historical 0x004CD6F9 extent crossed padding and the separate valid_patrol entry; action_oblit ends at its own terminal RET. Preserve the complete original behavior and identity without attributing another function's bytes to this body.","evidence":["pinned:0x004CD4E0-0x004CD69D complete instruction decode; terminal RET at 0x004CD69C and no branch leaves this body","pinned:0x004CD69D-0x004CD6A0 NOP padding; independent valid_patrol starts at 0x004CD6A0 and has separately mapped callers","source:src/veh.cpp valid_patrol owns 0x004CD6A0-0x004CDA23; both IDA and Ghidra catalogues agree on the separate entries"]}
// size      445 bytes
// prototype void (__cdecl ?action_oblit@@YAXHH@Z)(int veh_id, int base_id)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C0A0 0x0046B190 0x004E5250 0x0055EEE0 0x00625E30 0x00625EC0
// symbol    ?action_oblit@@YAXHH@Z
// HOMED from src/recovered/units/004cd4e0.cpp, 2026-09-04. Its indexed table
// bases are Vehs[].faction_id (0x00952836, 0x34 stride), Bases[]
// .faction_id_current / .faction_id_former / .population_size / .visibility /
// .name_string (0x0097D044/45/46/4A/53, 0x134 stride), and Rules.
// tgl_oblit_base_atrocity (0x00949860 = Rules + 0x128) gating the atrocity.
// Player grammar reads at 0x004CD55C/5B0/5C9 select is_leader_female (+0),
// not faction_bonus_count (+0x50C), as the historical seed interpreted them.
// Slot 1 at 0x004CD562 selects the console faction's title_leader (+0x4C),
// slot 2 the former owner's adj_name_faction (+0x304), and slots 3/4 the
// vehicle faction's title_leader (+0x4C) and name_leader (+0x34).
// All faction rows use the actual Player stride, 0x59C. Actor/former IDs
// are captured before parsing; visibility is captured after parsing and before
// base_kill, while the popup faction and atrocity rule are read afterward.
//
// Historical seed note, not confirmed by the complete pinned instruction decode:
// "BUG IN THE ORIGINAL: the owner's name row is computed once into a local and
// then reused for BOTH the fourth and the fifth parse_says, so the fifth call
// prints from a row it never recomputed for. Transcribed as written."
// The decode actually retains the vehicle faction's row for its title/name pair.
//
// TRIED (the units archive, which measured 0.90 similarity / 15 of 138 edits
//        at 445 bytes both sides): the remaining divergence is the prologue's
//        register-save ORDER - the image loads the veh_id argument into eax
//        before saving ebx/esi/edi, this tree saves a callee-saved register
//        first. Register allocation, not control flow.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_oblit(int veh_id, int base_id) {
    const unsigned veh_faction = Vehs[veh_id].faction_id;
    const unsigned base_former = Bases[base_id].faction_id_former;
    parse_num(0, (int)Bases[base_id].population_size * 10);
    veh_id = base_former;
    parse_says(0, Bases[base_id].name_string, -1, -1);

    PluralityDefault = 0;
    GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
    parse_says(1, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);

    parse_says(2, Players[base_former].adj_name_faction, -1, -1);

    PluralityDefault = 0;
    GenderDefault = Players[veh_faction].is_leader_female;
    parse_says(3, Players[veh_faction].title_leader, -1, -1);

    GenderDefault = Players[veh_faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(4, Players[veh_faction].name_leader, -1, -1);

    const int audible = Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
        || (Bases[base_id].visibility & (1 << ConsoleGlobal.faction_id_)) != 0;

    base_kill(base_id);
    draw_map(1);

    if ((int)veh_faction == ConsoleGlobal.faction_id_) {
        popp(ScriptPopupFile, VehLabelOBLITTED, 0,
             VehIconBASEOBL_SM_PCX, 0);
    } else if ((int)base_former == ConsoleGlobal.faction_id_ || audible) {
        popp(ScriptPopupFile, VehLabelOBLITTED2, 0,
             VehIconBASEOBL2_SM_PCX, 0);
    }

    if (Rules.tgl_oblit_base_atrocity != 0) {
        atrocity(veh_faction, base_former, 2, 0);
    }
}

/*
Purpose: Check whether the specified unit could ever reach the given tile as a patrol waypoint.
// ORIGINAL: 0x004CD6A0 ?valid_patrol@@YAHHHH@Z 0x004CD6A0-0x004CDA23 BYTE_EXACT
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
// Inlined: the doubled vector_dist halved once.
static inline int patrol_dist(int x_a, int y_a, int x_b, int y_b) {
    return (x_dist(x_a, x_b) + abs(y_a - y_b)) >> 1;
}
int __cdecl valid_patrol(int veh_id, int x, int y) {
    // One local: the sea base id and the air start-is-landing flag share a slot.
    int base_veh;
    if (x == Vehs[veh_id].x && y == Vehs[veh_id].y) {
        return false;
    }
    if (!on_map(x, y)) {
        return false;
    }
    uint8_t triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
    if (triad == TRIAD_LAND) {
        if (map_loc(x, y)->region != map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->region) {
            return false;
        }
    } else if (triad == TRIAD_SEA) {
        base_veh = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        int base_dst = base_at(x, y);
        if (base_veh >= 0) {
            if (base_dst >= 0) {
                if (!port_to_port(base_veh, base_dst)) {
                    return false;
                }
            } else if (!base_on_sea(base_veh, map_loc(x, y)->region)) {
                return false;
            }
        } else if (base_dst >= 0) {
            if (!base_on_sea(base_dst, map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->region)) {
                return false;
            }
        } else if (map_loc(x, y)->region != map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->region) {
            return false;
        }
    } else if (triad == TRIAD_AIR
               && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range) {
        if (base_at(Vehs[veh_id].x, Vehs[veh_id].y) || map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_AIRBASE) {
            base_veh = 1;
        } else {
            base_veh = 0;
        }
        int landing_dst;
        if (base_at(x, y) || map_loc(x, y)->bit & BIT_AIRBASE) {
            landing_dst = 1;
        } else {
            landing_dst = 0;
        }
        if (base_veh < 0 && landing_dst < 0) {
            return false;
        }
        int reach = (speed(veh_id, false)
            * (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range
               - Vehs[veh_id].terraforming_turns - 1)
            + veh_moves(veh_id)) / Rules.move_rate_roads;
        int dist = patrol_dist(x, y, Vehs[veh_id].x, Vehs[veh_id].y);
        if (base_veh && landing_dst) {
            if (dist > reach) {
                return false;
            }
        } else if (dist > reach / 2) {
            return false;
        }
    }
    return true;
}

/*
Purpose: Set a vehicle patrolling a waypoint loop.
// ORIGINAL: 0x004CDA30 ?action_patrol@@YAXHHH@Z 0x004CDA30-0x004CDAEA BYTE_EXACT
// size      186 bytes
// prototype void (__cdecl ?action_patrol@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004CD6A0
// HOMED from src/unrecovered/004cda30.cpp, 2026-09-04. Its eight raw
// `arr_9528xx` casts are Vehs[] members: +0x11 order, +0x12 waypoint_count,
// +0x13 patrol_current_point, +0x14 waypoint_x[0], +0x1C waypoint_y[0],
// +0x00 x, +0x02 y, +0x04 state.
// TRIED: fixed const-pointer globals for the two indexed word arrays (the
//        address itself scales, so extern-style raw casts were used instead);
//        reached MNEMONIC_ONLY, 188 vs 186 bytes.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_patrol(int veh_id, int x, int y) {
    if (valid_patrol(veh_id, x, y) != 0) {
        Vehs[veh_id].order = ORDER_MOVE_TO;
        Vehs[veh_id].waypoint_x[Vehs[veh_id].waypoint_count] = (int16_t)x;
        Vehs[veh_id].waypoint_y[Vehs[veh_id].waypoint_count] = (int16_t)y;
        const uint8_t count = Vehs[veh_id].waypoint_count;
        Vehs[veh_id].waypoint_count = count + 1;
        Vehs[veh_id].waypoint_x[Vehs[veh_id].waypoint_count] = Vehs[veh_id].x;
        Vehs[veh_id].waypoint_y[Vehs[veh_id].waypoint_count] = Vehs[veh_id].y;
        Vehs[veh_id].state |= 0x1000200;
        Vehs[veh_id].patrol_current_point = 0;
    }
}

// ORIGINAL: 0x004CDAF0 ?shoot_it@@YAHHHHHH@Z 0x004CDAF0-0x004CE206
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 004CDAF0; retained seed git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004cdaf0.cpp", "recovery/tasks/vehicle-special-actions-coverage.json; native scope expansion preserves task baseline", "Native4CDBD4/4CDC08 load inventory after the first base scan;4CDC14 increments live deployed count;4CDC4B/51 snapshots bounded remaining attempts.", "Native4CDFF8..4CE018 unconditionally decrements live inventory after sacrifice callbacks and clamps deployed count;4CE059/063 reads reported inventory after parsing the base.", "Players noun_faction+2E4, noun_gender+2FC, is_noun_plural+300 and adj_name_faction+304 distinguish outcome and sacrifice text."], "reason": "Complete typed orbital-pod and Flechette interception, sacrifice, cancellation and reporting paths. Original binding began as an explicitly unfinished dependency; preserve its original spans and complete retained seed. Source implementation does not establish byte, reference, structure or runtime acceptance."}
// TRIED: mechanical Ghidra-shape transcription (int-arithmetic bases for the fixed globals, char*/int* casts sized to match each DAT_ family's stride) compiles and matches through instruction #2; not chased to byte-exact given size (1814 bytes).
int __cdecl shoot_it(int attacker, int defender, int x, int y,
                     int fungal_attack) {
    BOOL has_flechette = false;
    for (int i = 0; i < 25; i++) {
        if (!has_flechette) {
            int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius)) {
                int base_id = base_at(x_radius, y_radius);
                if (base_id >= 0 && Bases[base_id].faction_id_current != attacker
                    && has_fac(FAC_FLECHETTE_DEFENSE_SYS, base_id, 0)) {
                    has_flechette = true;
                }
            }
        }
    }
    if (!PlayersData[defender].satellites_[3] && !has_flechette) {
        return 0;
    }
    int result = 0;
    int unused = PlayersData[defender].satellites_[3] - PlayersData[defender].sat_odp_deployed;
    for (int attempt = 0; attempt < unused; attempt++) {
        PlayersData[defender].sat_odp_deployed++;
        if (rand() % 100 >= 50) {
            result = 1;
            break;
        }
    }
    int remaining = range(PlayersData[defender].satellites_[3] - PlayersData[defender].sat_odp_deployed,
        0, 9999);
    if (!result) {
        for (int j = 0; j < 25; j++) {
            int x_radius = wrap_map_x(x + RadiusOffsetX[j]);
            int y_radius = y + RadiusOffsetY[j];
            if (on_map(x_radius, y_radius)) {
                int base_id = base_at(x_radius, y_radius);
                if (base_id >= 0 && Bases[base_id].faction_id_current != attacker
                    && has_fac(FAC_FLECHETTE_DEFENSE_SYS, base_id, 0) && rand() % 100 < 50) {
                    parse_says(0, Bases[base_id].name_string, -1, -1);
                    if (fungal_attack) {
                        if (defender == ConsoleGlobal.faction_id_) {
                            parse_says(1, Players[attacker].adj_name_faction, -1, -1);
                            popp(ScriptPopupFile, "FWESHOTIT2", 0, "space_sm.pcx", 0);
                        } else if (attacker == ConsoleGlobal.faction_id_) {
                            parse_says(1, Players[Bases[base_id].faction_id_current].adj_name_faction, -1, -1);
                            popp(ScriptPopupFile, "FTHEYSHOTIT2", 0, "space_sm.pcx", 0);
                        }
                    } else if (defender == ConsoleGlobal.faction_id_) {
                        parse_says(1, Players[attacker].adj_name_faction, -1, -1);
                        popp(ScriptPopupFile, "TWESHOTIT2", 0, "space_sm.pcx", 0);
                    } else if (attacker == ConsoleGlobal.faction_id_) {
                        parse_says(1, Players[Bases[base_id].faction_id_current].adj_name_faction, -1, -1);
                        popp(ScriptPopupFile, "TTHEYSHOTIT2", 0, "space_sm.pcx", 0);
                    }
                    return 1;
                }
            }
        }
        if (!PlayersData[defender].satellites_[3]) {
            goto flechette_failed;
        }
        if (!IsMultiplayerNet && (1 << defender) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) {
            if (defender == ConsoleGlobal.faction_id_) {
                parse_num(0, PlayersData[defender].satellites_[3]);
                parse_says(0, Bases[base_find(x, y)].name_string, -1, -1);
                parse_says(1, Players[attacker].adj_name_faction, -1, -1);
                int dismissed;
                if (fungal_attack) {
                    dismissed = popp(ScriptPopupFile, "FUNGALSAC", 0, "space_sm.pcx", 0);
                } else {
                    dismissed = popp(ScriptPopupFile, "TECTONICSAC", 0, "space_sm.pcx", 0);
                }
                if (dismissed) {
                    goto flechette_failed;
                }
            } else if (attacker == ConsoleGlobal.faction_id_) {
                parse_set(Players[defender].noun_gender, Players[defender].is_noun_plural);
                parse_says(0, Players[defender].noun_faction, -1, -1);
                popp(ScriptPopupFile, "THEYMUSTSAC", 0, "space_sm.pcx", 0);
            }
        } else if (defender == ConsoleGlobal.faction_id_) {
            if (fungal_attack) {
                popp(ScriptPopupFile, "FWEMUSTSAC", 0, "space_sm.pcx", 0);
            } else {
                popp(ScriptPopupFile, "TWEMUSTSAC", 0, "space_sm.pcx", 0);
            }
        } else if (attacker == ConsoleGlobal.faction_id_) {
            parse_set(Players[defender].noun_gender, Players[defender].is_noun_plural);
            parse_says(0, Players[defender].noun_faction, -1, -1);
            popp(ScriptPopupFile, "THEYMUSTSAC", 0, "space_sm.pcx", 0);
        }
        PlayersData[defender].satellites_[3]--;
        if ((int)(PlayersData[defender].satellites_[3] - PlayersData[defender].sat_odp_deployed) < 0) {
            PlayersData[defender].sat_odp_deployed = PlayersData[defender].satellites_[3];
        }
    }
    {
        int base_id = base_find(x, y, defender);
        if (base_id < 0) {
            base_id = base_find(x, y);
        }
        parse_says(0, Bases[base_id].name_string, -1, -1);
    }
    parse_num(0, PlayersData[defender].satellites_[3]);
    parse_num(1, remaining);
    if (fungal_attack) {
        if (defender == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[attacker].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "FWESHOTIT", 0, "space_sm.pcx", 0);
        } else if (attacker == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[defender].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "FTHEYSHOTIT", 0, "space_sm.pcx", 0);
        }
    } else if (defender == ConsoleGlobal.faction_id_) {
        parse_says(1, Players[attacker].adj_name_faction, -1, -1);
        popp(ScriptPopupFile, "TWESHOTIT", 0, "space_sm.pcx", 0);
    } else if (attacker == ConsoleGlobal.faction_id_) {
        parse_says(1, Players[defender].adj_name_faction, -1, -1);
        popp(ScriptPopupFile, "TTHEYSHOTIT", 0, "space_sm.pcx", 0);
    }
    return 1;
flechette_failed:
    if (has_flechette) {
        if (defender == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[attacker].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "WEFAILEDFLECHETTE", 0, "space_sm.pcx", 0);
        } else if (attacker == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[defender].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "THEYFAILEDFLECHETTE", 0, "space_sm.pcx", 0);
        }
    }
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004CE210 ?action_tectonic@@YAXHHH@Z 0x004CE210-0x004CE6A1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 004CE210; retained seed src/recovered/units/004ce210.cpp", "recovery/tasks/vehicle-special-actions-coverage.json; native scope expansion preserves task baseline", "Native4CE326/4CE7F0 kill precedes announcement; fungal EBX retains initial combat faction while later slot reads observe compacted storage;580B51/580BF7 are mask immediates, not globals."], "reason": "Typed transfer or terrain action preserves native parser, kill, interception and post-call vehicle-slot read order; retained seed remains evidence, not executable backing."}
void __cdecl action_tectonic(int vehicle_id, int x, int y) {
    BOOL focused_on_event = false;
    int shot_result = 0;
    g_FX.play(0x53, Vehs[vehicle_id].x, Vehs[vehicle_id].y);
    const int reactor = VehPrototypes[Vehs[vehicle_id].proto_id].reactor_id;
    const int affected_tile_count = RadiusRange[reactor];
    const int base_id = base_find(x, y);
    int attacker = Vehs[vehicle_id].faction_id;
    if (attacker != ConsoleGlobal.faction_id_
        && (PlayersData[ConsoleGlobal.faction_id_].flags & PFLAG_MAP_REVEALED
            || map_loc(x, y)->visibility & (1 << ConsoleGlobal.faction_id_))
        && base_id >= 0 && Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.focus(x, y, attacker);
        focused_on_event = true;
    }
    attacker = Vehs[vehicle_id].faction_id;
    kill(vehicle_id);
    if (attacker != ConsoleGlobal.faction_id_ && base_id >= 0) {
        parse_set(Players[attacker].is_leader_female, 0);
        parse_says(0, Players[attacker].title_leader, -1, -1);
        parse_set(Players[attacker].is_leader_female, 0);
        parse_says(1, Players[attacker].name_leader, -1, -1);
        parse_set(Players[attacker].noun_gender, Players[attacker].is_noun_plural);
        parse_says(2, Players[attacker].noun_faction, -1, -1);
        parse_says(3, Bases[base_id].name_string, -1, -1);
        popp(ScriptPopupFile, "TECTONICMISSILE", 0, "tectonic_sm.pcx", nullptr);
    }
    int hit_faction[MaxPlayerNum];
    memset(hit_faction, 0, sizeof(hit_faction));
    for (int i = 0; i < affected_tile_count; ++i) {
        const int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        const int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius)) {
            continue;
        }
        const int veh_id_def = stack_fix(veh_at(x_radius, y_radius));
        if (veh_id_def < 0) {
            continue;
        }
        const int defender = Vehs[veh_id_def].faction_id;
        if (attacker == defender || PlayersData[attacker].diplo_treaties[defender] & DTREATY_PACT
            || hit_faction[defender]) {
            continue;
        }
        shot_result = shoot_it(attacker, defender, x, y, 0);
        if (shot_result) {
            break;
        }
        hit_faction[defender] = 1;
    }
    const int territory = whose_territory(attacker, x, y, nullptr, 0);
    if (territory >= 0 && territory != attacker && !hit_faction[territory]) {
        shot_result = shoot_it(attacker, territory, x, y, 0);
    }
    if (!shot_result) {
        for (int i = 0; i < reactor; ++i) {
            world_raise_alt(x, y);
        }
        world_climate();
        draw_map(1);
        // The original reads the killed unit's slot here.
        if (focused_on_event) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_set(Players[Vehs[vehicle_id].faction_id].is_leader_female, 0);
            parse_says(1, Players[Vehs[vehicle_id].faction_id].title_leader, -1, -1);
            parse_set(Players[Vehs[vehicle_id].faction_id].is_leader_female, 0);
            parse_says(2, Players[Vehs[vehicle_id].faction_id].name_leader, -1, -1);
            parse_set(Players[Vehs[vehicle_id].faction_id].noun_gender,
                      Players[Vehs[vehicle_id].faction_id].is_noun_plural);
            parse_says(3, Players[Vehs[vehicle_id].faction_id].noun_faction, -1, -1);
            popp(ScriptPopupFile, "TECMOFIED", 0, "tectonic_sm.pcx", nullptr);
        }
        ++TectonicDetonationCount[Vehs[vehicle_id].faction_id];
        UnkBitfield1 |= 4;
    }
}

/*
Purpose: Place fungus, clear the original improvement masks, raise low
         ocean terrain to altitude 2, then synchronize and redraw the tile
         when the local faction can see it.

// ORIGINAL: 0x004CE6B0 sub_4ce6b0 0x004CE6B0-0x004CE78D FILE BYTE_EXACT
// symbol    _sub_4ce6b0
// TRIED: caching the recomputed table index in one local instead of re-reading the row stride fresh each use loses the frame; the cached `half` matches the original's stack-slot reuse of the a2 parameter.
// size      221 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046AF40 0x00591D60 0x00591E50 0x005C2020
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
extern "C" void __cdecl sub_4ce6b0(int x_coord, int y_coord) {
    bit_set(x_coord, y_coord, 0x20, 1);
    bit_set(x_coord, y_coord, 0x28805C, 0);
    bit_set(x_coord, y_coord, MapTileFlagClearMask, 0);

    if ((map_loc(x_coord, y_coord)->climate & ~0x1F) < 0x40) {
        world_alt_set(x_coord, y_coord, 2, 0);
    }

    const int faction = ConsoleGlobal.faction_id_;
    const uint32_t entry = PlayersData[faction].flags;
    if ((entry & 0x200) != 0
        || (map_loc(x_coord, y_coord)->visibility & (1 << faction)) != 0
        || (GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
        synch_bit(x_coord, y_coord, faction);
        draw_tile(x_coord, y_coord, 2);
    }
}

// ORIGINAL: 0x004CE790 ?action_fungal@@YAXHHH@Z 0x004CE790-0x004CEDDB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 004CE790; retained seed git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004ce790.cpp", "recovery/tasks/vehicle-special-actions-coverage.json; native scope expansion preserves task baseline", "Native4CE326/4CE7F0 kill precedes announcement; fungal EBX retains initial combat faction while later slot reads observe compacted storage;580B51/580BF7 are mask immediates, not globals."], "reason": "Typed transfer or terrain action preserves native parser, kill, interception and post-call vehicle-slot read order; retained seed remains evidence, not executable backing."}
// TRIED: literal parameter-slot layout (a1/a2/a3 kept live as named locals rather than reused stack slots the original repurposes after FX::play/kill); diverges at instruction #5, before the base_find call, so it's a prologue register-allocation gap. Full logic (opportunistic-attack radius scan, territory check, and the fungus-spread loop with veh_init/bit_set/ world_alt_set/synch_bit/draw_tile) is landed and faithful to the byte-offset table math (record stride 0x34, faction matrix stride 0x20cc), cross-checked instruction-by-instruction against the disassembly rather than trusted from Ghidra alone.
// The image inlines map.h's anything_at here (its dead `== -1` test survives).

void __cdecl action_fungal(int vehicle_id, int x, int y) {
    int i;
    BOOL focused_on_spread = false;
    int shot_result = 0;
    const int attacker = Vehs[vehicle_id].faction_id;
    const int base_id = base_find(x, y);
    const int nearby_tile_count = RadiusRange[1];
    g_FX.play(0x54, Vehs[vehicle_id].x, Vehs[vehicle_id].y);
    kill(vehicle_id);
    if (attacker != ConsoleGlobal.faction_id_ && base_id >= 0) {
        parse_set(Players[attacker].is_leader_female, 0);
        parse_says(0, Players[attacker].title_leader, -1, -1);
        parse_set(Players[attacker].is_leader_female, 0);
        parse_says(1, Players[attacker].name_leader, -1, -1);
        parse_set(Players[attacker].noun_gender, Players[attacker].is_noun_plural);
        parse_says(2, Players[attacker].noun_faction, -1, -1);
        parse_says(3, Bases[base_id].name_string, -1, -1);
        popp(ScriptPopupFile, "FUNGALMISSILE", 0, "fungpayld_sm.pcx", nullptr);
    }
    int hit_faction[MaxPlayerNum];
    memset(hit_faction, 0, sizeof(hit_faction));
    for (i = 0; i < nearby_tile_count; ++i) {
        const int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        const int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius)) {
            continue;
        }
        const int veh_id_def = stack_fix(veh_at(x_radius, y_radius));
        if (veh_id_def < 0) {
            continue;
        }
        const int defender = Vehs[veh_id_def].faction_id;
        if (attacker == defender || PlayersData[attacker].diplo_treaties[defender] & DTREATY_PACT
            || hit_faction[defender]) {
            continue;
        }
        shot_result = shoot_it(attacker, defender, x, y, 1);
        if (shot_result) {
            break;
        }
        hit_faction[defender] = 1;
    }
    const int territory = whose_territory(attacker, x, y, nullptr, 0);
    if (territory >= 0 && territory != attacker && !hit_faction[territory]) {
        shot_result = shoot_it(attacker, territory, x, y, 1);
    }
    if (shot_result) {
        return;
    }
    // The killed unit's slot is read again here, as in the original.
    const int spread_count = VehPrototypes[Vehs[vehicle_id].proto_id].reactor_id * 2 - 1;
    for (i = 0; i < spread_count + 1; ++i) {
        const int direction = i ? rand() % 8 + 1 : 0;
        const int x_spread = wrap_map_x(x + RadiusOffsetX[direction]);
        const int y_spread = y + RadiusOffsetY[direction];
        if (!on_map(x_spread, y_spread)) {
            continue;
        }
        if (rand() % 2 && anything_at(x_spread, y_spread) == -1) {
            veh_init(BSC_MIND_WORMS, 0, x_spread, y_spread);
        }
        if (bit_at(x_spread, y_spread) & BIT_FUNGUS
            && (map_loc(x_spread, y_spread)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
            continue;
        }
        const int focus_faction = Vehs[vehicle_id].faction_id;
        if (focus_faction != ConsoleGlobal.faction_id_
            && (PlayersData[ConsoleGlobal.faction_id_].flags & PFLAG_MAP_REVEALED
                || map_loc(x_spread, y_spread)->visibility & (1 << ConsoleGlobal.faction_id_))
            && base_id >= 0 && Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.focus(x_spread, y_spread, focus_faction);
            focused_on_spread = true;
        }
        // sub_4ce6b0 (0x004CE6B0), expanded here as in the image.
        bit_set(x_spread, y_spread, BIT_FUNGUS, 1);
        bit_set(x_spread, y_spread, 0x28805C, 0);
        bit_set(x_spread, y_spread, MapTileFlagClearMask, 0);
        if ((map_loc(x_spread, y_spread)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF) {
            world_alt_set(x_spread, y_spread, 2, 0);
        }
        const int faction = ConsoleGlobal.faction_id_;
        if (PlayersData[faction].flags & PFLAG_MAP_REVEALED
            || map_loc(x_spread, y_spread)->visibility & (1 << faction)
            || GameDataStateStorage.fields_.game_state_ & 0x80) {
            synch_bit(x_spread, y_spread, faction);
            draw_tile(x_spread, y_spread, 2);
        }
    }
    if ((map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
        veh_init(BSC_FUNGAL_TOWER, 0, x, y);
    }
    UnkBitfield1 |= 4;
    if (focused_on_spread) {
        parse_says(0, Bases[base_id].name_string, -1, -1);
        parse_set(Players[Vehs[vehicle_id].faction_id].is_leader_female, 0);
        parse_says(1, Players[Vehs[vehicle_id].faction_id].title_leader, -1, -1);
        parse_set(Players[Vehs[vehicle_id].faction_id].is_leader_female, 0);
        parse_says(2, Players[Vehs[vehicle_id].faction_id].name_leader, -1, -1);
        parse_set(Players[Vehs[vehicle_id].faction_id].noun_gender,
                  Players[Vehs[vehicle_id].faction_id].is_noun_plural);
        parse_says(3, Players[Vehs[vehicle_id].faction_id].noun_faction, -1, -1);
        popp(ScriptPopupFile, "FUNGMOTIZED", 0, "fungpayld_sm.pcx", nullptr);
    }
}

// Coupled transfer/terrain actions and their still-unfinished prototype dependencies.
// Partial dependencies retain explicit BODY GOES HERE sentinels and seed obligations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004CEDE0 ?action_give@@YAXHH@Z 0x004CEDE0-0x004CF379 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 004CEDE0; retained seed src/recovered/units/004cede0.cpp", "recovery/tasks/vehicle-special-actions-coverage.json; native scope expansion preserves task baseline", "Native4CE326/4CE7F0 kill precedes announcement; fungal EBX retains initial combat faction while later slot reads observe compacted storage;580B51/580BF7 are mask immediates, not globals."], "reason": "Typed transfer or terrain action preserves native parser, kill, interception and post-call vehicle-slot read order; retained seed remains evidence, not executable backing."}
void __cdecl action_give(int vehicle_id, int faction_id) {
    const int original_owner = Vehs[vehicle_id].faction_id;
    const int x = Vehs[vehicle_id].x;
    const int y = Vehs[vehicle_id].y;
    int id;
    // Hand over the unit and everything it carries.
    for (int i = veh_top(vehicle_id); i >= 0; i = Vehs[i].next_veh_id_stack) {
        if (i != vehicle_id
            && (Vehs[i].order != ORDER_SENTRY_BOARD || Vehs[i].waypoint_x[0] != vehicle_id)) {
            continue;
        }
        int proto_id;
        if (Vehs[i].proto_id >= MaxVehProtoFactionNum) {
            // Find or design an equivalent prototype for the recipient.
            proto_id = -1;
            for (int j = 0; j < MaxVehProtoFactionNum; j++) {
                id = faction_id * MaxVehProtoFactionNum + j;
                if (VehPrototypes[id].flags & PROTO_ACTIVE
                    && VehPrototypes[id].chassis_id == VehPrototypes[Vehs[i].proto_id].chassis_id
                    && VehPrototypes[id].weapon_id == VehPrototypes[Vehs[i].proto_id].weapon_id
                    && VehPrototypes[id].armor_id == VehPrototypes[Vehs[i].proto_id].armor_id
                    && VehPrototypes[id].ability_flags == VehPrototypes[Vehs[i].proto_id].ability_flags
                    && VehPrototypes[id].reactor_id == VehPrototypes[Vehs[i].proto_id].reactor_id) {
                    proto_id = id;
                    break;
                }
            }
            if (proto_id < 0) {
                proto_id = propose_proto(faction_id,
                    VehPrototypes[Vehs[i].proto_id].chassis_id,
                    VehPrototypes[Vehs[i].proto_id].weapon_id,
                    VehPrototypes[Vehs[i].proto_id].armor_id,
                    VehPrototypes[Vehs[i].proto_id].ability_flags,
                    VehPrototypes[Vehs[i].proto_id].reactor_id,
                    (uint8_t)VehPrototypes[Vehs[i].proto_id].unk_1, nullptr);
                if (proto_id < 0) {
                    for (int j = 0; j < MaxVehProtoFactionNum; j++) {
                        id = faction_id * MaxVehProtoFactionNum + j;
                        if (VehPrototypes[id].flags & PROTO_ACTIVE
                            && VehPrototypes[id].chassis_id == VehPrototypes[Vehs[i].proto_id].chassis_id
                            && VehPrototypes[id].weapon_id == VehPrototypes[Vehs[i].proto_id].weapon_id
                            && VehPrototypes[id].armor_id == VehPrototypes[Vehs[i].proto_id].armor_id) {
                            proto_id = id;
                            break;
                        }
                    }
                    if (proto_id < 0) {
                        for (int j = 0; j < MaxVehProtoFactionNum; j++) {
                            id = faction_id * MaxVehProtoFactionNum + j;
                            if (VehPrototypes[id].flags & PROTO_ACTIVE
                                && VehPrototypes[id].plan == VehPrototypes[Vehs[i].proto_id].plan) {
                                proto_id = id;
                                break;
                            }
                        }
                        if (proto_id < 0) {
                            for (int j = 0; j < MaxVehProtoFactionNum; j++) {
                                id = faction_id * MaxVehProtoFactionNum + j;
                                if (VehPrototypes[id].flags & PROTO_ACTIVE
                                    && VehPrototypes[id].chassis_id
                                        == VehPrototypes[Vehs[i].proto_id].chassis_id) {
                                    proto_id = id;
                                    break;
                                }
                            }
                            if (proto_id < 0) {
                                proto_id = get_proto_triad(Vehs[i].proto_id) == TRIAD_SEA
                                    ? BSC_TRANSPORT_FOIL : BSC_SCOUT_PATROL;
                            }
                        }
                    }
                }
            }
        } else {
            proto_id = Vehs[i].proto_id;
        }
        Vehs[i].faction_id = static_cast<uint8_t>(faction_id);
        Vehs[i].proto_id = static_cast<int16_t>(proto_id);
        Vehs[i].order = ORDER_NONE;
        Vehs[i].state &= 0xFCFFBDFF;
        const int home_base_id = base_find(x, y, faction_id);
        Vehs[i].home_base_id = static_cast<int16_t>(home_base_id >= 0 ? home_base_id : -1);
        if (base_who(x, y) < 0) {
            owner_set(x, y, faction_id);
        }
    }
    draw_radius(x, y, 3, 2);
    if (faction_id == ConsoleGlobal.faction_id_) {
        parse_says(0, VehPrototypes[Vehs[vehicle_id].proto_id].veh_name, -1, -1);
        parse_set(Players[original_owner].is_leader_female, 0);
        parse_says(1, Players[original_owner].title_leader, -1, -1);
        parse_set(Players[original_owner].is_leader_female, 0);
        parse_says(2, Players[original_owner].name_leader, -1, -1);
        parse_set(Players[original_owner].noun_gender, Players[original_owner].is_noun_plural);
        parse_says(3, Players[original_owner].noun_faction, -1, -1);
        ConsoleGlobal.focus(x, y, original_owner);
        const int base_id = base_find(x, y);
        if (base_id >= 0) {
            parse_says(4, Bases[base_id].name_string, -1, -1);
            if (Language || is_human(original_owner)) {
                NetMsgPrimary.pop("GOTUNIT", 5000, 0, nullptr);
            } else if (PlayersData[faction_id].region_base_plan[region_at(x, y)] == 2
                       || VehPrototypes[Vehs[vehicle_id].proto_id].plan == PLAN_DEFENSIVE) {
                X_pop("GOTUNITDEFEND", nullptr);
            } else {
                X_pop("GOTUNITOFFEND", nullptr);
            }
        }
    } else if (original_owner == ConsoleGlobal.faction_id_) {
        parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
        parse_says(3, Players[faction_id].noun_faction, -1, -1);
        if (faction_id) {
            NetMsgPrimary.pop("GAVEUNIT", 5000, 0, nullptr);
        } else {
            NetMsgPrimary.pop("GAVEWILD", 5000, 0, nullptr);
        }
    }
}

/*
Purpose: Gate a vehicle into a base, repainting both tiles.
// ORIGINAL: 0x004CF380 ?action_gate@@YAXHH@Z 0x004CF380-0x004CF477
// size      247 bytes
// prototype void (__cdecl ?action_gate@@YAXHH@Z)(int vehID, int baseID)
// callers   3   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AF40 0x004E3A50 0x005108A0 0x005BFFA0 0x005C0080
// HOMED from src/unrecovered/004cf380.cpp, 2026-09-04. Its `Bases[n] + off`
// casts are Base members: +0x00 x, +0x02 y, +0x30 state (bit 0x20000000 is
// the "gate/open" event the UI reads).
// TRIED: reproducing veh_lift's post-call EAX reuse - the call site pushes
//        EAX straight into veh_drop's first argument with no reload, so EAX
//        must be veh_lift's return value; a local function-pointer type got
//        86.6% mnemonic similarity and redeclaring veh_lift itself is C2556.
//        veh_lift already returns int here, so the plain spelling is used.
// TRIED: frameless /O2 body where the image has `frame` - a spill-count
//        difference, not a control-flow one.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_gate(int veh_id, int base_id) {
    const int gate_base = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
    if (gate_base >= 0) {
        Bases[gate_base].state |= 0x20000000;
    }
    Bases[base_id].state |= 0x20000000;

    veh_drop(veh_lift(veh_id), Bases[base_id].x, Bases[base_id].y);

    if (gate_base >= 0) {
        draw_tile(Bases[gate_base].x, Bases[gate_base].y, 2);
    }
    draw_tile(Bases[base_id].x, Bases[base_id].y, 2);

    if (veh_id == ConsoleGlobal.selected_vehicle_) {
        ConsoleGlobal.focus(Vehs[veh_id].x, Vehs[veh_id].y, Vehs[veh_id].faction_id);
    }
}

// Orbital combat uses the four source-owned faction satellite counters.
// ORIGINAL: 0x004CF480 ?action_sat_attack@@YAXHHHH@Z 0x004CF480-0x004CF73E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004cf480.cpp", "image:0x004CF480-0x004CF73E complete instructions and outcome literals", "source:src/faction.h:PlayerData satellite array", "source:src/reportif.h", "source:src/maininterface.h"], "reason": "Recover orbital attack effects in the vehicle-action domain using real faction diplomacy, grammar, indexed satellite counters and report state. Original CRT randomness, parsing order, unchecked satellite indexing and popup outcomes are retained; existing popup and diplomacy seams remain separate obligations."}
void __cdecl action_sat_attack(int attacker, int target, int type, int base_id) {
    if (!(PlayersData[attacker].diplo_treaties[target] & 0x10))
        double_cross(attacker, target, -1);

    if (attacker == ConsoleGlobal.faction_id_ || target == ConsoleGlobal.faction_id_) {
        PluralityDefault = 0;
        GenderDefault = Players[attacker].is_leader_female;
        parse_says(0, Players[attacker].title_leader, -1, -1);
        GenderDefault = Players[attacker].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[attacker].name_leader, -1, -1);
        GenderDefault = Players[attacker].noun_gender;
        PluralityDefault = Players[attacker].is_noun_plural;
        parse_says(2, Players[attacker].noun_faction, -1, -1);

        PluralityDefault = 0;
        GenderDefault = Players[target].is_leader_female;
        parse_says(3, Players[target].title_leader, -1, -1);
        GenderDefault = Players[target].is_leader_female;
        PluralityDefault = 0;
        parse_says(4, Players[target].name_leader, -1, -1);
        GenderDefault = Players[target].noun_gender;
        PluralityDefault = Players[target].is_noun_plural;
        parse_says(5, Players[target].noun_faction, -1, -1);
        if (type == 4) {
            parse_say(6, Facility[41].name, -1, -1);
        } else {
            parse_say(6, Facility[65 + type].name, -1, -1);
        }
        // Native 4CF606 selects Player + 0x304, the faction adjective.
        parse_says(7, Players[target].adj_name_faction, -1, -1);
    }

    // Original CRT rand, signed idiv remainder, exactly once after parsing.
    // No random() substitution, counter clamping or invalid-type filtering.
    if (rand() % 100 < 50) {
        if (type == 4) set_fac(41, base_id, 0);
        else --PlayersData[target].satellites_[type];
        ++PlayersData[attacker].sat_odp_deployed;
        if (attacker == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile, "WEKILLEDIT", 0, "satbat_sm.pcx", 0);
        else if (target == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile, "THEYKILLEDIT", 0, "satbat_sm.pcx", 0);
    } else {
        --PlayersData[attacker].satellites_[3];
        if (attacker == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile, "WEMUFFEDIT", 0, "satbat_sm.pcx", 0);
        else if (target == ConsoleGlobal.faction_id_)
            popp(ScriptPopupFile, "THEYMUFFEDIT", 0, "satbat_sm.pcx", 0);
    }
    if (ReportIfGlobal.field_8_ == 6 && IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5)
        ReportIfGlobal.on_redraw();
}

/*
Purpose: Dispatch a unit's order byte to the verb that carries it out.
// ORIGINAL: 0x004CF740 ?action@@YAHH@Z 0x004CF740-0x004CF7B4 BYTE_EXACT
// size      116 bytes
// prototype int (__cdecl ?action@@YAHH@Z)(int veh_id)
// callers   3   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C9B00 0x004CB310 0x004CB580
// HOMED from src/unrecovered/004cf740.cpp, which carried the archive's own
// FILE BYTE_EXACT claim for these 116 bytes; re-earned in the TU 2026-09-04.
// Its `g_00952839` byte view is Vehs[].order (+0x11) - the whole body is a
// dispatch on it. The three arms cover every order the AI and the interface
// can queue: the terraforming range ORDER_FARM..ORDER_MOVE_TO-1 (4..0x17) goes
// to action_terraform with the order's TERRA_* index, the two road verbs go to
// their walkers, and everything in the move family goes to action_go_to.
// LEVER: the order byte is `int8_t`, not `int` and not `uint8_t`. The image
//        keeps it in `al` and compares SIGNED (`cmp al,4 / jl`, `cmp al,0x18 /
//        jge`) - `int` widened it to `movsx eax` and cost the byte form, and
//        `uint8_t` turned the branch unsigned (`jb`/`jae`). Naming it after
//        the field it reads, which veh.h declares `int8_t order`, is what
//        closed the last instruction.
// LEVER: the terraforming arm's upper bound is ORDER_MOVE_TO (0x18), not
//        ORDER_PLACE_MONOLITH - the archive's own `state < 0x18` is the
//        half-open end of the range, and I first transcribed it as the
//        symbolic name the value 0x17 would have had.
Return Value: 1 when an arm ran and the order was cleared, 0 otherwise.
Status: BYTE_EXACT.
*/
int __cdecl action(int veh_id) {
    const int8_t order = Vehs[veh_id].order;
    if (order >= ORDER_FARM && order < ORDER_MOVE_TO) {
        action_terraform(veh_id, order - 4, 1);
    } else if (order == ORDER_ROAD_TO || order == ORDER_MAGTUBE_TO) {
        action_road_to(veh_id);
    } else if (order == ORDER_MOVE_TO || order == ORDER_AI_MOVE_TO
               || order == ORDER_MOVE) {
        action_go_to(veh_id);
    } else {
        return 0;
    }
    if (Vehs[veh_id].order & 0x40) {
        Vehs[veh_id].order = ORDER_NONE;
    }
    return 1;
}
