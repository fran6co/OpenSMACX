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
// ORIGINAL: 0x00593830 ?quick_zoc@@YAXHHHHHPAH0@Z 0x00593830-0x005939FC
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
        int x_radius = wrap_map_x(x_src + VehMoveDeltaX[i]);
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


static int supply_wrap_add(int left, int right) {
    return static_cast<int>(static_cast<uint32_t>(left) +
                            static_cast<uint32_t>(right));
}
static int supply_wrap_sub(int left, int right) {
    return static_cast<int>(static_cast<uint32_t>(left) -
                            static_cast<uint32_t>(right));
}
static int supply_wrap_mul(int left, int right) {
    return static_cast<int>(static_cast<uint32_t>(left) *
                            static_cast<uint32_t>(right));
}


// ORIGINAL: 0x00593A00 ?supply_options@@YAHHH@Z 0x00593A00-0x005947BF;0x00660BF0-0x00660F47
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x593a00 hot and cold spans; private full-body native-order review and actual-source callback/lifetime controls.", "git:decce96da7981b60ac91563a8323362cdab2e250:src/unrecovered/00593a00.cpp", "source:src/basepop.h:typed SpriteBox click callback at BasePop+3160; native CodeView and producer/consumer evidence."], "reason": "Recover the complete picker/supply operation through genuine Popup, Sprite, Veh, Base and network owners; preserve full original spans and seed findings without a new matching claim."}
// TRIED: same Popup-teardown-cascade shape as PickWin/battle_plans, but this one calls Dialogs' REAL destructor (`??1Dialogs@@QAE@XZ`) rather than the vtable-patch-then-close() dance those two used - simpler here, used directly. The final "apply the action" branch (0x5942C1 onward: which of two near-identical full teardowns fires) is approximated as always taking the first (`chosen2==1`) branch - the exact value tracked through BasePop::exec's result wasn't re-derived in the time available. sim 0.34-0.59 across flag sets.
int __cdecl supply_options(int veh_id, int base_id) {
    Popup popup;
    Veh &vehicle = Vehs[veh_id];
    const int owner = vehicle.faction_id;

    if ((static_cast<uint32_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << owner)) == 0) {
        return 0;
    }
    const int order = vehicle.order;
    if (order == ORDER_ROAD_TO || order == ORDER_MAGTUBE_TO ||
        order == ORDER_MOVE || (vehicle.state & 0x4000u) != 0) {
        return 0;
    }
    if (order == ORDER_MOVE_TO) {
        int dx = abs(static_cast<int>(vehicle.x) -
                     static_cast<int>(vehicle.waypoint_x[0]));
        if ((MapIsFlat & 1) == 0 && dx > static_cast<int>(MapLongitude))
            dx = static_cast<int>(MapLongitudeBounds) - dx;
        const int dy = abs(static_cast<int>(vehicle.y) -
                           static_cast<int>(vehicle.waypoint_y[0]));
        if (((dx + dy) & ~1) > 2) {
            return 0;
        }
    }

    const int base_owner = Bases[base_id].faction_id_current;
    if (owner != base_owner &&
        (PlayersData[owner].diplo_treaties[base_owner] & 1u) == 0) {
        return 0;
    }

    parse_says(1, VehPrototypes[vehicle.proto_id].veh_name, -1, -1);
    parse_says(2, Bases[base_id].name_string, -1, -1);
    const int queued_item = Bases[base_id].queue_production_id[0];
    int queued_cost;
    if (queued_item < 0) {
        const int facility_id = -queued_item;
        const int factor = cost_factor(owner, 1, -1);
        queued_cost = supply_wrap_mul(factor, Facility[facility_id].cost);
    } else {
        const int production_cost = veh_cost(queued_item, base_id, 0);
        const int factor = cost_factor(owner, 1, -1);
        queued_cost = supply_wrap_mul(production_cost, factor);
    }
    parse_num(0, Bases[base_id].minerals_accumulated);
    parse_num(1, queued_cost);
    popup.start(PopupStartCaption, "SUPPLYUNIT", -1, 0, 0, 0);

    const bool alien = (Players[owner].rule_flags & 0x80u) != 0;
    const char *picture_name = alien ? "al_col_sm.pcx" : "colfoun_sm.pcx";
    int picture_width = 0, picture_height = 0;
    if (Buffer::get_pcx_dimensions(picture_name, &picture_width,
                                   &picture_height) == 0) {
        if (popup.owned_picture().init(const_cast<char *>(picture_name),
                                      picture_width, picture_height) == 0) {
            popup.set_caller_picture(&popup.owned_picture());
        }
    }
    if (X_text_open(ScriptPopupFile, "SUPPLYMENU") != 0) {
        return 0;
    }

    text_get();
    StringTemp[0] = '\0';
    parse_string(TextBufferGetPtr, StringTemp);
    popup.dialogs().item(StringTemp, 0);
    int item_count = 1;

    text_get();
    const int live_queue_item = Bases[base_id].queue_production_id[0];
    if (live_queue_item <= -70) {
        const int facility_id = -live_queue_item;
        const int facility_factor = cost_factor(owner, 1, -1);
        const int facility_scaled_cost = supply_wrap_mul(
            facility_factor, Facility[facility_id].cost);
        queued_cost = facility_scaled_cost;
        parse_say(0, Facility[facility_id].name, -1, -1);
        parse_string(TextBufferGetPtr, StringTemp);
        popup.dialogs().item(StringTemp, item_count);
        ++item_count;
    }

    text_get();
    const int live_production = Bases[base_id].queue_production_id[0];
    if (live_production >= 0) {
        const int production_cost = veh_cost(live_production, base_id, 0);
        const int production_factor = cost_factor(owner, 1, -1);
        queued_cost = supply_wrap_mul(production_cost, production_factor);
    }
    if (live_production >= 64 &&
        (VehPrototypes[live_production].flags & 4u) == 0) {
        parse_says(0, VehPrototypes[live_production].veh_name, -1, -1);
        parse_string(TextBufferGetPtr, StringTemp);
        popup.dialogs().item(StringTemp, 1);
        ++item_count;
    }
    if (convoy(veh_id, base_id) != 0) {
        item_count += 3;
        for (int convoy_line = 2; convoy_line <= 4; ++convoy_line) {
            text_get();
            parse_string(TextBufferGetPtr, StringTemp);
            popup.dialogs().item(StringTemp, convoy_line);
        }
    }
    if (item_count <= 1) {
        return 0;
    }

    uint32_t &supply_action_flags = NetDaemonNet.channels_[0].leading_words_[3];
    int action;
    if (owner == ConsoleGlobal.faction_id_ && (supply_action_flags & 1u) != 0) {
        const int selected_action = popup.exec(0, 0);
        if (selected_action == 0) {
            return 0;
        }
        vehicle.probe_action = static_cast<uint8_t>(selected_action);
        action = selected_action;
    } else {
        action = vehicle.probe_action;
    }

    const uint32_t action_flags = supply_action_flags;
    // Native 0x5941DC / 0x5941E5 takes the simple return only for bit0 set,
    // bit1 clear. All other combinations continue at 0x5942C1 action dispatch.
    if ((action_flags & 1u) != 0 && (action_flags & 2u) == 0) {
        return 1;
    }

    if (action == 1) {
        const int factor = cost_factor(owner, 1, -1);
        const int action_cost = supply_wrap_mul(
            factor, VehPrototypes[vehicle.proto_id].cost);
        const int new_minerals = supply_wrap_add(
            Bases[base_id].minerals_accumulated, action_cost);
        Bases[base_id].minerals_accumulated = new_minerals;
        Bases[base_id].mineral_surplus_final = new_minerals;
        Bases[base_id].production_id_last =
            Bases[base_id].queue_production_id[0];
        synch_base(base_id);

        int amount_needed = supply_wrap_sub(
            queued_cost, Bases[base_id].minerals_accumulated);
        if (amount_needed < 0) amount_needed = 0;
        else if (amount_needed > 9999) amount_needed = 9999;
        if (owner == ConsoleGlobal.faction_id_) {
            parse_num(0, amount_needed);
            NetMsgPrimary.pop("SUPPLYASSIGN", 5000, 0, 0);
        }
        if (IsMultiplayerNet != 0) {
            message_veh(0x2407, veh_id, 1, 0);
            NetDaemonNet.await_exec(1);
        } else {
            kill(veh_id);
        }
        // Native 0x5943C0 closes Popup; 0x594508 closes its picture later.
        popup.close();
        popup.owned_picture().close();
        return 1;
    }

    vehicle.order = ORDER_CONVOY;
    vehicle.probe_sabotage_id = static_cast<uint8_t>(action - 2);
    // Native 0x594592 closes Popup before 0x5946C4 closes its picture.
    popup.close();
    popup.owned_picture().close();
    return 0;
}


// Movement tutorial and terrain-delay counters persist between orders. Native
// accesses at 0x596962 and 0x5979F2 identify these four-byte windows; the
// complete original allocations and original names remain unproved.
// RECOVERY_OBJECT: v1 0x00945AF4 ?OrderVehTutorialNoticeCounter@@3HA
int OrderVehTutorialNoticeCounter = 0;
// RECOVERY_OBJECT: v1 0x00945AF8 ?VehOrderDelayCounter@@3IA
uint32_t VehOrderDelayCounter = 0;

// Vehicle movement owner helpers.
// Native movement corridors are outlined here without changing their shared reaching state.
struct OrderVehDestination {
    int x;
    int y;
    int visible_unit_id;
    int base_id;
    int controlling_faction;
    int goody_type;
    const Map *tile;
    uint8_t region;
};

static bool order_veh_direction8_stack_allows_popup(int veh_id) {
    int stack_veh_id = veh_id;
    if (stack_veh_id >= 0) {
        while (Vehs[stack_veh_id].prev_veh_id_stack >= 0) {
            stack_veh_id = Vehs[stack_veh_id].prev_veh_id_stack;
        }
    }

    while (stack_veh_id >= 0) {
        const Veh &stack_vehicle = Vehs[stack_veh_id];
        // Original bug (0x5963C7): tests the mover, not the stack member.
        if (has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)) {
            return false;
        }

        const VehPrototype &prototype = VehPrototypes[stack_vehicle.proto_id];
        if (prototype.plan == PLAN_SUPPLY_CONVOY
            && Chassis[prototype.chassis_id].triad == TRIAD_AIR) {
            return false;
        }

        stack_veh_id = stack_vehicle.next_veh_id_stack;
    }
    return true;
}

static bool order_veh_artifact_announcement(int veh_id, int base_id,
                                            int action_faction,
                                            int stored_action_mode) {
    Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    if (prototype.plan != PLAN_ALIEN_ARTIFACT) return false;
    if ((vehicle.flags & 0x20U) != 0) return false;
    if ((stored_action_mode & 1) == 0) return false;

    vehicle.terraforming_turns = 0;
    if (base_id < 0) return false;
    if (Bases[base_id].faction_id_current != action_faction) return false;

    parse_says(0, Bases[base_id].name_string, -1, -1);
    return true;
}

static void order_veh_start_artifact_popup(Popup &popup) {
    popup.start(PopupStartCaption, "ARTIFACT", -1, NULL, 0, NULL);
}

static void order_veh_artifact_picture(Popup &popup, Sprite &sprite,
                                       const char *dimensions_path,
                                       const char *sprite_path) {
    int width;
    int height;
    if (Buffer::get_pcx_dimensions(dimensions_path, &width, &height) != 0) {
        return;
    }
    if (sprite.init(const_cast<char *>(sprite_path), width, height) == 0) {
        popup.set_caller_picture(&sprite);
    }
}

static bool order_veh_artifact_dialog_item(Popup &popup, char *file_name,
                                           const char *section, int row) {
    StringTemp[0] = '\0';
    if (X_text_open(file_name, section) != 0) return false;
    LPSTR row_text = text_get();
    parse_string(row_text, StringTemp);
    popup.dialogs().item(StringTemp, row);
    return true;
}

static bool order_veh_direction8_popup_gate(int veh_id, int action_faction,
                                            int destination_x,
                                            int destination_y, int mode) {
    const Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    if (chassis.triad != TRIAD_AIR || chassis.range == 0) return false;
    if ((mode & 1) == 0 || action_faction != ConsoleGlobal.faction_id_) {
        return false;
    }

    const Map &destination = *map_loc(destination_x, destination_y);
    if ((destination.bit & 1U) != 0) {
        if ((destination.val2 & 0x0FU) < 8U) return false;
    }
    if ((destination.bit & BIT_AIRBASE) != 0) return false;

    if (!order_veh_direction8_stack_allows_popup(veh_id)) return false;

    const int remaining = speed(veh_id, 0) - vehicle.moves_expended;
    if (remaining < 0 || remaining == 0) return false;
    if (chassis.range != 1 && vehicle.terraforming_turns == 0) return false;

    action_home(veh_id, 0); // Native result is ignored; order is reread below.
    return Vehs[veh_id].order == ORDER_MOVE_TO;
}

static void order_veh_artifact_cost_numbers(int base_id, int faction_id) {
    Base &base = Bases[base_id];
    parse_num(0, base.minerals_accumulated);

    int production_cost;
    const int queue_item = base.queue_production_id[0];
    if (queue_item < 0) {
        const int facility_id = -queue_item;
        production_cost = cost_factor(faction_id, 1, -1)
                        * Facility[facility_id].cost;
    } else {
        production_cost = veh_cost(queue_item, base_id, NULL)
                        * cost_factor(faction_id, 1, -1);
    }
    parse_num(1, production_cost);
}

static bool order_veh_waypoint_treaty_clears_order(
    int veh_id, int action_faction, bool preceding_action_mask_selected) {
    if (preceding_action_mask_selected) return false;
    Veh &vehicle = Vehs[veh_id];
    if (vehicle.order != ORDER_MOVE_TO) return false;
    const int owner = whose_territory(action_faction,
                                      vehicle.waypoint_x[0],
                                      vehicle.waypoint_y[0], 0, 0);
    if (owner < 0 || owner == action_faction) return false;
    if ((PlayersData[action_faction].diplo_treaties[owner] & 0xC11U) != 0) {
        return false;
    }
    vehicle.order = ORDER_NONE;
    return true;
}

static void order_veh_artifact_dialog_rows(
    Popup &popup, int veh_id, int base_id, int action_faction) {
    Base &base = Bases[base_id];
    Veh &vehicle = Vehs[veh_id];
    int row_count = 1;

    LPSTR row_text = text_get();
    int byte_offset;
    int bit_mask;
    bitmask(8, &byte_offset, &bit_mask);
    if ((base.facilities_built[byte_offset] & bit_mask) != 0 &&
        GameDataStateStorage.fields_.secret_project_.universal_translator() != base_id &&
        (base.state & 0x800U) == 0) {
        StringTemp[0] = '\0';
        parse_string(row_text, StringTemp);
        popup.dialogs().item(StringTemp, 1);
        row_count = 2;
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << (action_faction & 0x1f))) == 0) {
            vehicle.terraforming_turns = 1;
        }
    }

    row_text = text_get();
    if (GameDataStateStorage.fields_.secret_project_.universal_translator() == base_id) {
        StringTemp[0] = '\0';
        parse_string(row_text, StringTemp);
        popup.dialogs().item(StringTemp, 1);
        ++row_count;
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << (action_faction & 0x1f))) == 0) {
            vehicle.terraforming_turns = 1;
        }
    }

    row_text = text_get();
    const int queue_item = base.queue_production_id[0];
    if (queue_item <= -0x46) {
        int facility_id = queue_item;
        if (facility_id <= -1) facility_id = -facility_id;
        StringTemp[0] = '\0';
        parse_say(1, Facility[facility_id].name, -1, -1);
        parse_string(row_text, StringTemp);
        popup.dialogs().item(StringTemp, 2);
        ++row_count;
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << (action_faction & 0x1f))) == 0) {
            vehicle.terraforming_turns = 2;
        }
    }

    row_text = text_get();
    if (queue_item >= 0x40 &&
        (VehPrototypes[queue_item].flags & 4U) == 0) {
        StringTemp[0] = '\0';
        parse_says(2, VehPrototypes[queue_item].veh_name, -1, -1);
        parse_string(row_text, StringTemp);
        popup.dialogs().item(StringTemp, 2);
        ++row_count;
    }

    if (row_count >= 2 && action_faction == ConsoleGlobal.faction_id_ &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << (action_faction & 0x1f))) != 0) {
        vehicle.terraforming_turns = static_cast<uint8_t>(popup.exec(0, NULL));
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
    }
}

enum OrderVehDisembarkDialogResult {
    OrderVehDisembarkDialogCleanup,
    OrderVehDisembarkDialogSelectedAt5962BE
};
static char OrderVehDisembarkTextFile[] = "DISEMBARKERS";

static OrderVehDisembarkDialogResult order_veh_disembark_dialog(
    Popup &popup, int veh_id, int &selected_vehicle_id) {
    Veh &vehicle = Vehs[veh_id];
    vehicle.order = ORDER_NONE;
    parse_says(0,
        VehPrototypes[vehicle.proto_id].veh_name, -1, -1);
    popup.start(PopupStartCaption, "DISEMBARK", -1, NULL, 0, NULL);

    if (X_text_open(ScriptPopupFile, OrderVehDisembarkTextFile) != 0) {
        return OrderVehDisembarkDialogCleanup;
    }

    LPSTR row_template = text_get();
    stack_veh(veh_id, 0);
    int passenger = veh_id;
    int last_moving_passenger = -1;
    int attached_passengers = 0;
    if (passenger >= 0) {
        while (Vehs[passenger].prev_veh_id_stack >= 0) {
            passenger = Vehs[passenger].prev_veh_id_stack;
        }
        do {
            Veh &stacked = Vehs[passenger];
            if (stacked.order == ORDER_SENTRY_BOARD &&
                stacked.waypoint_x[0] == veh_id) {
                ++attached_passengers;
                const int current_speed = speed(passenger, FALSE);
                const int remaining_speed =
                    current_speed - stacked.moves_expended;
                if (remaining_speed >= 0 &&
                    (remaining_speed > 999 ||
                     current_speed != stacked.moves_expended)) {
                    parse_says(1,
                        VehPrototypes[stacked.proto_id].veh_name, -1, -1);
                    StringTemp[0] = '\0';
                    parse_string(row_template, StringTemp);
                    popup.dialogs().item(StringTemp, passenger + 1);
                    last_moving_passenger = passenger;
                }
            }
            passenger = stacked.next_veh_id_stack;
        } while (passenger >= 0);
    }

    if (last_moving_passenger >= 0) {
        selected_vehicle_id = popup.exec(0, NULL) - 1;
        if (selected_vehicle_id >= 0) {
            Vehs[selected_vehicle_id].order = ORDER_NONE;
            return OrderVehDisembarkDialogSelectedAt5962BE;
        }
    } else if (attached_passengers != 0) {
        NetMsgPrimary.pop("DISEMBARKFAIL", 5000, 0, NULL);
    }
    return OrderVehDisembarkDialogCleanup;
}

enum OrderVehNoBaseInfoPath {
    OrderVehNoBaseInfoContinue595083,
    OrderVehNoBaseInfoNetworkReturnOne,
    OrderVehNoBaseInfoCountChangedCleanup
};

extern int __cdecl probe(int veh_id, int target_veh_id,
                         int target_base_id, int action);

static void order_veh_local_stack_animation(int veh_id, int direction,
                                            int original_x, int original_y,
                                            int destination_x,
                                            int destination_y, int mode) {
    ConsoleGlobal.focus(original_x, original_y, mode);
    if (IsMultiplayerNet == 0) clock_wait(0x190);
    stack_veh(veh_id, 1);
    veh_scoot(veh_id, original_x, original_y, direction, 0);
    stack_put(veh_id, original_x, original_y);
    draw_tile(destination_x, destination_y, 2);
}

static bool order_veh_direction8_probe_sync(int veh_id, int base_id,
                                            int order_mode) {
    if (probe(veh_id, base_id, -1, 1) == 0) return false;
    if (IsMultiplayerNet == 0) return false;
    if (ConsoleControlTurnActive != 0) return false;
    if ((order_mode & 1) == 0 || (order_mode & 2) != 0) return false;
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    return true;
}

static bool order_veh_zoc_branch_gate(
    const VehPrototype &prototype, const RulesChassis &chassis,
    bool source_is_lowland,
    bool destination_is_lowland, int selected_base_id) {
    if (source_is_lowland || destination_is_lowland) return false;
    if (chassis.triad != TRIAD_LAND) return false;
    if ((prototype.ability_flags & 4U) != 0) return false;
    if (prototype.plan == PLAN_INFO_WARFARE) return false;
    if (selected_base_id >= 0) return false;
    return true;
}

static bool order_veh_zoc_tutorial_notice(
    int faction_id, int native_local_gate_value, int action_mode, int veh_id,
    int source_x, int source_y, int destination_x, int destination_y,
    int &tutorial_notice_counter) {
    if (faction_id != ConsoleGlobal.faction_id_) return false;
    if (native_local_gate_value != 0 || (action_mode & 1) == 0) {
        return false;
    }

    g_FX.play(13);
    if (faction_id != ConsoleGlobal.faction_id_) return !IsMultiplayerNet;
    if (IsMultiplayerNet) return false;

    if ((GameDataStateStorage.fields_.game_preferences_ & 0x20) == 0) {
        if (PlayersData[faction_id].diff_level > 1) return true;
        ++tutorial_notice_counter;
        if ((tutorial_notice_counter & 3) != 1) return true;
    }

    const int ability_id = abil_index(4);
    parse_say(0, Ability[ability_id].name, -1, -1);

    int forward_zoc_x;
    int forward_zoc_y;
    int reverse_zoc_x;
    int reverse_zoc_y;
    quick_zoc(source_x, source_y, faction_id, destination_x, destination_y,
              &forward_zoc_x, &forward_zoc_y);
    quick_zoc(destination_x, destination_y, faction_id, source_x, source_y,
              &reverse_zoc_x, &reverse_zoc_y);
    TutorialWindow.tut_map("ZOCMSG", forward_zoc_x, forward_zoc_y, veh_id,
                           NULL, 0, reverse_zoc_x, reverse_zoc_y);
    return !IsMultiplayerNet;
}

static bool order_veh_reveal_unseen_hostiles(
    int action_mode, int origin_x, int origin_y,
    uint8_t native_visibility_mask) {
    bool spotted_any = false;
    for (int direction = 0; direction < 8; ++direction) {
        const int x = wrap_map_x(origin_x + VehMoveDeltaX[direction]);
        const int y = origin_y + VehMoveDeltaY[direction];
        if (y < 0 || y >= MapLatitudeBounds ||
            x < 0 || x >= MapLongitudeBounds) {
            continue;
        }

        const int top_vehicle = stack_fix(veh_at(x, y));
        if (top_vehicle < 0) continue;

        const uint8_t enemy_faction = Vehs[top_vehicle].faction_id;
        if (enemy_faction == action_mode) continue;
        if ((PlayersData[action_mode].diplo_treaties[enemy_faction] & 1U) != 0) {
            continue;
        }
        if ((native_visibility_mask & Vehs[top_vehicle].visibility) != 0) {
            continue;
        }

        spot_loc(x, y, action_mode);
        spotted_any = true;
    }

    if (spotted_any) draw_radius(origin_x, origin_y, 1, 2);
    return spotted_any;
}

enum OrderVehSupplyOptionsPath {
    OrderVehSupplyOptionsCommonCleanupReturnZero,
    OrderVehSupplyOptionsContinue595422,
    OrderVehSupplyOptionsSyncedReturnOne
};

static OrderVehSupplyOptionsPath order_veh_supply_options_join(
    int veh_id, int proto_plan, int selected_base_id, int action_mode) {
    if (proto_plan != PLAN_SUPPLY_CONVOY || selected_base_id < 0 ||
        IsMultiplayerNet) {
        return OrderVehSupplyOptionsContinue595422;
    }

    const int options_shown = supply_options(veh_id, selected_base_id);
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) != 0) {
        return options_shown != 0
            ? OrderVehSupplyOptionsCommonCleanupReturnZero
            : OrderVehSupplyOptionsContinue595422;
    }
    if (options_shown == 0) {
        return OrderVehSupplyOptionsCommonCleanupReturnZero;
    }

    if (IsMultiplayerNet && ConsoleControlTurnActive == 0) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
        return OrderVehSupplyOptionsSyncedReturnOne;
    }
    return OrderVehSupplyOptionsContinue595422;
}

enum OrderVeh595422Path {
    OrderVeh595422Continue59512c,
    OrderVeh595422Continue595137,
    OrderVeh595422Continue596052,
    OrderVeh595422CommonCleanup,
    OrderVeh595422SpawnedCommonCleanup
};

static OrderVeh595422Path order_veh_595422_supply_spawn(
    int veh_id, int action_mode, int action_faction,
    int destination_x, int destination_y,
    bool source_is_lowland, bool destination_is_lowland,
    int &target_vehicle, int &chance_slot) {
    const Veh &vehicle = Vehs[veh_id];

    if ((action_mode & 2) == 0 ||
        Chassis[VehPrototypes[vehicle.proto_id].chassis_id].triad == TRIAD_AIR) {
        return OrderVeh595422Continue59512c;
    }
    if (target_vehicle >= 0) return OrderVeh595422Continue595137;
    if (action_faction == 0) return OrderVeh595422Continue59512c;

    const Map &destination = MapTiles[
        MapLongitude * destination_y + (destination_x >> 1)];
    if ((destination.bit & 0x20U) == 0 ||
        (destination.climate & 0xE0U) < 0x40U ||
        (destination.bit & 0x08U) != 0 ||
        source_is_lowland != destination_is_lowland) {
        return OrderVeh595422Continue59512c;
    }

    const RulesWeapon &weapon = Weapon[VehPrototypes[vehicle.proto_id].weapon_id];
    if ((weapon.offense_rating == 0 || vehicle.order == ORDER_MOVE_TO) &&
        GameDataStateStorage.fields_.secret_project_.xenoempathy_dome() >= 0 &&
        Bases[GameDataStateStorage.fields_.secret_project_.xenoempathy_dome()].faction_id_current ==
            action_faction) {
        return OrderVeh595422Continue59512c;
    }
    if (goody_at(destination_x, destination_y) != 0 ||
        (GameDataStateStorage.fields_.game_rules_ & 0x00200000U) != 0) {
        return OrderVeh595422Continue59512c;
    }

    (void)base_find(destination_x, destination_y);
    if (BaseFindDist <= 1) return OrderVeh595422Continue59512c;

    int chance = BaseFindDist;
    if (chance < 0) chance = 0;
    if (chance > 32) chance = 32;
    chance += static_cast<int>(MapNativeLifeForms) * 10;
    if ((destination.bit & 0x00100000U) != 0) {
        chance /= 2;
        if (MapNativeLifeForms == 0) chance /= 2;
    }
    if (destination_is_lowland) {
        chance /= 2;
        if (MapNativeLifeForms == 0) chance /= 2;
    }
    if ((destination.bit & 4U) != 0) chance /= 2;
    if (vehicle.proto_id < 64 &&
        (weapon.offense_rating < 0 || vehicle.proto_id == 15)) chance /= 2;
    chance_slot = chance;

    for (int offset_index = 0; offset_index < 100; offset_index += 4) {
        int x = destination_x + RadiusOffsetX[offset_index / 4];
        if ((MapIsFlat & 1) == 0) {
            if (x >= 0) {
                if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
            } else {
                x += MapLongitudeBounds;
            }
        }
        const int y = destination_y + RadiusOffsetY[offset_index / 4];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds) {
            continue;
        }
        const Map &candidate = MapTiles[MapLongitude * y + (x >> 1)];
        if (is_sensor(x, y) != 0 ||
            ((candidate.bit & 1U) != 0 && (candidate.val2 & 0x0FU) <= 7U)) {
            if (offset_index >= 0x24) {
                chance /= 2;
            } else {
                chance = (chance + ((chance >> 31) & 3)) >> 2;
            }
        }
    }

    chance_slot = chance;
    if (chance <= rand() % 100) return OrderVeh595422Continue59512c;

    int spawned_prototype = 9;
    if (!destination_is_lowland) {
        spawned_prototype = (rand() % 5 == 0 && ExpansionEnabled) ? 15 : 8;
    }
    const int spawned_vehicle = target_vehicle =
        veh_init(spawned_prototype, 0, destination_x, destination_y);
    if (spawned_vehicle < 0) {
        return OrderVeh595422Continue596052;
    }

    Vehs[veh_id].order = ORDER_NONE;
    spot_stack(spawned_vehicle, action_faction);
    draw_tile(destination_x, destination_y, 2);
    bit_set(destination_x, destination_y, 0x00100000, 1);
    if (action_faction == ConsoleGlobal.faction_id_) {
        g_FX.play(13);
        parse_says(0, Players[action_faction].adj_name_faction, -1, -1);
        parse_says(1,
            VehPrototypes[Vehs[spawned_vehicle].proto_id].veh_name,
            -1, -1);
        NetMsgPrimary.pop(destination_is_lowland ? "ISLESPOTTED" :
                              "WORMSSPOTTED",
                          5000, 0, NULL);
    }
    return OrderVeh595422SpawnedCommonCleanup;
}

enum OrderVeh595137Path {
    OrderVeh595137CommonCleanup,
    OrderVeh595137Continue596052,
    OrderVeh595137Continue59521F
};

static OrderVeh595137Path order_veh_595137_gate(
    int action_vehicle_id, int visible_vehicle_id, int action_faction,
    int destination_controller, int destination_x, int destination_y,
    int selected_base_id, bool native_probe_attempted, int action_mode) {
    Veh &action_vehicle = Vehs[action_vehicle_id];
    Veh &visible_vehicle = Vehs[visible_vehicle_id];
    const VehPrototype &action_prototype =
        VehPrototypes[action_vehicle.proto_id];

    const bool visible_is_flagged = (visible_vehicle.flags & 0x400U) != 0;
    if (!visible_is_flagged) {
        const bool same_faction = action_faction == destination_controller;
        const bool relation_allows = !same_faction &&
            (PlayersData[action_faction]
                 .diplo_treaties[destination_controller] & 1U) != 0;
        const bool info_warfare_without_flag =
            action_prototype.plan == PLAN_INFO_WARFARE &&
            (action_vehicle.flags & 4U) != 0;
        if (same_faction || (relation_allows && !info_warfare_without_flag)) {
            return OrderVeh595137Continue596052;
        }
    }

    if (native_probe_attempted) return OrderVeh595137CommonCleanup;

    if (visible_is_flagged) {
        action_vehicle.order = ORDER_NONE;
        spot_stack(visible_vehicle_id, action_faction);
        draw_tile(destination_x, destination_y, 2);
        if (action_faction == ConsoleGlobal.faction_id_) {
            g_FX.play(13);
            parse_says(0, Players[destination_controller].adj_name_faction,
                       -1, -1);
            parse_says(1,
                       VehPrototypes[visible_vehicle.proto_id].veh_name,
                       -1, -1);
            NetMsgPrimary.pop(destination_controller == 0 ? "SPOTTED0" : "SPOTTED",
                              5000, 0, NULL);
        }
        return OrderVeh595137CommonCleanup;
    }

    const unsigned int faction_bit =
        1U << (static_cast<unsigned int>(action_faction) & 0x1fU);
    const bool visible_is_other_actionable_stack =
        (visible_vehicle.faction_id != action_faction ||
         (visible_vehicle.flags & 0x440U) == 0x440U) &&
        (visible_vehicle.visibility & faction_bit) == 0 &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0;
    if (visible_is_other_actionable_stack && selected_base_id < 0 &&
        (action_mode & 1) != 0 &&
        action_faction == ConsoleGlobal.faction_id_) {
        action_vehicle.order = ORDER_NONE;
        spot_stack(visible_vehicle_id, action_faction);
        draw_tile(destination_x, destination_y, 2);
        g_FX.play(13);
        parse_says(0, Players[destination_controller].adj_name_faction, -1, -1);
        parse_says(1, VehPrototypes[visible_vehicle.proto_id].veh_name,
                   -1, -1);
        NetMsgPrimary.pop(destination_controller == 0 ? "SPOTTED0" : "SPOTTED",
                          5000, 0, NULL);
        return OrderVeh595137CommonCleanup;
    }

    return OrderVeh595137Continue59521F;
}

enum OrderVeh59521fPath {
    OrderVeh59521fCommonCleanup,
    OrderVeh59521fContinue595798
};

static OrderVeh59521fPath order_veh_59521f_front_gate(
    int action_vehicle_id, int visible_vehicle_id, int action_faction,
    int selected_base_id, const Map &destination_tile, int action_mode) {
    Veh &vehicle = Vehs[action_vehicle_id];
    if (selected_base_id < 0 &&
        (destination_tile.bit & 0x00040000U) == 0 &&
        stack_check(visible_vehicle_id, 0x12, -1, -1, -1) != 0 &&
        has_abil(vehicle.proto_id, 0x20) == 0 &&
        stack_check(visible_vehicle_id, 6, 0x80, -1, -1) == 0) {
        if (action_faction == ConsoleGlobal.faction_id_ &&
            (action_mode & 1) != 0) {
            const int ability_id = abil_index(0x20);
            parse_say(0, Ability[ability_id].name, -1, -1);
            NetMsgPrimary.pop("FIGHTERRULE", 5000, 0, NULL);
        }
        if ((vehicle.state & 0x00004200U) != 0) veh_skip(action_vehicle_id);
        return OrderVeh59521fCommonCleanup;
    }

    return OrderVeh59521fContinue595798;
}

static bool order_veh_psibomb_notice(int, int);
static bool order_veh_amphibious_tile_notice(int, int, const Map &);
enum OrderVeh595798Path {
    OrderVeh595798CommonCleanup,
    OrderVeh595798Continue5957D7
};

static OrderVeh595798Path order_veh_595798_artillery_amphib(
    int action_vehicle_id, int action_faction,
    bool source_is_lowland, bool destination_is_lowland,
    int saved_triad, const Map &destination_tile, int action_mode) {
    Veh &vehicle = Vehs[action_vehicle_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];

    if (vehicle.proto_id != 0x0e && source_is_lowland &&
        !destination_is_lowland && saved_triad == TRIAD_SEA &&
        can_arty(vehicle.proto_id, 1) == 0 &&
        Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating != 0) {
        order_veh_psibomb_notice(action_faction, action_mode);
        return OrderVeh595798CommonCleanup;
    }

    if (source_is_lowland && saved_triad == TRIAD_LAND &&
        has_abil(vehicle.proto_id, ABL_AMPHIBIOUS) == 0 &&
        VehPrototypes[vehicle.proto_id].plan != PLAN_INFO_WARFARE) {
        order_veh_amphibious_tile_notice(
            action_faction, action_mode, destination_tile);
        return OrderVeh595798CommonCleanup;
    }

    return OrderVeh595798Continue5957D7;
}

static bool order_veh_psibomb_notice(int action_faction, int action_mode) {
    if (action_faction != ConsoleGlobal.faction_id_ || (action_mode & 1) == 0) {
        return false;
    }
    NetMsgPrimary.pop("PSIBOMB", 5000, 0, NULL);
    return true;
}

static bool order_veh_amphibious_tile_notice(int action_faction,
                                             int action_mode,
                                             const Map &tile) {
    const int ability_id = abil_index(ABL_AMPHIBIOUS);
    if (action_faction != ConsoleGlobal.faction_id_ ||
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0) {
        return false;
    }
    parse_say(0, Ability[ability_id].name, -1, -1);

    const int owner = tile.val2 & 0x0F;
    const char *label = (tile.bit & BIT_BASE_IN_TILE) != 0 && owner < 8
        ? "AMPHIBBASE" : "NOTAMPHIB";
    NetMsgPrimary.pop(label, 5000, 0, NULL);
    return true;
}

static bool order_veh_amphibbase_notice(int mode, int selected_faction,
                                       const char *notice_label) {
    if (selected_faction != ConsoleGlobal.faction_id_ || (mode & 1) == 0) {
        return false;
    }

    const int ability = abil_index(ABL_AMPHIBIOUS);
    parse_say(0, Ability[ability].name, -1, -1);
    NetMsgPrimary.pop(notice_label, 5000, 0, NULL);
    return true;
}

enum OrderVeh595083ForkPath {
    OrderVeh595083Continue59533D,
    OrderVeh595083Continue5952C6,
    OrderVeh595083Continue59512C,
    OrderVeh595083CommonCleanup
};

enum OrderVeh595083TransportPath {
    OrderVeh595083TransportContinue59533D,
    OrderVeh595083TransportCommonCleanup
};

static void order_veh_596884_full_transport_notice(
    int vehicle_faction_snapshot, int action_mode, int visible_vehicle_id);
static void order_veh_5967cf_amphibbase2_or_full_transport(int, int, int, int, int);
static void order_veh_5967cf_amphibbase2_or_full_transport(
    int veh_id, int vehicle_faction_snapshot, int action_mode, int selected_base_id,
    int visible_vehicle_id);

static OrderVeh595083TransportPath order_veh_595083_amphibious_transport_fork(
    int veh_id, int vehicle_faction_snapshot, int action_mode,
    int selected_base_id, int visible_vehicle_id) {
    if (stack_check(visible_vehicle_id, 8,
                    vehicle_faction_snapshot, -1, -1) > 0) {
        return OrderVeh595083TransportContinue59533D;
    }

    if (selected_base_id < 0) {
        order_veh_596884_full_transport_notice(
            vehicle_faction_snapshot, action_mode, visible_vehicle_id);
        return OrderVeh595083TransportCommonCleanup;
    }

    if (has_abil(Vehs[veh_id].proto_id, 8) != 0 ||
        stack_check(visible_vehicle_id, 2, PLAN_NAVAL_TRANSPORT,
                    -1, -1) != 0 ||
        stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1) != 0) {
        return OrderVeh595083TransportContinue59533D;
    }

    order_veh_5967cf_amphibbase2_or_full_transport(
        veh_id, vehicle_faction_snapshot, action_mode, selected_base_id,
        visible_vehicle_id);
    return OrderVeh595083TransportCommonCleanup;
}

static void order_veh_5967cf_amphibbase2_or_full_transport(
    int veh_id, int vehicle_faction_snapshot, int action_mode, int selected_base_id,
    int visible_vehicle_id) {
    if (selected_base_id >= 0 &&
        has_abil(Vehs[veh_id].proto_id, 8) == 0 &&
        stack_check(visible_vehicle_id, 2, PLAN_NAVAL_TRANSPORT,
                    -1, -1) == 0 &&
        stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1) == 0) {
        if (vehicle_faction_snapshot == ConsoleGlobal.faction_id_ &&
            (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0) {
            const int ability = abil_index(8);
            parse_say(0, Ability[ability].name, -1, -1);
            NetMsgPrimary.pop("AMPHIBBASE2", 5000, 0, NULL);
        }
        return;
    }

    order_veh_596884_full_transport_notice(
        vehicle_faction_snapshot, action_mode, visible_vehicle_id);
}

static void order_veh_596884_full_transport_notice(
    int vehicle_faction_snapshot, int action_mode, int visible_vehicle_id) {
    if (stack_check(visible_vehicle_id, 2, PLAN_NAVAL_TRANSPORT,
                    -1, -1) == 0 ||
        vehicle_faction_snapshot != ConsoleGlobal.faction_id_ ||
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0) {
        return;
    }

    const int transport_count =
        stack_check(visible_vehicle_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1);
    NetMsgPrimary.pop(transport_count > 1 ? "FULLTRANSPORTS" :
                                           "FULLTRANSPORT",
                      5000, 0, NULL);
}

static bool order_veh_noncombatant_notice(int action_faction, int action_mode) {
    if ((action_mode & 1) == 0) return false;
    if (action_faction != ConsoleGlobal.faction_id_) return false;
    g_FX.play(0x33);
    NetMsgPrimary.pop("NONCOMBATANT", 5000, 0, NULL);
    return true;
}

enum OrderVeh5958B8PromptPath {
    OrderVeh5958B8PromptCleanup,
    OrderVeh5958B8PromptContinue595A92,
    OrderVeh5958B8PromptContinueProbeSuccess
};

static OrderVeh5958B8PromptPath order_veh_5958b8_probe_prompt(
    int action_vehicle_id, int action_faction, int destination_controller,
    int destination_x, int destination_y, int action_mode) {
    const int destination_base_id = base_find(destination_x, destination_y);
    if (destination_base_id >= 0 &&
        Bases[destination_base_id].faction_id_current == action_faction) {
        parse_says(0,
                   Players[destination_controller].adj_name_faction,
                   -1, -1);
        parse_says(1,
                   VehPrototypes[Vehs[action_vehicle_id].proto_id].veh_name,
                   -1, -1);
        PluralityDefault = 0;
        GenderDefault = Players[action_faction].is_leader_female;
        parse_says(2, Players[action_faction].title_leader, -1, -1);
        const bool action_is_locally_enabled =
            ((1U << (action_faction & 0x1f)) & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0;
        if (!action_is_locally_enabled) {
            return OrderVeh5958B8PromptContinueProbeSuccess;
        }

        const bool is_alien =
            (Players[destination_controller].rule_flags & RFLAG_ALIEN) != 0;
        const char *const picture = is_alien
            ? "al_cap_sm.pcx" : "capture_sm.pcx";
        const char *const prompt_label = "ENEMYPROBE";
        const int accepted = popp(ScriptPopupFile, prompt_label, 0,
                                  picture, NULL);
        if (accepted == 0) return OrderVeh5958B8PromptCleanup;
        if (accepted != 1) {
            return OrderVeh5958B8PromptContinue595A92;
        }
        return OrderVeh5958B8PromptContinueProbeSuccess;
    }

    if (action_faction != ConsoleGlobal.faction_id_) {
        return OrderVeh5958B8PromptContinue595A92;
    }

    parse_says(0, Players[destination_controller].adj_name_faction, -1, -1);
    parse_says(1, VehPrototypes[Vehs[action_vehicle_id].proto_id].veh_name,
               -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[action_faction].is_leader_female;
    parse_says(2, Players[action_faction].title_leader, -1, -1);
    const bool is_alien =
        (Players[destination_controller].rule_flags & RFLAG_ALIEN) != 0;
    const char *const picture = is_alien
        ? "al_cap_sm.pcx" : "capture_sm.pcx";
    const int accepted = popp(ScriptPopupFile, "ENEMYPROBE2", 0,
                              picture, NULL);
    (void)destination_controller;
    (void)action_mode;
    return accepted == 0 ? OrderVeh5958B8PromptCleanup
                         : OrderVeh5958B8PromptContinue595A92;
}

static void order_veh_named_probe_notice(int adjective_faction,
                                         int leader_faction,
                                         const char *notice_label) {
    parse_says(0, Players[adjective_faction].adj_name_faction, -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[leader_faction].is_leader_female;
    parse_says(1, Players[leader_faction].title_leader, -1, -1);
    NetMsgPrimary.pop(notice_label, 5000, 0, NULL);
}

static bool order_veh_focus_stack_scoot(
    int action_faction_id, int destination_controller, int active_faction,
    int veh_id, int direction, int tile_x, int tile_y) {
    if (action_faction_id != active_faction &&
        destination_controller != active_faction) {
        return false;
    }

    ConsoleGlobal.focus(tile_x, tile_y, action_faction_id);
    (void)stack_veh(veh_id, 1);
    veh_scoot(veh_id, tile_x, tile_y, direction, 0);
    stack_put(veh_id, tile_x, tile_y);
    draw_tile(tile_x, tile_y, 2);
    return true;
}

static void order_veh_probe_base_resolution(
    int selected_vehicle_id, int selected_faction_id, int action_faction_id,
    int active_faction, int origin_x, int origin_y) {
    int selected_base_id = -1;
    int largest_population = -1;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        Base &base = Bases[base_id];
        if (base.faction_id_current == selected_faction_id &&
            base.population_size > largest_population) {
            largest_population = base.population_size;
            selected_base_id = base_id;
        }
    }

    if (selected_base_id >= 0) {
        Base &base = Bases[selected_base_id];
        stack_put(selected_vehicle_id, base.x, base.y);
        Vehs[selected_vehicle_id].visibility = 0;
        parse_says(2, base.name_string, -1, -1);
    }
    else if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
              (1U << (selected_faction_id & 0x1f))) == 0) {
        veh_kill(selected_vehicle_id);
    }

    draw_tile(origin_x, origin_y, 2);
    if (selected_faction_id == ConsoleGlobal.faction_id_) {
        order_veh_named_probe_notice(action_faction_id, selected_faction_id,
                                     "GOTMYPROBE");
    } else if (action_faction_id == ConsoleGlobal.faction_id_) {
        order_veh_named_probe_notice(selected_faction_id, action_faction_id,
                                     "GOTYOURPROBE");
    }
}
static void order_veh_probe_success_corridor(
    int action_vehicle_id, int action_faction_id, int destination_controller,
    int active_faction, int direction, int destination_x, int destination_y,
    int selected_vehicle_id, int source_x, int source_y) {
    (void)order_veh_focus_stack_scoot(
        action_faction_id, destination_controller, active_faction,
        action_vehicle_id, direction, source_x, source_y);
    order_veh_probe_base_resolution(
        selected_vehicle_id, destination_controller, action_faction_id,
        active_faction, destination_x, destination_y);
}

enum OrderVeh595A92Path {
    OrderVeh595A92CommonCleanup,
    OrderVeh595A92Continue595BE6
};

static int order_veh_clamp_remaining_moves(int veh_id) {
    int remaining = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
    if (remaining < 0) return 0;
    if (remaining > 999) return 999;
    return remaining;
}

static OrderVeh595A92Path order_veh_595a92_hasty_gate(
    int action_vehicle_id, int action_faction, int action_mode,
    bool native_state_4200_snapshot) {
    const Veh &vehicle = Vehs[action_vehicle_id];
    const int remaining_moves = order_veh_clamp_remaining_moves(
        action_vehicle_id);
    if (remaining_moves >= static_cast<int>(Rules.move_rate_roads)) {
        return OrderVeh595A92Continue595BE6;
    }

    const int prototype_id = vehicle.proto_id;
    if (prototype_id == 15 || prototype_id == 8) {
        return OrderVeh595A92Continue595BE6;
    }

    const bool show_hasty_prompt =
        action_faction == ConsoleGlobal.faction_id_ &&
        !native_state_4200_snapshot &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0;
    if (show_hasty_prompt) {
        parse_says(0, VehPrototypes[prototype_id].veh_name, -1, -1);
        const int second_remaining_moves =
            order_veh_clamp_remaining_moves(action_vehicle_id);
        parse_num(0, second_remaining_moves);
        parse_num(1, static_cast<int>(Rules.move_rate_roads));
        if (popp(ScriptPopupFile, "HASTY", 0, "bad_sm.pcx", NULL) != 0) {
            return OrderVeh595A92CommonCleanup;
        }
        return OrderVeh595A92Continue595BE6;
    }

    const uint8_t actor_mask = static_cast<uint8_t>(
        1U << (action_faction & 0x1f));
    if ((actor_mask & GameDataStateStorage.fields_.lock_masks_.enable_mask_) != 0) {
        return OrderVeh595A92Continue595BE6;
    }
    const int second_remaining_moves = order_veh_clamp_remaining_moves(
        action_vehicle_id);
    if (second_remaining_moves <=
        static_cast<int>(Rules.move_rate_roads) / 2) {
        return OrderVeh595A92CommonCleanup;
    }
    return OrderVeh595A92Continue595BE6;
}

enum OrderVehBattleResultPath {
    OrderVehBattleResultCleanupZero,
    OrderVehBattleResultContinue598CB9,
    OrderVehBattleResultCleanupOther
};

enum OrderVeh596052Path {
    OrderVeh596052CommonCleanup,
    OrderVeh596052Continue597345,
    OrderVeh596052Continue597350,
    OrderVeh596052Continue596163
};

enum OrderVeh597350Path {
    OrderVeh597350CommonCleanup,
    OrderVeh597350Continue597564
};

int break_treaty(int faction_id, int target_faction_id, int treaty);

enum OrderVeh597564GoodyPath {
    OrderVeh597564GoodyContinue,
    OrderVeh597564GoodyCommonCleanup
};

enum OrderVeh59763bMovementPath {
    OrderVeh59763bCommonCleanup,
    OrderVeh59763bContinue5976D6,
    OrderVeh59763bContinue597B24
};

extern uint32_t VehOrderDelayCounter;

enum OrderVeh5976d6Path {
    OrderVeh5976d6Continue597B24,
    OrderVeh5976d6CommonCleanup
};

static int order_veh_5976d6_clamp_remaining(int veh_id) {
    const int spent_before_speed = Vehs[veh_id].moves_expended;
    int remaining = speed(veh_id, 0) - spent_before_speed;
    if (remaining < 0) {
        remaining = 0;
    } else if (remaining > 999) {
        remaining = 999;
    }
    return remaining;
}

static void order_veh_5976d6_focus_stack_scoot(
    int veh_id, int source_x, int source_y, int destination_x,
    int destination_y, int direction, int action_faction) {
    ConsoleGlobal.focus(source_x, source_y, ConsoleGlobal.faction_id_);
    stack_veh(veh_id, 1);
    veh_scoot(veh_id, source_x, source_y, direction + 8, 0);
    stack_put(veh_id, source_x, source_y);

    for (int neighbor = 0; neighbor < 8; ++neighbor) {
        int x = destination_x + VehMoveDeltaX[neighbor];
        const int y = destination_y + VehMoveDeltaY[neighbor];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        if (y >= 0 && y < MapLatitudeBounds && x >= 0 &&
            x < MapLongitudeBounds) {
            spot_loc(x, y, action_faction);
        }
    }
    draw_tiles(destination_x, destination_y, 2);
}

static OrderVeh5976d6Path order_veh_5976d6_terrain_delay(
    int veh_id, int action_faction, int direction,
    int source_x, int source_y, int destination_x, int destination_y,
    int visible_vehicle_id, int hex_cost, bool native_state_4200_snapshot,
    int &native_return_value) {
    Veh &vehicle = Vehs[veh_id];

    const int current_remaining = order_veh_5976d6_clamp_remaining(veh_id);
    if (current_remaining >= static_cast<int>(Rules.move_rate_roads)) {
        const Map &destination = *map_loc(destination_x, destination_y);
        if ((destination.bit & BIT_FUNGUS) == 0 ||
            (destination.climate & 0xE0U) < 0x40U) {
            return OrderVeh5976d6Continue597B24;
        }
        const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
        if (vehicle.proto_id < 64 &&
            Weapon[prototype.weapon_id].offense_rating < 0) {
            return OrderVeh5976d6Continue597B24;
        }
        if (prototype.plan == PLAN_TERRAFORMING ||
            prototype.plan == PLAN_ALIEN_ARTIFACT) {
            return OrderVeh5976d6Continue597B24;
        }
        if (visible_vehicle_id >= 0) return OrderVeh5976d6Continue597B24;
    }
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1U) == 0 &&
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<uint8_t>(1U << (action_faction & 0x1f))) != 0) {
        return OrderVeh5976d6Continue597B24;
    }

    int chance = 0;
    if (ConsoleControlTurnActive != 0) {
        if (hex_cost > 1) chance = std::rand() % hex_cost;
    } else {
        chance = static_cast<int>(random(0, hex_cost));
    }
    const int spent_before_speed = vehicle.moves_expended;
    int remaining_after_chance = speed(veh_id, 0) - spent_before_speed;
    if (remaining_after_chance < 0) remaining_after_chance = 0;
    else if (remaining_after_chance > 999) remaining_after_chance = 999;
    const int chance_threshold = remaining_after_chance +
        ((vehicle.state & 0x8000U) != 0
             ? static_cast<int>(Rules.move_rate_roads)
             : 0);
    if (chance < chance_threshold) {
        return OrderVeh5976d6Continue597B24;
    }

    if (action_faction == ConsoleGlobal.faction_id_ && vehicle.order == ORDER_NONE &&
        (::GetAsyncKeyState(VK_SHIFT) & 0xff00) == 0 &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1U) != 0 &&
        (vehicle.state & 0x0200U) == 0 &&
        (GameDataStateStorage.fields_.game_more_preferences_ & 0x80U) == 0) {
        order_veh_5976d6_focus_stack_scoot(
            veh_id, source_x, source_y, destination_x, destination_y,
            direction, action_faction);
    }

    bool tutorial_map_was_shown = false;
    bool slow_fungus_tutorial_gate = false;
    if (action_faction == ConsoleGlobal.faction_id_) {
        const Map &tutorial_destination = *map_loc(destination_x, destination_y);
        slow_fungus_tutorial_gate =
            (tutorial_destination.bit & BIT_FUNGUS) != 0 &&
            (tutorial_destination.climate & 0xE0U) >= 0x40U &&
            vehicle.order == ORDER_NONE &&
            VehPrototypes[vehicle.proto_id].plan != PLAN_TERRAFORMING &&
            VehPrototypes[vehicle.proto_id].plan != PLAN_ALIEN_ARTIFACT &&
            (NetDaemonNet.channels_[0].leading_words_[3] & 1U) != 0;
    }
    if (slow_fungus_tutorial_gate) {
        if (tut_check(0x1000) != 0) {
            ++VehOrderDelayCounter;
            g_FX.play(9);
            TutorialWindow.tut_map("SLOWFUNGUS", destination_x,
                                   destination_y, -1, NULL, 0, -1, -1);
            tutorial_map_was_shown = true;
        } else if (!native_state_4200_snapshot) {
            ++VehOrderDelayCounter;
        }
    }
    if (!tutorial_map_was_shown &&
        action_faction == ConsoleGlobal.faction_id_ &&
        vehicle.order == ORDER_NONE && !TutorialWindow.tutorial_blocked() &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1U) != 0 &&
        !((map_loc(destination_x, destination_y)->bit & BIT_FUNGUS) != 0 &&
          (map_loc(destination_x, destination_y)->climate & 0xE0U) >= 0x40U) &&
        tut_check(0x4000) != 0) {
        TutorialWindow.tut_map("FRACTIONALMOVE", destination_x,
                               destination_y, veh_id, NULL, 0, source_x, source_y);
        tutorial_map_was_shown = true;
    }

    if (tutorial_map_was_shown) {
        vehicle.order = ORDER_NONE;
        return OrderVeh5976d6CommonCleanup;
    }
    veh_skip(veh_id);
    vehicle.state |= 0x8000U;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1U) != 0 &&
        IsMultiplayerNet != 0 &&
        ConsoleControlTurnActive == 0) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
    }
    native_return_value = 1;
    return OrderVeh5976d6CommonCleanup;
}

enum OrderVeh597b24EntryPath {
    OrderVeh597b24EntryCleanupOne,
    OrderVeh597b24EntryContinueCombat
};

static OrderVeh597b24EntryPath order_veh_597b24_entry(
    int veh_id, int vehicle_faction_snapshot, int active_faction,
    int destination_base_id, int destination_controller, int native_local_2c,
    int &native_return_value) {
    const uint32_t action_mode =
        NetDaemonNet.channels_[0].leading_words_[3];
    native_return_value = 1;
    if ((action_mode & 2) == 0) {
        return OrderVeh597b24EntryCleanupOne;
    }
    Veh &vehicle = Vehs[veh_id];
    vehicle.moves_expended = static_cast<uint8_t>(
        vehicle.moves_expended + static_cast<uint8_t>(native_local_2c));
    vehicle.state &= ~0x00008040U;

    const bool foreign_unpacted_base = destination_base_id >= 0 &&
        destination_controller != vehicle_faction_snapshot &&
        (PlayersData[vehicle_faction_snapshot]
             .diplo_treaties[destination_controller] & 1U) == 0;
    if (foreign_unpacted_base &&
        (PlayersData[destination_controller]
             .diplo_treaties[vehicle_faction_snapshot] & 7U) != 0 &&
        destination_controller == active_faction) {
        const Player &actor = Players[vehicle_faction_snapshot];
        GenderDefault = actor.is_leader_female;
        PluralityDefault = 0;
        parse_says(0, actor.title_leader, -1, -1);
        GenderDefault = actor.is_leader_female;
        PluralityDefault = 0;
        parse_says(1, actor.name_leader, -1, -1);
        GenderDefault = actor.noun_gender;
        PluralityDefault = actor.is_noun_plural;
        parse_says(2, actor.noun_faction, -1, -1);
        NetMsgPrimary.pop("SURPRISE", 5000, 0, NULL);
    }

    active_faction = ConsoleGlobal.faction_id_;
    if (foreign_unpacted_base &&
        vehicle_faction_snapshot != active_faction &&
        destination_controller != active_faction &&
        (Bases[destination_base_id].faction_id_current == active_faction ||
        (Bases[destination_base_id].visibility &
          static_cast<uint8_t>(1U << (active_faction & 0x1f))) != 0) &&
        vehicle_faction_snapshot != 0) {
        spot_stack(veh_id, active_faction);
    }
    return OrderVeh597b24EntryContinueCombat;
}

static bool order_veh_597d80_sensor_radius_scan(
    int destination_x, int destination_y, int active_faction) {
    const Map &destination = *map_loc(destination_x, destination_y);
    bool found_local_control = false;

    for (int radius_index = 0; radius_index < 0x54; radius_index += 4) {
        int x = destination_x + RadiusOffsetX[radius_index / 4];
        const int y = destination_y + RadiusOffsetY[radius_index / 4];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds) {
            continue;
        }

        const Map &neighbor = *map_loc(x, y);
        const bool destination_is_lowland =
            (destination.climate & 0xE0U) < 0x60U;
        const bool neighbor_is_lowland =
            (neighbor.climate & 0xE0U) < 0x60U;
        if (destination_is_lowland != neighbor_is_lowland ||
            destination.region == neighbor.region) {
            if (is_sensor(x, y) != 0) {
                const int owner = whose_territory(ConsoleGlobal.faction_id_, x, y, NULL, 0);
                active_faction = ConsoleGlobal.faction_id_;
                if (owner == active_faction) found_local_control = true;
            }
        } else {
            continue;
        }

        active_faction = ConsoleGlobal.faction_id_;
        bool recognized_owner = false;
        if (radius_index < 0x24) {
            recognized_owner = (neighbor.bit & 3U) != 0 &&
                               (neighbor.val2 & 0x0FU) < 8;
        } else if ((neighbor.bit & 1U) != 0) {
            recognized_owner = (neighbor.val2 & 0x0FU) < 8;
        }
        if (recognized_owner &&
            static_cast<int>(neighbor.val2 & 0x0FU) == active_faction) {
            found_local_control = true;
        }
        if (found_local_control) break;
    }
    return found_local_control;
}

enum OrderVehPrefixResult {
    ORDER_VEH_RUN_RADIUS = 0,
    ORDER_VEH_ENTER_597F31 = 1,
    ORDER_VEH_SKIP_RADIUS_TO_59814A = 2
};

static OrderVehPrefixResult order_veh_prefix_result(
    int veh_id, int vehicle_faction_snapshot, int current_faction,
    int target_x, int target_y) {
    const Veh &vehicle = Vehs[veh_id];
    const uint32_t faction_mask = 1u << current_faction;
    int short_circuit = 0;

    if (vehicle_faction_snapshot == current_faction) {
        short_circuit = 1;
    } else if (vehicle.faction_id == current_faction
               && (vehicle.flags & 0x440u) != 0x440u) {
        short_circuit = 1;
    } else if ((vehicle.visibility & faction_mask) != 0) {
        short_circuit = 1;
    } else if ((GameDataStateStorage.fields_.game_state_ & 0x80u) != 0) {
        short_circuit = 1;
    }

    if ((GameDataStateStorage.fields_.game_state_ & 0x80u) == 0
        && (PlayersData[current_faction].flags & 0x200u) == 0) {
        const int map_index = target_y * MapLongitude + (target_x >> 1);
        if ((MapTiles[map_index].visibility & faction_mask) == 0) {
            return short_circuit != 0
                       ? ORDER_VEH_ENTER_597F31
                       : ORDER_VEH_SKIP_RADIUS_TO_59814A;
        }
    }

    return short_circuit != 0 ? ORDER_VEH_ENTER_597F31
                              : ORDER_VEH_RUN_RADIUS;
}

static bool order_veh_597cc8_to_597f31(
    int veh_id, int vehicle_faction_snapshot, int current_faction,
    int target_x, int target_y) {
    const OrderVehPrefixResult prefix = order_veh_prefix_result(
        veh_id, vehicle_faction_snapshot, current_faction,
        target_x, target_y);
    if (prefix == ORDER_VEH_ENTER_597F31) return true;
    if (prefix == ORDER_VEH_SKIP_RADIUS_TO_59814A) return false;
    return order_veh_597d80_sensor_radius_scan(
        target_x, target_y, current_faction);
}

static void order_veh_combat_and_turn_prelude(
    int veh_id, int vehicle_faction_snapshot,
    int original_action_mode,
    uint8_t vehicle_faction_mask,
    const Map &destination_tile,
    bool native_local_24_nonzero, bool native_local_30_nonzero) {
    Veh &vehicle = Vehs[veh_id];
    (void)original_action_mode;

    if (Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating == 0) {
        veh_demote(veh_id);
    }

    if (vehicle_faction_snapshot == 0) {
        if ((destination_tile.bit & BIT_FUNGUS) != 0 &&
            (destination_tile.climate & 0xE0u) >= 0x40u
            && vehicle.dmg_incurred != 0) {
            --vehicle.dmg_incurred;
        }
    }

    spot_all(veh_id, 1);

    const bool local_faction =
        vehicle_faction_snapshot == ConsoleGlobal.faction_id_;
    const bool lock_disallows_sleep =
        (vehicle_faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0;
    const bool omniscient = (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
    if (!local_faction && !lock_disallows_sleep && !omniscient
        && native_local_24_nonzero && ConsoleGlobal.vehicle_order_ui_state() == 0) {
        const bool shift_down = (GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0;
        if (!shift_down && IsMultiplayerNet == 0) {
            int delay = 5;
            if (native_local_30_nonzero && ConsoleGlobal.vehicle_order_ui_state() == 0) {
                delay = 100;
            }
            clock_wait(delay);
        }
    }
}

static bool order_veh_air_departure_stack_needs_transport(
    int veh_id, const Map &tile) {
    const Veh &selected = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[selected.proto_id];
    if (Chassis[prototype.chassis_id].triad != TRIAD_AIR) return false;

    const int base_owner = tile.val2 & 0x0F;
    bool needs_transport = true;
    if ((tile.bit & BIT_BASE_IN_TILE) != 0 && base_owner >= 0 && base_owner < 8) {
        needs_transport = false;
    } else if ((tile.bit & BIT_AIRBASE) != 0) {
        needs_transport = false;
    }

    int stack_veh_id = veh_id;
    if (stack_veh_id >= 0) {
        while (Vehs[stack_veh_id].prev_veh_id_stack >= 0) {
            stack_veh_id = Vehs[stack_veh_id].prev_veh_id_stack;
        }
    }
    if (needs_transport) {
        while (stack_veh_id >= 0) {
            const Veh &member = Vehs[stack_veh_id];
            const bool carrier = has_abil(member.proto_id, ABL_CARRIER);
            if (carrier) {
                needs_transport = false;
            }
            stack_veh_id = member.next_veh_id_stack;
            if (!needs_transport) break;
        }
    }
    return needs_transport;
}

struct OrderVeh5980a8State {
    bool native_local_30;
    bool native_v42;
    bool native_local_24;
};

static OrderVeh5980a8State order_veh_597f31_to_5980a8(
    int veh_id, int vehicle_faction_snapshot, int current_faction,
    int destination_base_id, int destination_x, int destination_y,
    int source_x, int source_y, int chassis_triad, int native_hex_cost,
    bool native_local_24_nonzero) {
    const Veh &vehicle = Vehs[veh_id];
    bool native_v42 = true;

    if (vehicle_faction_snapshot != current_faction &&
        (((GameDataStateStorage.fields_.game_preferences_ & 0x04U) == 0) ||
         (PlayersData[current_faction]
              .diplo_treaties[vehicle_faction_snapshot] & 1U) != 0) &&
        (((GameDataStateStorage.fields_.game_preferences_ & 0x8000U) == 0) ||
         (PlayersData[current_faction]
              .diplo_treaties[vehicle_faction_snapshot] & 1U) == 0) &&
        (destination_base_id < 0 ||
         Bases[destination_base_id].faction_id_current ==
             vehicle_faction_snapshot ||
         (PlayersData[vehicle_faction_snapshot]
              .diplo_treaties[Bases[destination_base_id].faction_id_current]
          & 1U) != 0)) {
        native_v42 = false;
    }
    bool native_local_30 = native_v42;

    const bool shift_down =
        (GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0;
    current_faction = ConsoleGlobal.faction_id_;
    if (shift_down) {
        native_v42 = false;
        native_local_30 = false;
    }

    bool take_cursor_path = false;
    if (native_hex_cost == 0) {
        if (vehicle_faction_snapshot != current_faction ||
            (vehicle.state & 0x00004200U) != 0 || vehicle.order == 0x18) {
            take_cursor_path = true;
        }
    }

    if (take_cursor_path || shift_down || (GameDataStateStorage.fields_.game_more_preferences_ & 0x80U) != 0) {
        native_v42 = false;
        native_local_30 = false;
        ConsoleGlobal.vehicle_order_ui_state() = 1;
        if (vehicle_faction_snapshot == current_faction) {
            ConsoleGlobal.vehicle_order_ui_state() = 2;
            ConsoleGlobal.cursor_next(destination_x, destination_y);
        }
    } else if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x40U) != 0) {
        if (vehicle_faction_snapshot == current_faction) {
            if ((vehicle.state & 0x00004200U) != 0 ||
                vehicle.order == 0x18) {
                native_v42 = false;
                native_local_30 = false;
                ConsoleGlobal.vehicle_order_ui_state() = 1;
                ConsoleGlobal.vehicle_order_ui_state() = 2;
                ConsoleGlobal.cursor_next(destination_x, destination_y);
            }
        } else if (chassis_triad == TRIAD_AIR) {
            native_v42 = false;
            native_local_30 = false;
            ConsoleGlobal.vehicle_order_ui_state() = 1;
        }
    }

    if (native_local_24_nonzero &&
        (GetAsyncKeyState(VK_CONTROL) & 0xFF00) == 0 &&
        (native_local_30 || (GameDataStateStorage.fields_.game_preferences_ & 0x80000U) == 0)) {
        ConsoleGlobal.focus(source_x, source_y, vehicle_faction_snapshot);
    }

    stack_veh(veh_id, 1);
    { OrderVeh5980a8State result = {
        native_local_30, native_v42, native_local_24_nonzero}; return result; }
}

static void order_veh_5980a8_restack_and_visibility(
    int veh_id, int vehicle_faction_snapshot,
    int direction, int destination_x, int destination_y,
    int source_x, int source_y, const Map &source_tile,
    const Map &destination_tile, bool native_v42, bool native_local_24) {
    (void)destination_x;
    (void)destination_y;
    if (native_v42) {
        if (vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
            g_FX.engine_pitch(
                ((destination_tile.climate & 0xE0U) <=
                         (source_tile.climate & 0xE0U)
                     ? 0
                     : 100) + 200);
        }
        veh_scoot(veh_id, source_x, source_y, direction, 0);
        if (vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
            g_FX.engine_pitch(0);
        }
    } else if (native_local_24) {
        veh_scoot(veh_id, source_x, source_y, direction, 0x4000);
    } else {
        Vehs[veh_id].facing = static_cast<uint8_t>(direction);
    }

    if (vehicle_faction_snapshot != 0) {
        unspot_stack(veh_id);
    } else {
        Veh &vehicle = Vehs[veh_id];
        vehicle.visibility &= destination_tile.visibility;
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            const uint8_t faction_bit = static_cast<uint8_t>(1U << faction);
            if ((vehicle.visibility & faction_bit) == 0) continue;
            (void)base_find(destination_x, destination_y, faction);
            if (BaseFindDist >= 8 &&
                whose_territory(faction, destination_x, destination_y,
                                NULL, 0) != faction) {
                vehicle.visibility &= static_cast<uint8_t>(~faction_bit);
            }
        }
    }
}

static void order_veh_598240_no_base_bunker_response(
    int veh_id, int action_faction, int destination_x, int destination_y,
    int destination_base_id, const Map &destination_tile) {
    const Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    if ((destination_tile.bit & BIT_BUNKER) == 0 ||
        destination_base_id >= 0 || chassis.triad != TRIAD_LAND ||
        prototype.plan == PLAN_INFO_WARFARE) {
        return;
    }

    const int territory_faction = whose_territory(
        action_faction, destination_x, destination_y, NULL, 0);
    if (territory_faction < 0 || territory_faction == action_faction ||
        (PlayersData[action_faction]
             .diplo_treaties[territory_faction] & 1U) != 0) {
        return;
    }

    veh_skip(veh_id);
    if (action_faction == ConsoleGlobal.faction_id_) {
        NetMsgPrimary.pop("BUNKERCAPTURED", 0x1388, 0, NULL);
    }
}

static bool order_veh_5986ec_stack_put_and_capture(
    int &veh_id, int vehicle_faction_snapshot, int destination_x,
    int destination_y, int destination_base_id, int destination_base_faction) {
    stack_put(veh_id, destination_x, destination_y);
    if (destination_base_id < 0 ||
        vehicle_faction_snapshot == destination_base_faction ||
        (PlayersData[vehicle_faction_snapshot]
             .diplo_treaties[destination_base_faction] & 1U) != 0) {
        return false;
    }

    Veh &vehicle = Vehs[veh_id];
    if (ExpansionEnabled != 0 && vehicle.proto_id != 0x10 &&
        vehicle.proto_id != 0x11 && vehicle.proto_id != 0x12) {
        vehicle.dmg_incurred = 0;
    }

    GameDataStateStorage.fields_.vehicle_selection_follow_limit_ = veh_id;
    (void)act_of_aggression(vehicle_faction_snapshot,
                            destination_base_faction);
    capture_base(destination_base_id, vehicle_faction_snapshot, 0);
    veh_id = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
    return true;
}

struct OrderVehEventFacilityResult {
    int facility_id;
    int attempts;
    bool facility_valid;
    bool native_local_68;
    bool native_local_30;
};

static OrderVehEventFacilityResult order_veh_598338_pick_event_facility(
    int base_id, int destination_x, int destination_y) {
    int candidate_facility = 0;
    int attempts = 0;
    bool facility_valid = true;
    bool native_local_68 = false;
    bool native_local_30 = true;
    do {
        candidate_facility = rand() % 0x46;
        facility_valid = true;
        native_local_30 = true;
        for (int bonus = 0; bonus < Players[0].faction_bonus_count; ++bonus) {
            const int bonus_kind = Players[0].faction_bonus_id[bonus];
            if (bonus_kind != 2 &&
                (bonus_kind != 0x0C ||
                 !has_tech(Facility[Players[0].faction_bonus_val1[bonus]].preq_tech, 0))) {
                continue;
            }
            if (candidate_facility ==
                Players[0].faction_bonus_val1[bonus]) {
                facility_valid = false;
                native_local_30 = false;
                break;
            }
        }

        if (candidate_facility == 0x1A) {
            const Map *tile = map_loc(destination_x, destination_y);
            if ((tile->climate & 0xE0U) < 0x60U) {
                facility_valid = false;
                native_local_30 = false;
            }
        } else if (candidate_facility <= 1 || candidate_facility >= 0x41) {
            ++attempts;
            continue;
        }

        int byte_offset;
        int bit_mask;
        bitmask(candidate_facility, &byte_offset, &bit_mask);
        const uint8_t native_byte = Bases[base_id].facilities_built[byte_offset];
        native_local_68 =
            (native_byte & static_cast<uint8_t>(bit_mask)) != 0;
        if (native_local_68 && facility_valid) break;
        ++attempts;
    } while (attempts <= 100);
    { OrderVehEventFacilityResult result = {candidate_facility, attempts, facility_valid, native_local_68,
            native_local_30}; return result; }
}

struct OrderVehNativeLifeEventResult {
    bool player_eliminated;
    bool native_local_68;
    int native_local_30;
};

static OrderVehNativeLifeEventResult order_veh_598317_native_life_base_event(
    int &veh_id, int base_id, int destination_faction,
    int vehicle_faction_snapshot, int target_x, int target_y,
    bool native_message_flag, int native_local_30_before_event) {
    if (base_id < 0 || vehicle_faction_snapshot == destination_faction ||
        vehicle_faction_snapshot != 0) {
        { OrderVehNativeLifeEventResult result = {false, false, native_local_30_before_event}; return result; }
    }

    const OrderVehEventFacilityResult event =
        order_veh_598338_pick_event_facility(base_id, target_x, target_y);
    StringTemp[0] = 0;
    strcpy(StringTemp, "WORMSATTACK");
    parse_says(2, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
    if (event.attempts <= 100) {
        StringTemp[0] = 0;
        strcpy(StringTemp, "1");
        set_fac(event.facility_id, base_id, 0);
        parse_say(1, Facility[event.facility_id].name, -1, -1);
    }
    if (native_message_flag &&
        destination_faction == ConsoleGlobal.faction_id_) {
        parse_says(0, Bases[base_id].name_string, -1, -1);
        NetMsgPrimary.pop(StringTemp, 5000, 0, NULL);
    }

    kill(veh_id);
    int native_life_units_seen = 0;
    for (int radius_index = 0; radius_index < 100; radius_index += 4) {
        int x = target_x + RadiusOffsetX[radius_index / 4];
        if (!MapIsFlat) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        const int y = target_y + RadiusOffsetY[radius_index / 4];
        if (y < 0 || y >= MapLatitudeBounds ||
            x < 0 || x >= MapLongitudeBounds) {
            continue;
        }

        int stack_id = veh_id = stack_fix(veh_at(x, y));
        if (stack_id < 0) continue;
        if (Vehs[stack_id].faction_id != 0) continue;
        while (Vehs[stack_id].prev_veh_id_stack >= 0) {
            stack_id = Vehs[stack_id].prev_veh_id_stack;
        }
        veh_id = stack_id;
        while (stack_id >= 0) {
            const int next_stack_id = Vehs[stack_id].next_veh_id_stack;
            if (PlayersData[destination_faction].planet_ecology <= native_life_units_seen) {
                veh_kill(stack_id);
            } else {
                ++native_life_units_seen;
            }
            stack_id = veh_id = next_stack_id;
        }
    }

    if (GameDataStateStorage.fields_.turn_current_num_ >= 10) {
        Base &base = Bases[base_id];
        base.population_size = static_cast<int8_t>(
            static_cast<uint8_t>(base.population_size) - 1U);
        if (is_objective(base_id) && base.population_size <= 0) {
            base.population_size = 1;
        }
        if (base.population_size == 0) {
            base_kill(base_id);
            draw_map(1);
            (void)eliminate_player(destination_faction, 0);
            draw_map(1);
            { OrderVehNativeLifeEventResult result = {true, event.native_local_68, 100}; return result; }
        }
    }
    if (event.attempts > 100) {
        Bases[base_id].minerals_accumulated = 0;
    }
    draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
    TutWinBaseWindow.check_base(base_id);
    { OrderVehNativeLifeEventResult result = {false, event.native_local_68, 100}; return result; }
}

static int order_veh_598793_no_base_sea_artillery_scan(
    int &attacker_id, int vehicle_faction_snapshot, int source_x, int source_y,
    bool native_sea_scan_enabled) {
    if (!native_sea_scan_enabled) return 0;

    Veh &attacker = Vehs[attacker_id];
    if (Chassis[VehPrototypes[attacker.proto_id].chassis_id].triad !=
        TRIAD_SEA) {
        return 0;
    }
    int spotted_count = 0;
    int last_spotted_faction = -1;
    for (int radius_index = 0; radius_index < 100; radius_index += 4) {
        int x = source_x + RadiusOffsetX[radius_index / 4];
        if (!MapIsFlat) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        const int y = source_y + RadiusOffsetY[radius_index / 4];
        if (y < 0 || y >= MapLatitudeBounds ||
            x < 0 || x >= MapLongitudeBounds) {
            continue;
        }

        int stack_id = stack_fix(veh_at(x, y));
        while (stack_id >= 0) {
            const Veh &candidate = Vehs[stack_id];
            const int next_stack_id = candidate.next_veh_id_stack;
            const int faction = candidate.faction_id;
            const uint8_t faction_bit = static_cast<uint8_t>(1U << faction);
            if (faction != vehicle_faction_snapshot &&
                can_arty(candidate.proto_id, 1) &&
                Chassis[VehPrototypes[candidate.proto_id].chassis_id].triad ==
                    TRIAD_LAND &&
                (PlayersData[vehicle_faction_snapshot]
                     .diplo_treaties[faction] & 0x10U) != 0 &&
                ((attacker.faction_id == faction &&
                  (attacker.flags & 0x0440U) != 0x0440U) ||
                 (attacker.visibility & faction_bit) != 0)) {
                last_spotted_faction = faction;
                ++spotted_count;
                spot_stack(stack_id, vehicle_faction_snapshot);
            }
            stack_id = next_stack_id;
        }
    }

    if (spotted_count != 0) {
        attacker.moves_expended = static_cast<uint8_t>(
            attacker.moves_expended + static_cast<int8_t>(spotted_count) *
                                          static_cast<int>(Rules.move_rate_roads));
        if (vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
            parse_says(0,
                Players[last_spotted_faction].adj_name_faction, -1, -1);
            NetMsgPrimary.pop("ARTILLERY", 5000, 0, NULL);
        }
    }
    return spotted_count;
}

static void order_veh_598993_clear_carried_sentry_orders(
    int veh_id, int vehicle_faction_snapshot, int destination_base_id,
    int destination_x, int destination_y,
    uint8_t faction_mask) {
    const int native_cargo_result = veh_cargo(veh_id);
    const bool native_controller_gate_allows_clear =
        vehicle_faction_snapshot == ConsoleGlobal.faction_id_ ||
        (IsMultiplayerNet && (faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0);
    if (!native_cargo_result || destination_base_id < 0 ||
        !native_controller_gate_allows_clear || veh_id < 0) {
        return;
    }

    const Veh &mover = Vehs[veh_id];
    if (mover.order == 0x18 &&
        (mover.waypoint_x[0] != destination_x ||
         mover.waypoint_y[0] != destination_y)) {
        return;
    }

    int stack_id = veh_id;
    while (Vehs[stack_id].prev_veh_id_stack >= 0) {
        stack_id = Vehs[stack_id].prev_veh_id_stack;
    }
    while (stack_id >= 0) {
        Veh &stack_vehicle = Vehs[stack_id];
        const int next_stack_id = stack_vehicle.next_veh_id_stack;
        if (stack_vehicle.faction_id == vehicle_faction_snapshot &&
            stack_vehicle.order == ORDER_SENTRY_BOARD &&
            (stack_vehicle.waypoint_x[0] < 0 ||
             stack_vehicle.waypoint_x[0] == veh_id)) {
            stack_vehicle.order = ORDER_NONE;
        }
        stack_id = next_stack_id;
    }
}

static bool order_veh_598a84_sea_cargo_recovery(
    int veh_id, int vehicle_faction_snapshot, int target_x, int target_y,
    bool native_high_bit_enabled, bool destination_is_lowland) {
    if (vehicle_faction_snapshot != ConsoleGlobal.faction_id_ ||
        !native_high_bit_enabled) return false;
    Veh &mover = Vehs[veh_id];
    if (Chassis[VehPrototypes[mover.proto_id].chassis_id].triad != TRIAD_SEA ||
        veh_cargo(veh_id) == 0 || !destination_is_lowland) return false;
    const int mover_speed = speed(veh_id, 0);
    const int mover_spent = mover.moves_expended;
    const int mover_remaining = mover_speed - mover_spent;
    if (!(mover_remaining < 0 || (mover_remaining <= 999 && mover_remaining == 0)))
        return false;
    if (mover.order == ORDER_MOVE_TO &&
        (mover.waypoint_x[0] != target_x || mover.waypoint_y[0] != target_y))
        return false;

    const bool flat = (MapIsFlat & 1) != 0;
    bool adjacent_landing_tile = false;
    for (int direction = 0; direction < 8; ++direction) {
        int x = target_x + VehMoveDeltaX[direction];
        const int y = target_y + VehMoveDeltaY[direction];
        if (!flat) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds)
            continue;
        const Map &tile = *map_loc(x, y);
        if ((tile.climate & 0xe0U) >= 0x60U ||
            ((tile.bit & BIT_BASE_IN_TILE) != 0 && (tile.val2 & 15) < 8)) {
            adjacent_landing_tile = true;
            break;
        }
    }
    if (!adjacent_landing_tile) return false;

    int member = veh_id;
    if (member < 0) return false;
    while (Vehs[member].prev_veh_id_stack >= 0)
        member = Vehs[member].prev_veh_id_stack;
    bool cleared = false;
    while (member >= 0) {
        Veh &passenger = Vehs[member];
        if (Chassis[VehPrototypes[passenger.proto_id].chassis_id].triad == TRIAD_LAND &&
            passenger.order == ORDER_SENTRY_BOARD) {
            const int passenger_speed = speed(member, 0);
            const int passenger_remaining = passenger_speed - passenger.moves_expended;
            if (passenger_remaining >= 0 &&
                (passenger_remaining > 999 || passenger_remaining != 0)) {
                passenger.order = ORDER_NONE;
                ConsoleGlobal.selected_vehicle_ = member;
                cleared = true;
            }
        }
        member = passenger.next_veh_id_stack;
    }
    return cleared;
}

enum OrderVeh598E84Continuation {
    OrderVeh598E84ContinueTo598F9E,
    OrderVeh598E84ContinueTo599151
};

struct OrderVeh598240TailResult {
    bool player_eliminated;
    bool needs_air_transport;
    OrderVeh598E84Continuation after_range_gate;
    bool exhausted_at_59903f;
    bool vehicle_killed;
    bool continue_to_599154;
    bool native_local_68;
};

static OrderVeh598E84Continuation order_veh_598e84_range_speed_gate(
    int veh_id, int vehicle_faction_snapshot, uint8_t vehicle_faction_mask,
    bool needs_transport, bool native_battle_continuation,
    int destination_x, int destination_y);
static bool order_veh_598f9e_action_home_gate(
    int veh_id, int vehicle_faction_snapshot,
    uint8_t vehicle_faction_mask);
static bool order_veh_59903f_speed_exhausted(int veh_id);
static bool order_veh_599061_fatigue_and_damage(
    int veh_id, int vehicle_faction_snapshot,
    uint8_t vehicle_faction_mask, bool native_local_24_nonzero);

static OrderVeh598E84Continuation order_veh_598e84_range_speed_gate(
    int veh_id, int vehicle_faction_snapshot, uint8_t vehicle_faction_mask,
    bool needs_transport, bool native_battle_continuation,
    int destination_x, int destination_y) {
    Veh &vehicle = Vehs[veh_id];
    const RulesChassis &chassis =
        Chassis[VehPrototypes[vehicle.proto_id].chassis_id];

    if ((vehicle_faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0) {
        if (needs_transport) return OrderVeh598E84ContinueTo598F9E;
        vehicle.terraforming_turns = 0;
        veh_skip(veh_id);
        veh_demote(veh_id);
        draw_tile(vehicle.x, vehicle.y, 2);
        return OrderVeh598E84ContinueTo599151;
    }
    if (needs_transport) return OrderVeh598E84ContinueTo598F9E;

    if (chassis.range == 1) {
        const int initial_speed = speed(veh_id, 0);
        const int remaining = initial_speed - vehicle.moves_expended;
        if (remaining >= 0 &&
            (remaining > 999 || initial_speed != vehicle.moves_expended) &&
            native_battle_continuation) {
            return OrderVeh598E84ContinueTo598F9E;
        }
    }

    const int checked_speed = speed(veh_id, 0);
    int remaining = checked_speed - vehicle.moves_expended;
    if (remaining < 0 || (remaining <= 999 &&
                          checked_speed == vehicle.moves_expended)) {
        vehicle.terraforming_turns = 0;
        veh_skip(veh_id);
        veh_demote(veh_id);
        draw_tile(vehicle.x, vehicle.y, 2);
        return OrderVeh598E84ContinueTo599151;
    }

    remaining = speed(veh_id, 0) - vehicle.moves_expended;
    if (remaining < 0) {
        remaining = 0;
    } else if (remaining > 999) {
        remaining = 999;
    }
    if (remaining <= static_cast<int>(Rules.move_rate_roads) ||
        Chassis[VehPrototypes[vehicle.proto_id].chassis_id].range <=
            static_cast<int>(vehicle.terraforming_turns) + 1) {
        if (zoc_veh(destination_x, destination_y,
                    vehicle_faction_snapshot) == 0) {
            vehicle.terraforming_turns = 0;
            veh_skip(veh_id);
            veh_demote(veh_id);
            draw_tile(vehicle.x, vehicle.y, 2);
            return OrderVeh598E84ContinueTo599151;
        }
    }
    return OrderVeh598E84ContinueTo598F9E;
}

static bool order_veh_598f9e_action_home_gate(
    int veh_id, int vehicle_faction_snapshot,
    uint8_t vehicle_faction_mask) {
    Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    if (chassis.range == 0) return false;

    const uint32_t state = vehicle.state;
    const bool action_home_allowed =
        (vehicle_faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0 ||
        (vehicle_faction_snapshot == ConsoleGlobal.faction_id_ &&
         (((GameDataStateStorage.fields_.game_preferences_ & 0x00010000U) != 0 ||
           (state & 0x00004200U) != 0) &&
          (state & 0x01000200U) != 0x01000200U &&
          ((state & 0x02000200U) != 0x02000200U ||
           vehicle.waypoint_count <= vehicle.patrol_current_point)));
    if (action_home_allowed) action_home(veh_id, 1);
    return true;
}

static bool order_veh_59903f_speed_exhausted(int veh_id) {
    const int current_speed = speed(veh_id, 0);
    const int spent = Vehs[veh_id].moves_expended;
    const int remaining = current_speed - spent;
    return remaining < 0 ||
           (remaining <= 999 && current_speed == spent);
}

static bool order_veh_599061_fatigue_and_damage(
    int veh_id, int vehicle_faction_snapshot,
    uint8_t vehicle_faction_mask, bool native_local_24_nonzero) {
    Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    const int next_morale = static_cast<int>(vehicle.terraforming_turns) + 1;

    if (static_cast<int>(chassis.range) <= next_morale) {
        vehicle.state |= 0x00000004U;
        if (native_local_24_nonzero) {
            boom(vehicle.x, vehicle.y, 0x80);
        }
        const VehPrototype &damage_prototype = VehPrototypes[vehicle.proto_id];
        const RulesChassis &damage_chassis = Chassis[damage_prototype.chassis_id];
        int damage_limit;
        if (damage_chassis.range == 1 && damage_prototype.chassis_id == 6) {
            const uint8_t damage_step = static_cast<uint8_t>(
                static_cast<unsigned int>(damage_prototype.reactor_id) * 3U);
            vehicle.dmg_incurred = static_cast<uint8_t>(
                vehicle.dmg_incurred + damage_step);
            damage_limit = damage_prototype.plan == PLAN_ALIEN_ARTIFACT ? 1 :
                (damage_prototype.reactor_id < 1 ? 10 :
                 (damage_prototype.reactor_id > 100 ? 1000 :
                  static_cast<int>(damage_prototype.reactor_id) * 10));
        } else {
            if (damage_prototype.plan == PLAN_ALIEN_ARTIFACT) {
                damage_limit = 1;
            } else {
                int weapon_step = damage_prototype.reactor_id;
                if (weapon_step < 1) weapon_step = 1;
                if (weapon_step > 100) weapon_step = 100;
                damage_limit = weapon_step * 10;
            }
            int damage_delta = damage_limit - vehicle.dmg_incurred;
            if (damage_delta < 0) {
                damage_delta = 0;
            } else if (damage_delta > 9999) {
                damage_delta = 9999;
            }
            vehicle.dmg_incurred = static_cast<uint8_t>(
                vehicle.dmg_incurred + static_cast<uint8_t>(damage_delta));
        }

        int remaining_damage = damage_limit - vehicle.dmg_incurred;
        if (remaining_damage < 0 ||
            (remaining_damage <= 9999 && remaining_damage <= 0)) {
            kill(veh_id);
            if (native_local_24_nonzero &&
                vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop("PLANECRASH", 0x1388, 0, NULL);
            }
            return true;
        }

        vehicle.terraforming_turns = 0;
        draw_tile(vehicle.x, vehicle.y, 2);
        if (native_local_24_nonzero &&
            vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
            NetMsgPrimary.pop("CHOPPERFUEL", 0x1388, 0, NULL);
        }
        return false;
    }

    if (static_cast<int>(chassis.range) / 2 <= next_morale &&
        ((vehicle_faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0 ||
        (vehicle_faction_snapshot == ConsoleGlobal.faction_id_ &&
        (((GameDataStateStorage.fields_.game_preferences_ & 0x00010000U) != 0 ||
          (vehicle.state & 0x00004200U) != 0) &&
         (vehicle.state & 0x01000200U) != 0x01000200U &&
         ((vehicle.state & 0x02000200U) != 0x02000200U ||
          vehicle.waypoint_count <= vehicle.patrol_current_point)) &&
        vehicle.order != 0x18))) {
        action_home(veh_id, 0);
    }
    ++vehicle.terraforming_turns;
    return false;
}

static bool order_veh_monolith_goody_corridor(
    int veh_id, int direction, int action_mode, const Map &destination_tile,
    int vehicle_triad, bool native_local_6c_nonzero, int goody_type) {
    Veh &vehicle = Vehs[veh_id];
    if ((destination_tile.bit & BIT_MONOLITH) != 0 && vehicle_triad != TRIAD_AIR) {
        bool monolith_allowed = true;
        if (direction < 0) {
            const int offense =
                Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating;
            if (offense == 0 || (vehicle.dmg_incurred == 0 &&
                (vehicle.state & 0x00000100u) != 0)) {
                monolith_allowed = false;
            }
        }
        if (monolith_allowed && !native_local_6c_nonzero) {
            monolith(veh_id);
        }
    }

    if (goody_type == 0 || vehicle_triad == TRIAD_AIR || action_mode == 0) {
        return false;
    }
    return goody_box(veh_id, goody_type) != 0;
}

enum OrderVehHighlandStackClaim {
    OrderVehNoHighlandStackClaim,
    OrderVehHighlandStackClaimed
};

static OrderVehHighlandStackClaim order_veh_highland_stack_claim(
    int veh_id, int current_stack, int selected_base_id,
    int original_direction, int destination_x, int destination_y,
    int action_mode, int &selected_stack_id) {
    const Veh &vehicle = Vehs[veh_id];
    if (VehPrototypes[vehicle.proto_id].plan != PLAN_ALIEN_ARTIFACT
        || selected_base_id >= 0
        || stack_check(current_stack, 1, -1, -1, -1) != 1
        || original_direction >= 8) {
        return OrderVehNoHighlandStackClaim;
    }

    int selected_stack = -1;
    int selected_faction = -1;

    for (int direction = 0; direction < 8; ++direction) {
        const int x = wrap_map_x(destination_x + VehMoveDeltaX[direction]);
        const int y = destination_y + VehMoveDeltaY[direction];
        if (!on_map(x, y)) continue;
        if ((map_loc(x, y)->climate & 0xE0) < 0x60) continue;

        const int stack = stack_fix(veh_at(x, y));
        if (stack < 0) continue;
        const int faction = Vehs[stack].faction_id;
        if (faction == action_mode || faction == 0) continue;
        if ((PlayersData[action_mode].diplo_treaties[faction] & 1U) != 0) {
            continue;
        }

        const int artifact_units = stack_check(stack, 2,
                                               PLAN_ALIEN_ARTIFACT,
                                               -1, -1);
        const int total_units = stack_check(stack, 1, -1, -1, -1);
        if (total_units > artifact_units) {
            selected_stack = stack;
            selected_faction = faction;
            break;
        }
    }

    if (selected_stack < 0) return OrderVehNoHighlandStackClaim;
    selected_stack_id = selected_stack;

    Vehs[veh_id].faction_id = static_cast<uint8_t>(selected_faction);
    Vehs[veh_id].order = ORDER_NONE;
    draw_tile(destination_x, destination_y, 2);
    owner_set(destination_x, destination_y, Vehs[veh_id].faction_id);
    return OrderVehHighlandStackClaimed;
}

static void order_veh_highland_artifact_notice(
    int veh_id, int selected_faction, int action_mode) {
    if (action_mode == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.selected_vehicle_ = -1;
        GenderDefault = Players[selected_faction].noun_gender;
        PluralityDefault = Players[selected_faction].is_noun_plural;
        parse_says(0, Players[selected_faction].noun_faction, -1, -1);
        popp(ScriptPopupFile, "TOOKARTIFACT", 0,
             "artstolen_sm.pcx", NULL);
        return;
    }

    if (selected_faction != ConsoleGlobal.faction_id_) return;

    const int prototype_id = Vehs[veh_id].proto_id;
    parse_says(0, VehPrototypes[prototype_id].veh_name, -1, -1);
    GenderDefault = Players[action_mode].noun_gender;
    PluralityDefault = Players[action_mode].is_noun_plural;
    parse_says(1, Players[action_mode].noun_faction, -1, -1);
    popp(ScriptPopupFile, "TAKENARTIFACT", 0,
         "artstolen_sm.pcx", NULL);
}

static void order_veh_599732_mark_region_plan_flag(
    int veh_id, int vehicle_faction_snapshot, uint8_t native_target_region,
    bool source_is_lowland, bool destination_is_lowland) {
    if (!source_is_lowland || destination_is_lowland ||
        VehPrototypes[Vehs[veh_id].proto_id].plan != 8) {
        return;
    }

    PlayersData[vehicle_faction_snapshot].unk_77[native_target_region] |= 2U;
}

static void order_veh_move_to_terrain_goal(
    int veh_id, int action_mode, int destination_x, int destination_y) {
    Veh &vehicle = Vehs[veh_id];
    int goal_x = destination_x;
    int goal_y = destination_y;
    if (vehicle.order == ORDER_MOVE_TO) {
        goal_x = (destination_x + vehicle.waypoint_x[0]) / 2;
        goal_y = (destination_y + vehicle.waypoint_y[0]) / 2;
        if ((goal_x & 1) != 0) --goal_x;
        if ((goal_y & 1) != 0) ++goal_x;
        const Map &candidate = MapTiles[MapLongitude * goal_y + (goal_x >> 1)];
        if ((candidate.climate & 0xe0U) >= 0x60U) {
            goal_x = destination_x;
            goal_y = destination_y;
        }
    }
    const Map &goal = MapTiles[MapLongitude * goal_y + (goal_x >> 1)];
    if ((goal.climate & 0xe0U) < 0x60U) {
        add_goal(action_mode, 6, 2, goal_x, goal_y, -1);
    }
}

static void order_veh_facing_keepmoving_tail(
    int veh_id, int direction, int action_mode) {
    Veh &vehicle = Vehs[veh_id];

    if (action_mode == ConsoleGlobal.faction_id_
        && (vehicle.order == ORDER_MOVE_TO ||
            (vehicle.state & 0x00004200u) != 0)) {
        ++vehicle.unk_6;
        if ((vehicle.state & 0x00010000u) != 0) {
            const int reverse_facing = static_cast<int>(vehicle.facing) ^ 4;
            if (direction == reverse_facing) {
                vehicle.unk_6 = static_cast<uint8_t>(vehicle.unk_6 + 8u);
            }
        }

        if (vehicle.unk_6 >= 0x20) {
            parse_says(0, VehPrototypes[vehicle.proto_id].veh_name, -1, -1);
            if (IsMultiplayerNet != 0 || X_pop("KEEPMOVING", NULL) != 0) {
                vehicle.order = ORDER_NONE;
                vehicle.state &= 0xFCFFBDFFu;
            }
            vehicle.unk_6 = 0;
        }
    }

    if (direction >= 0 && direction < 8) {
        vehicle.facing = static_cast<int8_t>(direction);
        vehicle.state |= 0x00010000u;
    } else {
        vehicle.state &= ~0x00010000u;
    }
}

static void order_veh_59977a_selector_and_action_corridor(
    int &veh_id, int vehicle_faction_snapshot, int native_stack_candidate,
    bool native_local_68, int original_direction, int action_mode,
    int destination_base_id, int destination_x, int destination_y,
    const Map &destination_tile) {
    if (native_local_68) {
        sleep(native_stack_candidate);
        veh_skip(native_stack_candidate);

        if (native_stack_candidate >= 0) {
            int stack_member = veh_id;
            while (stack_member >= 0 &&
                   Vehs[stack_member].prev_veh_id_stack >= 0) {
                stack_member = Vehs[stack_member].prev_veh_id_stack;
            }
            for (int member = stack_member; member >= 0;
                 member = Vehs[member].next_veh_id_stack) {
                if (veh_cargo(member) == 0) {
                    const int available = speed(member, 0);
                    const int spent = Vehs[member].moves_expended;
                    if (spent > available) {
                        Vehs[member].order = ORDER_NONE;
                    } else {
                        stack_veh(member, 0);
                        if (Vehs[veh_id].waypoint_x[0] == member) {
                            ConsoleGlobal.selected_vehicle_ = member;
                            Vehs[member].order = ORDER_NONE;
                            break;
                        }
                    }
                }
            }
        }

        draw_tile(destination_x, destination_y, 2);
    }

    bool reaches_goal_scan = true;
    const uint8_t faction_lock_bit = static_cast<uint8_t>(
        1U << (static_cast<unsigned>(vehicle_faction_snapshot) & 31U));
    if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_lock_bit) != 0 ||
        VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_NAVAL_TRANSPORT ||
        (destination_tile.climate & 0xE0U) >= 0x60U ||
        destination_base_id >= 0) {
        reaches_goal_scan = false;
    } else {
        const int remaining_speed = speed(veh_id, 0) -
                                    Vehs[veh_id].moves_expended;
        if (remaining_speed > 0) {
            reaches_goal_scan = false;
        } else {
            const int zoc = zoc_any(Vehs[veh_id].x, Vehs[veh_id].y,
                                    vehicle_faction_snapshot);
            if (zoc == 0 &&
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & Vehs[veh_id].visibility) == 0 &&
                Vehs[veh_id].move_to_ai_type != 0x75U) {
                reaches_goal_scan = true;
            } else {
                const int vehicle_id = veh_id;
                int head = vehicle_id;
                while (head >= 0 && Vehs[head].prev_veh_id_stack >= 0)
                    head = Vehs[head].prev_veh_id_stack;
                for (int member = head;
                     member >= 0;
                     member = Vehs[member].next_veh_id_stack) {
                    if (Vehs[member].order == 1 &&
                        Vehs[member].waypoint_x[0] == vehicle_id) {
                        Vehs[vehicle_id].order = ORDER_NONE;
                    }
                }
            }
        }
    }

    if (reaches_goal_scan) {
        order_veh_move_to_terrain_goal(
            veh_id, action_mode, destination_x, destination_y);
    }
    order_veh_facing_keepmoving_tail(veh_id, original_direction,
                                     action_mode);
}

static void order_veh_finalize_unselected_action(
    int veh_id, int action_faction_id, int action_mode) {
    Veh &vehicle = Vehs[veh_id];
    vehicle.order = ORDER_NONE;

    const uint32_t lock_bit = 1u << (action_faction_id & 0x1f);
    const bool faction_locked =
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & lock_bit) != 0;
    if (!faction_locked || (vehicle.state & 0x00004200u) != 0) {
        vehicle.unk_6 = static_cast<uint8_t>(vehicle.unk_6 + 1u);
        if (vehicle.unk_6 >= 0x18u || faction_locked) {
            vehicle.unk_6 = 0;
            veh_skip(veh_id);
        }
    }

    if ((action_mode & 2) == 0 && IsMultiplayerNet != 0 &&
        ConsoleControlTurnActive == 0) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
    }
}

// Vehicle movement owner entry.
// The movement entry retains one automatic Popup and the original shared-state joins.
// The few reused scalar slots deliberately have no default initialization.
// ORIGINAL: 0x005947C0 ?order_veh@@YAHHHH@Z 0x005947C0-0x0059A1AB;0x00660F47-0x00661200
// symbol    ?order_veh@@YAHHHH@Z
// kind      game
// DEFERRED: largest address in this batch (6708 instructions, no jump table) and its frame confirms the SAME object graph as 0x005BB000/tech_achieved in this batch, offset-for-offset: [ebp-0x5410] BasePop,GraphicWin,Popup   [ebp-0x49B4]/[ebp-0x3E68] FlatButton (Popup's own two, gap 0xB4C = sizeof(FlatButton) each)   [ebp-0x32F8] Sprite   [ebp-0x31F8] ListBox [ebp-0x30B8] GraphicWin   [ebp-0x26A0] Dialog   [ebp-0x2378] Spot   [ebp-0x21E0] GraphicWin (a second, separate Scroll) [ebp-0x1734]/[ebp-0xBE8] FlatButton,BaseButton (that Scroll's own two buttons) - plus [eax+0x1C] CheckBox and [eax+0x8C] EditGroup/SpriteBox reached through a register, matching tech_achieved's DialogEntry-linked-list widget teardown exactly. tech_achieved's own DEFERRED note (this batch) traced that ListBox/ GraphicWin/Dialog cluster to a hand-inlined `Dialogs::Dialogs()` doing real vbtable-relative virtual-base placement - i.e. `class Dialogs : public ListBox` sharing a GraphicWin virtual base, which the flattened `Dialogs` class this file's scaffold emits (a direct member, not `: ListBox`) cannot reproduce. Whatever fraction of THIS function's Popup/Dialogs teardown block hits that same inlined constructor is blocked the same way; the rest is dispatch-heavy multiplayer/combat logic (?parse_says@@ 36x, ?pop@NetMsg@@ 28x, ?stack_check@@ 20x, ?speed@@ 18x, ?draw_tile@@ 12x, ?has_abil@@ 10x, ?veh_skip@@ 8x, ?synch_veh@@/?await_synch@NetDaemon@@ 7x, _rand 7x) with the same low-repetition/bespoke-argument risk profile flagged on 0x00561080/enemy_strategy in this batch.
// Historical scaffold dependency note (not a current blocker):
// UNBLOCKER: same as 0x005BB000 for the Dialogs cluster (direct-offset
//     approximation, not true vbtable computation); the rest needs the
//     g_00xxxxxx cross-check technique applied at this function's scale,
//     which is a multi-pass undertaking on its own given 6708
//     instructions.
int __cdecl order_veh(int veh_id, int direction, int faction) {
    int selector = 0;
    int facility_present = 0;
    int battle_continuation = 0;
    int monolith_suppressed = 0;
    bool probe_attempted = false;
    Popup popup;
    int local_24;
    int local_2c;
    int local_30;
    int selected_passenger;
    int triad;
    int visible_vehicle;
    int controller;
    int base_id;
    int goody_type;
    int region;
    int source_lowland;
    int destination_lowland;
    int faction_mask;
    int destination_x;
    int destination_y;
    bool animate;
    NetDaemonNet.channels_[0].leading_words_[3] = faction;
    if ((faction & 1) != 0) ConsoleGlobal.field_23BE4_ = 0;
    else ConsoleGlobal.vehicle_order_ui_state() = 0;
    const int group_count_at_entry = GameDataStateStorage.fields_.vehicle_current_count_;
    const int source_x = Vehs[veh_id].x;
    const int source_y = Vehs[veh_id].y;
    faction = Vehs[veh_id].faction_id;
    if (direction < 0) direction = 8;
    destination_x = wrap_map_x(source_x + VehMoveDeltaX[direction]);
    destination_y = source_y + VehMoveDeltaY[direction];
    if (!on_map(destination_x, destination_y)) goto label_599b1a;
    region = map_loc(destination_x, destination_y)->region;
    visible_vehicle = stack_fix(veh_at(destination_x, destination_y));
    base_id = base_at(destination_x, destination_y);
    if (base_id >= 0) controller = Bases[base_id].faction_id_current;
    else if (visible_vehicle >= 0) controller = Vehs[visible_vehicle].faction_id;
    else controller = -1;
    source_lowland = (map_loc(source_x, source_y)->climate & 0xe0) < 0x60;
    destination_lowland = (map_loc(destination_x, destination_y)->climate & 0xe0) < 0x60;
    goody_type = goody_at(destination_x, destination_y);
    faction_mask = 1 << (faction & 31);

label_5949b3:
    local_30 = (Vehs[veh_id].state & 0x4200) != 0;
    triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
    if (order_veh_waypoint_treaty_clears_order(
            veh_id, faction, (faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0))
        goto label_599b1a;
    if (order_veh_artifact_announcement(veh_id, base_id, faction,
            NetDaemonNet.channels_[0].leading_words_[3])) {
        order_veh_start_artifact_popup(popup);
        order_veh_artifact_picture(popup, popup.owned_picture(), "art_dis_sm.pcx", "art_dis_sm.pcx");
        order_veh_artifact_cost_numbers(base_id, faction);
        if (order_veh_artifact_dialog_item(popup, ScriptPopupFile, "ARTIFACTMENU", 0))
            order_veh_artifact_dialog_rows(popup, veh_id, base_id, faction);
    }
    if (direction >= 8) {
        if (order_veh_direction8_popup_gate(veh_id, faction, destination_x,
                destination_y, NetDaemonNet.channels_[0].leading_words_[3])) {
            if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
            }
            return 0;
        }
        selector = 1;
        if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0)
            goto label_599b1a;
        veh_skip(veh_id);
        Vehs[veh_id].state &= ~0x40U;
        goto label_598cb9;
    }
    Vehs[veh_id].state |= 4;
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_INFO_WARFARE &&
            controller >= 0 && faction != controller) {
        if ((NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0) {
            Vehs[veh_id].flags &= ~4U;
            if ((PlayersData[faction].diplo_treaties[controller] & 1) != 0 &&
                    !local_30 && faction == ConsoleGlobal.faction_id_) {
                int accepted;
                if (base_id >= 0) {
                    parse_says(0, Bases[base_id].name_string, -1, -1);
                    parse_says(1, Players[controller].adj_name_faction, -1, -1);
                    accepted = X_pop("PROBEPACT", NULL);
                } else {
                    if (!stack_check(visible_vehicle, 2, PLAN_INFO_WARFARE, -1, -1))
                        goto label_594f3c;
                    parse_says(0, Players[controller].adj_name_faction, -1, -1);
                    accepted = X_pop("PROBEPACTTEAM", NULL);
                }
                if (!accepted) goto label_595083;
                Vehs[veh_id].flags |= 4;
            }
        }
label_594f3c:
        if (base_id >= 0) {
            if (visible_vehicle >= 0 && stack_check(visible_vehicle, 2,
                    PLAN_INFO_WARFARE, -1, -1)) goto label_595083;
            if ((faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0 &&
                    (PlayersData[faction].diplo_treaties[controller] & 1) != 0 &&
                    (Vehs[veh_id].waypoint_x[0] != Bases[base_id].x ||
                     Vehs[veh_id].waypoint_y[0] != Bases[base_id].y)) goto label_595083;
            if (controller == ConsoleGlobal.faction_id_)
                order_veh_local_stack_animation(veh_id, direction, source_x,
                    source_y, destination_x, destination_y, faction);
            if (order_veh_direction8_probe_sync(veh_id, base_id,
                    NetDaemonNet.channels_[0].leading_words_[3])) return 1;
            goto label_599b1a;
        }
        if ((PlayersData[faction].diplo_treaties[controller] & 1) == 0) {
            if (visible_vehicle >= 0 && stack_check(visible_vehicle, 2,
                    PLAN_INFO_WARFARE, -1, -1)) goto label_595083;
            probe_attempted = true;
            if (probe(veh_id, visible_vehicle, -1, 1) && IsMultiplayerNet &&
                    !ConsoleControlTurnActive &&
                    (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0 &&
                    (NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
                return 1;
            }
            if (GameDataStateStorage.fields_.vehicle_current_count_ != group_count_at_entry) return 0;
            if (visible_vehicle >= 0) controller = Vehs[visible_vehicle].faction_id;
        }
    }

label_595083:
    if (triad == TRIAD_LAND && destination_lowland) {
        if (faction == controller) goto label_5952c6;
        if ((PlayersData[faction].diplo_treaties[controller] & 1) != 0)
            goto label_599b1a;
        if (base_id >= 0) {
            if (!has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)) {
                order_veh_amphibbase_notice(NetDaemonNet.channels_[0].leading_words_[3],
                    faction, "AMPHIBBASE");
                goto label_599b1a;
            }
            goto label_59533d;
        }
        if (visible_vehicle < 0 || !has_abil(Vehs[veh_id].proto_id, 0x20) ||
                !stack_check(visible_vehicle, 0x12, -1, -1, -1) ||
                stack_check(visible_vehicle, 6, 0x80, -1, -1)) goto label_599b1a;
        goto label_59512c;
    }
    goto label_59533d;

label_5952c6:
    {
        const OrderVeh595083TransportPath path =
            order_veh_595083_amphibious_transport_fork(veh_id, faction,
                NetDaemonNet.channels_[0].leading_words_[3], base_id, visible_vehicle);
        if (path == OrderVeh595083TransportCommonCleanup) goto label_599b1a;
        if (base_id < 0) facility_present = 1;
    }

label_59533d:
    if (order_veh_zoc_branch_gate(VehPrototypes[Vehs[veh_id].proto_id],
            Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id],
            source_lowland != 0, destination_lowland != 0, visible_vehicle) &&
            zoc_move(source_x, source_y, faction) &&
            zoc_move(destination_x, destination_y, faction)) {
        if (order_veh_zoc_tutorial_notice(faction, local_30,
                NetDaemonNet.channels_[0].leading_words_[3], veh_id,
                source_x, source_y, destination_x, destination_y, OrderVehTutorialNoticeCounter))
            order_veh_reveal_unseen_hostiles(faction, destination_x, destination_y,
                static_cast<uint8_t>(faction_mask));
        goto label_599b1a;
    }
    {
        const OrderVehSupplyOptionsPath supply = order_veh_supply_options_join(
            veh_id, VehPrototypes[Vehs[veh_id].proto_id].plan, base_id,
            NetDaemonNet.channels_[0].leading_words_[3]);
        if (supply == OrderVehSupplyOptionsSyncedReturnOne) return 1;
        if (supply == OrderVehSupplyOptionsCommonCleanupReturnZero) goto label_599b1a;
    }
    {
        const OrderVeh595422Path supply = order_veh_595422_supply_spawn(veh_id,
            NetDaemonNet.channels_[0].leading_words_[3], faction,
            destination_x, destination_y, source_lowland != 0,
            destination_lowland != 0, visible_vehicle, local_24);
        if (supply == OrderVeh595422CommonCleanup ||
                supply == OrderVeh595422SpawnedCommonCleanup) goto label_599b1a;
        if (supply == OrderVeh595422Continue596052) goto label_596052;
        if (supply == OrderVeh595422Continue595137) goto label_595137;
    }

label_59512c:
    if (visible_vehicle < 0) goto label_596052;

label_595137:
    local_24 = visible_vehicle * sizeof(Veh);
    {
        const OrderVeh595137Path path = order_veh_595137_gate(veh_id,
            visible_vehicle, faction, controller, destination_x, destination_y,
            base_id, probe_attempted, NetDaemonNet.channels_[0].leading_words_[3]);
        if (path == OrderVeh595137CommonCleanup) goto label_599b1a;
        if (path == OrderVeh595137Continue596052) goto label_596052;
    }
    if (order_veh_59521f_front_gate(veh_id, visible_vehicle, faction, base_id,
            *map_loc(destination_x, destination_y),
            NetDaemonNet.channels_[0].leading_words_[3]) == OrderVeh59521fCommonCleanup)
        goto label_599b1a;
    if (order_veh_595798_artillery_amphib(veh_id, faction, source_lowland != 0,
            destination_lowland != 0, triad, *map_loc(source_x, source_y),
            NetDaemonNet.channels_[0].leading_words_[3]) == OrderVeh595798CommonCleanup)
        goto label_599b1a;
    {
        const RulesWeapon &weapon = Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id];
        if (weapon.mode == 11 && stack_check(visible_vehicle, 2, PLAN_INFO_WARFARE, -1, -1))
            goto label_595a92;
        if (weapon.offense_rating == 0) {
            order_veh_noncombatant_notice(faction, NetDaemonNet.channels_[0].leading_words_[3]);
            goto label_599b1a;
        }
        if (VehPrototypes[Vehs[visible_vehicle].proto_id].plan != PLAN_INFO_WARFARE ||
                stack_check(visible_vehicle, 1, -1, -1, -1) != 1 || IsMultiplayerNet)
            goto label_595a92;
        if ((PlayersData[faction].diplo_treaties[controller] & 6) == 0 ||
                (PlayersData[faction].diplo_treaties[controller] & 1) != 0)
            goto label_595a92;
        const OrderVeh5958B8PromptPath prompt = order_veh_5958b8_probe_prompt(
            visible_vehicle, faction, controller, destination_x, destination_y,
            NetDaemonNet.channels_[0].leading_words_[3]);
        if (prompt == OrderVeh5958B8PromptCleanup) goto label_599b1a;
        if (prompt == OrderVeh5958B8PromptContinueProbeSuccess) {
            order_veh_probe_success_corridor(veh_id, faction, controller,
                ConsoleGlobal.faction_id_, direction, destination_x, destination_y,
                visible_vehicle, source_x, source_y);
            goto label_599b1a;
        }
    }

label_595a92:
    if (order_veh_595a92_hasty_gate(veh_id, faction,
            NetDaemonNet.channels_[0].leading_words_[3], local_30 != 0) ==
            OrderVeh595A92CommonCleanup) goto label_599b1a;
    goto label_595be6;

// In-function fragment. Inputs and joins are documented in contracts.md.
// The enclosing owner supplies the sole Popup and native automatic lifetime.
label_595be6:
    GameDataStateStorage.fields_.vehicle_selection_follow_limit_ = veh_id;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0)
        goto label_59729d;
    if (controller != 0) goto label_59724d;
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == 2)
        goto label_59724d;
    if (Vehs[visible_vehicle].proto_id == 7) goto label_59724d;
    if (Vehs[visible_vehicle].proto_id == 19) goto label_59724d;
    local_2c = faction * static_cast<int>(sizeof(PlayerData));
    if (PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET] <= 0)
        goto label_59724d;
    {
        int remaining = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
        if (remaining < 0) remaining = 0;
        else if (remaining > 999) remaining = 999;
        if (remaining < static_cast<int>(Rules.move_rate_roads)) {
            if (PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET] <= 0)
                goto label_59724d;
            if (PlayersData[faction].proto_id_active[Vehs[visible_vehicle].proto_id] != 0)
                goto label_59724d;
        }
    }
    {
        const int nearby_base = base_find(destination_x, destination_y, faction);
        if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0)
            goto label_59724d;
        if (Vehs[visible_vehicle].order_auto_type == faction)
            goto label_597216;
        if (nearby_base >= 0 && Bases[nearby_base].eco_damage != 0)
            goto label_597216;
    }
    log_say("Precapture who, enemy", faction, visible_vehicle, 0);
    log_say("Precapture fac, proto", PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET],
            PlayersData[faction].proto_id_active[Vehs[visible_vehicle].proto_id], 0);
    log_say("Precapture type, soc2", Vehs[visible_vehicle].proto_id,
            PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET], 0);
    if ((Vehs[visible_vehicle].flags & 0x2000) != 0)
        goto label_5971ce;
    if (rand() % 4 < PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET]) {
        const int bound = GameDataStateStorage.fields_.turn_current_num_ / 5 + 100;
        int roll;
        if (bound - 1 > 0) roll = rand() % bound;
        else roll = 0;
        if (roll >= PlayersData[faction].proto_id_active[Vehs[visible_vehicle].proto_id] * 10)
            goto label_595e1b;
        if (rand() % 10 == 0) goto label_595e1b;
    }
    if (PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET] <= 0)
        goto label_5971ce;
    if (PlayersData[faction].proto_id_active[Vehs[visible_vehicle].proto_id] != 0)
        goto label_5971ce;

label_595e1b:
    {
        int captured_vehicle = visible_vehicle;
        if (captured_vehicle >= 0) {
            int previous = Vehs[visible_vehicle].prev_veh_id_stack;
            while (previous >= 0) {
                captured_vehicle = previous;
                previous = Vehs[captured_vehicle].prev_veh_id_stack;
            }
            if (captured_vehicle >= 0) {
                do {
                    Veh &captured = Vehs[captured_vehicle];
                    captured.faction_id = static_cast<unsigned char>(faction);
                    captured.morale = static_cast<unsigned char>(morale_alien(captured_vehicle, 0));
                    const int nearest = base_find(destination_x, destination_y);
                    // Original indexes Bases even when nearest is negative.
                    if (Bases[nearest].faction_id_current != faction ||
                        Bases[nearest].population_size < 3)
                        captured.home_base_id = -1;
                    else
                        captured.home_base_id = static_cast<short>(nearest);
                    veh_skip(captured_vehicle);
                    if (captured_vehicle < 0) break;
                    captured_vehicle = captured.next_veh_id_stack;
                } while (captured_vehicle >= 0);
            }
        }
    }
    bit_set(destination_x, destination_y, 0x100000, 1);
    owner_set(destination_x, destination_y, faction);
    if (faction == ConsoleGlobal.faction_id_)
        draw_tile_fixup(destination_x, destination_y, 1, 2);
    else
        draw_tile(destination_x, destination_y, 2);
    if (faction == ConsoleGlobal.faction_id_) {
        parse_says(0, VehPrototypes[Vehs[visible_vehicle].proto_id].veh_name, -1, -1);
        const int captured_proto = Vehs[visible_vehicle].proto_id;
        if (captured_proto == 15) {
            if (PlayersData[faction].proto_id_active[15] != 0)
                NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, 0);
            else
                popp(ScriptPopupFile, "WORMCAPTURE", 0, "sporlnch_sm.pcx", 0);
        } else if (captured_proto == 14) {
            if (PlayersData[faction].proto_id_active[14] != 0)
                NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, 0);
            else
                popp(ScriptPopupFile, "WORMCAPTURE", 0, "sealrk_sm.pcx", 0);
        } else {
            if (PlayersData[faction].proto_id_active[captured_proto] != 0)
                NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, 0);
            else {
                const char *picture = "isle_sm.pcx";
                if ((map_loc(destination_x, destination_y)->climate & 0xe0) >= 0x60)
                    picture = "mindworm_sm.pcx";
                popp(ScriptPopupFile, "WORMCAPTURE", 0, picture, 0);
            }
        }
    }
    log_say("Captured", 0, 0, 0);
    goto label_596052;

label_5971ce:
    Vehs[visible_vehicle].flags |= 0x2000;
    if (faction == ConsoleGlobal.faction_id_)
        NetMsgPrimary.pop("WORMNOCAPTURE", 5000, 0, 0);
    log_say("No Capture", 0, 0, 0);
    goto label_59724d;

label_597216:
    log_say("Agitated", 0, 0, 0);
    if (faction == ConsoleGlobal.faction_id_)
        NetMsgPrimary.pop("WORMSAGITATED", 5000, 0, 0);

label_59724d:
    monolith_suppressed = 1;
    {
        const int battle_result = battle_fight(veh_id, direction, 0, 1, 0);
        if (battle_result == 0) goto label_599b1a;
        if (battle_result != 1) {
            selector = 0;
            goto label_599b1a;
        }
        veh_id = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
        destination_x = source_x;
        destination_y = source_y;
        battle_continuation = 1;
        goody_type = 0;
        goto label_598cb9;
    }

label_59729d:
    selector = battle_fight(veh_id, direction, 0, 1, 0) == 0;
    goto label_599b1a;

label_596052:
    if (destination_lowland) goto label_597345;
    if (source_lowland && triad == TRIAD_LAND && visible_vehicle < 0 &&
            !has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS) &&
            (map_loc(source_x, source_y)->bit & BIT_BASE_IN_TILE) != 0 &&
            (map_loc(source_x, source_y)->val2 & 15) < 8 &&
            !stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1)) {
        order_veh_amphibbase_notice(NetDaemonNet.channels_[0].leading_words_[3],
            faction, "AMPHIBBASE2");
        goto label_599b1a;
    }
    if (base_id >= 0) goto label_597350;
    if (triad != TRIAD_SEA) goto label_597345;
    if (IsMultiplayerNet || faction != ConsoleGlobal.faction_id_ ||
            (NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0)
        goto label_599b1a;
    if (order_veh_disembark_dialog(popup, veh_id, selected_passenger) !=
            OrderVehDisembarkDialogSelectedAt5962BE) goto label_599b1a;
    veh_id = selected_passenger;
    goto label_5949b3;

label_597345:
    if (base_id < 0) goto label_597564;
// Additional owner-requested raw-asm audit:597350..5976D6.
label_597350:
    if (controller == faction) goto label_597564;
    if ((PlayersData[faction].diplo_treaties[controller] & 1) != 0)
        goto label_597564;
    if (triad == 2) {
        const RulesChassis &chassis = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id];
        if (chassis.range != 0) {
            if (faction == ConsoleGlobal.faction_id_ &&
                (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0) {
                parse_say(0, chassis.offsv_1_name, -1, -1);
                NetMsgPrimary.pop("AIRCONQUER", 5000, 0, 0);
            }
            goto label_599b1a;
        }
    } else if (triad == 1) {
        if (!destination_lowland) {
            if (faction == ConsoleGlobal.faction_id_ &&
                (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0)
                NetMsgPrimary.pop("SEACONQUER", 5000, 0, 0);
            goto label_599b1a;
        }
    } else if (triad == 0 && destination_lowland) {
        if (has_abil(Vehs[veh_id].proto_id, 8) == 0) {
            if (faction == ConsoleGlobal.faction_id_ &&
                (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0)
                NetMsgPrimary.pop("LANDCONQUER", 5000, 0, 0);
            goto label_599b1a;
        }
    }
    if (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating == 0) {
        if (faction == ConsoleGlobal.faction_id_ &&
            (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0)
            NetMsgPrimary.pop("COMBATCONQUER", 5000, 0, 0);
        goto label_599b1a;
    }
    if ((PlayersData[faction].diplo_treaties[controller] & 7) == 0)
        goto label_597564;
    if ((static_cast<unsigned char>(faction_mask) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0) {
        Vehs[veh_id].order = ORDER_NONE;
        goto label_599b1a;
    }
    if (faction != ConsoleGlobal.faction_id_) goto label_59763b;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0)
        goto label_597564;
    if (break_treaty(faction, controller, 7) != 0) goto label_599b1a;

label_597564:
    if (faction != ConsoleGlobal.faction_id_) goto label_59763b;
    if ((static_cast<unsigned char>(faction_mask) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0)
        goto label_59763b;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0)
        goto label_59763b;
    if (Vehs[veh_id].order != ORDER_MOVE_TO) goto label_59763b;
    if (Vehs[veh_id].waypoint_x[0] == destination_x &&
        Vehs[veh_id].waypoint_y[0] == destination_y)
        goto label_59763b;
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == 2)
        goto label_59763b;
    if ((Vehs[veh_id].state & 0x4200) != 0) goto label_59763b;
    if (goody_at(destination_x, destination_y) == 0) goto label_59763b;
    if (popp(ScriptPopupFile, "GOTOGOODY", 0, "supply_sm.pcx", 0) != 0) {
        Vehs[veh_id].order = ORDER_NONE;
        goto label_599b1a;
    }

label_59763b:
    {
        const int initial_remaining = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
        if (initial_remaining < 0) goto label_599b1a;
        if (initial_remaining <= 999 && initial_remaining == 0)
            goto label_599b1a;
        const int movement_cost = hex_cost(Vehs[veh_id].proto_id, faction,
                                          source_x, source_y,
                                          destination_x, destination_y, 0);
        // Native loads this byte before its second speed call.
        const int expended_before_speed = Vehs[veh_id].moves_expended;
        local_2c = movement_cost;
        int remaining = speed(veh_id, 0) - expended_before_speed;
        if (remaining < 0) remaining = 0;
        else if (remaining > 999) remaining = 999;
        if (local_2c <= remaining) goto label_597b24;
    }
    goto label_5976d6;
label_5976d6:
    if (triad == TRIAD_LAND &&
            order_veh_5976d6_terrain_delay(veh_id, faction, direction,
                source_x, source_y, destination_x, destination_y, visible_vehicle,
                local_2c, local_30 != 0, selector) == OrderVeh5976d6CommonCleanup)
        goto label_599b1a;
label_597b24:
    if (order_veh_597b24_entry(veh_id, faction, ConsoleGlobal.faction_id_, base_id,
            controller, local_2c, selector) == OrderVeh597b24EntryCleanupOne)
        goto label_599b1a;
    if (order_veh_597cc8_to_597f31(veh_id, faction, ConsoleGlobal.faction_id_,
            destination_x, destination_y)) {
        local_24 = 1;
        const OrderVeh5980a8State state = order_veh_597f31_to_5980a8(veh_id,
            faction, ConsoleGlobal.faction_id_, base_id, destination_x,
            destination_y, source_x, source_y, triad, local_2c, true);
        local_30 = state.native_local_30;
        animate = state.native_v42;
    } else {
        local_24 = 0;
        local_30 = 0;
        animate = false;
        stack_veh(veh_id, 1);
    }
    order_veh_5980a8_restack_and_visibility(veh_id, faction, direction,
        destination_x, destination_y, source_x, source_y,
        *map_loc(source_x, source_y), *map_loc(destination_x, destination_y),
        animate, local_24 != 0);
    order_veh_598240_no_base_bunker_response(veh_id, faction, destination_x,
        destination_y, base_id, *map_loc(destination_x, destination_y));
    if (base_id >= 0 && faction != controller && faction == 0) {
        const OrderVehNativeLifeEventResult event = order_veh_598317_native_life_base_event(
            veh_id, base_id, controller, faction, destination_x, destination_y,
            local_24 != 0, local_30);
        facility_present = event.native_local_68;
        local_30 = event.native_local_30;
        goto label_599b1a;
    }
    order_veh_5986ec_stack_put_and_capture(veh_id, faction, destination_x,
        destination_y, base_id, controller);
    if (base_id < 0)
        order_veh_598793_no_base_sea_artillery_scan(veh_id, faction,
            destination_x, destination_y, destination_lowland != 0);
    order_veh_598993_clear_carried_sentry_orders(veh_id, faction, base_id,
        destination_x, destination_y, static_cast<uint8_t>(faction_mask));
    order_veh_598a84_sea_cargo_recovery(veh_id, faction, destination_x,
        destination_y, (GameDataStateStorage.fields_.game_preferences_ & 0x80000000U) != 0, destination_lowland != 0);
    if (triad == TRIAD_LAND && source_lowland && !destination_lowland)
        ++PlayersData[faction].region_total_combat_vehs[region];

label_598cb9:
    order_veh_combat_and_turn_prelude(veh_id, faction,
        NetDaemonNet.channels_[0].leading_words_[3],
        static_cast<uint8_t>(faction_mask), *map_loc(destination_x, destination_y),
        local_24 != 0, local_30 != 0);
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR) {
        const bool needs_transport = order_veh_air_departure_stack_needs_transport(
            veh_id, *map_loc(destination_x, destination_y));
        if (order_veh_598e84_range_speed_gate(veh_id, faction,
                static_cast<uint8_t>(faction_mask), needs_transport,
                battle_continuation != 0, destination_x, destination_y) ==
                OrderVeh598E84ContinueTo598F9E) {
            if (order_veh_598f9e_action_home_gate(veh_id, faction,
                    static_cast<uint8_t>(faction_mask)) &&
                    order_veh_59903f_speed_exhausted(veh_id) &&
                    order_veh_599061_fatigue_and_damage(veh_id, faction,
                        static_cast<uint8_t>(faction_mask), local_24 != 0))
                goto label_599b1a;
        }
    }
    if (order_veh_monolith_goody_corridor(veh_id, direction, faction,
            *map_loc(destination_x, destination_y), triad,
            monolith_suppressed != 0, goody_type)) goto label_599b1a;
    {
        int selected_artifact_stack;
        const OrderVehHighlandStackClaim claim = order_veh_highland_stack_claim(
            veh_id, veh_id, base_id, direction, destination_x, destination_y, faction,
            selected_artifact_stack);
        if (claim != OrderVehNoHighlandStackClaim) {
            order_veh_highland_artifact_notice(selected_artifact_stack,
                Vehs[selected_artifact_stack].faction_id, faction);
            faction = Vehs[veh_id].faction_id;
        }
    }
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT &&
            base_id >= 0 && Vehs[veh_id].terraforming_turns != 0) {
        if (Vehs[veh_id].terraforming_turns == 1) {
            if (study_artifact(veh_id)) goto label_599b1a;
        } else if (Vehs[veh_id].terraforming_turns == 2) {
            Base &base = Bases[base_id];
            base.minerals_accumulated += VehPrototypes[Vehs[veh_id].proto_id].cost * 10 / 2;
            base.minerals_accumulated_2 = base.minerals_accumulated;
            base.production_id_last = base.queue_production_id[0];
            kill(veh_id);
            TutWinBaseWindow.check_base(base_id);
        }
    }
    order_veh_599732_mark_region_plan_flag(veh_id, faction, static_cast<uint8_t>(region),
        source_lowland != 0, destination_lowland != 0);
    order_veh_59977a_selector_and_action_corridor(veh_id, faction, veh_id,
        facility_present != 0, direction, faction, base_id, destination_x,
        destination_y, *map_loc(destination_x, destination_y));

label_599b1a:
    if (GameDataStateStorage.fields_.vehicle_current_count_ != group_count_at_entry) return 0;
    if (selector == 0) {
        order_veh_finalize_unselected_action(veh_id, faction,
            NetDaemonNet.channels_[0].leading_words_[3]);
    } else if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) != 0) {
        Veh &vehicle = Vehs[veh_id];
        if (direction >= 0 && direction < 8 && (vehicle.state & 8) != 0 &&
                vehicle.x == destination_x && vehicle.y == destination_y &&
                zoc_veh(destination_x, destination_y, faction) == 0)
            vehicle.state &= ~8U;
        if (IsMultiplayerNet && !ConsoleControlTurnActive && vehicle.order == ORDER_MOVE_TO &&
                vehicle.x == vehicle.waypoint_x[0] && vehicle.y == vehicle.waypoint_y[0]) {
            vehicle.order = ORDER_NONE;
            if ((vehicle.state & 0x02000200U) == 0x02000200U &&
                    vehicle.patrol_current_point < vehicle.waypoint_count) {
                ++vehicle.patrol_current_point;
                vehicle.order = ORDER_MOVE_TO;
                vehicle.waypoint_x[0] = vehicle.waypoint_x[vehicle.patrol_current_point];
                vehicle.waypoint_y[0] = vehicle.waypoint_y[vehicle.patrol_current_point];
                vehicle.state &= ~0x02010200U;
            }
        }
    }
    return selector;
}
