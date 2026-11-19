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
// The orders TU (0x00593510-0x0059A1AB): hex_cost, quick_zoc,
// supply_options and order_veh, the vehicle movement-order path. The
// boundary is inferred from the image (it follows the message/synch run and
// ends at the path TU's initializer); the original file name is unknown.
#include "stdafx.h"
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h
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
#include "faction_inline.h"
#include "crt_random_inline.h"
#include "infowin.h"
#include "worldwin.h"
#include "reportif.h"
#include "maininterface.h"

#include "wave.h"
#include "fx.h"
#include <mmsystem.h>
#include "designwin.h" // Crawler animation advances while the design window is visible.

extern void __cdecl unspot_stack(int veh_id);

/*
Purpose: Calculate the movement penalty/cost.
// ORIGINAL: 0x00593510 ?hex_cost@@YAHHHHHHHH@Z 0x00593510-0x0059382F BYTE_EXACT
// TRIED: tree's `proto_id != BSC_SEALURK // Bug fix` term has no matching `cmp` in the image - only ONE proto_id compare (`cmp eax, 9`, BSC_ISLE_OF_THE_DEEP) exists on this path. Removing the extra term is a structural fix (matches the image's condition), not a byte win by itself: 7/308 -> 8/308 agreeing under the best flag set. TRIED: hoisting `bit_src` next to `bit_dst` at the top (image DOES read the src tile's `bit` field before the is_ocean(dst) branch, per raw ebx allocation) - tried both declaration orders, both regressed (18/308 and 5/308) because it forces a second stack-resident local where the image keeps bit_src in a register (ebx) for the whole function and only spills bit_dst. The remaining gap past instruction 8 is VC6's own instruction scheduling across the three nested inlines (bit_at/is_ocean/altitude_at) - a scheduler decision, not reachable by reordering statements. Plateau at this structure.
// LEVER: exact once the body reads both tiles through Map pointers (the image
//        keeps the destination tile in esi for the later rocky read), tests
//        altitude as `climate >> 5` (gives the image's `and al,0xE0; cmp eax`
//        rather than map.h is_ocean's `& ~0x1F`), and ends in one `return cost`
//        with the hover/antigrav case and the fungus penalty as if/else arms.
// size      799 bytes
// prototype int (__cdecl ?hex_cost@@YAHHHHHHHH@Z)(int protoID, int factionID, int xCoordSrc, int yCoordSrc, int xCoordDst, int yCoordDst, int toggle)
// callers   5   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C13B0 0x00644F3A
Return Value: Movement cost
Status: Complete
*/
int __cdecl hex_cost(int proto_id, int faction_id, int x_src, int y_src, int x_dst,
                     int y_dst, BOOL toggle) {
    Map *tile_src = map_loc(x_src, y_src);
    uint32_t bit_src = tile_src->bit;
    Map *tile_dst = map_loc(x_dst, y_dst);
    uint32_t bit_dst = tile_dst->bit;
    int cost;
    if ((tile_dst->climate >> 5) < ALT_SHORE_LINE) {
        if (bit_dst & BIT_FUNGUS && (tile_dst->climate >> 5) == ALT_OCEAN_SHELF
            && get_proto_triad(proto_id) == TRIAD_SEA
            && proto_id != BSC_ISLE_OF_THE_DEEP && !has_project(SP_XENOEMPATYH_DOME, faction_id)) {
            return Rules.move_rate_roads * 3;
        }
        return Rules.move_rate_roads;
    }
    if ((tile_src->climate >> 5) < ALT_SHORE_LINE) {
        return Rules.move_rate_roads;
    }
    if (proto_id >= 0 && get_proto_triad(proto_id) != TRIAD_LAND) {
        return Rules.move_rate_roads;
    }
    // Land only conditions
    if (bit_src & (BIT_MAGTUBE | BIT_BASE_IN_TILE) && bit_dst & (BIT_MAGTUBE | BIT_BASE_IN_TILE)
        && faction_id) {
        return 0;
    }
    if ((bit_src & (BIT_ROAD | BIT_BASE_IN_TILE) || bit_src & BIT_FUNGUS && faction_id > 0
        && has_project(SP_XENOEMPATYH_DOME, faction_id)) && bit_dst & (BIT_ROAD | BIT_BASE_IN_TILE)
        && faction_id) {
        return 1;
    }
    if (faction_id >= 0 && (has_project(SP_XENOEMPATYH_DOME, faction_id) || !faction_id
        || proto_id == BSC_MIND_WORMS || proto_id == BSC_SPORE_LAUNCHER) && bit_dst & BIT_FUNGUS) {
        return 1;
    }
    // The image open-codes the y distance; only the x distance calls abs().
    if (bit_src & BIT_RIVER && bit_dst & BIT_RIVER && x_dist(x_src, x_dst) == 1
        && (y_src - y_dst < 0 ? y_dst - y_src : y_src - y_dst) == 1 && faction_id) {
        return 1;
    }
    if (VehPrototypes[proto_id].chassis_id == CHSI_HOVERTANK
        || has_abil(proto_id, ABL_ANTIGRAV_STRUTS)) {
        cost = Rules.move_rate_roads;
    } else {
        cost = Rules.move_rate_roads;
        if ((tile_dst->val3 >> 6) > ROCKINESS_ROLLING && !toggle) {
            cost += Rules.move_rate_roads;
        }
        if (bit_dst & BIT_FOREST && !toggle) {
            cost += Rules.move_rate_roads;
        }
        if (faction_id && bit_dst & BIT_FUNGUS && (proto_id >= MaxVehProtoFactionNum
            || get_proto_offense_rating(proto_id) >= 0)) {
            uint8_t plan = VehPrototypes[proto_id].plan;
            if (plan != PLAN_TERRAFORMING && plan != PLAN_ALIEN_ARTIFACT
                && PlayersData[faction_id].soc_effect_active.values_[SocialEffect::PLANET] <= 0) {
                cost += Rules.move_rate_roads * 2;
            } else {
                cost = max(cost, speed_proto(proto_id));
            }
        }
    }
    return cost;
}


/*
Purpose: Quickly check for unit related zone of control conflicts. If a ZOC conflict is found, store
         the coordinates of the tile inside ZOC pointers.
// ORIGINAL: 0x00593830 ?quick_zoc@@YAXHHHHHPAH0@Z 0x00593830-0x005939FC BYTE_EXACT
// TRIED: 12/152, 0.873 similar (best flag set /Oi-) - call count already
//            matches the image (`call_diff` agrees). First divergence is
//            is_ocean's own known plateau (`xor ecx,ecx` before the
//            map_tiles() pointer load in the image, this tree loads the
//            pointer first) - same root cause as is_ocean's standalone note.
// symbol    ?quick_zoc@@YAXHHHHHPAH0@Z
// size      460 bytes
// prototype void (__cdecl ?quick_zoc@@YAXHHHHHPAH0@Z)(int xCoordSrc, int yCoordSrc, int factionID, int xCoordDst, int yCoordDst, int* xCoordZoc, int* yCoordZoc)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644F3A
Return Value: n/a
Status: Complete
*/
void __cdecl quick_zoc(int x_src, int y_src, int faction_id, int x_dst, int y_dst,
                       int *x_zoc, int *y_zoc) {
    BOOL is_src_ocean = is_ocean(x_src, y_src);
    int search_zoc = -1;
    for (uint32_t i = 0; i < 8; i++) {
        // The image's wrap stores each arm to the x slot and reloads it for
        // on_map: map.h's flat-first wrap_map_x.
        int t = x_src + VehMoveDeltaX[i];
        int x_radius = wrap_map_x(t);
        int y_radius = y_src + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = veh_who(x_radius, y_radius);
            if (owner >= 0 && owner != faction_id
                && is_ocean(x_radius, y_radius) == is_src_ocean
                && !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)) {
                int proximity = vector_dist(x_radius, y_radius, x_dst, y_dst);
                if (proximity >= search_zoc) {
                    search_zoc = proximity;
                    *x_zoc = x_radius;
                    *y_zoc = y_radius;
                }
            }
        }
    }
}


// WORKAROUND(inline): dead /Ob2 budget probes; supply_options_budget_step(2)
// in supply_options folds to no code. A nested four-test spend anywhere
// between the picture and the convoy items (recover permute --probe-sweep: 40
// of 702 placements; every other size and form leaves both ??_D StringList
// teardowns expanded) makes VC6 call ??_D (0x00406820) out of line at both
// teardowns, as the image does. With NetDaemon's flat channel members that
// spend is one step: the two-step supply_options_budget_probe(2) there leaves
// its second step called out of line. The original spending is not
// identified. Defined at the end of the unit: defined here, the two helpers
// flip order_veh's layout.
static int supply_options_budget_probe(int x);
static int supply_options_budget_step(int x);

// ORIGINAL: 0x00593A00 ?supply_options@@YAHHH@Z 0x00593A00-0x005947BF;0x00660BF0-0x00660F47 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x593a00 hot and cold spans; private full-body native-order review and actual-source callback/lifetime controls.", "git:7f3107aa61899b2f1b5449bdac31cdb1d3d6952a:src/unrecovered/00593a00.cpp", "source:src/basepop.h:typed SpriteBox click callback at BasePop+3160; native CodeView and producer/consumer evidence."], "reason": "Recover the complete picker/supply operation through genuine Popup, Sprite, Veh, Base and network owners; preserve full original spans and seed findings without a new matching claim."}
// TRIED: same Popup-teardown-cascade shape as PickWin/battle_plans, but this one calls Dialogs' REAL destructor (`??1Dialogs@@QAE@XZ`) rather than the vtable-patch-then-close() dance those two used - simpler here, used directly. The final "apply the action" branch (0x5942C1 onward: which of two near-identical full teardowns fires) is approximated as always taking the first (`chosen2==1`) branch - the exact value tracked through BasePop::exec's result wasn't re-derived in the time available. sim 0.34-0.59 across flag sets.
int __cdecl supply_options(int veh_id, int base_id) {
    Popup popup;
    const int owner = Vehs[veh_id].faction_id;

    if (!is_human(owner)) {
        return 0;
    }
    if (Vehs[veh_id].order == ORDER_ROAD_TO || Vehs[veh_id].order == ORDER_MAGTUBE_TO ||
        Vehs[veh_id].order == ORDER_MOVE || (Vehs[veh_id].state & 0x4000u) != 0) {
        return 0;
    }
    if (Vehs[veh_id].order == ORDER_MOVE_TO
        && map_range(Vehs[veh_id].x, Vehs[veh_id].y, Vehs[veh_id].waypoint_x[0],
                       Vehs[veh_id].waypoint_y[0]) > 1) {
        return 0;
    }

    const int base_owner = Bases[base_id].faction_id_current;
    if (owner != base_owner &&
        !has_treaty(owner, base_owner, 1)) {
        return 0;
    }

    parse_says(1, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
    parse_says(2, Bases[base_id].name_string, -1, -1);
    const int home_base_id = Vehs[veh_id].home_base_id;
    if (home_base_id >= 0) {
        parse_says(3, Bases[home_base_id].name_string, -1, -1);
    }
    int queued_cost;
    int production = Bases[base_id].queue_production_id[0];
    if (production < 0) {
        const int facility_id = -production;
        queued_cost = cost_factor(owner, 1, -1) * Facility[facility_id].cost;
    } else {
        queued_cost = veh_cost(production, base_id, 0) * cost_factor(owner, 1, -1);
    }
    parse_num(0, Bases[base_id].minerals_accumulated);
    parse_num(1, queued_cost);
    popup.start("SUPPLYUNIT");

    // Each branch names its picture twice: the image keeps four separate
    // literals (0x68FC34/0x68FC44 al_col_sm.pcx, 0x68FC54/0x68FC64
    // colfoun_sm.pcx) and two get_pcx_dimensions/init call pairs.
    int picture_width, picture_height;
    if (Players[owner].rule_flags & 0x80u) {
        if (Buffer::get_pcx_dimensions("al_col_sm.pcx", &picture_width,
                                       &picture_height) == 0
            && popup.owned_picture().init("al_col_sm.pcx", picture_width,
                                          picture_height) == 0) {
            popup.select_owned_picture();
        }
    } else {
        if (Buffer::get_pcx_dimensions("colfoun_sm.pcx", &picture_width,
                                       &picture_height) == 0
            && popup.owned_picture().init("colfoun_sm.pcx", picture_width,
                                          picture_height) == 0) {
            popup.select_owned_picture();
        }
    }
    if (X_text_open(ScriptPopupFile, "SUPPLYMENU") != 0) {
        return 0;
    }

    text_get();
    supply_options_budget_step(2); // WORKAROUND(inline): see its declaration
    clear();
    parse_string(TextBufferGetPtr, StringTemp);
    popup.item(StringTemp, 0);
    int item_count = 1;

    text_get();
    if (Bases[base_id].queue_production_id[0] <= -70) {
        const int facility_id = -Bases[base_id].queue_production_id[0];
        queued_cost = cost_factor(owner, 1, -1) * Facility[facility_id].cost;
        parse_say(0, Facility[facility_id].name, -1, -1);
        parse_string(TextBufferGetPtr, StringTemp);
        popup.item(StringTemp, 1);
        item_count = 2;
    }

    text_get();
    production = Bases[base_id].queue_production_id[0];
    if (production >= 0) {
        queued_cost = veh_cost(production, base_id, 0) * cost_factor(owner, 1, -1);
        if (production >= 64 && (VehPrototypes[production].flags & 4u) == 0) {
            parse_says(0, VehPrototypes[production].veh_name, -1, -1);
            parse_string(TextBufferGetPtr, StringTemp);
            popup.item(StringTemp, 1);
            ++item_count;
        }
    }
    if (convoy(veh_id, base_id) != 0) {
        item_count += 3;
        for (int convoy_line = 2; convoy_line <= 4; ++convoy_line) {
            text_get();
            parse_string(TextBufferGetPtr, StringTemp);
            popup.item(StringTemp, convoy_line);
        }
    }
    if (item_count <= 1) {
        return 0;
    }

    int action;
    if (owner == ConsoleGlobal.faction_id_
        && (NetDaemonNet.action_mode_ & 1u) != 0) {
        action = popup.exec();
        if (action == 0) {
            return 0;
        }
        Vehs[veh_id].probe_action = static_cast<uint8_t>(action);
    } else {
        action = Vehs[veh_id].probe_action;
    }

    // Native 0x5941DC / 0x5941E5 takes the simple return only for bit0 set,
    // bit1 clear. All other combinations continue at 0x5942C1 action dispatch.
    if ((NetDaemonNet.action_mode_ & 1u) != 0
        && (NetDaemonNet.action_mode_ & 2u) == 0) {
        return 1;
    }

    if (action == 1) {
        // The product goes through a variable before the add (the image keeps
        // the cost_factor result in eax as the imul destination); a fresh
        // local here flips order_veh's block layout, so the dead `production`
        // carries it.
        production = cost_factor(owner, 1, -1) * VehPrototypes[Vehs[veh_id].proto_id].cost;
        Bases[base_id].minerals_accumulated += production;
        Bases[base_id].minerals_accumulated_2 = Bases[base_id].minerals_accumulated;
        Bases[base_id].production_id_last = Bases[base_id].queue_production_id[0];
        synch_base(base_id);

        const int amount_needed = range(queued_cost - Bases[base_id].minerals_accumulated, 0, 9999);
        if (owner == ConsoleGlobal.faction_id_) {
            parse_num(0, amount_needed);
            veh_battle_msg("SUPPLYASSIGN", 0);
        }
        if (IsMultiplayerNet != 0) {
            message_veh(0x2407, veh_id, 1, 0);
            NetDaemonNet.await_exec(1);
        } else {
            kill(veh_id);
        }
        return 1;
    }

    Vehs[veh_id].order = ORDER_CONVOY;
    // The convoy's resource goes in order_auto_type (Veh+0x26, image store
    // at 0x59456B), not probe_sabotage_id (+0x2D).
    Vehs[veh_id].order_auto_type = static_cast<uint8_t>(action - 2);
    return 0;
}


// Movement tutorial and terrain-delay counters persist between orders. Native
// accesses at 0x596962 and 0x5979F2 identify these four-byte windows; the
// complete original allocations and original names remain unproved.
// RECOVERY_OBJECT: v1 0x00945AF4 ?OrderVehTutorialNoticeCounter@@3HA
int OrderVehTutorialNoticeCounter = 0;
// RECOVERY_OBJECT: v1 0x00945AF8 ?VehOrderDelayCounter@@3IA
uint32_t VehOrderDelayCounter = 0;

extern int __cdecl probe(int veh_id, int target_veh_id,
                         int target_base_id, int action);
int break_treaty(int faction_id, int target_faction_id, int treaty);


// Vehicle movement: moves veh_id one step in `direction` (8 = in place),
// resolving probes, native capture, artifacts, supply, ZOC, combat, fuel and
// the move itself. `mode` is stored as NetDaemonNet's order flags word and
// read back from there (bits 0 and 1). Transcribed from the image in its
// block order; the single Popup is the function's only automatic object.
// ORIGINAL: 0x005947C0 ?order_veh@@YAHHHH@Z 0x005947C0-0x0059A1AB;0x00660F47-0x00661200
// symbol    ?order_veh@@YAHHHH@Z
// kind      game
int __cdecl order_veh(int veh_id, int direction, int mode) {
    int x;
    int v;
    int selector = 0;
    int flag_68 = 0;
    int flag_74 = 0;
    int flag_6c = 0;
    int probed = 0;
    Popup popup;
    int faction;
    int dest_x;
    int dest_y;
    int src_x;
    int src_y;
    int region;
    int target_veh;
    int base_id;
    int owner;
    int src_ocean;
    int dest_ocean;
    int goody;
    int faction_bit;
    int group_count_at_entry;
    int stealth;
    int triad;
    int territory;
    int spotted;
    int zoc_y2;
    int zoc_x2;
    int zoc_y1;
    int height;
    int zoc_x1;
    int width;


    NetDaemonNet.action_mode_ = mode;
    if (mode & 1)
        ConsoleGlobal.field_23BE4_ = 0;
    else
        ConsoleGlobal.vehicle_order_ui_state() = 0;
    src_y = Vehs[veh_id].y;
    group_count_at_entry = GameDataStateStorage.fields_.vehicle_current_count_;
    src_x = Vehs[veh_id].x;
    faction = Vehs[veh_id].faction_id;
    if (direction < 0)
        direction = 8;
    dest_x = wrap_map_x(src_x + VehMoveDeltaX[direction]);
    dest_y = src_y + VehMoveDeltaY[direction];
    if (!on_map(dest_x, dest_y))
        goto done;
    region = region_at(dest_x, dest_y);
    target_veh = stack_fix(veh_at(dest_x, dest_y));
    base_id = base_at(dest_x, dest_y);
    if (0 <= base_id)
        owner = Bases[base_id].faction_id_current;
    else if (target_veh >= 0)
        owner = Vehs[target_veh].faction_id;
    else
        owner = -1;
    src_ocean = is_ocean(src_x, src_y);
    dest_ocean = is_ocean(dest_x, dest_y);
    goody = goody_at(dest_x, dest_y);
    faction_bit = 1 << faction;

L_5949b3:
    stealth = 0;
    if (Vehs[veh_id].state & 0x4200)
        stealth = 1;
    triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
    if (!(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
        && Vehs[veh_id].order == ORDER_MOVE_TO) {
        territory = whose_territory(faction, Vehs[veh_id].waypoint_x[0],
                                    Vehs[veh_id].waypoint_y[0], NULL, 0);
        if (territory >= 0 && territory != faction
            && !(PlayersData[faction].diplo_treaties[territory] & 0xC11)) {
            Vehs[veh_id].order = ORDER_NONE;
            goto done;
        }
    }

    // 594a5f: alien artifact entering a friendly base.
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT
        && !(Vehs[veh_id].flags & 0x20)) {
        if (NetDaemonNet.action_mode_ & 1) {
            Vehs[veh_id].terraforming_turns = 0;
            if (base_id >= 0 && Bases[base_id].faction_id_current == faction) {
                parse_says(0, Bases[base_id].name_string, -1, -1);
                popup.start(PopupStartCaption, "ARTIFACT", -1, NULL, 0, NULL);
                if (Buffer::get_pcx_dimensions("art_dis_sm.pcx", &width, &height) == 0
                    && popup.owned_picture().init("art_dis_sm.pcx", width, height) == 0)
                    popup.select_owned_picture();
                parse_num(0, Bases[base_id].minerals_accumulated);
                int cost;
                if (Bases[base_id].queue_production_id[0] < 0)
                    cost = cost_factor(faction, 1, -1)
                         * Facility[-Bases[base_id].queue_production_id[0]].cost;
                else
                    cost = veh_cost(Bases[base_id].queue_production_id[0], base_id, NULL)
                         * cost_factor(faction, 1, -1);
                parse_num(1, cost);
                if (X_text_open(ScriptPopupFile, "ARTIFACTMENU") == 0) {
                    StringTemp[0] = 0;
                    parse_string(text_get(), StringTemp);
                    popup.dialogs().item(StringTemp, 0);
                    int rows = 1;
                    LPSTR text = text_get();
                    if (has_fac(FAC_NETWORK_NODE, base_id, 0)
                        && GameDataStateStorage.fields_.secret_project_.universal_translator() != base_id
                        && !(Bases[base_id].state & BSTATE_ARTIFACT_ALREADY_LINKED)) {
                        StringTemp[0] = 0;
                        parse_string(text, StringTemp);
                        popup.dialogs().item(StringTemp, 1);
                        rows++;
                        if (!(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))
                            Vehs[veh_id].terraforming_turns = 1;
                    }
                    text = text_get();
                    if (GameDataStateStorage.fields_.secret_project_.universal_translator() == base_id) {
                        StringTemp[0] = 0;
                        parse_string(text, StringTemp);
                        popup.dialogs().item(StringTemp, 1);
                        rows++;
                        if (!(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))
                            Vehs[veh_id].terraforming_turns = 1;
                    }
                    text = text_get();
                    if (Bases[base_id].queue_production_id[0] <= -70) {
                        StringTemp[0] = 0;
                        int facility = Bases[base_id].queue_production_id[0];
                        if (facility < 0)
                            facility = -facility;
                        parse_say(1, Facility[facility].name, -1, -1);
                        parse_string(text, StringTemp);
                        popup.dialogs().item(StringTemp, 2);
                        rows++;
                        if (!(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))
                            Vehs[veh_id].terraforming_turns = 2;
                    }
                    text = text_get();
                    if (Bases[base_id].queue_production_id[0] >= 0
                        && Bases[base_id].queue_production_id[0] >= 64
                        && !(VehPrototypes[Bases[base_id].queue_production_id[0]].flags & 4)) {
                        StringTemp[0] = 0;
                        parse_says(2, VehPrototypes[Bases[base_id].queue_production_id[0]].veh_name, -1, -1);
                        parse_string(text, StringTemp);
                        popup.dialogs().item(StringTemp, 2);
                        rows++;
                    }
                    if (rows > 1 && faction == ConsoleGlobal.faction_id_
                        && (faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
                        Vehs[veh_id].terraforming_turns = popup.exec(0, NULL);
                        synch_veh(veh_id);
                        NetDaemonNet.await_synch();
                    }
                }
            }
        }
    }

    // 594e09
    if (direction >= 8) {
        if (get_proto_triad(Vehs[veh_id].proto_id) == TRIAD_AIR
                && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range != 0
                && (NetDaemonNet.action_mode_ & 1) != 0
                && faction == ConsoleGlobal.faction_id_
                && base_who(dest_x, dest_y) < 0
                && (bit_at(dest_x, dest_y) & BIT_AIRBASE) == 0) {
            for (v = veh_top(veh_id); v >= 0; v = veh_next_stack_id(v)) {
                // Tests the mover, not the stack member (bug kept).
                if (has_abil(Vehs[veh_id].proto_id, ABL_CARRIER))
                    goto L_596573;
                if (VehPrototypes[Vehs[v].proto_id].plan == PLAN_SUPPLY_CONVOY
                        && Chassis[VehPrototypes[Vehs[v].proto_id].chassis_id].triad == TRIAD_AIR)
                    goto L_596573;
            }
            if (veh_moves(veh_id)
                    && (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range == 1
                        || Vehs[veh_id].terraforming_turns != 0)) {
                action_home(veh_id, 0);
                if (Vehs[veh_id].order == ORDER_MOVE_TO) {
                    if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                        synch_veh(veh_id);
                        NetDaemonNet.await_synch();
                    }
                    return 0;   // 5964cf
                }
            }
        }
    L_596573:
        selector = 1;
        if ((NetDaemonNet.action_mode_ & 2) == 0)
            goto done;
        veh_skip(veh_id);
        Vehs[veh_id].state &= ~0x40U;
        goto L_598cb9;
    }
    Vehs[veh_id].state |= 4;
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_INFO_WARFARE
        && owner >= 0 && faction != owner) {
        if (NetDaemonNet.action_mode_ & 1) {
            Vehs[veh_id].flags &= ~4;
            if ((PlayersData[faction].diplo_treaties[owner] & 1) && !stealth
                && faction == ConsoleGlobal.faction_id_) {
                if (base_id >= 0) {
                    parse_says(0, Bases[base_id].name_string, -1, -1);
                    parse_says(1, get_adjective(owner), -1, -1);
                    if (!X_pop("PROBEPACT", NULL))
                        goto L_595083;
                    Vehs[veh_id].flags |= 4;
                } else if (stack_check(target_veh, 2, PLAN_INFO_WARFARE, owner, -1)) {
                    parse_says(0, get_adjective(owner), -1, -1);
                    if (!X_pop("PROBEPACTTEAM", NULL))
                        goto L_595083;
                    Vehs[veh_id].flags |= 4;
                }
            }
        }
        // 594f3c
        if (base_id >= 0) {
            if (target_veh >= 0 && stack_check(target_veh, 2, PLAN_INFO_WARFARE, -1, -1))
                goto L_595083;
            if (!(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
                && (PlayersData[faction].diplo_treaties[owner] & 1)
                && (Vehs[veh_id].waypoint_x[0] != Bases[base_id].x
                    || Vehs[veh_id].waypoint_y[0] != Bases[base_id].y))
                goto L_595083;
            if (owner == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.focus(src_x, src_y, faction);
                if (!IsMultiplayerNet)
                    clock_wait(400);
                stack_veh(veh_id, 1);
                veh_scoot(veh_id, src_x, src_y, direction, 0);
                stack_put(veh_id, src_x, src_y);
                draw_tile(dest_x, dest_y, 2);
            }
            if (probe(veh_id, base_id, -1, 1) && IsMultiplayerNet && !ConsoleControlTurnActive
                    && (NetDaemonNet.action_mode_ & 1) != 0
                    && (NetDaemonNet.action_mode_ & 2) == 0) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
                return 1;   // 596667
            }
            goto done;
        }
        if (!(PlayersData[faction].diplo_treaties[owner] & 1)) {
            if (target_veh >= 0 && stack_check(target_veh, 2, PLAN_INFO_WARFARE, -1, -1))
                goto L_595083;
            probed = 1;
            if (probe(veh_id, -1, target_veh, 1) && IsMultiplayerNet
                && !ConsoleControlTurnActive
                && (NetDaemonNet.action_mode_ & 1)
                && !(NetDaemonNet.action_mode_ & 2)) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
                return 1;   // 5966f6
            }
            if (GameDataStateStorage.fields_.vehicle_current_count_ != group_count_at_entry)
                goto L_599b2a;
            if (target_veh >= 0)
                owner = Vehs[target_veh].faction_id;
        }
    }

L_595083:
    if (triad == TRIAD_LAND && dest_ocean) {
        if (faction != owner) {
            if (PlayersData[faction].diplo_treaties[owner] & 1)
                goto done;
            if (base_id < 0) {
                if (target_veh < 0)
                    goto done;
                if (!has_abil(Vehs[veh_id].proto_id, ABL_AIR_SUPERIORITY))
                    goto done;
                if (!stack_check(target_veh, 0x12, -1, -1, -1))
                    goto done;
                if (stack_check(target_veh, 6, ABL_CARRIER, -1, -1))
                    goto done;
                goto L_59512c;
            } else {
                if (!has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)) {
                    if (faction == ConsoleGlobal.faction_id_
                            && (NetDaemonNet.action_mode_ & 1) != 0) {
                        parse_say(0, Ability[abil_index(ABL_AMPHIBIOUS)].name, -1, -1);
                        NetMsgPrimary.pop("AMPHIBBASE", 5000, 0, NULL);
                    }
                    goto done;
                }
            }
        } else {
            // 5952c6: own unit/base on an ocean tile: transport check.
            if (stack_check(target_veh, 8, faction, -1, -1) <= 0
                && (base_id < 0
                    || (!has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)
                        && !stack_check(target_veh, 2, PLAN_NAVAL_TRANSPORT, -1, -1)
                        && !stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1)))) {
                if (base_id >= 0 && !has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)
                        && !stack_check(target_veh, 2, PLAN_NAVAL_TRANSPORT, -1, -1)
                        && !stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1)) {
                    if (faction == ConsoleGlobal.faction_id_
                            && (NetDaemonNet.action_mode_ & 1) != 0) {
                        parse_say(0, Ability[abil_index(ABL_AMPHIBIOUS)].name, -1, -1);
                        NetMsgPrimary.pop("AMPHIBBASE2", 5000, 0, NULL);
                    }
                    goto done;
                }
                if (stack_check(target_veh, 2, PLAN_NAVAL_TRANSPORT, -1, -1)
                        && faction == ConsoleGlobal.faction_id_
                        && (NetDaemonNet.action_mode_ & 1) != 0) {
                    NetMsgPrimary.pop(stack_check(target_veh, 2, PLAN_NAVAL_TRANSPORT, -1, -1) > 1
                                          ? "FULLTRANSPORTS" : "FULLTRANSPORT", 5000, 0, NULL);
                }
                goto done;
            }
            if (base_id < 0)
                flag_68 = 1;
        }
    }

    // 59533d
    if (!src_ocean && !dest_ocean
        && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_LAND
        && !(VehPrototypes[Vehs[veh_id].proto_id].ability_flags & ABL_CLOAKED)
        && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE
        && target_veh < 0
        && zoc_move(src_x, src_y, faction)
        && zoc_move(dest_x, dest_y, faction)) {
        if (faction != ConsoleGlobal.faction_id_ || stealth
                || (NetDaemonNet.action_mode_ & 1) == 0)
            goto done;
        g_FX.play(13);
        if (faction == ConsoleGlobal.faction_id_ && !IsMultiplayerNet
                && ((GameDataStateStorage.fields_.game_preferences_ & 0x20) != 0
                    || (PlayersData[faction].diff_level <= 1
                        && (++OrderVehTutorialNoticeCounter & 3) == 1))) {

            parse_say(0, Ability[abil_index(ABL_CLOAKED)].name, -1, -1);
            quick_zoc(src_x, src_y, faction, dest_x, dest_y, &zoc_x1, &zoc_y1);
            quick_zoc(dest_x, dest_y, faction, src_x, src_y, &zoc_x2, &zoc_y2);
            TutorialWindow.tut_map("ZOCMSG", zoc_x1, zoc_y1, veh_id, NULL, 0, zoc_x2, zoc_y2);
        }
        if (IsMultiplayerNet)
            goto done;
        probed = 0;
        {
        for (int dir = 0; dir < 8; dir++) {
            x = wrap_map_x(dest_x + VehMoveDeltaX[dir]);
            int y = dest_y + VehMoveDeltaY[dir];
            if (!on_map(x, y))
                continue;
            int spotted = stack_fix(veh_at(x, y));
            if (spotted >= 0 && Vehs[spotted].faction_id != faction
                    && !(PlayersData[faction].diplo_treaties[Vehs[spotted].faction_id] & 1)
                    && !(faction_bit & Vehs[spotted].visibility)) {
                spot_loc(x, y, faction);
                probed = 1;
            }
        }
        }
        if (probed)
            draw_radius(dest_x, dest_y, 1, 2);
        goto done;
    }

    // 5953ba: supply convoy entering a base.
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_SUPPLY_CONVOY
        && base_id >= 0 && !IsMultiplayerNet) {
        int shown = supply_options(veh_id, base_id);
        if (!(NetDaemonNet.action_mode_ & 2)) {
            if (!shown)
                goto done;
            if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
                return 1;   // 596b32
            }
        } else {
            if (shown)
                goto done;
        }
    }

    // 595422
    if (!(NetDaemonNet.action_mode_ & 2))
        goto L_59512c;
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR)
        goto L_59512c;
    if (target_veh >= 0)
        goto L_595137;
    if (faction == 0)
        goto L_59512c;
    {
        // Native life may spawn on a fungus tile the unit enters.
        Map *tile = map_loc(dest_x, dest_y);
        if (!(tile->bit & BIT_FUNGUS))
            goto L_59512c;
        if ((tile->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF)
            goto L_59512c;
        if (tile->bit & BIT_MAGTUBE)
            goto L_59512c;
    }
    if ((Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating == 0
         || Vehs[veh_id].order == ORDER_MOVE_TO)
        && has_project(SP_XENOEMPATYH_DOME, faction))
        goto L_59512c;
    if (src_ocean != dest_ocean)
        goto L_59512c;
    if (goody_at(dest_x, dest_y))
        goto L_59512c;
    if (GameDataStateStorage.fields_.game_rules_ & 0x200000)
        goto L_59512c;
    base_find(dest_x, dest_y);
    if (BaseFindDist <= 1)
        goto L_59512c;
    {
        int chance = range(BaseFindDist, 0, 32) + MapNativeLifeForms * 10;
        if (map_loc(dest_x, dest_y)->bit & BIT_SUPPLY_REMOVE) {
            chance /= 2;
            if (!MapNativeLifeForms)
                chance /= 2;
        }
        if (dest_ocean) {
            chance /= 2;
            if (!MapNativeLifeForms)
                chance /= 2;
        }
        if (map_loc(dest_x, dest_y)->bit & BIT_ROAD)
            chance /= 2;
        if (Vehs[veh_id].proto_id < 64
            && (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating < 0
                || Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER))
            chance /= 2;
        for (int i = 0; i < 25; i++) {
            int x = wrap_map_x(dest_x + RadiusOffsetX[i]);
            int y = dest_y + RadiusOffsetY[i];
            if (on_map(x, y) && (is_sensor(x, y) || base_who(x, y) >= 0)) {
                if (i < 9)
                    chance /= 4;
                else
                    chance /= 2;
            }
        }
        if (rand() % 100 < chance) {
            int spawn_proto;
            if (dest_ocean)
                spawn_proto = BSC_ISLE_OF_THE_DEEP;
            else if (rand() % 5 == 0 && ExpansionEnabled)
                spawn_proto = BSC_SPORE_LAUNCHER;
            else
                spawn_proto = BSC_MIND_WORMS;
            target_veh = veh_init(spawn_proto, 0, dest_x, dest_y);
            if (target_veh >= 0) {
                // WORKAROUND(inline): dead budget spend (supply_options' probe, see
                // its declaration). With every message block written at its test
                // (literal order) and no probe, the sites are 6,6,6,6,7,16 deep
                // against the image's 5,5,5,5,7,15; five to eight of these calls,
                // anywhere in the body, give the final teardown's 15. No position or
                // count moves the four early sites (structural, not spend). The
                // original spending is not identified.
                supply_options_budget_probe(2);
                supply_options_budget_probe(2);
                supply_options_budget_probe(2);
                supply_options_budget_probe(2);
                supply_options_budget_probe(2);
                Vehs[veh_id].order = ORDER_NONE;
                spot_stack(target_veh, faction);
                draw_tile(dest_x, dest_y, 2);
                bit_set(dest_x, dest_y, 0x100000, 1);
                if (faction == ConsoleGlobal.faction_id_) {
                    g_FX.play(13);
                    parse_says(0, get_adjective(owner), -1, -1);
                    parse_says(1, VehPrototypes[Vehs[target_veh].proto_id].veh_name, -1, -1);
                    NetMsgPrimary.pop(dest_ocean ? "ISLESPOTTED" : "WORMSSPOTTED", 5000, 0, NULL);
                }
                goto done;
            }
            goto L_596052;
        }
    }

L_59512c:
    if (target_veh < 0)
        goto L_596052;
L_595137:
    if (!(Vehs[target_veh].flags & 0x400)) {
        if (faction == owner)
            goto L_596052;
        if ((PlayersData[faction].diplo_treaties[owner] & 1)
            && (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE
                || !(Vehs[veh_id].flags & 4)))
            goto L_596052;
    }
    if (probed)
        goto done;
    if (Vehs[target_veh].flags & 0x400) {
        Vehs[veh_id].order = ORDER_NONE;
        spot_stack(target_veh, faction);
        draw_tile(dest_x, dest_y, 2);
        if (faction == ConsoleGlobal.faction_id_) {
            g_FX.play(13);
            parse_says(0, get_adjective(owner), -1, -1);
            parse_says(1, VehPrototypes[Vehs[target_veh].proto_id].veh_name, -1, -1);
            NetMsgPrimary.pop(owner ? "SPOTTED" : "SPOTTED0", 5000, 0, NULL);
        }
        goto done;
    }
    if ((Vehs[target_veh].faction_id != faction
         || (Vehs[target_veh].flags & 0x440) == 0x440)
        && !(faction_bit & Vehs[target_veh].visibility)
        && (faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
        if (base_id >= 0)
            goto L_595734;
        if ((NetDaemonNet.action_mode_ & 1)
            && faction == ConsoleGlobal.faction_id_) {
            Vehs[veh_id].order = ORDER_NONE;
            spot_stack(target_veh, faction);
            draw_tile(dest_x, dest_y, 2);
            g_FX.play(13);
            parse_says(0, get_adjective(owner), -1, -1);
            parse_says(1, VehPrototypes[Vehs[target_veh].proto_id].veh_name, -1, -1);
            NetMsgPrimary.pop(owner ? "SPOTTED" : "SPOTTED0", 5000, 0, NULL);
            goto done;
        }
    }
    // 59521f
    if (base_id >= 0)
        goto L_595734;
    if (map_loc(dest_x, dest_y)->bit & BIT_AIRBASE)
        goto L_595734;
    if (stack_check(target_veh, 0x12, -1, -1, -1)
        && !has_abil(Vehs[veh_id].proto_id, ABL_AIR_SUPERIORITY)
        && !stack_check(target_veh, 6, ABL_CARRIER, -1, -1)) {
        if (faction == ConsoleGlobal.faction_id_
                && (NetDaemonNet.action_mode_ & 1) != 0) {
            parse_say(0, Ability[abil_index(ABL_AIR_SUPERIORITY)].name, -1, -1);
            NetMsgPrimary.pop("FIGHTERRULE", 5000, 0, NULL);
        }
        if (Vehs[veh_id].state & 0x4200)
            veh_skip(veh_id);
        goto done;
    }
    goto L_595734;

L_595734:
    if (Vehs[veh_id].proto_id != BSC_SEALURK && src_ocean && !dest_ocean
        && triad == TRIAD_SEA && !can_arty(Vehs[veh_id].proto_id, 1)
        && Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating) {
        if (faction == ConsoleGlobal.faction_id_
                && (NetDaemonNet.action_mode_ & 1) != 0)
            NetMsgPrimary.pop("PSIBOMB", 5000, 0, NULL);
        goto done;
    }
    if (src_ocean && triad == TRIAD_LAND
        && !has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)
        && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE) {
        {
            int ability = abil_index(ABL_AMPHIBIOUS);
            if (faction == ConsoleGlobal.faction_id_
                    && (NetDaemonNet.action_mode_ & 1) != 0) {
                parse_say(0, Ability[ability].name, -1, -1);
                if (base_who(src_x, src_y) >= 0)
                    NetMsgPrimary.pop("AMPHIBBASE", 5000, 0, NULL);
                else
                    NetMsgPrimary.pop("NOTAMPHIB", 5000, 0, NULL);
            }
        }
        goto done;
    }

    // 5957d7
    if (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].mode == WPN_MODE_INFOWAR
        && stack_check(target_veh, 2, PLAN_INFO_WARFARE, -1, -1))
        goto L_595a92;
    if (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating == 0) {
        if ((NetDaemonNet.action_mode_ & 1) != 0
                && faction == ConsoleGlobal.faction_id_) {
            g_FX.play(0x33);
            NetMsgPrimary.pop("NONCOMBATANT", 5000, 0, NULL);
        }
        goto done;
    }
    if (VehPrototypes[Vehs[target_veh].proto_id].plan != PLAN_INFO_WARFARE)
        goto L_595a92;
    if (stack_check(target_veh, 1, -1, -1, -1) != 1)
        goto L_595a92;
    if (IsMultiplayerNet)
        goto L_595a92;
    if (!(PlayersData[faction].diplo_treaties[owner] & 6))
        goto L_595a92;
    if (PlayersData[faction].diplo_treaties[owner] & 1)
        goto L_595a92;
    {
        // 5958ab: capturing an enemy probe team.
        int found_base = base_find(dest_x, dest_y);
        if (found_base >= 0 && Bases[found_base].faction_id_current == faction) {
            parse_says(0, get_adjective(owner), -1, -1);
            parse_says(1, VehPrototypes[Vehs[target_veh].proto_id].veh_name, -1, -1);
            parse_says(2, get_title(faction), -1, -1);
            if (!(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))
                goto L_596fbc;
            int picked;
            if (Players[owner].rule_flags & RFLAG_ALIEN)
                picked = popp(ScriptPopupFile, "ENEMYPROBE", 0, "al_cap_sm.pcx", NULL);
            else
                picked = popp(ScriptPopupFile, "ENEMYPROBE", 0, "capture_sm.pcx", NULL);
            if (!picked)
                goto done;
            if (picked == 1) {
            L_596fbc:
                if (faction == ConsoleGlobal.faction_id_ || owner == ConsoleGlobal.faction_id_) {
                    ConsoleGlobal.focus(src_x, src_y, faction);
                    stack_veh(veh_id, 1);
                    veh_scoot(veh_id, src_x, src_y, direction, 0);
                    stack_put(veh_id, src_x, src_y);
                    draw_tile(src_x, src_y, 2);
                }
                {
                    int best_base = -1;
                    int best_pop = -1;
                    for (int b = 0; b < GameDataStateStorage.fields_.base_current_count_; b++) {
                        if (Bases[b].faction_id_current == owner && Bases[b].population_size > best_pop) {
                            best_pop = Bases[b].population_size;
                            best_base = b;
                        }
                    }
                    if (best_base >= 0) {
                        stack_put(target_veh, Bases[best_base].x, Bases[best_base].y);
                        Vehs[target_veh].visibility = 0;
                        parse_says(2, Bases[best_base].name_string, -1, -1);
                    } else if (((1 << owner)
                                & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0) {
                        veh_kill(target_veh);
                    }
                }
                draw_tile(dest_x, dest_y, 2);
                if (owner == ConsoleGlobal.faction_id_) {
                    parse_says(0, get_adjective(faction), -1, -1);
                    parse_says(1, get_title(owner), -1, -1);
                    NetMsgPrimary.pop("GOTMYPROBE", 5000, 0, NULL);
                } else if (faction == ConsoleGlobal.faction_id_) {
                    parse_says(0, get_adjective(owner), -1, -1);
                    parse_says(1, get_title(faction), -1, -1);
                    NetMsgPrimary.pop("GOTYOURPROBE", 5000, 0, NULL);
                }
                goto done;
            }
            goto L_595a92;
        }
        if (faction == ConsoleGlobal.faction_id_) {
            parse_says(0, get_adjective(owner), -1, -1);
            parse_says(1, VehPrototypes[Vehs[target_veh].proto_id].veh_name, -1, -1);
            parse_says(2, get_title(faction), -1, -1);
            int picked;
            if (Players[owner].rule_flags & RFLAG_ALIEN)
                picked = popp(ScriptPopupFile, "ENEMYPROBE2", 0, "al_cap_sm.pcx", NULL);
            else
                picked = popp(ScriptPopupFile, "ENEMYPROBE2", 0, "capture_sm.pcx", NULL);
            if (!picked)
                goto done;
        }
    }

L_595a92:
    // HASTY: attacking with less than a full move left.
    if (veh_moves(veh_id) < Rules.move_rate_roads
        && Vehs[veh_id].proto_id != BSC_SPORE_LAUNCHER
        && Vehs[veh_id].proto_id != BSC_MIND_WORMS) {
        if (faction == ConsoleGlobal.faction_id_ && !stealth
            && (NetDaemonNet.action_mode_ & 1)) {
            parse_says(0, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
            parse_num(0, veh_moves(veh_id));
            parse_num(1, Rules.move_rate_roads);
            if (popp(ScriptPopupFile, "HASTY", 0, "bad_sm.pcx", NULL))
                goto done;
        } else if (!(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
            if (veh_moves(veh_id) <= Rules.move_rate_roads / 2)
                goto done;
        }
    }
    // falls through to L_595be6
L_595be6:
    GameDataStateStorage.fields_.vehicle_selection_follow_limit_ = veh_id;
    if ((NetDaemonNet.action_mode_ & 2) == 0)
        goto L_59729d;
    if (owner != 0)
        goto L_59724d;
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR)
        goto L_59724d;
    if (Vehs[target_veh].proto_id == BSC_ALIEN_ARTIFACT)
        goto L_59724d;
    if (Vehs[target_veh].proto_id == BSC_FUNGAL_TOWER)
        goto L_59724d;
    if (PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET] <= 0)
        goto L_59724d;
    if (veh_moves(veh_id) < Rules.move_rate_roads) {
        if (PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET] <= 0)
            goto L_59724d;
        if (PlayersData[faction].proto_id_active[Vehs[target_veh].proto_id] != 0)
            goto L_59724d;
    }
    {
        int nearby_base = base_find(dest_x, dest_y, faction);
        if ((NetDaemonNet.action_mode_ & 2) == 0)
            goto L_59724d;
        if (Vehs[target_veh].order_auto_type == faction)
            goto L_597216;
        if (nearby_base >= 0 && Bases[nearby_base].eco_damage != 0)
            goto L_597216;
    }
    log_say("Precapture who, enemy", faction, target_veh, 0);
    log_say("Precapture fac, proto",
            PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET],
            PlayersData[faction].proto_id_active[Vehs[target_veh].proto_id], 0);
    log_say("Precapture type, soc2", Vehs[target_veh].proto_id,
            PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET], 0);
    if ((Vehs[target_veh].flags & 0x2000) != 0)
        goto L_5971ce;
    if (rand() % 4 < PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET]) {
        int bound = GameDataStateStorage.fields_.turn_current_num_ / 5 + 100;
        int roll;
        if (bound - 1 <= 0)
            roll = 0;
        else
            roll = rand() % bound;
        if (roll >= PlayersData[faction].proto_id_active[Vehs[target_veh].proto_id] * 10
                || rand() % 10 == 0)
            goto L_595e1b;
    }
    if (PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET] <= 0)
        goto L_5971ce;
    if (PlayersData[faction].proto_id_active[Vehs[target_veh].proto_id] != 0)
        goto L_5971ce;

L_595e1b:
    {
    for (v = veh_top(target_veh); v >= 0; v = veh_next_stack_id(v)) {
        Vehs[v].faction_id = (uint8_t)faction;
        Vehs[v].morale = (uint8_t)morale_alien(v, 0);
        int b = base_find(dest_x, dest_y);
        // Bases[-1] is read when no base exists (bug kept).
        if (Bases[b].faction_id_current == faction && Bases[b].population_size >= 3)
            Vehs[v].home_base_id = (int16_t)b;
        else
            Vehs[v].home_base_id = -1;
        veh_skip(v);
    }
    }
    bit_set(dest_x, dest_y, 0x100000, 1);
    owner_set(dest_x, dest_y, faction);
    if (faction == ConsoleGlobal.faction_id_)
        draw_tile_fixup(dest_x, dest_y, 1, 2);
    else
        draw_tile(dest_x, dest_y, 2);
    if (faction == ConsoleGlobal.faction_id_) {
        parse_says(0, VehPrototypes[Vehs[target_veh].proto_id].veh_name, -1, -1);
        if (Vehs[target_veh].proto_id == BSC_SPORE_LAUNCHER) {
            if (PlayersData[faction].proto_id_active[BSC_SPORE_LAUNCHER] == 0)
                popp(ScriptPopupFile, "WORMCAPTURE", 0, "sporlnch_sm.pcx", 0);
            else
                NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, NULL);
        } else if (Vehs[target_veh].proto_id == BSC_SEALURK) {
            if (PlayersData[faction].proto_id_active[BSC_SEALURK] == 0)
                popp(ScriptPopupFile, "WORMCAPTURE", 0, "sealrk_sm.pcx", 0);
            else
                NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, NULL);
        } else if (PlayersData[faction].proto_id_active[Vehs[target_veh].proto_id] == 0) {
            popp(ScriptPopupFile, "WORMCAPTURE", 0,
                 is_ocean(dest_x, dest_y) ? "isle_sm.pcx" : "mindworm_sm.pcx", 0);
        } else {
            NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, NULL);
        }
    }
    log_say("Captured", 0, 0, 0);
    goto L_596052;

L_5971ce:
    Vehs[target_veh].flags |= 0x2000;
    if (faction == ConsoleGlobal.faction_id_)
        NetMsgPrimary.pop("WORMNOCAPTURE", 5000, 0, NULL);
    log_say("No Capture", 0, 0, 0);
    goto L_59724d;

L_597216:
    log_say("Agitated", 0, 0, 0);
    if (faction == ConsoleGlobal.faction_id_)
        NetMsgPrimary.pop("WORMSAGITATED", 5000, 0, NULL);

L_59724d:
    flag_6c = 1;
    switch (battle_fight(veh_id, direction, 0, 1, 0)) {
    case 0:
        goto done;
    default:
        selector = 0;
        goto done;
    case 1:
        veh_id = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
        dest_y = src_y;
        dest_x = src_x;
        flag_74 = 1;
        goody = 0;
        goto L_598cb9;
    }

L_59729d:
    selector = battle_fight(veh_id, direction, 0, 1, 0) == 0;
    goto done;


L_596052:
    if (src_ocean) {
        if (dest_ocean)
            goto L_597345;
        if (triad == TRIAD_LAND && target_veh < 0
                && !has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)
                && base_who(src_x, src_y) >= 0
                && !stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1)) {
            if (faction == ConsoleGlobal.faction_id_
                    && (NetDaemonNet.action_mode_ & 1) != 0) {
                parse_say(0, Ability[abil_index(ABL_AMPHIBIOUS)].name, -1, -1);
                veh_battle_msg("AMPHIBBASE2", NULL);
            }
            goto done;
        }
    } else {
        if (dest_ocean)
            goto L_597345;
    }
    if (base_id >= 0)
        goto L_597350;
    if (triad == TRIAD_SEA && IsMultiplayerNet)
        goto done;
    if (triad != TRIAD_SEA || IsMultiplayerNet)
        goto L_597345;
    if (faction != ConsoleGlobal.faction_id_)
        goto done;
    if ((NetDaemonNet.action_mode_ & 1) == 0)
        goto done;
    // Disembark dialog.
    Vehs[veh_id].order = ORDER_NONE;
    parse_says(0, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
    popup.start(PopupStartCaption, "DISEMBARK", -1, NULL, 0, NULL);
    if (X_text_open(ScriptPopupFile, "DISEMBARKERS"))
        goto done;
    text_get();
    stack_veh(veh_id, 0);
    {
        int last_moving = -1;
        int attached = 0;
        for (v = veh_top(veh_id); v >= 0; v = veh_next_stack_id(v)) {
            if (Vehs[v].order == ORDER_SENTRY_BOARD && Vehs[v].waypoint_x[0] == veh_id) {
                attached++;
                if (veh_moves(v)) {
                    last_moving = v;
                    StringTemp[0] = '\0';
                    parse_says(1, VehPrototypes[Vehs[v].proto_id].veh_name, -1, -1);
                    parse_string(TextBufferGetPtr, StringTemp);
                    popup.dialogs().item(StringTemp, v + 1);
                }
            }
        }
        if (last_moving >= 0) {
            int chosen = popup.exec(0, NULL) - 1;
            if (chosen < 0)
                goto done;
            veh_id = chosen;
            Vehs[chosen].order = ORDER_NONE;
            goto L_5949b3;
        } else {
            // Image places this else arm out of line at 59731d.
            if (attached)
                NetMsgPrimary.pop("DISEMBARKFAIL", 5000, 0, NULL);
            goto done;
        }
    }

L_597345:
    if (base_id >= 0) {
L_597350:
        if (owner != faction
                && !(PlayersData[faction].diplo_treaties[owner] & 1)) {
            if (triad == TRIAD_AIR) {
                if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range) {
                    if (faction == ConsoleGlobal.faction_id_
                            && (NetDaemonNet.action_mode_ & 1)) {
                        parse_say(0, Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].offsv_1_name, -1, -1);
                        NetMsgPrimary.pop("AIRCONQUER", 5000, 0, 0);
                    }
                    goto done;
                }
            } else if (triad == TRIAD_SEA) {
                if (!dest_ocean) {
                    if (faction == ConsoleGlobal.faction_id_
                            && (NetDaemonNet.action_mode_ & 1)) {
                        NetMsgPrimary.pop("SEACONQUER", 5000, 0, 0);
                    }
                    goto done;
                }
            } else if (triad == TRIAD_LAND && dest_ocean
                    && !has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)) {
                if (faction == ConsoleGlobal.faction_id_
                        && (NetDaemonNet.action_mode_ & 1)) {
                    NetMsgPrimary.pop("LANDCONQUER", 5000, 0, 0);
                }
                goto done;
            }
            if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating) {
                if (faction == ConsoleGlobal.faction_id_
                        && (NetDaemonNet.action_mode_ & 1)) {
                    NetMsgPrimary.pop("COMBATCONQUER", 5000, 0, 0);
                }
                goto done;
            }
            if (PlayersData[faction].diplo_treaties[owner] & 7) {
                if (!(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
                    Vehs[veh_id].order = ORDER_NONE;
                    goto done;
                }
                if (faction == ConsoleGlobal.faction_id_
                        && (NetDaemonNet.action_mode_ & 1)
                        && break_treaty(faction, owner, 7)) {
                    goto done;
                }
            }
        }
    }
    // 597564
    if (faction == ConsoleGlobal.faction_id_
            && (faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
            && (NetDaemonNet.action_mode_ & 1)
            && Vehs[veh_id].order == ORDER_MOVE_TO
            && (Vehs[veh_id].waypoint_x[0] != dest_x
                || Vehs[veh_id].waypoint_y[0] != dest_y)
            && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != TRIAD_AIR
            && !(Vehs[veh_id].state & 0x4200)
            && goody_at(dest_x, dest_y)
            && popp(ScriptPopupFile, "GOTOGOODY", 0, "supply_sm.pcx", 0)) {
        Vehs[veh_id].order = ORDER_NONE;
        goto done;
    }
    // 59763b
    if (!veh_moves(veh_id)) {
        goto done;
    }
    {
        int cost = hex_cost(Vehs[veh_id].proto_id, faction, src_x, src_y,
                            dest_x, dest_y, 0);
        if (cost > veh_moves(veh_id)
                && triad == TRIAD_LAND
                && (veh_moves(veh_id) < Rules.move_rate_roads
                    || ((map_loc(dest_x, dest_y)->bit & BIT_FUNGUS)
                        && (map_loc(dest_x, dest_y)->climate & ~0x1F) >= 0x40
                        && (Vehs[veh_id].proto_id >= 64
                            || Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating >= 0)
                        && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_TERRAFORMING
                        && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_ALIEN_ARTIFACT
                        && target_veh < 0))
                && ((NetDaemonNet.action_mode_ & 1)
                    || !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit))) {
            // 5977c7
            int roll;
            if (ConsoleControlTurnActive) {
                roll = bounded_crt_random(cost);
            } else {
                roll = random(0, cost);
            }
            if (roll >= veh_moves(veh_id)
                    + ((Vehs[veh_id].state & 0x8000) ? Rules.move_rate_roads : 0)) {
                if (faction == ConsoleGlobal.faction_id_
                        && !Vehs[veh_id].order
                        && !HIBYTE(GetAsyncKeyState(VK_SHIFT))
                        && (NetDaemonNet.action_mode_ & 1)
                        && !(Vehs[veh_id].state & 0x200)
                        && !(GameDataStateStorage.fields_.game_more_preferences_ & 0x80)) {
                    ConsoleGlobal.focus(src_x, src_y, ConsoleGlobal.faction_id_);
                    stack_veh(veh_id, 1);
                    veh_scoot(veh_id, src_x, src_y, direction + 8, 0);
                    stack_put(veh_id, src_x, src_y);
                    for (int i = 0; i < 8; i++) {
                        int x = wrap_map_x(dest_x + VehMoveDeltaX[i]);
                        int y = dest_y + VehMoveDeltaY[i];
                        if (on_map(x, y)) {
                            spot_loc(x, y, faction);
                        }
                    }
                    draw_tiles(dest_x, dest_y, 2);
                }
                // 597969
                if (faction == ConsoleGlobal.faction_id_
                        && (map_loc(dest_x, dest_y)->bit & BIT_FUNGUS)
                        && (map_loc(dest_x, dest_y)->climate & ~0x1F) >= 0x40
                        && !Vehs[veh_id].order
                        && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_TERRAFORMING
                        && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_ALIEN_ARTIFACT
                        && (NetDaemonNet.action_mode_ & 1)) {
                    if (tut_check(0x1000)) {
                        ++VehOrderDelayCounter;
                        g_FX.play(9);
                        TutorialWindow.tut_map("SLOWFUNGUS", dest_x, dest_y, -1,
                                               NULL, 0, -1, -1);
                        Vehs[veh_id].order = ORDER_NONE;
                        goto done;
                    }
                    if (!stealth) {
                        ++VehOrderDelayCounter;
                    }
                }
                // 597a37
                if (faction == ConsoleGlobal.faction_id_
                        && !Vehs[veh_id].order
                        && !TutorialWindow.tutorial_blocked()
                        && (NetDaemonNet.action_mode_ & 1)
                        && (!(map_loc(dest_x, dest_y)->bit & BIT_FUNGUS)
                            || (map_loc(dest_x, dest_y)->climate & ~0x1F) < 0x40)
                        && tut_check(0x4000)) {
                    TutorialWindow.tut_map("FRACTIONALMOVE", dest_x, dest_y, veh_id,
                                           NULL, 0, src_x, src_y);
                    Vehs[veh_id].order = ORDER_NONE;
                    goto done;
                }
                // 597acf
                veh_skip(veh_id);
                Vehs[veh_id].state |= 0x8000;
                if ((NetDaemonNet.action_mode_ & 1)
                        && IsMultiplayerNet && !ConsoleControlTurnActive) {
                    synch_veh(veh_id);
                    NetDaemonNet.await_synch();
                }
                selector = 1;
                goto done;
            }
        }
        // 597b24
        selector = 1;
        if (!(NetDaemonNet.action_mode_ & 2)) {
            goto done;
        }
        Vehs[veh_id].moves_expended += cost;
        Vehs[veh_id].state &= ~0x8040;
        if (base_id >= 0 && owner != faction
                && !(PlayersData[faction].diplo_treaties[owner] & 1)) {
            if ((PlayersData[owner].diplo_treaties[faction] & 7)
                    && owner == ConsoleGlobal.faction_id_) {
                // get_title / get_name / get_noun, expanded
                parse_says(0, get_title(faction), -1, -1);
                parse_says(1, get_name(faction), -1, -1);
                parse_says(2, get_noun(faction), -1, -1);
                NetMsgPrimary.pop("SURPRISE", 5000, 0, 0);
            }
            if (faction != ConsoleGlobal.faction_id_
                    && owner != ConsoleGlobal.faction_id_
                    && (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
                        || (Bases[base_id].visibility & (1 << ConsoleGlobal.faction_id_)))
                    && faction) {
                spot_stack(veh_id, ConsoleGlobal.faction_id_);
            }
        }
        // 597cce
        spotted = (faction == ConsoleGlobal.faction_id_
                   || (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_
                       && (Vehs[veh_id].flags & 0x440) != 0x440)
                   || (Vehs[veh_id].visibility & (1 << ConsoleGlobal.faction_id_))
                   || (GameDataStateStorage.fields_.game_state_ & 0x80));
        // 597d18
        if (((GameDataStateStorage.fields_.game_state_ & 0x80)
                || (PlayersData[ConsoleGlobal.faction_id_].flags & 0x200)
                || (map_loc(dest_x, dest_y)->visibility & (1 << ConsoleGlobal.faction_id_)))
                && !spotted) {
            for (int i = 0; !spotted && i < 21; i++) {
                int x = wrap_map_x(dest_x + RadiusOffsetX[i]);
                int y = dest_y + RadiusOffsetY[i];
                if (on_map(x, y)) {
                    if (is_ocean(x, y) != is_ocean(dest_x, dest_y)
                            || region_at(x, y) == region_at(dest_x, dest_y)) {
                        if (is_sensor(x, y)
                                && whose_territory(ConsoleGlobal.faction_id_, x, y, NULL, 0)
                                   == ConsoleGlobal.faction_id_) {
                            spotted = 1;
                        }
                        if ((i < 9 ? anything_at(x, y) : base_who(x, y))
                                == ConsoleGlobal.faction_id_) {
                            spotted = 1;
                        }
                    }
                }
            }
        }
        // 597f26
        if (spotted) {
            stealth = (faction == ConsoleGlobal.faction_id_
                       || ((GameDataStateStorage.fields_.game_preferences_ & 4)
                           && !(PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & 1))
                       || ((GameDataStateStorage.fields_.game_preferences_ & 0x8000)
                           && (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & 1))
                       || (base_id >= 0
                           && Bases[base_id].faction_id_current != faction
                           && !(PlayersData[faction].diplo_treaties[Bases[base_id].faction_id_current] & 1)));
            int shift_down = HIBYTE(GetAsyncKeyState(VK_SHIFT));
            if (shift_down) {
                stealth = 0;
            }
            if ((!cost
                    && (faction != ConsoleGlobal.faction_id_
                        || (Vehs[veh_id].state & 0x4200)
                        || Vehs[veh_id].order == ORDER_MOVE_TO))
                    || shift_down
                    || (GameDataStateStorage.fields_.game_more_preferences_ & 0x80)
                    || ((GameDataStateStorage.fields_.game_more_preferences_ & 0x40)
                        && ((faction == ConsoleGlobal.faction_id_
                             && ((Vehs[veh_id].state & 0x4200)
                                 || Vehs[veh_id].order == ORDER_MOVE_TO))
                            || (faction != ConsoleGlobal.faction_id_
                                && triad == TRIAD_AIR)))) {
                stealth = 0;
                ConsoleGlobal.vehicle_order_ui_state() = 1;
                if (faction == ConsoleGlobal.faction_id_) {
                    ConsoleGlobal.vehicle_order_ui_state() = 2;
                    ConsoleGlobal.cursor_next(dest_x, dest_y);
                }
            }
            // 598072
            if (spotted && !HIBYTE(GetAsyncKeyState(VK_CONTROL))
                    && (stealth
                        || !(GameDataStateStorage.fields_.game_preferences_ & 0x80000))) {
                ConsoleGlobal.focus(src_x, src_y, faction);
            }
        } else {
            stealth = 0;
        }
    }
    // 5980a8
    stack_veh(veh_id, 1);
    if (stealth) {
        if (faction == ConsoleGlobal.faction_id_) {
            g_FX.engine_pitch((map_loc(src_x, src_y)->climate & ~0x1F)
                              < (map_loc(dest_x, dest_y)->climate & ~0x1F) ? 300 : 200);
        }
        veh_scoot(veh_id, src_x, src_y, direction, 0);
        if (faction == ConsoleGlobal.faction_id_) {
            g_FX.engine_pitch(0);
        }
    } else if (spotted) {
        veh_scoot(veh_id, src_x, src_y, direction, 0x4000);
    } else {
        Vehs[veh_id].facing = direction;
    }
    // 59818b
    if (faction) {
        unspot_stack(veh_id);
    } else {
        Vehs[veh_id].visibility &= map_loc(dest_x, dest_y)->visibility;
        for (territory = 1; territory < 8; territory++) {
            if (Vehs[veh_id].visibility & (1 << territory)) {
                base_find(dest_x, dest_y, territory);
                if (BaseFindDist >= 8
                        && whose_territory(territory, dest_x, dest_y, NULL, 0) != territory) {
                    Vehs[veh_id].visibility &= ~(1 << territory);
                }
            }
        }
    }
    // 598240
    if ((map_loc(dest_x, dest_y)->bit & BIT_BUNKER)
            && target_veh < 0
            && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_LAND
            && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE) {
        territory = whose_territory(faction, dest_x, dest_y, NULL, 0);
        if (territory >= 0 && territory != faction
                && !(PlayersData[faction].diplo_treaties[territory] & 1)) {
            veh_skip(veh_id);
            if (faction == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop("BUNKERCAPTURED", 5000, 0, 0);
            }
        }
    }
    // 598317
    if (base_id >= 0 && faction != owner && !faction) {
        int tries;
        int fac;
        for (tries = 0; tries <= 100; tries++) {
            fac = rand() % 70;
            int ok = 1;
            for (int b = 0; b < Players[0].faction_bonus_count; b++) {
                if ((Players[0].faction_bonus_id[b] == 2
                        || (Players[0].faction_bonus_id[b] == 12
                            && has_tech(Facility[Players[0].faction_bonus_val1[b]].preq_tech, 0)))
                        && fac == Players[0].faction_bonus_val1[b]) {
                    ok = 0;
                    break;
                }
            }
            if (fac == 26) {
                if (is_ocean(dest_x, dest_y)) {
                    ok = 0;
                }
            } else if (fac <= 1 || fac >= 65) {
                continue;
            }
            flag_68 = has_fac_built(fac, base_id);
            if (flag_68 && ok) {
                break;
            }
        }
        // 59843a
        *StringTemp = 0;
        strcat(StringTemp, "WORMSATTACK");
        parse_says(2, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
        if (tries <= 100) {
            strcat(StringTemp, "1");
            set_fac(fac, base_id, 0);
            parse_say(1, Facility[fac].name, -1, -1);
        }
        if (spotted && owner == ConsoleGlobal.faction_id_) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
        }
        kill(veh_id);
        int seen = 0;
        for (int i = 0; i < 25; i++) {
            int x = wrap_map_x(dest_x + RadiusOffsetX[i]);
            int y = dest_y + RadiusOffsetY[i];
            if (on_map(x, y)) {
                veh_id = stack_fix(veh_at(x, y));
                if (veh_id >= 0 && !Vehs[veh_id].faction_id) {
                    veh_id = veh_top(veh_id);
                    while (veh_id >= 0) {
                        int next = Vehs[veh_id].next_veh_id_stack;
                        if (seen < PlayersData[owner].planet_ecology) {
                            seen++;
                        } else {
                            veh_kill(veh_id);
                        }
                        veh_id = next;
                    }
                }
            }
        }
        // 598625
        if (GameDataStateStorage.fields_.turn_current_num_ >= 10) {
            Bases[base_id].population_size--;
            if (is_objective(base_id)) {
                Bases[base_id].population_size = max(1, Bases[base_id].population_size);
            }
            if (!Bases[base_id].population_size) {
                base_kill(base_id);
                draw_map(1);
                eliminate_player(owner, 0);
                draw_map(1);
                goto done;
            }
        }
        if (tries > 100) {
            Bases[base_id].minerals_accumulated = 0;
        }
        draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
        TutWinBaseWindow.check_base(base_id);
        goto done;
    }
    // 5986ef
    stack_put(veh_id, dest_x, dest_y);
    if (base_id >= 0) {
        if (faction != owner
                && !(PlayersData[faction].diplo_treaties[owner] & 1)) {
            if (ExpansionEnabled
                    && Vehs[veh_id].proto_id != 16
                    && Vehs[veh_id].proto_id != 17
                    && Vehs[veh_id].proto_id != 18) {
                Vehs[veh_id].dmg_incurred = 0;
            }
            GameDataStateStorage.fields_.vehicle_selection_follow_limit_ = veh_id;
            act_of_aggression(faction, owner);
            capture_base(base_id, faction, 0);
            veh_id = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
        }
    } else if (dest_ocean
            && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA) {
        // 5987c5
        int arty_count = 0;
        for (int i = 0; i < 25; i++) {
            int x = wrap_map_x(dest_x + RadiusOffsetX[i]);
            int y = dest_y + RadiusOffsetY[i];
            if (on_map(x, y)) {
                for (v = stack_fix(veh_at(x, y)); v >= 0; v = veh_next_stack_id(v)) {
                    if (can_arty(Vehs[v].proto_id, 1)
                            && Chassis[VehPrototypes[Vehs[v].proto_id].chassis_id].triad == TRIAD_LAND
                            && Vehs[v].faction_id != faction
                            && (PlayersData[faction].diplo_treaties[Vehs[v].faction_id] & 0x10)
                            && ((Vehs[veh_id].faction_id == Vehs[v].faction_id
                                 && (Vehs[veh_id].flags & 0x440) != 0x440)
                                || (Vehs[veh_id].visibility & (1 << Vehs[v].faction_id)))) {
                        arty_count++;
                        territory = Vehs[v].faction_id;
                        spot_stack(v, faction);
                    }
                }
            }
        }
        if (arty_count) {
            Vehs[veh_id].moves_expended += arty_count * Rules.move_rate_roads;
            if (faction == ConsoleGlobal.faction_id_) {
                parse_says(0, get_adjective(territory), -1, -1);
                NetMsgPrimary.pop("GIBRALTAR", 5000, 0, 0);
            }
        }
    }
    // 598993
    if (veh_cargo(veh_id) && base_id >= 0
            && (faction == ConsoleGlobal.faction_id_
                || (IsMultiplayerNet
                    && (faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)))
            && (Vehs[veh_id].order != ORDER_MOVE_TO
                || (Vehs[veh_id].waypoint_x[0] == dest_x
                    && Vehs[veh_id].waypoint_y[0] == dest_y))) {
        for (v = veh_top(veh_id); v >= 0; v = veh_next_stack_id(v)) {
            if (Vehs[v].faction_id == faction
                    && Vehs[v].order == ORDER_SENTRY_BOARD
                    && (Vehs[v].waypoint_x[0] < 0 || Vehs[v].waypoint_x[0] == veh_id)) {
                Vehs[v].order = ORDER_NONE;
            }
        }
    }
    // 598a84
    if (faction == ConsoleGlobal.faction_id_
            && (GameDataStateStorage.fields_.game_preferences_ & 0x80000000)
            && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA
            && veh_cargo(veh_id)
            && dest_ocean
            && !veh_moves(veh_id)
            && (Vehs[veh_id].order != ORDER_MOVE_TO
                || (Vehs[veh_id].waypoint_x[0] == dest_x
                    && Vehs[veh_id].waypoint_y[0] == dest_y))) {
        int found = 0;
        for (int dir = 0; dir < 8; dir++) {
            int x = wrap_map_x(dest_x + VehMoveDeltaX[dir]);
            int y = dest_y + VehMoveDeltaY[dir];
            if (on_map(x, y) && (!is_ocean(x, y) || base_who(x, y) >= 0)) {
                found = 1;
                break;
            }
        }
        if (found) {
            for (v = veh_top(veh_id); v >= 0; v = veh_next_stack_id(v)) {
                if (Chassis[VehPrototypes[Vehs[v].proto_id].chassis_id].triad == TRIAD_LAND
                        && Vehs[v].order == ORDER_SENTRY_BOARD
                        && veh_moves(v)) {
                    Vehs[v].order = ORDER_NONE;
                    ConsoleGlobal.selected_vehicle_ = v;
                }
            }
        }
    }
    // 598c83
    if (triad == TRIAD_LAND && src_ocean && !dest_ocean) {
        PlayersData[faction].region_total_combat_vehs[region]++;
    }
    // falls into L_598cb9
L_598cb9:
    if (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating == 0)
        veh_demote(veh_id);
    if (!faction) {
        if ((bit_at(dest_x, dest_y) & BIT_FUNGUS)
                && (map_loc(dest_x, dest_y)->climate & ~0x1F) >= 0x40
                && Vehs[veh_id].dmg_incurred > 0)
            Vehs[veh_id].dmg_incurred--;
    }
    spot_all(veh_id, 1);
    if (faction != ConsoleGlobal.faction_id_
            && !(static_cast<uint8_t>(faction_bit)
                 & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_))
            && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            && spotted
            && !ConsoleGlobal.vehicle_order_ui_state()
            && !HIBYTE(GetAsyncKeyState(VK_SHIFT))
            && !IsMultiplayerNet) {
        clock_wait((stealth && !ConsoleGlobal.vehicle_order_ui_state()) ? 100 : 5);
    }

    // 598db1: air units out of range / out of moves.
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR) {
        int stranded = base_who(dest_x, dest_y) < 0
            && !(bit_at(dest_x, dest_y) & BIT_AIRBASE);
        for (int stack_id = veh_top(veh_id); stranded && stack_id >= 0;
                stack_id = Vehs[stack_id].next_veh_id_stack) {
            if (has_abil(Vehs[stack_id].proto_id, ABL_CARRIER))
                stranded = 0;
        }
        if (static_cast<uint8_t>(faction_bit)
                & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
            if (!stranded
                    && (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range != 1
                        || !veh_moves(veh_id) || !flag_74)
                    && (!veh_moves(veh_id)
                        || ((veh_moves(veh_id) <= Rules.move_rate_roads
                             || Vehs[veh_id].terraforming_turns + 1
                                >= Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range)
                            && !zoc_veh(dest_x, dest_y, faction))))
                goto L_599121;
        } else if (!stranded)
            goto L_599121;

        // 598f9e
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range) {
            if (!(static_cast<uint8_t>(faction_bit)
                  & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_))
                    || (faction == ConsoleGlobal.faction_id_
                        && ((GameDataStateStorage.fields_.game_preferences_ & 0x10000)
                            || (Vehs[veh_id].state & 0x4200))
                        && (Vehs[veh_id].state & 0x1000200) != 0x1000200
                        && ((Vehs[veh_id].state & 0x2000200) != 0x2000200
                            || Vehs[veh_id].patrol_current_point >= Vehs[veh_id].waypoint_count)))
                action_home(veh_id, 1);
            if (!veh_moves(veh_id)) {
                if (Vehs[veh_id].terraforming_turns + 1
                        >= Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range) {
                    // 599098: out of fuel.
                    Vehs[veh_id].state |= 0x4;
                    if (spotted)
                        boom(Vehs[veh_id].x, Vehs[veh_id].y, 0x80);
                    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range == 1
                            && VehPrototypes[Vehs[veh_id].proto_id].chassis_id == CHSI_COPTER) {
                        Vehs[veh_id].dmg_incurred +=
                            VehPrototypes[Vehs[veh_id].proto_id].reactor_id * 3;
                    } else {
                        Vehs[veh_id].dmg_incurred += range(
                            proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999);
                    }
                    if (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) <= 0) {
                        kill(veh_id);
                        if (spotted && faction == ConsoleGlobal.faction_id_)
                            NetMsgPrimary.pop("PLANECRASH", 5000, 0, NULL);
                        goto done;
                    }
                    Vehs[veh_id].terraforming_turns = 0;
                    draw_tile(Vehs[veh_id].x, Vehs[veh_id].y, 2);
                    if (spotted && faction == ConsoleGlobal.faction_id_)
                        NetMsgPrimary.pop("CHOPPERFUEL", 5000, 0, NULL);
                } else {
                    // 5993c2: half range, head home.
                    if (Vehs[veh_id].terraforming_turns + 1
                                >= Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range / 2
                            && (!(static_cast<uint8_t>(faction_bit)
                                  & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_))
                                || (faction == ConsoleGlobal.faction_id_
                                    && ((GameDataStateStorage.fields_.game_preferences_ & 0x10000)
                                        || (Vehs[veh_id].state & 0x4200))
                                    && (Vehs[veh_id].state & 0x1000200) != 0x1000200
                                    && ((Vehs[veh_id].state & 0x2000200) != 0x2000200
                                        || Vehs[veh_id].patrol_current_point
                                           >= Vehs[veh_id].waypoint_count)
                                    && Vehs[veh_id].order != ORDER_MOVE_TO)))
                        action_home(veh_id, 0);
                    Vehs[veh_id].terraforming_turns++;
                }
            }
        }
        goto L_599154;
L_599121:
            Vehs[veh_id].terraforming_turns = 0;
            veh_skip(veh_id);
            veh_demote(veh_id);
            draw_tile(Vehs[veh_id].x, Vehs[veh_id].y, 2);
    }
L_599154:
    // 599154: monolith and supply pod.
    if ((bit_at(dest_x, dest_y) & BIT_MONOLITH) && triad != TRIAD_AIR
            && (direction >= 0
                || (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating
                    && (Vehs[veh_id].dmg_incurred || !(Vehs[veh_id].state & 0x100))))
            && !flag_6c)
        monolith(veh_id);
    if (goody && triad != TRIAD_AIR && faction && goody_box(veh_id, goody))
        goto done;

    // 5991f6: a lone artifact next to a stronger foreign land stack is taken.
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT
            && base_id < 0
            && stack_check(veh_id, 1, -1, -1, -1) == 1
            && direction < 8) {
        for (int i = 0; i < 8; i++) {
            int x = wrap_map_x(dest_x + VehMoveDeltaX[i]);
            int y = dest_y + VehMoveDeltaY[i];
            if (!on_map(x, y))
                continue;
            if (is_ocean(x, y))
                continue;
            int stack_id = stack_fix(veh_at(x, y));
            if (stack_id >= 0
                    && Vehs[stack_id].faction_id != faction
                    && Vehs[stack_id].faction_id
                    && !(PlayersData[faction].diplo_treaties[Vehs[stack_id].faction_id] & 1)
                    && stack_check(stack_id, 2, PLAN_ALIEN_ARTIFACT, -1, -1)
                       < stack_check(stack_id, 1, -1, -1, -1)) {
                // 599547
                Vehs[veh_id].faction_id = Vehs[stack_id].faction_id;
                Vehs[veh_id].order = ORDER_NONE;
                draw_tile(dest_x, dest_y, 2);
                owner_set(dest_x, dest_y, Vehs[veh_id].faction_id);
                if (faction == ConsoleGlobal.faction_id_) {
                    ConsoleGlobal.selected_vehicle_ = -1;
                    parse_says(0, get_noun(Vehs[stack_id].faction_id), -1, -1);
                    popp(ScriptPopupFile, "TOOKARTIFACT", 0, "artstolen_sm.pcx", NULL);
                } else if (Vehs[stack_id].faction_id == ConsoleGlobal.faction_id_) {
                    parse_says(0, VehPrototypes[Vehs[stack_id].proto_id].veh_name, -1, -1);
                    parse_says(1, get_noun(faction), -1, -1);
                    popp(ScriptPopupFile, "TAKENARTIFACT", 0, "artstolen_sm.pcx", NULL);
                }
                faction = Vehs[veh_id].faction_id;
                break;
            }
        }
    }

    // 599692: artifact inside a base (terraforming_turns = pending action).
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT
            && base_id >= 0 && Vehs[veh_id].terraforming_turns) {
        switch (Vehs[veh_id].terraforming_turns) {
        case 2:
            Bases[base_id].minerals_accumulated +=
                VehPrototypes[Vehs[veh_id].proto_id].cost * 10 / 2;
            Bases[base_id].production_id_last = Bases[base_id].queue_production_id[0];
            Bases[base_id].minerals_accumulated_2 = Bases[base_id].minerals_accumulated;
            kill(veh_id);
            TutWinBaseWindow.check_base(base_id);
            break;
        case 1:
            if (study_artifact(veh_id))
                goto done;
            break;
        }
    }

    // 599732
    if (src_ocean && !dest_ocean
            && VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_COLONIZATION)
        PlayersData[faction].unk_77[region] |= 2;

    // 59977d
    if (flag_68) {
        sleep(veh_id);
        veh_skip(veh_id);
        for (v = veh_top(veh_id); v >= 0; v = veh_next_stack_id(v)) {
            if (!veh_cargo(v)) {
                if (Vehs[v].moves_expended > speed(v, 0)) {
                    Vehs[v].order = ORDER_NONE;
                } else {
                    stack_veh(v, 0);
                    if (Vehs[veh_id].waypoint_x[0] == v) {
                        ConsoleGlobal.selected_vehicle_ = v;
                        Vehs[v].order = ORDER_NONE;
                        break;
                    }
                }
            }
        }
        draw_tile(dest_x, dest_y, 2);
    }

    // 59985a: an AI transport out of moves at sea sets a transport goal.
    if (!(static_cast<uint8_t>(1 << faction)
          & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_))
            && VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_NAVAL_TRANSPORT
            && is_ocean(dest_x, dest_y)
            && base_id < 0
            && !veh_moves(veh_id)) {
        if (zoc_any(Vehs[veh_id].x, Vehs[veh_id].y, faction)
                || (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & Vehs[veh_id].visibility)
                || Vehs[veh_id].move_to_ai_type == 0x75) {
            for (v = veh_top(veh_id); v >= 0; v = veh_next_stack_id(v)) {
                if (Vehs[v].order == ORDER_SENTRY_BOARD && Vehs[v].waypoint_x[0] == veh_id)
                    Vehs[veh_id].order = ORDER_NONE;
            }
        }
        int goal_x = dest_x;
        int goal_y = dest_y;
        if (Vehs[veh_id].order == ORDER_MOVE_TO) {
            goal_x = (Vehs[veh_id].waypoint_x[0] + dest_x) / 2;
            goal_y = (Vehs[veh_id].waypoint_y[0] + dest_y) / 2;
            if (goal_x & 1)
                goal_x--;
            if (goal_y & 1)
                goal_x++;   // sic: the image bumps x, not y
            if (!is_ocean(goal_x, goal_y)) {
                goal_x = dest_x;
                goal_y = dest_y;
            }
        }
        if (is_ocean(goal_x, goal_y))
            add_goal(faction, 6, 2, goal_x, goal_y, -1);
    }

    // 599a24: KEEPMOVING nag for the local player.
    if (faction == ConsoleGlobal.faction_id_
            && (Vehs[veh_id].order == ORDER_MOVE_TO || (Vehs[veh_id].state & 0x4200))) {
        Vehs[veh_id].unk_6++;
        if ((Vehs[veh_id].state & 0x10000) && direction == (Vehs[veh_id].facing ^ 4))
            Vehs[veh_id].unk_6 += 8;
        if (Vehs[veh_id].unk_6 >= 0x20) {
            parse_says(0, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
            if (IsMultiplayerNet || X_pop("KEEPMOVING", NULL)) {
                Vehs[veh_id].order = ORDER_NONE;
                Vehs[veh_id].state &= 0xFCFFBDFF;
            }
            Vehs[veh_id].unk_6 = 0;
        }
    }
    if (direction >= 0 && direction < 8) {
        Vehs[veh_id].facing = static_cast<int8_t>(direction);
        Vehs[veh_id].state |= 0x10000;
    } else {
        Vehs[veh_id].state &= ~0x10000;
    }
    // falls into done (L_599b1a)
done:
    if (GameDataStateStorage.fields_.vehicle_current_count_ != group_count_at_entry) {
L_599b2a:
        return 0;
    }
    if (selector == 0) {
        Vehs[veh_id].order = ORDER_NONE;
        const int locked = (1 << faction) &
            static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        if (!locked || (Vehs[veh_id].state & 0x4200) != 0) {
            if (++Vehs[veh_id].unk_6 >= 24 || locked) {
                Vehs[veh_id].unk_6 = 0;
                veh_skip(veh_id);
            }
        }
        if ((NetDaemonNet.action_mode_ & 2) == 0 && IsMultiplayerNet &&
                !ConsoleControlTurnActive) {
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
        }
    } else if ((NetDaemonNet.action_mode_ & 2) != 0) {
        if (direction >= 0 && direction < 8 && (Vehs[veh_id].state & 8) != 0 &&
                Vehs[veh_id].x == dest_x && Vehs[veh_id].y == dest_y &&
                !zoc_veh(dest_x, dest_y, faction))
            Vehs[veh_id].state &= ~8U;
        if (IsMultiplayerNet && !ConsoleControlTurnActive &&
                Vehs[veh_id].order == ORDER_MOVE_TO &&
                Vehs[veh_id].x == Vehs[veh_id].waypoint_x[0] &&
                Vehs[veh_id].y == Vehs[veh_id].waypoint_y[0]) {
            Vehs[veh_id].order = ORDER_NONE;
            if ((Vehs[veh_id].state & 0x2000200) == 0x2000200 &&
                    Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count) {
                Vehs[veh_id].patrol_current_point++;
                Vehs[veh_id].order = ORDER_MOVE_TO;
                Vehs[veh_id].waypoint_x[0] = Vehs[veh_id].waypoint_x[Vehs[veh_id].patrol_current_point];
                Vehs[veh_id].waypoint_y[0] = Vehs[veh_id].waypoint_y[Vehs[veh_id].patrol_current_point];
                Vehs[veh_id].state &= ~0x2010200U;
            }
        }
    }
    return selector;
}

// WORKAROUND(inline): supply_options' dead budget probe (see its declaration).
static int supply_options_budget_step(int x) {
    if (x == 0) x = 1;
    if (x == 1) x = 4;
    if (x == 2) x = 7;
    if (x == 3) x = 10;
    return x;
}
static int supply_options_budget_probe(int x) {
    if (supply_options_budget_step(x) == 0) return 1;
    return supply_options_budget_step(x + 1);
}
