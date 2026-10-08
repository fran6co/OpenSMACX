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
// ORIGINAL: 0x0051DE60 ?on_sys_close@thunk1_Console@@QAEXXZ 0x0051DE60-0x0051DE68 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The pinned Console constructor 0x0050F460 stores Win-view table 0x0066EC18; slot 14 (byte offset 0x38) contains original thunk 0x0051DE60.", "The pinned Win primary slot 14 targets 0x00404280, the existing Win::on_sys_close contract; Console declares the real override.", "Original thunk 0x0051DE60 adjusts the vtordisp receiver and tail-jumps to mapped Console::on_sys_close at 0x0051D7C0.", "Seed history records `void (__thiscall ?on_sys_close@thunk1_Console@@QAEXXZ)(Console* this)` and flags hidden/sp_ready/purged_ok; those source findings remain with this original identity."], "reason": "Bind the original Console Win-view receiver adjustment to the existing typed Console callback at 0051DE60; the pinned Win base contract and Console constructor-installed view establish the ownership relation."}
// symbol    ?on_sys_close@Console@@$4PPPPPPPM@A@AEXXZ
// kind game
// ORIGINAL: 0x0051DED0 ?on_key_click@thunk1_Console@@QAEHHH@Z 0x0051DED0-0x0051DED8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The pinned Console constructor 0x0050F460 stores Win-view table 0x0066EC18; slot 25 (byte offset 0x64) contains original thunk 0x0051DED0.", "The pinned Win primary slot 25 targets 0x00406A80; Win declares int on_key_click(int, int), and Console retains the same int callback contract.", "Original thunk 0x0051DED0 adjusts the vtordisp receiver and tail-jumps to mapped Console::on_key_click at 0x005178C0.", "Seed history records `int (__thiscall ?on_key_click@thunk1_Console@@QAEHHH@Z)(Console* this, int, int)` and flags hidden/sp_ready/purged_ok; those source findings remain with this original identity."], "reason": "Bind the original Console Win-view receiver adjustment to the existing typed Console callback at 0051DED0; the pinned Win base contract and Console constructor-installed view establish the ownership relation."}
// symbol    ?on_key_click@Console@@$4PPPPPPPM@A@AEHHH@Z
// kind game
// ORIGINAL: 0x0051DF40 ?on_nc_hittest@thunk1_Console@@QAEXHH@Z 0x0051DF40-0x0051DF48 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The pinned Console constructor 0x0050F460 stores Win-view table 0x0066EC18; slot 75 (byte offset 0x12C) contains original thunk 0x0051DF40.", "The pinned Win primary slot 75 targets 0x005F5AD0; Win::on_nc_hittest and the existing Console::on_nc_hittest source method both have int(int,int) contracts.", "The seed catalogue's thunk prototype says void, but the original thunk preserves EAX while tail-jumping to 0x0050F680; that body sets EAX to 0 or 1 on its exits. The source owner keeps the int result; no source callback or base slot is renamed.", "Seed history records `void (__thiscall ?on_nc_hittest@thunk1_Console@@QAEXHH@Z)(Console* this, int, int)` and flags hidden/sp_ready/purged_ok; those source findings remain with this original identity."], "reason": "Bind the original Console Win-view receiver adjustment to the existing typed Console callback at 0051DF40; the pinned Win base contract and Console constructor-installed view establish the ownership relation."}
// symbol    ?on_nc_hittest@Console@@$4PPPPPPPM@A@AEHHH@Z
// kind game
#include "stdafx.h"
#include "prefwin.h"  // Console owns preference-window calls.
#include "interlude.h"  // interlude is declared by its owner.
#include "text.h" // TextBufferGetPtr is reloaded after parser callbacks.
#include "alphamovie.h"

// Console constructor-installed Win branch: capture4, resize13, mouse17, update63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x0066EC18 ? ??_7Console@@6BWin@@@ 0x0050F460
#include "original_seam.h"
#include "console.h"
#include "reportif.h"
#include "datalink.h"
#include "worldwin.h"
#include "multiwin.h"
#include "dipedit.h"  // editor_diplomacy's stack DipEdit
#include "game.h"
#include "base.h"
#include "general.h"
#include "popup.h"  // editor_diplomacy's modal faction picker
#include "basepop.h"
#include "infowin.h"
#include "map.h"
#include "path.h"  // Paths - map reset rebuilds the editor's path tables
#include "mapwin.h"
#include "maininterface.h"
#include "statuswin.h"
#include "reportwin.h"  // veh_turn observes the real report window's visibility
#include "win.h"
#include "fx.h"
#include "graphicwin.h"
#include "spritebox.h"
#include "net_class.h"
#include "menu.h"
#include "sprite.h"
#include "time.h"
#include "texture.h"
#include "strings.h"  // StringTemp - scenario-objective text scratch buffer
#include "veh.h"  // Vehs - the group table is the Vehs[i].state run
#include "faction.h"  // Players/PlayersData - the per-faction records editor_energy and set_ai walk
#include "xpops.h"  // X_pop - editor_on and friends raise the message pops
#include "lock.h"  // map_tile_bytes() - editor_clear_vehicles rewrites its records
#include "log.h"  // log_say - the chat / message handlers report through it
#include "netdaemon.h"  // synch_veh - the turn handlers resync vehicles

#include "netmsg.h"  // NetMsgPrimary - the editor's ALREADYDONE/PROTECT* pops
#include "tutwin.h"  // TutWinGlobal - survey2 raises the map tutorial on it
#include "technology.h"  // has_tech - editor_tech and the tech orders gate on it
#include "random.h"  // random - the editor's randomizer and the AI orders
#include "alpha.h"  // AlphaIniPrefs - the toggles mirror the prefs into it
#include "terraforming.h"  // Terraforming rules drive the scenario terrain picker
#include "planwin.h"
#include <cstdlib>
#include <cstring>

void __cdecl base_kill(int base_id);
int __cdecl wants_to_speak(int faction_id, int other_faction_id);
extern int ConsoleGroupAction;

// Shared tutorial popup caption: the original pointer cell selects the
// six-byte TUTOR string, not the adjacent WIZARD caption.
// RECOVERY_OBJECT: v2 0x00691B8C 0x00691B92 ?TutorialPopupCaptionText@@3PADA 0x00511800
char TutorialPopupCaptionText[6] = "TUTOR";
// RECOVERY_OBJECT: v2 0x00691B14 0x00691B18 ?TutorialPopupCaption@@3PADA 0x00511800
char *TutorialPopupCaption = TutorialPopupCaptionText;

// assemble uses Popup's shared checkbox result word for Vehicle Groups;
// do not duplicate the source-owned storage in popup.cpp.
extern int PopupWriteCheckState;
// The adjacent action selector distinguishes grouping versus removal.
// RECOVERY_OBJECT: v2 0x009BC070 0x009BC074 ?ConsoleGroupAction@@3HA 0x004D95F0
int ConsoleGroupAction;
// Existing free-function owners used by the commands below.
void __cdecl action_oblit(int vehicle_id, int base_id);
int __cdecl tut_check(int marker);
int __cdecl diplo_lock(int code);
void __cdecl diplo_unlock();
void __cdecl kill(int veh_id);
void __cdecl read_faction(int player_id);
int __cdecl veh_pick(int veh_id, const char *label, int flags);
int __cdecl eliminate_player(int faction_id, int is_eliminated);
int __cdecl veh_init(int prototype, int faction, int x, int y);

// Original go_to writes this flag around the PlanWindow selection callback.
// RECOVERY_OBJECT: v1 0x00688348 ?GoToPreviewActive@@3HA
int GoToPreviewActive = 1;

// Stateful selections shared between the scenario editor's vehicle and tech
// dialogs.  Each is a distinct original data cell with an independent
// lifetime; the initial values and call-site roles are preserved here.
// RECOVERY_OBJECT: v2 0x00688350 0x00688354 ?ScenarioVehicleFaction@@3HA 0x004DED00
int ScenarioVehicleFaction = -1;
// RECOVERY_OBJECT: v2 0x00688354 0x00688358 ?ScenarioVehicleMorale@@3HA 0x004DED00
int ScenarioVehicleMorale = 2;
// RECOVERY_OBJECT: v2 0x00688358 0x0068835C ?ScenarioTechFaction@@3HA 0x004DFAD0
int ScenarioTechFaction = -1;
// RECOVERY_OBJECT: v2 0x0068835C 0x00688360 ?ScenarioTechSelection@@3HA 0x004DFAD0
int ScenarioTechSelection = -1;
// RECOVERY_OBJECT: v2 0x0090DB8C 0x0090DB90 ?ScenarioVehicleMode@@3HA 0x004DED00
int ScenarioVehicleMode = 0;
// RECOVERY_OBJECT: v2 0x0090DB90 0x0090DB94 ?ScenarioPreviousPrototype@@3HA 0x004DED00
int ScenarioPreviousPrototype = 0;

// Original go_to stores source X/Y at 4D6E01/4D6E0D; goto_routine
// loads the independent dwords at 915680/915650 (4D60C2/4D60A8,
// 4D6103/4D60FC). Both starts are initially zero-filled storage.
// RECOVERY_OBJECT: v1 0x00915680 ?GoToSourceX@@3HA
int GoToSourceX = 0;
// RECOVERY_OBJECT: v1 0x00915650 ?GoToSourceY@@3HA
int GoToSourceY = 0;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D04A0 ?wait@Console@@QAEXH@Z 0x004D04A0-0x004D04E0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d04a0.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::wait(int veh_id) {
    field_23D18_ = 0;
    Vehs[veh_id].state |= 2;
    ready();
    field_23BE4_ = 0;
}

// ORIGINAL: 0x004D2DE0 ?disband@Console@@QAEXHH@Z 0x004D2DE0-0x004D2EF3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004d2de0.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004d2de0.cpp"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void Console::disband(int x_coord, int y_coord) {
    const int vehicle = veh_at(x_coord, y_coord);
    if (vehicle >= 0) {
        disband(vehicle);
        return;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 0x40) == 0 || IsMultiplayerNet) return;
    auto_undo();
    const int base = base_at(x_coord, y_coord);
    if (base < 0) return;
    parse_says(0, Bases[base].name_string, -1, -1);
    const bool alien = (Players[Bases[base].faction_id_current].rule_flags & 0x80) != 0;
    if (!popp(ScriptPopupFile,
              "DISBAND", 0,
              alien ? "al_col_sm.pcx" : "colfoun_sm.pcx", nullptr)) return;
    const int faction = Bases[base].faction_id_current;
    base_kill(base);
    ::draw_map(1);
    ConsoleStatusWin.redraw();
    eliminate_player(faction, 0);
}
#include "worldwin.h"  // WorldClimateWorldWin - editor_flat / editor_clear redraw it
#include "terraforming.h"  // Terraforming - destroy's option list reads its bit/label rows
#include "buttongroup.h"  // ButtonGroup::set - the editor brush/paint selectors
#include <cstdlib>
#include <cstring>

void __cdecl draw_cursor();                               // 0x0046AE00

// ORIGINAL: 0x00511520 ?set_move@Console@@QAEXH@Z 0x00511520-0x005117FD
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963abb2ef8a11892e1d754d80934bd50b12e:src/recovered/units/00511520.cpp", "source:src/console.h", "source:src/veh.h", "source:src/game.h", "source:src/mapwin.h"], "reason": "Coverage transcription using Console members, ConsoleGlobal, Vehs, and named game/network owners in place of seed receiver offsets and duplicate absolute aliases. The ready helper's selection operation is represented by ready_search. Preserved discoveries from the seed: ambience_veh (0x00446D10) and spot_all (0x005B6260) are not product-bound, the multiplayer guards at 0x0093E944 / 0x0093E8E0 are unowned, and 0x0093E8EC is a faction-indexed waiting latch. Their owners need recovery before those effects can be restored here."}
void Console::set_move(int force_update) {
    // A network handoff or automated turn owns movement input. not_my_turn()
    // already combines the network flag, handoff bit and active faction.
    if (IsMultiplayerNet &&
        (ConsoleControlTurnActive != 0 || not_my_turn() != 0)) {
        return;
    }

    bool did_reset = false;
    if (ConsoleGlobal.cursor_active_ == 0 || force_update != 0) {
        cursor_active_ = 0;
        field_23BF8_ = 0;
        field_23BFC_ = 1;
        did_reset = true;

        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        ::draw_cursor();
        cursor_active_ = 1;
        field_23BE4_ = 0;
    }

    // The seed calls an unhomed Console::ready() helper here. Its recovered
    // selection algorithm is ready_search; this preserves unit coverage while
    // leaving the helper's additional presentation effects for its own task.
    if (veh_selectable(selected_vehicle_) == 0) {
        selected_vehicle_ = ready_search(-1);
        did_reset = false;
    }

    if (selected_vehicle_ >= 0 && veh_selectable(selected_vehicle_) != 0) {
        const int veh_id = selected_vehicle_;
        const int tile_x = Vehs[veh_id].x;
        const int tile_y = Vehs[veh_id].y;
        field_23BF8_ = 0;
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        console_map_win()->draw_cursor();
        cursor_next(tile_x, tile_y);

        if (did_reset) {
            ConsoleInfoWin.change(0);
            ConsoleStatusWin.redraw();
            console_map_win()->main_caption();
            const int faction = ConsoleGlobal.cursor_history_index_;
            focus(ConsoleGlobal.cursor_history_x_[faction],
                  ConsoleGlobal.cursor_history_y_[faction],
                  Vehs[veh_id].faction_id);
            alpha_menu_.check(veh_id);
        }
        field_23BF4_ = 1;
        return;
    }

    field_23BE4_ = 0;
    if (ConsoleGlobal.cursor_active_ != 0) {
        ambience(0x1E);
        cursor_active_ = 0;
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        ::draw_cursor();
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        console_map_win()->main_caption();
        alpha_menu_.check(selected_vehicle_);
    }

    const int index = ConsoleGlobal.cursor_history_index_;
    const int tile_x = ConsoleGlobal.cursor_history_x_[index];
    const int tile_y = ConsoleGlobal.cursor_history_y_[index];
    ConsoleStatusWin.set_loc(tile_x, tile_y);
    focus(tile_x, tile_y, faction_id_);
}

namespace {

void append_decimal(char *output, int value) {
    char digits[16];
    _itoa(value, digits, 10);
    ::strcat(output, digits);
}

void update_then_redraw_vehicle_editor_tile(const Veh &vehicle) {
    ConsoleGlobal.update_data(0);
    draw_tile(vehicle.x, vehicle.y, 1);
}

}

// ORIGINAL: 0x004DDA50 ?editor_edit_veh@Console@@QAEXH@Z 0x004DDA50-0x004DE95B;0x0065B97E-0x0065BC13
// RECOVERY_BINDING: v1 {"evidence": ["Pinned4DDA50..4DE95B and65B97E..65BC13; retained git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004dda50.cpp", "Popup=-53DC; Scroll=-21AC and embedded Dialogs/Spot/Sprite/buttons/Heap account for native teardown. Ordinary Popup destructor replaces the entire cleanup, without an extra explicit close.", "Veh+E is faction_id. Mode2 loads prototype+24 chassis_id and Chassis+4A unsigned range, not weapon offense. Initial selection/history comes from ConsoleGlobal absolute storage, not the incoming receiver. Morale and faction menus retain current-value defaults; native tile call pushes type,y,x and reads coordinates after update callbacks.", "recovery/tasks/console-scenario-owner-wave.json; recovery/tests/test_console_editor_edit_veh_host.py actual-source behavior and lifecycle controls"], "reason": "Recover vehicle-editor modes in the genuine Console owner using typed vehicle, prototype, faction and base storage and a lexical Popup. Preserve original identity, seed and historical obligations; this is not current byte or runtime acceptance."}
// TRIED: like 0x005AE120, this has a real SEH frame with ~40 widget destructor calls; used the same real-RAII-for-Popup + separate-approximate-locals-for-the-rest strategy. The harder problem here was call ARGUMENTS: this compiler emits /EHsc calls through a FIXED scratch-slot area (puStack_20, pcStack_24, ... one slot per 4 bytes, reused across every call) rather than push/pop, because esp must stay constant for the fs:[0] chain - so Ghidra shows every call as `FUN_xxx();` with the real arguments as separate slot assignments beforehand, in scratch-slot order rather than argument order. Confirmed against several call sites' raw asm (veh_at, X_pop, strcat) that argument order is the REVERSE of assignment order (nearest-to-the-call assignment is arg1), and used that rule mechanically to reconstruct all ~50 call sites; ONE callee (itoa) broke the rule and was fixed by hand from its own push sequence. The scratch slots were declared `void *` uniformly (with `int *`/ `unsigned char *` kept only where real pointer arithmetic or dereference was needed) since their type varies call to call - this is why the rebuilt prologue class already diverges at #0. Every FUN_ callee resolved to a real name from the disassembly's own comments; none were guessed.
void Console::editor_edit_veh(int vehicle_id) {
    Popup popup;
    auto_undo();

    if (vehicle_id < 0) {
        if (ConsoleGlobal.cursor_active_ != 0) {
            vehicle_id = ConsoleGlobal.selected_vehicle_;
        } else {
            const int history = ConsoleGlobal.cursor_history_index_;
            const int x = ConsoleGlobal.cursor_history_x_[history];
            const int y = ConsoleGlobal.cursor_history_y_[history];
            const int found = veh_at(x, y);
            if (found < 0) {
                return;
            }
            vehicle_id = veh_pick(found, "PICKEDIT", 1);
            if (vehicle_id < 0) {
                return;
            }
        }
    }

    Veh &vehicle = Vehs[vehicle_id];
    const VehPrototype &initial_prototype = VehPrototypes[vehicle.proto_id];

    int checks = PopupWriteCheckState;
    const uint16_t original_flags = vehicle.flags;
    checks = (checks & ~0x01) | ((original_flags & VFLAG_IS_OBJECTIVE) ? 0x01 : 0);
    checks = (checks & ~0x02) | ((original_flags & 0x0080) ? 0x02 : 0);
    checks = (checks & ~0x04) | ((original_flags & 0x0100) ? 0x04 : 0);
    checks = (checks & ~0x08) | ((original_flags & 0x0200) ? 0x08 : 0);
    checks = (checks & ~0x10) | ((original_flags & VFLAG_LURKER) ? 0x10 : 0);
    checks = (checks & ~0x20) | ((original_flags & VFLAG_INVISIBLE) ? 0x20 : 0);
    checks = (checks & ~0x40) | ((vehicle.state & VSTATE_MONOLITH_UPGRADED) ? 0x40 : 0);
    PopupWriteCheckState = checks;

    StringTemp[0] = '\0';
    ::strcat(StringTemp, initial_prototype.veh_name);
    ::strcat(StringTemp, " ");
    ::strcat(StringTemp, "(");
    append_decimal(StringTemp, vehicle_id);
    ::strcat(StringTemp, ")");
    if (x_pop_short("EDITVEH", 0x41, nullptr) < 0) {
        return;
    }

    vehicle.flags &= static_cast<uint16_t>(0xF81F);
    vehicle.state &= ~VSTATE_MONOLITH_UPGRADED;
    checks = PopupWriteCheckState;
    if (checks & 0x01) vehicle.flags |= VFLAG_IS_OBJECTIVE;
    if (checks & 0x02) vehicle.flags |= 0x0080;
    if (checks & 0x04) vehicle.flags |= 0x0100;
    if (checks & 0x08) vehicle.flags |= 0x0200;
    if (checks & 0x10) vehicle.flags |= VFLAG_LURKER;
    if (checks & 0x20) vehicle.flags |= VFLAG_INVISIBLE;
    if (checks & 0x40) vehicle.state |= VSTATE_MONOLITH_UPGRADED;
    if (vehicle.flags & VFLAG_INVISIBLE) {
        // 4DDD47..5E pushes -1, y, x for owner_set(x, y, faction).
        owner_set(vehicle.x, vehicle.y, -1);
    }
    update_then_redraw_vehicle_editor_tile(vehicle);

    if (ConsoleGroupAction == 1) {
        popup.start(PopupStartCaption, "EDITMORALE", -1, nullptr, 0x40, nullptr);
        for (int morale_id = 0; morale_id < MaxMoraleNum; ++morale_id) {
            const char *name = Morale[morale_id].name;
            const VehPrototype &listed_prototype = VehPrototypes[vehicle.proto_id];
            if (vehicle.proto_id < 0x40 &&
                (Weapon[listed_prototype.weapon_id].offense_rating < 0 || vehicle.proto_id == 0x0F)) {
                name = Morale[morale_id].name_lifecycle;
            }
            popup.dialogs_.item(StringTable.get(
                static_cast<int>(reinterpret_cast<uintptr_t>(name))), morale_id);
            if (morale_veh(vehicle_id, 1, 0) == morale_id) {
                popup.set_value(morale_id);
            }
        }

        const int effective_morale = morale_veh(vehicle_id, 1, 0);
        const uint8_t old_morale = vehicle.morale;
        const int selected = popup.exec(0, nullptr);
        if (selected < 0) {
            return;
        }

        int morale = selected - (effective_morale - old_morale);
        if (morale < 0) morale = 0;
        if (morale > 6) morale = 6;
        vehicle.morale = static_cast<uint8_t>(morale);

        if (vehicle.faction_id == 0) {
            if (selected != 0) {
                for (;;) {
                    const int alien_morale = static_cast<int>(morale_alien(vehicle_id, 0));
                    const unsigned int rank = (vehicle.flags >> 3) & 3;
                    if (alien_morale <= selected || rank == 0) break;
                    vehicle.flags = static_cast<uint16_t>(
                        (rank * 8 - 8) | (vehicle.flags & 0xFFE7));
                }
            }
            if (selected < 7) {
                for (;;) {
                    const int alien_morale = static_cast<int>(morale_alien(vehicle_id, 0));
                    const unsigned int rank = (vehicle.flags >> 3) & 3;
                    if (selected <= alien_morale || rank >= 3) break;
                    vehicle.flags = static_cast<uint16_t>(
                        (rank * 8 + 8) | (vehicle.flags & 0xFFE7));
                }
            }
        }
        update_then_redraw_vehicle_editor_tile(vehicle);
        return;
    }

    if (ConsoleGroupAction == 2) {
        StringTemp[0] = '\0';
        append_decimal(StringTemp, static_cast<uint8_t>(vehicle.order));
        parse_says(0, StringTemp, -1, -1);
        StringTemp[0] = '\0';
        append_decimal(StringTemp, vehicle.moves_expended);
        parse_says(1, StringTemp, -1, -1);
        StringTemp[0] = '\0';
        append_decimal(StringTemp, vehicle.dmg_incurred);
        parse_says(2, StringTemp, -1, -1);

        const uint8_t input_fields_range =
            Chassis[VehPrototypes[vehicle.proto_id].chassis_id].range;
        if (input_fields_range > 1) {
            StringTemp[0] = '\0';
            append_decimal(StringTemp, vehicle.terraforming_turns);
            parse_says(3, StringTemp, -1, -1);
        }
        const uint8_t dialog_range =
            Chassis[VehPrototypes[vehicle.proto_id].chassis_id].range;
        const char *dialog = dialog_range <= 1 ? "EDITVEH2" : "EDITVEH2A";
        popup.start(PopupStartCaption, dialog, -1, nullptr, 0x44, nullptr);
        if (popup.exec(0, nullptr) < 0) {
            return;
        }
        vehicle.order = static_cast<int8_t>(::atoi(ParseStrBuffer[0].str));
        vehicle.moves_expended = static_cast<uint8_t>(::atoi(ParseStrBuffer[1].str));
        vehicle.dmg_incurred = static_cast<uint8_t>(::atoi(ParseStrBuffer[2].str));
        if (Chassis[VehPrototypes[vehicle.proto_id].chassis_id].range > 1) {
            vehicle.terraforming_turns = static_cast<uint8_t>(::atoi(ParseStrBuffer[3].str));
        }
        update_then_redraw_vehicle_editor_tile(vehicle);
        return;
    }

    if (ConsoleGroupAction == 3) {
        popup.start(PopupStartCaption, "EDITVEHWHO", -1, nullptr, 0x42, nullptr);
        for (int faction_id = 0; faction_id < MaxPlayerNum; ++faction_id) {
            PluralityDefault = Players[faction_id].is_noun_plural;
            StringTemp[0] = '\0';
            ::strcat(StringTemp, Players[faction_id].noun_faction);
            GenderDefault = Players[faction_id].noun_gender;
            popup.dialogs_.item(StringTemp, faction_id);
        }
        popup.set_value(vehicle.faction_id);
        const int faction = popup.exec(0, nullptr);
        if (faction < 0) {
            return;
        }
        vehicle.faction_id = static_cast<uint8_t>(faction);
        draw_tile(vehicle.x, vehicle.y, 1);
        ConsoleGlobal.update_data(0);
        return;
    }

    if (ConsoleGroupAction == 4) {
        popup.start(PopupStartCaption, "EDITVEHHOME", -1, nullptr, 0x42, nullptr);
        StringTemp[0] = '\0';
        ::strcat(StringTemp, label_get(25));
        popup.dialogs_.item(StringTemp, 9999);
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current == vehicle.faction_id) {
                popup.dialogs_.item(Bases[base_id].name_string, base_id);
            }
        }
        popup.set_value(vehicle.home_base_id < 0 ? 9999 : vehicle.home_base_id);
        int selected_base = popup.exec(0, nullptr);
        if (selected_base < 0) {
            return;
        }
        if (selected_base == 9999) selected_base = -1;
        vehicle.home_base_id = static_cast<int16_t>(selected_base);
        draw_tile(vehicle.x, vehicle.y, 1);
        ConsoleGlobal.update_data(0);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D2220 ?sentry@Console@@QAEXHH@Z 0x004D2220-0x004D24BB
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004D2220; retained seed src/recovered/units/004d2220.cpp", "Native4D245B parameter store is Veh+1C waypoint_y[0], not waypoint_x[2]. Typed Chassis triad, stack links, receiver faction, network locking and mapped tile state preserve original landing/load paths.", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions retain the original task baseline"], "reason": "Recover the complete Console command into its genuine owner using typed state and existing lifecycle/network APIs. Preserve original spans and retained seed evidence; this binding is not a byte, reference or runtime acceptance claim."}
void Console::sentry(int vehicle_id, int parameter) { // 0x004D2220
    const Veh &vehicle = Vehs[vehicle_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const uint8_t triad = Chassis[prototype.chassis_id].triad;
    Map *const tile = map_loc(vehicle.x, vehicle.y);
    const int tile_owner = tile->val2 & 0x0F;

    if (triad == TRIAD_AIR &&
        !(((tile->bit & BIT_BASE_IN_TILE) != 0 && tile_owner < MaxPlayerNum) ||
          (tile->bit & BIT_AIRBASE) != 0 ||
          stack_check(vehicle_id, 6, 0x80, -1, -1) != 0)) {
        return;
    }

    const bool empty_air_destination =
        triad == TRIAD_LAND &&
        (((tile->bit & BIT_BASE_IN_TILE) == 0 || tile_owner >= MaxPlayerNum) &&
         (tile->bit & BIT_AIRBASE) == 0);
    if (empty_air_destination) {
        int leader_id = vehicle_id;
        while (Vehs[leader_id].prev_veh_id_stack >= 0) {
            leader_id = Vehs[leader_id].prev_veh_id_stack;
        }

        for (int stack_id = leader_id; stack_id >= 0;
             stack_id = Vehs[stack_id].next_veh_id_stack) {
            const Veh &stack_vehicle = Vehs[stack_id];
            const VehPrototype &stack_prototype =
                VehPrototypes[stack_vehicle.proto_id];
            if (Chassis[stack_prototype.chassis_id].triad != TRIAD_AIR ||
                stack_prototype.plan != 7) {
                continue;
            }

            const bool alien = (Players[faction_id_].rule_flags & RFLAG_ALIEN) != 0;
            const char *const picture = alien ? "alair_sm.pcx" : "air_sm.pcx";
            if (!popp(ScriptPopupFile, "AIRLOAD", 0, picture, nullptr)) {
                return;
            }
            break;
        }
    }

    if (NetDaemonNet.lock_veh(&vehicle_id, 0, -1, -1, 0) != 0) {
        return;
    }
    sleep(vehicle_id);
    if (parameter != 0) {
        Vehs[vehicle_id].waypoint_y[0] = static_cast<int16_t>(parameter);
    }
    synch_veh(vehicle_id);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
    field_23BE4_ = 0;
    draw_tile(Vehs[vehicle_id].x, Vehs[vehicle_id].y, 1);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D5D30 ?oblit@Console@@QAEXH@Z 0x004D5D30-0x004D5F64
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004D5D30; retained seed src/recovered/units/004d5d30.cpp", "Native4D5D44 captures vehicle faction/coordinates; post-lock lookup rechecks ownership. Original post-lock disappearance/mismatch returns without unlocking; cancellation unlocks; network and local obliteration paths remain distinct.", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions retain the original task baseline"], "reason": "Recover the complete Console command into its genuine owner using typed state and existing lifecycle/network APIs. Preserve original spans and retained seed evidence; this binding is not a byte, reference or runtime acceptance claim."}
void Console::oblit(int vehicle_id) { // 0x004D5D30
    const Veh &vehicle = Vehs[vehicle_id];
    const int faction_id = vehicle.faction_id;
    const int x = vehicle.x;
    const int y = vehicle.y;

    if (not_my_turn()) {
        NetMsgPrimary.pop("SIMULNO", 5000, 0, nullptr);
        return;
    }
    if (!veh_ready(vehicle_id)) {
        NetMsgPrimary.pop("UNITMOVED", 5000, 0, nullptr);
        return;
    }

    int base_id = base_at(x, y);
    if (base_id < 0) {
        NetMsgPrimary.pop("OBLITNOBASE", 5000, 0, nullptr);
        return;
    }
    if (Bases[base_id].faction_id_current != faction_id) {
        NetMsgPrimary.pop("OBLITNOTYOURS", 5000, 0, nullptr);
        return;
    }
    if (is_objective(base_id)) {
        NetMsgPrimary.pop("OBLITNOTOBJECTIVE", 5000, 0, nullptr);
        return;
    }

    if (NetDaemonNet.lock_veh(&vehicle_id, 4, -1, -1, 0) != 0) {
        return;
    }

    // Recheck ownership after the network lock, as the original does before
    // exposing the destructive confirmation dialog.
    base_id = base_at(x, y);
    if (base_id < 0 || Bases[base_id].faction_id_current != faction_id) {
        return;
    }

    parse_says(0, Bases[base_id].name_string, -1, -1);
    const bool alien = (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;
    const char *const picture = alien ? "al_col_sm.pcx" : "colfoun_sm.pcx";
    const char *message = "OBLIT";
    if (Rules.tgl_oblit_base_atrocity == 0) {
        message = "OBLITOK";
    }

    if (!popp(ScriptPopupFile, message, 0, picture, nullptr)) {
        NetDaemonNet.unlock_veh();
        return;
    }
    if (NetDaemonNet.add_global() == 0) {
        if (IsMultiplayerNet != 0) {
            message_veh(0x2409, vehicle_id, base_id, 0);
            NetDaemonNet.await_exec(1);
            return;
        }
        action_oblit(vehicle_id, base_id);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D95F0 ?assemble@Console@@QAEXXZ 0x004D95F0-0x004D9970
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004D95F0; retained seed src/recovered/units/004d95f0.cpp", "Native4D9775..4D97D0 filters Chassis[prototype.chassis_id].triad, not chassis ID. Existing PopupWriteCheckState owns9BC06C; receiver faction filters vehicles while global cursor history supplies destinations.", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions retain the original task baseline"], "reason": "Recover the complete Console command into its genuine owner using typed state and existing lifecycle/network APIs. Preserve original spans and retained seed evidence; this binding is not a byte, reference or runtime acceptance claim."}
void Console::assemble() { // 0x004D95F0
    const int stored_filters = prefs_get("Vehicle Groups", 7, 0);
    for (int filter = 0; filter < 12; ++filter) {
        const uint32_t bit = 1u << filter;
        if ((stored_filters & static_cast<int>(bit)) != 0) {
            PopupWriteCheckState |= static_cast<int>(bit);
        } else {
            PopupWriteCheckState &= ~static_cast<int>(bit);
        }
    }

    if (x_pop_short("ASSEMBLEGROUP", 0x41, nullptr) < 0) {
        return;
    }

    clear_group();
    uint32_t active_filters = 0;
    for (int selected_filter = 0; selected_filter < 12; ++selected_filter) {
        const uint32_t bit = 1u << selected_filter;
        if ((static_cast<uint32_t>(PopupWriteCheckState) & bit) != 0) {
            active_filters |= bit;
        }
    }

    field_23D1C_ = 1;
    int best_vehicle = -1;
    for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
        Veh &vehicle = Vehs[vehicle_id];
        if (vehicle.faction_id != faction_id_) {
            continue;
        }

        if ((active_filters & 1) != 0) {
            int delta_x = std::abs(static_cast<int>(vehicle.x) -
                                   ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_]);
            if ((MapIsFlat & 1) == 0 && delta_x > static_cast<int>(MapLongitude)) {
                delta_x = static_cast<int>(MapLongitudeBounds) - delta_x;
            }
            const int delta_y = std::abs(static_cast<int>(vehicle.y) -
                                         ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_]);
            if (((delta_x + delta_y) & ~1) > 4) {
                continue;
            }
        }

        const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
        const int triad = Chassis[prototype.chassis_id].triad;
        const int plan = prototype.plan;
        if ((active_filters & 2) != 0 &&
            (vehicle.order == 1 || vehicle.order == 2)) {
            continue;
        }
        if ((active_filters & 4) != 0 &&
            (((vehicle.state & 0x01000200u) == 0x01000200u) ||
             (vehicle.state & 0x4200u) != 0)) {
            continue;
        }
        if (((active_filters & 8) != 0 && triad == TRIAD_LAND) ||
            ((active_filters & 0x10) != 0 && triad == TRIAD_SEA) ||
            ((active_filters & 0x20) != 0 && triad == TRIAD_AIR)) {
            continue;
        }
        if ((active_filters & 0x40) != 0 &&
            (plan == 0 || plan == 1 || plan == 5 || plan == 6)) {
            continue;
        }
        if ((active_filters & 0x80) != 0 && (plan == 2 || plan == 4)) {
            continue;
        }
        if (((active_filters & 0x100) != 0 && plan == 9) ||
            ((active_filters & 0x200) != 0 && plan == 0xB) ||
            ((active_filters & 0x400) != 0 &&
             (plan == 8 || plan == 0xA || plan == 7 || plan == 0xC))) {
            continue;
        }

        vehicle.state |= 0x08000000u;
        best_vehicle = vehicle_id;
    }

    if (ConsoleGroupAction == 1 && best_vehicle >= 0) {
        Veh &vehicle = Vehs[best_vehicle];
        vehicle.order = 0x18;
        vehicle.waypoint_x[0] = static_cast<int16_t>(
            ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_]);
        vehicle.waypoint_y[0] = static_cast<int16_t>(
            ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_]);
        vehicle.state &= 0xFCFFBDFFu;
        group(best_vehicle);
    }

    if (ConsoleGroupAction == 2) {
        for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
            Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id == faction_id_ &&
                (vehicle.state & 0x08000000u) != 0) {
                vehicle.order = 0;
                vehicle.state &= 0xFCFFBDFFu;
            }
        }
    }

    ::draw_map(1);
    prefs_put("Vehicle Groups", static_cast<int>(active_filters), 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E2000 ?corner_the_market@Console@@QAEXXZ 0x004E2000-0x004E2235
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004E2000; retained seed src/recovered/units/004e2000.cpp", "Native4E20BB..20DA formats amount/deadline before already-cornered and affordability checks. Native4E219F reloads current turn/rules after lock; message and final formatting read source-owned faction state after callbacks.", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions retain the original task baseline"], "reason": "Recover the complete Console command into its genuine owner using typed state and existing lifecycle/network APIs. Preserve original spans and retained seed evidence; this binding is not a byte, reference or runtime acceptance claim."}
void Console::corner_the_market() { // 0x004E2000
    if ((GameDataStateStorage.fields_.game_rules_ & 4) == 0) {
        return;
    }

    const int faction_id = ConsoleGlobal.faction_id_;
    if (!has_tech(Rules.tech_economic_victory, faction_id)) {
        return;
    }

    bool has_headquarters = false;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        if (Bases[base_id].faction_id_current == faction_id &&
            has_fac_built(FAC_HEADQUARTERS, base_id)) {
            has_headquarters = true;
            break;
        }
    }
    if (!has_headquarters) {
        NetMsgPrimary.pop("CORNERNOHQ", 5000, 0, nullptr);
        return;
    }

    PlayerData &faction = PlayersData[faction_id];
    const int amount = corner_market(faction_id);
    const int proposed_turn = GameDataStateStorage.fields_.turn_current_num_ + Rules.turns_corner_gbl_energy_mrkt;
    parse_num(0, amount);
    parse_num(1, static_cast<int>(game_year(proposed_turn)));

    if (faction.corner_market_turn > GameDataStateStorage.fields_.turn_current_num_) {
        parse_num(0, static_cast<int>(game_year(faction.corner_market_turn)));
        NetMsgPrimary.pop("CORNERALREADY", 5000, 0, nullptr);
        return;
    }

    if (faction.energy_reserves < amount) {
        parse_num(1, faction.energy_reserves);
        NetMsgPrimary.pop("CORNERNOTYET", 5000, 0, nullptr);
        return;
    }

    if (!popp(ScriptPopupFile, "CORNERMARKET", 0, "econwin_sm.pcx", nullptr)) {
        return;
    }
    if (diplo_lock(0x32) != 0) {
        return;
    }

    faction.corner_market_turn =
        GameDataStateStorage.fields_.turn_current_num_ + Rules.turns_corner_gbl_energy_mrkt;
    faction.corner_market_active = amount;
    faction.energy_reserves -= amount;
    synch_leader(faction_id);
    message_data(0x244F, 0, faction_id, faction.corner_market_turn, amount, 0);
    diplo_unlock();
    ConsoleGlobal.update_data(0);

    parse_num(0, static_cast<int>(game_year(faction.corner_market_turn)));
    parse_num(1, faction.corner_market_active);
    popp(ScriptPopupFile, "CORNERING", 0, "econwin_sm.pcx", nullptr);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00511800 ?turn_hints@Console@@QAEXXZ 0x00511800-0x00511BA8
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00511800; retained seed src/recovered/units/00511800.cpp", "Existing TutWin guard at8CC210 is member+53A8; stage8CC228 is+53C0. Typed global Console history, PlayerData prototype/base/population fields and diplomacy arrays replace seed address arithmetic. Caption pointer691B14 targets TUTOR691B8C.", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions retain the original task baseline"], "reason": "Recover the complete Console command into its genuine owner using typed state and existing lifecycle/network APIs. Preserve original spans and retained seed evidence; this binding is not a byte, reference or runtime acceptance claim."}
void Console::turn_hints() { // 0x00511800
    if ((GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_TUTORIAL_MSGS) == 0 ||
        IsMultiplayerNet != 0 || TutWinGlobal->tutorial_blocked()) {
        return;
    }

    const int faction_id = ConsoleGlobal.faction_id_;
    int turn = GameDataStateStorage.fields_.turn_current_num_;
    const int history = ConsoleGlobal.cursor_history_index_;
    const int history_x = ConsoleGlobal.cursor_history_x_[history];
    const int history_y = ConsoleGlobal.cursor_history_y_[history];

    if (turn == 1 && tut_check(1) != 0) {
        const int base_id = base_find(history_x, history_y, faction_id);
        if (base_id >= 0) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            if (selected_vehicle_ >= 0) {
                parse_says(1,
                           VehPrototypes[Vehs[selected_vehicle_].proto_id].veh_name,
                           -1, -1);
                TutWinGlobal->set_tour_stage(0);
                TutWinGlobal->tut_map("LANDFALL", history_x, history_y, -1,
                                      nullptr, 0x100, -1, -1);
            }
        }
        turn = GameDataStateStorage.fields_.turn_current_num_;
    }

    bool second_base_tutorial_shown = false;
    PlayerData &faction = PlayersData[faction_id];
    if (turn >= 0x14 && faction.current_num_bases == 1 &&
        faction.pop_total >= 2 && faction.proto_id_active[0] == 0 &&
        faction.proto_id_queue[0] == 0 && tut_check(0x100000) != 0) {
        const int base_id = base_find(history_x, history_y, faction_id);
        if (base_id >= 0) {
            const int x = Bases[base_id].x;
            const int y = Bases[base_id].y;
            focus(x, y, faction_id);
            parse_says(0, Bases[base_id].name_string, -1, -1);
            TutWinGlobal->tut_map("SECONDBASE", x, y, -2 - base_id,
                                  nullptr, 0, -1, -1);
            faction.last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
            second_base_tutorial_shown = true;
        }
    }

    if (!second_base_tutorial_shown) {
        const int turns_since_new_base = turn - faction.last_turn_new_base;
        if (turns_since_new_base >= 0x18 && faction.proto_id_active[0] == 0) {
            const int base_count = static_cast<int>(faction.current_num_bases);
            if (base_count > 1 && base_count <= 5) {
                const int base_id = base_find(history_x, history_y, faction_id);
                if (base_id >= 0) {
                    const int x = Bases[base_id].x;
                    const int y = Bases[base_id].y;
                    TutWinGlobal->tut_map("BUILDMORE", x, y, -2 - base_id,
                                          nullptr, 0, -1, -1);
                    faction.last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
                }
            }
        }
    }

    for (int other_faction = 1; other_faction < MaxPlayerNum;
         ++other_faction) {
        if (other_faction == faction_id) {
            continue;
        }
        const uint32_t treaties = faction.diplo_treaties[other_faction];
        if ((treaties & DTREATY_COMMLINK) == 0 ||
            (treaties & DTREATY_VENDETTA) == 0) {
            continue;
        }
        if (tut_check(0x80) == 0) {
            return;
        }

        GenderDefault = Players[other_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[other_faction].title_leader, -1, -1);
        GenderDefault = Players[other_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[other_faction].name_leader, -1, -1);
        GenderDefault = Players[other_faction].noun_gender;
        PluralityDefault = Players[other_faction].is_noun_plural;
        parse_says(2, Players[other_faction].noun_faction, -1, -1);
        x_pop_caption_label(TutorialPopupCaption, "VENDETTA", nullptr);
        return;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00516DA0 ?governor_directives@Console@@QAEXXZ 0x00516DA0-0x00517033
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00516DA0; retained seed src/recovered/units/00516da0.cpp", "Native516DC7/516E0E/516E97 uses ConsoleGlobal faction for eligibility/base scanning. Native516EBB..516FC9 repeatedly writes BaseCurrent->governor, not each visited base. Advanced settings use receiver faction at+23BD4; preserve this original behavior rather than silently fixing it.", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions retain the original task baseline"], "reason": "Recover the complete Console command into its genuine owner using typed state and existing lifecycle/network APIs. Preserve original spans and retained seed evidence; this binding is not a byte, reference or runtime acceptance claim."}
void Console::governor_directives() { // 0x00516DA0
    if (ConsoleControlTurnActive != 0 || field_23BE8_ == 0 ||
        !is_human(ConsoleGlobal.faction_id_)) {
        return;
    }

    const int directive = x_pop_short("GOVERNORCONTROL", 0x40, 0);
    if (directive < 0 || GameDataStateStorage.fields_.base_current_count_ <= 0) {
        return;
    }

    int first_owned_base = -1;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
            first_owned_base = base_id;
            break;
        }
    }
    if (first_owned_base < 0) {
        return;
    }

    if (directive == 5) {
        set_base(first_owned_base);
        if (TutWinBaseWindow.governor(1) != 0) {
            return;
        }
    }
    if (diplo_lock(0x32) != 0) {
        return;
    }

    StringTemp[0] = 0;
    for (int owned_base_id = 0; owned_base_id < GameDataStateStorage.fields_.base_current_count_; ++owned_base_id) {
        if (Bases[owned_base_id].faction_id_current != ConsoleGlobal.faction_id_) {
            continue;
        }

        // The image mutates BaseCurrent, the selected-base pointer, while
        // walking the owner's base records. It does not retarget that pointer
        // to each matching record inside this loop.
        Base &base = *BaseCurrent;

        int label_offset;
        switch (directive) {
        case 0:
            base.governor &= 0x7FFFFFFFu;
            base.governor &= 0xF0FFFFFFu;
            label_offset = 0xAF8 / 4;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            base.governor |= 0x80000000u;
            base.governor &= 0xF0FFFFFFu;
            base.governor |= static_cast<uint32_t>(1u << (23 + directive));
            label_offset = (0x824 + (directive - 1) * 4) / 4;
            break;
        case 5:
            base.governor = static_cast<uint32_t>(
                PlayersData[faction_id_].base_governor_adv);
            label_offset = 0xAFC / 4;
            break;
        default:
            continue;
        }
        strcat(StringTemp, label_get(label_offset));
    }

    parse_says(0, StringTemp, -1, -1);
    NetMsgPrimary.pop("GOVERNORCONTROLDONE", 5000, 0, nullptr);
    diplo_unlock();
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004DE960 ?editor_kill_veh@Console@@QAEXH@Z 0x004DE960-0x004DECF7;0x0065BC13-0x0065BD30 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original4DE989 constructs local Popup before auto_undo5ABF20 at4DE995. Negative explicit ID reads absolute ConsoleGlobal939288/28C and cursor-history arrays, not receiver-relative fields; canceled lookup/picker returns through real cleanup.", "Pinned original4DE9B7..4DE9D6 pushes parse_says(0,VehPrototypes[Vehs[vehicle].proto_id].veh_name,-1,-1), correcting conflicting seed arguments. Confirmation invokes real kill5C0B00, not veh_kill.", "Pinned relocation-free NUL literals PICKKILL at689068..689071 and DELETEME at689074..68907D. Full code4DE960..4DECF7 and cleanup65BC13..65BD30 remain original obligations; seed004de960 retained."], "reason": "Recover the complete editor vehicle-deletion command with genuine Popup lifetime and typed global Console selection. The original ignores its member receiver; selecting a different Console must not change its global cursor lookup."}
void Console::editor_kill_veh(int veh_id) {
    Popup popup;
    auto_undo();

    int vehicle = veh_id;
    if (vehicle < 0) {
        if (ConsoleGlobal.cursor_active_ == 0) {
            const int cursor = ConsoleGlobal.cursor_history_index_;
            vehicle = veh_at(ConsoleGlobal.cursor_history_x_[cursor],
                             ConsoleGlobal.cursor_history_y_[cursor]);
            if (vehicle < 0) return;
            vehicle = veh_pick(vehicle, "PICKKILL", 1);
            if (vehicle < 0) return;
        } else {
            vehicle = ConsoleGlobal.selected_vehicle_;
        }
    }

    parse_says(0, VehPrototypes[Vehs[vehicle].proto_id].veh_name, -1, -1);
    if (X_pop("DELETEME", nullptr) != 0) kill(vehicle);
}

// Shared by the turn loop and network lock guards. The original writers
// are control_turn, net_end_of_turn and net_control_turn (see console.h).
// RECOVERY_OBJECT: v2 0x0093A938 0x0093A93C ?ConsoleControlTurnActive@@3HA 0x00530D50
int ConsoleControlTurnActive;

// Chat passes 939FE8 as default text, not editable output (5157CC).
// BasePop forwards it to EditGroup::set_default, which allocates and copies
// input at 612108..612134; entered text is ParseStrBuffer[0].str.
// Only the initial NUL is established: the original allocation extent and
// containing owner remain unknown. Do not overlap the distinct default
// text start passed by name_landmark at 5163E6 (939FEC).
// RECOVERY_OBJECT: v1 0x00939FE8 ?ConsoleChatInput@@3PADA
char ConsoleChatInput[1] = {0};
// RECOVERY_OBJECT: v1 0x00939FEC ?LandmarkDefaultName@@3PADA
char LandmarkDefaultName[1] = {0};

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00515770 ?chat@Console@@QAEXH@Z 0x00515770-0x005158F4
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00515770.cpp", "src/netdaemon.h", "src/xpops.h"], "reason": "The chat command belongs to Console, reads network lock state through NetDaemonNet, and uses source-owned text buffers and message services rather than fixed executable addresses; seed matching alone is not structural proof."}
void Console::chat(int target_faction) {
    if (!IsMultiplayerNet) return;

    const LockStateEntry *state = NetDaemonNet.lock_state_;
    const unsigned int local_bit = 1u << (ConsoleGlobal.faction_id_ & 31);
    if (ConsoleControlTurnActive &&
        (state[1].leading_words_[1] & local_bit) == 0 &&
        state[0].trailing_words_[4] == 0) {
        NetMsgPrimary.pop("FINISHUPKEEP", 1000, 0, 0);
        return;
    }

    if (X_pop_ask("CHATASK", 0xff, ConsoleChatInput, pop_wait, 0) != 0)
        return;

    char *message = ParseStrBuffer[0].str;
    if (message[0] == '!') {
        message_chat(0x4e00, 0, message + 1);
        log_say("Sent CHAT ALL(1)", message, 0, 0, 0);
        return;
    }

    const unsigned int enabled = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff;
    if (state[0].trailing_words_[0] >= 2 &&
        (enabled & state[2].trailing_words_[6]) != enabled) {
        for (int faction = 1; faction < 8; ++faction) {
            const unsigned int bit = 1u << faction;
            if (!(enabled & bit)) continue;
            const bool selected = target_faction > 0
                ? faction == target_faction
                : CommlinkPending ? faction == CommlinkFaction
                : TurnRedrawPending || (state[2].trailing_words_[6] & bit) != 0;
            if (!selected) continue;
            message_chat(0x1e00, faction, message);
            log_say("Sent CHAT", message, faction, 0, 0);
        }
        return;
    }

    message_chat(0x4e00, 0, message);
    log_say("Sent CHAT ALL(2)", message, 0, 0, 0);
}

// FREE FUNCTIONS THE CONSOLE BODIES CALL that no header in src/ declares yet.
// Each line carries the image address its catalogue entry names, and the
// argument list is decoded from that entry's mangled name - not guessed.
// These are HANDOFF rows: the declaration belongs beside its own family's
// header (veh_ready beside veh.h's other veh_* verbs, load_map beside
// save_game in the save/_load family, draw_cursor beside mapwin.h's free
// draw_map), and the definition is whoever's batch owns that address. They
// are declared here so the console callers compile and measure; when the real
// header lands, delete the line here. Their forwarders are at the end of
// src/pending_bodies.cpp.
int __cdecl veh_ready(int veh_id);                        // 0x005C1B70 ?veh_ready@@YAHH@Z
int __cdecl tut_check(int veh_id);                            // 0x004BA5F0 ?tut_check@@YAHH@Z
int __cdecl tut_check2(int veh_id);                           // 0x004BA650 ?tut_check2@@YAHH@Z
int __cdecl save_game(int save_slot);                            // 0x005A9EB0 ?save_game@@YAHH@Z
void __cdecl base_kill(int base_id);                      // 0x004E5250 ?base_kill@@YAXH@Z
int __cdecl load_game(int save_slot, int faction_id);                    // 0x005AAAB0 ?load_game@@YAHHH@Z
int __cdecl load_map();                                   // 0x005AB600 ?load_map@@YAHXZ
void __cdecl save_map();                                  // 0x005ABB90 ?save_map@@YAXXZ
// a2 is 0 at all three call sites and a3 is -1 at two of them; nothing in
// the tree names either, so the forward declaration leaves them unnamed.
void __cdecl set_dirty();                                 // 0x00592A80 ?set_dirty@@YAXXZ
int __cdecl size_of_planet(int planet_size);                       // 0x0058CED0 ?size_of_planet@@YAHH@Z
int __cdecl supply_options(int veh_id, int base_id);               // 0x00593A00 ?supply_options@@YAHHH@Z
void __cdecl veh_kill(int veh_id);                        // 0x005C08C0 ?veh_kill@@YAXH@Z
int __cdecl veh_pick(int veh_id, const char *label, int flags);  // 0x0048D0D0 ?veh_pick@@YAHHPBDH@Z
void __cdecl world_build();                               // 0x005C86E0 ?world_build@@YAXXZ
void __cdecl world_fungus();                              // 0x005C3440 ?world_fungus@@YAXXZ
int __cdecl veh_selectable(int veh_id);                       // 0x005C1A20 ?veh_selectable@@YAHH@Z
int __cdecl eliminate_player(int faction_id, int is_eliminated);             // 0x005B3380 ?eliminate_player@@YAHHH@Z
int __cdecl veh_unmoved(int veh_id);                          // 0x005C1AB0 ?veh_unmoved@@YAHH@Z
char *__cdecl get_pact_hood(int faction_id, int faction_id_with);              // 0x0058FB00 ?get_pact_hood@@YAPADHH@Z
void __cdecl diplomacy_ends(int faction_id, int faction_id_with);              // 0x0053E470 ?diplomacy_ends@@YAXHH@Z
int __cdecl diplo_lock(int faction_id);                           // 0x00539820 ?diplo_lock@@YAHH@Z
void __cdecl double_cross(int faction_id, int faction_id_with, int treaty);        // 0x0050BCC0 ?double_cross@@YAXHHH@Z
int __cdecl upgrade_cost(int facility_id, int base_id, int flags);         // 0x004EFD50 ?upgrade_cost@@YAHHHH@Z
int __cdecl resource_yield(int x, int y, int resource_kind, int faction_id, int a5); // 0x004E7DC0 ?resource_yield@@YAHHHHHH@Z
int __cdecl can_convoy(int faction_id, int veh_id);                   // 0x00564D90 ?can_convoy@@YAHHH@Z
void __cdecl hall_of_fame(int is_retire);                        // 0x0044AE50 ?hall_of_fame@@YAXH@Z
void __cdecl show_credits();                              // 0x00428DF0 ?show_credits@@YAXXZ
void __cdecl show_replay();                               // 0x005ADBF0 ?show_replay@@YAXXZ
int __cdecl tech_advance(int tech_id);                         // 0x005BE530 ?tech_advance@@YAHH@Z
void __cdecl social_select(int faction_id);                       // 0x005B5620 ?social_select@@YAXH@Z
void __cdecl design_new_veh(int faction_id, int proto_id);              // 0x00583CB0 ?design_new_veh@@YAXHH@Z
int __cdecl enemy_veh(int veh_id);                            // 0x00579240 ?enemy_veh@@YAHH@Z
void __cdecl message_veh(int veh_id, int a2, int a3, int a4); // 0x00592E10 ?message_veh@@YAXHHHH@Z
void __cdecl message_base(int base_id, int a2, char *text, int a4); // 0x00592E70 ?message_base@@YAXHHPADH@Z
void __cdecl message_chat(int a1, int a2, char *text);    // 0x00593190 ?message_chat@@YAXHHPAD@Z
void __cdecl message_landmark(int landmark_id, int a2, char *text, int a4, int a5, int a6, int a7); // 0x00593110
void __cdecl monument(int base_id);                            // 0x00476A50 ?monument@@YAXH@Z
void __cdecl quayle(int base_id);                              // 0x00496670 ?quayle@@YAXH@Z
void __cdecl report_score(int faction_id);                        // 0x004ACB80 ?report_score@@YAXH@Z
char *__cdecl get_phrase(int phrase_id, int faction_id);                 // 0x0058F810 ?get_phrase@@YAPADHH@Z
char *__cdecl get_noun_phrase(int phrase_id, int faction_id);            // 0x0058F8B0 ?get_noun_phrase@@YAPADHH@Z

// 0x00939FD0, the tick counter auto_play_callback (below) drives: it counts
// this up once per callback, and when it passes 100 it plays one turn for the
// current faction through Console::end_my_turn and clears it again. The only
// references to the address in the whole image are this function's three
// (increment, compare, clear), and the slot is zero-fill .data, so it starts
// at 0 and the auto-play never fires until something - Console::on_key_click,
// which registers the callback - starts it.
int AutoPlayCounter = 0;  // 0x00939FD0, defined here
// The splitter-drag line's last drawn position, the pair the nc handlers
// draw over with window_line_raw before the next move repaints it.
// 0x00939FD8 (vertical line x) and 0x00939FDC (horizontal line y).
int ConsoleDragLineX = 0;  // 0x00939FD8, defined here
int ConsoleDragLineY = 0;  // 0x00939FDC, defined here

// HANDOFF: the process-wide WorldWin at 0x008E9F60 is map.cpp's
// WorldClimateWorldWin - that file defines it and world_climate clears its
// terrain. The shared extern belongs in worldwin.h, which this TU does not
// own, so it is declared here until that row lands; delete the line when it
// does. editor_flat and editor_clear reach it as `mov ecx, 0x8e9f60` and tail
// into GraphicWin::redraw.
extern WorldWin WorldClimateWorldWin;  // 0x008E9F60

// THE CURSOR SPRITE ARRAY sprite.cpp owns. Declared here because
// Console::cursor_on hands its elements to Win::set_cursor by address; the
// definition and the 0x2c stride comment live in src/sprite.cpp.

/*
Purpose: Build the game console - its virtual GraphicWin base, its MapWin
         base, its own extra (non-virtual) embedded GraphicWin, and its own
         Buffer/Time/Menu/Sprite members.
// ORIGINAL: 0x0050F460 ??0Console@@QAE@H@Z 0x0050F460-0x0050F629;0x0065D250-0x0065D2EF BYTE_EXACT
// symbol    ??0Console@@QAE@XZ
// size      616 bytes
// prototype void (__thiscall ??0Console@@QAE@H@Z)(Console* this, int)
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004626E0 0x005D4CF0 0x005D7210 0x005E37E0 0x005FAC60 0x006161D0 0x006457C2
Return Value: this

MEASURED: spelled as the bare real constructor - the catalogue's `H` is the
hidden most-derived flag (see the note in `console.h` and mapwin.h's
attempt-#2 note). The earlier `Console(int input)` spelling landed with a
known regression by direction: VC6 emitted flag AND int, doubling the body's
vbase machinery to 169 instructions against the image's 106. That regression
comes back off with the bare spelling now that MapWin's real constructor
converted.
*/
Console::Console()
    : MapWin() {
    // The compiler owns the construction machinery for the real constructor:
    // the Console vbtable store, the virtual GraphicWin base at +0x23D94
    // under the most-derived flag, the MapWin base (the mem-initialiser
    // above - the compiler pushes the hidden flag 0 for the base stage,
    // where the image's ??0Console pushes it once), and every declared
    // member below in declaration order - the same sequence the image's
    // ??0Console spells. What remains in the body is the menu subobject's
    // share of the scalar field clears.

    // The menu subobject's AlphaMenu-stage vptrs need no hand store either:
    // alpha_menu_ is a real AlphaMenu, and its own constructor - which the
    // compiler calls with the rest of the members - installs both of its
    // vtables (the Win-part table at +0 and the Buffer-part table at +0x444).

    field_23BC8_ = static_cast<uint32_t>(-1);
    cursor_active_ = 0;
    field_23BE4_ = 0;
    field_23BE8_ = 0;
    field_23BF0_ = 0;
    field_23BF4_ = 0;
    survey_mode_ = 0;
    field_23C04_ = 0;
    field_23BFC_ = 0;
    field_23BF8_ = 0;
    cursor_history_index_ = 0;

    // 0x0050F5D3..0x0050F5EE repeatedly clears only the current slot (zero).
    // The other slots and history head remain untouched by construction.
    for (int32_t i = 0x20; i != 0; i--) {
        cursor_history_x_[cursor_history_index_] = 0;
        cursor_history_y_[cursor_history_index_] = 0;
    }

    field_23D10_ = 0;
    field_23D18_ = 0;
    field_23D88_ = 0;
    field_23D80_ = 0;
    field_23D8C_ = 0;
    field_23D84_ = 0;
}

/*
Purpose: Tear down the game console - its own Buffer/Time/Menu/Sprite
         members, its own extra embedded GraphicWin, then the MapWin
         fields it holds directly (no separate call into MapWin's own
         destructor).
// ORIGINAL: 0x0051D9F0 ??1Console@@QAE@XZ 0x0051D9F0-0x0051DC10;0x0065D0E0-0x0065D220
// symbol    ??1Console@@UAE@XZ
// body      src/console.h
//
// HOMED INTO THE CLASS 2026-08-30: the hand-spelled body this spot carried
// measured MISMATCH (7/123) because everything but `close()` is
// compiler-owned - the EH frame, the vtable/vtordisp re-stores on the
// vbase, the member teardown of the GraphicWin/Buffer/Time/Menu/Sprite
// members console.h now declares, and MapWin's own base teardown inlined
// (clear(0) and MapWin's carved members). `MEASURED ~Console() { close();
// }` in console.h emits THIS piece as the out-of-line COMDAT
// ??_DConsole@@QAEXXZ (the base-object destructor; the catalogue's ??1
// name sits on the base flavor - the true ??1Console complete destructor
// is 0x0051DDF0, `lea esi,[ecx-0x23D94]` adjust, a call to this, then
// ??1GraphicWin on the vbase, then the conditional operator delete, with
// the deleting wrapper ??_GConsole at 0x0051DDE0 before it). The funclet
// span 0x0065D0E0-0x0065D220 is that EH machinery's own.
//
// TRIED (2026-08-30, after the carve): 30/123, 151 instructions against the
//        image's 123, and every teardown element is present in the right
//        order - the whole frame (EH prologue, all three vtable/vtordisp
//        re-stores) agrees through instruction 22. The +28 are VC6's /O2
//        member-teardown discipline, which the image does not carry: our
//        compile nulls each Buffer member's vptr (`mov [edi],0`) before
//        every ??1Buffer call - five of them, own and inlined-MapWin
//        alike - spills the member pointer through [ebp-0x10]/[ebp-0x14],
//        and renumbers the EH states (+3 at close: 9 where the image has
//        6). The image calls the same five ??1Buffer functions bare.
//        Established along the way: ~Buffer's virtuality is NOT the
//        trigger (the store appears with it spelled either way - a
//        non-virtual spelling was tried and reverted; the Buffer-side
//        vftable 0x0066FC48 opens with a deleting-dtor thunk, which only
//        a virtual destructor produces), and the /Od build tree emits no
//        stores - this is /O2-specific. MapWin's own ??1DMapWin, carved
//        identically, went BYTE_EXACT 73/73 in the same pass, so the
//        model and the method are right; whatever makes VC6 skip the
//        vptr-null for Console's Buffer members specifically is still
//        hiding.
// size      864 bytes
// prototype void (__thiscall ??1Console@@QAE@XZ)(Console* this)
// callers   2   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462870 0x0051D7D0 0x005D4DD0 0x005D7410 0x005E3820 0x005FAD00 0x00616200 0x00618EE0 0x006252B0 0x006456E4
Return Value: n/a

`this` IS NOT Console's own front, matching `guarded_teardowns.cpp`'s
already-matching `->Console::~Console()` on `TeardownObject00939444` -
`g_CONSOLE`'s front plus 0x23D94, the offset of the embedded virtual-base
GraphicWin, the same convention PlanWin's and MapWin's destructors use (see
the note in `planwin.cpp`).
*/
/*
Purpose: Open the shared preferences window to the preferences page.
// ORIGINAL: 0x00514EF0 ?set_preferences@Console@@QAEXXZ 0x00514EF0-0x00514EFD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_preferences() {
    ConsolePrefWin.PrefWin::display(0);
}

/*
Purpose: Open the shared preferences window to the automation page.
// ORIGINAL: 0x00514F10 ?set_auto_preferences@Console@@QAEXXZ 0x00514F10-0x00514F1D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_auto_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_auto_preferences() {
    ConsolePrefWin.PrefWin::display(3);
}

/*
Purpose: Open the shared preferences window to the base page.
// ORIGINAL: 0x00514F20 ?set_base_preferences@Console@@QAEXXZ 0x00514F20-0x00514F2D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_base_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_base_preferences() {
    ConsolePrefWin.PrefWin::display(2);
}

/*
Purpose: Open the shared preferences window to the audiovisual page.
// ORIGINAL: 0x00514F30 ?set_audiovisual@Console@@QAEXXZ 0x00514F30-0x00514F3D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_audiovisual@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_audiovisual() {
    ConsolePrefWin.PrefWin::display(4);
}

/*
Purpose: Open the shared preferences window to the map-display page.
// ORIGINAL: 0x00514F40 ?set_map_display@Console@@QAEXXZ 0x00514F40-0x00514F4D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_map_display@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_map_display() {
    ConsolePrefWin.PrefWin::display(5);
}







/*
Purpose: Clear the console's active-group field and drop the highlight bit
         (0x08000000) from every entry of the group table, at its 0x34 stride.
// ORIGINAL: 0x0050F650 ?clear_group@Console@@QAEXXZ 0x0050F650-0x0050F67B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?clear_group@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
Verification note: the sweep's surviving swaps reorder the object-field clear
against the group-count load and one declaration against another; both pairs
touch disjoint state, so the order is unobservable and the mutants are
equivalent by construction.
*/
void Console::clear_group() {
    field_23D1C_ = 0;
    const int32_t count = GameDataStateStorage.fields_.vehicle_current_count_;
    for (int32_t index = 0; index < count; ++index) {
        // 0x0095282C is Vehs + 4: the group table is the state dword of every
        // Veh record (see console.h), so the walk is an offset of Vehs, not a
        // separate object.
        Vehs[index].state &= 0xF7FFFFFFu;
    }
}


/*
Purpose: Report whether editing is locked out. Only meaningful in the scenario
         editor; there editing is locked whenever Scroll Lock is toggled on or
         the game is in editor-only mode.
// ORIGINAL: 0x004E1F40 ?edit_lock@Console@@QAEHXZ 0x004E1F40-0x004E1F70 BYTE_EXACT
// size      48 bytes
// prototype int (__thiscall ?edit_lock@Console@@QAEHXZ)(Console* this)
// callers   8   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004E1F51
Return Value: 1 when editing is locked, 0 otherwise
Status: Complete
*/
int Console::edit_lock() {
    if (!(GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR)) {
        return 0;
    }
    if ((::GetKeyState(VK_SCROLL) & 1) ||
        (GameDataStateStorage.fields_.game_state_ & STATE_EDITOR_ONLY_MODE)) {
        return 1;
    }
    return 0;
}


/*
Purpose: Open the shared preferences window to the advanced page.
// ORIGINAL: 0x00514F00 ?set_adv_preferences@Console@@QAEXXZ 0x00514F00-0x00514F0D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_adv_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_adv_preferences() {
    ConsolePrefWin.PrefWin::display(1);
}

/*
Purpose: Undo the last scenario-editor change, by asking load_undo for the
         editor's undo slot.
// ORIGINAL: 0x004E1F20 ?editor_undo@Console@@QAEXXZ 0x004E1F20-0x004E1F29 BYTE_EXACT
// size      9 bytes
// prototype void (__thiscall ?editor_undo@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ABE40
Return Value: n/a
Status: Complete
*/
void Console::editor_undo() {
    load_undo(1);
}




/*
Purpose: Refresh everything the console shows after a selection or turn change:
         hand the change code to the shared InfoWin, redraw the shared
         StatusWin, then push the map's caption into the main interface.
// ORIGINAL: 0x00514880 ?update_data@Console@@QAEXH@Z 0x00514880-0x005148AA BYTE_EXACT
// size      42 bytes
// prototype void (__thiscall ?update_data@Console@@QAEXH@Z)(Console* this, int)
// callers   33   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458900 0x0046FB10 0x004B9EA0
Return Value: n/a
Status: Complete with a temporary StatusWin::redraw original dependency
Verification note: three properties of this body are unobservable at every
oracle tier, because the original's own callees discard them rather than
because the fixtures are weak.
  1. InfoWin::change at 0x00458900 is a bare `ret 4` - it drops both its
     `this` and its argument - so neither the call nor the value of a1 has any
     effect to observe. src/infowin.cpp models it as an empty body.
  2. MapWin::main_caption at 0x0046FB10 overwrites ecx before using it, so the
     MapWin pointer loaded out of ConsoleMapWinSlot never reaches anything
     that reads it.
  3. The original leaves EAX holding main_caption's residue, whose chain is
     already terminated by the committed `void MapWin::main_caption()`, so
     there is no residue available to return and fabricating a constant would
     be wrong. Kept `void`, matching the mangled name.
Mutation-harness survivors for the dropped `change` call and for a perturbed
a1 are therefore equivalent by construction.
*/
void Console::update_data(int change_flags) {
    // 0x00514883 mov eax,[ebp+8] / 0x00514886 mov ecx,0x7ad2a0 /
    // 0x0051488B push eax / 0x0051488C call 0x458900.
    ConsoleInfoWin.change(change_flags);
    // 0x00514891 mov ecx,0x8c5568 / 0x00514896 call 0x4b9ea0.
    ConsoleStatusWin.StatusWin::redraw();
    // 0x0051489B mov ecx,[0x7d3c3c] - a load, not an address-of - then
    // 0x005148A1 call 0x46fb10. The slot is read here, never cached.
    console_map_win()->main_caption();
}


/*
Purpose: Report the cursor's new tile position to the status bar and record
         it in this Console's own ring buffer, unless it is the same tile
         already selected in ConsoleGlobal's history. In debug mode, warn
         when the coordinates have mismatched parity, then normalise them
         to matching parity before anything else runs.
// ORIGINAL: 0x005109B0 ?cursor_next@Console@@QAEXHH@Z 0x005109B0-0x00510A59 BYTE_EXACT
// size      169 bytes
// prototype void (__thiscall ?cursor_next@Console@@QAEXHH@Z)(Console* this, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00517C50 0x00644C40 0x0064B330
Return Value: n/a
Status: Complete
*/
void Console::cursor_next(int x_coord, int y_coord) {
    if (((x_coord ^ y_coord) & 1) != 0) {
        if ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
            danger(CursorNextDangerMsg1, CursorNextDangerMsg2, x_coord,
                  y_coord, 0);
        }
        if ((x_coord & 1) != 0) {
            x_coord -= 1;
        }
        if ((y_coord & 1) != 0) {
            x_coord += 1;
        }
    }
    ConsoleStatusWin.set_loc(x_coord, y_coord);
    int idx = ConsoleGlobal.cursor_history_index_;
    if (x_coord == ConsoleGlobal.cursor_history_x_[idx]) {
        if (y_coord == ConsoleGlobal.cursor_history_y_[idx]) {
            return;
        }
    }
    int newIdx = (cursor_history_head_ + 1) % 32;
    cursor_history_head_ = newIdx;
    cursor_history_index_ = newIdx;
    cursor_history_x_[newIdx] = x_coord;
    int reIdx = cursor_history_head_;
    cursor_history_y_[reIdx] = y_coord;
}

// The retained exact seed establishes a Console receiver, two tile
// coordinates and this sequence of cursor-overlay operations. The original
// symbol has no semantic name, so keep its neutral identity.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00510C90 sub_510c90 0x00510C90-0x00510CE0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00510c90.cpp", "source:src/console.h", "source:src/mapwin.h"], "reason": "The retained exact seed shows an unchanged Console receiver, two tile coordinates, and the overlay-clear, redraw, cursor-next call sequence. The image supplies no semantic name."}
// symbol    ?sub_510c90@Console@@QAEHHH@Z
int Console::sub_510c90(int x_coord, int y_coord) {
    const int had_survey = survey_mode_;
    field_23BF8_ = 0;
    if (had_survey != 0) {
        survey_mode_ = 0;
        console_map_win()->draw_map(1);
    }
    console_map_win()->draw_cursor();
    cursor_next(x_coord, y_coord);
    return 0;
}

/*
Purpose: Point the map windows at one tile on behalf of one faction. Build a
         view-tag mask from the faction argument, walk the eight map-window
         slots, and hand the coordinate to every window whose tag the mask
         selects. The primary window, slot 0, is handled quite differently from
         the rest, and only its success counts: when it moves, the queued input
         is flushed and 1 is returned.
// ORIGINAL: 0x005108A0 ?focus@Console@@QAEXHHH@Z 0x005108A0-0x005109A3 BYTE_EXACT
// LEVER: four stacked fixes took this from 32 of 85 to BYTE_EXACT, each measured on its own. (1) THE TAG TEST IS THE OUTER ONE. `if (slot == 0) { if (!tagged ...) } else if (!tagged)` makes the tag survive the slot branch, so VC6 materialises it (`and` plus `setne` plus a second `test`) where the image has one `test ecx, edx` at 0x00510908 with two consumers; the chained guard `(mask & flags) == 0 && (slot != 0 || ctrl == 0)` produces the image's own tag-then-slot-then-ctrl order. 32 -> 79/85, and 81/85 written as the single chained guard rather than two nested ifs.
// LEVER: (2) `volatile` ON THE OVERLAY LATCH was the last four instructions. The image hoists `push edi; push ebx` - MapWin::focus's arguments - ABOVE the `mov dword ptr [eax + 0x23c00], 0` at 0x0051093C, and a volatile store cannot be reordered against them. Dropping it is 81/85 -> 85/85; the store survives because a call it cannot see through follows immediately. Ruled out at the same time: moving the `MapWinTable[0]` load ahead of the clear scores 80/85 either way.
// LEVER: (3) `int slot`, not `size_t`: the loop's back edge is `jl 0x5108de` at 0x00510985, and an unsigned induction variable compiles `jb`. `MapWinTableSlots` is a `size_t`, so the bound needs the cast or the comparison promotes back to unsigned.
// LEVER: (4) the repaint went through `MapWinOriginalDrawMap`, a pointer, and compiled `call dword ptr [...]` where 0x0051095D is a `call rel32`. `MapWin::draw_map` is declared in mapwin.h and defined at the end of mapwin.cpp, so naming it directly costs nothing and emits the E8.
// symbol    ?focus@Console@@QAEHHHH@Z
// size      259 bytes
// prototype void (__thiscall ?focus@Console@@QAEXHHH@Z)(Console* this, int xCoord, int yCoord, int factionID)
// callers   32   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550 0x0046B310 0x005109B0 0x005FD120
Return Value: 1 when the primary map window took the coordinate, 0 otherwise
Status: Complete with temporary Console::cursor_next, MapWin::focus,
        MapWin::draw_map and flush_input original dependencies
Verification note: the catalogued mangled name ?focus@Console@@QAEXHHH@Z says
        void and is wrong. EAX is loaded from the [ebp-4] flag slot at
        0x0051098B and 0x0051099A, and 12 of the 61 call sites branch on it two
        instructions later - 0x004F7D7D, 0x00508838 and 0x0050D606 among them.
        Declared int; do not "fix" it back on the strength of the name.
Verification note: slot 0 is special in FOUR distinct ways, each transcribed at
        the address that decides it.
          1. 0x005108ED / 0x005108EF - slot 0 skips the per-window activity gate
             at 0x1DD74 entirely, the same exemption draw_tile makes.
          2. 0x0051090C / 0x0051090E - an UNTAGGED slot 1..7 is dropped, while
             an untagged slot 0 falls through to 0x00510910 and gets a second
             chance from the control-turn global.
          3. 0x0051091B / 0x0051091D - a TAGGED slot 1..7 goes straight to the
             plain focus call at 0x00510964, while a tagged slot 0 runs the
             console cursor and the survey-overlay latch first.
          4. 0x00510976 / 0x00510978 - a successful focus on slots 1..7 does NOT
             raise the flag. Only slot 0 can set it, so only slot 0 can reach
             flush_input or make this function return 1.
Verification note: the reload of the flag slot at 0x0051099A, after flush_input
        returns, is a register-clobber reload of a local this frame owns.
        flush_input is a cdecl no-argument function and cannot touch it, so the
        reload is not observable and is transcribed as one `return focused;`.
Verification note: the mask is computed once at 0x005108C1-0x005108D5 and then
        reloaded from the [ebp+0x10] argument slot at 0x005108FF on every
        iteration, and `this` is reloaded from [ebp-8] at 0x0051092B. Nothing
        this body calls can write a caller-pushed argument slot or another
        frame's local, so both are register-allocation artifacts and the single
        locals below are faithful. The MapWinTable reloads are NOT in that
        class and are kept - see their own comments.
Verification note: the original has no null check other than the slot test at
        0x005108E5. It does not defend against a null `this`, a null
        ConsoleGlobal, a null seam, or a null MapWinTable[0] at 0x00510946 and
        0x00510955 - the last two genuinely reachable if a callee empties the
        slot mid-iteration, and the original would fault. That absence is
        deliberate and is not repaired here.
*/
int Console::focus(int x_coord, int y_coord, int faction_id) {
    // 0x005108AD `cmp eax, esi` against an already-zeroed ESI, then 0x005108B6
    // `jge`: a negative faction takes 0x60000000 and matches both view tags at
    // once. Otherwise the neg/sbb/and/add idiom at 0x005108C1-0x005108D5 folds
    // one equality into a constant, giving 0x40000000 for a different faction
    // and 0x20000000 for the local one. The global faction is read only on the
    // non-negative path, exactly as the original's branch structure has it.
    // 0x005108B3 `mov [ebp-4], esi` with ESI zero. The literal 1 at 0x0051097A
    // is the ONLY store into that slot, so this is a plain 0/1 flag and the
    // returns are exactly 0 and 1, never a callee's residue.
    int focused = 0;
    uint32_t mask;
    if (faction_id < 0) {
        mask = 0x60000000U;
    } else {
        mask = (faction_id != ConsoleGlobal.faction_id_) ? 0x40000000U : 0x20000000U;
    }
    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        // 0x005108DE `mov eax, [esi*4 + 0x7d3c3c]`. Re-read every iteration,
        // do NOT hoist: cursor_next, MapWin::focus and MapWin::draw_map below
        // can republish the table, and later slots must see that.
        MapWin *const window = MapWinTable[slot];
        // 0x005108E5 test / 0x005108E7 je. The null test runs FIRST, so an
        // empty slot 0 is skipped despite every exemption below.
        if (window == nullptr) {
            continue;
        }
        if (slot != 0) {
            const uint32_t active =
                static_cast<const volatile MapWin *>(window)->field_1DD74_;
            if (active == 0) {
                continue;
            }
        }
        // 0x00510902 reads the window's wide flag dword, one dword ahead of the
        // activity field mapwin.h already names. MapWin owns both members;
        // retain the volatile read on the actual window receiver. 0x00510908
        // `test ecx, edx` is a bitwise AND against the tag bits, not an
        // equality, so a window carrying 0x60000000 matches either mask.
        const uint32_t flags =
            static_cast<const volatile MapWin *>(window)->field_1DD70_;
        // 0x00510908 `test ecx, edx` is ONE test with two consumers: the
        // image branches on the tag FIRST and only then on the slot, which is
        // this chained guard and not an `if (slot == 0)` outer test. Written
        // the other way round the tag has to survive the slot branch, so VC6
        // materialises it (`and`/`setne` plus a second `test`) and the whole
        // tail shifts - 32 of 85 against 81 of 85 here.
        if ((mask & flags) == 0
            && (slot != 0 || ConsoleControlTurnActive == 0)) {
            continue;
        }
        if (slot == 0) {
            // Specialisation 3. cursor_next runs on the process-wide Console at
            // 0x009156B0, NOT on `this`. Every call site happens to enter focus
            // with the same object, but the constant is in the instruction
            // stream and is transcribed as one.
            ConsoleGlobal.cursor_next(x_coord, y_coord);
            // 0x0051092B reloads `this`, 0x0051092E reads the survey-overlay
            // latch. Read AFTER the call, do NOT hoist: this comes from `this`
            // while cursor_next was handed ConsoleGlobal, and cursor_next
            // writes its own object's history fields in this region.
            // NOT `volatile`. The image hoists the two argument pushes for
            // MapWin::focus ABOVE this store (`push edi; push ebx` at
            // 0x0051093A, then the clear at 0x0051093C); a volatile store
            // cannot be reordered against them, which pinned it four
            // instructions early and cost the match at 81 of 85. The store
            // still survives - it is followed by a call VC6 cannot see
            // through - and the read below still cannot be hoisted over
            // cursor_next for the same reason.
            if (survey_mode_ != 0) {
                // 0x0051093C clears the latch BEFORE the table load at
                // 0x00510946 and before the call at 0x0051094C. The clear is
                // ordered against MapWin::focus, not folded into it.
                survey_mode_ = 0;
                // 0x00510946 reads slot 0 by absolute address, with no [esi*4]
                // index, and as a fresh load. Do NOT reuse `window`:
                // cursor_next may have republished the table.
                MapWin *const primary = MapWinTable[0];
                if (primary->MapWin::focus(x_coord, y_coord) != 0) {
                    focused = 1;
                    continue;
                }
                // 0x00510955 reads slot 0 a SECOND time, after MapWin::focus
                // returned. Re-read, do NOT hoist and do not share with the
                // load above. The literal 1 is the draw type.
                MapWin *const repaint = MapWinTable[0];
                repaint->MapWin::draw_map(1);
                continue;
            }
            // Latch already clear (0x00510936 je 0x510964): fall through to the
            // plain focus call, exactly as a tagged slot 1..7 does.
        }
        // 0x00510964 - the third table read this iteration can make. Re-read,
        // do NOT hoist: on the slot-0 path cursor_next has already run between
        // this and the load at 0x005108DE.
        MapWin *const target = MapWinTable[slot];
        // Specialisation 4: a successful focus on slots 1..7 is discarded; only
        // the primary window raises the flag.
        if (target->MapWin::focus(x_coord, y_coord) != 0
            && slot == 0) {
            focused = 1;
        }
    }
    // 0x00510991 test / 0x00510993 je / 0x00510995 call - once, after the whole
    // eight-slot walk, never inside it.
    if (focused != 0) {
        flush_input();
    }
    return focused;
}

// Original 0x005150D0 selects the world-map or an eligible secondary map
// beneath the pointer when passed a negative window index, then adjusts the
// selected windows' signed zoom offsets and redraws the world view.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005150D0 ?zoom@Console@@QAEXHH@Z 0x005150D0-0x005153DF FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005150d0.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
void Console::zoom(int zoom_command, int window_index) {
    int selected_window = window_index;
    if (window_index < 0) {
        selected_window = 0;
        bool primary_map_hit = false;
        int mouse_x;
        int mouse_y;
        WorldClimateWorldWin.get_mouse_pos(&mouse_x, &mouse_y);
        int tile_x = 0;
        int tile_y = 0;
        if (in_box(mouse_x, mouse_y, 0, 0,
                   WorldClimateWorldWin.dib_.bmiHeader.biWidth,
                   -WorldClimateWorldWin.dib_.bmiHeader.biHeight) != 0 &&
            WorldClimateWorldWin.pixel_to_tile(mouse_x, mouse_y,
                                               &tile_x, &tile_y) == 0 &&
            tile_x >= 0 && tile_x < MapLongitudeBounds &&
            tile_y >= 0 && tile_y < MapLatitudeBounds) {
            bool visible = (PlayersData[ConsoleGlobal.faction_id_].flags &
                            0x200U) != 0;
            if (!visible) {
                const uint8_t faction_bit = static_cast<uint8_t>(
                    1U << (ConsoleGlobal.faction_id_ & 0x1f));
                visible = (map_loc(tile_x, tile_y)->visibility & faction_bit) != 0;
                if (!visible) {
                    visible = (GameDataStateStorage.fields_.game_state_ & 0x80U) != 0;
                }
            }
            if (visible) {
                MapWin *const primary = MapWinTable[0];
                primary->field_1DD9C_ = tile_x;
                primary->field_1DDA0_ = tile_y;
                primary_map_hit = true;
            }
        }

        if (!primary_map_hit) {
            for (int slot = 0; slot < static_cast<int>(MapWinTableSlots);
                 ++slot) {
                MapWin *const window = MapWinTable[slot];
                if (window == nullptr ||
                    (slot != 0 && window->field_1DD74_ == 0) ||
                    window->field_1DD98_ >= 0) {
                    continue;
                }
                int window_mouse_x;
                int window_mouse_y;
                window->get_mouse_pos(&window_mouse_x, &window_mouse_y);
                int candidate_x = 0;
                int candidate_y = 0;
                if (in_box(window_mouse_x, window_mouse_y, 0, 0,
                           window->dib_.bmiHeader.biWidth,
                           -window->dib_.bmiHeader.biHeight) == 0 ||
                    window->pixel_to_tile(window_mouse_x, window_mouse_y,
                                          &candidate_x, &candidate_y) != 0 ||
                    candidate_x < 0 || candidate_x >= MapLongitudeBounds ||
                    candidate_y < 0 || candidate_y >= MapLatitudeBounds) {
                    continue;
                }
                bool visible = (PlayersData[ConsoleGlobal.faction_id_].flags &
                                0x200U) != 0;
                if (!visible) {
                    const uint8_t faction_bit = static_cast<uint8_t>(
                        1U << (ConsoleGlobal.faction_id_ & 0x1f));
                    visible = (map_loc(candidate_x, candidate_y)->visibility &
                               faction_bit) != 0;
                    if (!visible) {
                        visible = (GameDataStateStorage.fields_.game_state_ & 0x80U) != 0;
                    }
                }
                if (!visible) {
                    continue;
                }
                window->field_1DD9C_ = candidate_x;
                window->field_1DDA0_ = candidate_y;
                selected_window = slot;
                break;
            }
        }
    }

    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == nullptr ||
            (slot != 0 && window->field_1DD74_ == 0) ||
            (selected_window >= 0 && slot != selected_window)) {
            continue;
        }
        switch (zoom_command) {
        case 0x203:
            if (window->field_1DD98_ < 0x10) {
                window->field_1DD98_ += 2;
            }
            break;
        case 0x204:
            if (window->field_1DD98_ > -0xE) {
                window->field_1DD98_ -= 2;
            }
            break;
        case 0x205:
            window->field_1DD98_ = 0;
            break;
        case 0x206:
            window->field_1DD98_ = -6;
            break;
        case 0x207:
            window->field_1DD98_ = 0x10;
            break;
        case 0x208:
            window->field_1DD98_ = -0xE;
            break;
        default:
            break;
        }
        window->draw_map(1);
    }
    WorldClimateWorldWin.redraw();
}


/*
Purpose: Drain the keyboard queue, clear the two pointer-owner slots and let
         the network layer poll, then do the same for the mouse queue - each
         PeekMessage loop PEELED, one check outside before the do-while, as
         the image has it.
// ORIGINAL: 0x005FD120 ?flush_input@@YAXXZ 0x005FD120-0x005FD1BA BYTE_EXACT
// size      154 bytes
// prototype void (__cdecl ?flush_input@@YAXXZ)()
// callers   17   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062D5D0
// indirect  0x005FD13F 0x005FD157 0x005FD180 0x005FD198
Return Value: n/a
Status: Complete
*/
void __cdecl flush_input() {
    MSG msg;
    if (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
        do {
        } while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE));
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
    if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
        do {
        } while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
                             PM_REMOVE));
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
    check_net();
}

/*
Purpose: Record that the console was closed from the system menu, by raising
         the flag the turn loop watches.
// ORIGINAL: 0x0051D7C0 ?on_sys_close@Console@@QAEXXZ 0x0051D7C0-0x0051D7CB BYTE_EXACT
// symbol    ?on_sys_close@Console@@UAEXXZ
// size      11 bytes
// prototype void (__thiscall ?on_sys_close@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Console::on_sys_close() {
    ExitTurnLoop = 1;
}

/*
// ORIGINAL: 0x004E0FA0 ?editor_climate@Console@@QAEXXZ 0x004E0FA0-0x004E0FC3 BYTE_EXACT
// size      35 bytes
// prototype void (__thiscall ?editor_climate@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0058C2A0 0x005ABF20 0x005C5A30
Status: Complete
*/
void Console::editor_climate() {
    auto_undo();
    if (!custom_planet(0, 0)) {
        world_climate();
        // QUALIFIED: `Console : MapWin` inherits `MapWin::draw_map`, so the
        // unqualified name binds to the member and compiles a `__thiscall`
        // where the image has a `__cdecl` call to the FREE draw_map at
        // 0x0046B190. The `::` is what makes the two the same call.
        ::draw_map(1);
    }
}

/*
// ORIGINAL: 0x004E1F30 ?editor_redo@Console@@QAEXXZ 0x004E1F30-0x004E1F39 BYTE_EXACT
// size      9 bytes
// prototype void (__thiscall ?editor_redo@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005ABE40
Status: Complete
*/
void Console::editor_redo() {
    load_undo(-1);
}

/*
// ORIGINAL: 0x0051D740 ?menu_update@Console@@QAEXXZ 0x0051D740-0x0051D753 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?menu_update@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00460DD0
Status: Complete
*/
void Console::menu_update() {
    // alpha_menu_ IS the object the image calls 0x00460DD0 on - the member
    // form replaces the MainMenu pun this body used to carry, and measured
    // BYTE_EXACT unchanged (2026-09-04).
    alpha_menu_.check(selected_vehicle_);
}

/*
// ORIGINAL: 0x004E0F80 ?editor_polar@Console@@QAEXXZ 0x004E0F80-0x004E0F98 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?editor_polar@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x005C58E0 0x005C5A30
Status: Complete
*/
void Console::editor_polar() {
    auto_undo();
    world_polar_caps();
    world_climate();
    ::draw_map(1);   // the FREE draw_map; see Console::editor_climate above
}

/*
Purpose: Open the diplomacy editor. A modal list popup asks which faction to
         edit - one item per playable faction, preselected to the faction the
         human is playing - and the answer builds a stack DipEdit, whose
         construction the image has no ??0DipEdit for: every member
         constructor is inlined here, then init() runs and the scope-end
         teardown destroys it.
// ORIGINAL: 0x004DB870 ?editor_diplomacy@Console@@QAEXXZ 0x004DB870-0x004DBB40;0x0065B469-0x0065B55F
// size      966 bytes
// prototype void (__thiscall ?editor_diplomacy@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   22
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004064D0 0x00406880 0x00406E60 0x004080B0 0x0043EF70 0x004837F0 0x004DB080 0x004E2240 0x005D4CF0 0x005D4DD0 0x00602600 0x00607040 0x00607CF0 0x00607DA0 0x00609DB0 0x0060E670 0x00612A70 0x00618EA0 0x00618EE0 0x00645550
// LEVER: THE WHOLE BODY IS ONE STACK DipEdit PLUS ONE STACK Popup. `DipEdit
//   edit;` and `Popup popup;` are what produce the image's 22 calls - the
//   compiler inlines DipEdit's empty constructor and runs GraphicWin, Font,
//   ListBox, CheckBox and the five FlatButtons itself, then re-stores
//   DipEdit's own two vptrs (0x66e964/0x66e95c, slot 0 being ??_GDipEdit) at
//   the tail, exactly where the image has them. The two frame locals are
//   also why the frame is 0xacc4.
// LEVER: THE POPUP'S SCOPE ENDS BEFORE THE BRANCH. Spelled as one flat
//   function, our compiler ran the popup teardown after `edit.init()` where
//   the image destroys it immediately after exec returns; nesting the popup
//   in a block that closes on the result moved the teardown to the image's
//   site and took the body from 50/157 to 58/157.
// LEVER: ListBox's constructor had to leave the header for this body - see
//   listbox.h. An inline-visible `ListBox() { ; }` made VC6 expand the
//   virtual-base stage (25 instructions, and a four-byte frame) at this one
//   site where the image makes a single `call ??0ListBox@@QAE@H@Z`.
//   11/157 -> 50/157 on that edit alone.
// CEILING, NOT A SPELLING: the image expands ~Popup INLINE at the scope end
//   - `mov [popup],0x6695c8 / mov [popup+0x444],0x6695c0 / call close /
//   call ~Scroll / call ~BasePop` - because its ~Popup is visible there.
//   This tree's ~Popup is a pending_bodies forwarder, so we emit one `call
//   ??1Popup`. Closing it needs ~Popup/~Scroll/~BasePop defined inline in
//   their headers, which moves ??1TutWin/??1PickWin and every other Popup
//   destroyer; see the same note on DipEdit::on_button_clicked.
// CEILING, NOT A SPELLING: the image's `result < 0` early exit re-expands
//   ~DipEdit member by member - both ~FlatButton expansions first, then
//   out-of-line ??1FlatButton for the remaining three, then the CheckBox,
//   ListBox, Font and GraphicWin teardowns, ~40 instructions - while the
//   success exit makes a single `call ??1DipEdit`. ~DipEdit is defined in
//   dipedit.cpp, not visible here, so our compiler emits one call and MERGES
//   the two exits into a shared tail. Both are the image's inliner choosing
//   a different depth per site; no source form reaches it.
// TRIED: the frame slots. The image puts the popup BELOW the DipEdit
//   (popup at ebp-0xacd0, edit at ebp-0x5954); ours lays them the other way
//   round. Measured twice on 20-line repros: this compiler gives the
//   FIRST-declared local the lowest slot, so the image's layout implies the
//   popup was declared first there, and its construction order (DipEdit
//   first, 0x004DB894 before 0x004DB947) then contradicts C++ declaration
//   order. Declaring the popup first here moves its constructor call to the
//   top of the body and misaligns the whole listing, which is far worse
//   than ~30 ebp-relative displacements. Not chased further.
*/
void Console::editor_diplomacy() {
    DipEdit edit;

    // Read before the popup exists: the image caches the faction in a local
    // across the whole item loop and the two popup calls, which cannot be
    // the global re-read - either of those calls could store through its
    // char* argument.
    int faction = ConsoleGlobal.faction_id_;

    int result;
    {
        Popup popup;

        PopupModalState = 6;
        popup.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40);

        // Players[0] is the native-life faction and is not offered; the loop
        // is over the seven playable ones, each item text the faction noun.
        for (int i = 1; i < MaxPlayerNum; i++) {
            GenderDefault = Players[i].noun_gender;
            PluralityDefault = Players[i].is_noun_plural;
            popup.dialogs().item(Players[i].noun_faction, i);
        }

        // BasePop's default answer: when the player accepts without picking,
        // the popup hands back the faction already being played.
        if (faction >= 0) {
            popup.set_value(faction);
        }

        result = popup.exec(0, 0);
        PopupModalState = 0;
    }

    if (result < 0) {
        return;
    }

    edit.init(result);
}

/*
Purpose: Reset the console - clear its scalar state and history slot zero,
         then close the Buffer, Time and GraphicWin
         subobjects and clear the map.
The historical forwarding notes below predate the separate timer lifecycle TU;
the timer call now uses the existing Time member directly.
// ORIGINAL: 0x0051D7D0 ?close@Console@@QAEXXZ 0x0051D7D0-0x0051D88E BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0051d7d0.cpp, whose claim proved the ARTIFACT reproduced while the program held a pending_bodies forwarder. `void close()` was not declared on Console at all - that, not the body, is what kept it in an artifact.
// LEVER (2026-09-03): time_close_call, not `->close()` on the Time at
//        0x22a04. The image CALLS 0x00616780 here (0x0051D873) where
//        time.h's in-class `MEASURED close` otherwise expands at this site
//        too - 68 compiled instructions against the image's 40, 22/40
//        agreeing, the whole extra block being Time's own teardown written
//        out. Buffer::close and GraphicWin::close needed nothing: both are
//        declared out-of-line in their headers, so both were already calls.
// symbol    ?close@Console@@QAEXXZ
// size      190 bytes
// kind      game
Return Value: n/a
Status: Complete
*/
void Console::close() {
    char *self = reinterpret_cast<char *>(this);
    field_23BC8_ = 0xffffffff;
    cursor_active_ = 0;
    field_23BE4_ = 0;
    field_23BE8_ = 0;
    field_23BF0_ = 0;
    field_23BF4_ = 0;
    survey_mode_ = 0;
    field_23C04_ = 0;
    field_23BFC_ = 0;
    field_23BF8_ = 0;
    cursor_history_index_ = 0;
    // 0x0051D821..0x0051D83C reloads this same index on every iteration.
    // Slots 1..31 and the history head survive close().
    int count = 0x20;
    do {
        count -= 1;
        int32_t idx1 = cursor_history_index_;
        cursor_history_x_[idx1] = 0;
        int32_t idx2 = cursor_history_index_;
        cursor_history_y_[idx2] = 0;
    } while (count != 0);
    field_23D10_ = 0;
    field_23D18_ = 0;
    field_23D88_ = 0;
    field_23D80_ = 0;
    field_23D8C_ = 0;
    field_23D84_ = 0;
    reinterpret_cast<Buffer *>(self + 0x2247c)->close();
    time_.close();
    reinterpret_cast<GraphicWin *>(self + 0x22a2c)->close();
    clear(0);
}

// ====================

// 0x009156B0 - the bare declaration is what the image has: its ??__E
// dynamic initialiser pushes the hidden most-derived flag (push 1 at
// 0x0050E852) and the compiler emits that push for a most-derived global
// on its own. The old explicit `ConsoleGlobal(1)` was the flag spelled as
// a real int - the exact doubling Console(int) carried.
// Absolute cursor loads at 0x00510A01..0x00510A19 address this object's
// history members. The following stores instead use the method receiver.
// RECOVERY_OBJECT: v1 0x009156B0 ?ConsoleGlobal@@3VConsole@@A
Console ConsoleGlobal;
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (image bytes: ??__E at 0x0050E980 builds the ConsoleTimer object at
// 0x00939E88, the one at 0x0050E9B0 builds the other five - Blink 0x915688,
// Blink2 0x939EB0, Line 0x915658, Go 0x939E60, Turn 0x915628, each
// `mov ecx, <addr>; call Time::Time` - and registers their teardowns;
// winedbg-confirmed walker return 0x00644EEB, table at 0x682568..). Here
// the same recovered constructors run through this build's own startup,
// and the matching destructors close them at exit. Declarations in time.h
// beside their users.
// Keep the Console timer lifecycles in their original domain order.
// RECOVERY_OBJECT: v1 0x00939E88 ?ConsoleTimer@@3VTime@@A
Time ConsoleTimer; // 0x00939E88
// RECOVERY_OBJECT: v2 0x00915658 0x00915680 ?LineTimer@@3VTime@@A 0x0050F440
// RECOVERY_OBJECT: v2 0x00915688 0x009156B0 ?BlinkTimer@@3VTime@@A 0x0050F440
// RECOVERY_OBJECT: v2 0x00939EB0 0x00939ED8 ?Blink2Timer@@3VTime@@A 0x0050F440
Time BlinkTimer,   // 0x00915688
     Blink2Timer,  // 0x00939EB0
     LineTimer,    // 0x00915658
     GoTimer,      // 0x00939E60
     TurnTimer;    // 0x00915628

// Compiler-generated source-owned lifecycle entry; no handwritten wrapper.
/*
// ORIGINAL: 0x0050E9B0 sub_50e9b0 0x0050E9B0-0x0050E9EE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bcad94a018ff94737abb85df10d24ca7aa405b1a:src/recovered/0050e9b0.cpp", "pinned:terranx_original lifecycle0x50e9b0", "historical source owner before rehome:src/time.cpp"], "reason": "Generated complete five-Time-object initializer composes the real constructor group and destructor registration. Delegated generated helpers are current lowering, not a claim of one-to-one byte layout. Preserve complete original spans, catalogue names, claims and historical lessons; current compiler emission is not a fresh grant or linked selection proof."}
// symbol    _$E21
*/

// Historical seed finding: Native construction order: Blink, Blink2, Line, Go, Turn; registers the aggregate reverse teardown.
// Historical seed finding: ConsoleTimer startup50E980 is a separate original group and is not annexed by this binding.

// Compiler-generated source-owned lifecycle entry; no handwritten wrapper.
/*
// ORIGINAL: 0x0050E9F0 sub_50e9f0 0x0050E9F0-0x0050EA22 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bcad94a018ff94737abb85df10d24ca7aa405b1a:src/recovered/0050e9f0.cpp", "pinned:terranx_original lifecycle0x50e9f0", "historical source owner before rehome:src/time.cpp"], "reason": "Generated destructor for the single five-Time-object declaration preserves native reverse destruction and timer shutdown effects. Preserve complete original spans, catalogue names, claims and historical lessons; current compiler emission is not a fresh grant or linked selection proof."}
// symbol    _$E19
*/

// Historical seed finding: Native destruction order: Turn, Go, Line, Blink2, Blink.
// Historical seed finding: The former separate per-object declarations did not own the complete original aggregate.
int Console::game_data_private_word(FILE *file, int write_mode) {
    if (write_mode != 0) {
        field_23D20_[0] = 0;
        field_23D20_[1] = 0;
        field_23D20_[2] = 0;
        field_23D20_[3] = 0;
    }
    return game_data_transfer_block(file, field_23D20_, sizeof(field_23D20_));
}


// Shared four-field rectangle written by original Console51D760/51D890.
// Start binding only: no complete original allocation/lifetime claim.
// RECOVERY_OBJECT: v1 0x00939EE8 ?ConsoleMenuBounds@@3UtagRECT@@A
RECT ConsoleMenuBounds;

// The original order at 0x004D7BE0 checks the rover's plan, chassis and
// current terrain before placing a road cursor.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D7BE0 ?road_to@Console@@QAEXHH@Z 0x004D7BE0-0x004D7CED
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d7be0.cpp"], "reason": "Seed 004d7be0 identifies the Console road cursor order and extent; typed vehicle, prototype, terrain and technology owners preserve the sequence."}
void Console::road_to(int veh_id, int cursor_mode) {
    const Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    if (prototype.plan != 9) {
        NetMsgPrimary.pop("NEEDTERRAFORMERS", 5000, 0, 0);
        return;
    }
    if (Chassis[prototype.chassis_id].triad != TRIAD_LAND) {
        NetMsgPrimary.pop("NOTINTRIAD", 5000, 0, 0);
        return;
    }
    const Map &tile = map_tiles()[vehicle.y * MapLongitude + (vehicle.x >> 1)];
    if ((tile.climate & ~0x1F) < 0x60) {
        NetMsgPrimary.pop("NOTINOCEAN", 5000, 0, 0);
        return;
    }
    if (cursor_mode != 0x1C ||
        has_tech(Terraforming[TERRA_FARM].preq_tech, vehicle.faction_id)) {
        ConsoleGlobal.cursor_on((cursor_mode == 0x1C) + 2, -1);
    }
}

// Advance the view history after clearing the transient survey overlay.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00510B70 ?next_cursor@Console@@QAEXXZ 0x00510B70-0x00510C89 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00510b70.cpp"], "reason": "Seed 00510b70 and adjacent recovered prev_cursor establish the next history transition on Console cursor fields."}
void Console::next_cursor() {
    field_23BF8_ = 0;
    if (survey_mode_ != 0) {
        survey_mode_ = 0;
        console_map_win()->draw_map(1);
    }
    console_map_win()->draw_cursor();
    if (ConsoleGlobal.cursor_active_ != 0) {
        ambience(0x1E);
        cursor_active_ = 0;
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        ::draw_cursor();
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        console_map_win()->main_caption();
        alpha_menu_.check(selected_vehicle_);
    }
    if (cursor_history_index_ != cursor_history_head_) {
        cursor_history_index_ = (cursor_history_index_ + 1) % 32;
        const int slot = ConsoleGlobal.cursor_history_index_;
        ConsoleStatusWin.set_loc(ConsoleGlobal.cursor_history_x_[slot],
                                 ConsoleGlobal.cursor_history_y_[slot]);
        const int slot_now = ConsoleGlobal.cursor_history_index_;
        console_map_win()->focus(ConsoleGlobal.cursor_history_x_[slot_now],
                                 ConsoleGlobal.cursor_history_y_[slot_now]);
    }
}

// Assign an automated order while holding the vehicle's network lock.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D8A70 ?automate@Console@@QAEXHH@Z 0x004D8A70-0x004D8CC9
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d8a70.cpp"], "reason": "Seed 004d8a70 identifies the automation order and extent; typed Veh, prototype, chassis, terrain and NetDaemon owners match its accessed records."}
void Console::automate(int veh_id, int order) {
    Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    const Map &tile = map_tiles()[vehicle.y * MapLongitude + (vehicle.x >> 1)];
    bool direct_lock = false;
    if (prototype.plan == 9) {
        if (order == 1 || order == 2) {
            if (chassis.triad != TRIAD_LAND || (tile.climate & 0xE0) < 0x60) {
                return;
            }
        }
        if (order == 2) {
            if (!has_tech(Terraforming[TERRA_FARM].preq_tech,
                          vehicle.faction_id)) {
                return;
            }
            direct_lock = true;
        }
        if (!direct_lock && (order == 4 || order == 5) &&
            (tile.bit & 1) != 0 && (tile.val2 & 0x0F) < 8) {
            return;
        }
    }
    if (!direct_lock && order == 10) {
        if (chassis.triad != TRIAD_AIR ||
            Weapon[prototype.weapon_id].offense_rating == 0) {
            return;
        }
        vehicle.order = 0;
        ConsoleGlobal.cursor_on(0xB, -1);
        return;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return;
    }
    if (order == 0xB) {
        vehicle.waypoint_x[1] = vehicle.x;
        vehicle.waypoint_y[1] = vehicle.y;
        if (chassis.range != 0) {
            action_home(veh_id, 0);
            if (vehicle.order == 0x18) {
                vehicle.waypoint_x[1] = vehicle.waypoint_x[0];
                vehicle.waypoint_y[1] = vehicle.waypoint_y[0];
            }
            vehicle.order = 0;
        }
    }
    vehicle.order_auto_type = static_cast<uint8_t>(order);
    vehicle.order = 0;
    vehicle.state = (vehicle.state & 0xFCFFFFFF) | 0x200;
    vehicle.waypoint_count = 0;
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
}

// A convoy either opens the supply panel at its base or chooses an extraction
// resource at its current site. The vehicle lock covers both interactions.
// Resource labels are RulesResource members. The original also sets an
// unowned net action slot at 0x0093E904 before opening supply_options;
// that effect still requires its owner.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D8FA0 ?convoy@Console@@QAEXH@Z 0x004D8FA0-0x004D916D
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d8fa0.cpp", "source:src/alpha.h"], "reason": "Seed 004d8fa0 identifies the Console convoy order and lock lifecycle; typed Veh, Base and RulesResource records replace receiver and table offsets. The resource label run at 0x0094615C is Resource[0].name_plural, with the singular member selected for a yield of one. The net action slot at 0x0093E904 remains an external ownership discovery; this binding does not claim that effect."}
void Console::convoy(int veh_id) {
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return;
    }
    Veh &vehicle = Vehs[veh_id];
    const int base_id = base_at(vehicle.x, vehicle.y);
    if (base_id >= 0) {
        supply_options(veh_id, base_id);
        NetDaemonNet.unlock_veh();
        return;
    }
    if (vehicle.home_base_id >= 0) {
        if (can_convoy(vehicle.x, vehicle.y) == 0) {
            NetMsgPrimary.pop("CANTCONVOY", 5000, 0, 0);
            NetDaemonNet.unlock_veh();
            return;
        }
        for (int resource = 0; resource < 3; ++resource) {
            const int amount = resource_yield(resource, faction_id_,
                                              vehicle.home_base_id,
                                              vehicle.x, vehicle.y);
            parse_num(resource, amount);
            const char *name = amount == 1
                ? Resource[resource].name_singular
                : Resource[resource].name_plural;
            parse_say(resource, reinterpret_cast<int>(name), -1, -1);
        }
        parse_says(3, Bases[vehicle.home_base_id].name_string, -1, -1);
        const int choice = x_pop_short("SUPPLYEXTRACT", 0x40, 0);
        if (choice >= 0) {
            vehicle.order = 3;
            vehicle.order_auto_type = static_cast<uint8_t>(choice);
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
        }
    }
    NetDaemonNet.unlock_veh();
}

// ORIGINAL: 0x00515740 ?toggle_dest@Console@@QAEXXZ 0x00515740-0x00515758 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?toggle_dest@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190
// 0x00515740  ?toggle_dest@Console@@QAEXXZ  ->  ?toggle_dest@Console@@QAEXXZ
//
// Purpose: flip the "draw vehicle destinations" bit on the live map window
//          and repaint it.


// LEVER: ::draw_map, not draw_map. The image calls the FREE cdecl draw_map
//          (0x0046B190, ?draw_map@@YAXH@Z), which cleans its own argument -
//          the unqualified spelling resolves to the MapWin member this class
//          inherits, and that thiscall leaves the cleanup to the callee, which
//          drops the image's trailing `pop ecx` (4 of 6).
void Console::toggle_dest() {
    console_map_win()->field_1DD70_ ^= 0x40000;
    ::draw_map(1);
}

// ORIGINAL: 0x004D2AC0 ?disband@Console@@QAEXH@Z 0x004D2AC0-0x004D2DDC FILE
// RECOVERY_BINDING: v1 {"reason":"Recover the disband order through typed Console, vehicle, base, tutorial-window and NetDaemon owners; pinned branches establish editor/network refusal, no-base dialog selection, custom-prototype refunds and queue bookkeeping.","evidence":["image:0x004D2AC0-0x004D2DDC","source:src/veh.cpp:action_destruct","source:src/base.h","source:src/tutwin.h","recovery/tasks/console-owner-coverage.json"]}
// size      796 bytes
// prototype void (__thiscall ?disband@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   5   call targets   14
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A7A0 0x0047A890 0x0048C0A0 0x004CD2F0 0x004E3A50 0x004E4430 0x00531020 0x005310F0 0x00531300 0x00592E10 0x00593250 0x005C0B00 0x00625EC0 0x00645470
// Foreign vehicles require the offline scenario-editor flag; multiplayer
// foreign vehicles are refused even with that flag. No-base dialogs append
// the DISBAND2 suffix. Custom prototypes bypass the basic-unit weapon filter.


void Console::disband(int veh_id) {
    if ((Vehs[veh_id].flags & 0x20) && !(GameDataStateStorage.fields_.game_state_ & 0x40)) {
        NetMsgPrimary.pop("DISBANDOBJ", 5000, 0, nullptr);
        return;
    }

    if (NetDaemonNet.lock_veh(&veh_id, 4, -1, -1, 0)) return;

    Veh &vehicle = Vehs[veh_id];
    if (vehicle.faction_id != faction_id_ &&
        (IsMultiplayerNet || !(GameDataStateStorage.fields_.game_state_ & 0x80))) {
        NetDaemonNet.unlock_veh();
        return;
    }

    const int x = vehicle.x;
    const int y = vehicle.y;
    field_23BE4_ = 0;
    const int base_id = base_at(x, y);
    const int display_proto_id = vehicle.proto_id;
    parse_says(0, VehPrototypes[display_proto_id].veh_name, -1, -1);
    StringTemp[0] = 0;
    strcat(StringTemp, "DISBAND");
    if (base_id < 0) strcat(StringTemp, "2");

    const int console_faction = faction_id_;
    const bool alien = (Players[console_faction].rule_flags & 0x80) != 0;
    const int triad = Chassis[VehPrototypes[display_proto_id].chassis_id].triad;
    const char *picture;
    if (alien) {
        picture = triad == 2 ? "alair_sm.pcx"
            : triad == 1 ? "navun_sm.pcx" : "alunit_sm.pcx";
    } else {
        picture = triad == 2 ? "air_sm.pcx"
            : triad == 1 ? "navun_sm.pcx" : "unit_sm.pcx";
    }

    const int answer = popp(ScriptPopupFile, StringTemp, 0, picture, nullptr);
    if (!answer) {
        NetDaemonNet.unlock_veh();
        return;
    }

    if (base_id >= 0) {
        const int refund_proto_id = Vehs[veh_id].proto_id;
        if ((refund_proto_id >= 0x40 ||
             (Weapon[VehPrototypes[refund_proto_id].weapon_id].offense_rating >= 0 &&
              refund_proto_id != 15)) && answer != 2) {
            const int faction = Vehs[veh_id].faction_id;
            const int factor = cost_factor(faction, 1, -1);
            const int cost = VehPrototypes[Vehs[veh_id].proto_id].cost;
            const int refund = factor * cost / 2;
            Bases[base_id].minerals_accumulated += refund;
            Bases[base_id].minerals_accumulated_2 =
                Bases[base_id].minerals_accumulated;
            Bases[base_id].production_id_last = Bases[base_id].queue_production_id[0];
            synch_base(base_id);
            TutWinBaseWindow.check_base(base_id);
        }
    }

    if (IsMultiplayerNet) {
        message_veh(0x2407, veh_id, answer, 0);
        NetDaemonNet.await_exec(1);
    } else if (answer == 2) {
        action_destruct(veh_id);
    } else {
        kill(veh_id);
    }
}

// .rdata strings these bodies pass to X_pop. The image names them by
// address; the tree names them by their text, read straight out of
// .rdata (0x0068A9CC "MENUSIMPLE", 0x00688F98 "RESETALLFAC").
static const char *const StrMenuSimple = (const char *)0x0068A9CC;
static const char *const StrResetAllFac = (const char *)0x00688F98;

// THE PER-FACTION DWORD RUN at 0x00946138 - game.h binds that address as
// TectonicDetonationCount, and editor_reset_faction is the reader that shows
// it is a RUN of eight, one per faction slot: it walks factions 1..7 and
// clears exactly one dword per pass, starting at 0x0094613C and stopping on
// the exclusive 0x00946158 where RulesResource Resource[] begins. The run
// sits between Citizen[9] and Resource[0], so it is not part of either array.

// .rdata strings, read out of the image: 0x00688FA4 is "RESETALLTECH", the
// confirmation X_pop asks before editor_reset_tech throws every faction's
// research away.
static const char *const StrResetAllTech = (const char *)0x00688FA4;

// .rdata strings the editor commands pass to x_pop_short / X_pop, read out of
// the image. The image PUSHES each address as an immediate, so the binding has
// to keep the image's address rather than a tree string literal.
static const char *const EditorRockinessLabel = (const char *)0x00689174;    // "ROCKOSITY"
static const char *const StrEditorMinor = (const char *)0x00689120;  // "EDITORMINOR"
static const char *const StrMapFlat = (const char *)0x00689148;      // "MAPFLAT"
static const char *const StrMapRound = (const char *)0x00689150;     // "MAPROUND"
static const char *const StrEditorYear = (const char *)0x006890F0;   // "EDITORYEAR"
static const char *const StrMenuDetail = (const char *)0x0068A9C0;   // "MENUDETAIL"
static const char StrScenarioObjectivesPopup[] = "THEYFAILEDFLECHETTE";
static char StrScenarioObjectivesTitle[] = "FLECHETTE";
static const char *const StrFungosity = (const char *)0x0068915C;    // "FUNGOSITY"
static const char *const StrFungmotize = (const char *)0x00689168;   // "FUNGMOTIZE"
static const char *const StrProtectOff = (const char *)0x00689198;   // "PROTECTOFF"
static const char *const StrProtectOn = (const char *)0x006891A4;    // "PROTECTON"
static const char *const StrTerraSurvey2 = (const char *)0x0068A8E0; // "TERRASURVEY2"
static const char *const StrTechChange = (const char *)0x0068A8D4;   // "TECHCHANGE"
static const char *const StrAcLogoSm = (const char *)0x0068AA7C;     // "ac_logo_sm.pcx"
static const char *const StrVersionControlX = (const char *)0x0068AA8C;  // "VERSIONCONTROLX"
static const char *const StrDesignated = (const char *)0x00688E44;   // "DESIGNATED"
static const char *const StrUndesignated = (const char *)0x00688E50; // "UNDESIGNATED"
// show_version reads two more .data slots that hold POINTERS to strings:
// 0x00691870 -> "RELEASE 2.0" and 0x00691874 -> "20-Dec-1999".
static int *const VersionReleaseTextSlot = (int *)0x00691870;
static int *const VersionBuildDateSlot = (int *)0x00691874;

typedef int (__cdecl *NullCallback)(void);

// .rdata strings the newly homed bodies hand to the pop helpers, read out of
// the image exactly like the sibling bindings above: the image PUSHES each
// address as an immediate, so the binding keeps the image's address and the
// literal is the comment.
static const char *const StrEditorEnergy = (const char *)0x006890E0;  // "EDITORENERGY"
static const char *const StrBrush = (const char *)0x00689140;         // "BRUSH"
static const char *const StrTechRandom = (const char *)0x0068A8C8;    // "TECHRANDOM"

// .rdata labels the order handlers raise through NetMsgPrimary, read out of the
// image. SIMULNO and UNITMOVED each exist twice - the image pushes two
// distinct .rdata copies of the same text from different guards, so each
// address keeps its own binding.
static const char *const StrSimulNo = (const char *)0x00688C68;        // "SIMULNO"
static const char *const StrUnitMoved = (const char *)0x00688C70;      // "UNITMOVED"
static const char *const StrSimulNo2 = (const char *)0x00688C7C;       // "SIMULNO"
static const char *const StrUnitMoved2 = (const char *)0x00688C84;     // "UNITMOVED"
static const char *const StrOnlyCombat = (const char *)0x00688C90;     // "ONLYCOMBAT"
static const char *const StrNotBase = (const char *)0x00688C9C;        // "NOTBASE"
static const char *const StrNotInTriad = (const char *)0x00688CA4;     // "NOTINTRIAD"
static const char *const StrDestroyBomb = (const char *)0x00688CB0;    // "DESTROYBOMB"
static const char *const StrDestroy = (const char *)0x00688CBC;        // "DESTROY"
static const char *const StrNotHere = (const char *)0x00688CC4;        // "NOTHERE"
static const char *const StrNoBomb = (const char *)0x00688EC0;         // "NOBOMB"
static const char *const StrScenParam = (const char *)0x00688FF4;      // "SCENPARAM"
static const char *const StrEditorWhoListTitle = (const char *)0x00688F6C;  // "EDITORWHO"
static const char *const StrCustomNames = (const char *)0x00689000;    // "CUSTOMNAMES"
static const char *const StrHostTime = (const char *)0x0068A93C;       // "HOSTTIME"
static const char *const StrCustomTime = (const char *)0x0068A948;     // "CUSTOMTIME"
// ORIGINAL: 0x004D0450 ?skip@Console@@QAEXH@Z 0x004D0450-0x004D0494 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004D0450-0x004D0494", "source:src/netdaemon.h", "source:src/veh.h", "source:src/game.h", "source:src/console.h"], "reason": "Home the skip order through the typed vehicle-ready predicate, turn guard and NetDaemon order API, and the already named Console skip-state member; remove the seed's receiver/table pointer arithmetic."}
// The seed's blocked callee at 0x00531780 is now NetDaemon::order_veh; its
// not_my_turn seed warning remains relevant because that predicate is inline
// here while the original contains a call at 0x0052DC70.
void Console::skip(int veh_id) {
    if (veh_ready(veh_id) != 0 && not_my_turn() == 0) {
        NetDaemonNet.order_veh(veh_id, -1, 0);
        return;
    }
    field_23BE4_ = 0;
}

// ORIGINAL: 0x004D8DC0 ?set_road_to@Console@@QAEXHHHH@Z 0x004D8DC0-0x004D8F37
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004D8DC0-0x004D8F37", "source:src/veh.h", "source:src/map.h", "source:src/netdaemon.h"], "reason": "Represent the road target in Veh waypoint members and compare typed Map region values; use the existing map dimensions and NetDaemon vehicle-lock lifecycle rather than seed globals and byte offsets."}
void Console::set_road_to(int veh_id, int x_coord, int y_coord, int order) {
    if (y_coord < 0 || y_coord >= MapLatitudeBounds ||
        x_coord < 0 || x_coord >= MapLongitudeBounds) {
        return;
    }

    Veh &vehicle = Vehs[veh_id];
    int distance_x = abs(x_coord - vehicle.x);
    if ((MapIsFlat & 1) == 0 && distance_x > static_cast<int>(MapLongitude)) {
        distance_x = MapLongitudeBounds - distance_x;
    }
    const int distance_y = abs(y_coord - vehicle.y);
    if (((distance_y + distance_x) & ~1) < 2) {
        return;
    }

    const int target_index = MapLongitude * y_coord + (x_coord >> 1);
    const int origin_index = MapLongitude * vehicle.y + (vehicle.x >> 1);
    if ((map_tiles()[target_index].climate & ~0x1F) < 0x60) {
        NetMsgPrimary.pop(StrNotHere, 5000, 0, 0);
        return;
    }
    if (map_tiles()[target_index].region != map_tiles()[origin_index].region) {
        return;
    }

    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return;
    }
    Vehs[veh_id].order = static_cast<int8_t>(order);
    Vehs[veh_id].waypoint_x[0] = static_cast<int16_t>(x_coord);
    Vehs[veh_id].waypoint_y[0] = static_cast<int16_t>(y_coord);
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
}

// ORIGINAL: 0x004D24C0 ?hold@Console@@QAEXHH@Z 0x004D24C0-0x004D2697
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004D24C0-0x004D2697", "source:src/veh.h", "source:src/map.h", "source:src/netdaemon.h", "source:src/basewin.h"], "reason": "Recover the hold order with Veh stack/waypoint state, typed map terrain and region bits, RulesChassis traits, and NetDaemon synchronization; base and tile refreshes use their product owners."}
void Console::hold(int veh_id, int waypoint_y) {
    int lock_vehicle = veh_id;
    if (NetDaemonNet.lock_veh(&lock_vehicle, 0, -1, -1, 0) != 0) {
        return;
    }

    const Veh &vehicle = Vehs[veh_id];
    const int x_coord = vehicle.x;
    const int y_coord = vehicle.y;
    const int tile_index = MapLongitude * y_coord + (x_coord >> 1);
    const Map &tile = map_tiles()[tile_index];
    const RulesChassis &chassis = Chassis[VehPrototypes[vehicle.proto_id].chassis_id];

    bool should_hold = (tile.climate & 0xE0) >= 0x60 || chassis.triad != 0;
    if (!should_hold) {
        if ((tile.bit & 1) == 0 || (tile.val2 & 0x0F) < 8) {
            NetDaemonNet.unlock_veh();
            return;
        }
    }

    if (chassis.triad == TRIAD_AIR && chassis.range != 0) {
        if ((tile.bit & 1) == 0 || (tile.val2 & 0x0F) >= 8) {
            if ((tile.bit & 0x40000) == 0) {
                NetDaemonNet.unlock_veh();
                return;
            }
        }
    }

    Vehs[veh_id].waypoint_y[0] = static_cast<int16_t>(waypoint_y);
    Vehs[veh_id].order = ORDER_HOLD;
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
    TutWinBaseWindow.check_veh(veh_id);
    draw_tile(x_coord, y_coord, 1);

    if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_ALIEN_ARTIFACT) {
        return;
    }
    int owner = -1;
    if (map_tiles()[tile_index].bit & 1) {
        owner = map_tiles()[tile_index].val2 & 0x0F;
        if (owner > 7) {
            owner = -1;
        }
    }
    if (static_cast<uint8_t>(owner) == Vehs[veh_id].faction_id) {
        NetDaemonNet.order_veh(veh_id, -1, 0);
    }
}

// ORIGINAL: 0x004D04E0 ?unload@Console@@QAEXH@Z 0x004D04E0-0x004D06B6
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004D04E0-0x004D06B6", "image:.data:0x00688568 contains UNLOADNO and .data:0x00688574 contains UNLOADNEXT", "source:src/veh.h", "source:src/netdaemon.h", "source:src/netmsg.h", "source:src/console.h"], "reason": "Recover cargo unloading through Veh stack links, chassis traits and source-owned movement/ability predicates; preserve the daemon lock/synchronization lifecycle, selected-vehicle Console state, and the image's localization keys without absolute-address aliases."}
void Console::unload(int veh_id) {
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return;
    }

    const int carrier_proto = Vehs[veh_id].proto_id;
    const bool has_carried_vehicles = veh_cargo(veh_id) != 0;
    const bool carrier_ability = has_abil(carrier_proto, 0x80) != 0;
    if (!has_carried_vehicles && !carrier_ability) {
        NetDaemonNet.unlock_veh();
        return;
    }

    const int wanted_triad = carrier_ability ? 2 : 0;
    int first_cargo = veh_id;
    if (first_cargo < 0) {
        first_cargo = -1;
    } else {
        while (Vehs[first_cargo].next_veh_id_stack >= 0) {
            first_cargo = Vehs[first_cargo].next_veh_id_stack;
        }
    }

    int flags = 0;
    for (int cargo_id = first_cargo; cargo_id >= 0;
         cargo_id = Vehs[cargo_id].prev_veh_id_stack) {
        Veh &cargo = Vehs[cargo_id];
        if (cargo.year_end_lurking == 1) {
            cargo.year_end_lurking = 0;
            synch_veh(cargo_id);
        }

        const RulesChassis &chassis = Chassis[VehPrototypes[cargo.proto_id].chassis_id];
        if (chassis.triad == wanted_triad) {
            flags |= 1;
            if (veh_unmoved(cargo_id) != 0 && cargo.year_end_lurking == 0) {
                ConsoleGlobal.selected_vehicle_ = cargo_id;
                flags |= 2;
            }
        }
    }

    NetDaemonNet.unlock_veh();
    if (flags == 0) {
        NetMsgPrimary.pop("UNLOADNO", 0x1388, 0, nullptr);
        return;
    }
    if ((flags & 2) == 0) {
        NetMsgPrimary.pop("UNLOADNEXT", 0x1388, 0, nullptr);
        return;
    }
    field_23BE4_ = 0;
    set_move(1);
}

// THE TIME-CONTROLS STATE the CUSTOMTIME dialog drives: 0x009A64E0 is the
// preselected row (start's value slot copies it into the popup) and
// 0x0094F25C..0x0094F274 are alpha.h's TimeControl[5].turn/.base/.unit/
// .event/.extra/.refresh/.accumulated - the six parse_says'd fields plus the
// seventh stored on accept.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.


// The turn-assignment guard: game.cpp owns the same storage as its TU-private
// TurnsAssigned static at 0x0093E8C0. Until that static is exported this
// accessor reaches it without a second definition (the storage audit counts
// cross-TU duplicate identities).
inline int &console_turns_assigned() { return *reinterpret_cast<int *>(0x0093E8C0); }

// THE POPUP CHECKBOX ANSWER BYTE at 0x009BC06C. popup.cpp's pops() owns this
// storage as its TU-private PopupWriteCheckState (read before write_check,
// overwritten from read_check's result); until that static is exported this
// file reaches the same dword through a rebindable byte-wide slot - set_ai
// seeds the four AI checkboxes through it and reads the answer back after
// x_pop_short, both as a single byte.
inline unsigned char &console_popup_check_state() {
    return *reinterpret_cast<unsigned char *>(0x009BC06C);
}

// THE GAME-IN-PROGRESS FLAG at 0x009B2074, between game.h's ControlTurnPhase
// (0x009B2070) and SaveNameBuffer (0x009B2078). load_proc reopens a running
// game through game_reload when it is set and re-initialises the desktop when
// it is clear. The object is source-owned in game.cpp.
// Source-owned session flag is declared in game.h.

// Historical direct-address callers are retired now that action_destroy and
// the BaseWin methods have typed source-owned implementations.

// ORIGINAL: 0x004DBB40 ?editor_reset_faction@Console@@QAEXXZ 0x004DBB40-0x004DBC32 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned4DBB70..4DBC25 and faction.h PlayerData stride0x20CC: +324/+328 se_alloc_psych/labs; +228/+238 pending/active models; +1D8 then+1D4 major_atrocities then atrocities; +124 integrity_blemishes.", "Pinned4DBBC7..4DBBE3 reads Base faction byte+4, masks governor+38 and clears queue_size+4C, not queue items. Pinned4DBBF4..4DBC0E reads Veh faction+E, clears order+11 and masks state+4.", "Pinned4DBB8D/90 alternates active then pending category stores. Base count is sampled after social_upkeep; vehicle count is sampled once after base traversal. Seven faction iterations retain TectonicDetonationCount words1..7 and every original mask."], "reason": "Reset faction editor state through existing PlayerData, SocialCategory, Base and Veh owners instead of raw interior-pointer walks; preserve dialog, modifier/upkeep calls and subsequent count snapshots."}
// TRIED: nothing yet - MISMATCH #16 mov/push (94% mnemonic sim), a push/store reordering around the `compute_faction_modifiers` call; not chased further. `this` is unused throughout (thiscall receiver never touched).
// size      242 bytes
// prototype void (__thiscall ?editor_reset_faction@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B0D70 0x005B44D0 0x005BF310
void Console::editor_reset_faction() {
    if (X_pop(StrResetAllFac, (NullCallback)0) != 0) {
        PlayerData *faction = &PlayersData[1];
        unsigned int factionId = 1;
        uint32_t *rules_flag = TectonicDetonationCount + 1;
        do {
            faction->se_alloc_labs = 5;
            faction->se_alloc_psych = 0;
            compute_faction_modifiers(factionId);

            uint32_t *pending_model = faction->soc_category_pending.models_;
            uint32_t *active_model = faction->soc_category_active.models_;
            int i = 4;
            do {
                *active_model = 0;
                *pending_model = 0;
                ++active_model;
                ++pending_model;
                --i;
            } while (i != 0);

            social_upkeep(factionId);
            int count = GameDataStateStorage.fields_.base_current_count_;
            faction->major_atrocities = 0;
            faction->atrocities = 0;
            *rules_flag = 0;
            faction->integrity_blemishes = 0;

            if (count > 0) {
                Base *base = Bases;
                int n = count;
                do {
                    if (base->faction_id_current == factionId) {
                        base->governor &= 0x7fffffff;
                        base->queue_size = 0;
                    }
                    ++base;
                    --n;
                } while (n != 0);
            }
            int vehicle_count = GameDataStateStorage.fields_.vehicle_current_count_;
            if (vehicle_count > 0) {
                Veh *vehicle = Vehs;
                int n = vehicle_count;
                do {
                    if (vehicle->faction_id == factionId) {
                        vehicle->order = 0;
                        vehicle->state &= 0xfcffbdff;
                    }
                    ++vehicle;
                    --n;
                } while (n != 0);
            }
            ++factionId;
            ++rules_flag;
            ++faction;
        } while (rules_flag < TectonicDetonationCount + 8);
    }
}


// ORIGINAL: 0x004E1300 ?editor_clear_vehicles@Console@@QAEXXZ 0x004E1300-0x004E137F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned4E1300..4E137F reads MapArea then MapTiles after auto_undo and before resetting editor globals. Initial JLE and loop-end JL compare signed32-bit tile counts; the loop reloads MapArea.", "Pinned4E1343..4E1366 identifies Map record+8 bit maskedFFFFFFFC, seven copies from bit to+10..+28 bit_visible, and stride2C; bit2 at+C is not changed. map.h declares these members and map_init allocates MapArea*sizeof(Map)."], "reason": "Use the genuine Map allocation and bit/visibility members for the editor reset, preserving original signed count checks and callback/reset/redraw order."}
// working copy - scaffold materialised by --work
// size      127 bytes
// prototype void (__thiscall ?editor_clear_vehicles@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x00523DD0 0x005ABF20

void Console::editor_clear_vehicles() {
    auto_undo();
    const int initial_area = static_cast<int>(MapArea);
    Map *tile = MapTiles;
    GameDataStateStorage.fields_.game_state_ |= 4;
    int count = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    if (initial_area > 0) {
        do {
            tile->bit &= 0xfffffffcU;
            for (int i = 0; i < 7; ++i) {
                tile->bit_visible[i] = tile->bit;
            }
            ++count;
            ++tile;
        } while (count < static_cast<int>(MapArea));
    }
    reset_territory();
    ::draw_map(1);
}

// ORIGINAL: 0x004E1290 ?editor_clear@Console@@QAEXXZ 0x004E1290-0x004E12FB
// RECOVERY_BINDING: v1 {"evidence": ["git:d5d0b2c01472baa3cf01dda67b73e270d64b4db7:src/recovered/units/004e1290.cpp;source:src/console.cpp:Console::editor_clear_vehicles;source:src/map.h;source:src/path.h;source:src/worldwin.h;source:src/game.h;source:src/veh.h;source:src/base.h;source:src/console.h"], "reason": "Recover the editor map-reset coordinator through the typed GameState, vehicle/base counts, Console cursor state, map lifecycle, Path and WorldWin owners. The historical body establishes the ordered reset and redraw operations."}
void Console::editor_clear() {
    auto_undo();
    GameDataStateStorage.fields_.game_state_ |= STATE_UNK_4;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    map_wipe();
    size_of_planet(2);
    map_shutdown();
    map_init();
    Paths.init();
    map_wipe();
    draw_map(1);
    WorldClimateWorldWin.redraw();
}

// ORIGINAL: 0x004E0210 ?editor_date@Console@@QAEXXZ 0x004E0210-0x004E0282
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004e0210.cpp;source:src/console.cpp:StrEditorYear;source:src/game.h:game_year;source:src/game.h:TurnCurrentNum;source:src/game.h:StartingMissionYear;source:src/general.h:ParseNumTable;source:src/mapwin.h:MapWinTable;source:src/win.h:Win::on_redraw;source:src/basewin.h:BaseWin::on_redraw;source:src/console.h:Console::update_data"], "reason": "Recover the editor year prompt from its named EDITORYEAR string, the current turn/year fields, and the existing Console, MapWin and BaseWin owners. The seed’s indirect call through the primary MapWin pointer reaches the virtual GraphicWin base at Win::on_redraw slot 12."}
void Console::editor_date() {
    auto_undo();
    const int accepted = X_pop_ask_number(
        StrEditorYear, static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_)), 0);
    if (accepted == 0) {
        GameDataStateStorage.fields_.turn_current_num_ = ParseNumTable[0] - static_cast<int>(ScenarioSave.starting_mission_year);
        if (GameDataStateStorage.fields_.turn_current_num_ < 1) {
            GameDataStateStorage.fields_.turn_current_num_ = 1;
        }
        MapWinTable[0]->on_redraw();
        TutWinBaseWindow.on_redraw();
        ConsoleGlobal.update_data(0);
    }
}

// THE CURSOR HOTSPOT TABLES, two int runs immediately ahead of the cursor
// sprite array sprite.cpp owns at 0x0093AA70. cursor_on passes
// CursorHotspotX[shape] and CursorHotspotY[shape] as Win::set_cursor's
// hot_x / hot_y, which is the whole of the evidence for the pairing and the
// order; neither table has a symbol in the image.
// RECOVERY_OBJECT: v2 0x0093AA10 0x0093AA40 ?CursorHotspotX@@3PAHA 0x00454FC0
int CursorHotspotX[12] = {0};  // 0x0093AA10, defined here
// RECOVERY_OBJECT: v2 0x0093AA40 0x0093AA70 ?CursorHotspotY@@3PAHA 0x00454FC0
int CursorHotspotY[12] = {0};  // 0x0093AA40, defined here

namespace {
int selecting_signed(uint32_t value) {
    return value <= 0x7fffffffU ? static_cast<int>(value)
                              : -1 - static_cast<int>(0xffffffffU - value);
}
}

// ORIGINAL: 0x005105C0 ?selecting@Console@@QAEHHH@Z 0x005105C0-0x005107D5
// RECOVERY_BINDING: v1 {"evidence": ["Original 0x005105C0..0x005107D5 and its retired transcription preserved in Git history.", "Original early mode1/mode0 returns skip pumping. Ordinary cursor mode is captured after valid_patrol; each called receiver is retained for cache writeback while subsequent table slots and hotspots are read live.", "Original68FAF0 is MapLongitude, the half-width used by the wrap test; valid initialized vehicle/window state and representable coordinate arithmetic are required by the original."], "reason": "Console owns selection-mode admission and the shared map-window cursor update. Recover genuine receivers and cursor globals; the range and patrol refusals both select loaded sprite5."}
int Console::selecting(int tile_x, int tile_y) {
    const uint32_t initial_mode = field_23D10_;
    if (initial_mode == 1) {
        return 1;
    }
    if (initial_mode == 0) {
        return 0;
    }

    bool forbidden = false;
    if (selecting_signed(field_23D14_) >= 0) {
        const int vehicle = selected_vehicle_;
        const int vehicle_x = Vehs[vehicle].x;
        const int vehicle_y = Vehs[vehicle].y;
        int dx = abs(tile_x - vehicle_x);
        if ((MapIsFlat & 1) == 0 && dx > selecting_signed(MapLongitude)) {
            dx = MapLongitudeBounds - dx;
        }
        const int distance_sum = dx + abs(tile_y - vehicle_y);
        // Map coordinates have representable differences and sums. The
        // original SAR floors negative odd sums rather than truncating.
        const int distance = distance_sum >= 0 ? distance_sum / 2
                           : -1 - ((-1 - distance_sum) / 2);
        forbidden = distance > selecting_signed(field_23D14_);
    }
    if (!forbidden && field_23D10_ == 8) {
        forbidden = valid_patrol(selected_vehicle_, tile_x, tile_y) == 0;
    }

    // Both refusals select sprite5. Capture an ordinary mode after the
    // patrol call; preserve that choice through the window callbacks.
    const int shape = forbidden ? 5 : selecting_signed(field_23D10_);
    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window != 0 && (slot == 0 || window->field_1DD74_ != 0)) {
            Sprite *const sprite = &g_CURSOR_SPRITES[shape];
            if (window->cursor_sprite_ != sprite) {
                window->GraphicWin::set_cursor(sprite,
                    CursorHotspotX[shape], CursorHotspotY[shape]);
                // Write back to the called receiver, even if its callback
                // replaced the table slot. Subsequent slots are read live.
                window->cursor_sprite_ = sprite;
            }
        }
    }
    do_non_input();
    return 1;
}

// The editor's landmark removal command. The original uses the current
// cursor-history coordinate when either supplied coordinate is outside the
// map, then asks for confirmation before deleting the named landmark.
// The three popup keys are the strings at 0x0068A994/0x0068A9A4/0x0068A9B0.
// ORIGINAL: 0x00516490 ?erase_landmark@Console@@QAEXHH@Z 0x00516490-0x005165A6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The original 0x00516490..0x005165A6 seed searches a landmark, checks editor permission, confirms LANDMARKKILL, then either sends a multiplayer message or deletes and redraws locally.", "src/map.h owns MapLandmark, MapLandmarkCount, map bounds and landmark helpers; src/console.h owns cursor-history coordinates and faction_id_; src/game.h owns GameState and IsMultiplayerNet.", "The pinned image stores popup keys LANDMARKNONE, LANDMARKNO and LANDMARKKILL at 0x0068A994, 0x0068A9A4 and 0x0068A9B0.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00516490.cpp"], "reason": "Recover the Console landmark removal command through typed map, game and cursor owners while preserving its historical byte claim in the separate seed until remeasurement. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void Console::erase_landmark(int x_coord, int y_coord) {
    if (y_coord < 0 || y_coord >= MapLatitudeBounds ||
        x_coord < 0 || x_coord >= MapLongitudeBounds) {
        const int index = ConsoleGlobal.cursor_history_index_;
        x_coord = ConsoleGlobal.cursor_history_x_[index];
        y_coord = ConsoleGlobal.cursor_history_y_[index];
    }

    const int landmark_id = find_landmark(x_coord, y_coord, 1);
    if (landmark_id < 0) {
        NetMsgPrimary.pop("LANDMARKNONE", 5000, 0, nullptr);
        return;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 0x80) == 0 &&
        valid_landmark(x_coord, y_coord, ConsoleGlobal.faction_id_) == 0) {
        NetMsgPrimary.pop("LANDMARKNO", 5000, 0, nullptr);
        return;
    }

    parse_says(0, MapLandmark[landmark_id].name, -1, -1);
    if (X_pop("LANDMARKKILL", nullptr) == 0) {
        return;
    }
    if (IsMultiplayerNet != 0) {
        message_data(0x2e02, 0, x_coord, y_coord, 0, 0);
        return;
    }
    kill_landmark(MapLandmark[landmark_id].x, MapLandmark[landmark_id].y);
    ::draw_map(1);
}

// ORIGINAL: 0x00510400 ?cursor_on@Console@@QAEXH@Z 0x00510400-0x00510490 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      144 bytes
// prototype void (__thiscall ?cursor_on@Console@@QAEXH@Z)(Console* this, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC740 0x005FCA30

void Console::cursor_on(int cursor_shape) {
    // The original assumes cursor_shape names one of the twelve loaded
    // cursor sprites; neither it nor the hotspot lookup adds a bounds guard.
    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window != nullptr && (slot == 0 || window->field_1DD74_ != 0)) {
            Sprite *const sprite = &g_CURSOR_SPRITES[cursor_shape];
            if (window->cursor_sprite_ != sprite) {
                window->GraphicWin::set_cursor(sprite,
                    CursorHotspotX[cursor_shape], CursorHotspotY[cursor_shape]);
                // set_cursor may dispatch callbacks. Publish to the window
                // we called, not a replacement installed in its table slot.
                window->cursor_sprite_ = sprite;
            }
        }
    }
    do_non_input();
}

// ORIGINAL: 0x005107E0 ?selected@Console@@QAEXXZ 0x005107E0-0x00510899 FILE
// symbol    ?selected@Console@@QAEHXZ
// TRIED: manual vbase-adjust pointer arith for Win::set_cursor `this` (obj + *(int*)(*(int*)obj+4)); #0 push/mov is frame-setup register order, not a logic mismatch
// working copy - scaffold materialised by --work
// size      185 bytes
// prototype void (__thiscall ?selected@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550 0x005EC7C0 0x005FCA30

// The catalogue's void return is wrong: 0x005107E4 saves the entry mode,
// 0x00510881 restores it to EAX after callbacks, and MapWin::click consumes it.
int Console::selected() {
    const int selected_mode = static_cast<int>(field_23D10_);
    if (selected_mode != 0) {
        field_23D10_ = 0;
        for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
            MapWin *const window = MapWinTable[slot];
            if (window != nullptr && (slot == 0 || window->field_1DD74_ != 0)) {
                if (window->cursor_sprite_ != nullptr ||
                    window->cursor_resource_id_ != 0x7f00) {
                    window->GraphicWin::set_cursor(0x7f00);
                    window->cursor_resource_id_ = 0x7f00;
                    window->cursor_sprite_ = nullptr;
                }
                // Unlike cursor writeback, repaint state belongs to the
                // live slot after the cursor callback (0x00510859).
                const uint32_t flags = MapWinTable[slot]->field_1DD70_;
                if ((flags & 0xc40000) != 0) {
                    MapWinTable[slot]->field_1DD70_ = flags & 0xff3bffff;
                    MapWinTable[slot]->draw_map(1);
                }
                do_non_input();
            }
        }
    }
    return selected_mode;
}

// ORIGINAL: 0x00515EF0 ?toggle_patrol@Console@@QAEXXZ 0x00515EF0-0x00515F70 FILE
// working copy - scaffold materialised by --work
// size      128 bytes
// prototype void (__thiscall ?toggle_patrol@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00467970 0x0046A550

void Console::toggle_patrol() {
    MapWin *const first = MapWinTable[0];
    int mask = first->field_1DD70_ & 0x400000;
    int *p = reinterpret_cast<int *>(MapWinTable);
    do {
        MapWin *const window = *reinterpret_cast<MapWin *const *>(p);
        if (window != nullptr && (p == reinterpret_cast<int *>(MapWinTable) ||
                                 window->field_1DD74_ != 0)) {
            if (mask == 0) {
                window->field_1DD70_ |= 0x400000;
                window->draw_base_dest(1);
            } else {
                window->field_1DD70_ &= 0xffbfffff;
                window->draw_map(1);
            }
        }
        p = p + 1;
    } while (p < reinterpret_cast<int *>(MapWinTable + MapWinTableSlots));
}

// ORIGINAL: 0x00516970 ?menu_simple@Console@@QAEXXZ 0x00516970-0x005169A1 FILE BYTE_EXACT
// size      49 bytes
// prototype void (__thiscall ?menu_simple@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00460DD0 0x005BF310
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00516970
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00516970/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?menu_simple@Console@@QAEXXZ  at 0x00516970  (49 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void Console::menu_simple() {
    // NEAR MISS, not byte-exact (MNEMONIC_ONLY): the original loads the
    // field-0x11b0 value into EAX and cleans the X_pop call with
    // `add esp,8`; this toolchain's /O2 register allocator instead puts the
    // value in EDX here (mnemonics match one-for-one, only the register
    // operand differs), and under /O1 it folds the load into `push
    // dword ptr [...]` plus a `pop/pop` cleanup - both worse.
    //
    // RULED OUT: ten source-level variants tried, all producing the exact
    // same EDX choice under /O2 - named vs. inlined field read, swapping
    // statement order, `volatile`, `register`, reusing ONE local for both
    // the flag and the field value (to bias reuse of the just-freed EAX),
    // a comma-expression folding the RMW and the read into one argument
    // expression, and routing the read through a forced-inline getter (to
    // invoke the "return value lives in eax" convention). Also confirmed in
    // ISOLATION, outside this function: a bare N-argument forwarding probe
    // (unrelated to this call) shows this same EAX-after-just-freed-EAX
    // reuse never happens for a fresh single-use temporary under these
    // project flags - see 0x00624D00's near miss for the same family of
    // issue (fixed register-count/window choices by the scheduler, not
    // reachable from portable source).
    // menu+0x11b0 is selected_vehicle_: 0x22A2C + 0x11B0 = 0x23BDC. The member
    // form replaces the MainMenu pun this body used to carry.
    GameDataStateStorage.fields_.game_more_preferences_ &= 0xffdfffff;
    alpha_menu_.check(selected_vehicle_);
    X_pop(StrMenuSimple, 0);
}

// ORIGINAL: 0x00516930 ?menu_detail@Console@@QAEXXZ 0x00516930-0x00516961 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x00516930 seed sets GameMorePreferences bit 0x200000, calls the Console member at +0x22A2C using its current vehicle, and passes the image string at 0x0068A9C0 to X_pop.", "src/console.h identifies alpha_menu_ at Console +0x22A2C and selected_vehicle_; src/console.cpp already binds the matching MENUDETAIL text as StrMenuDetail, and alpha.h names GameMorePreferences."], "reason": "Express menu detail activation through the existing preference flag, typed AlphaMenu member, Console selection state and established string binding."}
void Console::menu_detail() {
    GameDataStateStorage.fields_.game_more_preferences_ |= 0x200000;
    alpha_menu_.check(selected_vehicle_);
    X_pop(StrMenuDetail, 0);
}

// ORIGINAL: 0x00516C50 ?show_scen_objectives@Console@@QAEXXZ 0x00516C50-0x00516D98
// RECOVERY_BINDING: v1 {"evidence": ["The 0x00516C50 seed parses ObjectiveReqVictory, ObjectivesSuddenDeathVictory and EndingMissionYear before processing faction-specific names.", "src/faction.h defines Players as 0x59C Player records: the seed's 0x00946A50, 0x00946A9C, 0x00946A84, 0x00946D4C, 0x00946D50 and 0x00946D34 references correspond to Player::is_leader_female, title_leader, name_leader, noun_gender, is_noun_plural and noun_faction.", "src/game.h owns GameRules and victory settings; general.h owns GenderDefault, PluralityDefault, parse_num, parse_says and say_year; strings.h owns StringTemp. The two popup strings at 0x0068AA0C and 0x0068AA18 were read from the pinned image as THEYFAILEDFLECHETTE and FLECHETTE.", "The seed's 0x005BF330 caption/label/callback call is represented by xpops.h's x_pop_caption_label, whose implementation forwards with the defaults (-1, no override, flags 0) to the typed full popup."], "reason": "Render scenario objectives through the existing faction, game-state, parser, scratch-buffer and popup owners, using the exact popup key values without raw data addresses."}
void Console::show_scen_objectives() {
    const int faction_id = ConsoleGlobal.faction_id_;
    const Player &faction = Players[faction_id];
    parse_num(0, ScenarioSave.objective_req_victory);
    parse_num(1, ScenarioSave.objectives_sudden_death_victory);
    parse_num(2, num_objectives(faction_id, GameDataStateStorage.fields_.game_rules_ & 0x1000));
    parse_num(3, ScenarioSave.ending_mission_year);

    GenderDefault = faction.is_leader_female;
    PluralityDefault = 0;
    parse_says(1, faction.title_leader, -1, -1);
    GenderDefault = faction.is_leader_female;
    PluralityDefault = 0;
    parse_says(2, faction.name_leader, -1, -1);
    GenderDefault = faction.noun_gender;
    PluralityDefault = faction.is_noun_plural;
    parse_says(3, faction.noun_faction, -1, -1);

    StringTemp[0] = 0;
    say_year(StringTemp);
    parse_says(4, StringTemp, -1, -1);
    parse_says(0, StringTemp, -1, -1);
    x_pop_caption_label(StrScenarioObjectivesTitle, StrScenarioObjectivesPopup, 0);
}

// Fixed-slot bindings carried from 004dbc40.cpp

// ORIGINAL: 0x004DBC40 ?editor_reset_tech@Console@@QAEXXZ 0x004DBC40-0x004DBD18 FILE
// TRIED: `field_23BDC_` named field is WRONG - it lands at 0x245f0, not 0x23bdc (raw `self+0x23bdc` cast is correct, per the "scaffold's named fields can be wrong" caution). `reinterpret_cast<int>` on the three pointer loop bounds gets `jl` instead of `jb` (pointer `<` compiles unsigned). Landed at MNEMONIC_ONLY: the one remaining diff is the `*p=0` store and the `p[1]` load at the very last loop swapped order (compiler schedules the independent load before the store regardless of source order) - tried a named temp, a ternary store, and an if/else split; none reproduced store-then-load.
// working copy - scaffold materialised by --work
// size      216 bytes
// prototype void (__thiscall ?editor_reset_tech@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005BF310 0x005C08C0

void Console::editor_reset_tech() {
    int *field23bdc = &selected_vehicle_;
    if (X_pop(StrResetAllTech, 0) != 0) {
        for (int *p1 = reinterpret_cast<int *>(0x96ee08);
             reinterpret_cast<int>(p1) < 0x97d39c; p1 += 0x833) {
            *p1 = 0;
        }
        memset(reinterpret_cast<void *>(0x9a6670), 0, 0x59);
        for (unsigned char *p2 = reinterpret_cast<unsigned char *>(0x9ab898);
             reinterpret_cast<int>(p2) < 0x9b2098; p2 += 0x34) {
            p2[-4] = 0;
            if (reinterpret_cast<int>(p2) >= 0x9ac598) {
                *p2 &= 0xfe;
            }
        }
        *field23bdc = *reinterpret_cast<int *>(0x9a64c8) - 1;
        if (*field23bdc >= 0) {
            do {
                if (*reinterpret_cast<short *>(0x952832 + *field23bdc * 0x34) >= 0x40) {
                    veh_kill(*field23bdc);
                }
                *field23bdc = *field23bdc - 1;
            } while (*field23bdc >= 0);
        }
        int count = *reinterpret_cast<int *>(0x9a64cc);
        if (count > 0) {
            int *p3 = reinterpret_cast<int *>(0x97d08c);
            do {
                *p3 = 0;
                int next = p3[1];
                if (next >= 0x40) {
                    *p3 = 0xffffffbb;
                }
                p3 = reinterpret_cast<int *>(reinterpret_cast<char *>(p3) + 0x134);
                --count;
            } while (count != 0);
        }
        draw_map(1);
    }
}

// Fixed-slot bindings carried from 00510ce0.cpp

// ORIGINAL: 0x00510CE0 ?ready_search@Console@@QAEHH@Z 0x00510CE0-0x00510F48 FILE
// TRIED: prologue register order (7-slot frame, edi saved to a local purely to free it as scratch across the inner loop) - not chased byte-for-byte; control flow/offsets re-derived from raw disasm since Ghidra's local_N numbering did not line up with the real ebp-relative slots here.
// working copy - scaffold materialised by --work
// size      616 bytes
// prototype int (__thiscall ?ready_search@Console@@QAEHH@Z)(Console* this, int)
// callers   4   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C1B70 0x00644F3A

int Console::ready_search(int veh_id) {
    int bestScore = 9999;
    int result = -1;
    int coordA, coordB;

    if (veh_id < 0 || Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_) {
        veh_id = -1;
        coordA = ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
        coordB = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
    } else {
        coordA = *(short *)((unsigned char *)Vehs + veh_id * 0x34);
        coordB = Vehs[veh_id].y;
    }

    int outerIdx = (IsMultiplayerNet != 0) ? -1 : 0;

    while (outerIdx <= 2) {
        int vehCount = GameDataStateStorage.fields_.vehicle_current_count_;

        if ((IsMultiplayerNet != 0) && (outerIdx == 0) && (focus_vehicle_ >= 0) &&
            (focus_vehicle_ < vehCount) &&
            (Vehs[focus_vehicle_].faction_id == ConsoleGlobal.faction_id_) &&
            (veh_ready(focus_vehicle_) != 0)) {
            result = focus_vehicle_;
            focus_vehicle_ = -1;
            goto searchDone;
        }

        vehCount = GameDataStateStorage.fields_.vehicle_current_count_;
        for (int innerIdx = 0; innerIdx < vehCount; innerIdx++) {
            unsigned char *rec = reinterpret_cast<unsigned char *>(&Vehs[innerIdx].state);

            if (veh_ready(innerIdx) == 0) continue;

            if (outerIdx >= 0 ||
                (*(unsigned char *)(rec + 1) & 2) != 0 ||
                (*(unsigned int *)rec & 0x1000200) == 0x1000200 ||
                (*(unsigned char *)(rec + 1) & 0x40) != 0 ||
                rec[0xd] != 0) {

                if (*rec & 2) continue;

                int vehX = *(short *)(rec - 4);
                int vehY = *(short *)(rec - 2);

                int diffX = abs(coordA - vehX);
                if ((MapIsFlat & 1) == 0 && diffX > (int)MapLongitude) {
                    diffX = MapLongitudeBounds - diffX;
                }
                int diffY = abs(coordB - vehY);
                diffX = abs(diffX);
                diffY = abs(diffY);

                int hi = (diffX > diffY) ? diffX : diffY;
                int lo = (diffX < diffY) ? diffX : diffY;
                int score = hi - (((diffX + diffY) >> 1) - lo + 1 >> 1);

                if (score < bestScore) {
                    bestScore = score;
                    result = innerIdx;
                } else if (score == bestScore && innerIdx == selected_vehicle_) {
                    result = innerIdx;
                }
            }
        }

        if (result >= 0) goto searchDone;

        if (outerIdx >= 0) {
            vehCount = GameDataStateStorage.fields_.vehicle_current_count_;
            for (int clearIdx = 0; clearIdx < vehCount; clearIdx++) {
                unsigned int *rec = reinterpret_cast<unsigned int *>(&Vehs[clearIdx].state);
                if (outerIdx != 0 || clearIdx != selected_vehicle_ || veh_id < 0) {
                    *rec &= 0xfffffffd;
                }
            }
        }

        outerIdx++;
    }

searchDone:
    if (IsMultiplayerNet != 0 && outerIdx < 0) {
        field_23D24_ = 1;
    } else {
        field_23D24_ = 0;
    }
    return result;
}

// Fixed-slot bindings carried from 004e0a00.cpp

// ORIGINAL: 0x004E0A00 ?editor_load@Console@@QAEXXZ 0x004E0A00-0x004E0A50 FILE
// size      80 bytes
// prototype void (__thiscall ?editor_load@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005AB600
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004E0A00
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e0a00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?editor_load@Console@@QAEXXZ  at 0x004E0A00  (80 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void Console::editor_load() {
    if (load_map() != 0) {
        return;
    }
    int flags = GameDataStateStorage.fields_.game_state_;
    flags |= 4;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    GameDataStateStorage.fields_.game_state_ = flags;
    draw_map(1);
    ConsoleInfoWin.set_redraw_phase(7);
    ConsoleInfoWin.on_redraw();
}

// MainInterface owns the concrete third FlatButton selected by the
// original constructor's 42-element member-array initialization.

// ORIGINAL: 0x004E0FD0 ?editor_generate@Console@@QAEXXZ 0x004E0FD0-0x004E10BC FILE
// size      236 bytes
// prototype void (__thiscall ?editor_generate@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046A550 0x0046B190 0x005ABF20 0x005C86E0
// indirect  0x004E10A6 0x004E10B6


void Console::editor_generate() {
    auto_undo();

    int count = MapArea;
    Map *tile = map_tiles();

    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;

    if (count > 0) {
        int i = 0;
        do {
            i++;
            tile->bit &= ~(BIT_BASE_IN_TILE | BIT_VEH_IN_TILE);
            ++tile;
        } while (i < static_cast<int>(MapArea));
    }

    ConsoleGlobal.field_23BF8_ = 0;
    GameDataStateStorage.fields_.game_state_ = GameDataStateStorage.fields_.game_state_ | 4;
    // 0x004E1033 loads slot zero before accessing the window's +0x1DD98
    // member. The array itself is neither the receiver nor member storage.
    MapWinTable[0]->field_1DD98_ = static_cast<uint32_t>(-14);
    MapWinTable[0]->draw_map(1);

    int flags = GameDataStateStorage.fields_.game_state_;
    int wasSet = flags & 0x80;
    flags |= 0x80;
    GameDataStateStorage.fields_.game_state_ = flags;
    world_build();
    ::draw_map(1);
    if (wasSet == 0) {
        int f2 = GameDataStateStorage.fields_.game_state_;
        f2 &= ~0x80;
        GameDataStateStorage.fields_.game_state_ = f2;
        ::draw_map(1);
    }

    // MainInterface construction at 0x0045EF10 creates 42 FlatButtons at
    // +0xE00 with stride 0xB4C. Element 2 is the original 0x007B0CB8
    // receiver; its table selects Win::show and BaseButton::hide.
    unsigned char flagByte = static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
    if ((flagByte & 0x80) != 0 && (flagByte & 0x40) != 0) {
        MainInterfaceGlobal.flat_button_[2].show(0);
        return;
    }
    MainInterfaceGlobal.flat_button_[2].hide();
}

// Fixed-slot bindings carried from 004e1190.cpp

// ORIGINAL: 0x004E1190 ?editor_randomize@Console@@QAEXXZ 0x004E1190-0x004E11F7 FILE
// size      103 bytes
// prototype void (__thiscall ?editor_randomize@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x00625810
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004E1190
// measured tier  MISMATCH
// divergence     11
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e1190/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?editor_randomize@Console@@QAEXXZ  at 0x004E1190  (103 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void Console::editor_randomize() {
    auto_undo();
    unsigned int r = random(0, 0x7fff);
    int guard = MapArea;
    MapRandSeed = r + 1;
    int i = 0;
    Map *record = map_tiles();
    if (0 < guard) {
        do {
            record->bit &= 0xcfeffbff;
            uint32_t *dest = record->bit_visible;
            int n = 7;
            do {
                *dest = record->bit;
                ++dest;
            } while (--n);
            ++i;
            ++record;
        } while (i < MapArea);
    }
    draw_map(1);
}

// ORIGINAL: 0x004E1200 ?editor_clear_random@Console@@QAEXXZ 0x004E1200-0x004E128B FILE
// working copy - scaffold materialised by --work
// size      139 bytes
// prototype void (__thiscall ?editor_clear_random@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x00625810

void Console::editor_clear_random() {
    auto_undo();
    if (MapRandSeed == 0) {
        MapRandSeed = random(0, 0x7fff) + 1;
        draw_map(1);
        return;
    }
    MapRandSeed = 0;
    if (MapArea > 0) {
        Map *tile = map_tiles();
        int i = 0;
        do {
            uint32_t v = tile->bit;
            if ((v & 0x400) != 0 && (v & 0x20000000) == 0) {
                v &= 0xdffffbff;
                tile->bit = v;
                uint32_t *dst = tile->bit_visible;
                int count = 7;
                do {
                    *dst = v;
                    dst++;
                    count--;
                } while (count != 0);
            }
            i++;
            ++tile;
        } while (i < MapArea);
    }
    draw_map(1);
}

// Fixed-slot bindings carried from 00510490.cpp

// ORIGINAL: 0x00510490 ?cursor_on@Console@@QAEXHH@Z 0x00510490-0x0051051D FILE
// working copy - scaffold materialised by --work
// size      141 bytes
// prototype void (__thiscall ?cursor_on@Console@@QAEXHH@Z)(Console* this, int, int)
// callers   5   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC740 0x005FCA30

void Console::cursor_on(int cursor_shape, int veh_id) {
    field_23D14_ = static_cast<uint32_t>(veh_id);
    field_23D10_ = static_cast<uint32_t>(cursor_shape);
    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window != nullptr && (slot == 0 || window->field_1DD74_ != 0)) {
            Sprite *const sprite = &g_CURSOR_SPRITES[cursor_shape];
            if (window->cursor_sprite_ != sprite) {
                window->GraphicWin::set_cursor(sprite,
                    CursorHotspotX[cursor_shape], CursorHotspotY[cursor_shape]);
                window->cursor_sprite_ = sprite;
            }
        }
    }
    do_non_input();
}

// ORIGINAL: 0x004E14A0 ?editor_rockiness@Console@@QAEXXZ 0x004E14A0-0x004E14C8 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?editor_rockiness@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x005BF3F0 0x005C32A0
// 0x004E14A0  ?editor_rockiness@Console@@QAEXXZ  ->  ?editor_rockiness@Console@@QAEXXZ
//
// Purpose: editor command - ask for the rocky-terrain density and, if the
//          prompt is not cancelled, repaint the world that rocky.
//
// PREPARED, NOT HOMED (2026-09-03, P1 calibration). Two things are missing,
// neither of them in this body:
//   1. `world_rocky` is ?world_rocky@@YAXXZ at 0x005C32A0
//      (world_rocky - homed into src/map.cpp with the 2026-09-04 map batch),
//      general.h or map.h look like the home - and a pending_bodies forwarder;
//      the shape is already written there beside the veh_ready/stack_kill
//      ones.
//   2. `EditorRockinessLabel` is the image's .data string "ROCKOSITY" at
//      0x00689174. Nothing in the tree binds it. HOWEVER this is spelled, note
//      that the image PUSHES the address as an immediate (`push 0x689174`),
//      so a tree string literal at a different address will not match byte for
//      byte the way the `"EDITORWHO"` literal in Console::editor_who does.
//
// LEVER (already applied here, unmeasured because of the two blockers): the
//   image's callee at 0x005BF3F0 is this tree's `x_pop_short`, not X_pop - the
//   scaffold's `X_pop(label, 0x40, 0)` names a function this tree spells
//   differently. And it is `::draw_map`, not the inherited MapWin::draw_map:
//   the free cdecl wrapper cleans its own argument, which is where the image's
//   trailing `pop ecx` comes from.
//
// HOMED 2026-09-04: both blockers cleared. world_rocky is declared in map.h
// beside its world_* siblings (the definition landed in map.cpp); the string
// is bound as `EditorRockinessLabel` at 0x00689174. No forwarder was needed -
// world_rocky is a real definition in map.cpp, not a pending_bodies stub.


void Console::editor_rockiness() {
    auto_undo();
    if (x_pop_short(EditorRockinessLabel, 0x40, 0) != 0) {
        world_rocky();
        ::draw_map(1);
    }
}

// ORIGINAL: 0x005148B0 ?move_cursor@Console@@QAEXH@Z 0x005148B0-0x00514990 FILE
// size      224 bytes
// prototype void (__thiscall ?move_cursor@Console@@QAEXH@Z)(Console* this, int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550 0x0046AD60 0x0046B310 0x004B9EA0 0x005109B0
// LEVER: the image's two `draw_cursor` calls are MapWin::draw_cursor
// (0x0046AD60, the per-window member), not the free ::draw_cursor at
// 0x0046AE00 that mapwin.cpp defines - the archive body named the free one
// and so was calling a function the image never calls here. Declared on
// MapWin with a pending_bodies forwarder; VehMoveDeltaX/Y moved to veh.h
// beside the array definitions so the cursor's own deltas stop being
// anonymous fixed addresses.
// TRIED: `const int slot`; operand order `delta + coord` for the x sum. The
// image loads ConsoleGlobal.cursor_history_x_[slot] BEFORE VehMoveDeltaX[direction] (0x005148BE
// then 0x005148C6) and the reverse for y, so the x sum is spelled
// coordinate-first below; neither ordering nor the const moved the first
// divergence, which is which register holds ConsoleGlobal.cursor_history_index_ (image: edx,
// this tree: eax) and follows from the image deferring its `this` spill
// (0x005148CD `mov edi, ecx`) until after both loads.


void Console::move_cursor(int direction) {
    const int slot = ConsoleGlobal.cursor_history_index_;
    int new_x = ConsoleGlobal.cursor_history_x_[slot] + VehMoveDeltaX[direction];
    if ((MapIsFlat & 1) == 0) {
        if (new_x < 0) {
            new_x = new_x + MapLongitudeBounds;
        } else if (new_x >= MapLongitudeBounds) {
            new_x = new_x - MapLongitudeBounds;
        }
    }
    int new_y = VehMoveDeltaY[direction] + ConsoleGlobal.cursor_history_y_[slot];
    if (new_y >= 0 && new_y < MapLatitudeBounds && new_x >= 0 &&
        new_x < MapLongitudeBounds) {
        field_23BF8_ = 0;
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        console_map_win()->draw_cursor();
        cursor_next(new_x, new_y);
        ConsoleStatusWin.redraw();
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        console_map_win()->draw_cursor();
        console_map_win()->focus(new_x, new_y);
    }
}

// ORIGINAL: 0x005147E0 ?set_cursor@Console@@QAEXHH@Z 0x005147E0-0x0051487D FILE
// TRIED: nesting the last bound check as its own `if` (both alone and after a 3-way `||` early return), and a fully split `&&` chain with success nested two levels deep - all reproduce the same MISMATCH #15 'jl' vs 'jge' on the 4th comparison (a1 < bound2); similarity 0.9091. void return keeps the C2556 declared prototype; the eax=1/xor eax,eax result codes are dropped.
// size      157 bytes
// prototype void (__thiscall ?set_cursor@Console@@QAEXHH@Z)(Console* this, int xCoord, int yCoord)
// callers   6   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550 0x0046AD60 0x004B9EA0 0x005109B0
// symbol    ?set_cursor@Console@@QAEHHH@Z
// LEVER: the catalogued `void` return is wrong, and the archive body's TRIED
//        note says exactly where that cost it. Every one of the four bound
//        failures reaches 0x00514807, which loads eax with 1 before the shared
//        epilogue, and the success path clears eax at the end - so the body
//        returns int: 1 for out of bounds, 0 after the repaint. Declared int
//        (the emitted name above is the legitimately different mangling, the
//        same fact Console::focus carries) and the four tests reproduce.
//        13 of 47 -> measured below.
// TRIED: `void` with a bare `return` on the failure path - that is the
//        archive's spelling, and its eax=1 / xor eax,eax codes are dropped,
//        which is what made the 4th comparison's branch target diverge.
// TRIED: three shapes for the FOURTH bound test, all 17 of 47 with identical
//        instruction counts: chained as the fourth `||` term (the compiler
//        emits `jge <failure>`), as its own `if (x_coord >= lon) return 1;`,
//        and polarised as the in-bounds guard `if (x_coord < lon) { body;
//        return 0; }` so the source asks for the image's jump-to-the-work
//        `jl 0x514812`. VC6 inverts all three into the same `jge` and merges
//        the tails, so the 4th comparison's branch target is not reachable
//        from source form here. Not chased further.


int Console::set_cursor(int x_coord, int y_coord) {
    if (y_coord < 0 || y_coord >= MapLatitudeBounds || x_coord < 0) {
        return 1;
    }
    // The image's FOURTH test jumps to the body on `jl` (0x00514805 `jl
    // 0x514812`) and falls through to the failure epilogue, so it is spelled
    // as the in-bounds guard rather than chained into the rejection above -
    // the "jumps TO the work" shape, not a fourth `||` term.
    if (x_coord < MapLongitudeBounds) {
        field_23BF8_ = 0;
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        console_map_win()->draw_cursor();
        cursor_next(x_coord, y_coord);
        ConsoleStatusWin.redraw();
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        console_map_win()->draw_cursor();
        return 0;
    }
    return 1;
}

// ORIGINAL: 0x00515530 ?survey2@Console@@QAEXXZ 0x00515530-0x005155F5 FILE BYTE_EXACT
// size      197 bytes
// prototype void (__thiscall ?survey2@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046A550 0x004BA5F0 0x004BE650


void Console::survey2() {
    if (survey_mode_ != 0) {
        survey_mode_ = 0;
        console_map_win()->draw_map(1);
        return;
    }
    survey_mode_ = 1;
    uint32_t saved = console_map_win()->field_1DD70_;
    console_map_win()->field_1DD70_ = saved & 0xfffff7ff;
    console_map_win()->field_1DD70_ =
        console_map_win()->field_1DD70_ | 0x1000;
    console_map_win()->draw_map(1);
    console_map_win()->field_1DD70_ = saved;
    console_map_win()->field_1DD70_ =
        console_map_win()->field_1DD70_ & 0xffffefff;
    if (tut_check(0x400) != 0) {
        const int slot = ConsoleGlobal.cursor_history_index_;
        TutorialWindow.tut_map(StrTerraSurvey2, ConsoleGlobal.cursor_history_x_[slot],
                               ConsoleGlobal.cursor_history_y_[slot], -1, nullptr, 0, -1, -1);
    }
}

// ORIGINAL: 0x004E13E0 ?editor_fungus@Console@@QAEXXZ 0x004E13E0-0x004E149B FILE
// LEVER: the FUNGMOTIZE answer is tested directly, not carried in a boolean.
//        The archive body spelled `plant = (confirmed != 0)` and `if (plant)`,
//        which compiles a setne/test pair the image does not have - the image
//        branches on the pop result itself (`je 0x4e1472` at 0x004E141B skips
//        the plant loop). Seeding `confirmed` with -1 so the answer == 0 path
//        falls into the loop made one test spell both. 20 -> 23 of 59.
// LEVER: field_23BD4_ is SIGNED and MapArea is read SIGNED. `uint32_t` on the
//        field compiled `jb` on the inner back edge where the image has `jl`
//        (0x004E1461), and an unsigned MapArea gave `jbe` on the guard - the
//        field is now `int32_t` in console.h and both MapArea reads go through
//        an int cast. 23 -> 44 of 59.
// LEVER: MapArea is NOT cached in a local. The image reads [0x949884] for the
//        guard and again at 0x004E1463 on every outer iteration; holding it in
//        `const int count` cost a callee-saved register and the extra push that
//        goes with it. 44 -> 45 of 59.
// TRIED: the remaining 14 are ordering and register naming only (59 vs 59
//        instructions): the image schedules the MapArea load, the
//        map_tiles() load and then `xor ecx, ecx` before the test, where this
//        tree zeroes i first. Not reached from source form after four
//        measured spellings.

// size      187 bytes
// prototype void (__thiscall ?editor_fungus@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x005BF3F0 0x005C3440


void Console::editor_fungus() {
    auto_undo();
    const int answer = x_pop_short(StrFungosity, 0x40, 0);
    if (answer >= 0) {
        // The image carries no boolean here: it tests the FUNGMOTIZE answer
        // directly (`je 0x4e1472` at 0x004E141B skips the plant loop) and
        // falls into the loop on the answer == 0 path, where the second prompt
        // was never asked. Seeding `confirmed` with -1 makes one test spell
        // both of those.
        int confirmed = -1;
        if (answer > 0) {
            confirmed = x_pop_short(StrFungmotize, 0x40, 0);
            if (confirmed < 0) {
                return;
            }
        }
        if (confirmed != 0) {
            // MapArea is NOT cached: the image reads [0x949884] for the guard
            // and again at 0x004E1463 on every outer iteration.
            // MapArea is NOT cached: the image reads [0x949884] for the guard
            // and again at 0x004E1463 on every outer iteration. Both compares
            // are SIGNED in the image (`jle` at 0x004E142C, `jl` at the tail),
            // so the read goes through an int cast - uint32_t compiles jbe.
            if (static_cast<int>(MapArea) > 0) {
                Map *tile = map_tiles();
                int i = 0;
                do {
                    tile->bit &= 0xFFFFFFDFu;
                    faction_id_ = 1;
                    do {
                        tile->bit_visible[faction_id_ - 1] = tile->bit;
                        faction_id_ = faction_id_ + 1;
                    } while (faction_id_ < 8);
                    i++;
                    tile++;
                } while (i < static_cast<int>(MapArea));
            }
        }
        uint32_t saved = MapNativeLifeForms;
        if (answer > 0) {
            MapNativeLifeForms = answer - 1;
            world_fungus();
        }
        MapNativeLifeForms = saved;
        ::draw_map(1);
    }
}

// ORIGINAL: 0x004E1F70 ?editor_lock2@Console@@QAEXXZ 0x004E1F70-0x004E1FF6 FILE
// TRIED: straightforward if/else on the bit-test, reusing the already declared NetMsg::pop / MainInterface::draw_edit_pane / this member set_view(1). MISMATCH #0 'push' vs 'mov' remains open (register allocation for `this`/esi differs from the start).
// size      134 bytes
// prototype void (__thiscall ?editor_lock2@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045B550 0x0047A890 0x00511480


// TRIED: three spellings for the 0x8000000 mask - the bare immediates
//        (`flags & 0x8000000` / `& 0xf7ffffff`), `~0x8000000u` for the clear,
//        and a named `const uint32_t lock_mask` used for both the test and the
//        `~lock_mask` clear. All three score 1/38. The image materialises the
//        mask in a register (`mov eax, 0x8000000`, then `test eax, ecx` and
//        `or edx, eax`) AND spills `this` before anything else (`push esi` /
//        `mov esi, ecx` at 0x004E1F70/0x004E1F71); this tree keeps the mask
//        immediate and loads GameState first. That is the receiver-spill wall
//        palette.cpp documented - the image pays a callee-saved push the source
//        does not ask for - and neither guard reordering nor the mask's source
//        form reaches it. Not chased further.
void Console::editor_lock2() {
    const uint32_t flags = GameDataStateStorage.fields_.game_state_;
    const uint32_t lock_mask = 0x8000000;
    if (flags & lock_mask) {
        GameDataStateStorage.fields_.game_state_ = flags & ~lock_mask;
        NetMsgPrimary.pop(StrProtectOff, 5000, 0, 0);
        MainInterfaceGlobal.draw_edit_pane();
        return;
    }
    GameDataStateStorage.fields_.game_state_ = flags | lock_mask;
    NetMsgPrimary.pop(StrProtectOn, 5000, 0, 0);
    if (ConsoleGlobal.cursor_active_ != 0) {
        set_view(1);
    }
    MainInterfaceGlobal.draw_edit_pane();
}

// ORIGINAL: 0x00510A60 ?prev_cursor@Console@@QAEXXZ 0x00510A60-0x00510B70 FILE BYTE_EXACT
// size      272 bytes
// prototype void (__thiscall ?prev_cursor@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   10
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00446B40 0x00458900 0x00460DD0 0x0046A550 0x0046AD60 0x0046AE00 0x0046B310 0x0046FB10 0x004B9EA0 0x004B9F90
//
// moved into src/ and was still being written into new files.
//
// address        0x00510A60
//
// emitted scaffolding and the body alone will not reproduce the
//   build/byte-match/00510a60/unit.cpp
//
//
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This


void Console::prev_cursor() {
    const int had_move = survey_mode_;
    field_23BF8_ = 0;
    if (had_move != 0) {
        survey_mode_ = 0;
        console_map_win()->draw_map(1);
    }
    console_map_win()->draw_cursor();
    if (ConsoleGlobal.cursor_active_ != 0) {
        ambience(0x1E);
        const int had_move_now = survey_mode_;
        cursor_active_ = 0;
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        if (had_move_now != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        ::draw_cursor();
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        console_map_win()->main_caption();
        alpha_menu_.check(selected_vehicle_);
    }
    const int cursor_timer = cursor_history_index_ - 1;
    cursor_history_index_ = cursor_timer;
    if (cursor_timer < 0) {
        cursor_history_index_ = 0x1F;
    }
    const int slot = ConsoleGlobal.cursor_history_index_;
    ConsoleStatusWin.set_loc(ConsoleGlobal.cursor_history_x_[slot], ConsoleGlobal.cursor_history_y_[slot]);
    const int slot_now = ConsoleGlobal.cursor_history_index_;
    console_map_win()->focus(ConsoleGlobal.cursor_history_x_[slot_now], ConsoleGlobal.cursor_history_y_[slot_now]);
}

// ORIGINAL: 0x00515600 ?base_names@Console@@QAEXXZ 0x00515600-0x00515638 FILE BYTE_EXACT
// size      56 bytes
// prototype void (__thiscall ?base_names@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0059E5D0
//
// moved into src/ and was still being written into new files.
//
// address        0x00515600
//
// emitted scaffolding and the body alone will not reproduce the
//   build/byte-match/00515600/unit.cpp
//
//
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
//
// HOMED 2026-09-04, BYTE_EXACT 18/18 - and the archive's "documented
// INSTRUCTION SCHEDULING wall" was a scaffold artifact, not a wall. The five
// spellings it recorded as producing the wrong store order all named
// g_009a6494 / g_0094b468 as `int *const` fixed-address bindings; naming them
// GameMorePreferences and AlphaIniPrefs.more_preferences - real globals, so
// VC6 knows the second store cannot alias the first - lets the compiler keep
// the image's own order without help. The lesson is the tree's oldest one
// wearing a third disguise: an opaque binding costs the optimiser the
// aliasing information the image's own compiler had.


void Console::base_names() {
    // The 0x1000 / 0x2000 pair is the show-base-names / show-production
    // toggle: setting either forces 0x1000 on, clearing both clears it.
    const int flags = GameDataStateStorage.fields_.game_more_preferences_;
    int new_flags;
    if (flags & 0x1000) {
        new_flags = flags & 0xFFFFCFFF;
    } else if (flags & 0x2000) {
        new_flags = flags | 0x1000;
    } else {
        new_flags = flags | 0x2000;
    }
    GameDataStateStorage.fields_.game_more_preferences_ = AlphaIniPrefs.more_preferences = new_flags;
    prefs_save(0);
    ::draw_map(1);
}

// ORIGINAL: 0x00515640 ?toggle_flatten@Console@@QAEXXZ 0x00515640-0x00515691 FILE BYTE_EXACT
// size      81 bytes
// prototype void (__thiscall ?toggle_flatten@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0059E5D0 0x005FB760 0x005FB7C0
//
// moved into src/ and was still being written into new files.
//
// address        0x00515640
//
// emitted scaffolding and the body alone will not reproduce the
//   build/byte-match/00515640/unit.cpp
//
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void Console::toggle_flatten() {
    const int flags = GameDataStateStorage.fields_.game_more_preferences_ ^ 0x10000;
    GameDataStateStorage.fields_.game_more_preferences_ = flags;
    if (flags & 0x10000) {
        alpha_menu_.check_menu_item(2, 0x209);
    } else {
        alpha_menu_.uncheck_menu_item(2, 0x209);
    }
    AlphaIniPrefs.more_preferences = GameDataStateStorage.fields_.game_more_preferences_;
    prefs_save(0);
    ::draw_map(1);
}

// ORIGINAL: 0x005156A0 ?toggle_grid@Console@@QAEXXZ 0x005156A0-0x005156EF FILE BYTE_EXACT
// size      79 bytes
// prototype void (__thiscall ?toggle_grid@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0059E5D0 0x005FB760 0x005FB7C0
//
// moved into src/ and was still being written into new files.
//
// address        0x005156A0
//
// emitted scaffolding and the body alone will not reproduce the
//   build/byte-match/005156a0/unit.cpp
//
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void Console::toggle_grid() {
    const int flags = GameDataStateStorage.fields_.game_preferences_ ^ 0x100;
    GameDataStateStorage.fields_.game_preferences_ = flags;
    if (flags & 0x100) {
        alpha_menu_.check_menu_item(2, 0x20A);
    } else {
        alpha_menu_.uncheck_menu_item(2, 0x20A);
    }
    AlphaIniPrefs.preferences = GameDataStateStorage.fields_.game_preferences_;
    prefs_save(0);
    ::draw_map(1);
}

// ORIGINAL: 0x005156F0 ?toggle_base_grid@Console@@QAEXXZ 0x005156F0-0x0051573F FILE BYTE_EXACT
// size      79 bytes
// prototype void (__thiscall ?toggle_base_grid@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0059E5D0 0x005FB760 0x005FB7C0
//
// moved into src/ and was still being written into new files.
//
// address        0x005156F0
// refusal        u005156f0.cpp(49) : error C2065: 'BOOL' : undeclared identifier u005156f0.cpp(49) : error C2146: syntax error : missing ')' before identifier 'saveFactions' u00
//
// emitted scaffolding and the body alone will not reproduce the
//   build/byte-match/005156f0/unit.cpp
//
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
//
//
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This


void Console::toggle_base_grid() {
    const int flags = GameDataStateStorage.fields_.game_preferences_ ^ 0x200;
    GameDataStateStorage.fields_.game_preferences_ = flags;
    if (flags & 0x200) {
        alpha_menu_.check_menu_item(2, 0x20B);
    } else {
        alpha_menu_.uncheck_menu_item(2, 0x20B);
    }
    AlphaIniPrefs.preferences = GameDataStateStorage.fields_.game_preferences_;
    prefs_save(0);
    ::draw_map(1);
}

// ORIGINAL: 0x00515400 ?center@Console@@QAEXXZ 0x00515400-0x00515423 BYTE_EXACT
// size      35 bytes
// prototype void (__thiscall ?center@Console@@QAEXXZ)(Console* this)
// callers   5   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046B1F0
// 0x00515400  ?center@Console@@QAEXXZ  ->  ?center@Console@@QAEXXZ
//
// BLOCKED (2026-09-03, P1 calibration) on a mapwin.h declaration this TU does
// not own: 0x0046B1F0 is ?set_center@MapWin@@QAEXHHH@Z, undeclared. It needs
// `void set_center(int, int, int);` on MapWin and a pending_bodies forwarder
// Deleted seed provenance: git:9f6ee90ae7baa6354fcca006def5561fb956792b:src/recovered/units/0046b1f0.cpp
// (the body itself is src/recovered/units/0046b1f0.cpp, unclaimed, 275
// bytes). Everything the body reads is already bound here: ConsoleGlobal.cursor_history_index_ /
// ConsoleGlobal.cursor_history_x_ / ConsoleGlobal.cursor_history_y_ in console.h and the accessor console_map_win()
// for the single absolute load of 0x007D3C3C the image makes at 0x00515416.
//
// The de-punned body, ready to drop in once set_center is declared:
//
// void Console::center() {
//     const int index = ConsoleGlobal.cursor_history_index_;
//     console_map_win()->set_center(ConsoleGlobal.cursor_history_x_[index], ConsoleGlobal.cursor_history_y_[index], 1);
// }


void Console::center() {
    const int slot = ConsoleGlobal.cursor_history_index_;
    console_map_win()->set_center(ConsoleGlobal.cursor_history_x_[slot], ConsoleGlobal.cursor_history_y_[slot], 1);
}

// ORIGINAL: 0x005169B0 ?base_me@Console@@QAEXXZ 0x005169B0-0x005169E6 BYTE_EXACT
// size      54 bytes
// prototype void (__thiscall ?base_me@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0041A920 0x004E3C60
// 0x005169B0  ?base_me@Console@@QAEXXZ  ->  ?base_me@Console@@QAEXXZ
//
// BLOCKED (2026-09-03, P1 calibration) on a basewin.h declaration this TU does
// not own: 0x0041A920 is BaseWin::zoom, undeclared. It needs a declaration on
// BaseWin and a pending_bodies forwarder. Everything else is already bound:
// base_find (base.h:533, the three-int overload), LocalFaction (game.h:865,
// 0x00939284 - the scaffold's g_00939284), and tutwin.h's
// TutWinBaseWindow for 0x006A7628.
//
// The de-punned body, ready to drop in once zoom is declared:
//
// void Console::base_me() {
//     const int index = ConsoleGlobal.cursor_history_index_;
//     const int result = base_find(ConsoleGlobal.cursor_history_x_[index], ConsoleGlobal.cursor_history_y_[index],
//                                  LocalFaction);
//     if (result >= 0) {
//         TutWinBaseWindow.zoom(result, 0);
//     }
// }


void Console::base_me() {
    const int slot = ConsoleGlobal.cursor_history_index_;
    const int base_id =
        base_find(ConsoleGlobal.cursor_history_x_[slot], ConsoleGlobal.cursor_history_y_[slot], ConsoleGlobal.faction_id_);
    if (base_id >= 0) {
        TutWinBaseWindow.zoom(base_id, 0);
    }
}

// ORIGINAL: 0x00515450 ?survey@Console@@QAEXXZ 0x00515450-0x0051552B FILE BYTE_EXACT
// LEVER: re-read the fixed global for the `this` of the draw_map() call instead of reusing the pointer already cached for the field save/restore - the original re-reads it too
// size      219 bytes
// prototype void (__thiscall ?survey@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046A550


void Console::survey() {
    // Cycle through three survey modes: 0 -> 1 -> 2 -> 3 -> 0. Each
    // step forces one repaint with the map's detail mask pinned to a value
    // that shows only the layer that step is about, then puts the mask back.
    const int state = survey_mode_;
    if (state == 1) {
        survey_mode_ = 2;
        const uint32_t saved = console_map_win()->field_1DD70_;
        console_map_win()->field_1DD70_ = 0x603;
        console_map_win()->draw_map(1);
        console_map_win()->field_1DD70_ = saved;
        return;
    }
    if (state == 2) {
        survey_mode_ = 3;
        const uint32_t saved = console_map_win()->field_1DD70_;
        console_map_win()->field_1DD70_ = 0x402;
        console_map_win()->draw_map(1);
        console_map_win()->field_1DD70_ = saved;
        return;
    }
    if (state != 0) {
        survey_mode_ = 0;
        console_map_win()->draw_map(1);
        return;
    }
    survey_mode_ = 1;
    const uint32_t saved = console_map_win()->field_1DD70_;
    console_map_win()->field_1DD70_ = 0x623;
    console_map_win()->draw_map(1);
    console_map_win()->field_1DD70_ = saved;
}

// ORIGINAL: 0x004E1380 ?editor_clear_terrain@Console@@QAEXXZ 0x004E1380-0x004E13D2 FILE BYTE_EXACT
// size      82 bytes
// prototype void (__thiscall ?editor_clear_terrain@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20
//
// moved into src/ and was still being written into new files.
//
// address        0x004E1380
//
// emitted scaffolding and the body alone will not reproduce the
//   build/byte-match/004e1380/unit.cpp
//
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void Console::editor_clear_terrain() {
    auto_undo();
    // The mask 0x420347A3 is the image's own immediate, kept verbatim - it
    // clears the rocky/lock/using channels of tile->bit in one go.
    Map *tile = map_tiles();
    int i = 0;
    for (; i < static_cast<int>(MapArea); i++, tile++) {
        tile->bit &= 0x420347A3u;
        uint32_t *dst = &tile->bit_visible[0];
        int j = 7;
        do {
            *dst = tile->bit;
            dst++;
        } while (--j != 0);
    }
    ::draw_map(1);
}

// ORIGINAL: 0x00511480 ?set_view@Console@@QAEXH@Z 0x00511480-0x00511518 FILE BYTE_EXACT
// size      152 bytes
// prototype void (__thiscall ?set_view@Console@@QAEXH@Z)(Console* this, int)
// callers   8   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00446B40 0x00458900 0x00460DD0 0x0046A550 0x0046AE00 0x0046FB10 0x004B9EA0


void Console::set_view(int force_update) {
    // The vehicle-cursor repaint cycle, guarded two ways: the global cursor flag
    // says a cursor vehicle exists at all, force_update says the caller wants
    // the repaint regardless (the editor toggles pass 1 so the view follows
    // the flag they just flipped even with no vehicle selected).
    if (ConsoleGlobal.cursor_active_ != 0 || force_update != 0) {
        ambience(0x1E);
        cursor_active_ = 0;
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        ::draw_cursor();
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        console_map_win()->main_caption();
        alpha_menu_.check(selected_vehicle_);
    }
}

// ORIGINAL: 0x004E09E0 ?editor_save@Console@@QAEXXZ 0x004E09E0-0x004E09F9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?editor_save@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ABB90
// 0x004E09E0  ?editor_save@Console@@QAEXXZ  ->  ?editor_save@Console@@QAEXXZ
//
// 0x007AD330 is ConsoleInfoWin's redraw member at +0x90, not independent
// storage. save_map is 0x005ABB90 (?save_map@@YAXXZ).
//
// The image reaches the redraw with `mov ecx, 0x7ad2a0; jmp 0x458910` - a
// tail call on the fixed InfoWin, which is why the C++ is the plain statement
// sequence below rather than anything returning.


void Console::editor_save() {
    save_map();
    ConsoleInfoWin.set_redraw_phase(7);
    ConsoleInfoWin.on_redraw();
}

// ORIGINAL: 0x004E0F30 ?editor_flat@Console@@QAEXXZ 0x004E0F30-0x004E0F7D FILE BYTE_EXACT
// size      77 bytes
// prototype void (__thiscall ?editor_flat@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x005BF310 0x005D5A70
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Homed 2026-09-04. The bindings this body needed were already named in the
// tree: 0x0094988C is map.cpp's MapIsFlat, 0x00689148/0x00689150 are
// console.cpp's StrMapFlat/StrMapRound, and 0x008E9F60 is map.cpp's
// WorldClimateWorldWin (declared here as a HANDOFF row until the shared
// extern lands in worldwin.h).


void Console::editor_flat() {
    auto_undo();
    MapIsFlat ^= 1;
    ::draw_map(1);
    WorldClimateWorldWin.GraphicWin::redraw();
    if (MapIsFlat & 1) {
        X_pop(StrMapFlat, (NullCallback)0);
        return;
    }
    X_pop(StrMapRound, (NullCallback)0);
}

// ORIGINAL: 0x004E10C0 ?editor_fast@Console@@QAEXXZ 0x004E10C0-0x004E118D FILE BYTE_EXACT
// size      205 bytes
// prototype void (__thiscall ?editor_fast@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x005C86E0
// LEVER      reread-global-in-loop-condition  leave the loop bound as a fresh read of the global on every iteration instead of hoisting it into a cached local. This was recorded as a TRIED - the thing that did NOT work - and the body then reached BYTE_EXACT with the opposite spelling, which is exactly the refutation the grammar exists to force. /O2 will not re-read a global it has already loaded unless the source asks again, so caching the bound is a one-instruction difference that persists through the whole loop.
//
// Homed 2026-09-04. The emitter's seven anonymous externs all had names in
// the tree already: editor_state is game.cpp's GameState (0x009A64C0),
// veh_count is map.cpp's MapArea (0x00949884), veh_array is map.h's
// map_tiles() (0x0094A30C), editor_undo_a/b are VehCurrentCount
// (0x009A64C8) and BaseCurrentCount (0x009A64CC), and editor_flag_a/b are
// Historical note, corrected: the pinned MainInterface constructor at
// 0x0045EF10 builds 42 FlatButtons at +0xE00 with stride 0xB4C; the
// receiver at 0x007B0CB8 is element 2 of MainInterfaceGlobal.flat_button_.
// LEVER: the loop bound is `static_cast<int>(MapArea)` and not MapArea
// itself. MapArea is uint32_t (map.h) and the emitter's `veh_count` was
// int, so the plain spelling compiled `jbe` against the image's `jl` -
// signedness, not scheduling, and the only divergence in the body.


void Console::editor_fast() {
    auto_undo();
    unsigned int state = GameDataStateStorage.fields_.game_state_ | 4;
    unsigned char *base = reinterpret_cast<unsigned char *>(map_tiles());
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    GameDataStateStorage.fields_.game_state_ = state;
    for (int i = 0; i < static_cast<int>(MapArea); i++) {
        *reinterpret_cast<unsigned int *>(base + i * 0x2c + 8) &= 0xfffffffc;
    }
    unsigned int flag80 = GameDataStateStorage.fields_.game_state_ & 0x80;
    GameDataStateStorage.fields_.game_state_ = GameDataStateStorage.fields_.game_state_ & 0xffffff7f;
    world_build();
    if (flag80 != 0) {
        GameDataStateStorage.fields_.game_state_ = GameDataStateStorage.fields_.game_state_ | 0x80;
    }
    unsigned char al = *reinterpret_cast<unsigned char *>(&GameDataStateStorage.fields_.game_state_);
    if ((al & 0x80) != 0 && (al & 0x40) != 0) {
        MainInterfaceGlobal.flat_button_[2].show(0);
    } else {
        MainInterfaceGlobal.flat_button_[2].hide();
    }
    ::draw_map(1);
}

// ORIGINAL: 0x00510520 ?cursor_off@Console@@QAEXXZ 0x00510520-0x005105BE FILE BYTE_EXACT
// size      158 bytes
// prototype void (__thiscall ?cursor_off@Console@@QAEXXZ)(Console* this)
// callers   8   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046A550 0x005EC7C0 0x005FCA30
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Homed 2026-09-04. The two raw slots the emitter bound - 0x007D3C3C and the
// exclusive 0x007D3C5C - are mapwin.h's MapWinTable and MapWinTableSlots,
// and 0x1DD74 is the active-window member. The typed walk below preserves
// their eight-slot domain without truncating relocated pointers.
// Historical LEVER: the old loop tail compared CASTED ints, not pointers -
// `reinterpret_cast<int>(entry) < reinterpret_cast<int>(...)`. A pointer
// comparison compiles `jb` where the image has `jl`, and that one byte was
// the whole divergence (39/40 -> 40/40). Runs under the /Oy- set.


void Console::cursor_off() {
    field_23D10_ = 0;
    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window != nullptr && (slot == 0 || window->field_1DD74_ != 0)) {
            if (window->cursor_sprite_ != nullptr ||
                window->cursor_resource_id_ != 0x7f00) {
                window->GraphicWin::set_cursor(0x7f00);
                window->cursor_resource_id_ = 0x7f00;
                window->cursor_sprite_ = nullptr;
            }
            // Preserve the original post-callback reload (0x00510584),
            // while the two cursor stores above still use the old receiver.
            const uint32_t flags = MapWinTable[slot]->field_1DD70_;
            if ((flags & 0xc40000) != 0) {
                MapWinTable[slot]->field_1DD70_ = flags & 0xff3bffff;
                MapWinTable[slot]->draw_map(1);
            }
            do_non_input();
        }
    }
}

// ORIGINAL: 0x00515010 ?set_tech@Console@@QAEXH@Z 0x00515010-0x005150C2 FILE BYTE_EXACT
// size      178 bytes
// prototype void (__thiscall ?set_tech@Console@@QAEXH@Z)(Console* this, int factionID)
// callers   2   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00593340 0x005BEB70 0x005BF310 0x00625E30 0x00625E50
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Homed 2026-09-04. The two slots the emitter bound at 0x0096CDA8 and
// 0x0096CDAC are PlayersData[faction].tech_accumulated (0x3C8) and
// .tech_id_researching (0x3CC) - faction.h declares both, and
// technology.cpp already reads the 0x3D0 sibling. `g_0094f35c[n * 11]` is
// Technology[n].name: Technology is at 0x0094F358, name sits at +4, and
// sizeof(RulesTechnology) is 0x2C, so 11 * 4 lands on the name. The label at
// 0x0068A8D4 reads "TECHCHANGE" out of the image's .data.


void Console::set_tech(int faction_id) {
    int *research = &PlayersData[faction_id].tech_accumulated;
    int *researching = &PlayersData[faction_id].tech_id_researching;
    if (*research != 0 && *researching >= 0) {
        parse_say(0, (int)Technology[*researching].name, -1, -1);
        parse_num(0, *research);
        if (X_pop(StrTechChange, (NullCallback)0) == 0) {
            return;
        }
    }
    int picked = tech_pick(faction_id, 0, -1, 0);
    if (picked >= 0) {
        if (*researching >= 0 && picked != *researching) {
            *research = *research / 2;
        }
        *researching = picked;
    }
    synch_research(faction_id);
}

// ORIGINAL: 0x00517180 ?show_version@Console@@QAEXXZ 0x00517180-0x005171C1 FILE BYTE_EXACT
// size      65 bytes
// prototype void (__thiscall ?show_version@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048C0A0 0x00625EC0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Homed 2026-09-04. The unit here measured NO_COMPILE, not MISMATCH: the
// body only ever needed names this tree already had. The caption slot at
// 0x00691B0C is game.h's ClimatePopupTitle - the same address, so the body
// reuses it rather than binding a second name. The two parse_says inputs are
// .data slots holding char* (0x00691870 -> "RELEASE 2.0", 0x00691874 ->
// "20-Dec-1999"), and the two literals popp takes read straight out of the
// image: "ac_logo_sm.pcx" at 0x0068AA7C, "VERSIONCONTROLX" at 0x0068AA8C.


void Console::show_version() {
    parse_says(0, (LPCSTR)*VersionReleaseTextSlot, -1, -1);
    parse_says(1, (LPCSTR)*VersionBuildDateSlot, -1, -1);
    popp((LPCSTR)*ClimatePopupTitle, StrVersionControlX, 0, StrAcLogoSm,
         (NullCallback)0);
}

// Fixed-slot palette-control bindings carried from 004e0a50.cpp. Tool
// selection and improvement masks are now owned by mapwin.cpp.
// PaintCycleGroup: use-site evidence, see editor_paints
static ButtonGroup *const PaintCycleGroup = (ButtonGroup *)0x007CD1C0;
// PaintCycleSelection: use-site evidence, see editor_paints
static int *const PaintCycleSelection = (int *)0x007CD244;

// ORIGINAL: 0x004E0A50 ?editor_paints@Console@@QAEXH@Z 0x004E0A50-0x004E0ACF FILE
// RECOVERY_BINDING: v1 {"evidence": ["The historical 0x004E0A50 recovery is recorded by the ORIGINAL marker and TRIED notes below.", "src/mapwin.cpp:EditorTerrainRules", "image:0x00691888-0x00691B08 first descriptor words select paint availability"], "reason": "Share a typed paint selection and source-level terrain rules with MapWin instead of reading the original executable's fixed descriptor table; prior TRIED notes below describe the older spelling."}
// TRIED: `a1 > 5` (jl/jle flip at #12, fixed by `a1 >= 6`); `counter = a1 - 6` as one expression (lea vs sub at #15, tried splitting into `counter=a1; counter-=6;`, no change). Same size as original (127/127), mismatch only at #15.
// size      127 bytes
// prototype void (__thiscall ?editor_paints@Console@@QAEXH@Z)(Console* this, int)
// callers   5   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458910 0x0045B550 0x0062B870
//
// 0x007AD330 is ConsoleInfoWin's redraw member; 0x007AD2A0 is the owning
// object and 0x007AE820 is MainInterfaceGlobal. The remaining five slots
// still need ownership recovery.


// TRIED: hoisting the receiver into `InfoWin *const win = &ConsoleInfoWin;`
//   above the first store - the image loads `mov ecx, 0x7ad2a0` at
//   instruction 4, before both stores. No change: &ConsoleInfoWin is a
//   RELOCATION, so VC6 will not fold and hoist it, where the archive's
//   `static int *const g_007ad2a0` was internal and folded. Second wall, at
//   #15: the image `sub esi, 6` reuses the parameter register for the loop
//   counter where this tree `lea edx, [esi - 6]`; splitting the expression
//   did not reach it either (also tried against the archive, see its TRIED).
//   30 of 38 agree; both remaining runs are these two.
void Console::editor_paints(int paint_id) {
    // The original paint-availability walk runs the 20 tool rules high to low;
    // 0x007CD1C0/0x007CD244 are the map-paint cycle ButtonGroup and its
    // selected-slot int (set(result, 1) only when different).
    InfoWin *const win = &ConsoleInfoWin;
    MapEditorPaintId = paint_id;
    ConsoleInfoWin.set_redraw_phase(7);
    win->on_redraw();
    MainInterfaceGlobal.draw_edit_pane();
    int result = paint_id + 0x3f3;
    if (paint_id >= 6 && paint_id < 0x1a) {
        int counter = paint_id - 6;
        for (int limit = 19; limit >= 0; --limit) {
            if (map_editor_paint_mask(limit + 6) == 0 && limit <= counter) {
                --counter;
            }
        }
        result = counter + 0x3f9;
    }
    if (*PaintCycleSelection != result) {
        PaintCycleGroup->set(result, 1);
    }
}

// ORIGINAL: 0x004E0AD0 ?editor_minor@Console@@QAEXXZ 0x004E0AD0-0x004E0AF5 FILE BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?editor_minor@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004E0A50 0x005BF3F0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Homed 2026-09-04. The scaffold's three-argument `X_pop(label, 0x40, 0)` is
// this tree's x_pop_short (0x005BF3F0), the misnaming editor_rockiness
// already documented in console.cpp; the label is console.cpp's own
// StrEditorMinor.


void Console::editor_minor() {
    int paint = x_pop_short(StrEditorMinor, 0x40, (NullCallback)0);
    if (paint >= 0) {
        editor_paints(paint + 0x1a);
    }
}

// ORIGINAL: 0x004D8F40 ?explore@Console@@QAEXH@Z 0x004D8F40-0x004D8FA0 FILE BYTE_EXACT
// size      96 bytes
// prototype void (__thiscall ?explore@Console@@QAEXH@Z)(Console* this, int)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00531020 0x005310F0 0x00531480 0x00593220
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Homed 2026-09-04. The emitter's VehRecord shim existed only because it
// indexed the records from 0x0095282C - &Vehs[0].state - rather than from the
// array front veh.h already declares. Read from Vehs, the two fields are
// Veh::order (+0x11, the byte the shim called byte_d_) and Veh::state (+4,
// the dword it called flags_). The daemon at 0x0093CD90 is netdaemon.h's
// NetDaemonNet.


void Console::explore(int veh_id) {
    int result = NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0);
    if (result == 0) {
        Vehs[veh_id].order = 0;
        Vehs[veh_id].state |= 0x4000;
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
        NetDaemonNet.unlock_veh();
    }
}

// ORIGINAL: 0x004D7CF0 ?designate@Console@@QAEXH@Z 0x004D7CF0-0x004D7DD8 FILE BYTE_EXACT
// size      232 bytes
// prototype void (__thiscall ?designate@Console@@QAEXH@Z)(Console* this, int)
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00531020 0x005310F0 0x00593220 0x00625EC0 0x0062B870
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Homed 2026-09-04. The emitter's five `tbl_ADDR + a1 * 0x34` byte-offset
// tables are all typed arrays the tree already declares: 0x00952832 is
// &Vehs[a1].proto_id, 0x0095282C is &Vehs[a1].state, 0x009AB868 is
// veh.h's VehPrototypes (whose +0x25 byte is VehPrototype::weapon_id) and
// 0x0094AE68 is Weapon[0].offense_rating. The daemon is NetDaemonNet, the
// popup is netmsg.h's NetMsgPrimary, and the two labels read DESIGNATED /
// UNDESIGNATED out of the image.


void Console::designate(int veh_id) {
    int16_t proto_id = Vehs[veh_id].proto_id;
    uint8_t weapon_id = VehPrototypes[proto_id].weapon_id;
    uint8_t offense = static_cast<uint8_t>(Weapon[weapon_id].offense_rating);
    if (offense != 0) {
        int lockResult = NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0);
        if (lockResult == 0) {
            Vehs[veh_id].state ^= 0x80;
            synch_veh(veh_id);
            NetDaemonNet.unlock_veh();
            int16_t proto = Vehs[veh_id].proto_id;
            parse_says(0, VehPrototypes[proto].veh_name, -1, -1);
            if ((Vehs[veh_id].state & 0x80) != 0) {
                NetMsgPrimary.pop(StrDesignated, 5000, 0, 0);
                return;
            }
            NetMsgPrimary.pop(StrUndesignated, 5000, 0, 0);
        }
    }
}


// ORIGINAL: 0x0050E890 ?auto_play_callback@@YAXH@Z 0x0050E890-0x0050E97E BYTE_EXACT
// size      238 bytes
// prototype void (__cdecl ?auto_play_callback@@YAXH@Z)(int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005169F0 0x00625810
// indirect  0x0050E8FE 0x0050E90F 0x0050E939 0x0050E968 0x0050E978
// Homed 2026-09-05 from src/unrecovered/0050e890.cpp (deleted). The
// catalogue's callers-0 is right in spirit: nothing calls it, Console::
// on_key_click (0x005178C0) takes its ADDRESS at 0x00517987 - it is a timer
// callback - the source-owned on_key_click handler below installs it
// through the real Time owner.
//
// Every fixed address in it is a name the tree already owns: AutoPlayCounter
// (0x00939FD0, above), WinModalDepth (0x009B7AE4), GameState's low byte
// (0x009A64C0), ConsoleGlobal (0x009156B0), HandleMain (0x009B7B28),
// WinScreenWidth / WinScreenHeight (0x009B7B1C / 0x009B7B20), and the two
// USER32 imports reached by name - SetCursorPos (0x00669310) and
// PostMessageA (0x00669314).
//
// LEVER (from the artifact, re-measured there on cl 12.00.8168): the
// PostMessageA pointer is a LOCAL in the keystroke branch. Spelled inline,
// VC6 emits `call dword ptr [0x669314]` twice and needs no callee-saved
// register for it; the image's `mov edi, [0x669314]` plus two `call edi` is
// a value held across a call, which only a local gets. That is what makes
// EDI live and puts `push esi; push edi` at 0x0050E8C6 rather than at entry.
// The packed word is an EXPRESSION written out at both call sites, not a
// local: naming it costs a whole-branch register home and a stack slot. And
// `(unsigned short)((y & 0xff) << 8) | (x & 0xff)` - the cast on the high
// half only, MAKEWORD's own shape - is what frees x to live in ESI and y to
// take EBX, the image's allocation.
// TRIED, all measured in the artifact: signed against unsigned x and y
// (no effect); thirteen pack spellings, of which only the one below reaches
// mnemonic #64; locals declared at function top; one shared set of locals
// across both branches; computing the pack before SetCursorPos; `register`,
// `const`, a nested scope, a function-scope variable assigned in both
// branches, assignment inside the first call's callee expression, a `void **`
// read, and a const-qualified or `extern` global for the pointer.
// CLOSED 2026-09-05: the artifact's last instruction - the image loads the
// pointer into ESI after the pack and calls `esi` twice in the mouse branch -
// falls to reaching the import as the DESIGNATOR in the keystroke branch
// (`PostMessageCallback post = PostMessageA;`, no explicit address read) and
// BY NAME in the mouse branch. VC6 dllimports both into registers it can
// reuse for the second pair, `mov esi, [0x669314]` landing after the mask
// exactly where the image puts it. Reading the IAT slot as data
// (`*(PostMessageAFn *)0x00669314`), which is every spelling the artifact
// had, is what forced the load to schedule early. BYTE_EXACT 80 of 80.
typedef int(__stdcall *PostMessageCallback)(HWND, UINT, WPARAM, LPARAM);

void __cdecl auto_play_callback(int) {
    AutoPlayCounter = AutoPlayCounter + 1;
    if (AutoPlayCounter > 100) {
        if (WinModalDepth == 0) {
            unsigned char state = *reinterpret_cast<unsigned char *>(&GameDataStateStorage.fields_.game_state_);
            if ((state & 2) != 0) {
                ConsoleGlobal.end_my_turn();
                AutoPlayCounter = 0;
            }
        }
    }

    if (random(0, 2) != 0) {
        int key = (signed char)random(0, 0xff);
        PostMessageCallback post = PostMessageA;
        post(HandleMain, WM_KEYDOWN, key, 0);
        post(HandleMain, WM_KEYUP, key, 0);
        return;
    }

    int x = random(0, WinScreenWidth);
    int y = random(0, WinScreenHeight);
    SetCursorPos(x, y);

    PostMessageA(HandleMain, WM_LBUTTONDOWN, 0,
                 static_cast<unsigned short>((y & 0xff) << 8) | (x & 0xff));
    PostMessageA(HandleMain, WM_LBUTTONUP, 0,
                 static_cast<unsigned short>((y & 0xff) << 8) | (x & 0xff));
}

// Compiler-generated Win-branch capture adjustor from the genuine
// MapWin override and virtual GraphicWin base; no handwritten thunk body.
/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
// ORIGINAL: 0x0051DE30 ?on_lose_mouse_capture@thunk4_MapWin@@QAEXXZ 0x0051DE30-0x0051DE3E FILE BYTE_EXACT
// symbol    ?on_lose_mouse_capture@MapWin@@$4PPPPPPPM@CDCI@AEXXZ
// size      14 bytes
// prototype void (__thiscall ?on_lose_mouse_capture@thunk4_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// Compiler-generated Win slot13 adjustor for the genuine MapWin resize
// override. The virtual-base layout owns the receiver adjustment.
/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x0051DE50 ?on_resize@thunk4_MapWin@@QAEXHH@Z 0x0051DE50-0x0051DE5E FILE BYTE_EXACT
// symbol    ?on_resize@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
// size      14 bytes
// prototype void (__thiscall ?on_resize@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// Compiler-generated Win slot17 receiver adjustment for MapWin mouse feedback.
// The original catalogue name remains on the mapping below.
/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0051DE70 ?on_mouse_move@thunk4_MapWin@@QAEXHH@Z 0x0051DE70-0x0051DE7E FILE BYTE_EXACT
// symbol    ?on_mouse_move@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
// size      14 bytes
// prototype void (__thiscall ?on_mouse_move@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Ask the energy priority for one faction and refresh it - seeding the
         parser's per-faction defaults while the prompt loop runs.
// ORIGINAL: 0x004E0120 ?editor_energy@Console@@QAEXXZ 0x004E0120-0x004E020E FILE
// TRIED (seed history): statement order for the two loop-body stores - the
//        original interleaves `push edi` BETWEEN the two default stores
//        (push -1,-1,esi; store; push edi; store; call) where this body's
//        natural statement order schedules both stores before the 4th push.
//        Every spelling lands the same 76 mnemonics; two-instruction swap.
// size      238 bytes
// prototype void (__thiscall ?editor_energy@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0041E790 0x004D9AD0 0x00514880 0x005ABF20 0x005BFD10 0x00625E30 0x00625EC0
*/
void Console::editor_energy() {
    auto_undo();
    const int faction = editor_who(ConsoleGlobal.faction_id_, 1);
    if (faction >= 0) {
        parse_says(0, Players[faction].adj_name_faction, -1, -1);
        // The prompt loop walks factions 1..7: each pass re-derives the
        // parser's gender default from that faction's leader record and feeds
        // the faction label (Players[f].name_leader) plus its unk_48 value to
        // the script parser.
        for (int loop_faction = 1; loop_faction < MaxPlayerNum; ++loop_faction) {
            GenderDefault = Players[loop_faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(loop_faction, Players[loop_faction].name_leader, -1, -1);
            parse_num(loop_faction, PlayersData[loop_faction].unk_48);
        }

        const int answer = X_pop_ask_number(
            StrEditorEnergy, PlayersData[faction].energy_reserves,
            (NullCallback)0);
        if (answer == 0) {
            PlayersData[faction].energy_reserves = ParseNumTable[0];
            TutWinBaseWindow.on_redraw();
            ConsoleGlobal.update_data(0);
        }
    }
}

/*
Purpose: Clear one vehicle's waypoint run and switch every live map window to
         the patrol cursor.
// ORIGINAL: 0x004D7EE0 ?set_patrol@Console@@QAEXH@Z 0x004D7EE0-0x004D7F5F FILE
// TRIED (seed history): reached SHAPE_EXACT in the seed - same 38 mnemonics;
//        remaining diffs are embedded constants (reload `*p` for the
//        draw_base_dest call instead of reusing the cached window local, and
//        the signed slot-pointer compare).
// size      127 bytes
// prototype void (__thiscall ?set_patrol@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   2   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00467970 0x00510490 0x00510520
*/
void Console::set_patrol(int veh_id) {
    Vehs[veh_id].waypoint_count = 0;
    if (field_23D10_ == 8) {
        cursor_off();
        return;
    }
    // The image walks the MapWinTable slot pointers from 0x007D3C3C to the
    // exclusive 0x007D3C5C end: the primary window always repaints, slots
    // 1..7 only when field_1DD74_ says they hold a live view.
    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window != 0 && (slot == 0 || window->field_1DD74_ != 0)) {
            window->field_1DD70_ |= 0x400000;
            window->draw_base_dest(1);
        }
    }
    ConsoleGlobal.cursor_on(8, -1);
}

/*
Purpose: Edit one faction's four AI priority toggles through the checkbox
         popup, store the answer back and resync that faction's AI.
// ORIGINAL: 0x00514F50 ?set_ai@Console@@QAEXH@Z 0x00514F50-0x0051500F FILE
// TRIED (seed history): closest at 171/191 bytes (mnemonic similarity 0.80) -
//        VC6 folds each field access into a `[eax*4+CONST]` SIB scale instead
//        of materialising the full byte offset once, either spelling; not
//        chased further.
// size      191 bytes
// prototype void (__thiscall ?set_ai@Console@@QAEXH@Z)(Console* this, int factionID)
// callers   3   call targets   2
// calls     0x00593310 0x005BF3F0
*/
void Console::set_ai(int faction_id) {
    // The four toggles live at PlayersData + 0x3E0: the third dword of the
    // faction's unk_38 run.
    int *const entry = PlayersData[faction_id].unk_38 + 2;
    unsigned char flags = (entry[0] != 0) ? 1 : 0;
    if (entry[1] != 0) {
        flags |= 2;
    } else {
        flags &= 0xFD;
    }
    if (entry[2] != 0) {
        flags |= 4;
    } else {
        flags &= 0xFB;
    }
    if (entry[3] != 0) {
        flags |= 8;
    } else {
        flags &= 0xF7;
    }
    console_popup_check_state() = flags;
    if (x_pop_short(StrTechRandom, 1, (NullCallback)0) >= 0) {
        const char answer = static_cast<char>(console_popup_check_state());
        entry[0] = answer & 1;
        entry[1] = (answer & 2) >> 1;
        entry[2] = (answer & 4) >> 2;
        entry[3] = (answer & 8) >> 3;
        synch_ai(faction_id);
    }
}

/*
Purpose: Ask which terrain-improvement brush the scenario editor paints with
         and refresh the interface panes around the answer.
// ORIGINAL: 0x004E0EA0 ?editor_set_brush@Console@@QAEXXZ 0x004E0EA0-0x004E0F22 FILE
// RECOVERY_BINDING: v1 {"evidence": ["MainInterfaceGlobal original7AE820; header button_group_[4] begins1E878 with stride94. Element3 is7CD254, its selection field84 is7CD2D8, and the following MainInterface field1EAC8 is7CD2E8.", "Pinned MainInterface constructor45EF10 constructs four ButtonGroups at+1E878 with ctor62B7C0 and dtor4325B0; destructor45EB80 tears down that same owned array. Noncompact init populates group3 with brush buttons1036..1041.", "Pinned MainInterface init4596CD clears its+1EAC8 member; Console key517A0B/517A93 reads that same original cell. The former ConsoleModalState7CD2E8 symbol retains its unbounded identity as a member binding next to MainInterfaceGlobal."], "reason": "Use MainInterface's constructed editor button group and selection member rather than fixed original-address aliases, retaining dialog/redraw order, compact-view guard and conditional selection notification."}
// size      130 bytes
// prototype void (__thiscall ?editor_set_brush@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458910 0x0045B550 0x005BF390 0x0062B870
*/
void Console::editor_set_brush() {
    PopupModalState = 6;
    const int answer =
        x_pop_value_flags(StrBrush, MapEditorBrushId, 0x40, (NullCallback)0);
    PopupModalState = 0;
    if (answer >= 0) {
        MapEditorBrushId = answer;
    }
    ConsoleInfoWin.set_redraw_phase(7);
    ConsoleInfoWin.on_redraw();
    MainInterfaceGlobal.draw_edit_pane();
    if (WinScreenWidth != 0x320) {
        const int brush_button = MapEditorBrushId + 0x40C;
        if (MainInterfaceGlobal.button_group(3).field_84_ != brush_button) {
            MainInterfaceGlobal.button_group(3).set(brush_button, 1);
        }
    }
}

/*
Purpose: Load a saved game: leave the base window, refuse outside an active
         network/PBEM session, then rebuild the desktop or reload the running
         game and redraw the world.
// ORIGINAL: 0x0051BBA0 ?load_proc@Console@@QAEXHH@Z 0x0051BBA0-0x0051BC62 FILE
// size      194 bytes
// prototype void (__thiscall ?load_proc@Console@@QAEXHH@Z)(Console* this, int, int)
// callers   2   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A4C0 0x00460DD0 0x0046B190 0x0058EE60 0x0058F450 0x005AAAB0 0x005D5A70 0x005FCBB0 0x00644DFF
*/
void Console::load_proc(int save_slot, int faction_id) {
    TutWinBaseWindow.exit();
    if (IsMultiplayerNet != 0 && (GameDataStateStorage.fields_.game_state_ & 0x1000) == 0) {
        return;
    }
    if (IsMultiplayerPBEM != 0 && (GameDataStateStorage.fields_.game_state_ & 0x1000) == 0) {
        return;
    }
    if (load_game(save_slot, faction_id) != 0) {
        return;
    }
    if (GameSessionActive == 0) {
        desktop_init(0);
    } else if (game_reload(1, 0) != 0) {
        exit(3);
    }
    if (save_slot != 0) {
        GameDataStateStorage.fields_.game_state_ |= 0xE0;
        do_all_draws();
        ::draw_map(1);
        UnkBitfield1 |= 4;
        WorldClimateWorldWin.GraphicWin::redraw();
    }
    alpha_menu_.check(selected_vehicle_);
}

/*
Purpose: Play out the turn boundary until the game says to stop: drive
         end_of_game, wait out the loop's queued work, and launch the tutorial
         when it was requested.
// ORIGINAL: 0x00513FC0 ?end_of_turn@Console@@QAEXXZ 0x00513FC0-0x0051404E FILE
// size      142 bytes
// prototype void (__thiscall ?end_of_turn@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BC6E0 0x0051F200 0x00524870
*/
void Console::end_of_turn() {
    field_23BE4_ = 1;
    const int saved_group_count = GameDataStateStorage.fields_.vehicle_current_count_;

    if (ConsoleGlobal.cursor_active_ == 0) {
        for (;;) {
            if (field_23BE4_ == 0) break;
            if (field_23BE8_ == 0) break;
            if (ExitTurnLoop != 0) break;

            if (end_of_game(0) != 0) break;
            if (saved_group_count != GameDataStateStorage.fields_.vehicle_current_count_) break;

            if (IsMultiplayerNet != 0) {
                if ((GameDataStateStorage.fields_.game_state_ & 0x800) != 0) break;
            }

            wait_loop();
            if (TutWinTourPending != 0) {
                TutWinTourPending = 0;
                TutorialWindow.tour();
            }
            if (ConsoleGlobal.cursor_active_ != 0) break;
        }
    }

    field_23BE4_ = 0;
}

// ORIGINAL: 0x005165B0 ?zoom_to_upkeep_base@Console@@QAEXXZ 0x005165B0-0x005166C3
// RECOVERY_BINDING: v1 {"evidence": ["The retained 0x005165B0 seed selects the nearest own base carrying Base::state bit 0x10, using the current MapWin camera coordinates and wraps X on round maps.", "base.h names the 0x0097D040 array as Bases with 0x134-byte Base records; its x/y/faction/state members are at offsets 0/2/4/0x30, matching all seed reads.", "MapWin fields 0x1DD9C/0x1DDA0 supply the camera point; map.cpp binds MapLongitude/MapIsFlat/MapLongitudeBounds; tutwin.h binds the image's BaseWin at 0x006A7628 as TutWinBaseWindow."], "reason": "Choose the nearest base of ConsoleGlobal's current faction with the seed's wrapped-map distance calculation and pass its typed index to the established BaseWin zoom path."}
void Console::zoom_to_upkeep_base() {
    const MapWin *map_window = console_map_win();
    const int camera_x = static_cast<int>(map_window->field_1DD9C_);
    const int camera_y = static_cast<int>(map_window->field_1DDA0_);
    int best_distance = 99999;
    int best_base = -1;

    if (GameDataStateStorage.fields_.base_current_count_ > 0) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            const Base &base = Bases[base_id];
            if (base.faction_id_current == ConsoleGlobal.faction_id_ &&
                (base.state & BSTATE_UNK_10) != 0) {
                int dx = abs(camera_x - base.x);
                if ((MapIsFlat & 1) == 0 && dx > static_cast<int>(MapLongitude)) {
                    dx = MapLongitudeBounds - dx;
                }
                const int dy = abs(camera_y - base.y);
                const int abs_dx = abs(dx);
                const int abs_dy = abs(dy);
                const int high = (abs_dx > abs_dy) ? abs_dx : abs_dy;
                const int low = (abs_dx < abs_dy) ? abs_dx : abs_dy;
                const int distance = high - (((abs_dx + abs_dy >> 1) - low) + 1 >> 1);
                if (distance <= best_distance) {
                    best_base = base_id;
                    best_distance = distance;
                }
            }
        }

        if (best_base >= 0) {
            TutWinBaseWindow.zoom(best_base, 0);
        }
    }
}

// 0x004D9AD0 ?editor_who@Console@@QAEHHH@Z - which faction/personality the
// scenario editor is working on. The catalogue, the partition and
// editor_energy's own call list all name this address (span
// 0x004D9AD0-0x004D9DB4); the console.h declaration carries the corrected
// address too.
// The picker calls Dialogs::item on the Popup's +0x21D0 member: original
// popup stack slot -0x5388 plus 0x21D0 equals the item receiver -0x31B8.
// Its automatic destructor owns the historical teardown sequence.
// ORIGINAL: 0x004D9AD0 ?editor_who@Console@@QAEHHH@Z 0x004D9AD0-0x004D9DB4;0x0065B275-0x0065B347 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/004d9ad0.cpp", "source:src/console.cpp:Console::editor_diplomacy", "source:src/faction.h:Player", "source:src/basepop.h:BasePop"], "reason": "Recover the editor faction picker through the typed popup, dialog member, faction nouns and post-start default value; C++ lifetime replaces the seed's explicit teardown. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// TRIED: a plain `Popup popup;` local reproduces the SEH frame, the __alloca_probe (Popup is ~21KB), and the whole close()-then- automatic-dtor cascade over its embedded scroll_/sprite_/ flat_button1_/flat_button2_/heap_ members for free - matched through instruction #11.
// TRIED: MISMATCH #11 'push' vs 'lea', not chased further (950B, heaviest object graph in this batch: Popup->BasePop-> {Scroll{2x FlatButton}, 2x FlatButton, Sprite, Heap} plus separate Dialogs/Spot locals and two `sub_4066c0`-typed objects the emitter could only give a nullary cdecl decl, handled via a `Sub4066c0Thunk` shim class for the thiscall). Exact interleaving of the inlined implicit-dtor chain against the separate Dialogs/Spot/mystery destructors is optimizer scheduling, not fully controlled from source.
int Console::editor_who(int default_faction, int first_faction) {
    Popup popup;
    PopupModalState = 6;
    popup.start(PopupStartCaption, StrEditorWhoListTitle, -1, 0, 0x40);

    for (int faction = first_faction < 0 ? 0 : first_faction;
         faction < MaxPlayerNum; ++faction) {
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        popup.dialogs().item(Players[faction].noun_faction, faction);
    }
    if (default_faction >= 0) {
        popup.set_value(default_faction);
    }
    const int selected = popup.exec(0, 0);
    PopupModalState = 0;
    return selected;
}

// Scenario-editor configuration dialogs recovered from their original
// routines.  These dialogs stage their check states in Popup's shared
// check-state word, then copy accepted values back to the selected faction.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004DBD20 ?editor_personality@Console@@QAEXXZ 0x004DBD20-0x004DBFA8
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004dbd20.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_personality() {
    const int faction = editor_who(ConsoleGlobal.faction_id_, 1);
    if (faction < 0) {
        return;
    }
    auto_undo();

    GenderDefault = Players[faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[faction].title_leader, -1, -1);
    GenderDefault = Players[faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[faction].name_leader, -1, -1);
    GenderDefault = Players[faction].noun_gender;
    PluralityDefault = Players[faction].is_noun_plural;
    parse_says(2, Players[faction].noun_faction, -1, -1);

    PlayerData &data = PlayersData[faction];
    unsigned int checks = PopupWriteCheckState;
    checks = data.ai_growth ? checks | 1u : checks & ~1u;
    checks = data.ai_tech ? checks | 2u : checks & ~2u;
    checks = data.ai_wealth ? checks | 4u : checks & ~4u;
    checks = data.ai_power ? checks | 8u : checks & ~8u;
    checks = data.ai_fight >= 1 ? checks | 0x10u : checks & ~0x10u;
    checks = data.ai_fight < 0 ? checks | 0x20u : checks & ~0x20u;
    checks = (data.flags & 0x00800000u) ? checks | 0x40u : checks & ~0x40u;
    checks = (data.flags & 0x00400000u) ? checks | 0x80u : checks & ~0x80u;
    checks = (data.flags & 0x01000000u) ? checks | 0x100u : checks & ~0x100u;
    checks = (data.flags & 0x02000000u) ? checks | 0x200u : checks & ~0x200u;
    PopupWriteCheckState = (data.flags_ext & 0x20u) ? checks | 0x400u : checks & ~0x400u;

    if (x_pop_short("AIEDITOR", 0x41, nullptr) < 0) {
        return;
    }

    const unsigned int accepted = PopupWriteCheckState;
    data.flags &= ~0x03C00000u;
    data.flags_ext &= ~0x20u;
    data.ai_growth = (accepted & 1u) != 0;
    data.ai_tech = (accepted & 2u) != 0;
    data.ai_wealth = (accepted & 4u) != 0;
    data.ai_power = (accepted & 8u) != 0;
    if ((accepted & 0x10u) != 0 && (accepted & 0x20u) == 0) {
        data.ai_fight = 1;
    } else if ((accepted & 0x10u) == 0 && (accepted & 0x20u) != 0) {
        data.ai_fight = -1;
    } else {
        data.ai_fight = 0;
    }
    if (accepted & 0x40u) data.flags |= 0x00800000u;
    if (accepted & 0x80u) data.flags |= 0x00400000u;
    if (accepted & 0x100u) data.flags |= 0x01000000u;
    if (accepted & 0x200u) data.flags |= 0x02000000u;
    if (accepted & 0x400u) data.flags_ext |= 0x20u;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004DBFB0 ?editor_strategy@Console@@QAEXXZ 0x004DBFB0-0x004DC225
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004dbfb0.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_strategy() {
    const int faction = editor_who(ConsoleGlobal.faction_id_, 1);
    if (faction < 0) {
        return;
    }
    auto_undo();

    GenderDefault = Players[faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[faction].title_leader, -1, -1);
    GenderDefault = Players[faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[faction].name_leader, -1, -1);
    GenderDefault = Players[faction].noun_gender;
    PluralityDefault = Players[faction].is_noun_plural;
    parse_says(2, Players[faction].noun_faction, -1, -1);

    PlayerData &data = PlayersData[faction];
    const unsigned int flags = data.flags;
    const unsigned int extended = data.flags_ext;
    unsigned int checks = PopupWriteCheckState;
    checks = (flags & 0x04000000u) ? checks | 1u : checks & ~1u;
    checks = (flags & 0x08000000u) ? checks | 2u : checks & ~2u;
    checks = (flags & 0x10000000u) ? checks | 4u : checks & ~4u;
    checks = (flags & 0x80000000u) ? checks | 8u : checks & ~8u;
    checks = (flags & 0x40000000u) ? checks | 0x10u : checks & ~0x10u;
    checks = (flags & 0x20000000u) ? checks | 0x20u : checks & ~0x20u;
    checks = (flags & 0x00200000u) ? checks | 0x40u : checks & ~0x40u;
    checks = (extended & 1u) ? checks | 0x80u : checks & ~0x80u;
    checks = (extended & 2u) ? checks | 0x100u : checks & ~0x100u;
    checks = (extended & 4u) ? checks | 0x200u : checks & ~0x200u;
    checks = (extended & 8u) ? checks | 0x400u : checks & ~0x400u;
    checks = (extended & 0x10u) ? checks | 0x800u : checks & ~0x800u;
    PopupWriteCheckState = (extended & 0x40u) ? checks | 0x1000u : checks & ~0x1000u;

    const int result = x_pop_short("AIEDITOR2", 0x41, nullptr);
    if (result < 0) {
        return;
    }
    const unsigned int accepted = PopupWriteCheckState;
    data.flags &= ~0xFC200000u;
    data.flags_ext &= ~0x5Fu;
    if (accepted & 1u) data.flags |= 0x04000000u;
    if (accepted & 2u) data.flags |= 0x08000000u;
    if (accepted & 4u) data.flags |= 0x10000000u;
    if (accepted & 8u) data.flags |= 0x80000000u;
    if (accepted & 0x10u) data.flags |= 0x40000000u;
    if (accepted & 0x20u) data.flags |= 0x20000000u;
    if (accepted & 0x40u) data.flags |= 0x00200000u;
    if (accepted & 0x80u) data.flags_ext |= 1u;
    if (accepted & 0x100u) data.flags_ext |= 2u;
    if (accepted & 0x200u) data.flags_ext |= 4u;
    if (accepted & 0x400u) data.flags_ext |= 8u;
    if (accepted & 0x800u) data.flags_ext |= 0x10u;
    if (accepted & 0x1000u) data.flags_ext |= 0x40u;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004DC230 ?editor_rules@Console@@QAEXXZ 0x004DC230-0x004DC515
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004dc230.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_rules() {
    auto_undo();
    for (;;) {
        uint32_t rules = GameDataStateStorage.fields_.game_rules_;
        uint32_t state = GameDataStateStorage.fields_.game_state_ & ~0x01800000u;
        GameDataStateStorage.fields_.game_state_ = state;
        unsigned int checks = (rules >> 11) & 1u;
        checks |= (rules & 2u) ? 2u : 0u;
        checks |= (rules & 8u) ? 4u : 0u;
        checks |= (rules & 4u) ? 8u : 0u;
        checks |= (rules & 0x1000u) ? 0x10u : 0u;
        checks |= (rules & 1u) ? 0x20u : 0u;
        checks |= (rules & 0x10u) ? 0x40u : 0u;
        checks |= (rules & 0x20u) ? 0x80u : 0u;
        checks |= (rules & 0x4000u) ? 0x100u : 0u;
        checks |= (rules & 0x200u) ? 0x200u : 0u;
        checks |= (rules & 0x40u) ? 0x400u : 0u;
        checks |= (rules & 0x100u) ? 0u : 0x800u;
        checks |= (rules & 0x2000u) ? 0x1000u : 0u;
        checks |= (rules & 0x8000u) ? 0x2000u : 0u;
        checks |= (rules & 0x80u) ? 0x4000u : 0u;
        checks |= (rules & 0x400u) ? 0x8000u : 0u;
        checks |= (state & 0x00800000u) ? 0x10000u : 0u;
        checks |= (state & 0x01000000u) ? 0x20000u : 0u;
        PopupWriteCheckState = checks;
        PopupModalState = 6;
        const int result = x_pop_short("RULES", 0x41, nullptr);
        PopupModalState = 0;
        if (result < 0) {
            break;
        }

        rules = GameDataStateStorage.fields_.game_rules_ & 0xFFFF0000u;
        state = GameDataStateStorage.fields_.game_state_ & ~0x01800000u;
        checks = PopupWriteCheckState;
        if (ConsoleGroupAction == 0) {
            if (checks & 1u) rules |= 0x800u;
            if (checks & 2u) rules |= 2u;
            if (checks & 4u) rules |= 8u;
            if (checks & 8u) rules |= 4u;
            if (checks & 0x10u) rules |= 0x1000u;
            if (checks & 0x20u) rules |= 1u;
            if (checks & 0x40u) rules |= 0x10u;
            if (checks & 0x80u) rules |= 0x20u;
            if (checks & 0x100u) rules |= 0x4000u;
            if (checks & 0x200u) rules |= 0x200u;
            if (checks & 0x400u) rules |= 0x40u;
            if (!(checks & 0x800u)) rules |= 0x100u;
            if (checks & 0x1000u) rules |= 0x2000u;
            if (checks & 0x2000u) rules |= 0x8000u;
            if (checks & 0x4000u) rules |= 0x80u;
            if (checks & 0x8000u) rules |= 0x400u;
            if (checks & 0x10000u) state |= 0x00800000u;
            if (checks & 0x20000u) state |= 0x01000000u;
            GameDataStateStorage.fields_.game_rules_ = rules;
            GameDataStateStorage.fields_.game_state_ = state;
            AlphaIniPrefs.rules = rules;
            AlphaIniPrefs.semaphore = state & 0x01800000u;
            prefs_save(0);
            return;
        }
        GameDataStateStorage.fields_.game_rules_ |= default_rules();
        AlphaIniPrefs.rules = GameDataStateStorage.fields_.game_rules_;
        AlphaIniPrefs.semaphore = GameDataStateStorage.fields_.game_state_ & 0x01800000u;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004DD6F0 ?editor_diff@Console@@QAEXXZ 0x004DD6F0-0x004DDA4E;0x0065B872-0x0065B97E
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004dd6f0.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_diff() {
    Popup popup;
    auto_undo();
    popup.start(PopupStartCaption, "EDITORDIFF", -1, nullptr, 0x40);
    for (int i = 0; i < MaxDiffNum; ++i) {
        popup.dialogs().item(StringTable.get(reinterpret_cast<int>(Difficulty[i])), i);
    }
    popup.set_value(GameDataStateStorage.fields_.diff_level_current_);
    const int selected = popup.exec(0, nullptr);
    if (selected >= 0) {
        GameDataStateStorage.fields_.diff_level_current_ = selected;
        for (int faction = 0; faction < MaxPlayerNum; ++faction) {
            PlayersData[faction].diff_level = selected;
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E0290 ?editor_reload@Console@@QAEXXZ 0x004E0290-0x004E05D9;0x0065C0C8-0x0065C1AB
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004e0290.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_reload() {
    Popup popup;
    PopupModalState = 6;
    popup.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40);
    for (int faction_entry = 1; faction_entry < MaxPlayerNum; ++faction_entry) {
        GenderDefault = Players[faction_entry].noun_gender;
        PluralityDefault = Players[faction_entry].is_noun_plural;
        popup.dialogs().item(Players[faction_entry].noun_faction, faction_entry);
    }
    const int faction = popup.exec(0, nullptr);
    PopupModalState = 0;
    if (faction < 0) {
        return;
    }
    const int answer = X_pop_ask("RELOADFAC", 0x50, Players[faction].filename,
                                 nullptr, 0);
    if (answer == 0) {
        strcpy(Players[faction].filename, ParseStrBuffer[0].str);
        strcpy(Players[faction].search_key, ParseStrBuffer[0].str);
        read_faction(faction);
        compute_faction_modifiers(faction);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E05E0 ?editor_eliminate@Console@@QAEXXZ 0x004E05E0-0x004E09A1;0x0065C1AB-0x0065C2AF
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004e05e0.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_eliminate() {
    Popup popup;
    auto_undo();
    PopupModalState = 6;
    popup.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40);
    for (int faction_entry = 1; faction_entry < MaxPlayerNum; ++faction_entry) {
        GenderDefault = Players[faction_entry].noun_gender;
        PluralityDefault = Players[faction_entry].is_noun_plural;
        popup.dialogs().item(Players[faction_entry].noun_faction, faction_entry);
    }
    const int faction = popup.exec(0, nullptr);
    PopupModalState = 0;
    if (faction < 0) {
        return;
    }

    const unsigned char faction_bit = static_cast<unsigned char>(1u << faction);
    uint8_t *status = reinterpret_cast<uint8_t *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    status[0] &= static_cast<uint8_t>(~faction_bit);
    status[1] &= static_cast<uint8_t>(~faction_bit);
    for (int base = GameDataStateStorage.fields_.base_current_count_ - 1; base >= 0; --base) {
        if (Bases[base].faction_id_current == faction) {
            base_kill(base);
        }
    }
    ::draw_map(1);
    eliminate_player(faction, -2);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E0B00 ?editor_terrain@Console@@QAEXXZ 0x004E0B00-0x004E0E9E;0x0065C2AF-0x0065C38C
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004e0b00.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_terrain() {
    Popup popup;
    PopupModalState = 6;
    popup.start(PopupStartCaption, "EDITORTERRAIN", -1, nullptr, 0);
    for (int index = 0; index < MaxTerrainNum; ++index) {
        if (Terraforming[index].bit == 0) {
            continue;
        }
        char text[256];
        text[0] = 0;
        char *land_name = StringTable.get(reinterpret_cast<int>(Terraforming[index].name));
        char *sea_name = StringTable.get(reinterpret_cast<int>(Terraforming[index].name_sea));
        strcat(text, land_name);
        if (strcmp(land_name, sea_name) != 0) {
            strcat(text, "/");
            strcat(text, sea_name);
        }
        popup.dialogs().item(text, index);
        if (index == MapEditorPaintId - 6) {
            popup.set_value(index);
        }
    }
    const int result = popup.exec(0, nullptr);
    editor_paints(result + 6);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004DC520 ?editor_scen_rules@Console@@QAEXXZ 0x004DC520-0x004DC852;0x0065B55F-0x0065B5CC
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004dc520.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_scen_rules() {
    Popup popup;
    auto_undo();
    const uint32_t original_rules = GameDataStateStorage.fields_.game_rules_;
    const uint32_t original_turn_flags = GameDataStateStorage.fields_.net_turn_flags_;
    unsigned int checks = PopupWriteCheckState;
    const unsigned int low = checks & 0xFFu;
    const unsigned int high = (checks >> 8) & 0xFFu;
    unsigned int packed = low;
    packed = (original_rules & 0x00200000u) ? packed | 0x01u : packed & ~0x01u;
    packed = (original_rules & 0x00400000u) ? packed | 0x02u : packed & ~0x02u;
    packed = (original_rules & 0x00800000u) ? packed | 0x04u : packed & ~0x04u;
    packed = (original_rules & 0x04000000u) ? packed | 0x08u : packed & ~0x08u;
    packed = (original_rules & 0x02000000u) ? packed | 0x10u : packed & ~0x10u;
    packed = (original_rules & 0x80000000u) ? packed | 0x20u : packed & ~0x20u;
    packed = (original_rules & 0x00040000u) ? packed | 0x40u : packed & ~0x40u;
    packed = (original_rules & 0x00020000u) ? packed | 0x80u : packed & ~0x80u;
    unsigned int packed_high = high;
    packed_high = (original_rules & 0x00010000u) ? packed_high | 1u : packed_high & ~1u;
    packed_high = (original_turn_flags & 1u) ? packed_high | 2u : packed_high & ~2u;
    packed_high = (original_turn_flags & 2u) ? packed_high | 4u : packed_high & ~4u;
    packed_high = (original_rules & 0x01000000u) ? packed_high | 8u : packed_high & ~8u;
    packed_high = (original_rules & 0x00100000u) ? packed_high | 0x10u : packed_high & ~0x10u;
    packed_high = (original_turn_flags & 4u) ? packed_high | 0x20u : packed_high & ~0x20u;
    packed_high = (original_turn_flags & 8u) ? packed_high | 0x40u : packed_high & ~0x40u;
    PopupWriteCheckState = packed | (packed_high << 8);

    if (x_pop_short("SCENRULES", 0x41, nullptr) < 0) {
        return;
    }

    const unsigned int accepted = PopupWriteCheckState;
    GameDataStateStorage.fields_.game_rules_ &= 0x7808FFFFu;
    GameDataStateStorage.fields_.net_turn_flags_ &= 0xFFFFFFF0u;
    if (accepted & 0x0001u) GameDataStateStorage.fields_.game_rules_ |= 0x00200000u;
    if (accepted & 0x0002u) GameDataStateStorage.fields_.game_rules_ |= 0x00400000u;
    if (accepted & 0x0004u) GameDataStateStorage.fields_.game_rules_ |= 0x00800000u;
    if (accepted & 0x0008u) GameDataStateStorage.fields_.game_rules_ |= 0x04000000u;
    if (accepted & 0x0010u) GameDataStateStorage.fields_.game_rules_ |= 0x02000000u;
    if (accepted & 0x0020u) GameDataStateStorage.fields_.game_rules_ |= 0x80000000u;
    if (accepted & 0x0040u) GameDataStateStorage.fields_.game_rules_ |= 0x00040000u;
    if (accepted & 0x0080u) GameDataStateStorage.fields_.game_rules_ |= 0x00020000u;
    if (accepted & 0x0100u) GameDataStateStorage.fields_.game_rules_ |= 0x00010000u;
    if (accepted & 0x0200u) GameDataStateStorage.fields_.net_turn_flags_ |= 1u;
    if (accepted & 0x0400u) GameDataStateStorage.fields_.net_turn_flags_ |= 2u;
    if (accepted & 0x0800u) GameDataStateStorage.fields_.game_rules_ |= 0x01000000u;
    if (accepted & 0x1000u) GameDataStateStorage.fields_.game_rules_ |= 0x00100000u;
    if (accepted & 0x2000u) GameDataStateStorage.fields_.net_turn_flags_ |= 4u;
    if (accepted & 0x4000u) GameDataStateStorage.fields_.net_turn_flags_ |= 8u;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004DC860 ?editor_scen_victory@Console@@QAEXXZ 0x004DC860-0x004DCCB4;0x0065B5CC-0x0065B6A4
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004dc860.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
void Console::editor_scen_victory() {
    Popup popup;
    unsigned int checks = PopupWriteCheckState;
    const uint32_t rules = GameDataStateStorage.fields_.game_rules_;
    const uint32_t state = GameDataStateStorage.fields_.game_state_;
    checks = (rules & 0x08000000u) ? checks | 0x001u : checks & ~0x001u;
    checks = (rules & 0x10000000u) ? checks | 0x002u : checks & ~0x002u;
    checks = (state & 0x00040000u) ? checks | 0x004u : checks & ~0x004u;
    checks = (rules & 0x40000000u) ? checks | 0x008u : checks & ~0x008u;
    checks = (rules & 0x20000000u) ? checks | 0x010u : checks & ~0x010u;
    checks = (state & 0x02000000u) ? checks | 0x020u : checks & ~0x020u;
    checks = (state & 0x04000000u) ? checks | 0x040u : checks & ~0x040u;
    checks = (state & 0x00008000u) ? checks | 0x080u : checks & ~0x080u;
    checks = (state & 0x80000000u) ? checks | 0x100u : checks & ~0x100u;
    checks = (state & 0x20000000u) ? checks | 0x200u : checks & ~0x200u;
    checks = (state & 0x40000000u) ? checks | 0x400u : checks & ~0x400u;
    checks = (rules & 0x00080000u) ? checks | 0x800u : checks & ~0x800u;
    PopupWriteCheckState = (state & 0x00080000u) ? checks | 0x1000u : checks & ~0x1000u;

    if (x_pop_short("SCENVICTORY", 0x41, nullptr) < 0) {
        return;
    }

    const unsigned int accepted = PopupWriteCheckState;
    GameDataStateStorage.fields_.game_rules_ &= 0x87F7FFFFu;
    GameDataStateStorage.fields_.game_state_ &= 0x19F37FFFu;
    if (accepted & 0x001u) GameDataStateStorage.fields_.game_rules_ |= 0x08000000u;
    if (accepted & 0x002u) GameDataStateStorage.fields_.game_rules_ |= 0x10000000u;
    if (accepted & 0x004u) GameDataStateStorage.fields_.game_state_ |= 0x00040000u;
    if (accepted & 0x008u) GameDataStateStorage.fields_.game_rules_ |= 0x40000000u;
    if (accepted & 0x010u) GameDataStateStorage.fields_.game_rules_ |= 0x20000000u;
    if (accepted & 0x020u) GameDataStateStorage.fields_.game_state_ |= 0x02000000u;
    if (accepted & 0x040u) GameDataStateStorage.fields_.game_state_ |= 0x04000000u;
    if (accepted & 0x080u) GameDataStateStorage.fields_.game_state_ |= 0x00008000u;
    if (accepted & 0x100u) GameDataStateStorage.fields_.game_state_ |= 0x80000000u;
    if (accepted & 0x200u) GameDataStateStorage.fields_.game_state_ |= 0x20000000u;
    if (accepted & 0x400u) GameDataStateStorage.fields_.game_state_ |= 0x40000000u;
    if (accepted & 0x800u) GameDataStateStorage.fields_.game_rules_ |= 0x00080000u;
    if (accepted & 0x1000u) GameDataStateStorage.fields_.game_state_ |= 0x00080000u;

    if (GameDataStateStorage.fields_.game_state_ & 0x04000000u) {
        popup.start(PopupStartCaption, "FACOBJ", -1, nullptr, 0);
        for (int facility = 1; facility <= 64; ++facility) {
            popup.dialogs().item(
                StringTable.get(reinterpret_cast<int>(Facility[facility].name)),
                facility);
        }
        GameDataStateStorage.fields_.scenario_victory_facility_ = static_cast<uint32_t>(popup.exec(0, nullptr));
    }
}

// ORIGINAL: 0x004DF4F0 ?editor_view@Console@@QAEXXZ 0x004DF4F0-0x004DFAD0;0x0065BE73-0x0065BFB1
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004df4f0.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
// TRIED: same raw-buffer + AT(offset) reinterpret_cast approach as 0x005AB600 (Popup/BasePop/Scroll/FlatButton sizes uncertain in this scaffold); VCall::slot001 retyped to take an int (the disasm pushes one arg only on that branch, none on slot002). Compiles, right call order/args/ offsets, sim up to 0.60. No SEH frame, so byte-exact ruled out; not retried given size (1822 bytes).
void Console::editor_view() {
    Popup popup;
    PopupModalState = 6;
    popup.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40);
    uint8_t *status = reinterpret_cast<uint8_t *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if ((status[1] & (1u << faction)) == 0) {
            continue;
        }
        char text[256];
        text[0] = 0;
        strcat(text, Players[faction].formal_name_faction);
        strcat(text, "(");
        strcat(text, Players[faction].noun_faction);
        strcat(text, ")");
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        popup.dialogs().item(text, faction);
        if (faction == ConsoleGlobal.faction_id_) {
            popup.set_value(faction);
        }
    }
    const char *mode = Language == 1 ? "TOUS" : Language == 2 ? "ALLE" : "ALL";
    popup.dialogs().item(const_cast<char *>(mode), 0x63);
    const int selected = popup.exec(0, nullptr);
    PopupModalState = 0;
    if (selected < 0) {
        return;
    }

    if (selected == 0x63) {
        PopupModalState = 6;
        if (X_pop("EDITORCONTROLALL", nullptr) != 0) {
            status[0] = static_cast<uint8_t>(status[1] & 0xFEu);
        }
        GameDataStateStorage.fields_.game_state_ |= 0x80u;
        PopupModalState = 0;
        ::draw_map(1);
        desktop_update();
        return;
    }

    GameDataStateStorage.fields_.game_state_ &= ~0x80u;
    desktop_update();
    const unsigned char view_state = static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
    if ((view_state & 0xC0u) == 0xC0u) {
        MainInterfaceGlobal.flat_button_[2].show(0);
    } else {
        MainInterfaceGlobal.flat_button_[2].hide();
    }
    if ((status[0] & (1u << selected)) == 0) {
        GenderDefault = Players[selected].noun_gender;
        PluralityDefault = Players[selected].is_noun_plural;
        parse_says(0, Players[selected].noun_faction, -1, -1);
        PopupModalState = 6;
        const int answer = X_pop("EDITORCONTROL", nullptr);
        status[0] = answer != 0 ? static_cast<uint8_t>(1u << selected) : 0;
        PopupModalState = 0;
    }

    ConsoleGlobal.faction_id_ = selected;
    const int slot = ConsoleGlobal.cursor_history_index_;
    const int x = ConsoleGlobal.cursor_history_x_[slot];
    const int y = ConsoleGlobal.cursor_history_y_[slot];
    if (base_find(x, y, selected) >= 0) {
        ConsoleGlobal.focus(x, y, selected);
    }
    ConsoleGlobal.set_view(1);
    ::draw_map(1);
}

// ORIGINAL: 0x004E14D0 ?editor_natural@Console@@QAEXH@Z 0x004E14D0-0x004E1E99;0x0065C38C-0x0065C564
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004e14d0.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
// TRIED: `Popup popup;` RAII local with explicit business logic (world_* feature placement switch for a1!=0; landmark search/removal for a1==0, edx==7 vs the second switch->bit-index path) converging on popup.close(); diverges at instr #0 (alloca-probe prologue). RETURN TYPE: the 1/2/3/4/5 values set by the second jump table (0x4E18D2) are consumed immediately as an array index into 0x66F8C4 (never reach `ret` in EAX) - void stands, not a status return.
void Console::editor_natural(int place) {
    Popup popup;
    auto_undo();
    popup.start(PopupStartCaption, place != 0 ? "NATPLACE" : "NATUNPLACE",
                -1, nullptr, 0x40);
    for (int natural = 0; natural < MaxNaturalNum; ++natural) {
        popup.dialogs().item(
            StringTable.get(reinterpret_cast<int>(Natural[natural].name)), natural);
    }
    const int selected = popup.exec(0, nullptr);
    if (selected < 0) {
        return;
    }

    const int cursor = ConsoleGlobal.cursor_history_index_;
    const int x = ConsoleGlobal.cursor_history_x_[cursor];
    const int y = ConsoleGlobal.cursor_history_y_[cursor];
    if (place != 0) {
        switch (selected) {
        case 0: world_crater(x, y); break;
        case 1: world_volcano(x, y, 0); break;
        case 2: world_monsoon(x, y); break;
        case 3: world_diamond(x, y); break;
        case 4: world_sargasso(x, y); break;
        case 5: world_ruin(x, y); break;
        case 6: world_dune(x, y); break;
        case 7: world_fresh(x, y); break;
        case 8: world_mesa(x, y); break;
        case 9: world_canyon(x, y); break;
        case 10: world_geothermal(x, y); break;
        case 11: world_ridge(x, y); break;
        case 12: world_borehole(x, y); break;
        case 13: world_temple(x, y); break;
        case 14: world_unity(x, y); break;
        case 15: world_fossil(x, y); break;
        }
        world_climate();
        ::draw_map(1);
        return;
    }

    if (selected == LM_FRESH) {
        const int center_offset = static_cast<int>(MapLongitude) * y + (x >> 1);
        const Map &center = MapTiles[center_offset];
        if ((center.bit2 & BIT2_FRESH) != 0 && (center.climate & 0xE0u) < 0x60u) {
            const uint8_t region = center.region;
            for (int row = 0; row < static_cast<int>(MapLatitudeBounds); ++row) {
                for (int column = 0; column < static_cast<int>(MapLongitudeBounds); column += 2) {
                    Map &tile = MapTiles[static_cast<int>(MapLongitude) * row + (column >> 1)];
                    if (tile.region != region) {
                        continue;
                    }
                    bit2_set(column, row, BIT2_FRESH, 0);
                    const int landmark = find_landmark(column, row, 1);
                    if (landmark >= 0 &&
                        strcmp(MapLandmark[landmark].name,
                               StringTable.get(reinterpret_cast<int>(Natural[LM_FRESH].name))) == 0) {
                        kill_landmark(MapLandmark[landmark].x, MapLandmark[landmark].y);
                    }
                }
            }
        }
        world_climate();
        ::draw_map(1);
        return;
    }

    int radius_index;
    switch (selected) {
    case 0: radius_index = 3; break;
    case 1: radius_index = 1; break;
    case 2: radius_index = 5; break;
    case 3: radius_index = 2; break;
    case 4: radius_index = 4; break;
    case 5: case 6: case 8: case 9: case 10: case 11: radius_index = 2; break;
    case 12: case 13: case 15: radius_index = 1; break;
    case 14: radius_index = 2; break;
    default: radius_index = place; break;
    }
    const uint32_t feature_bit = 1u << selected;
    const int limit = RadiusRange[radius_index];
    bool has_clearable_feature = false;
    for (int probe_offset = 0; probe_offset < limit; ++probe_offset) {
        const int tile_x = xrange(x + RadiusOffsetX[probe_offset]);
        const int tile_y = y + RadiusOffsetY[probe_offset];
        if (tile_y < 0 || tile_y >= static_cast<int>(MapLatitudeBounds) ||
            tile_x < 0 || tile_x >= static_cast<int>(MapLongitudeBounds)) {
            continue;
        }
        Map &tile = MapTiles[static_cast<int>(MapLongitude) * tile_y + (tile_x >> 1)];
        if ((tile.bit2 & feature_bit) != 0 && (tile.bit2 & 0xFF000000u) == 0) {
            has_clearable_feature = true;
            break;
        }
    }
    if (!has_clearable_feature) return;

    for (int clear_offset = 0; clear_offset < limit; ++clear_offset) {
        const int tile_x = xrange(x + RadiusOffsetX[clear_offset]);
        const int tile_y = y + RadiusOffsetY[clear_offset];
        if (tile_y < 0 || tile_y >= static_cast<int>(MapLatitudeBounds) ||
            tile_x < 0 || tile_x >= static_cast<int>(MapLongitudeBounds)) {
            continue;
        }
        Map &tile = MapTiles[static_cast<int>(MapLongitude) * tile_y + (tile_x >> 1)];
        if ((tile.bit2 & feature_bit) == 0) continue;

        bit2_set(tile_x, tile_y, feature_bit, 0);
        code_set(tile_x, tile_y, 0);
        if (selected == 4) {
            bit_set(tile_x, tile_y, 0x20, 0);
        } else if (selected == 5) {
            bit_set(tile_x, tile_y, 0x00102020, 0);
        } else if (selected == 12) {
            bit_set(tile_x, tile_y, 0x01100000, 0);
        }
        const int landmark = find_landmark(tile_x, tile_y, 1);
        if (landmark >= 0 &&
            strcmp(MapLandmark[landmark].name,
                   StringTable.get(reinterpret_cast<int>(Natural[selected].name))) == 0) {
            kill_landmark(MapLandmark[landmark].x, MapLandmark[landmark].y);
        }
    }
    world_climate();
    ::draw_map(1);
}

// Historical seed finding: original 65BFBC is a borrowed-frame Popup cleanup at EBP-0x5398, not a standalone function; the isolated constant-null Popup pointer scaffold diverged at instruction 0 (7/11 bytes). Preserve as a failed standalone experiment, not candidate implementation.
// ORIGINAL: 0x0065BFBC sub_65bfbc 0x0065BFBC-0x0065BFC7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech prologue at 0x004DFADB registers handler 0x0065C0BE; its FuncInfo unwind row at 0x0067ABD8 names action 0x0065BFBC", "src:console.cpp Console::editor_tech contains the branch-local automatic Popup who at the source location carrying this child identity", "src:Popup is a BasePop with its real automatic branch-scoped destructor; no standalone callable definition is supplied for the child"], "reason": "Retain 0x0065BFBC as the generated cleanup action of editor_tech's branch-local automatic Popup who; this borrowed-frame EH child is not a standalone function body, and generated-action matching remains an explicit obligation."}
// kind      game
// Historical seed finding: original 65C001 is a borrowed-frame BaseButton cleanup at EBP-0x14; the isolated constant-null BaseButton pointer scaffold diverged at instruction 0 (7/8 bytes). Preserve as a failed standalone experiment, not candidate implementation.
// ORIGINAL: 0x0065C001 sub_65c001 0x0065C001-0x0065C009 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech prologue registers handler 0x0065C0BE; its FuncInfo unwind row at 0x0067AC08 names action 0x0065C001", "pinned:at 0x4DFEF1 [ebp-0x14]=&[ebp-0x16BC]; FlatButton::close at 0x4DFF04 then BaseButton::~ at 0x4DFF13", "pinned:Popup constructor at 0x4DFDC5 uses this=[ebp-0x5398]; its Scroll member is constructed at 0x004048C4 with this+0x3230", "pinned:Scroll constructor at 0x006051F5 constructs flat_button_left_ at this+0xAAC; 0x00605204 constructs flat_button_right_ at this+0x15F8; BaseButton construction is at 0x00607CF4", "src:Popup::scroll_ and Scroll::flat_button_left_ are the actual nested typed owner; FlatButton directly derives from BaseButton", "src:Scroll teardown owns flat_button_left_; FlatButton destructor owns ordinary BaseButton base teardown"], "reason": "Retain 0x0065C001 as the generated BaseButton cleanup for Popup who's typed member path scroll_.flat_button_left_; it is not a standalone function body, and exact generated-action matching remains an explicit obligation."}
// kind      game
// Historical seed group (inactive): the 23 cleanup identities below were formerly EXCLUDED S2a because C++ EH unwind funclets borrow the enclosing editor_tech frame (docs/EXCLUSIONS.md section 2a). That was a generated-code categorization, not absent runtime lifetime behavior. The FILE bindings below are nonclaims; exact current generated-action and unwind-state correspondence remains pending.
// ORIGINAL: 0x0065BFB1 sub_65bfb1 0x0065BFB1-0x0065BFBC FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech constructs Popup popup at EBP-0xA714 in the 0x004DFAFB call", "pinned:Popup ctor 0x004048A0 constructs BasePop then Scroll at this+0x3230 at 0x004048C4", "source:src/console.cpp Console::editor_tech owns the real automatic Popup popup; src/popup.h declares its real destructor and scroll_ member"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Console::editor_tech automatic Popup popup; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065BFC7 sub_65bfc7 0x0065BFC7-0x0065BFD2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech constructs Popup who at EBP-0x5398 in the 0x004DFDCB call", "pinned:Popup ctor 0x004048BF calls BasePop ctor on unchanged Popup this", "source:src/console.cpp declares Popup who in the input-poll branch; src/popup.h Popup derives from BasePop"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: branch-local Popup who BasePop base and owned-member teardown; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065BFD2 sub_65bfd2 0x0065BFD2-0x0065BFDD FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Popup ctor 0x004048C4 constructs Scroll at this+0x3230", "pinned:Popup who receiver EBP-0x5398 plus 0x3230 equals Scroll receiver EBP-0x2168", "source:src/popup.h declares Scroll scroll_ at 0x3230; src/scroll.h declares the real Scroll destructor"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.scroll_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065BFDD sub_65bfdd 0x0065BFDD-0x0065BFE5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:the parent stores the Scroll subobject receiver EBP-0x2168 in its temporary receiver slot before the cleanup sequence", "pinned:Scroll ctor 0x006051EE constructs its GraphicWin base on unchanged this", "source:src/scroll.h Scroll directly derives from GraphicWin"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.scroll_ Scroll/GraphicWin base; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065BFE5 sub_65bfe5 0x0065BFE5-0x0065BFF3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Scroll ctor 0x006051F5 constructs left FlatButton at this+0xAAC", "pinned:Popup who Scroll is EBP-0x2168; plus 0xAAC is left button EBP-0x16BC", "source:src/scroll.h declares flat_button_left_ at 0xAAC"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.scroll_.flat_button_left_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065BFF3 sub_65bff3 0x0065BFF3-0x0065C001 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Scroll ctor 0x00605204 constructs right FlatButton at this+0x15F8", "pinned:Popup who Scroll is EBP-0x2168; plus 0x15F8 is right button EBP-0xB70", "source:src/scroll.h declares flat_button_right_ at 0x15F8"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.scroll_.flat_button_right_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C009 sub_65c009 0x0065C009-0x0065C011 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:FlatButton ctor 0x00607CF4 calls BaseButton ctor with unchanged ECX", "source:src/flatbutton.h FlatButton derives from BaseButton; source retains both Scroll FlatButton objects"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.scroll_ FlatButton BaseButton subobject; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C011 sub_65c011 0x0065C011-0x0065C01C FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Popup who is constructed at EBP-0x5398; BasePop ctor 0x00600888 constructs the GraphicWin base on unchanged this", "source:src/basepop.h BasePop derives from GraphicWin; src/popup.h Popup derives from BasePop"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who BasePop/GraphicWin base chain; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C01C sub_65c01c 0x0065C01C-0x0065C027 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:BasePop ctor 0x0060088D passes this+0xA28 to Heap construction", "pinned:Popup who receiver EBP-0x5398 plus 0xA28 equals cleanup receiver EBP-0x4970", "source:src/basepop.h declares Heap heap_ at 0xA28"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.heap_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C027 sub_65c027 0x0065C027-0x0065C032 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:BasePop ctor 0x0060089C constructs FlatButton at this+0xA5C", "pinned:Popup who receiver EBP-0x5398 plus 0xA5C equals cleanup receiver EBP-0x493C", "source:src/basepop.h declares flat_button1_ at 0xA5C"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.flat_button1_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C032 sub_65c032 0x0065C032-0x0065C03D FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:BasePop ctor 0x006008AC constructs FlatButton at this+0x15A8", "pinned:Popup who receiver EBP-0x5398 plus 0x15A8 equals cleanup receiver EBP-0x3DF0", "source:src/basepop.h declares flat_button2_ at 0x15A8"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.flat_button2_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C03D sub_65c03d 0x0065C03D-0x0065C048 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:BasePop ctor 0x006008BC passes this+0x2118 to Sprite construction", "pinned:Popup who receiver EBP-0x5398 plus 0x2118 equals cleanup receiver EBP-0x3280", "source:src/basepop.h declares Sprite sprite_ at 0x2118"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.sprite_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C048 sub_65c048 0x0065C048-0x0065C053 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:BasePop ctor initializes the 0x30-byte StringList object at this+0x2150", "pinned:Popup who receiver EBP-0x5398 plus 0x2150 equals cleanup receiver EBP-0x3248", "source:src/basepop.h declares StringList item_ids_ at 0x2150"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.item_ids_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C053 sub_65c053 0x0065C053-0x0065C05E FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:BasePop ctor initializes the second 0x30-byte StringList object at this+0x2180", "pinned:Popup who receiver EBP-0x5398 plus 0x2180 equals cleanup receiver EBP-0x3218", "source:src/basepop.h declares StringList unit_ids_ at 0x2180"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.unit_ids_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C05E sub_65c05e 0x0065C05E-0x0065C069 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:BasePop ctor 0x006009BF passes this+0x21D0 to Dialogs construction", "pinned:Popup who receiver EBP-0x5398 plus 0x21D0 equals cleanup receiver EBP-0x31C8", "source:src/basepop.h declares the actual Dialogs dialogs_ member at 0x21D0; src/dialogs.h declares real direct bases in ListBox, RadioButton, CheckBox, SpriteBox, EditGroup order", "source:current generated Dialogs teardown runs the source-owned base destructor chain in reverse declaration order, then the shared Dialog/GraphicWin virtual-base stages; source member dtors preserve the nested cleanup operations"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.dialogs_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C069 sub_65c069 0x0065C069-0x0065C074 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:BasePop ctor 0x006009D1 passes this+0x3098 to Spot construction", "pinned:Popup who receiver EBP-0x5398 plus 0x3098 equals cleanup receiver EBP-0x2300", "source:src/basepop.h declares Spot spot_ at 0x3098"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup who.spot_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C074 sub_65c074 0x0065C074-0x0065C07F FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:the 0x0065C074 action uses the same receiver as the enclosing Popup at EBP-0xA714", "pinned:Popup ctor 0x004048BF calls BasePop ctor on unchanged Popup this", "source:src/popup.h Popup derives from BasePop; src/basepop.h declares the actual BasePop base/member lifetimes"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup popup BasePop base and owned-member teardown; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C07F sub_65c07f 0x0065C07F-0x0065C08A FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Popup ctor 0x004048C4 constructs Scroll at this+0x3230", "pinned:editor_tech Popup receiver EBP-0xA714 plus 0x3230 equals Scroll receiver EBP-0x74E4", "source:src/popup.h declares Scroll scroll_ at the BasePop-sized offset 0x3230; src/scroll.h declares its real destructor"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup popup.scroll_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C08A sub_65c08a 0x0065C08A-0x0065C092 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech stores the Scroll subobject receiver EBP-0x74E4 in its temporary receiver slot before the cleanup sequence", "pinned:Scroll ctor 0x006051EE constructs its GraphicWin base on unchanged this", "source:src/scroll.h Scroll directly derives from GraphicWin; Scroll's real destructor owns base teardown"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup popup.scroll_ Scroll/GraphicWin base; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C092 sub_65c092 0x0065C092-0x0065C0A0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Scroll ctor 0x006051F5 constructs the left FlatButton at this+0xAAC", "pinned:original receiver arithmetic gives EBP-0x74E4+0xAAC for the left Scroll button", "source:src/scroll.h declares flat_button_left_ at 0xAAC; src/flatbutton.h declares the typed FlatButton member"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup popup.scroll_.flat_button_left_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C0A0 sub_65c0a0 0x0065C0A0-0x0065C0AE FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Scroll ctor 0x00605204 constructs the right FlatButton at this+0x15F8", "pinned:original receiver arithmetic gives EBP-0x74E4+0x15F8 for the right Scroll button", "source:src/scroll.h declares flat_button_right_ at 0x15F8; src/flatbutton.h declares the typed FlatButton member"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup popup.scroll_.flat_button_right_; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C0AE sub_65c0ae 0x0065C0AE-0x0065C0B6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:FlatButton ctor at 0x00607CF4 calls BaseButton ctor with unchanged ECX", "source:src/flatbutton.h FlatButton publicly derives from BaseButton and its destructor retains ordinary base teardown"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup popup.scroll_ FlatButton BaseButton subobject; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065C0B6 sub_65c0b6 0x0065C0B6-0x0065C0BE FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Scroll ctor constructs distinct left/right FlatButtons at 0x006051F5 and 0x00605204; each FlatButton ctor reaches BaseButton at 0x00607CF4", "source:src/scroll.h owns two distinct FlatButton members; src/flatbutton.h FlatButton derives from BaseButton"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Popup popup.scroll_ sibling FlatButton BaseButton subobject; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x004DFAD0 ?editor_tech@Console@@QAEXXZ 0x004DFAD0-0x004E0118;0x0065C0BE-0x0065C0C8
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004dfad0.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
// TRIED: the SEH __try/__finally frame and its per-object unwind-state byte (tracked at ebp-4 in the original, one value per live local); a real nested class model for the two Popup dialogs and their widget cascades, since the scaffold's BasePop/Popup are missing the Dialogs/Dialog/Spot members this function's *second* popup actually has - a raw ebp-relative buffer addressed the same way the disassembly does instead, so every call lands on the right bytes without needing the class widened.
void Console::editor_tech() {
    Popup popup;
    auto_undo();
    int faction = ScenarioTechFaction;
    if (faction < 0) faction = ConsoleGlobal.faction_id_;

    for (;;) {
        ScenarioTechFaction = faction;
        PopupModalState = 6;
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        parse_says(0, Players[faction].noun_faction, -1, -1);
        popup.start(PopupStartCaption, "EDITORTECH", -1, nullptr, 0x42);
        popup.set_cancel_text(label_get(270));
        popup.set_ok_text(label_get(1008));

        for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
            const RulesTechnology &entry = Technology[tech_id];
            if (entry.preq_tech_1 < TechNone || entry.preq_tech_2 < TechNone) {
                continue;
            }
            StringTemp[0] = 0;
            say_tech(StringTemp, tech_id, 0);
            strcat(StringTemp, "|");
            const int status = has_tech(tech_id, faction) ? 246 :
                tech_avail(tech_id, faction) ? 247 : 248;
            strcat(StringTemp, label_get(status));
            popup.dialogs().item(StringTemp, tech_id);
        }
        popup.item_ids_.sort(0);
        if (ScenarioTechSelection >= 0) {
            popup.set_value(ScenarioTechSelection);
        }

        const int selected = popup.exec(0, nullptr);
        PopupModalState = 0;
        if (selected < 0) {
            see_map_check();
            return;
        }
        if (popup.input_poll_state() == 1) {
            Popup who;
            who.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40);
            for (int player = 1; player < MaxPlayerNum; ++player) {
                GenderDefault = Players[player].noun_gender;
                PluralityDefault = Players[player].is_noun_plural;
                who.dialogs().item(Players[player].noun_faction, player);
            }
            if (faction >= 0) who.set_value(faction);
            PopupModalState = 6;
            const int chosen = who.exec(0, nullptr);
            PopupModalState = 0;
            if (chosen > 0) faction = chosen;
            continue;
        }

        if (has_tech(selected, faction)) {
            int ignored_a = 0;
            int ignored_b = 0;
            bitmask(selected, &ignored_a, &ignored_b);
            GameDataStateStorage.fields_.game_tech_achieved_[selected] = static_cast<uint8_t>(
                GameDataStateStorage.fields_.game_tech_achieved_[selected] & ~(1u << faction));
        } else {
            tech_achieved(faction, selected, 0, 0);
        }
        ScenarioTechSelection = selected;
        PlayersData[faction].tech_ranking = 0;
        for (int owned_tech = 0; owned_tech < MaxTechnologyNum; ++owned_tech) {
            if (has_tech(owned_tech, faction)) PlayersData[faction].tech_ranking += 2;
        }
        TutWinBaseWindow.redraw();
    }
}

// ORIGINAL: 0x004DED00 ?editor_veh@Console@@QAEXXZ 0x004DED00-0x004DF4E5;0x0065BD30-0x0065BE73
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004ded00.cpp records the original entry and full body/unwind spans; retained seed is not accepted source structure.", "Task console-scenario-owner-wave and its native grants authorize the thirteen owner methods, typed persistent editor state, existing Popup item sorting, and genuine technology-bit storage. Original instructions resolve callee argument roles and field ownership; dual configured compiler trials and host controls are diagnostics, not acceptance."], "reason": "Recover the selected scenario-editor operation in its existing typed Console owner, retaining its original spans, seed provenance and historical obligations."}
// TRIED: real RAII locals (Popup for the outer/who dialogs, plus individually-declared Scroll/FlatButton/BasePop/Spot/ Dialogs/Dialog/StringStruct/Sprite/Heap locals for the tile-edit-window cleanup block) so VC6 emits the SEH prologue/epilogue and destructor cascade itself, with item()/veh_avail/veh_init/bit_set/synch_bit/draw_tile argument order verified against raw disasm; compiles, MISMATCH #2 (this function has ~15 embedded local aggregates sharing/reusing stack ranges across mutually exclusive branches - true byte-exactness needs the exact source-level object layout, not attempted further here).
void Console::editor_veh() {
    Popup popup;
    auto_undo();
    if (ScenarioVehicleFaction < 0) ScenarioVehicleFaction = ConsoleGlobal.faction_id_;

    for (;;) {
        popup.start(PopupStartCaption, "EDITORUNIT", -1, nullptr, 0x40, nullptr);
        for (int proto_idx = 0; proto_idx < MaxVehProtoNum; ++proto_idx) {
            const VehPrototype &entry = VehPrototypes[proto_idx];
            if ((entry.flags & 1u) == 0) continue;
            if (ScenarioVehicleMode == 0 && proto_idx >= 64 &&
                !veh_avail(proto_idx, ScenarioVehicleFaction, -1)) {
                continue;
            }
            StringTemp[0] = 0;
            strcat(StringTemp, entry.veh_name);
            popup.dialogs().item(StringTemp, proto_idx);
            if (proto_idx == ScenarioPreviousPrototype) popup.set_value(proto_idx);
        }
        const int selected_prototype = popup.exec(0, nullptr);
        PopupModalState = 0;
        if (selected_prototype < 0) return;
        ScenarioPreviousPrototype = selected_prototype;
        const uint32_t action = popup.input_poll_state();
        if (action == 1) {
            ScenarioVehicleMode = ScenarioVehicleMode == 0 ? 1 : 0;
            continue;
        }
        if (action == 2) {
            Popup who;
            who.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40);
            for (int faction = 0; faction < MaxPlayerNum; ++faction) {
                GenderDefault = Players[faction].noun_gender;
                PluralityDefault = Players[faction].is_noun_plural;
                who.dialogs().item(Players[faction].noun_faction, faction);
            }
            who.set_value(ScenarioVehicleFaction);
            PopupModalState = 6;
            const int chosen = who.exec(0, nullptr);
            PopupModalState = 0;
            if (chosen >= 0) ScenarioVehicleFaction = chosen;
            continue;
        }
        if (action == 3) {
            popup.init(0, 0);
            for (int morale = 0; morale < MaxMoraleNum; ++morale) {
                popup.dialogs().item(StringTable.get(reinterpret_cast<int>(Morale[morale].name)), morale);
            }
            const int chosen = popup.exec(0, nullptr);
            PopupModalState = 0;
            ScenarioVehicleMorale = chosen;
            continue;
        }

        const int cursor = ConsoleGlobal.cursor_history_index_;
        const int x = ConsoleGlobal.cursor_history_x_[cursor];
        const int y = ConsoleGlobal.cursor_history_y_[cursor];
        if (selected_prototype == 0x13) {
            bit_set(x, y, 0x20, 1);
            ScenarioVehicleFaction = 0;
            bit_set(x, y, 0x0028805C, 0);
            for (int bit = 1; bit <= 7; ++bit) synch_bit(x, y, bit);
            draw_tile(x, y, 1);
        }
        const int vehicle = veh_init(selected_prototype, ScenarioVehicleFaction, x, y);
        if (vehicle >= 0) {
            Vehs[vehicle].morale = static_cast<uint8_t>(ScenarioVehicleMorale);
            const int prototype_id = Vehs[vehicle].proto_id;
            draw_tile_fixup(x, y, VehPrototypes[prototype_id].flags & 2u, 2);
            if (Vehs[vehicle].faction_id == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.selected_vehicle_ = vehicle;
                if (ConsoleGlobal.cursor_active_) set_move(1);
            }
            uint8_t *const alive = reinterpret_cast<uint8_t *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_) + 1;
            *alive |= static_cast<uint8_t>(1u << ScenarioVehicleFaction);
        }
        return;
    }
}

/*
Purpose: Order one air unit on a bombing run against a tile: refuse unless the
         tile's base owner is at vendetta with us, lock the unit over the net,
         restate its order and resync.
// ORIGINAL: 0x004D8CD0 ?set_bombing_run@Console@@QAEXHHH@Z 0x004D8CD0-0x004D8DBE FILE
// TRIED (seed history): computing the veh_id*0xd flag-slot index once vs
//        re-indexing read+write - both compile; landed at MISMATCH (edit_count
//        3, 97% mnemonic similarity) with the array-indexed form.
// size      238 bytes
// prototype void (__thiscall ?set_bombing_run@Console@@QAEXHHH@Z)(Console* this, int vehID, int xCoord, int yCoord)
// callers   2   call targets   6
// calls     0x0047A890 0x004E3A50 0x00531020 0x005310F0 0x00531480 0x00593220
*/
void Console::set_bombing_run(int veh_id, int x_coord, int y_coord) {
    const int base_id = base_at(x_coord, y_coord);
    // The gate is the base owner's diplomacy word: no vendetta against that
    // faction, no bombing run.
    if (base_id < 0 ||
        (PlayersData[Vehs[veh_id].faction_id]
             .diplo_treaties[Bases[base_id].faction_id_current] &
         DTREATY_VENDETTA) == 0) {
        NetMsgPrimary.pop(StrNoBomb, 5000, 0, 0);
        return;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return;
    }
    // Drop the two stale order bits (0x03000000) and mark the 0x200 order
    // bit, then park the target in waypoint slot 1 as an auto bombing run.
    Vehs[veh_id].state = (Vehs[veh_id].state & 0xFCFFFFFFu) | 0x200;
    Vehs[veh_id].order_auto_type = ORDERA_BOMBING_RUN;
    Vehs[veh_id].waypoint_x[1] = static_cast<int16_t>(x_coord);
    Vehs[veh_id].waypoint_y[1] = static_cast<int16_t>(y_coord);
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
}

/*
Purpose: Send one vehicle home: lock it, find the base it stands on, hand it
         to that base (or its transport) and resync.
// ORIGINAL: 0x004D7DE0 ?go_home@Console@@QAEXH@Z 0x004D7DE0-0x004D7EDA FILE
// TRIED (seed history): structure (lock_veh via &veh_id, base_at, owner-byte
//        compare against the veh table, synch/await/unlock NetDaemon calls,
//        BaseWin::check_base x2, Console::update_data) all match in order;
//        remaining ~9-edit MISMATCH is mostly frame-pointer allocation.
// size      250 bytes
// prototype void (__thiscall ?go_home@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   3   call targets   8
// calls     0x0041A7A0 0x004CBAA0 0x004E3A50 0x00514880 0x00531020 0x005310F0 0x00531480 0x00593220
*/
void Console::go_home(int veh_id) {
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) == 0) {
        const int x = Vehs[veh_id].x;
        const int y = Vehs[veh_id].y;
        const int base_id = base_at(x, y);
        bool matched = false;
        if (base_id >= 0) {
            // A base counts as home when its current owner is ours.
            matched = Bases[base_id].faction_id_current == Vehs[veh_id].faction_id;
        } else {
            action_home(veh_id, 0);
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
        }
        if (matched) {
            const int16_t old_base = Vehs[veh_id].home_base_id;
            Vehs[veh_id].home_base_id = static_cast<int16_t>(base_id);
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
            NetDaemonNet.unlock_veh();
            if (old_base >= 0) {
                TutWinBaseWindow.check_base(old_base);
            }
            TutWinBaseWindow.check_base(base_id);
            update_data(0);
            return;
        }
        NetDaemonNet.unlock_veh();
    }
}

// editor_scen_param's repeated "render the field value into StringTemp and
// hand the text to the script parser" triple. The original serves it from a
// shared subroutine (one itoa/strcat family in its call list); the file-static
// helper gives this TU the same shape.
static void parse_field_number(int index, int value) {
    char num_buf[0x40];
    StringTemp[0] = '\0';
    _itoa(value, num_buf, 10);
    strcat(StringTemp, num_buf);
    parse_says(index, StringTemp, -1, -1);
}

// The CUSTOMTIME accept path clamps every re-parsed field to 0..9999.
static int parse_turn_time_clamped(const char256 &text) {
    int value = atoi(text.str);
    if (value < 0) {
        value = 0;
    } else if (value > 0x270F) {
        value = 0x270F;
    }
    return value;
}

/*
Purpose: Edit the seven scenario victory parameters on one SCENPARAM popup and
         re-parse the edited fields back into the game's objective globals.
// ORIGINAL: 0x004DCCC0 ?editor_scen_param@Console@@QAEXXZ 0x004DCCC0-0x004DD1A1;0x0065B6A4-0x0065B7B0 FILE
// TRIED (seed history): mirrors the recovered AlphaNet::create/join siblings -
//        a stack `Popup popup;` gives the SEH frame/alloca/dtor cascade for
//        free; inlining all seven itoa/strcat/parse_says triples drops the
//        frame entirely past the third copy, the static-local-helper shape
//        keeps it (MISMATCH #9 `push` vs `lea` with the full logic intact).
// size      1517 bytes
// prototype void (__thiscall ?editor_scen_param@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   22
// calls     0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406820 0x00406880 0x00406AF0 0x005ABF20 0x005D45B0 0x005D4DD0 0x005E3820 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00625EC0 0x00645470 0x00645550 0x00645DA7 0x0064FC88
*/
void Console::editor_scen_param() {
    Popup popup;

    auto_undo();

    parse_field_number(0, ScenarioSave.objective_req_victory);
    parse_field_number(1, ScenarioSave.objectives_sudden_death_victory);
    parse_field_number(2, ScenarioSave.objective_achieve_points);
    parse_field_number(3, ScenarioSave.victory_achieve_bonus_points);
    parse_field_number(4, ScenarioSave.starting_mission_year);
    parse_field_number(5, ScenarioSave.ending_mission_year);
    parse_field_number(6, GameDataStateStorage.fields_.sunspot_duration_);

    popup.start(PopupStartCaption, StrScenParam, -1, nullptr, 0x44, nullptr);

    if (popup.exec(0, 0) < 0) {
        popup.close();
        return;
    }

    // The parser parked each field's text in ParseStrBuffer; read the edited
    // values back out.
    ScenarioSave.objective_req_victory = atoi(ParseStrBuffer[0].str);
    ScenarioSave.objectives_sudden_death_victory = atoi(ParseStrBuffer[1].str);
    ScenarioSave.objective_achieve_points = atoi(ParseStrBuffer[2].str);
    ScenarioSave.victory_achieve_bonus_points = atoi(ParseStrBuffer[3].str);
    ScenarioSave.starting_mission_year = atoi(ParseStrBuffer[4].str);
    ScenarioSave.ending_mission_year = atoi(ParseStrBuffer[5].str);
    GameDataStateStorage.fields_.sunspot_duration_ = atoi(ParseStrBuffer[6].str);

    popup.close();
}

/*
Purpose: Run the destroy-order dialog on one vehicle: eligibility guards, the
         terraforming-option picker, the diplomacy consequences of attacking
         inside another faction's territory, then the destroy itself.
// ORIGINAL: 0x004D52A0 ?destroy@Console@@QAEXH@Z 0x004D52A0-0x004D5D23;0x0065AB02-0x0065AD6A FILE
// TRIED (seed history): an SEH-heavy function (7+ automatic RAII objects);
//        the guard-clause prefix is transcribed from the raw disassembly and
//        the post-exec tail is structurally present but its trailing
//        confirmation-dialog construction is simplified away rather than
//        reproduced - treat that half as unverified.
// size      3307 bytes
// prototype void (__thiscall ?destroy@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   3   call targets   33
// calls     0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406820 0x00406880 0x00406910 0x00406E60 0x0047A890 0x004CAA50 0x004E3EF0 0x0050BCC0 0x0052DC70 0x00531020 0x005310F0 0x00531300 0x0055F770 0x00592E10 0x00592EE0 0x005C1B70 0x005D45B0 0x005D4DD0 0x005E3820 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00645550
*/
void Console::destroy(int veh_id) {
    // The original wraps this body in an SEH frame from a chain of automatic
    // objects (Popup, Scroll, two FlatButtons, a GraphicWin); declaring the
    // real RAII locals reproduces the constructor/destructor cascade rather
    // than hand-tracking the compiler's unwind states.
    Popup popup;
    Scroll scroll;

    if (not_my_turn()) {
        NetMsgPrimary.pop(StrSimulNo, 5000, 0, 0);
        return;
    }
    if (veh_ready(veh_id) == 0) {
        NetMsgPrimary.pop(StrUnitMoved, 5000, 0, 0);
        return;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, -1) != 0) {
        return;
    }
    if (not_my_turn()) {
        NetMsgPrimary.pop(StrSimulNo2, 5000, 0, 0);
        NetDaemonNet.unlock_veh();
        return;
    }
    if (veh_ready(veh_id) == 0) {
        NetMsgPrimary.pop(StrUnitMoved2, 5000, 0, 0);
        NetDaemonNet.unlock_veh();
        return;
    }

    const int vy = Vehs[veh_id].y;
    const int vx = Vehs[veh_id].x;
    const uint8_t faction = Vehs[veh_id].faction_id;
    const int x_half = vx >> 1;
    Map *const tile = map_loc(vx, vy);
    const bool not_ocean = (tile->climate & 0xE0) < 0x60;
    const int proto_id = Vehs[veh_id].proto_id;
    const int weapon_id = VehPrototypes[proto_id].weapon_id;

    if ((Weapon[weapon_id].offense_rating == 0) &&
        (Weapon[weapon_id].mode != WPN_MODE_TERRAFORMER)) {
        NetMsgPrimary.pop(StrOnlyCombat, 5000, 0, 0);
        NetDaemonNet.unlock_veh();
        return;
    }
    if (((tile->bit & 1) != 0) && ((tile->val2 & 0xF) < 8) &&
        ((tile->val2 & 0xF) >= 0)) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop(StrNotBase, 5000, 0, 0);
        return;
    }

    const int triad = Chassis[VehPrototypes[proto_id].chassis_id].triad;
    if ((triad != 2) && (static_cast<int>(not_ocean) != (triad == 1))) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop(StrNotInTriad, 5000, 0, 0);
        return;
    }

    FlatButton btn1;
    FlatButton btn2;
    GraphicWin win;

    const uint32_t tile_flags = tile->bit;
    const char *title = (triad == 2) ? StrDestroyBomb : StrDestroy;
    popup.start(PopupStartCaption, title, tile_flags, nullptr, 0x40, nullptr);

    Dialogs dialogs;
    int item_count = 0;
    for (int a = 0; a < 0x13; a++) {
        const uint32_t abil_flag = Terraforming[a].bit;
        if (abil_flag == 0 || (tile_flags & abil_flag) == 0 || a == 10) {
            continue;
        }
        // The rules' label fields carry StringTable offsets; the original
        // reads the a-th row's int at byte offset x_half*4 of the
        // Terraforming table and resolves it through the string table.
        const int label_key = reinterpret_cast<const int *>(Terraforming)[x_half + a * 8];
        dialogs.item(StringTable.get(label_key), abil_flag);
        item_count++;
    }

    if (item_count == 0) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop(StrNotHere, 5000, 0, 0);
        return;
    }

    int32_t result = -1;
    if (item_count >= 2) {
        result = popup.exec(0, 0);
    }
    if (result == -1) {
        return;
    }

    int owner;
    const int territory = whose_territory(faction, vx, vy, &owner, 0);
    if ((territory >= 0) && (territory != faction)) {
        if (break_treaty(faction, territory, 7) != 0) {
            NetDaemonNet.unlock_veh();
            return;
        }
        if ((PlayersData[territory].diplo_treaties[faction] & DTREATY_VENDETTA) == 0) {
            if (IsMultiplayerNet != 0) {
                message_data(0x2440, 0, 0, faction, territory, -1);
                NetDaemonNet.await_exec(0);
                message_data(0x2214, 0, 0, 0, 0, 0);
            } else {
                double_cross(faction, territory, -1);
            }
        }
    }

    field_23BE4_ = 0;
    if (IsMultiplayerNet != 0) {
        message_veh(0x2406, veh_id, result, 0);
        NetDaemonNet.await_exec(1);
    } else {
        action_destroy(veh_id, result, -1, -1);
    }
}

/*
Purpose: Customise one faction's leader and faction names: pick the faction on
         a who-list popup, edit its five name fields, and resync the faction.
// ORIGINAL: 0x004DD1B0 ?custom_my_faction@Console@@QAEXXZ 0x004DD1B0-0x004DD6E5;0x0065B7B0-0x0065B872 FILE
// TRIED (seed history): two `Popup` locals matching the disassembly's two
//        separate constructor calls; BasePop::dialogs_ reached the inner
//        popup's Dialogs sub-object. MISMATCH #0 (no SEH frame at all) - the
//        frame threshold was not isolated before the seed's budget ran out.
// size      1527 bytes
// prototype void (__thiscall ?custom_my_faction@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   20
// calls     0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406880 0x00406C00 0x00406E60 0x0043EF70 0x00532E00 0x005D4DD0 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00611F80 0x00612A70 0x006169A0 0x00645460 0x00645550 0x0064FDF0
*/
void Console::custom_my_faction() {
    Popup popup;
    char name_buf[0xC8];
    int faction_idx;

    const unsigned char state = static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
    if ((state & 0x40) != 0 && (state & 0x80) != 0) {
        int chosen;
        {
            Popup popup2;
            // start's third argument is the popup's value - the initial
            // list selection - which the original re-stores after start()
            // when the current faction is a playable one.
            popup2.start(PopupStartCaption, StrEditorWhoListTitle,
                         (ConsoleGlobal.faction_id_ >= 0) ? ConsoleGlobal.faction_id_ : -1,
                         nullptr, 0x40);
            PopupModalState = 6;
            // The who list walks factions 1..7 by noun record: each item's
            // text is the faction's noun, with its gender/plurality feeding
            // the parser defaults.
            int idx = 1;
            int *entry = &Players[1].noun_gender;
            do {
                PluralityDefault = entry[1];
                GenderDefault = entry[0];
                popup2.dialogs().item(reinterpret_cast<char *>(entry - 6), idx);
                entry += 0x167;
                idx++;
            } while (entry < &Players[MaxPlayerNum].noun_gender);
            chosen = popup2.exec(0, 0);
            PopupModalState = 0;
        }
        if (chosen < 0) {
            popup.close();
            return;
        }
        faction_idx = chosen;
    } else {
        faction_idx = ConsoleGlobal.faction_id_;
    }

    Player &player = Players[faction_idx];
    // The label pool's key table: slots 0x2E2/0x2E3 pick the custom-name
    // question's wording, the edit fields' captions sit around 0xAE and 0x1E3.
    const int *const label_keys = reinterpret_cast<const int *>(Labels.strings_ptr);
    const int key = (player.is_leader_female == 0) ? label_keys[0x2E2] : label_keys[0x2E3];
    strcpy(name_buf, StringTable.get(key));

    popup.start(PopupStartCaption, StrCustomNames, -1, nullptr, 0x44, nullptr);

    char *const f1 = player.name_leader;
    char *const f2 = player.formal_name_faction;
    char *const f3 = player.noun_faction;
    char *const f4 = player.adj_name_faction;
    char *const f5 = player.title_leader;
    char *const f6 = player.adj_leader;

    popup.dialogs().edit(StringTable.get(label_keys[0x2B8 / 4]), f1, 0x17);
    popup.dialogs().edit(StringTable.get(label_keys[0x2BC / 4]), f2, 0x27);
    popup.dialogs().edit(StringTable.get(label_keys[0x2C0 / 4]), f3, 0x17);
    popup.dialogs().edit(StringTable.get(label_keys[0x2C4 / 4]), f4, 0x17);
    popup.dialogs().edit(StringTable.get(label_keys[0x78C / 4]), f5, 0x17);
    popup.dialogs().edit(StringTable.get(label_keys[0x790 / 4]), f6, 0x17);
    popup.dialogs().edit(StringTable.get(label_keys[0x794 / 4]), name_buf, 0x17);

    const int result = popup.exec(0, 0);
    if (result < 0) {
        popup.close();
        return;
    }

    strcpy(ParseStrBuffer[0].str, f1);
    strcpy(ParseStrBuffer[1].str, f2);
    strcpy(ParseStrBuffer[2].str, f3);
    strcpy(ParseStrBuffer[3].str, f4);
    strcpy(ParseStrBuffer[4].str, f5);
    strcpy(ParseStrBuffer[5].str, f6);

    // The entered leader name counts as female when its first letter matches
    // the female wording of the question.
    const char *const match_name = StringTable.get(label_keys[0x2E3]);
    const int cmp = _strnicmp(match_name, name_buf, 1);
    player.is_leader_female = (cmp == 0) ? 1 : 0;

    NetDaemonNet.synch(7, faction_idx, 0, 0, nullptr, 1, 0x2101);

    popup.close();
}

/*
Purpose: Open the multiplayer time-controls dialog, and on a CUSTOMTIME accept
         re-parse and clamp the seven per-faction turn times and resync.
// ORIGINAL: 0x00515900 ?time_controls@Console@@QAEXXZ 0x00515900-0x00515EE6;0x0065D2EF-0x0065D3F3 FILE
// TRIED (seed history): a single automatic `Popup popup;` local standing in
//        for the ~20 individual unwind funclets of the original's Popup
//        sub-objects (Spot/Dialogs/Dialog/Sprite/FlatButton x2/Heap inside
//        Popup's own footprint); compiled and matched through instruction 10.
//        The seed's literal vtable re-stores before each close() are
//        compiler-owned with a real local here and are not spelled.
// size      1770 bytes
// prototype void (__thiscall ?time_controls@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   12
// calls     0x004048A0 0x00404900 0x004059A0 0x00406380 0x004064D0 0x00406C00 0x00589330 0x00593250 0x005D45B0 0x005D4DD0 0x005E3820 0x00625EC0
*/
void Console::time_controls() {
    Popup popup;

    if (IsMultiplayerNet == 0) {
        popup.close();
        return;
    }

    if (console_turns_assigned() == 0) {
        NetMsgPrimary.pop(StrHostTime, 5000, 0, 0);
        popup.close();
        return;
    }

    time_controls_dialog(&popup);
    // BasePop's value slot (basepop.h's private value_ at 0xA44): the row the
    // dialog opens preselected. editor_diplomacy spells the same write the
    // same way.
    *reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(&popup) + 0xa44) =
        static_cast<uint32_t>(GameDataStateStorage.fields_.time_controls_selection_);
    const int result = popup.exec(0, 0);
    if (result < 0) {
        popup.close();
        return;
    }

    if (result == 5) {
        // The six editable turn times go through the parser in this order,
        // then the accept path re-parses and clamps all seven fields of the
        // active TimeControl row.
        parse_field_number(0, TimeControl[5].base);
        parse_field_number(1, TimeControl[5].unit);
        parse_field_number(2, TimeControl[5].event);
        parse_field_number(3, TimeControl[5].turn);
        parse_field_number(4, TimeControl[5].extra);
        parse_field_number(result, TimeControl[5].refresh);

        popup.start(PopupStartCaption, StrCustomTime, -1, nullptr, 0x44, nullptr);
        if (popup.exec(0, 0) >= 0) {
            TimeControl[5].base = parse_turn_time_clamped(ParseStrBuffer[0]);
            TimeControl[5].unit = parse_turn_time_clamped(ParseStrBuffer[1]);
            TimeControl[5].event = parse_turn_time_clamped(ParseStrBuffer[2]);
            TimeControl[5].turn = parse_turn_time_clamped(ParseStrBuffer[3]);
            TimeControl[5].extra = parse_turn_time_clamped(ParseStrBuffer[4]);
            TimeControl[5].refresh = parse_turn_time_clamped(ParseStrBuffer[5]);
            TimeControl[5].accumulated = parse_turn_time_clamped(ParseStrBuffer[6]);

            NetDaemonNet.synch(0x85, 0, 0, 0, nullptr, 1, 0x2101);
        }
    }

    message_data(0x4C02, 0, result, 0, 0, 0);
    popup.close();
}

// ORIGINAL: 0x0050F680 ?on_nc_hittest@Console@@QAEXHH@Z 0x0050F680-0x0050F951
// RECOVERY_BINDING: v1 {"evidence": ["The original 0x0050F680..0x0050F951 disassembly exits return EAX values 1 or 0; its retired transcription is preserved in Git history."], "reason": "Translate the Console non-client hit-test into its existing virtual method; the disassembly exits return EAX values 1 or 0, correcting the seed-only void prototype."}
// symbol    ?on_nc_hittest@Console@@UAEHHH@Z
int Console::on_nc_hittest(int x_coord, int y_coord) {
    // The original exits with EAX=1 while any split line is armed.  The
    // archived unit's catalogued void prototype was therefore incomplete;
    // the virtual slot and the product declaration both return int.
    if (field_23D80_ == 0xFFFFFFFFu || field_23D84_ == 0xFFFFFFFFu ||
        field_23D88_ == 0xFFFFFFFFu || field_23D8C_ == 0xFFFFFFFFu) {
        return 1;
    }

    RECT caption_box;
    caption_box.top = 0xE;
    caption_box.bottom = 0x1F;
    caption_box.left = client_rect_.right - client_rect_.left - 0x5b;
    caption_box.right = caption_box.left + 0x26;
    if (in_box(x_coord, y_coord, &caption_box)) {
        return 1;
    }

    const int width = client_rect_.right - client_rect_.left;
    const int height = client_rect_.bottom - client_rect_.top;
    const int border = border_thickness_;
    const int bottom_border = bottom_border_thickness_ >= 0
        ? bottom_border_thickness_ : border_thickness_;
    const int sprite_a_width = sprite_a_.width();
    const int sprite_b_height = sprite_b_.height();

    if (x_coord <= width - border) {
        if (y_coord <= height - bottom_border || x_coord < bottom_border ||
            x_coord > width - border) {
            return 1;
        }
        const int first_left = static_cast<int>(field_23D80_) -
                               sprite_a_width + border;
        const int first_right = static_cast<int>(field_23D80_) +
                                border + sprite_a_width;
        if (x_coord >= first_left && x_coord <= first_right) {
            return 1;
        }
        const int second_left = static_cast<int>(field_23D84_) -
                                sprite_a_width + border;
        if (x_coord < second_left) {
            return 1;
        }
        const int second_right = static_cast<int>(field_23D84_) +
                                 border + sprite_a_width;
        if (x_coord > second_right) {
            return 1;
        }
        return 0;
    }

    // These are the bottom non-client track limits expressed in Win's real
    // client/outer rectangles. The two handles use the shared drag slots.
    const int track_top = height - (outer_rect_.bottom - outer_rect_.top) -
                          bottom_border;
    if (y_coord < track_top || y_coord > height - bottom_border) {
        return 1;
    }
    const int first_top = track_top - sprite_b_height +
                          static_cast<int>(field_23D88_);
    const int first_bottom = track_top + static_cast<int>(field_23D88_) +
                             sprite_b_height;
    if (y_coord >= first_top && y_coord <= first_bottom) {
        return 1;
    }
    const int second_top = track_top - sprite_b_height +
                           static_cast<int>(field_23D8C_);
    if (y_coord < second_top) {
        return 1;
    }
    const int second_bottom = track_top + static_cast<int>(field_23D8C_) +
                              sprite_b_height;
    return y_coord > second_bottom ? 1 : 0;
}

// ORIGINAL: 0x0050F960 ?on_post_redraw_nc_buffer@Console@@QAEXPAUBuffer@@H@Z 0x0050F960-0x0050FB08
// RECOVERY_BINDING: v1 {"evidence": ["The original 0x0050F960..0x0050FB08 draws two splitter handles per orientation; its retired transcription remains in Git history."], "reason": "Render the two Console non-client splitter handles with their owned Sprite members and the Window client/outer geometry; the archived scaffold confirms two handles per orientation and the image buffer dimensions."}
// symbol    ?on_post_redraw_nc_buffer@Console@@UAEXPAVBuffer@@H@Z
void Console::on_post_redraw_nc_buffer(Buffer *buffer, int which) {
    if (buffer == nullptr) {
        return;
    }

    if (which == 1) {
        const int border = bottom_border_thickness_ >= 0
            ? bottom_border_thickness_ : border_thickness_;
        const int track_top = client_rect_.bottom - client_rect_.top -
                              outer_rect_.bottom - caption_height_ +
                              outer_rect_.top;
        const int x = buffer->dib_.bmiHeader.biWidth - sprite_b_.width();
        const int first_y = track_top - sprite_b_.height() / 2 +
                            static_cast<int>(field_23D88_) - border;
        const int second_y = track_top - sprite_b_.height() / 2 +
                             static_cast<int>(field_23D8C_) - border;
        sprite_b_.draw(buffer, sprite_b_.transparent_index(), x, first_y, 1, 1);
        sprite_b_.draw(buffer, sprite_b_.transparent_index(), x, second_y, 1, 1);
    } else if (which == 2) {
        const int image_bottom = -buffer->dib_.bmiHeader.biHeight;
        const int y = image_bottom - sprite_a_.height();
        const int first_x = static_cast<int>(field_23D80_) -
                            sprite_a_.width() / 2 + border_thickness_;
        const int second_x = static_cast<int>(field_23D84_) -
                             sprite_a_.width() / 2 + border_thickness_;
        sprite_a_.draw(buffer, sprite_a_.transparent_index(), first_x, y, 1, 1);
        sprite_a_.draw(buffer, sprite_a_.transparent_index(), second_x, y, 1, 1);
    }
}

/*
Purpose: A non-client left press on the console frame: the minimize and
         maximize caption boxes, then the right-edge splitter (choosing the
         nearer of its two drag slots) and the bottom-edge splitter, each
         claiming the shared input-focus pair when nothing owns it yet.

// ORIGINAL: 0x0050FB10 ?on_nc_left_down@Console@@QAEXHH@Z 0x0050FB10-0x0050FE69 FILE
// symbol    ?on_nc_left_down@Console@@UAEXHH@Z
// size      857 bytes
// prototype void (__thiscall ?on_nc_left_down@Console@@QAEXHH@Z)(Console* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EDE60 0x005EDEB0 0x005FA7E0
// TRIED (seed): minimize/maximize hit-tests then two edge-distance blocks
//         (right vs left/right splitter, sign-tested via a discarded add
//         whose flags a later `js` reuses) reusing the same client_rect_/
//         outer_rect_/caption_height_/border_thickness_ offsets confirmed on
//         0x00510110. The seed's raw -0x23cXX reads are the runtime vbtable
//         walk (the body is entered on the GraphicWin subobject, the virtual
//         base at Console+0x23d94); the real virtual entry lets the compiler
//         own that walk, and the four drag slots are the fields at
//         Console+0x23d80..0x23d8c.
Status: Coverage transcription - unclaimed body
*/
void Console::on_nc_left_down(int x_coord, int y_coord) {
    RECT box;
    box.left = 0;
    box.top = 0xE;
    box.right = 0x11;
    box.bottom = 0x1F;

    box.left = client_rect_.right - client_rect_.left - 0x5b;
    box.right = box.left + 0x11;
    if (in_box(x_coord, y_coord, &box)) {
        minimize();
        return;
    }

    box.left += 0x15;
    box.right += 0x15;
    if (in_box(x_coord, y_coord, &box)) {
        maximize();
        return;
    }

    const int border = (bottom_border_thickness_ >= 0)
        ? bottom_border_thickness_ : border_thickness_;
    if (x_coord > client_rect_.right - border_thickness_ - client_rect_.left) {
        const int extent = client_rect_.top - client_rect_.bottom -
                           outer_rect_.top + outer_rect_.bottom -
                           static_cast<int>(field_23D88_) + border;
        int left_reach;
        if (extent + y_coord >= 0) {
            left_reach = extent + y_coord;
        } else {
            left_reach = client_rect_.bottom - client_rect_.top -
                         outer_rect_.bottom + outer_rect_.top - border -
                         y_coord + static_cast<int>(field_23D88_);
        }

        int right_reach;
        if (extent + y_coord >= 0) {
            right_reach = extent - static_cast<int>(field_23D8C_) + y_coord;
        } else {
            right_reach = client_rect_.bottom - client_rect_.top -
                          outer_rect_.bottom - caption_height_ +
                          outer_rect_.top - border - y_coord +
                          static_cast<int>(field_23D8C_) + caption_height_;
        }

        if (left_reach < right_reach) {
            field_23D88_ = 0xFFFFFFFFu;
        } else {
            field_23D8C_ = 0xFFFFFFFFu;
        }

        if (WinPointerOwner4 == 0) {
            WinPointerOwner3 = 0;
            WinPointerOwner4 = this;
        }
    }

    const int bottom_border = (bottom_border_thickness_ >= 0)
        ? bottom_border_thickness_ : border_thickness_;
    if (y_coord > client_rect_.bottom - client_rect_.top - bottom_border) {
        const int edge = border_thickness_;

        const int to_top_slot = x_coord - edge -
                                static_cast<int>(field_23D80_);
        const int dist_top = (to_top_slot >= 0) ? to_top_slot : -to_top_slot;

        const int to_bottom_slot = x_coord - edge -
                                   static_cast<int>(field_23D84_);
        const int dist_bottom = (to_bottom_slot >= 0) ? to_bottom_slot
                                                      : -to_bottom_slot;

        if (dist_top < dist_bottom) {
            field_23D80_ = 0xFFFFFFFFu;
        } else {
            field_23D84_ = 0xFFFFFFFFu;
        }

        if (WinPointerOwner4 == 0) {
            WinPointerOwner3 = 0;
            WinPointerOwner4 = this;
        }
    }
}

/*
Purpose: A non-client mouse move while a splitter drag is armed: whichever of
         the four drag slots holds a live position follows the cursor between
         its half-track clamp bounds, the dragged edge repaints without the
         line, and the crossing drag line is drawn at the new position.

// ORIGINAL: 0x00510110 ?on_nc_mouse_move@Console@@QAEXHH@Z 0x00510110-0x005103FF FILE
// symbol    ?on_nc_mouse_move@Console@@UAEXHH@Z
// size      751 bytes
// prototype void (__thiscall ?on_nc_mouse_move@Console@@QAEXHH@Z)(Console* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D55D0 0x005F2AC0 0x005F4C30 0x005F4CA0
// TRIED (seed): two near-duplicate splitter-drag blocks (vertical/horizontal),
//         each a 3-way slot-select then clamp-to-half then
//         redraw_nc_buffer/update_nc_buffer, sharing one last-value pair
//         threaded into a final draw section. The five-argument
//         GraphicWin::update (0x005D55D0, BYTE_EXACT in the retained unit) is
//         not declared on the class, so the final section's two invalidation
//         calls go through the original's address.
Status: Coverage transcription - unclaimed body
*/
void Console::on_nc_mouse_move(int x_coord, int y_coord) {
    int line_x;
    int line_y;

    // Sprite keeps its geometry private and Console is not among its friends,
    // so the two track extents keep the image's own offsets (+0x18 into the
    // vertical drag sprite, +0x1c into the horizontal one).
    const int track_height = static_cast<int>(*reinterpret_cast<const int *>(
        reinterpret_cast<const uint8_t *>(&sprite_a_) + 0x18));
    const int track_width = static_cast<int>(*reinterpret_cast<const int *>(
        reinterpret_cast<const uint8_t *>(&sprite_b_) + 0x1C));

    if (field_23D80_ == 0xFFFFFFFFu) {
        const int half = track_height / 2;
        field_23D80_ =
            static_cast<uint32_t>(x_coord - border_thickness_);
        if (static_cast<int>(field_23D80_) < half) {
            field_23D80_ = static_cast<uint32_t>(half);
        }
        const int limit = client_rect_.right - client_rect_.left -
                          border_thickness_ * 2 - half;
        if (static_cast<int>(field_23D80_) > limit) {
            field_23D80_ = static_cast<uint32_t>(limit);
        }
        line_x = static_cast<int>(field_23D80_);
        line_y = static_cast<int>(field_23D88_);
        redraw_nc_buffer(2);
        field_23D80_ = 0xFFFFFFFFu;
        update_nc_buffer(2);
    } else if (field_23D84_ == 0xFFFFFFFFu) {
        const int half = track_height / 2;
        field_23D84_ =
            static_cast<uint32_t>(x_coord - border_thickness_);
        if (static_cast<int>(field_23D84_) < half) {
            field_23D84_ = static_cast<uint32_t>(half);
        }
        const int limit = client_rect_.right - client_rect_.left -
                          border_thickness_ * 2 - half;
        if (static_cast<int>(field_23D84_) > limit) {
            field_23D84_ = static_cast<uint32_t>(limit);
        }
        line_x = static_cast<int>(field_23D84_);
        line_y = static_cast<int>(field_23D88_);
        redraw_nc_buffer(2);
        field_23D84_ = 0xFFFFFFFFu;
        update_nc_buffer(2);
    } else {
        line_x = x_coord;
        line_y = x_coord;
    }

    if (field_23D88_ == 0xFFFFFFFFu) {
        const int bottom_border = (bottom_border_thickness_ >= 0)
            ? bottom_border_thickness_ : border_thickness_;
        const int extent = client_rect_.top - caption_height_ -
                           client_rect_.bottom - outer_rect_.top +
                           outer_rect_.bottom + caption_height_ +
                           bottom_border;
        const int half = track_width / 2;
        field_23D88_ = static_cast<uint32_t>(extent + y_coord);
        if (static_cast<int>(field_23D88_) < half) {
            field_23D88_ = static_cast<uint32_t>(half);
        }
        const int limit = client_rect_.bottom - client_rect_.top -
                          caption_height_ - bottom_border - half;
        if (static_cast<int>(field_23D88_) > limit) {
            field_23D88_ = static_cast<uint32_t>(limit);
        }
        line_x = static_cast<int>(field_23D80_);
        line_y = static_cast<int>(field_23D88_);
        redraw_nc_buffer(1);
        field_23D88_ = 0xFFFFFFFFu;
        update_nc_buffer(1);
    } else if (field_23D8C_ == 0xFFFFFFFFu) {
        const int bottom_border = (bottom_border_thickness_ >= 0)
            ? bottom_border_thickness_ : border_thickness_;
        const int extent = client_rect_.top - caption_height_ -
                           client_rect_.bottom - outer_rect_.top +
                           outer_rect_.bottom + caption_height_ +
                           bottom_border;
        const int half = track_width / 2;
        field_23D8C_ = static_cast<uint32_t>(extent + y_coord);
        if (static_cast<int>(field_23D8C_) < half) {
            field_23D8C_ = static_cast<uint32_t>(half);
        }
        const int limit = client_rect_.bottom - client_rect_.top -
                          caption_height_ - bottom_border - half;
        if (static_cast<int>(field_23D8C_) > limit) {
            field_23D8C_ = static_cast<uint32_t>(limit);
        }
        line_x = static_cast<int>(field_23D84_);
        line_y = static_cast<int>(field_23D8C_);
        redraw_nc_buffer(1);
        field_23D8C_ = 0xFFFFFFFFu;
        update_nc_buffer(1);
    }

    if (field_23D80_ != 0xFFFFFFFFu && field_23D84_ != 0xFFFFFFFFu &&
        field_23D88_ != 0xFFFFFFFFu && field_23D8C_ != 0xFFFFFFFFu) {
        return;
    }

    GraphicWin::update(0, ConsoleDragLineY,
                       static_cast<int>(dib_.bmiHeader.biWidth), 1, nullptr);
    GraphicWin::update(ConsoleDragLineX, 0, 1,
                       -static_cast<int>(dib_.bmiHeader.biHeight), nullptr);
    window_line_raw(static_cast<int>(dib_.bmiHeader.biWidth), line_y,
                    static_cast<int>(dib_.bmiHeader.biWidth), 0, 0, 1, 0);
    window_line_raw(line_x, -static_cast<int>(dib_.bmiHeader.biHeight),
                    line_x, 0, 0, 1, 0);
    ConsoleDragLineX = line_x;
    ConsoleDragLineY = line_y;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D6070 ?goto_routine@Console@@QAAXH@Z 0x004D6070-0x004D6196
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d6070.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
// symbol    ?goto_routine@Console@@SAXH@Z
void __cdecl Console::goto_routine(int base_id) {
    PlanWindow.clear_lines();
    if (base_id < 0 || base_id >= GameDataStateStorage.fields_.base_current_count_) {
        PlanWindow.back_redraw();
        PlanWindow.on_redraw();
        return;
    }
    const int x = Bases[base_id].x;
    const int y = Bases[base_id].y;
    if (!GoToPreviewActive) {
        PlanWindow.monocenter(x, y, 0);
        PlanWindow.add_line(x, y, x, y,
                            InfoFactionTextColors[ConsoleGlobal.faction_id_]);
        PlanWindow.on_redraw();
        return;
    }
    PlanWindow.opticenter(GoToSourceX, GoToSourceY, x, y, 0);
    if (base_id < GameDataStateStorage.fields_.base_current_count_)
        PlanWindow.add_line(GoToSourceX, GoToSourceY, x, y,
                            InfoFactionTextColors[ConsoleGlobal.faction_id_]);
    PlanWindow.on_redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D9170 ?arty@Console@@QAEXH@Z 0x004D9170-0x004D92CD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d9170.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::arty(int vehicle) {
    if (!can_arty(Vehs[vehicle].proto_id, 1)) return;
    if (not_my_turn()) {
        NetMsgPrimary.pop("SIMULNO", 5000, 0, nullptr);
        return;
    }
    if (!veh_ready(vehicle)) {
        NetMsgPrimary.pop("UNITMOVED", 5000, 0, nullptr);
        return;
    }
    Map *const tile = map_loc(Vehs[vehicle].x, Vehs[vehicle].y);
    if ((tile->climate & 0xe0) < 0x60 &&
        Chassis[VehPrototypes[Vehs[vehicle].proto_id].chassis_id].triad != 1 &&
        (!(tile->bit & 1) || (tile->val2 & 15) >= 8)) {
        NetMsgPrimary.pop("SEAARTY", 5000, 0, nullptr);
        return;
    }
    ConsoleGlobal.cursor_on(10, static_cast<int>(Rules.artillery_max_rng));
    int pixel_x, pixel_y;
    MapWinTable[0]->get_mouse_pos(&pixel_x, &pixel_y);
    MapWinTable[0]->on_mouse_move(pixel_x, pixel_y);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005166D0 ?cursor_key@Console@@QAEXHHH@Z 0x005166D0-0x005168B7
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005166d0.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::cursor_key(int, int character, int) {
    switch (character) {
    case ' ': {
        const int i = ConsoleGlobal.cursor_history_index_;
        const int base = base_at(ConsoleGlobal.cursor_history_x_[i],
                                 ConsoleGlobal.cursor_history_y_[i]);
        if (base >= 0 && ((GameDataStateStorage.fields_.game_state_ & 0x80) ||
            Bases[base].faction_id_current == ConsoleGlobal.faction_id_ ||
            ((Bases[base].visibility & (1u << ConsoleGlobal.faction_id_)) &&
             spying(Bases[base].faction_id_current)))) {
            TutWinBaseWindow.zoom(base, 0);
            return;
        }
    }
    case 'a':
        if (!NetDaemonNet.lock_state_[2].payload_.flags) {
            const int i = ConsoleGlobal.cursor_history_index_;
            activate(ConsoleGlobal.cursor_history_x_[i],
                     ConsoleGlobal.cursor_history_y_[i], 1);
            return;
        }
        break;
    case 'b': {
        const int i = ConsoleGlobal.cursor_history_index_;
        const int base = base_find(ConsoleGlobal.cursor_history_x_[i],
            ConsoleGlobal.cursor_history_y_[i], ConsoleGlobal.faction_id_);
        if (base >= 0) TutWinBaseWindow.zoom(base, 0);
        break;
    }
    case 'g':
        if (!ConsoleControlTurnActive && !NetDaemonNet.lock_state_[2].payload_.flags) {
            const int i = ConsoleGlobal.cursor_history_index_;
            const int base = base_at(ConsoleGlobal.cursor_history_x_[i],
                                     ConsoleGlobal.cursor_history_y_[i]);
            if (base >= 0) go_to(base, -1, 0);
        }
        break;
    case 'p': {
        const unsigned int shown = MapWinTable[0]->field_1DD70_ & 0x400000;
        for (int i = 0; i < static_cast<int>(MapWinTableSlots); ++i) {
            MapWin *const window = MapWinTable[i];
            if (!window || (i && !window->field_1DD74_)) continue;
            if (!shown) {
                window->field_1DD70_ |= 0x400000;
                window->draw_base_dest(1);
            } else {
                window->field_1DD70_ &= ~0x400000u;
                window->draw_map(1);
            }
        }
        break;
    }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00517060 ?use_time_bonus@Console@@QAEXXZ 0x00517060-0x00517173
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00517060.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::use_time_bonus() {
    if (ConsoleControlTurnActive || !GameDataStateStorage.fields_.time_controls_selection_ || !IsMultiplayerNet) return;
    const int selection = GameDataStateStorage.fields_.time_controls_selection_;
    const int faction = ConsoleGlobal.faction_id_;
    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10) && GameDataStateStorage.fields_.net_turn_faction_ != faction) {
        NetMsgPrimary.pop("SIMULNO", 5000, 0, nullptr);
        return;
    }
    if (PlayersData[faction].time_bonus_count <= 0) {
        parse_num(0, TimeControl[selection].extra);
        NetMsgPrimary.pop("NOTIMEBONUSES", 5000, 0, nullptr);
        return;
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x800) {
        NetMsgPrimary.pop("NOBONUSATEND", 5000, 0, nullptr);
        return;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 2) &&
        ((1u << faction_id_) & NetDaemonNet.lock_state_[0].trailing_words_[7])) {
        NetMsgPrimary.pop("NOBONUSDONE", 5000, 0, nullptr);
        return;
    }
    message_data(0x2306, 0, 0, 0, 0, 0);
}

namespace {
inline void console_close_network(unsigned int final_channel_mask) {
    NetDaemonNet.hang_up();
    LockStateEntry *const state = NetDaemonNet.lock_state_;
    state[0].leading_words_[1] = 0;
    state[0].leading_words_[2] = 0;
    state[0].leading_words_[3] = 0;
    state[0].payload_.flags = 0;
    state[0].trailing_words_[8] = 0;
    state[0].trailing_words_[7] = 0;
    state[1].leading_words_[1] = 0;
    state[1].leading_words_[0] = 0;
    state[1].trailing_words_[0] = 0;
    state[2].leading_words_[1] = 0;
    state[1].trailing_words_[9] = 0;
    state[2].leading_words_[0] = 0;
    state[2].trailing_words_[5] = 0;
    state[2].trailing_words_[4] = 0;
    state[0].trailing_words_[4] = 0;
    state[0].trailing_words_[5] = 0;
    state[0].trailing_words_[6] = 0;
    state[2].trailing_words_[3] = 0;
    state[2].trailing_words_[2] = 0;
    state[2].leading_words_[3] = 0;
    state[2].leading_words_[2] = 0;
    state[2].payload_.flags = 0;
    state[0].trailing_words_[0] = 0;
    state[0].trailing_words_[1] = 0;
    state[2].trailing_words_[8] = 0;
    state[2].trailing_words_[7] = 0;
    state[0].trailing_words_[2] = 0;
    state[0].trailing_words_[3] = 0;
    state[2].trailing_words_[6] = final_channel_mask;
    state[1].payload_.vehicle_selection_follow = 0;
    for (int i = 6; i < 14; ++i) state[1].trailing_words_[i - 5] = 0;
    NetDaemonNet.locks_.clear();
    NetDaemonNet.AlphaNet::close();
    IsMultiplayerNet = 0;
}
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00514990 ?quit_game@Console@@QAEXXZ 0x00514990-0x00514B2C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00514990.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::quit_game() {
    const char *prompt = "REALLYQUIT";
    if ((IsMultiplayerNet || IsMultiplayerPBEM) &&
        bit_count(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 255) > 1) prompt = "REALLYWIMPOUT";
    if (!X_pop(prompt, nullptr)) return;
    if (IsMultiplayerPBEM) {
        // The original modifies only the human-faction byte of this word.
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(static_cast<unsigned char>(1u << ConsoleGlobal.faction_id_));
        save_game(0);
    }
    if ((GameDataStateStorage.fields_.game_rules_ & 0x400) && !IsMultiplayerPBEM) auto_save();
    if (IsMultiplayerNet) console_close_network(255);
    ExitTurnLoop = 1;
    log_say("Quit***********************************", 0, 0, 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00514D30 ?new_game@Console@@QAEXXZ 0x00514D30-0x00514EE2
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00514d30.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::new_game() {
    if (IsMultiplayerNet) return;
    const char *prompt = "REALLYQUIT";
    if (IsMultiplayerPBEM && bit_count(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 255) > 1)
        prompt = "REALLYWIMPOUT";
    if (X_pop(prompt, nullptr)) {
        if (IsMultiplayerPBEM) {
            GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(static_cast<unsigned char>(1u << ConsoleGlobal.faction_id_));
            save_game(0);
        }
        if ((GameDataStateStorage.fields_.game_rules_ & 0x400) && !IsMultiplayerPBEM) auto_save();
        // Re-read after callbacks; the initial network refusal does not
        // establish that the network flag stayed clear during the popup.
        if (IsMultiplayerNet) {
            console_close_network(255);
        }
        ExitTurnLoop = 1;
        log_say("Quit***********************************", 0, 0, 0);
    }
    if (ExitTurnLoop) GameRestartQueued = 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005162A0 ?name_landmark@Console@@QAEXHH@Z 0x005162A0-0x0051648F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005162a0.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::name_landmark(int x, int y) {
    if (x < 0 || x >= MapLongitudeBounds || y < 0 || y >= MapLatitudeBounds) {
        const int i = ConsoleGlobal.cursor_history_index_;
        x = ConsoleGlobal.cursor_history_x_[i];
        y = ConsoleGlobal.cursor_history_y_[i];
    }
    const int landmark = find_landmark(x, y, 1);
    if (!(GameDataStateStorage.fields_.game_state_ & 0x80)) {
        const int valid = landmark < 0 ? valid_landmark(x, y, ConsoleGlobal.faction_id_)
            : valid_landmark(MapLandmark[landmark].x, MapLandmark[landmark].y,
                             ConsoleGlobal.faction_id_);
        if (!valid) {
            NetMsgPrimary.pop("LANDMARKNO", 5000, 0, nullptr);
            return;
        }
    }
    char *const previous = landmark < 0 ? LandmarkDefaultName : MapLandmark[landmark].name;
    if (X_pop_ask("LANDMARK", 31, previous, nullptr, 0)) return;
    char *const name = ParseStrBuffer[0].str;
    purge_spaces(name);
    if (!strlen(name)) return;
    if (landmark < 0) {
        if (IsMultiplayerNet) {
            message_landmark(0x2e01, 0, name, x, y, 0, 0);
            return;
        }
        if (new_landmark(x, y, name) < 0) {
            parse_num(0, 64);
            NetMsgPrimary.pop("LANDMARKTOOMANY", 5000, 0, nullptr);
        }
        ::draw_map(1);
    } else if (!IsMultiplayerNet) {
        strcpy(MapLandmark[landmark].name, name);
        ::draw_map(1);
    } else {
        message_landmark(0x2e03, 0, name,
            MapLandmark[landmark].x, MapLandmark[landmark].y, 0, 0);
    }
}

extern int IfaceIdToSlotStorage[30];
extern int ActiveIfaceIdStorage;
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D9970 ?editor_on@Console@@QAEXXZ 0x004D9970-0x004D9AC5
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d9970.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::editor_on() {
    const unsigned int flags = GameDataStateStorage.fields_.game_state_;
    if ((IsMultiplayerNet || IsMultiplayerPBEM) &&
        !(flags & 0x61) && !(flags & 0x1000)) {
        NetMsgPrimary.pop("EDITORNET", 5000, 0, nullptr);
        return;
    }
    if (!(flags & 0x60) && !X_pop("SCENEDITOR", nullptr)) return;
    GameDataStateStorage.fields_.game_state_ = (GameDataStateStorage.fields_.game_state_ ^ 0x40) | 0x20;
    if (GameDataStateStorage.fields_.game_rules_ & 0x400) auto_save();
    if (GameDataStateStorage.fields_.game_state_ & 0x40) {
        NetMsgPrimary.pop("EDITORON", 5000, 0, nullptr);
        GameDataStateStorage.fields_.game_state_ |= 0x80;
    } else {
        NetMsgPrimary.pop("EDITOROFF", 5000, 0, nullptr);
        GameDataStateStorage.fields_.game_state_ &= ~0x80u;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 0xc0) == 0xc0 && !IfaceIdToSlotStorage[ActiveIfaceIdStorage])
        MainInterfaceGlobal.flat_button(2).show(0);
    else MainInterfaceGlobal.flat_button(2).hide();
    alpha_menu_.check(selected_vehicle_);
    MainInterfaceGlobal.button_group(0).set((GameDataStateStorage.fields_.game_state_ & 0xc0) == 0xc0 ? 1002 : 1001, 1);
    set_dirty();
    ::draw_map(1);
    WorldClimateWorldWin.GraphicWin::redraw();
    desktop_update();
    update_data(0);
}

void __cdecl ambience_veh(int vehicle);
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00510F50 ?ready@Console@@QAEXXZ 0x00510F50-0x00511471
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00510f50.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::ready() {
    field_23BF8_ = 0;
    ::draw_cursor();
    selected_vehicle_ = ready_search(field_23D18_ ? -1 : selected_vehicle_);
    field_23D18_ = 0;
    if (selected_vehicle_ < 0 || !ConsoleGlobal.cursor_active_) return;
    if (!Vehs[selected_vehicle_].order || Vehs[selected_vehicle_].order > 23) {
        const int x = Vehs[selected_vehicle_].x;
        const int y = Vehs[selected_vehicle_].y;
        field_23BF8_ = 0;
        if (survey_mode_) {
            survey_mode_ = 0;
            MapWinTable[0]->draw_map(1);
        }
        MapWinTable[0]->draw_cursor();
        cursor_next(x, y);
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        MapWinTable[0]->main_caption();
        const unsigned int state = Vehs[selected_vehicle_].state;
        if (!(GameDataStateStorage.fields_.game_preferences_ & 0x80000) ||
            (!Vehs[selected_vehicle_].order && !(state & 0x200) &&
             (state & 0x1000200) != 0x1000200)) {
            focus(Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y,
                  Vehs[selected_vehicle_].faction_id);
            const int vx = Vehs[selected_vehicle_].x;
            const int vy = Vehs[selected_vehicle_].y;
            bool altitude_clipped = false;
            if (vx >= 0 && vx < MapLongitudeBounds && vy >= 0 && vy < MapLatitudeBounds) {
                const unsigned int climate = map_loc(vx, vy)->climate;
                altitude_clipped = (climate & 0xe0) >= 0x61 &&
                    field_1DDA8_ + 3 > vy - static_cast<int>(climate >> 5) + 3;
            }
            bool in_view = false;
            if (!altitude_clipped && field_1DDA8_ + 3 <= vy &&
                vy < field_1DDD8_ + field_1DDD0_ - 3 + field_1DDA8_) {
                int adjusted_x = vx;
                if (!(MapIsFlat & 1)) {
                    if (vx < field_1DDA4_ + 3) adjusted_x += MapLongitudeBounds;
                    if (field_1DDD4_ + field_1DDCC_ - 3 + field_1DDA4_ <= adjusted_x)
                        adjusted_x -= MapLongitudeBounds;
                }
                in_view = field_1DDA4_ + 3 <= adjusted_x &&
                    adjusted_x < field_1DDD4_ + field_1DDCC_ - 3 + field_1DDA4_;
            }
            if (!in_view) MapWinTable[0]->set_center(vx, vy, 1);
        }
        alpha_menu_.check(selected_vehicle_);
        flush_input();
        Vehs[selected_vehicle_].unk_6 = 0;
    }
    const int base = base_at(Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
    const unsigned int state = Vehs[selected_vehicle_].state;
    if (!Vehs[selected_vehicle_].order && !(state & 0x200) &&
        (state & 0x1000200) != 0x1000200 && !(state & 0x4000) && base >= 0) {
        const int triad = Chassis[VehPrototypes[Vehs[selected_vehicle_].proto_id].chassis_id].triad;
        const int destination = triad == 0 ? Bases[base].auto_forward_land_base_id
            : triad == 1 ? Bases[base].auto_forward_sea_base_id
                         : Bases[base].auto_forward_air_base_id;
        if (destination >= 0 && destination != base) {
            Vehs[selected_vehicle_].order = 24;
            Vehs[selected_vehicle_].waypoint_x[0] = Bases[destination].x;
            Vehs[selected_vehicle_].waypoint_y[0] = Bases[destination].y;
        }
    }
    if (Vehs[selected_vehicle_].faction_id == ConsoleGlobal.faction_id_ &&
        (Vehs[selected_vehicle_].flags & 0x1000) && !Vehs[selected_vehicle_].order &&
        !(Vehs[selected_vehicle_].state & 0x200)) {
        Vehs[selected_vehicle_].flags &= ~0x1000;
        const int prototype = Vehs[selected_vehicle_].proto_id;
        const int chassis = VehPrototypes[prototype].chassis_id;
        const int weapon = VehPrototypes[prototype].weapon_id;
        const int triad = Chassis[chassis].triad;
        int sound = -1;
        if (triad == 0) {
            sound = chassis == 0 && Weapon[weapon].offense_rating != 0 ? 0x40
                : Weapon[weapon].offense_rating == 0 ? 0x42 : 0x41;
        } else if (triad == 1) sound = Weapon[weapon].offense_rating == 0 ? 0x46 : 0x45;
        else if (triad == 2) sound = !Chassis[chassis].range ? 0x43
            : Chassis[chassis].range == 1 ? 0x44 : 0x47;
        if (sound >= 0) g_FX.play(sound, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
    }
    ambience_veh(selected_vehicle_);
    flush_input();
}

int __cdecl popv(char *caption, const char *label, int vehicle, int flags);
// RECOVERY_OBJECT: v1 0x0068834C ?GoToLastSelection@@3HA
int GoToLastSelection = -1;
void __cdecl action_gate(int vehicle_id, int base_id);

static int console_destination_distance(int x1, int y1, int x2, int y2) {
    int dx = abs(x1 - x2);
    if (!(MapIsFlat & 1) && dx > static_cast<int>(MapLongitude)) dx = MapLongitudeBounds - dx;
    return (dx + abs(y1 - y2)) >> 1;
}

// ORIGINAL: 0x004D61A0 ?go_to@Console@@QAEXHHH@Z 0x004D61A0-0x004D7B3C;0x0065AD6A-0x0065AFD2
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004d61a0.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
// TRIED: full-function transcription (~95% of the call sequence by count) of a genuine multi-branch dispatcher: the a2==-1 multi-option popup, a2==-2 "next unit", a1<0/a2==-3 fake-veh "go home", the real-vehicle lock/cargo-check/base-at/action_home paths, the shared cursor-position tail, the 8-case jump-table message builder, the base-candidate eligibility+sort loop (fixed 512-entry stack arrays inferred from the frame span, not confirmed against a named constant), and the post-exec() a2 in {-2,-1,-3,default} redispatch. Confirmed via the exception funclet table (0x0065AD6A-0x0065AFD2) that every "standalone-looking" teardown local (2 FlatButton, Sprite, Heap, Spot, Dialogs, RadioButton/ListBox/EditGroup/SpriteBox/ CheckBox/Dialog/GraphicWin sub-objects, 2 StringStruct-shaped fields) is actually embedded inside Popup/BasePop/Scroll/ Dialogs at a fixed offset - so the whole function needs only one real local (`Popup popup;`) and the RAII lever applies to it alone. Approximated rather than exact: the per-pair diplomacy bit-matrix index feeding the eligibility test (0x004D7179-0x004D718B / 0x004D720B-0x004D721D), the speed-based "half speed" gate deep in that same loop (0x004D7382 area), and the name/distance tie-break in the candidate sort (0x004D706E-0x004D7124) - all replaced with a same-faction/plain-distance stand-in that keeps the same call shape. MISMATCH at instruction #12 (xor vs push), which is register/stack-slot allocation this early, not a logic fork.
void Console::go_to(int subject, int order, int grouped) {
    Popup popup;
    const unsigned int saved_flags = MapWinTable[0]->field_1DD70_;
    int sorting = prefs_get("Base Sort", 0, 0);
    int source_x, source_y, faction, triad = -1, forward_triad = -1;
    int centre_x = 0, centre_y = 0, maximum_range = 0;
    bool cargo = false;
    int ids[512], scores[512];
    GoToPreviewActive = 1;
    if (order == -1) {
        Base &base = Bases[subject];
        source_x = base.x; source_y = base.y; faction = base.faction_id_current;
        set_base(subject);
        if (NetDaemonNet.lock_base(subject, 0, -1, -1)) return;
        if (!(saved_flags & 0x40000)) {
            MapWinTable[0]->field_1DD70_ &= ~0x40u;
            MapWinTable[0]->field_1DD70_ |= 0x40000;
            ::draw_map(1);
        }
        parse_says(0, base.name_string, -1, -1);
        popup.start(PopupStartCaption, "DESTTRIAD", -1, nullptr, 0x40, nullptr);
        for (int i = 0; i < 3; ++i) {
            const int destination = i == 0 ? base.auto_forward_land_base_id
                : i == 1 ? base.auto_forward_sea_base_id : base.auto_forward_air_base_id;
            strcpy(StringTemp, StringTable.get(reinterpret_cast<int>(Triad[i])));
            strcat(StringTemp, " (");
            say_base(StringTemp, destination);
            strcat(StringTemp, ")");
            popup.dialogs().item(StringTemp, i);
        }
        forward_triad = popup.exec(0, nullptr);
        if (forward_triad < 0) goto finish;
        triad = forward_triad;
        parse_say(0, reinterpret_cast<int>(Triad[triad]), -1, -1);
        parse_says(1, base.name_string, -1, -1);
    } else if (order == -2) {
        source_x = ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
        source_y = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
        faction = ConsoleGlobal.faction_id_;
        GoToPreviewActive = 0;
    } else {
        if (subject < 0) {
            if (order == -3) return;
            subject = veh_fake(0, ConsoleGlobal.faction_id_);
            Vehs[subject].x = static_cast<short>(ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_]);
            Vehs[subject].y = static_cast<short>(ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_]);
            Vehs[subject].state = 0;
        }
        if (NetDaemonNet.lock_veh(&subject, 0, -1, -1, 0)) return;
        Veh &vehicle = Vehs[subject];
        source_x = vehicle.x; source_y = vehicle.y; faction = vehicle.faction_id;
        const int proto = vehicle.proto_id;
        triad = Chassis[VehPrototypes[proto].chassis_id].triad;
        parse_says(0, VehPrototypes[proto].veh_name, -1, -1);
        cargo = VehPrototypes[proto].plan == 12
            || (veh_cargo(subject) && stack_check(subject, 2, 12, -1, -1));
        if (order == -3) {
            const int base = base_at(source_x, source_y);
            if (base < 0) {
                NetMsgPrimary.pop("GATENOBASE", 0x1388, 0, 0);
                goto finish;
            }
            if (!has_fac(0x21, base, 0)) {
                NetMsgPrimary.pop("GATENOGATE", 0x1388, 0, 0);
                goto finish;
            }
            if (Bases[base].state & 0x20000000) {
                parse_says(0, Bases[base].name_string, -1, -1);
                NetMsgPrimary.pop("GATEUSED", 0x1388, 0, 0);
                goto finish;
            }
        }
        if (order == 128) {
            maximum_range = speed(subject, 0);
            if (Chassis[VehPrototypes[proto].chassis_id].range == 1) maximum_range /= 2;
            Map *tile = map_loc(source_x, source_y);
            if ((tile->bit & 1) && (tile->val2 & 15) < 8) {
                centre_x = source_x; centre_y = source_y;
            } else {
                action_home(subject, 0);
                if (vehicle.order != 24) { NetDaemonNet.unlock_veh(); return; }
                centre_x = vehicle.waypoint_x[0]; centre_y = vehicle.waypoint_y[0];
                vehicle.order = 0;
            }
        }
    }
    PlanWindow.init(label_get(0x868 / 4));
    GoToSourceX = PlanWindow.field_1DD9C_ = source_x;
    GoToSourceY = PlanWindow.field_1DDA0_ = source_y;
    PlanWindow.GraphicWin::show(0);
    for (;;) {
        const char *title = "";
        if (order == -3) title = "GATETO";
        else if (order == -2) title = "FINDTO";
        else if (order == -1) title = "DESTTO";
        else if (order == 24) title = grouped ? "GROUPTO" : "GOTO";
        else if (order == 27) title = "ROADTO";
        else if (order == 28) title = "TUBETO";
        else if (order == 128) title = "BOMBTO";
        strcpy(StringTemp, title);
        popup.start(PopupStartCaption, StringTemp, -1, nullptr, 0x42, nullptr);
        popup.button(label_get(0x70c / 4)); popup.button(label_get(0x710 / 4)); popup.button(label_get(0x714 / 4));
        popup.set_loc(-10, -10);
        if (order == -2) popup.button(label_get(0x718 / 4));
        int count = 0;
        if (order == -1) {
            strcpy(StringTemp, "("); strcat(StringTemp, label_get(0x3ec / 4)); strcat(StringTemp, ")");
            popup.dialogs().item(StringTemp, 9999); count = 1;
        }
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
            ids[i] = i;
            if (sorting == 1) scores[i] = Bases[i].population_size;
            else if (sorting == 2) scores[i] = -console_destination_distance(source_x, source_y, Bases[i].x, Bases[i].y);
        }
        for (int pass = 0; pass < GameDataStateStorage.fields_.base_current_count_; ++pass) for (int j = 0; j < GameDataStateStorage.fields_.base_current_count_ - pass - 1; ++j) {
            if (sorting ? scores[j] < scores[j + 1] : _stricmp(Bases[ids[j]].name_string, Bases[ids[j + 1]].name_string) > 0) {
                const int id = ids[j]; ids[j] = ids[j + 1]; ids[j + 1] = id;
                if (sorting) { const int score = scores[j]; scores[j] = scores[j + 1]; scores[j + 1] = score; }
            }
        }
        for (int candidate = 0; candidate < GameDataStateStorage.fields_.base_current_count_; ++candidate) {
            const int id = ids[candidate]; Base &base = Bases[id];
            if (base.faction_id_current != faction) {
                if (order == 128 ? !(PlayersData[faction].diplo_treaties[base.faction_id_current] & 0x10)
                    : order < 0 || !(PlayersData[faction].diplo_treaties[base.faction_id_current] & 1)) continue;
            } else if (order == 128) continue;
            if (order == -3) {
                if (!has_fac(0x21, id, 0)) continue;
            } else if (triad == 1) {
                if ((map_loc(source_x, source_y)->climate & 0xe0) >= 0x60) {
                    if (id >= 0 && !port_to_port(base_at(source_x, source_y), id)) continue;
                } else if (!base_on_sea(id, map_loc(source_x, source_y)->region)) continue;
            } else if (triad == 0) {
                if (map_loc(base.x, base.y)->region != map_loc(source_x, source_y)->region) continue;
            } else if (triad == 2) {
                if (order == 128) {
                    if (console_destination_distance(centre_x, centre_y, base.x, base.y) > maximum_range) continue;
                } else if (order >= 0) {
                    Veh &vehicle = Vehs[subject];
                    const int range = Chassis[VehPrototypes[vehicle.proto_id].chassis_id].range;
                    if (range) {
                        const int moves = speed(subject, 0);
                        const int remaining = ::range(moves - vehicle.moves_expended, 0, 999);
                        if (console_destination_distance(source_x, source_y, base.x, base.y)
                            > (moves * (range - vehicle.terraforming_turns - 1) + remaining)
                               / static_cast<int>(Rules.move_rate_roads)) continue;
                    }
                }
            }
            if (base.faction_id_current != faction && !(base.visibility & (1 << faction)) && !(GameDataStateStorage.fields_.game_state_ & 0x80)) continue;
            strcpy(StringTemp, base.name_string); strcat(StringTemp, "|");
            if (base.faction_id_current != faction) {
                strcat(StringTemp, " ("); GenderDefault = Players[base.faction_id_current].noun_gender;
                PluralityDefault = Players[base.faction_id_current].is_noun_plural;
                strcat(StringTemp, Players[base.faction_id_current].noun_faction); strcat(StringTemp, ")");
            }
            if (cargo && base.faction_id_current == faction) {
                if (id == GameDataStateStorage.fields_.secret_project_.universal_translator() || (has_fac(8, id, 0) && !(base.state & 0x400))) {
                    strcat(StringTemp, " (");
                    strcat(StringTemp, id == GameDataStateStorage.fields_.secret_project_.universal_translator()
                        ? StringTable.get(reinterpret_cast<int>(Facility[90].name)) : label_get(0x4c4 / 4));
                    strcat(StringTemp, ")");
                }
            }
            strcat(StringTemp, " ");
            if (order == -3) strcat(StringTemp, label_get((base.state & 0x20000000 ? 0x848 : 0x844) / 4));
            else {
                char number[40]; strcat(StringTemp, label_get(0x3e8 / 4)); strcat(StringTemp, ": ");
                strcat(StringTemp, _itoa(console_destination_distance(source_x, source_y, base.x, base.y), number, 10));
                if (sorting == 1) {
                    strcat(StringTemp, ", "); strcat(StringTemp, label_get(0x90c / 4)); strcat(StringTemp, ": ");
                    strcat(StringTemp, _itoa(base.population_size, number, 10));
                }
            }
            popup.dialogs().item(StringTemp, id); ++count;
            int preferred = GoToLastSelection;
            if (order == -1) preferred = forward_triad == 0 ? Bases[subject].auto_forward_land_base_id
                : forward_triad == 1 ? Bases[subject].auto_forward_sea_base_id : Bases[subject].auto_forward_air_base_id;
            else if (order != -2 && preferred < 0) preferred = Vehs[subject].home_base_id;
            if (id == preferred) popup.set_value(id);
        }
        if (!count) {
            if (order == -3) NetMsgPrimary.pop("GATENODEST", 0x1388, 0, 0);
            PlanWindow.close(); break;
        }
        popup.dialogs().set_selection_callback(goto_routine);
        goto_routine(popup.dialogs().get_selected_id());
        const int selected = popup.exec(0, nullptr);
        PlanWindow.close();
        if (selected < 0) break;
        GoToLastSelection = selected;
        if (popup.input_poll_state() > 0 && popup.input_poll_state() <= 3) {
            sorting = popup.input_poll_state() - 1; prefs_put("Base Sort", sorting, 0); continue;
        }
        if (order == -2) {
            ConsoleGlobal.cursor_next(Bases[selected].x, Bases[selected].y);
            MapWinTable[0]->set_center(Bases[selected].x, Bases[selected].y, 1);
            if (popup.input_poll_state() == 4 && Bases[selected].faction_id_current == ConsoleGlobal.faction_id_)
                TutWinBaseWindow.zoom(selected, 0);
        } else if (order == -1) {
            const short destination = static_cast<short>(selected == 9999 ? -1 : selected);
            if (!forward_triad) Bases[subject].auto_forward_land_base_id = destination;
            else if (forward_triad == 1) Bases[subject].auto_forward_sea_base_id = destination;
            else Bases[subject].auto_forward_air_base_id = destination;
            ::draw_map(1);
        } else if (order == -3) {
            if (Bases[selected].x != Vehs[subject].x || Bases[selected].y != Vehs[subject].y) {
                log_say("Psi Gate teleport", subject, selected, 0);
                if (!NetDaemonNet.add_lock(0, Bases[selected].x, Bases[selected].y)) {
                    if (IsMultiplayerNet) { message_veh(0x2410, subject, selected, 0); NetDaemonNet.await_exec(1); }
                    else action_gate(subject, selected);
                }
            }
        } else {
            log_say("Setting special veh orders", subject, order, 0);
            Veh &vehicle = Vehs[subject];
            if (order == 128) {
                vehicle.state = (vehicle.state & 0xfcffffff) | 0x200;
                vehicle.order_auto_type = 10;
                vehicle.waypoint_x[1] = Bases[selected].x; vehicle.waypoint_y[1] = Bases[selected].y;
            } else {
                vehicle.order = static_cast<signed char>(order);
                vehicle.waypoint_x[0] = Bases[selected].x; vehicle.waypoint_y[0] = Bases[selected].y;
                if (order == 24) { vehicle.state &= ~0x10000u; if (grouped) group(subject); }
            }
            synch_veh(subject); NetDaemonNet.await_synch();
        }
        break;
    }
finish:
    if (order == -1) NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    else if (order != -2) NetDaemonNet.unlock_veh();
    if (MapWinTable[0]->field_1DD70_ != saved_flags) {
        MapWinTable[0]->field_1DD70_ = saved_flags; ::draw_map(1);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D92D0 ?group@Console@@QAEXH@Z 0x004D92D0-0x004D95E7
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d92d0.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::group(int vehicle_id) {
    Veh &leader = Vehs[vehicle_id];
    const int source_x = leader.x, source_y = leader.y;
    const int faction = leader.faction_id;
    const int destination_region = map_loc(leader.waypoint_x[0], leader.waypoint_y[0])->region;
    leader.state |= 0x08000000;
    for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; ++i) {
        Veh &vehicle = Vehs[i];
        if (i == vehicle_id || vehicle.faction_id != faction) continue;
        if (field_23D1C_ ? !(vehicle.state & 0x08000000)
                         : vehicle.x != source_x || vehicle.y != source_y) continue;
        const int region = map_loc(vehicle.x, vehicle.y)->region;
        if (region != destination_region) {
            const int triad = Chassis[VehPrototypes[vehicle.proto_id].chassis_id].triad;
            if (triad != 2) {
                if ((region < 0x40) == (destination_region < 0x40) || triad == 0) continue;
                if (triad == 1) {
                    const int destination = base_at(leader.waypoint_x[0], leader.waypoint_y[0]);
                    const int source = base_at(vehicle.x, vehicle.y);
                    if (destination >= 0 && source >= 0 && !port_to_port(destination, source)) continue;
                }
            }
        }
        vehicle.state &= 0xfcffbdff;
        vehicle.order = leader.order;
        vehicle.waypoint_count = leader.waypoint_count;
        vehicle.patrol_current_point = leader.patrol_current_point;
        for (int waypoint = 0; waypoint < 4; ++waypoint) {
            vehicle.waypoint_x[waypoint] = leader.waypoint_x[waypoint];
            vehicle.waypoint_y[waypoint] = leader.waypoint_y[waypoint];
        }
        vehicle.state |= (leader.state & 0x02000200) | 0x08000000;
    }
    for (int slot = 0; slot < 8; ++slot) {
        MapWin *window = MapWinTable[slot];
        if (!window || (slot && !window->field_1DD74_)) continue;
        window->field_1DD84_ = 1;
        for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; ++i) {
            Veh &vehicle = Vehs[i];
            if (vehicle.faction_id == ConsoleGlobal.faction_id_ && (vehicle.state & 0x08000000)) {
                MapWinTable[slot]->dest_line(vehicle.x, vehicle.y, vehicle.waypoint_x[0], vehicle.waypoint_y[0],
                    vehicle.proto_id, vehicle.faction_id,
                    Chassis[VehPrototypes[vehicle.proto_id].chassis_id].triad, 0, 2, 2, 0);
            }
        }
        on_update(); // Original slot 63 belongs to this Console, not the iterated MapWin.
        MapWinTable[slot]->field_1DD84_ = 0;
    }
    field_23D1C_ = 1;
    ::draw_map(1);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D2F00 ?activate@Console@@QAEXHHH@Z 0x004D2F00-0x004D3467
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d2f00.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::activate(int x, int y, int current_selection) {
    log_say("Console ACTIVATE", x, y, 0);
    if (IsMultiplayerNet) {
        if (NetDaemonNet.lock_state_[2].leading_words_[2] ||
            NetDaemonNet.lock_state_[2].payload_.flags) return;
        const unsigned int faction_bit = 1u << ConsoleGlobal.faction_id_;
        if (((NetDaemonNet.lock_state_[0].trailing_words_[7] & faction_bit) &&
             !(GameDataStateStorage.fields_.net_turn_flags_ & 0x10)) || ConsoleControlTurnActive) {
            int vehicle = veh_at(x, y);
            if (stack_check(vehicle, 1, -1, -1, -1) != 1)
                vehicle = veh_pick(vehicle, "PICKUNIT", 1);
            if (vehicle >= 0) {
                selected_vehicle_ = vehicle;
                focus_vehicle_ = vehicle;
            }
            const bool ready_now = (NetDaemonNet.lock_state_[0].trailing_words_[7] &
                (1u << ConsoleGlobal.faction_id_)) && veh_ready(vehicle);
            NetMsgPrimary.pop(ready_now ? "ALREADYDONE" : "FIRSTMOVE", 2000, 0, nullptr);
            return;
        }
    }
    int vehicle;
    if (ConsoleGlobal.cursor_active_ && current_selection) {
        field_23BE4_ = 0;
        vehicle = selected_vehicle_;
        if (stack_check(vehicle, 1, ConsoleGlobal.faction_id_, -1, -1) == 1 &&
            veh_selectable(vehicle)) {
            field_23BEC_ = 1;
            return;
        }
        vehicle = veh_pick(vehicle, "PICKUNIT", 1);
        if (vehicle < 0) return;
    } else {
        vehicle = veh_at(x, y);
        if (vehicle < 0 || !stack_check(vehicle, 1, ConsoleGlobal.faction_id_, -1, -1)) {
            field_23BE4_ = 0;
            return;
        }
        if (stack_check(vehicle, 1, -1, -1, -1) != 1 ||
            (!veh_selectable(vehicle) && !Vehs[vehicle].order && !(Vehs[vehicle].state & 0x4200))) {
            vehicle = veh_pick(vehicle, "PICKUNIT", 1);
            if (vehicle < 0) return;
        } else {
            log_say("VEH ORDERS", vehicle, 0, 0);
            if (Vehs[vehicle].order > 2 && !(GameDataStateStorage.fields_.game_more_preferences_ & 0x40000)) {
                parse_says(0, VehPrototypes[Vehs[vehicle].proto_id].veh_name, -1, -1);
                StringTemp[0] = 0;
                say_orders(vehicle);
                parse_says(1, StringTemp, -1, -1);
                StringTemp[0] = 0;
                strcat(StringTemp, "VEHORDERS");
                const int order = Vehs[vehicle].order;
                if (order >= 4 && order < 24) {
                    parse_num(0, action_terraform(vehicle, order - 4, 0) + 1);
                    strcat(StringTemp, "1");
                }
                const int answer = X_pop(StringTemp, nullptr);
                if (!answer) return;
                if (answer == 2) {
                    GameDataStateStorage.fields_.game_more_preferences_ |= 0x40000;
                    prefs_save(0);
                }
            }
        }
    }
    if (Vehs[vehicle].faction_id != faction_id_) return;
    log_say("Activate AWAKEN", vehicle, Vehs[vehicle].x, Vehs[vehicle].y);
    if (veh_jail(vehicle)) {
        NetMsgPrimary.pop("INJAIL", 5000, 0, nullptr);
        return;
    }
    if (IsMultiplayerNet && veh_selectable(vehicle) && !Vehs[vehicle].order &&
        !(Vehs[vehicle].state & 0x8004200)) {
        selected_vehicle_ = vehicle;
        set_move(1);
        field_23BEC_ = 1;
        return;
    }
    if (NetDaemonNet.lock_veh(&vehicle, 0, -1, -1, 0)) return;
    log_say("Activate AWAKENING", vehicle, Vehs[vehicle].x, Vehs[vehicle].y);
    veh_wake(vehicle);
    if (!veh_selectable(vehicle)) {
        draw_tile(Vehs[vehicle].x, Vehs[vehicle].y, 1);
        if (!ConsoleGlobal.cursor_active_ && !(GameDataStateStorage.fields_.game_state_ & 2) &&
            (!IsMultiplayerNet || (!ConsoleControlTurnActive &&
             !NetDaemonNet.lock_state_[2].payload_.flags))) {
            if (popv(ScriptPopupFile, "ALREADYMOVED2", vehicle, 0))
                set_move(0);
        } else if (IsMultiplayerNet) {
            NetMsgPrimary.pop("ALREADYMOVED", 5000, 0, nullptr);
        } else {
            popv(ScriptPopupFile, "ALREADYMOVED", vehicle, 0);
        }
    } else field_23BE4_ = 0;
    selected_vehicle_ = vehicle;
    synch_veh(vehicle);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
    set_move(1);
    field_23BEC_ = 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D26A0 ?give_veh@Console@@QAEXH@Z 0x004D26A0-0x004D2ABC
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 004D26A0; retained seed src/recovered/units/004d26a0.cpp", "Native4D270F observes receiver faction after whose_territory;4D2889/296C reload vehicle prototype after the first parser;4D298F..4D2A28 reload receiver flags and current chassis after the second parser. Retain local action_give versus message2416 and original unlock/cancel paths.", "recovery/tasks/console-scenario-owner-wave.json; native scope decisions preserve the original baseline"], "reason": "Recover the complete Console command in its genuine owner, retaining original spans and seed evidence. Typed state and lifecycle/network APIs replace seed address arithmetic; this source binding is not byte, reference, structure or runtime acceptance."}
void Console::give_veh(int vehicle_id) { // 0x004D26A0
    if (NetDaemonNet.lock_veh(&vehicle_id, 4, -1, -1, 0) != 0) {
        return;
    }

    const Veh &vehicle = Vehs[vehicle_id];
    if (vehicle.faction_id != ConsoleGlobal.faction_id_) {
        NetDaemonNet.unlock_veh();
        return;
    }

    const int x = vehicle.x;
    const int y = vehicle.y;
    int recipient_faction = whose_territory(ConsoleGlobal.faction_id_, x, y,
                                             nullptr, nullptr);
    const int receiver_faction = faction_id_;
    const int prototype_id = vehicle.proto_id;
    const VehPrototype &prototype = VehPrototypes[prototype_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    const int8_t weapon_rating = Weapon[prototype.weapon_id].offense_rating;

    if (recipient_faction == receiver_faction || recipient_faction < 0 ||
        (PlayersData[receiver_faction].diplo_treaties[recipient_faction] &
         DTREATY_COMMLINK) == 0 ||
        (PlayersData[receiver_faction].diplo_treaties[recipient_faction] &
         DTREATY_VENDETTA) != 0) {
        if (prototype_id > 0x3F || (weapon_rating >= 0 && prototype_id != 0xF)) {
            NetDaemonNet.unlock_veh();
            NetMsgPrimary.pop("CANTGIVE", 5000, 0, nullptr);
            return;
        }
        recipient_faction = 0;
    }

    const uint8_t unit_class = chassis.triad;
    if (unit_class == TRIAD_AIR) {
        const Map *const tile = map_loc(x, y);
        const bool blocked = (tile->bit & BIT_BASE_IN_TILE) == 0 ||
                             (tile->val2 & 0x0F) > 7;
        if (blocked && (tile->bit & BIT_AIRBASE) == 0) {
            NetDaemonNet.unlock_veh();
            NetMsgPrimary.pop("CANTGIVEAIR", 5000, 0, nullptr);
            return;
        }
    }

    const bool prototype_unit =
        prototype_id < 0x40 && (weapon_rating < 0 || prototype_id == 0xF);
    GenderDefault = Players[recipient_faction].noun_gender;
    PluralityDefault = Players[recipient_faction].is_noun_plural;
    parse_says(0, Players[recipient_faction].noun_faction, -1, -1);
    parse_says(1, VehPrototypes[vehicle.proto_id].veh_name, -1, -1);

    if (prototype_unit && recipient_faction != 0) {
        const int answer = popp(ScriptPopupFile, "CANGIVE2", 0,
                                "native_sm.pcx", nullptr);
        if (answer == 0) {
            NetDaemonNet.unlock_veh();
            return;
        }
        if (answer == 2) {
            recipient_faction = 0;
        }
    } else {
        const char *label;
        const char *picture;
        if (prototype_unit) {
            label = "CANGIVE1";
            picture = "native_sm.pcx";
        } else {
            label = "CANGIVE";
            const int current_receiver_faction = faction_id_;
            const bool alien =
                (Players[current_receiver_faction].rule_flags & RFLAG_ALIEN) != 0;
            const int current_prototype_id = vehicle.proto_id;
            const uint8_t current_unit_class = Chassis[
                VehPrototypes[current_prototype_id].chassis_id].triad;
            if (alien) {
                picture = current_unit_class == TRIAD_AIR ? "alair_sm.pcx" :
                          current_unit_class == TRIAD_SEA ? "navun_sm.pcx" : "alunit_sm.pcx";
            } else {
                picture = current_unit_class == TRIAD_AIR ? "air_sm.pcx" :
                          current_unit_class == TRIAD_SEA ? "navun_sm.pcx" : "unit_sm.pcx";
            }
        }
        if (popp(ScriptPopupFile, label, 0, picture, nullptr) == 0) {
            NetDaemonNet.unlock_veh();
            return;
        }
    }

    if (!IsMultiplayerNet) {
        action_give(vehicle_id, recipient_faction);
        return;
    }
    message_veh(0x2416, vehicle_id, recipient_faction, 0);
    NetDaemonNet.await_exec(1);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00514B30 ?retire_game@Console@@QAEXXZ 0x00514B30-0x00514D2B
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00514B30; retained seed src/recovered/units/00514b30.cpp", "Native514B55..64 passes the low-byte greater-than-one boolean into bit_count. Native514D10 writes closed-network==0 to93A948 GameRestartQueued,514D16 sets9B2068 ExitTurnLoop. Existing typed close-network helper reproduces lock clear/transport close; scenario, PBEM, ironman and replay branches remain distinct.", "recovery/tasks/console-scenario-owner-wave.json; native scope decisions preserve the original baseline"], "reason": "Recover the complete Console command in its genuine owner, retaining original spans and seed evidence. Typed state and lifecycle/network APIs replace seed address arithmetic; this source binding is not byte, reference, structure or runtime acceptance."}
void Console::retire_game() { // 0x00514B30
    if (GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) {
        auto_save();
    }

    const bool multiple_players =
        (IsMultiplayerNet || IsMultiplayerPBEM) &&
        bit_count(static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) > 1) != 0;
    const char *const prompt = multiple_players ? "REALLYWIMPOUT" :
        (GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) ? "REALLYRETIRE2" : "REALLYRETIRE";
    if (!X_pop(prompt, nullptr)) {
        return;
    }

    if (IsMultiplayerPBEM) {
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~static_cast<uint8_t>(1u << ConsoleGlobal.faction_id_);
        save_game(0);
    }

    bool closed_network = false;
    if (IsMultiplayerNet) {
        console_close_network(0xFF);
        closed_network = true;
    }

    if (GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) {
        kill_auto_save();
    }
    report_score(1);
    if ((GameDataStateStorage.fields_.game_state_ & STATE_IS_SCENARIO) == 0) {
        quayle(ConsoleGlobal.faction_id_);
        hall_of_fame(1);
    }
    if (!closed_network) {
        show_replay();
    }
    GameRestartQueued = !closed_network;
    ExitTurnLoop = 1;
    log_say("Retire**", 0, 0, 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00515F70 ?show_movies@Console@@QAEXXZ 0x00515F70-0x00516291;0x0065D3F3-0x0065D4D0
// RECOVERY_BINDING: v1 {"evidence": ["Native515FA5..516035 rebuilds64 FacilitySPStart rows through StringTable and owned Dialogs, sorts the virtual-base StringList and dispatches403BA0 until exec returns negative; Popup lifetime is compiler-managed.", "src/recovered/units/00515f70.cpp", "Root-only console-movie-wrapper-20261001 central dependency closure; immutable task baseline preserved."], "reason": "Recover complete source-owned movie selection and project-index wrapper with proper Popup lifetime, typed facilities, actual text parsing and existing movie playback. Retain exact seed identities and all original spans; no match or acceptance claim is granted by this binding."}
void Console::show_movies() { // 0x00515F70
    Popup popup;
    for (;;) {
        popup.start(PopupStartCaption, "SHOWMOVIES", -1, nullptr, 0x42,
                    nullptr);

        for (int project_id = 0; project_id < MaxSecretProjectNum;
             ++project_id) {
            StringTemp[0] = 0;
            const int facility_id = FacilitySPStart + project_id;
            strcat(StringTemp, StringTable.get(
                static_cast<int>(
                    reinterpret_cast<intptr_t>(Facility[facility_id].name))));
            popup.dialogs().item(StringTemp, project_id);
        }
        popup.item_ids_.sort(0);

        const int selected_project = popup.exec(0, nullptr);
        if (selected_project < 0) {
            return;
        }
        amovie_project(selected_project);
    }
}


// ORIGINAL: 0x004D06C0 ?upgrade@Console@@QAEHH@Z 0x004D06C0-0x004D2218;0x0065A090-0x0065A559
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004d06c0.cpp", "image:0x004D06C0-0x004D2218 and 0x0065A090-0x0065A559"], "reason": "Recover Console vehicle upgrade through real Popup lifetime, loaded design tables, faction-local candidate selection and live human/AI transaction state. Preserve the original seed and complete ordinary/unwind spans; native locks, callbacks, picture selection and completion publication identify the owner and argument roles."}
// TRIED: mismatch #0 - Popup/BasePop declared as real C++ locals (Popup ctor + explicit close()) rather than the original's staggered same-slot vtable pokes; the 0x40-entry candidate scan uses Vehs[]/VehPrototypes[] (0x952828/0x9AB868, stride 0x34) by raw offset, and the "single auto-selected candidate" fast path is approximated (falls through rather than tracking which slot was picked) - faithful call order and eligibility conditions, not byte fidelity.
int Console::upgrade(int veh_id) { // 0x004D06C0
    // The original constructs a Popup before its negative-id early exit.
    // Its BasePop and Scroll subobjects are ordinary members, so keep their
    // lifetime in C++ rather than reconstructing the old local vtable writes.
    Popup popup;
    if (veh_id < 0) {
        popup.close();
        return 0;
    }

    const int faction_id = Vehs[veh_id].faction_id;
    const uint32_t faction_bit = 1U << faction_id;
    const uint8_t human_factions = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    const bool human_faction = (human_factions & faction_bit) != 0;
    if (Vehs[veh_id].moves_expended != 0 && human_faction) {
        NetMsgPrimary.pop(IsMultiplayerNet ? "UNITMOVED" : "UNITMOVED2",
                         5000, 0, nullptr);
        popup.close();
        return 0;
    }

    int proto_id;
    if (human_faction) {
        if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
            popup.close();
            return 0;
        }

        // lock_veh may rewrite the ID. The original pre-lock faction stays
        // in use, while this moved-state check addresses the returned slot.
        if (Vehs[veh_id].moves_expended != 0) {
            NetMsgPrimary.pop(IsMultiplayerNet ? "UNITMOVED" : "UNITMOVED2",
                             5000, 0, nullptr);
            NetDaemonNet.unlock_veh();
            popup.close();
            return 0;
        }
    }

    if (human_faction) {
    // Parse the current-unit caption before building candidates. Re-read the
    // selected vehicle's prototype after parser callbacks, as the native path
    // does rather than carrying a stale record reference into the list pass.
    proto_id = Vehs[veh_id].proto_id;
    const VehPrototype &caption_prototype = VehPrototypes[proto_id];
    StringTemp[0] = '\0';
    strcat(StringTemp, caption_prototype.veh_name);
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    say_stats_2(StringTemp, proto_id);
    if (caption_prototype.ability_flags != 0) {
        strcat(StringTemp, " ");
        for (int ability_id = 0; ability_id < MaxAbilityNum; ++ability_id) {
            if (has_abil(proto_id, 1 << ability_id)) {
                strcat(StringTemp,
                       StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(
                           Ability[ability_id].abbreviation))));
            }
        }
    }
    strcat(StringTemp, ")");
    parse_says(0, StringTemp, -1, -1);
    popup.start(PopupStartCaption, "WHICHUPGRADE", -1, nullptr, 0x40);

    // The popup's embedded Sprite is initialized from the selected unit's
    // faction/triad artwork. Native first probes dimensions using the current
    // prototype, then reloads the prototype after that call before choosing
    // the filename passed to Sprite::init.
    const bool alien_faction =
        (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;
    const int picture_proto_id = Vehs[veh_id].proto_id;
    const uint8_t picture_triad =
        Chassis[VehPrototypes[picture_proto_id].chassis_id].triad;
    const char *picture_file = alien_faction
                                   ? (picture_triad == TRIAD_AIR
                                          ? "alair_sm.pcx"
                                          : picture_triad == TRIAD_SEA
                                                ? "navun_sm.pcx"
                                                : "alunit_sm.pcx")
                                   : (picture_triad == TRIAD_AIR
                                          ? "air_sm.pcx"
                                          : picture_triad == TRIAD_SEA
                                                ? "navun_sm.pcx"
                                                : "unit_sm.pcx");
    int picture_width = 0;
    int picture_height = 0;
    if (Buffer::get_pcx_dimensions(picture_file, &picture_width,
                                  &picture_height) == 0) {
        const int sprite_proto_id = Vehs[veh_id].proto_id;
        const uint8_t sprite_triad =
            Chassis[VehPrototypes[sprite_proto_id].chassis_id].triad;
        char *sprite_file = const_cast<char *>(
            alien_faction
                ? (sprite_triad == TRIAD_AIR
                       ? "alair_sm.pcx"
                       : sprite_triad == TRIAD_SEA ? "navun_sm.pcx"
                                                   : "alunit_sm.pcx")
                : (sprite_triad == TRIAD_AIR
                       ? "air_sm.pcx"
                       : sprite_triad == TRIAD_SEA ? "navun_sm.pcx"
                                                   : "unit_sm.pcx"));
        if (popup.owned_picture().init(sprite_file, picture_width,
                                       picture_height) == 0) {
            popup.select_owned_picture();
        }
    }
    }

    proto_id = Vehs[veh_id].proto_id;
    const VehPrototype &current = VehPrototypes[proto_id];
    // Walk the native candidate set in prototype-table order. The image calls
    // upgrade_cost for every eligible design, including the AI path.
    const int current_weapon_rating =
        Weapon[current.weapon_id].offense_rating;
    const int current_armor_rating =
        Armor[current.armor_id].defense_rating;
    int candidate_count = 0;
    int only_candidate_id = -1;

    bool scan_candidates = true;
    if (proto_id < MaxVehProtoFactionNum &&
        (Weapon[current.weapon_id].offense_rating < 0 || proto_id == 15)) {
        scan_candidates = false;
    }
    const int first_candidate_id = faction_id * MaxVehProtoFactionNum;
    for (int candidate_id = first_candidate_id; scan_candidates &&
         candidate_id < first_candidate_id + MaxVehProtoFactionNum;
         ++candidate_id) {
        const VehPrototype &candidate = VehPrototypes[candidate_id];
        if ((candidate.flags & 1U) == 0) {
            continue;
        }

        const uint8_t human_factions =
            static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        const bool human_faction = (human_factions & faction_bit) != 0;
        if ((candidate.flags & 4U) == 0 && !human_faction) {
            continue;
        }
        if ((static_cast<uint8_t>(candidate.obsolete_factions) &
             faction_bit) != 0 || candidate_id == proto_id) {
            continue;
        }
        if (!human_faction && candidate.unk_1 != current.unk_1) {
            continue;
        }
        if (candidate.chassis_id != current.chassis_id ||
            has_abil(candidate_id, 0x8000) != has_abil(proto_id, 0x8000)) {
            continue;
        }
        if (candidate.plan != current.plan &&
            (!human_faction || current.plan > 3 || candidate.plan > 3)) {
            continue;
        }

        const int candidate_weapon_rating =
            Weapon[candidate.weapon_id].offense_rating;
        if ((current_weapon_rating == 0 &&
             candidate.weapon_id != current.weapon_id) ||
            (current_weapon_rating != 0 &&
             current_weapon_rating > candidate_weapon_rating)) {
            continue;
        }

        const int candidate_armor_rating =
            Armor[candidate.armor_id].defense_rating;
        if (current_armor_rating > candidate_armor_rating &&
            (!human_faction || candidate_armor_rating == 1 ||
             (current_weapon_rating != 0 &&
              candidate_weapon_rating <= current_weapon_rating))) {
            continue;
        }

        const int displayed_cost =
            upgrade_cost(faction_id, proto_id, candidate_id) * 10;
        if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) == 0) {
            // The non-human path evaluates the first eligible prototype
            // immediately. Its native threshold is based on the highest and
            // lowest ranked human factions' energy/rank state, then the
            // active faction's AI settings and base count.
            const int ranking_divisor =
                8 - static_cast<int>(PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].ranking);
            int energy_threshold =
                (static_cast<int>(static_cast<uint32_t>(GameDataStateStorage.fields_.diff_level_current_) *
                                  static_cast<uint32_t>(displayed_cost)) /
                 10) /
                ranking_divisor;
            const int faction_energy = PlayersData[faction_id].energy_reserves;
            const int comparison_energy =
                PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].energy_reserves;
            if (faction_energy > comparison_energy) {
                if (faction_energy >= 1000) {
                    energy_threshold /= 2;
                }
                if (faction_energy >= 2000) {
                    energy_threshold = 0;
                }
            }

            const int ai_cost = static_cast<int>(
                static_cast<uint32_t>(displayed_cost) -
                static_cast<uint32_t>(energy_threshold));
            uint32_t denominator_bits = static_cast<uint32_t>(candidate.plan != 2);
            denominator_bits -= static_cast<uint32_t>(PlayersData[faction_id].ai_power);
            denominator_bits += static_cast<uint32_t>(PlayersData[faction_id].ai_wealth);
            denominator_bits += PlayersData[faction_id].current_num_bases + 1U;
            const int denominator = static_cast<int>(denominator_bits);
            if (ai_cost >= faction_energy / denominator) {
                popup.close();
                return 0;
            }

            PlayersData[faction_id].energy_reserves -= ai_cost;
            const int old_proto_id = Vehs[veh_id].proto_id;
            --PlayersData[faction_id].proto_id_active[old_proto_id];
            Vehs[veh_id].proto_id = static_cast<uint16_t>(candidate_id);
            ++PlayersData[faction_id].proto_id_active[candidate_id];
            if (Chassis[VehPrototypes[candidate_id].chassis_id].triad != TRIAD_AIR) {
                veh_skip(veh_id);
            }
            popup.close();
            return 1;
        }

        if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0) {
            StringTemp[0] = '\0';
            strcat(StringTemp, candidate.veh_name);
            strcat(StringTemp, " ");
            strcat(StringTemp, "(");
            say_stats_2(StringTemp, candidate_id);
            if (candidate.ability_flags != 0) {
                strcat(StringTemp, " ");
                for (int ability_id = 0; ability_id < MaxAbilityNum;
                     ++ability_id) {
                    if (has_abil(candidate_id, 1 << ability_id)) {
                        strcat(StringTemp,
                               StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(
                                   Ability[ability_id].abbreviation))));
                    }
                }
            }
            strcat(StringTemp, ")");
            strcat(StringTemp, "|");
            strcat(StringTemp, label_get(88));
            strcat(StringTemp, ": ");
            char cost_text[16];
            _itoa(displayed_cost, cost_text, 10);
            strcat(StringTemp, cost_text);
            if ((candidate.flags & 4U) == 0) {
                strcat(StringTemp, "*");
            }
            ++candidate_count;
            only_candidate_id = candidate_id;
            popup.dialogs().item(StringTemp, candidate_id);
        }
    }

    // The native no-candidate branch notifies before releasing the lock.
    if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) != 0 &&
        candidate_count == 0) {
        NetMsgPrimary.pop("NOUPGRADE", 5000, 0, nullptr);
        NetDaemonNet.unlock_veh();
        popup.close();
        return 0;
    }

    // AI candidates have already returned from their score/commit path in
    // the loop. Reaching here without one means the native scan found none.
    if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_bit) == 0) {
        popup.close();
        return 0;
    }

    // A sole human candidate is selected directly; with multiple candidates,
    // the popup returns the prototype ID stored in the list item.
    int selected_proto_id = only_candidate_id;
    if (candidate_count > 1) {
        selected_proto_id = popup.exec(0, nullptr);
        if (selected_proto_id < 0) {
            NetDaemonNet.unlock_veh();
            popup.close();
            return 0;
        }
    }

    // The selection and confirmation callbacks can change the vehicle's
    // prototype. Native reloads it for pricing, then reloads it again after
    // the confirmation before changing the prototype-count table.
    proto_id = Vehs[veh_id].proto_id;
    const VehPrototype &selected = VehPrototypes[selected_proto_id];
    StringTemp[0] = '\0';
    strcat(StringTemp, VehPrototypes[proto_id].veh_name);
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    say_stats_2(StringTemp, proto_id);
    if (VehPrototypes[proto_id].ability_flags != 0) {
        strcat(StringTemp, " ");
        for (int ability_id = 0; ability_id < MaxAbilityNum; ++ability_id) {
            if (has_abil(proto_id, 1 << ability_id)) {
                strcat(StringTemp,
                       StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(
                           Ability[ability_id].abbreviation))));
            }
        }
    }
    strcat(StringTemp, ")");
    parse_says(0, StringTemp, -1, -1);

    StringTemp[0] = '\0';
    strcat(StringTemp, selected.veh_name);
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    say_stats_2(StringTemp, selected_proto_id);
    if (selected.ability_flags != 0) {
        strcat(StringTemp, " ");
        for (int ability_id = 0; ability_id < MaxAbilityNum; ++ability_id) {
            if (has_abil(selected_proto_id, 1 << ability_id)) {
                strcat(StringTemp,
                       StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(
                           Ability[ability_id].abbreviation))));
            }
        }
    }
    strcat(StringTemp, ")");
    parse_says(1, StringTemp, -1, -1);

    // Parsing the selected-design prompt can mutate the vehicle's prototype;
    // the native pricing call reloads that field only after both parses.
    proto_id = Vehs[veh_id].proto_id;
    const int selected_cost =
        upgrade_cost(faction_id, proto_id, selected_proto_id) * 10;
    parse_num(0, selected_cost);
    parse_num(1, PlayersData[faction_id].energy_reserves);
    if (selected_cost > PlayersData[faction_id].energy_reserves) {
        NetDaemonNet.unlock_veh();
        const bool alien = (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;
        const uint8_t triad = Chassis[selected.chassis_id].triad;
        const char *const picture = alien
                                        ? (triad == TRIAD_AIR
                                               ? "alair_sm.pcx"
                                               : triad == TRIAD_SEA
                                                     ? "navun_sm.pcx"
                                                     : "alunit_sm.pcx")
                                        : (triad == TRIAD_AIR
                                               ? "air_sm.pcx"
                                               : triad == TRIAD_SEA
                                                     ? "navun_sm.pcx"
                                                     : "unit_sm.pcx");
        const char *const label = (selected.flags & 4U) != 0
                                      ? "COSTTHISUPGRADE"
                                      : "COSTTHISUPGRADE2";
        popp(ScriptPopupFile, label, 0, picture, pop_wait);
        popup.close();
        return 0;
    }

    const bool alien = (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;
    const uint8_t triad = Chassis[selected.chassis_id].triad;
    const char *const confirmation_picture =
        alien ? (triad == TRIAD_AIR
                     ? "alair_sm.pcx"
                     : triad == TRIAD_SEA ? "navun_sm.pcx" : "alunit_sm.pcx")
              : (triad == TRIAD_AIR
                     ? "air_sm.pcx"
                     : triad == TRIAD_SEA ? "navun_sm.pcx" : "unit_sm.pcx");
    const char *const confirmation_label = (selected.flags & 4U) != 0
                                               ? "THISUPGRADE"
                                               : "THISUPGRADE2";
    if (!popp(ScriptPopupFile, confirmation_label, 0,
              confirmation_picture, nullptr)) {
        NetDaemonNet.unlock_veh();
        popup.close();
        return 0;
    }

    PlayersData[faction_id].energy_reserves -= selected_cost;
    synch_energy(faction_id);
    const int old_proto_id = Vehs[veh_id].proto_id;
    --PlayersData[faction_id].proto_id_active[old_proto_id];
    Vehs[veh_id].proto_id = static_cast<uint16_t>(selected_proto_id);
    ++PlayersData[faction_id].proto_id_active[selected_proto_id];

    // Keep the original's post-write reads: both ability probes address the
    // now-updated vehicle record, even though this makes the normal delta a
    // no-op unless intervening code changed its prototype.
    if (has_abil(selected_proto_id, 0x4000) != 0 &&
        has_abil(Vehs[veh_id].proto_id, 0x4000) == 0) {
        const int morale = Vehs[veh_id].morale + 1;
        Vehs[veh_id].morale = static_cast<uint8_t>(morale < 0 ? 0 :
                                                    morale > 6 ? 6 : morale);
    }
    if (has_abil(selected_proto_id, 0x4000) == 0 &&
        has_abil(Vehs[veh_id].proto_id, 0x4000) != 0) {
        const int morale = Vehs[veh_id].morale - 1;
        Vehs[veh_id].morale = static_cast<uint8_t>(morale < 0 ? 0 :
                                                    morale > 6 ? 6 : morale);
    }
    veh_skip(veh_id);
    draw_tile(Vehs[veh_id].x, Vehs[veh_id].y, 1);
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    if ((VehPrototypes[selected_proto_id].flags & PROTO_TYPED_COMPLETE) == 0) {
        VehPrototypes[selected_proto_id].flags |= PROTO_TYPED_COMPLETE;
        synch_proto(selected_proto_id);
        NetDaemonNet.await_synch();
    }
    NetDaemonNet.unlock_veh();
    popup.close();
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00513B90 ?veh_turn@Console@@QAEHXZ 0x00513B90-0x00513FB9
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00513b90.cpp; image:0x00513B90-0x00513FB9", "Native513EA3 identifies pointer-preserving NetDaemon::automated_lock; native513F85 clears selected ID only before turn-loop entry."], "reason": "Recover selected-vehicle turn scheduling through real Console state, NetDaemon locking and UI owners. Native branches distinguish cursor activity, readiness and ordered actions, retaining selection after an entered turn loop and clearing it on early refusal; no raw receiver accesses or executable callbacks."}
int Console::veh_turn() { // 0x00513B90
    int result = 0;
    int flag10 = 0;
    int n = 0;
    field_23BE4_ = 1;

    if ((GameDataStateStorage.fields_.game_preferences_ & 0x20u) != 0 &&
        Vehs[selected_vehicle_].order == 0) {
        give_hints();
    }

    const int first_ready = veh_ready(selected_vehicle_);
    for (;;) {
        if (!veh_selectable(selected_vehicle_) ||
            veh_ready(selected_vehicle_) != first_ready ||
            (first_ready == 0 && Vehs[selected_vehicle_].order != 0)) {
            if (ConsoleGlobal.cursor_active_ != 0) {
                goto finish_turn;
            }
        }

        if (field_23BE4_ == 0 && field_23BF0_ != 2) {
            goto finish_turn;
        }
        if (field_23BE8_ == 0 || ExitTurnLoop != 0 ||
            end_of_game(0) != 0 ||
            (IsMultiplayerNet != 0 && (GameDataStateStorage.fields_.game_state_ & 0x800u) != 0)) {
            goto finish_turn;
        }

        flag10 = 1;
        if (field_23BF0_ == 2) {
            field_23BF0_ = 0;
            field_23BE4_ = Vehs[selected_vehicle_].order == 0x18;
        }

        // Native reads 0x0093E8E0 here: NetDaemon + 0x1B50, the already
        // typed lock_state_[0].dword_[9]. Its wider semantic role is unknown.
        if (NetDaemonNet.lock_state_[0].trailing_words_[4] != 0) {
            if (ConsoleGlobal.cursor_active_ == 0) {
                goto wait_for_vehicle;
            }
            ambience(0x1E);
            cursor_active_ = 0;
            field_23BF8_ = 1;
            field_23BFC_ = 1;
            if (survey_mode_ != 0) {
                survey_mode_ = 0;
                MapWinTable[0]->draw_map(1);
            }
            ::draw_cursor();
            ConsoleInfoWin.change(0);
            ConsoleStatusWin.redraw();
            MapWinTable[0]->main_caption();
            alpha_menu_.check(selected_vehicle_);
            continue;
        }
        goto update_vehicle;

    wait_for_vehicle:
        result = 1;
        wait_loop();
        continue;

    update_vehicle:
        if (field_23D24_ != 0) {
            const Veh &vehicle = Vehs[selected_vehicle_];
            const bool skip_auto_order_check =
                (vehicle.state & 0x200u) != 0 ||
                (vehicle.state & 0x01000200u) == 0x01000200u ||
                (vehicle.state & 0x4000u) != 0;
            if (!skip_auto_order_check && vehicle.order == 0) {
                field_23D24_ = 0;
                goto selected_vehicle_unavailable;
            }
        }

        if (ConsoleGlobal.cursor_active_ == 0) {
            goto wait_and_tour;
        }
        if (veh_ready(selected_vehicle_) == 0) {
            goto check_auto_order;
        }
        if (TutWinBaseWindow.is_visible() != 0 ||
            ReportWinGlobal.is_visible() != 0) {
            goto wait_and_tour;
        }

        {
            const Veh &vehicle = Vehs[selected_vehicle_];
            if ((vehicle.state & 0x4200u) != 0) {
                goto acquire_auto_lock;
            }
            const uint8_t order = vehicle.order;
            if (order == 0) {
                n = 0;
            } else {
                ++n;
                if (n <= (order == 0x18 ? 100 : 20)) {
                    goto issue_auto_action;
                }
                if (order == 0x18) {
                    Vehs[selected_vehicle_].order = 0;
                } else {
                    veh_skip(selected_vehicle_);
                }
                if (IsMultiplayerNet != 0) {
                    synch_veh(selected_vehicle_);
                    NetDaemonNet.await_synch();
                    NetDaemonNet.unlock_veh();
                }
                continue;
            }

        issue_auto_action:
            if (NetDaemonNet.action(selected_vehicle_, 0) != 0) {
                continue;
            }
            Vehs[selected_vehicle_].state |= 0x04000000u;
            goto wait_and_tour;
        }

    acquire_auto_lock:
        // Native target 0x00531150 is NetDaemon::automated_lock(int *, int);
        // its product declaration/body remains an outside-owner dependency.
        if (NetDaemonNet.automated_lock(&selected_vehicle_, 4) != 0) {
            goto finish_turn;
        }
        ++NetBusyCount;
        ++n;
        if (n <= 20) {
            goto check_enemy_vehicle;
        }
        veh_skip(selected_vehicle_);
        if (IsMultiplayerNet != 0) {
            synch_veh(selected_vehicle_);
            NetDaemonNet.await_synch();
            NetDaemonNet.unlock_veh();
        }
        --NetBusyCount;
        continue;

    check_enemy_vehicle:
        if (enemy_veh(selected_vehicle_) != 0) {
            if (IsMultiplayerNet != 0) {
                NetDaemonNet.unlock_veh();
                selected_vehicle_ = -1;
            }
            --NetBusyCount;
            goto finish_turn;
        }
        --NetBusyCount;
        continue;

    check_auto_order:
        if (Vehs[selected_vehicle_].order != 0 ||
            (Vehs[selected_vehicle_].state & 0x4200u) != 0) {
            goto selected_vehicle_unavailable;
        }

        goto wait_and_tour;

    selected_vehicle_unavailable:
        selected_vehicle_ = -1;
        goto finish_turn;

    wait_and_tour:
        result = 1;
        wait_loop();
        if (TutWinTourPending != 0) {
            TutWinTourPending = 0;
            TutorialWindow.tour();
        }
        continue;
    }

finish_turn:
    field_23BE4_ = 0;
    if (flag10 == 0) {
        selected_vehicle_ = -1;
    }
    return result;
}

static int give_hints_wrap_x(int x) {
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) x += MapLongitudeBounds;
        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
    }
    return x;
}
static bool give_hints_in_map(int x, int y) {
    return x >= 0 && x < static_cast<int>(MapLongitudeBounds) &&
           y >= 0 && y < static_cast<int>(MapLatitudeBounds);
}
static void give_hints_parse_faction_identity(int faction) {
    GenderDefault = Players[faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[faction].title_leader, -1, -1);
    GenderDefault = Players[faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(2, Players[faction].name_leader, -1, -1);
    GenderDefault = Players[faction].noun_gender;
    PluralityDefault = Players[faction].is_noun_plural;
    parse_says(3, Players[faction].noun_faction, -1, -1);
}
static void give_hints_map(const char *text, int x, int y, int vehicle,
                           Sprite *sprite, int flag, int arrow_x, int arrow_y) {
    TutorialWindow.tut_map(text, x, y, vehicle, sprite, flag, arrow_x, arrow_y);
}

// ORIGINAL: 0x00511BB0 ?give_hints@Console@@QAEXXZ 0x00511BB0-0x00513B85
// RECOVERY_BINDING: v1 {"evidence": ["Pinned511BB0..513B85; retained git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00511bb0.cpp; original vehicle-turn caller513BD5", "Initial state2000 branch511C37 encloses early cards and colonist scan; clearstate enters51222C. Tutorial stage511E71 is TutorialWindow+53C0 via set_tour_stage, not separate DesignTutorState73396C. Colonist tail uses ConsoleGlobal faction and always returns; scan ownership uses faction snapshot across whose_territory.", "Native51318A..197 evaluates unowned or own-faction candidate sites, skips hostile-owned ones. Enemy-base scan exhaustion5136AE..B7 continues enemyvehicle/territory hints, never explorationchecker. Matchingpod513596 enters5135E6; valid six-sprite index uses captured triad and bounded coordinates. Historical intermediate source added a defensive bounds check under an earlier permission; faithful-milestone recovery supersedes that deviation and restores the unchecked original dispatch.", "Shared terra epilogue5134E1..F5 calls tut_map then GameState|400; farm writes before its call. All33 distinct original caption literals independently read bounded from pinned data. Actual Sprite sizeof2C and existing banks govern indices; no fixed executable data/function aliases.", "recovery/tasks/console-scenario-owner-wave.json; complete-owner expansion and corrected native-flow history retained; actual-source host controls required", "Pinned 0x5134xx..0x5135E6 predecessor walks the eight RadiusBase offsets, wraps longitude on round maps, rejects y outside [0,MapLatitudeBounds) and x outside [0,MapLongitudeBounds), requires goody_at, and retains the first candidate whose terrain triad matches the selected vehicle.", "Pinned 0x5135E6 calls tut_check(0x2000) before any Unity sprite-index arithmetic; rejection returns immediately. On acceptance, 0x5135FB..0x513623 computes (5*y+7*x) / 3 with signed idiv and forms the captured sea/land discriminator plus 2*remainder; 0x513628..0x513639 performs the indexed table-address calculation and draw call with no intervening range branch.", "Current source's matching-pod inputs are captured only after give_hints_in_map accepts bounded coordinates; selected_is_sea is the captured sea/land discriminator and the native modulo result selects one of the six existing g_TER1_UNITY_POD_SPRITES. Preserve callback/checker ordering and the unchecked native indexed dispatch; do not add an invalid-index runtime scenario."], "reason": "Recover the complete Console tutorial-hint owner using typed vehicle/design/rules/map/faction storage, real tutorial windows and source-owned sprite banks. Preserve original identity, seed and historical obligations; no matching or runtime acceptance is implied. Restore the original unchecked Unity-pod dispatch after the checker; earlier behavior-fix permission is superseded by the faithful milestone."}
// TRIED: literal per-instruction register simulation (eax/ebx/ecx/edx/esi/edi as locals) with goto-labeled control flow matching every branch target; not tried: reconstructing idiomatic per-block source (member/local variable reuse) that would let the compiler's CSE match the original's code size - rebuilt is 4555 bytes against 8149 original, same first divergence at the prologue (push vs sub), so the whole body likely needs restructuring, not a single-instruction fix.
void Console::give_hints() {
// 0x511BBE tests the selected vehicle's state before any other work.
    const int entry_vehicle = selected_vehicle_;
    if ((Vehs[entry_vehicle].state & 0x4200u) != 0 || IsMultiplayerNet != 0 ||
        TutWinBaseWindow.is_visible() != 0 ||
        TutWinSocWindow.is_visible() != 0) {
        return;
    }

    // The image reloads the selected vehicle after the visibility calls, then
    // keeps these signed coordinates through the early cards and site scan.
    const int cursor_x = Vehs[selected_vehicle_].x;
    const int cursor_y = Vehs[selected_vehicle_].y;
    int proto_id;
    if ((Vehs[selected_vehicle_].state & 0x2000u) != 0) {
        proto_id = Vehs[selected_vehicle_].proto_id;
        if (VehPrototypes[proto_id].plan <= PLAN_RECONNAISANCE &&
            Vehs[selected_vehicle_].home_base_id >= 0 &&
            tut_check(8) != 0 && GameDataStateStorage.fields_.turn_current_num_ < 0x19) {
            StringTemp[0] = '\0';
            say_base(StringTemp, Vehs[selected_vehicle_].home_base_id);
            parse_says(0, StringTemp, -1, -1);
            proto_id = Vehs[selected_vehicle_].proto_id;
            parse_says(1, VehPrototypes[proto_id].veh_name, -1, -1);
            TutorialWindow.tut_map("NEWUNIT", cursor_x, cursor_y,
                                   selected_vehicle_, 0, 0, -1, -1);
            return;
        }

        // LAB_00511D12 is reached both when the first card is ineligible and
        // when tut_check(8)/the turn gate rejects it. The checker side effect
        // therefore remains even when the turn is already 25 or later.
        proto_id = Vehs[selected_vehicle_].proto_id;
        if (VehPrototypes[proto_id].plan <= PLAN_RECONNAISANCE &&
            tut_check(0x10) != 0 && GameDataStateStorage.fields_.turn_current_num_ >= 0x28) {
            TutorialWindow.tut_map("NEWUNITS", cursor_x, cursor_y,
                                   selected_vehicle_, 0, 0, -1, -1);
            return;
        }

        // LAB_00511D74, including the native active-count == 1 condition and
        // its sea-triad exclusion (triad value 1). The message and caption
        // inputs are rebuilt from the then-current selection after the checker.
        proto_id = Vehs[selected_vehicle_].proto_id;
        if (VehPrototypes[proto_id].plan == PLAN_TERRAFORMING &&
            PlayersData[faction_id_].proto_id_active[proto_id] == 1 &&
            tut_check(0x10000) != 0) {
            proto_id = Vehs[selected_vehicle_].proto_id;
            const int chassis_id = VehPrototypes[proto_id].chassis_id;
            if (Chassis[chassis_id].triad != TRIAD_SEA) {
                StringTemp[0] = '\0';
                say_base(StringTemp, Vehs[selected_vehicle_].home_base_id);
                parse_says(0, StringTemp, -1, -1);
                proto_id = Vehs[selected_vehicle_].proto_id;
                parse_says(1, VehPrototypes[proto_id].veh_name, -1, -1);
                TutorialWindow.set_tour_stage(2);
                TutorialWindow.tut_map("NEWTERRAFORMER", cursor_x, cursor_y,
                                       selected_vehicle_, 0, 0x100,
                                       -1, -1);
                GameDataStateStorage.fields_.game_state_ |= 0x400;
                return;
            }
        }
    // LAB_00511E99: the one-time colonization map scan is restricted to the
    // native plan-8/land/ability/flag combination. The source's 240 iterations
    // index the existing 289-entry RadiusOffset tables beginning at element 49.
    proto_id = Vehs[selected_vehicle_].proto_id;
    const int chassis_id = VehPrototypes[proto_id].chassis_id;
    if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION &&
        Chassis[chassis_id].triad == TRIAD_LAND &&
        (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x1000000u) == 0) {
        int best_site = 0;
        int best_x = -1;
        int best_y = -1;
        ConsoleGlobal.focus(cursor_x, cursor_y, ConsoleGlobal.faction_id_);
        for (int radius = 0; radius < 240; ++radius) {
            int candidate_x = cursor_x + RadiusOffsetX[49 + radius];
            if ((MapIsFlat & 1) == 0) {
                if (candidate_x < 0) {
                    candidate_x += MapLongitudeBounds;
                } else if (candidate_x >= MapLongitudeBounds) {
                    candidate_x -= MapLongitudeBounds;
                }
            }
            const int candidate_y = cursor_y + RadiusOffsetY[49 + radius];
            if (candidate_y < 0 || candidate_y >= MapLatitudeBounds ||
                candidate_x < 0 || candidate_x >= MapLongitudeBounds) {
                continue;
            }

            // The native body assumes the primary map window exists; this is
            // deliberately not replaced by a new null-window recovery path.
            Map &candidate = *map_loc(candidate_x, candidate_y);
            if ((candidate.climate & 0xE0u) > 0x60u) {
                const int altitude = candidate.climate >> 5;
                if (candidate_y - altitude + 3 <
                    MapWinTable[0]->field_1DDA8_ + 2) {
                    continue;
                }
            }
            MapWin *map_window = MapWinTable[0];
            if (candidate_y < map_window->field_1DDA8_ + 2 ||
                candidate_y >= map_window->field_1DDA8_ +
                    map_window->field_1DDD0_ + map_window->field_1DDD8_ - 2) {
                continue;
            }

            int viewport_x = candidate_x;
            if ((MapIsFlat & 1) == 0) {
                map_window = MapWinTable[0];
                if (viewport_x < map_window->field_1DDA4_ + 2) {
                    viewport_x += MapLongitudeBounds;
                }
                if (viewport_x >= map_window->field_1DDA4_ +
                        map_window->field_1DDCC_ + map_window->field_1DDD4_ - 2) {
                    viewport_x -= MapLongitudeBounds;
                }
            }
            map_window = MapWinTable[0];
            if (viewport_x < map_window->field_1DDA4_ + 2 ||
                viewport_x >= map_window->field_1DDA4_ +
                    map_window->field_1DDCC_ + map_window->field_1DDD4_ - 2) {
                continue;
            }
            // Native reloads both region bytes each iteration, after the
            // visibility/viewport checks; don't cache the cursor tile field.
            if (candidate.region != map_loc(cursor_x, cursor_y)->region) {
                continue;
            }
            const int candidate_faction = faction_id_;
            if (whose_territory(candidate_faction, candidate_x, candidate_y,
                                0, 0) != candidate_faction) {
                continue;
            }
            const uint32_t faction_bit = 1u << (candidate_faction & 31);
            if ((PlayersData[candidate_faction].flags & 0x200u) == 0 &&
                (candidate.visibility & faction_bit) == 0) {
                continue;
            }
            if ((candidate.bit & 0x20u) != 0 &&
                (candidate.climate & 0xE0u) >= 0x40u) {
                continue;
            }
            if ((candidate.val3 & 0xC0u) > 0x40u) {
                continue;
            }

            int site;
            if ((candidate.bit & 0x1000u) != 0) {
                site = 0;
            } else if ((candidate.val2 >> 4) != 0) {
                site = candidate.val2 >> 4;
            } else {
                site = world_site(candidate_x, candidate_y, 0);
                site_set(candidate_x, candidate_y, site);
            }
            if (site >= best_site) {
                best_site = site;
                best_x = candidate_x;
                best_y = candidate_y;
            }
        }

        StringTemp[0] = '\0';
        say_base(StringTemp, Vehs[selected_vehicle_].home_base_id);
        parse_says(0, StringTemp, -1, -1);
        if ((Players[ConsoleGlobal.faction_id_].rule_flags & 0x100u) == 0) {
            TutorialWindow.tut_map("NEWCOLONIST", cursor_x, cursor_y,
                                   selected_vehicle_, 0, 0x1000000,
                                   best_x, best_y);
        }
        return;
    }
    }


// 0x51222C first checks the tutorial window's persistent stop flag, then
// snapshots whether the selected prototype is a sea-triad unit into local_14.
if (TutorialWindow.tutorial_blocked()) {
    return;
}
proto_id = Vehs[selected_vehicle_].proto_id;
bool selected_is_sea =
    Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA;

if (Vehs[selected_vehicle_].dmg_incurred != 0 && tut_check(0x200000) != 0) {
    TutorialWindow.tut_map("DAMAGED", cursor_x, cursor_y, selected_vehicle_,
                           0, 0, -1, -1);
    return;
}

// LAB_005122A9: these checks apply only to land chassis. The terrain altitude
// test is the cursor tile's `(climate & 0xE0) >= 0x60` condition.
proto_id = Vehs[selected_vehicle_].proto_id;
if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_LAND &&
    (map_loc(cursor_x, cursor_y)->climate & 0xE0u) >= 0x60u) {
    for (int offset = 0; offset < 0x20; offset += 4) {
            int x = cursor_x + VehMoveDeltaX[offset / 4];
            if ((MapIsFlat & 1) == 0) {
                if (x < 0) {
                    x += MapLongitudeBounds;
                } else if (x >= MapLongitudeBounds) {
                    x -= MapLongitudeBounds;
                }
            }
            const int y = cursor_y + VehMoveDeltaY[offset / 4];
            if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
                x >= MapLongitudeBounds) {
                continue;
            }
            const int nearby = veh_at(x, y);
            if (nearby < 0 || Vehs[nearby].faction_id != 0 ||
                VehPrototypes[Vehs[nearby].proto_id].plan != PLAN_COLONIZATION) {
                continue;
            }
            if (tut_check(0x4000000) != 0) {
                TutorialWindow.tut_map("MINDWORMS", x, y, nearby, 0,
                                       0, -1, -1);
                return;
            }
    }

// LAB_005124E9 fast-unit card: the current chassis must be land triad, speed
// exactly 2, the weapon must have nonzero offense, and the tutorial bit must
// be newly accepted. Name/stat parsing uses fresh selection reads after calls.
proto_id = Vehs[selected_vehicle_].proto_id;
if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_LAND &&
    Chassis[VehPrototypes[proto_id].chassis_id].speed == 2 &&
    Weapon[VehPrototypes[proto_id].weapon_id].offense_rating != 0 &&
    tut_check(0x2000000) != 0) {
    StringTemp[0] = '\0';
    say_stats_3(StringTemp, Vehs[selected_vehicle_].proto_id);
    parse_says(1, StringTemp, -1, -1);
    proto_id = Vehs[selected_vehicle_].proto_id;
    parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
    TutorialWindow.tut_map("FASTUNIT", cursor_x, cursor_y, selected_vehicle_,
                           0, 0, -1, -1);
    return;
}

// LAB_00512503 artillery card. The signed int8 offense comparison deliberately
// remains > 1; the native JLE treats a byte such as psi's -1 as signed here.
proto_id = Vehs[selected_vehicle_].proto_id;
if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_LAND &&
    Chassis[VehPrototypes[proto_id].chassis_id].speed == 1 &&
    Weapon[VehPrototypes[proto_id].weapon_id].offense_rating > 1 &&
    can_arty(proto_id, 1) == 0 && tut_check(0x40000000) != 0) {
    StringTemp[0] = '\0';
    say_stats_3(StringTemp, Vehs[selected_vehicle_].proto_id);
    parse_says(1, StringTemp, -1, -1);
    proto_id = Vehs[selected_vehicle_].proto_id;
    parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
    TutorialWindow.tut_map("ARTYUNIT", cursor_x, cursor_y, selected_vehicle_,
                           0, 0, -1, -1);
    return;
}

// LAB_00512602 big-artillery card is the complementary can_arty==true case.
proto_id = Vehs[selected_vehicle_].proto_id;
if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_LAND &&
    can_arty(proto_id, 1) != 0 && tut_check(0x80000000u) != 0) {
    proto_id = Vehs[selected_vehicle_].proto_id;
    parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
    TutorialWindow.tut_map("BIGUNIT", cursor_x, cursor_y, selected_vehicle_,
                           0, 0, -1, -1);
    return;
}

// LAB_005126BD/LAB_00512739: the native shown flags are distinct from
// TutWinFlags9A64BC. Already-shown bits are ORed again, then flow continues.
proto_id = Vehs[selected_vehicle_].proto_id;
if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA) {
    if ((GameDataStateStorage.fields_.tutorial_flags_ & 0x20) == 0) {
        TutorialWindow.tut_map("NEWSHIP", cursor_x, cursor_y, selected_vehicle_,
                               0, 0, -1, -1);
        GameDataStateStorage.fields_.tutorial_flags_ |= 0x20;
        return;
    }
    GameDataStateStorage.fields_.tutorial_flags_ |= 0x20;
}
// Native reloads the selection/prototype/chassis after the sea-card path;
// keep this independent so callback-visible changes affect the air branch.
proto_id = Vehs[selected_vehicle_].proto_id;
if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
    if ((GameDataStateStorage.fields_.tutorial_flags_ & 0x40) == 0) {
        TutorialWindow.tut_map("NEWAIR", cursor_x, cursor_y, selected_vehicle_,
                               0, 0, -1, -1);
        GameDataStateStorage.fields_.tutorial_flags_ |= 0x40;
        return;
    }
    GameDataStateStorage.fields_.tutorial_flags_ |= 0x40;
}

// LAB_005127B5 base-defense card. The population/force/prototype thresholds
// select the shared defense path; the weak-base suffix is checked only after
// stats/name parsing, with a fresh current prototype.
const int cursor_base = base_at(cursor_x, cursor_y);
if (cursor_base >= 0 &&
    Bases[cursor_base].faction_id_current == faction_id_) {
    const int current_proto = Vehs[selected_vehicle_].proto_id;
    const Base &base = Bases[cursor_base];
    if (base.population_size > 2 ||
        PlayersData[faction_id_].total_mil_veh > 3 ||
        PlayersData[faction_id_].proto_id_active[current_proto] > 1) {
        if (enemy_move(~selected_vehicle_) != 0 &&
            (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x800000u) == 0) {
            StringTemp[0] = '\0';
            say_stats_3(StringTemp, Vehs[selected_vehicle_].proto_id);
            parse_says(2, StringTemp, -1, -1);
            parse_says(0, base.name_string, -1, -1);
            proto_id = Vehs[selected_vehicle_].proto_id;
            parse_says(1, VehPrototypes[proto_id].veh_name, -1, -1);

            proto_id = Vehs[selected_vehicle_].proto_id;
            const VehPrototype &prototype = VehPrototypes[proto_id];
            const bool weak_defensive_design =
                prototype.plan == PLAN_DEFENSIVE &&
                Weapon[prototype.weapon_id].offense_rating == 1 &&
                Armor[prototype.armor_id].defense_rating > 1;
            TutorialWindow.tut_map(weak_defensive_design ? "DEFENDIT2" : "DEFENDIT",
                                   cursor_x, cursor_y, selected_vehicle_, 0,
                                   0x800000, -1, -1);
            return;
        }
    }
}

// LAB_00512959 empty-base scan. The image checks the current weapon before
// iterating, visits BaseCurrentCount with 0x134-byte records, and calls
// tut_check2 before the independent every-20-turn fallback.
proto_id = Vehs[selected_vehicle_].proto_id;
if (GameDataStateStorage.fields_.turn_current_num_ >= 0x14 &&
    (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x4000000u) == 0 &&
    Weapon[VehPrototypes[proto_id].weapon_id].offense_rating != 0) {
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        const Base &base = Bases[base_id];
        if (base.faction_id_current != faction_id_ ||
            veh_at(base.x, base.y) >= 0 || base.population_size == 1) {
            continue;
        }

        MapWin *map_window = MapWinTable[0];
        if (base.y >= 0 && base.y < MapLatitudeBounds && base.x >= 0 &&
            base.x < MapLongitudeBounds) {
            const uint8_t climate = map_loc(base.x, base.y)->climate;
            if ((climate & 0xE0u) > 0x60u) {
                const int altitude = climate >> 5;
                if (base.y - altitude + 3 < map_window->field_1DDA8_ + 2) {
                    continue;
                }
            }
        }
        if (base.y < map_window->field_1DDA8_ + 2 ||
            base.y >= map_window->field_1DDA8_ + map_window->field_1DDD0_ +
                map_window->field_1DDD8_ - 2) {
            continue;
        }

        int viewport_x = base.x;
        if ((MapIsFlat & 1) == 0) {
            if (viewport_x < map_window->field_1DDA4_ + 2) {
                viewport_x += MapLongitudeBounds;
            }
            if (viewport_x >= map_window->field_1DDA4_ +
                    map_window->field_1DDCC_ + map_window->field_1DDD4_ - 2) {
                viewport_x -= MapLongitudeBounds;
            }
        }
        if (viewport_x < map_window->field_1DDA4_ + 2 ||
            viewport_x >= map_window->field_1DDA4_ +
                map_window->field_1DDCC_ + map_window->field_1DDD4_ - 2) {
            continue;
        }
        if (tut_check2(0x100) != 0 || GameDataStateStorage.fields_.turn_current_num_ % 20 == 0) {
            parse_says(0, base.name_string, -1, -1);
            TutorialWindow.tut_map("EMPTYBASE", base.x, base.y, -1, 0,
                                   0x4000000, -1, -1);
            return;
        }
    }
}
}

// LAB_00512B90 cursor-tile owner filter. The native signed JGE after masking
// a nibble is equivalent here because the value is in [0,15].
const Map &cursor_tile = *map_loc(cursor_x, cursor_y);
if ((cursor_tile.bit & 1u) != 0 && (cursor_tile.val2 & 0x0Fu) < 8) {
    return;
}

// 0x512BCE artifact card; tut_check's bit is recorded before map display.
proto_id = Vehs[selected_vehicle_].proto_id;
if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT &&
    tut_check(0x1000000) != 0) {
    TutorialWindow.tut_map("ARTIFACT", cursor_x, cursor_y, selected_vehicle_,
                           0, 0, -1, -1);
    return;
}

// The native calls tut_check2 before examining the prototype ID, so its flag
// side effect remains when the current design is not one of the three Ogre IDs.
if (tut_check2(0x20000) != 0) {
    proto_id = Vehs[selected_vehicle_].proto_id;
    if (proto_id == 0x10 || proto_id == 0x11 || proto_id == 0x12) {
        TutorialWindow.tut_map("GETOGRE", cursor_x, cursor_y, selected_vehicle_,
                               0, 0, -1, -1);
        return;
    }
}

// LAB_00512C82 nearby native-unit hints. Each candidate uses RadiusBase's
// first eight entries; a successful checker draws and returns, otherwise the
// loop continues and falls through to 0x512D4E.
for (int offset = 0; offset < 0x20; offset += 4) {
    int x = cursor_x + VehMoveDeltaX[offset / 4];
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) {
            x += MapLongitudeBounds;
        } else if (x >= MapLongitudeBounds) {
            x -= MapLongitudeBounds;
        }
    }
    const int y = cursor_y + VehMoveDeltaY[offset / 4];
    if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
        x >= MapLongitudeBounds) {
        continue;
    }
    const int nearby = veh_at(x, y);
    if (nearby < 0 || Vehs[nearby].faction_id != 0) {
        continue;
    }
    const int nearby_proto = Vehs[nearby].proto_id;
    if (nearby_proto == 0x0F) {
        if (tut_check2(0x80000) != 0) {
            TutorialWindow.tut_map("SPORELAUNCHER", x, y, nearby, 0,
                                   0, -1, -1);
            return;
        }
    } else if (nearby_proto == 0x0E && tut_check2(0x40000) != 0) {
        TutorialWindow.tut_map("SEALURK", x, y, nearby, 0, 0, -1, -1);
        return;
    }
}


{
// 0x512D4E resource scan: RadiusBase order, coordinate wrapping, tile
    // exclusions, goody suppression, bonus lookup, checker, second bonus_at
    // read, then sprite selection. Tut-map returns immediately in each case.
    for (int radius = 0; radius < 8; ++radius) {
        const int x = give_hints_wrap_x(cursor_x + VehMoveDeltaX[radius]);
        const int y = cursor_y + VehMoveDeltaY[radius];
        if (!give_hints_in_map(x, y)) continue;
        Map &tile = *map_loc(x, y);
        if ((tile.bit & 3u) != 0 && (tile.val2 & 0x0Fu) < 8) continue;
        if ((tile.bit & 0x81E02850u) != 0) continue;
        if (goody_at(x, y) != 0) continue;
        int bonus = bonus_at(x, y, 0);
        if (bonus == 1 && tut_check(0x20000)) {
            bonus = bonus_at(x, y, 0);
            const int sprite_index = 2 * static_cast<int>(selected_is_sea) +
                                     4 * (bonus - 1);
            give_hints_map("BONUSNUTRIENT", x, y, -1,
                 &g_TER1_SEA_LAND_RESOURCE_SPRITES[sprite_index], 0, -1, -1);
            return;
        }
        if (bonus == 2 && tut_check(0x40000)) {
            bonus = bonus_at(x, y, 0);
            const int sprite_index = 2 * static_cast<int>(selected_is_sea) +
                                     4 * (bonus - 1);
            give_hints_map("BONUSMINERALS", x, y, -1,
                 &g_TER1_SEA_LAND_RESOURCE_SPRITES[sprite_index], 0, -1, -1);
            return;
        }
        if (bonus == 3 && tut_check(0x80000)) {
            bonus = bonus_at(x, y, 0);
            const int sprite_index = 2 * static_cast<int>(selected_is_sea) +
                                     4 * (bonus - 1);
            give_hints_map("BONUSENERGY", x, y, -1,
                 &g_TER1_SEA_LAND_RESOURCE_SPRITES[sprite_index], 0, -1, -1);
            return;
        }
    }

    // The selected cursor tile's fungus-water/land cards follow the neighbor
    // scan. Native tests bit 0x20, then the altitude band, with distinct masks.
    Map &cursor_tile = *map_loc(cursor_x, cursor_y);
    if ((cursor_tile.bit & 0x20u) != 0) {
        const uint8_t altitude_band = cursor_tile.climate & 0xE0u;
        if (altitude_band >= 0x40u && altitude_band < 0x60u &&
            tut_check(0x8000)) {
            give_hints_map("FUNGUSSEA", cursor_x, cursor_y, -1, 0, 0, -1, -1);
            return;
        }
        if (altitude_band >= 0x60u && tut_check(0x100)) {
            give_hints_map("FUNGUSAMUNGUS", cursor_x, cursor_y, -1, 0, 0, -1, -1);
            return;
        }
    }

    // 0x513022: tutorial-state gate; colonization-site advice. The current
    // tile's site is initialized/read before the neighbor scan, and each
    // candidate updates its site through the real world_site/site_set owners.
    if ((GameDataStateStorage.fields_.game_state_ & 0x400u) != 0) return;
    proto_id = Vehs[selected_vehicle_].proto_id;
    if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION &&
        (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x10000u) == 0) {
        Map &here = *map_loc(cursor_x, cursor_y);
        int site = (here.bit & 0x1000u) ? 0 : (here.val2 >> 4);
        if (site == 0) {
            site = world_site(cursor_x, cursor_y, 0);
            site_set(cursor_x, cursor_y, site);
        }
        if (site < 4) {
            const int region = map_loc(cursor_x, cursor_y)->region;
            if (site < PlayersData[faction_id_].unk_82[region]) return;
        }
        bool better_site_in_unowned_region = false;
        for (int i = 0; i < 21; ++i) {
            const int x = give_hints_wrap_x(cursor_x + RadiusOffsetX[i]);
            const int y = cursor_y + RadiusOffsetY[i];
            if (!give_hints_in_map(x, y)) continue;
            const int owner = whose_territory(faction_id_, x, y, 0, 0);
            if (owner >= 0 && owner != faction_id_) continue;
            Map &candidate = *map_loc(x, y);
            int candidate_site;
            if ((candidate.bit & 0x1000u) != 0) {
                candidate_site = 0;
            } else {
                candidate_site = candidate.val2 >> 4;
                if (candidate_site == 0) {
                    candidate_site = world_site(x, y, 0);
                    site_set(x, y, candidate_site);
                }
            }
            if (candidate_site > site) {
                better_site_in_unowned_region = true;
                break;
            }
        }
        if (better_site_in_unowned_region) return;
        give_hints_map("BUILDHERE", cursor_x, cursor_y, selected_vehicle_, 0,
             0x10000, -1, -1);
        return;
    }

    // 0x51323A terraforming chain, only for plan 9 when the entry-captured
    // local_14 is false. BaseFindDist and BaseCurrent are written/read by the
    // actual base_find owner, not synthesized from a radius test.
    proto_id = Vehs[selected_vehicle_].proto_id;
    if (VehPrototypes[proto_id].plan == PLAN_TERRAFORMING && !selected_is_sea) {
        const int base_id = base_find(cursor_x, cursor_y, faction_id_);
        if (base_id < 0) return;
        if (BaseFindDist <= 2) {
            if (at_goal(faction_id_, 0x29, cursor_x, cursor_y) != 0 &&
                (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x200000u) == 0) {
                parse_says(0, BaseCurrent->name_string, -1, -1);
                if ((map_loc(cursor_x, cursor_y)->bit & 0x40u) != 0) {
                    give_hints_map("TERRARAISE", cursor_x, cursor_y, -1, 0,
                         0x200000, -1, -1);
                }
                GameDataStateStorage.fields_.game_state_ |= 0x400u;
                return;
            }

            const int improve = can_terraform(faction_id_, cursor_x, cursor_y,
                                              0, base_id);
            if (improve == 4 &&
                (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x100000u) == 0 &&
                (map_loc(cursor_x, cursor_y)->bit & 0x800000u) == 0) {
                GameDataStateStorage.fields_.game_state_ |= 0x400u;
                parse_says(0, Bases[base_id].name_string, -1, -1);
                give_hints_map("TERRAFARM", cursor_x, cursor_y, -1,
                     &g_TER1_FARM_SPRITES[3], 0x100000, -1, -1);
                return;
            }
            if (improve == 6 &&
                (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x40000u) == 0) {
                give_hints_map("TERRAMINE", cursor_x, cursor_y, -1,
                     &g_TER1_MINE_SPRITE, 0x40000, -1, -1);
                GameDataStateStorage.fields_.game_state_ |= 0x400u;
                return;
            }
            if (improve == 7 &&
                (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x20000u) == 0) {
                give_hints_map("TERRASOLAR", cursor_x, cursor_y, -1,
                     &g_TER1_SOLAR_COLLECTOR_SPRITE, 0x20000, -1, -1);
                GameDataStateStorage.fields_.game_state_ |= 0x400u;
                return;
            }
            if (improve == 8 &&
                (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x400000u) == 0) {
                give_hints_map("TERRAFOREST", cursor_x, cursor_y, -1, 0,
                     0x400000, -1, -1);
                GameDataStateStorage.fields_.game_state_ |= 0x400u;
                return;
            }

            if ((at_goal(faction_id_, 0x79, cursor_x, cursor_y) != 0 ||
                 good_sensor(ConsoleGlobal.faction_id_, cursor_x, cursor_y) != 0) &&
                (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x2000000u) == 0) {
                give_hints_map("TERRASENSOR", cursor_x, cursor_y, -1, 0,
                     0x2000000, -1, -1);
                GameDataStateStorage.fields_.game_state_ |= 0x400u;
                return;
            }
            const uint32_t tile_flags = map_loc(cursor_x, cursor_y)->bit;
            if ((tile_flags & 4u) == 0 &&
                (GameDataStateStorage.fields_.tutwin_flags_9a64bc_ & 0x80000u) == 0 &&
                ((tile_flags & 0x10u) != 0 ||
                 at_goal(faction_id_, 9, cursor_x, cursor_y) != 0)) {
                give_hints_map("TERRAROAD", cursor_x, cursor_y, -1, 0,
                     0x80000, -1, -1);
                GameDataStateStorage.fields_.game_state_ |= 0x400u;
                return;
            }
        }
    }

    // The exploration/goody scan checks the 8-tile RadiusBase sequence for a
    // pod on matching sea/land terrain. A matching pod branches directly to
    // the shared 0x5135E6 card tail.
    bool matching_pod = false;
    int pod_x = cursor_x;
    int pod_y = cursor_y;
    if ((GameDataStateStorage.fields_.tutorial_flags_ & 0x2000) == 0) {
        for (int i = 0; i < 8; ++i) {
            const int x = give_hints_wrap_x(cursor_x + VehMoveDeltaX[i]);
            const int y = cursor_y + VehMoveDeltaY[i];
            if (!give_hints_in_map(x, y) || goody_at(x, y) == 0) continue;
            const bool shallow_or_land = (map_loc(x, y)->climate & 0xE0u) < 0x60u;
            if (shallow_or_land == selected_is_sea) {
                matching_pod = true;
                pod_x = x;
                pod_y = y;
                break;
            }
        }
    }

    // A matching goody is the only route to LAB_005135E6 / EXPLORATION.
    // Preserve the native checker-before-index/draw order and unchecked
    // computed sprite-table access.
    if (matching_pod) {
        if (!tut_check(0x2000)) return;
        const int remainder = (5 * pod_y + 7 * pod_x) % 3;
        const int sprite_index = static_cast<int>(selected_is_sea) + remainder * 2;
        give_hints_map("EXPLORATION", pod_x, pod_y, -1,
             &g_TER1_UNITY_POD_SPRITES[sprite_index], 0, -1, -1);
        return;
    }

    // 0x5135AD enemy-base scan. Its exhaustion proceeds to 0x5136B7, not to
    // the exploration checker. A previously set base flag bypasses this scan
    // but still proceeds to the same enemy-vehicle scan.
    if ((GameDataStateStorage.fields_.tutorial_flags_ & 0x8000000) == 0) {
        for (int i = 0; i < 8; ++i) {
            const int x = give_hints_wrap_x(cursor_x + VehMoveDeltaX[i]);
            const int y = cursor_y + VehMoveDeltaY[i];
            if (!give_hints_in_map(x, y)) continue;
            const int base_id = base_at(x, y);
            if (base_id < 0) continue;
            const int base_faction = Bases[base_id].faction_id_current;
            if (base_faction == faction_id_) continue;
            if (!tut_check(0x8000000)) return;
            parse_says(0, Bases[base_id].name_string, -1, -1);
            give_hints_parse_faction_identity(base_faction);
            give_hints_map("ENEMYBASE", x, y, -1, 0, 0, -1, -1);
            return;
        }
    }

    // Native 0x5136B7 enemy-vehicle scan follows either an exhausted base
    // scan or the pre-set base-hint bit. Its own independent flag controls it.
    if ((GameDataStateStorage.fields_.tutorial_flags_ & 0x10000000) == 0) {
        for (int i = 0; i < 8; ++i) {
            const int x = give_hints_wrap_x(cursor_x + VehMoveDeltaX[i]);
            const int y = cursor_y + VehMoveDeltaY[i];
            if (!give_hints_in_map(x, y)) continue;
            Map &tile = *map_loc(x, y);
            if ((tile.bit & 1u) != 0 && (tile.val2 & 0x0Fu) < 8) continue;
            const int vehicle = veh_at(x, y);
            if (vehicle < 0) continue;
            const int other_faction = Vehs[vehicle].faction_id;
            if (other_faction == faction_id_ || other_faction == 0) continue;
            if (!tut_check(0x10000000)) return;
            give_hints_parse_faction_identity(other_faction);
            give_hints_map("ENEMYVEH", x, y, vehicle, 0, 0, -1, -1);
            return;
        }
    }

    // 0x5138B3 territory edge scan. It compares each candidate owner against
    // the cursor tile owner, then emits only the
    // first edge faction meeting the two comparisons.
    if ((GameDataStateStorage.fields_.tutorial_flags_ & 0x20000000) == 0) {
        for (int i = 0; i < 8; ++i) {
            const int x = give_hints_wrap_x(cursor_x + VehMoveDeltaX[i]);
            const int y = cursor_y + VehMoveDeltaY[i];
            if (!give_hints_in_map(x, y)) continue;
            const Map &tile = *map_loc(x, y);
            if ((tile.bit & 3u) != 0 && (tile.val2 & 0x0Fu) < 8) continue;
            const int candidate_owner = whose_territory(faction_id_, x, y,
                                                        0, 0);
            if (candidate_owner < 0 || candidate_owner == faction_id_) continue;
            const int again_cursor_owner = whose_territory(
                faction_id_, cursor_x, cursor_y, 0, 0);
            if (again_cursor_owner == candidate_owner) continue;
            if (!tut_check(0x20000000)) return;
            give_hints_parse_faction_identity(candidate_owner);
            give_hints_map("ENEMYTERR", x, y, -1, 0, 0, -1, -1);
            return;
        }
    }
}


}

// ORIGINAL: 0x004D7F60 ?use_ability@Console@@QAEXH@Z 0x004D7F60-0x004D8A62;0x0065AFD2-0x0065B275
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004d7f60.cpp;source:src/console.h;image:0x004D7F60-0x004D8A62;image:0x0065AFD2-0x0065B275", "Popup constructor before entity reads:0x004D7F85-0x004D7FC4;native inline destruction:0x004D89BE-0x004D8A4A", "Chassis.missile and captured mask:0x004D8043-0x004D8072;missile base/altitude/radius:0x004D8160-0x004D8271;radius source:src/map.h", "Treaty row/arguments and locks:0x004D82D4-0x004D85CC;masked popcount and full selector result:0x004D8653-0x004D875A;drop state at Veh+4 and late moves:0x004D875A-0x004D89BE"], "reason": "Recover the Console ability order handler with automatic Popup lifetime, source-owned vehicle/design/map rules, faction treaties and NetDaemon locking. Preserve the captured terraform-bit mask, native missile/base/altitude gates, typed radius tables, live locked vehicle plan and drop-state/movement checks. The selector branch retains the original masked-popcount guard; no bug fix widens its natural reachability. Retain the full seed and compiler cleanup spans; this source interpretation is not a measured matching or runtime claim."}
// TRIED: `Popup popup;` RAII (same class this catalog's introduce() at 0x0053D360 established: BasePop(0x3230)+Scroll scroll_, dialogs_ blob at +0x21D0) reproduces the prologue and the per-branch close()+return teardown the compiler generates automatically; matches 21 instructions. The deep branch after the bit_count()>=2 dialog picker (engineer-terraform vs. drop-range-cursor path) is a best-effort shape, not verified instruction-by-instruction against the disasm - next pass should re-derive LAB_004d875a/LAB_004d8969 precisely before trusting it beyond MISMATCH.
void Console::use_ability(int vehicle_id) {
    Popup popup;
    const int entry_x = Vehs[vehicle_id].x;
    const int entry_y = Vehs[vehicle_id].y;
    const int entry_faction = Vehs[vehicle_id].faction_id;

    // The pinned entry checks turn ownership before looking at ability or
    // tile state.  The message and timing are shared with the other Console
    // order handlers.
    if (not_my_turn() != 0) {
        NetMsgPrimary.pop("SIMULNO", 5000, 0, nullptr);
        return;
    }

    const int entry_proto_id = Vehs[vehicle_id].proto_id;
    const VehPrototype &entry_prototype = VehPrototypes[entry_proto_id];
    const uint32_t entry_ability_mask = entry_prototype.ability_flags & 0x10u;
    const bool has_terraform_ability = entry_ability_mask != 0;
    const bool missile_former =
        Chassis[entry_prototype.chassis_id].missile != 0 &&
        (entry_prototype.plan == PLAN_TECTONIC_MISSILE ||
         entry_prototype.plan == PLAN_FUNGAL_MISSILE);

    if (!has_terraform_ability && !missile_former) {
        NetMsgPrimary.pop("NOABILITY", 5000, 0, nullptr);
        return;
    }
    if (has_terraform_ability && !missile_former &&
        (Vehs[vehicle_id].moves_expended != 0 ||
         veh_ready(vehicle_id) == 0)) {
        NetMsgPrimary.pop("UNITMOVED", 5000, 0, nullptr);
        return;
    }

    // Former missile orders take a separate base/territory/treaty path before
    // the general ability selector.  Its remaining native effect dispatches
    // the missile order and network synchronization; do not route it through
    // the ordinary airdrop cursor behavior.
    if (missile_former) {
        const Map *const tile = map_loc(entry_x, entry_y);
        const int base_owner = tile->val2 & 0x0F;
        if ((tile->bit & BIT_BASE_IN_TILE) != 0 && base_owner < 8) {
            NetMsgPrimary.pop("MISSILEBASE", 5000, 0, nullptr);
            return;
        }
        if (entry_prototype.plan == PLAN_TECTONIC_MISSILE &&
            (tile->climate & 0xE0) < 0x60) {
            NetMsgPrimary.pop("TECTONICOCEAN", 5000, 0, nullptr);
            return;
        }
        // The original radius count uses the game's RadiusRange progression;
        // native 0x66F8B8/0x66F8C8 operands are interior positions in that
        // typed source table, not independent arrays.
        const bool fungal_missile =
            entry_prototype.plan == PLAN_FUNGAL_MISSILE;
        const int radius_index = (tile->climate >> 5) + 1;
        int radius_count = 9;
        if (!fungal_missile && radius_index >= 0) {
            radius_count = 1;
            if (radius_index > 3) {
                radius_count = radius_index > 6 ? RadiusRange[2]
                                                : RadiusRange[radius_index - 4];
            }
        }
        for (int offset = 0; offset < radius_count; ++offset) {
            int target_x = entry_x + RadiusOffsetX[offset];
            if ((MapIsFlat & 1) == 0) {
                if (target_x < 0) target_x += MapLongitudeBounds;
                else if (target_x >= MapLongitudeBounds) target_x -= MapLongitudeBounds;
            }
            const int target_y = entry_y + RadiusOffsetY[offset];
            if (target_y < 0 || target_y >= MapLatitudeBounds ||
                target_x < 0 || target_x >= MapLongitudeBounds) {
                continue;
            }
            const int target_faction = whose_territory(
                entry_faction, target_x, target_y, nullptr, 0);
            if (target_faction < 0 || target_faction == entry_faction ||
                (PlayersData[entry_faction].diplo_treaties[target_faction] & 7u) == 0) {
                continue;
            }

            if (NetDaemonNet.lock_veh(&vehicle_id, 2, -1, -1, 0) != 0) {
                return;
            }
            if (break_treaty(entry_faction, target_faction, 7) != 0) {
                NetDaemonNet.unlock_veh();
                return;
            }
            if (IsMultiplayerNet != 0) {
                message_data(0x2440, 0, entry_faction, target_faction, -1, 0);
                NetDaemonNet.await_exec(1);
            } else {
                double_cross(entry_faction, target_faction, -1);
            }
        }

        if (NetDaemonNet.lock_veh(&vehicle_id, 2, -1, -1, 0) != 0) {
            return;
        }
        // The lock may replace/compact the vehicle record. Native reloads the
        // prototype and plan after the lock but retains the entry coordinates.
        const int locked_proto_id = Vehs[vehicle_id].proto_id;
        const int locked_plan = VehPrototypes[locked_proto_id].plan;
        if (locked_plan == PLAN_TECTONIC_MISSILE) {
            if (IsMultiplayerNet != 0) {
                message_veh(0x2417, vehicle_id, entry_x, entry_y);
                NetDaemonNet.await_exec(1);
            } else {
                action_tectonic(vehicle_id, entry_x, entry_y);
            }
            return;
        }
        if (locked_plan == PLAN_FUNGAL_MISSILE) {
            if (IsMultiplayerNet != 0) {
                message_veh(0x2418, vehicle_id, entry_x, entry_y);
                NetDaemonNet.await_exec(1);
            } else {
                action_fungal(vehicle_id, entry_x, entry_y);
            }
            return;
        }
        return;
    }

    uint32_t selected_ability = entry_ability_mask;
    if (bit_count(static_cast<int>(selected_ability)) > 1) {
        popup.init(0x40, 0);
        for (int ability_id = 0; ability_id < MaxAbilityNum; ++ability_id) {
            const uint32_t ability_bit = 1u << ability_id;
            const int current_proto_id = Vehs[vehicle_id].proto_id;
            if (has_abil(current_proto_id, static_cast<int>(ability_bit)) == 0) {
                continue;
            }
            const int name_address = static_cast<int>(
                reinterpret_cast<intptr_t>(Ability[ability_id].name));
            popup.dialogs().item(StringTable.get(name_address),
                                 static_cast<int>(ability_bit));
        }
        const int result = popup.exec(0, nullptr);
        if (result < 0) {
            return;
        }
        selected_ability = static_cast<uint32_t>(result);
    }

    if ((selected_ability & 0x10u) != 0) {
        const Map *const tile = map_loc(entry_x, entry_y);
        const int base_owner = tile->val2 & 0x0F;
        if (((tile->bit & BIT_BASE_IN_TILE) == 0 || base_owner >= 8) &&
            (tile->bit & BIT_AIRBASE) == 0) {
            X_pop("DROPBASE", nullptr);
            return;
        }
        if ((Vehs[vehicle_id].state & 0x20u) != 0) {
            X_pop("DROPALREADY", nullptr);
            return;
        }
        if (Vehs[vehicle_id].moves_expended != 0) {
            X_pop("DROPMOVED", nullptr);
            return;
        }
    }

    if (selected_ability == 0x10u) {
        ConsoleGlobal.cursor_on(4, drop_range(entry_faction));
    }
}

// ORIGINAL: 0x004CF7C0 ?new_base@Console@@QAEXH@Z 0x004CF7C0-0x004D0442 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004cf7c0.cpp", "image:terranx_original.exe:0x004CF7C0"], "reason": "Recover settlement creation through the actual Console receiver, typed vehicle/map/base/faction owners and genuine diplomacy contact. Native invalid-tile guards precede base_at, and ready_search uses this while update_data uses ConsoleGlobal."}
// TRIED: structural transcription following the Ghidra shape (validation checks each calling NetMsg::pop and returning; territory-scan loop; diplomacy/build-base wizard), corrected against the raw asm's confirmed 0x34 stride for the 0x952828/0x9AB892 unit-position tables (Ghidra showed 0x1a, a known 2-byte-stride bug) and fixed `ready_search` to use the saved `this` rather than the fixed Console singleton (only `update_data` uses 0x9156B0). Diverges at instr #8.
void Console::new_base(int vehicle_id) { // 0x004CF7C0
    // The native entry checks the selected prototype and target tile before
    // opening any diplomacy or naming UI. Keep these guards in native order.
    const Veh &vehicle = Vehs[vehicle_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    if (prototype.plan != PLAN_COLONIZATION) {
        NetMsgPrimary.pop("NEEDCOLONISTS", 5000, 0, nullptr);
        return;
    }

    const int x = vehicle.x;
    const int y = vehicle.y;
    const int faction_id = vehicle.faction_id;
    const uint32_t altitude = climate_at(x, y) & 0xE0;
    const bool sea_chassis =
        Chassis[prototype.chassis_id].triad == TRIAD_SEA;
    if ((altitude < 0x60) != sea_chassis) {
        NetMsgPrimary.pop("NOTINTRIAD", 5000, 0, nullptr);
        return;
    }

    const Map *const tile = map_loc(x, y);
    if ((tile->bit & BIT_FUNGUS) != 0 && altitude >= 0x40) {
        NetMsgPrimary.pop("FUNGUSDANGUS", 5000, 0, nullptr);
        return;
    }
    if ((tile->bit & BIT_MONOLITH) != 0) {
        NetMsgPrimary.pop("NOTONMONOLITH", 5000, 0, nullptr);
        return;
    }
    const int existing_base = base_at(x, y);
    if ((tile->val3 & 0xC0) > 0x40 && altitude >= 0x60 &&
        existing_base < 0) {
        NetMsgPrimary.pop("NOTONROCKY", 5000, 0, nullptr);
        return;
    }
    if (not_my_turn()) {
        NetMsgPrimary.pop("SIMULNO", 5000, 0, nullptr);
        return;
    }
    if (!veh_ready(vehicle_id)) {
        NetMsgPrimary.pop("UNITMOVED", 5000, 0, nullptr);
        return;
    }

    if (existing_base < 0) {
        bool adjacent_base = false;
        for (int direction = 0; direction < 8; ++direction) {
            int adjacent_x = x + VehMoveDeltaX[direction];
            const int adjacent_y = y + VehMoveDeltaY[direction];
            if ((MapIsFlat & 1) == 0) {
                if (adjacent_x < 0) {
                    adjacent_x += MapLongitudeBounds;
                } else if (adjacent_x >= MapLongitudeBounds) {
                    adjacent_x -= MapLongitudeBounds;
                }
            }
            if (adjacent_y < 0 || adjacent_y >= MapLatitudeBounds ||
                adjacent_x < 0 || adjacent_x >= MapLongitudeBounds) {
                continue;
            }
            const Map *const adjacent = map_loc(adjacent_x, adjacent_y);
            if ((adjacent->bit & BIT_BASE_IN_TILE) != 0 &&
                (adjacent->val2 & 0x0F) < MaxPlayerNum) {
                adjacent_base = true;
                break;
            }
        }
        if (adjacent_base) {
            NetMsgPrimary.pop("BASETOONEAR", 5000, 0, nullptr);
            return;
        }
    }

    if (existing_base < 0) {
    const int other_faction =
        whose_territory(faction_id, x, y, nullptr, nullptr);
    uint32_t relation = other_faction >= 0
                            ? PlayersData[faction_id]
                                  .diplo_treaties[other_faction]
                            : 0;
    const int32_t base_count = static_cast<int32_t>(
        PlayersData[faction_id].current_num_bases);
    if (base_count != 0 &&
        other_faction >= 0 && other_faction != faction_id &&
        (relation & DTREATY_VENDETTA) == 0) {
        if (!IsMultiplayerNet) {
            DiplomacyEndState = 8;
            wants_to_speak(faction_id, other_faction);
        }
        GenderDefault = Players[faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[faction_id].title_leader, -1, -1);
        parse_says(1, get_pact_hood(faction_id, other_faction), -1, -1);
        GenderDefault = Players[other_faction].noun_gender;
        PluralityDefault = Players[other_faction].is_noun_plural;
        parse_says(2, Players[other_faction].noun_faction, -1, -1);
        parse_says(3, Players[other_faction].adj_name_faction, -1, -1);
        GenderDefault = Players[other_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(5, Players[other_faction].title_leader, -1, -1);
        GenderDefault = Players[other_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(6, Players[other_faction].name_leader, -1, -1);

        relation = other_faction >= 0
                       ? PlayersData[faction_id].diplo_treaties[other_faction]
                       : 0;
        if ((relation & DTREATY_PACT) != 0) {
            NetMsgPrimary.pop((climate_at(x, y) & 0xE0) >= 0x60
                                 ? "TERRPACT" : "TERRPACT2",
                             5000, 0, nullptr);
            return;
        }
        if ((relation & DTREATY_TREATY) == 0) {
            if (!IsMultiplayerNet) {
                parse_says(0, Players[other_faction].adj_name_faction, -1, -1);
                GenderDefault = Players[faction_id].is_leader_female;
                PluralityDefault = 0;
                parse_says(1, Players[faction_id].title_leader, -1, -1);
                parse_says(2, Players[faction_id].name_leader, -1, -1);
                parse_says(3, Players[faction_id].adj_faction, -1, -1);
                const char *const land_prompt =
                    (climate_at(x, y) & 0xE0) >= 0x60
                        ? "THISLANDISMYLAND" : "THISLANDISMYLAND2";
                Sprite *const portrait = FactionDiplomacyPortraits[other_faction];
                if (x_pops_minimal(land_prompt, portrait, nullptr) == 0) {
                    diplomacy_ends(faction_id, other_faction);
                    return;
                }

                GenderDefault = Players[faction_id].is_leader_female;
                PluralityDefault = 0;
                StringTemp[0] = 0;
                parse_says(0, Players[faction_id].title_leader, -1, -1);
                GenderDefault = Players[faction_id].is_leader_female;
                PluralityDefault = 0;
                parse_says(1, Players[faction_id].name_leader, -1, -1);
                GenderDefault = Players[faction_id].is_leader_female;
                PluralityDefault = 0;
                parse_says(2, Players[faction_id].adj_faction, -1, -1);
                GenderDefault = Players[faction_id].is_leader_female;
                PluralityDefault = 0;
                parse_says(3, Players[faction_id].adj_insult_faction, -1, -1);
                parse_says(4, get_phrase(other_faction, 0x0D), -1, -1);
                parse_says(5, get_noun_phrase(faction_id, 10), -1, -1);
                strcat(StringTemp, "BULLY");
                char random_text[16];
                std::sprintf(random_text, "%u", random(0, 7));
                strcat(StringTemp, random_text);
                x_pops_minimal(StringTemp,
                               FactionDiplomacyPortraits[other_faction], nullptr);
                diplomacy_ends(faction_id, other_faction);
            } else {
                if (X_pop((climate_at(x, y) & 0xE0) >= 0x60
                              ? "TERRTRUCE" : "TERRTRUCE2",
                          nullptr) == 0) {
                    return;
                }
            }
        } else if (X_pop((climate_at(x, y) & 0xE0) >= 0x60
                             ? "TERRTREATY" : "TERRTREATY2",
                         nullptr) == 0) {
            return;
        }
    }

    int drones = 0;
    int talents = 0;
    psych_check(faction_id, &drones, &talents);
    const int32_t post_psych_base_count = static_cast<int32_t>(
        PlayersData[faction_id].current_num_bases);
    if (post_psych_base_count != 0 && post_psych_base_count % drones == 0) {
        parse_num(0, drones);
        const int32_t prompt_base_count = static_cast<int32_t>(
            PlayersData[faction_id].current_num_bases);
        const char *const prompt = prompt_base_count == drones
                                       ? "BUREAUCRACY"
                                       : "BUREAUCRACY2";
        if (popp(*ClimatePopupTitle, prompt, 0x40, "bureau_sm.pcx", nullptr) < 0) {
            return;
        }
    }
    }

    char base_name[256];
    base_name[0] = 0;
    name_base(faction_id, base_name, 0, altitude < 0x60);
    if (existing_base < 0) {
        MapFocusX = x;
        MapFocusY = y;
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) != 0) {
            draw_map(1);
        }
        const int answer =
            X_pop_ask("NEWBASE", 0x17, base_name, nullptr, 0);
        MapFocusY = -99;
        MapFocusX = -99;
        if (answer != 0) {
            if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
                return;
            }
            draw_map(1);
            return;
        }
        std::strcpy(base_name, ParseStrBuffer[0].str);
    }

    ambience(0xD1);
    if (not_my_turn()) {
        NetMsgPrimary.pop("NEEDCOLONISTS", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        draw_map(1);
        return;
    }

    int locked_vehicle_id = vehicle_id;
    if (NetDaemonNet.lock_veh(&locked_vehicle_id, 4, -1, -1, 0) != 0) {
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        draw_map(1);
        return;
    }

    const Veh &locked_vehicle = Vehs[locked_vehicle_id];
    const VehPrototype &locked_prototype =
        VehPrototypes[locked_vehicle.proto_id];
    if (locked_prototype.plan != PLAN_COLONIZATION) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("NEEDCOLONISTS", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        draw_map(1);
        return;
    }

    const uint32_t locked_altitude = climate_at(x, y) & 0xE0;
    const bool locked_sea_chassis =
        Chassis[locked_prototype.chassis_id].triad == TRIAD_SEA;
    if ((locked_altitude < 0x60) != locked_sea_chassis) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("NOTINTRIAD", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        draw_map(1);
        return;
    }
    const Map *const locked_tile = map_loc(x, y);
    if ((locked_tile->bit & BIT_FUNGUS) != 0 && locked_altitude >= 0x40) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("FUNGUSDANGUS", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        draw_map(1);
        return;
    }
    if ((locked_tile->bit & BIT_MONOLITH) != 0) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("NOTONMONOLITH", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        draw_map(1);
        return;
    }
    if (!veh_ready(locked_vehicle_id)) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("UNITMOVED", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        draw_map(1);
        return;
    }

    const int locked_base = base_at(x, y);
    if (locked_base >= 0) {
        ++Bases[locked_base].population_size;
        synch_base(locked_base);
        if (IsMultiplayerNet) {
            message_veh(0x2407, locked_vehicle_id, 0, 0);
        } else {
            kill(locked_vehicle_id);
        }
        TutWinBaseWindow.check_base(locked_base);
        NetDaemonNet.await_exec(1);
        return;
    }

    bool locked_adjacent_base = false;
    for (int direction = 0; direction < 8; ++direction) {
        int adjacent_x = x + VehMoveDeltaX[direction];
        const int adjacent_y = y + VehMoveDeltaY[direction];
        if ((MapIsFlat & 1) == 0) {
            if (adjacent_x < 0) {
                adjacent_x += MapLongitudeBounds;
            } else if (adjacent_x >= MapLongitudeBounds) {
                adjacent_x -= MapLongitudeBounds;
            }
        }
        if (adjacent_y < 0 || adjacent_y >= MapLatitudeBounds ||
            adjacent_x < 0 || adjacent_x >= MapLongitudeBounds) {
            continue;
        }
        const Map *const adjacent = map_loc(adjacent_x, adjacent_y);
        if ((adjacent->bit & BIT_BASE_IN_TILE) != 0 &&
            (adjacent->val2 & 0x0F) < MaxPlayerNum) {
            locked_adjacent_base = true;
            break;
        }
    }
    if (locked_adjacent_base) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("BASETOONEAR", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        draw_map(1);
        return;
    }

    int new_base_id;
    if (!IsMultiplayerNet) {
        log_say("Manually building base", 0, 0, 0);
        new_base_id = action_build(locked_vehicle_id, base_name);
        if (new_base_id < 0) {
            return;
        }
    } else {
        log_say("Sending BUILD BASE message", base_name, locked_vehicle_id,
                faction_id, 0);
        message_base(0x2408, locked_vehicle_id, base_name, 0);
        NetDaemonNet.await_exec(1);
        new_base_id = base_at(x, y);
    }

    if (ready_search(-1) < 0) {
        GameDataStateStorage.fields_.game_state_ |= 4;
    }
    parse_says(0, Bases[new_base_id].name_string, -1, -1);
    parse_num(0, GameDataStateStorage.fields_.mission_year_current_);

    if (!IsMultiplayerNet) {
        const bool alien = (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0;
        const uint32_t founded_altitude = climate_at(x, y) & 0xE0;
        const char *const picture = alien
                                        ? (founded_altitude >= 0x60
                                               ? "al_col_sm.pcx"
                                               : "seabas_sm.pcx")
                                        : (founded_altitude >= 0x60
                                               ? "colfoun_sm.pcx"
                                               : "seabas_sm.pcx");
        popp(*ClimatePopupTitle, "BASEFOUNDED", 0, picture, nullptr);
    } else {
        NetMsgPrimary.pop("BASEFOUNDED", 5000, 0, nullptr);
    }
    if (!IsMultiplayerNet) {
        TutWinBaseWindow.zoom(new_base_id, 1);
    }
    ConsoleGlobal.update_data(1);
}

// ORIGINAL: 0x004D3470 ?terraform@Console@@QAEXHH@Z 0x004D3470-0x004D5277;0x0065A559-0x0065AB02 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004d3470.cpp", "image:terranx_original.exe:0x004D3470", "image:terranx_original.exe:0x004D4973,0x004D4CA6"], "reason": "Recover terraforming through the Console receiver, Map bit storage, typed order descriptors and real vehicle/network lock lifecycle. Native lock failure refunds without unlocking; conflict names distinguish sea and land. Popup owns teardown, not handwritten vtable cleanup."}
// TRIED: full-function transcription (all ~30 validation branches, the 6-way terraform-type switch incl. borehole/aquifer neighbor scans and the raise/lower-terrain payment+territory/treaty flow, the post-switch lock/exec/unlock tail) using validated Veh/Map/VehPrototype/Chassis record layouts from src/veh.h and src/map.h; Popup RAII lever (`Popup popup;` + plain `return;`) for all teardown/SEH machinery, never hand-written. MISMATCH #2: rebuilt frame is 4 bytes larger than the original's 0x5398, so every ebp-relative offset drifts from the prologue on - not a control-flow or call-order defect, a local-variable-count difference in the /O2 stack layout that was not tracked down further. Two inline uses of a materialized `VehRec *veh` pointer were already removed in favor of re-deriving `g_vehs[a1].field` at each site (matched the original's shape and recovered 8 of that 12-byte gap); the remaining 4 bytes are still open. Approximated without full confidence: the exact terraformID reassignment thresholds in the a2==0/a2==5 auto-select block, the case-0x10/0x11 payment/steps-count arithmetic, and which register (`edx`) still holds `altBits` vs a freshly reloaded value at a few of the later bit tests - transcribed as re-reads of tile fields rather than a persisted local where raw-disasm register lineage got ambiguous after ~40 intervening instructions.
void Console::terraform(int vehicle_id, int requested_terraform) { // 0x004D3470
    Popup popup;

    // The native entry tests the current vehicle prototype's plan before
    // reading its map tile.  0x004D34C6 compares VehPrototype::plan to 9.
    const int entry_proto_id = Vehs[vehicle_id].proto_id;
    if (VehPrototypes[entry_proto_id].plan != 9) {
        NetMsgPrimary.pop("NEEDTERRAFORMERS", 5000, 0, nullptr);
        return;
    }

    // Original 0x004D3521..0x004D3585 snapshots the selected vehicle's
    // coordinates, classifies altitude from climate & 0xE0, and permits AIR
    // chassis on either terrain.  The even/odd map-column convention is the
    // same one used by map_loc throughout the typed map owner.
    const int x = Vehs[vehicle_id].x;
    const int y = Vehs[vehicle_id].y;
    Map *const tile = map_loc(x, y);
    const int altitude = tile->climate & 0xE0;
    const int is_sea = altitude < 0x60;
    const int chassis_id = VehPrototypes[entry_proto_id].chassis_id;
    const int triad = Chassis[chassis_id].triad;
    if (triad != TRIAD_AIR && is_sea != (triad == TRIAD_SEA)) {
        NetMsgPrimary.pop("NOTINTRIAD", 5000, 0, nullptr);
        return;
    }

    // Preserve the native order substitution rules: an existing road can
    // become a magtube; a farm request can remove fungus or select soil
    // enrichment.  The prereq comparison is >= -1, exactly as the native
    // signed DWORD comparison at 0x004D35EA / 0x004D361F.
    int terraform_id = requested_terraform;
    if (terraform_id == TERRA_ROAD) {
        if ((tile->bit & BIT_ROAD) != 0 &&
            (is_sea ? Terraforming[TERRA_MAGTUBE].preq_tech_sea
                    : Terraforming[TERRA_MAGTUBE].preq_tech) >= -1) {
            terraform_id = TERRA_MAGTUBE;
        }
    } else if (terraform_id == TERRA_FARM) {
        if ((tile->bit & BIT_FUNGUS) != 0 && altitude >= 0x40) {
            terraform_id = TERRA_REMOVE_FUNGUS;
        } else if ((tile->bit & BIT_FARM) != 0 && altitude >= 0x60 &&
                   (is_sea ? Terraforming[TERRA_SOIL_ENR].preq_tech_sea
                           : Terraforming[TERRA_SOIL_ENR].preq_tech) >= -1) {
            terraform_id = TERRA_SOIL_ENR;
        }
    }

    // Native auto-selection only rewrites the ordinary road/farm requests;
    // explicit Soil Enricher remains a separate order and is validated below.

    // 0x004D3632 passes the selected Order label directly to parse_says;
    // it is not a StringTable key. The native table starts at Order[4].
    char *order_label = is_sea ? Order[terraform_id + 4].order_sea
                               : Order[terraform_id + 4].order;
    parse_says(6, order_label, -1, -1);

    // The first availability check is based on each terrain row's typed
    // prerequisite; < -1 means no availability in that environment.
    const int selected_prerequisite = is_sea
        ? Terraforming[terraform_id].preq_tech_sea
        : Terraforming[terraform_id].preq_tech;
    if (selected_prerequisite < -1) {
        const int other_prerequisite = is_sea
            ? Terraforming[terraform_id].preq_tech
            : Terraforming[terraform_id].preq_tech_sea;
        const char *message = other_prerequisite < -1 ? "NOTATALL"
            : is_sea ? "NOTATSEA" : "NOTONLAND";
        NetMsgPrimary.pop(message, 5000, 0, nullptr);
        return;
    }

    if (terraform_id == TERRA_SOIL_ENR && (tile->bit & BIT_FARM) == 0) {
        return;
    }
    if (terraform_id == TERRA_MAGTUBE && (tile->bit & BIT_ROAD) == 0) {
        return;
    }

    const uint32_t terrain_bit = Terraforming[terraform_id].bit;
    if (terrain_bit != 0 && (tile->bit & terrain_bit) == terrain_bit) {
        char *current_improvement = is_sea
            ? Terraforming[terraform_id].name_sea
            : Terraforming[terraform_id].name;
        parse_says(0, current_improvement, -1, -1);
        NetMsgPrimary.pop("ALREADYBUILT", 5000, 0, nullptr);
        return;
    }
    if ((tile->bit & BIT_MONOLITH) != 0 && terrain_bit != 0 &&
        terraform_id != TERRA_ROAD && terraform_id != TERRA_MAGTUBE &&
        terraform_id != TERRA_PLANT_FUNGUS) {
        NetMsgPrimary.pop("NOTONMONOLITH", 5000, 0, nullptr);
        return;
    }

    if (is_sea && altitude < 0x40 &&
        terraform_id != TERRA_RAISE_LAND &&
        terraform_id != TERRA_LOWER_LAND) {
        if ((Players[faction_id_].rule_flags & RFLAG_AQUATIC) == 0) {
            NetMsgPrimary.pop("LOWTIDAL", 5000, 0, nullptr);
            return;
        }
        if (!has_tech(0x3E, faction_id_)) {
            NetMsgPrimary.pop("NEEDADVECOENG", 5000, 0, nullptr);
            return;
        }
    }

    // 0x004D3B3B identifies volcano squares in Map::bit2. The native warning
    // differs by one literal suffix when the improvement itself is not in
    // the volatile group.
    const uint32_t volcano_state = tile->bit2;
    if ((volcano_state & 0x80000002u) == 2u) {
        const uint32_t volatile_bits = Terraforming[terraform_id].bit;
        if ((volatile_bits & 0x208000u) != 0 ||
            (volcano_state & 0xFF000000u) == 0) {
            StringTemp[0] = 0;
            strcat(StringTemp, "NOTONVOLCANO");
            if ((volatile_bits & 0x208000u) == 0) {
                strcat(StringTemp, "1");
            }
            NetMsgPrimary.pop(StringTemp, 5000, 0, nullptr);
            return;
        }
    }

    if (!is_sea && (tile->contour & 0xC0) > 0x40 &&
        (terrain_bit & 0x208000u) != 0) {
        NetMsgPrimary.pop("NOTINROCKY", 5000, 0, nullptr);
        return;
    }

    // Existing fungus requires the corresponding rules tech. TechDisabled is
    // the native sentinel selecting the generic label; otherwise say_tech
    // materializes the configured technology name into StringTemp first.
    if ((tile->bit & BIT_FUNGUS) != 0 && altitude >= 0x40) {
        const int fungus_tech = Rules.tech_improve_fungus_sqr;
        if ((terrain_bit & 4u) != 0 && !has_tech(fungus_tech, faction_id_)) {
            const char *message = "NOTINFUNGUS";
            if (fungus_tech >= 0) {
                StringTemp[0] = 0;
                say_tech(StringTemp, fungus_tech, 0);
                parse_says(0, StringTemp, -1, -1);
                message = "NOTYETFUNGUS";
            }
            NetMsgPrimary.pop(message, 5000, 0, nullptr);
            return;
        }
    } else if ((tile->bit & BIT_FUNGUS) != 0) {
        const int road_tech = Rules.tech_build_roads_fungus;
        if (!has_tech(road_tech, faction_id_)) {
            const char *message = "NOTINFUNGUS";
            if (road_tech >= 0) {
                StringTemp[0] = 0;
                say_tech(StringTemp, road_tech, 0);
                parse_says(0, StringTemp, -1, -1);
                message = "NOTYETFUNGUS";
            }
            NetMsgPrimary.pop(message, 5000, 0, nullptr);
            return;
        }
    }

    // Native terrain_avail checks the selected prerequisite against the
    // faction's known technologies, with the Weather Paradigm and terraforming
    // abilities handled by that existing owner.  The earlier numeric check
    // above only distinguishes an unavailable terrain type/environment.
    if (!terrain_avail(terraform_id, is_sea, faction_id_)) {
        if (terraform_id == TERRA_REMOVE_FUNGUS &&
            has_abil(Vehs[vehicle_id].proto_id, 0x2000)) {
            // Native former ability bypass at 0x004D37E7..0x004D380D.
        } else {
            const int prerequisite = is_sea
                ? Terraforming[terraform_id].preq_tech_sea
                : Terraforming[terraform_id].preq_tech;
            if (prerequisite >= TechNone) {
                StringTemp[0] = 0;
                say_tech(StringTemp, prerequisite, 0);
                parse_says(0, StringTemp, -1, -1);
            }
            const char *message = terraform_id == TERRA_MAGTUBE ? "HASROAD"
                : terraform_id == TERRA_SOIL_ENR ? "HASFARM" : "NOTUNTILTECH";
            NetMsgPrimary.pop(message, 5000, 0, nullptr);
            return;
        }
    }

    if (terraform_id == TERRA_RAISE_LAND ||
        terraform_id == TERRA_LOWER_LAND) {
        if ((GameDataStateStorage.fields_.game_rules_ & 0x00800000u) != 0) {
            NetMsgPrimary.pop("NOTINSCENARIO", 5000, 0, nullptr);
            return;
        }
        if (terraform_id == TERRA_RAISE_LAND) {
            if (altitude >= 0x60) {
                if (altitude >= 0xC0) {
                    NetMsgPrimary.pop("TOOHIGH", 5000, 0, nullptr);
                    return;
                }
            } else if (altitude >= 0x40) {
                NetMsgPrimary.pop("SEATOOHIGH", 5000, 0, nullptr);
                return;
            }
        } else if (altitude == 0) {
            NetMsgPrimary.pop("SEATOOLOW", 5000, 0, nullptr);
            return;
        } else if (altitude == 0x60) {
            NetMsgPrimary.pop("TOOLOW", 5000, 0, nullptr);
            return;
        }
    }

    switch (terraform_id) {
    case TERRA_SOLAR_TIDAL:
        if (energy_yield(faction_id_, -1, x, y, 1) == 0) {
            NetMsgPrimary.pop("LOWSOLAR", 5000, 0, nullptr);
            return;
        }
        break;

    case TERRA_THERMAL_BORE: {
        const int own_altitude_level = tile->climate >> 5;
        for (int borehole_direction = 0; borehole_direction < 8;
             ++borehole_direction) {
            int adjacent_x = x + VehMoveDeltaX[borehole_direction];
            if ((MapIsFlat & 1) == 0) {
                if (adjacent_x < 0) adjacent_x += MapLongitudeBounds;
                else if (adjacent_x >= MapLongitudeBounds) {
                    adjacent_x -= MapLongitudeBounds;
                }
            }
            const int adjacent_y = y + VehMoveDeltaY[borehole_direction];
            if (adjacent_y < 0 || adjacent_y >= MapLatitudeBounds ||
                adjacent_x < 0 || adjacent_x >= MapLongitudeBounds) {
                continue;
            }
            const Map *const adjacent_tile = map_loc(adjacent_x, adjacent_y);
            if ((adjacent_tile->climate & 0xE0) <= 0x5F) {
                continue;
            }
            if ((adjacent_tile->bit & BIT_THERMAL_BORE) != 0) {
                X_pop("TWOBOREHOLES", nullptr);
                return;
            }
            for (int nearby_vehicle = veh_at(adjacent_x, adjacent_y);
                 nearby_vehicle >= 0;
                 nearby_vehicle = Vehs[nearby_vehicle].next_veh_id_stack) {
                if (Vehs[nearby_vehicle].faction_id == faction_id_ &&
                    Vehs[nearby_vehicle].order == 0x12) {
                    X_pop("TWOBOREHOLES", nullptr);
                    return;
                }
            }
            if ((adjacent_tile->climate >> 5) < own_altitude_level) {
                X_pop("NOBOREHOLE", nullptr);
                return;
            }
        }
        break;
    }

    case TERRA_AQUIFER: {
        int aquifer_direction;
        for (aquifer_direction = 0; aquifer_direction < 8;
             ++aquifer_direction) {
            int adjacent_x = x + VehMoveDeltaX[aquifer_direction];
            if ((MapIsFlat & 1) == 0) {
                if (adjacent_x < 0) adjacent_x += MapLongitudeBounds;
                else if (adjacent_x >= MapLongitudeBounds) {
                    adjacent_x -= MapLongitudeBounds;
                }
            }
            const int adjacent_y = y + VehMoveDeltaY[aquifer_direction];
            if (adjacent_y < 0 || adjacent_y >= MapLatitudeBounds ||
                adjacent_x < 0 || adjacent_x >= MapLongitudeBounds) {
                continue;
            }
            const Map *const adjacent_tile = map_loc(adjacent_x, adjacent_y);
            if ((adjacent_tile->climate & 0xE0) > 0x5F &&
                (adjacent_tile->bit & BIT_RIVER) != 0) {
                break;
            }
        }
        if (aquifer_direction == 8) {
            NetMsgPrimary.pop("NOAQUIFER", 5000, 0, nullptr);
            return;
        }
        break;
    }

    case TERRA_LEVEL_TERRAIN:
        if ((tile->contour & 0xC0) == 0) {
            X_pop("TOOFLAT", nullptr);
            return;
        }
        break;

    case TERRA_RAISE_LAND:
    case TERRA_LOWER_LAND: {
        log_say("Want to terraform", faction_id_, 0, 0);
        int payment_amount = 0;
        if (stack_check(vehicle_id, 9, terraform_id + 4, -1, -1) == 0) {
            const int cost = terraform_cost(x, y, faction_id_);
            if (cost != 0) {
                payment_amount = cost;
                parse_num(0, cost);
                parse_num(1, PlayersData[faction_id_].energy_reserves);
                StringTemp[0] = 0;
                strcat(StringTemp, "TERRACOST");
                popup.start(PopupStartCaption, StringTemp, -1, nullptr, 0,
                            nullptr);

                if (PlayersData[faction_id_].energy_reserves >= payment_amount) {
                    popup.start(PopupStartCaption,
                                terraform_id == TERRA_LOWER_LAND ? "DOWN" : "UP",
                                -1, nullptr, 0x80, nullptr);
                    log_say("Offering option to pay", 0, 0, 0);
                }
                if (popup.exec(0, nullptr) == 0) {
                    log_say("Didn't or couldn't pay; returning", 0, 0, 0);
                    return;
                }
                log_say("Took option to pay", 0, 0, 0);
                PlayersData[faction_id_].energy_reserves -= payment_amount;
                synch_energy(faction_id_);
                update_data(0);
            }
        }

        int steps = 1;
        if (terraform_id == TERRA_RAISE_LAND) {
            int level = (tile->climate >> 5) + 1;
            if (level > 6) {
                level = 6;
                steps = RadiusRange[level - 3];
            } else if (level > 3) {
                steps = RadiusRange[level - 3];
            }
        } else {
            int level = (tile->climate >> 5) - 1;
            if (level < 0) {
                level = 0;
            }
            if (level < 3) {
                steps = RadiusRange[4 - level];
            }
        }

        int terraform_offset;
        for (terraform_offset = 0; terraform_offset < steps;
             ++terraform_offset) {
            int affected_x = x + RadiusOffsetX[terraform_offset];
            if ((MapIsFlat & 1) == 0) {
                if (affected_x < 0) {
                    affected_x += MapLongitudeBounds;
                } else if (affected_x >= MapLongitudeBounds) {
                    affected_x -= MapLongitudeBounds;
                }
            }
            const int affected_y = y + RadiusOffsetY[terraform_offset];
            if (affected_y < 0 || affected_y >= MapLatitudeBounds ||
                affected_x < 0 || affected_x >= MapLongitudeBounds) {
                continue;
            }

            const int target_faction = whose_territory(
                faction_id_, affected_x, affected_y, nullptr, nullptr);
            if (target_faction < 0 || target_faction == faction_id_ ||
                (PlayersData[faction_id_].diplo_treaties[target_faction] &
                 7) == 0) {
                continue;
            }

            if (NetDaemonNet.lock_veh(&vehicle_id, 2, -1, -1, 0) != 0) {
                if (payment_amount != 0) {
                    PlayersData[faction_id_].energy_reserves += payment_amount;
                    synch_energy(faction_id_);
                    update_data(0);
                }
                return;
            }
            if (break_treaty(faction_id_, target_faction, 7) != 0) {
                if (payment_amount != 0) {
                    PlayersData[faction_id_].energy_reserves += payment_amount;
                    synch_energy(faction_id_);
                    update_data(0);
                }
                NetDaemonNet.unlock_veh();
                return;
            }
            if (IsMultiplayerNet != 0) {
                message_data(0x2440, 0, faction_id_, target_faction, -1, 0);
                NetDaemonNet.await_exec(1);
            } else {
                double_cross(faction_id_, target_faction, -1);
            }
        }

        if (tut_check(0x800) != 0) {
            popt(TutorialPopupCaption, "TERRAFORM", -1, nullptr, 0);
        }
        break;
    }

    default:
        break;
    }

    // The common post-dispatch pass reloads the original map tile before
    // checking the selected order against incompatible terrain descriptors.
    Map *const final_tile = map_loc(x, y);
    if (terraform_id != TERRA_REMOVE_FUNGUS &&
        terraform_id != TERRA_RAISE_LAND &&
        terraform_id != TERRA_LOWER_LAND) {
        const int final_altitude = final_tile->climate & 0xE0;
        if (final_altitude > 0x3F) {
            const uint32_t conflict =
                Terraforming[terraform_id].bit_incompatible & final_tile->bit;
            if (conflict != 0) {
                int conflict_index;
                for (conflict_index = 0; conflict_index < 20;
                     ++conflict_index) {
                    if ((Terraforming[conflict_index].bit & conflict) != 0) {
                        char *conflicting_name = final_altitude < 0x60
                            ? Terraforming[conflict_index].name_sea
                            : Terraforming[conflict_index].name;
                        parse_says(0, conflicting_name, -1, -1);
                        if (X_pop("MIMIMI", nullptr) == 0) return;
                        break;
                    }
                }
            }
        }
    }

    if (NetDaemonNet.lock_state_[2].leading_words_[3] != 0) {
        log_say("I think I've already got my lock", 0, 0, 0);
    }
    if (NetDaemonNet.lock_state_[2].leading_words_[3] == 0) {
        if (NetDaemonNet.lock_veh(&vehicle_id, 0, -1, -1, 0) != 0) return;
    }

    Vehs[vehicle_id].order = static_cast<uint8_t>(terraform_id + 4);
    synch_veh(vehicle_id);
    log_say("Issuing Vehicle Orders", vehicle_id, terraform_id + 4, 0);
    if (IsMultiplayerNet == 0) {
        NetDaemonNet.unlock_veh();
    } else {
        const int current_speed = speed(vehicle_id, 0);
        const int movement_remaining =
            current_speed - Vehs[vehicle_id].moves_expended;
        if (movement_remaining > 0) {
            NetDaemonNet.action(vehicle_id, 1);
        } else {
            NetDaemonNet.unlock_veh();
        }
    }
    log_say("Finished issuing orders", vehicle_id, terraform_id + 4, 0);
    field_23BE4_ = 0;
}

// ORIGINAL: 0x00514050 ?human_turn@Console@@QAEXXZ 0x00514050-0x005147DA FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x00514050-0x005147DA", "git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00514050.cpp", "FX bank749CF8 plus57 times6C identifies74B504 as existing Effect57; original dispatch uses Wave virtual operations.", "Native5140DC/5143B4/51467E/514694 establish cursor polarity, selection reload and branch-specific flag/audio order."], "reason": "Recover player-turn orchestration in its genuine Console owner, preserving callback-driven selection/faction reloads, cursor guards, turn flags and real UI/audio lifetimes."}
// TRIED: direct transcription using Console's already-declared field_23BDC_ etc members (no raw casts needed, no embedded objects here - simple frameless prologue); cross-checked against raw asm for the neg/sbb/inc bool-negate idiom, the xor-swapped focus() arg orders, and the veh_ready/veh_selectable branch. Compiles, sim 0.55 frameless; not retried given size (1930 bytes).
void Console::human_turn() {
    unsigned int flush_cache = 0;
    int previous_vehicle_count = -1;

    WorldClimateWorldWin.viewport_outline_enabled_ = 0;
    WorldClimateWorldWin.set_world_map();
    GameDataStateStorage.fields_.game_state_ &= ~static_cast<uint32_t>(STATE_UNK_400);
    MapBaseDisplayOverride = 1;
    field_23D1C_ = 0;

    for (int vehicle = 0; vehicle < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle) {
        Vehs[vehicle].state &= ~static_cast<uint32_t>(VSTATE_UNK_8000000);
    }

    if (ControlTurnPhase == 0) {
        selected_vehicle_ = -1;
        set_move(0);
    } else if (ConsoleGlobal.cursor_active_ != 0) {
        set_move(1);
    } else {
        ambience(0x1E);
        cursor_active_ = 0;
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            MapWinTable[0]->draw_map(1);
        }
        ::draw_cursor();
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        MapWinTable[0]->main_caption();
        alpha_menu_.check(selected_vehicle_);
    }

    alpha_menu_.check(selected_vehicle_);
    ConsoleStatusWin.reset();
    ConsoleInfoWin.reset();
    field_23BE8_ = 1;
    field_23D24_ = 0;

    if (ControlTurnPhase != 0) {
        const int selected = selected_vehicle_;
        bool refresh = selected < 0 || veh_ready(selected) == 0;
        if (!refresh) {
            // Native reloads selected_vehicle_ after veh_ready returns.
            const int current_selected = selected_vehicle_;
            refresh = (Vehs[current_selected].state & 0x4200u) != 0 ||
                      Vehs[current_selected].order != 0;
        }
        if (refresh && ConsoleGlobal.cursor_active_ != 0) {
            ambience(0x1E);
            field_23BF8_ = 1;
            field_23BFC_ = 1;
            cursor_active_ = 0;
            if (survey_mode_ != 0) {
                survey_mode_ = 0;
                MapWinTable[0]->draw_map(1);
            }
            ::draw_cursor();
            ConsoleInfoWin.change(0);
            ConsoleStatusWin.redraw();
            MapWinTable[0]->main_caption();
            alpha_menu_.check(selected_vehicle_);
        }
    }

    if (IsMultiplayerNet != 0) {
        g_FX.play(0x21);
    }
    ControlTurnPhase = 0;

    for (;;) {
        if ((GameDataStateStorage.fields_.game_state_ & 0x2000u) != 0 && (GameDataStateStorage.fields_.game_state_ & 1u) == 0 &&
            (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_CONQUEST) != 0) {
            GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
            GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
            GenderDefault = Players[ConsoleGlobal.faction_id_].noun_gender;
            PluralityDefault = Players[ConsoleGlobal.faction_id_].is_noun_plural;
            parse_says(2, Players[ConsoleGlobal.faction_id_].noun_faction, -1, -1);
            X_pop("CONQUERPLANET", nullptr);
            GameDataStateStorage.fields_.game_state_ |= 1u;
        }

        bool keep_selection = false;
        const int selected = selected_vehicle_;
        const bool cursor_active = ConsoleGlobal.cursor_active_ != 0;
        if (!cursor_active) {
            if (selected >= 0 && GameDataStateStorage.fields_.vehicle_current_count_ == previous_vehicle_count) {
                keep_selection = field_23BEC_ == 0
                    ? veh_ready(selected) != 0
                    : veh_selectable(selected) != 0;
            }
        } else if (selected >= 0 &&
                   GameDataStateStorage.fields_.vehicle_current_count_ == previous_vehicle_count &&
                   (field_23BEC_ == 0 ? veh_ready(selected) != 0
                                      : veh_selectable(selected) != 0)) {
            keep_selection = true;
            if (field_23BF4_ == 0) {
                // The helper can run callbacks; native reloads the selection
                // after the eligibility callback and before reading its tile.
                const int selected_for_cursor = selected_vehicle_;
                const int x = Vehs[selected_for_cursor].x;
                const int y = Vehs[selected_for_cursor].y;
                field_23BF8_ = 0;
                if (survey_mode_ != 0) {
                    survey_mode_ = 0;
                    MapWinTable[0]->draw_map(1);
                }
                MapWinTable[0]->draw_cursor();
                cursor_next(x, y);
                // cursor_next may dispatch callbacks that change the live
                // selection; native reloads selected_vehicle_ afterward.
                const int current_selected = selected_vehicle_;
                if (field_23BF0_ == 0 || Vehs[current_selected].order != 0x18) {
                    const int focus_x = Vehs[current_selected].x;
                    const int focus_y = Vehs[current_selected].y;
                    const int focus_faction = Vehs[current_selected].faction_id;
                    focus(focus_x, focus_y, focus_faction);
                    ConsoleInfoWin.change(0);
                    ConsoleStatusWin.redraw();
                    MapWinTable[0]->main_caption();
                    alpha_menu_.check(selected_vehicle_);
                }
            }
        }
        if (!keep_selection) {
            ready();
        }

        previous_vehicle_count = GameDataStateStorage.fields_.vehicle_current_count_;
        field_23BF4_ = 0;
        field_23BF0_ = 0;
        field_23BEC_ = 0;

        if (selected_vehicle_ < 0) {
            GameDataStateStorage.fields_.game_state_ |= 2u;
            if (TutWinBaseWindow.is_visible() || ReportWinGlobal.is_visible()) {
                GameDataStateStorage.fields_.game_state_ |= 4u;
            }
            field_23BE8_ = 1;
            const int lock_result = edit_lock();
            if (lock_result == 0) {
                const bool manual_stop =
                    (GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_PAUSE_END_TURN) == 0 &&
                    IsMultiplayerNet == 0 && IsMultiplayerPBEM == 0 &&
                    flush_cache != 0 && (GameDataStateStorage.fields_.game_state_ & 4u) == 0;
                if (manual_stop) {
                    field_23BE8_ = 0;
                } else if ((GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_PAUSE_END_TURN) == 0 ||
                           IsMultiplayerNet != 0) {
                    parse_num(0, static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_)));
                    NetMsgPrimary.pop(flush_cache != 0 ? "ENDOFTURN" : "ENDOFTURN1",
                                     0x1388, 0, nullptr);
                }
            }
            if (field_23BE8_ == 0) {
                goto finish_turn;
            }

            ambience(0x1E);
            cursor_active_ = 0;
            field_23BF8_ = 1;
            field_23BFC_ = 1;
            if (survey_mode_ != 0) {
                survey_mode_ = 0;
                MapWinTable[0]->draw_map(1);
            }
            ::draw_cursor();
            ConsoleInfoWin.change(0);
            ConsoleStatusWin.redraw();
            MapWinTable[0]->main_caption();
            alpha_menu_.check(selected_vehicle_);
            focus(ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
                  ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_],
                  faction_id_);
        }

        turn_hints();
        GameDataStateStorage.fields_.game_state_ &= ~4u;

        if (selected_vehicle_ >= 0) {
            GameDataStateStorage.fields_.game_state_ &= ~2u;
            flush_cache |= static_cast<unsigned int>(veh_turn());
        } else {
            // 0x0074B504 is g_FX.effects_[57] (g_FX + 57 * 0x6C).
            // Native reaches it only on the negative-selection wait branch,
            // after turn_hints and the nonnegative-selection dispatch. Keep
            // these calls virtual on the typed Effect/Wave receiver.
            Effect &cpu_turn_complete = g_FX.effect(57);
            if (!cpu_turn_complete.is_playing()) {
                cpu_turn_complete.load();
                cpu_turn_complete.set_reverb_mix(0.0f);
                cpu_turn_complete.play();
            }
            ambience(0xCB);
            field_23BE4_ = 1;
            const int saved_vehicle_count = GameDataStateStorage.fields_.vehicle_current_count_;
            while (ConsoleGlobal.cursor_active_ == 0 && field_23BE4_ != 0 &&
                   field_23BE8_ != 0 && ExitTurnLoop == 0 &&
                   end_of_game(0) == 0 &&
                   saved_vehicle_count == GameDataStateStorage.fields_.vehicle_current_count_ &&
                   (IsMultiplayerNet == 0 || (GameDataStateStorage.fields_.game_state_ & 0x800u) == 0)) {
                wait_loop();
                if (TutWinTourPending != 0) {
                    TutWinTourPending = 0;
                    TutorialWindow.tour();
                }
            }
            field_23BE4_ = 0;
        }

        if (ExitTurnLoop != 0) {
            return;
        }
        if ((GameDataStateStorage.fields_.game_state_ & 4u) != 0) {
            flush_cache = 0;
        }
        if (field_23BF8_ != 0) {
            ::draw_cursor();
        }
        if (field_23BE8_ == 0 || ExitTurnLoop != 0 ||
            (IsMultiplayerNet != 0 && (GameDataStateStorage.fields_.game_state_ & 0x800u) != 0) ||
            end_of_game(0) != 0) {
            goto finish_turn;
        }
    }

finish_turn:
    ambience(0x1E);
    flush_input();
    field_23BE8_ = 0;
    go_reset();
}

// ORIGINAL: 0x005171D0 ?veh_key@Console@@QAEXHHH@Z 0x005171D0-0x0051778A FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005171d0.cpp", "image:terranx_original.exe:0x005171D0", "Pinned key selector tables5177E4/517844 and accelerator targets517898; native selection reloads between dispatch stages preserve callback effects."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
// TRIED: MISMATCH #3 push-vs-mov in the prologue; hoisting the two guard reads into locals before the branch did not change register allocation order. Full switch/goto structure otherwise matches.
void Console::veh_key(int key, int character, int command) {
    if (IsMultiplayerNet != 0 && character != 'a') {
        if (ConsoleControlTurnActive != 0) return;
        if (NetDaemonNet.lock_state_[2].payload_.flags != 0) return;
        if ((NetDaemonNet.lock_state_[0].trailing_words_[7] &
             (1 << ConsoleGlobal.faction_id_)) != 0) return;
    }

    int vehicle_id = selected_vehicle_;
    switch (key) {
    case 'B': terraform(vehicle_id, 14); break;
    case 'E': terraform(vehicle_id, 13); break;
    case 'F': terraform(vehicle_id, 4); break;
    case 'G': go_home(vehicle_id); break;
    case 'H': hold(vehicle_id, 10); break;
    case 'I': go_to(vehicle_id, -3, 0); break;
    case 'L': automate(vehicle_id, 11); break;
    case 'P': {
        const uint32_t destination_flag = MapWinTable[0]->field_1DD70_ & 0x400000u;
        for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
            MapWin *const window = MapWinTable[slot];
            if (window != 0 && (slot == 0 || window->field_1DD74_ != 0)) {
                if (destination_flag == 0) {
                    window->field_1DD70_ |= 0x400000u;
                    window->draw_base_dest(1);
                } else {
                    window->field_1DD70_ &= 0xffbfffffu;
                    window->draw_map(1);
                }
            }
        }
        break;
    }
    case 'U': unload(vehicle_id); break;
    case 'a':
        if (NetDaemonNet.lock_state_[2].payload_.flags == 0) {
            activate(Vehs[vehicle_id].x, Vehs[vehicle_id].y, 1);
        }
        break;
    case 'f':
        if (can_arty(Vehs[vehicle_id].proto_id, 1) == 0) {
            terraform(vehicle_id, 0);
        } else {
            arty(vehicle_id);
        }
        break;
    case 'g': go_to(vehicle_id, 0x18, 0); break;
    case 'h': hold(vehicle_id, 0); break;
    case 'i': use_ability(vehicle_id); break;
    case 'l': sentry(vehicle_id, 0); break;
    case 'n': terraform(vehicle_id, 12); break;
    case 'o':
        if (VehPrototypes[Vehs[vehicle_id].proto_id].plan != 10) {
            terraform(vehicle_id, 9);
        } else {
            convoy(vehicle_id);
        }
        break;
    case 'p': set_patrol(vehicle_id); break;
    case 'q': terraform(vehicle_id, 15); break;
    case 'r': terraform(vehicle_id, 5); break;
    case 'b': {
        const VehPrototype &prototype = VehPrototypes[Vehs[vehicle_id].proto_id];
        const RulesChassis &chassis = Chassis[prototype.chassis_id];
        const RulesWeapon &weapon = Weapon[prototype.weapon_id];
        if (chassis.triad == TRIAD_AIR && prototype.plan != 8 &&
            weapon.offense_rating != 0) {
            automate(vehicle_id, 10);
        } else if (weapon.offense_rating == 0 || chassis.triad == TRIAD_AIR) {
            if (prototype.plan == 9) terraform(vehicle_id, 14);
            else new_base(vehicle_id);
        } else {
            oblit(vehicle_id);
        }
        break;
    }
    }

    // The image reloads the selected-id member after this first key switch;
    // several called Console operations can change it.
    vehicle_id = selected_vehicle_;
    switch (character) {
    case 0x20: skip(vehicle_id); break;
    case 0x2e: terraform(vehicle_id, 8); break;
    case 0x2f: explore(vehicle_id); break;
    case '[': terraform(vehicle_id, 17); break;
    case ']': terraform(vehicle_id, 16); break;
    case '_': terraform(vehicle_id, 18); break;
    case 'd': destroy(vehicle_id); break;
    case 'k': terraform(vehicle_id, 7); break;
    case 'm': terraform(vehicle_id, 2); break;
    case 's': terraform(vehicle_id, 3); break;
    }

    // The accelerator tree also reloads the member rather than preserving
    // the earlier switch's value.
    vehicle_id = selected_vehicle_;
    if (command == 0x30046) { automate(vehicle_id, 6); return; }
    if (command < 0x30047) {
        if (command == 0x20052) { road_to(vehicle_id, 0x1b); return; }
        if (command == 0x20044) { designate(vehicle_id); return; }
        if (command == 0x20046) { terraform(vehicle_id, 11); return; }
        if (command == 0x20048) { go_home(vehicle_id); return; }
        if (command == 0x20054) { road_to(vehicle_id, 0x1c); return; }
        if (command == 0x20055) { upgrade(vehicle_id); return; }
        if (command == 0x30041) {
            const VehPrototype &prototype = VehPrototypes[Vehs[vehicle_id].proto_id];
            const RulesChassis &chassis = Chassis[prototype.chassis_id];
            const RulesWeapon &weapon = Weapon[prototype.weapon_id];
            automate(vehicle_id, chassis.triad == TRIAD_AIR &&
                     weapon.offense_rating != 0 ? 12 : 3);
            return;
        }
    } else {
        switch (command) {
        case 0x3004d: automate(vehicle_id, 5); return;
        case 0x3004f: automate(vehicle_id, 7); return;
        case 0x30052: automate(vehicle_id, 1); return;
        case 0x30053: automate(vehicle_id, 4); return;
        case 0x30054: automate(vehicle_id, 2); return;
        case 0x30055: give_veh(vehicle_id); return;
        }
    }
}

// Faction-bit turn waiting state shared with net_not_my_turn. Original
// scalar accesses prove the role, not an exact allocation extent.
// RECOVERY_OBJECT: v1 0x0093E8EC ?ConsoleFactionsWaitingMask@@3HA
int ConsoleFactionsWaitingMask;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050FE70 ?on_nc_left_up@Console@@QAEXHH@Z 0x0050FE70-0x0051010A
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050fe70.cpp: retained complete original identity/spans and seed evidence", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions through sequence27", "Source owner interpretation preserves mapped original accesses, lifetime calls and branch roles; source-bound members and globals replace the seed offsets/address aliases. Historical seeds and claims remain independent restoration obligations."], "reason": "Recover splitter release into the genuine Console owner using source Win capture owners, real Win inheritance, embedded Sprite geometry and named splitter members. Preserve signed sentinel ordering and the original update operations; no fixed executable calls or self-offset access."}
// symbol    ?on_nc_left_up@Console@@UAEXHH@Z
void Console::on_nc_left_up(int x, int y) {
    Win *const window = static_cast<Win *>(this);
    if (WinPointerOwner3 == window) {
        WinPointerOwner3 = nullptr;
        window->on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == window) {
        WinPointerOwner4 = nullptr;
    }

    const uint32_t dragging = 0xFFFFFFFFu;
    if (field_23D80_ == dragging || field_23D84_ == dragging) {
        uint32_t *const target = field_23D80_ == dragging
            ? &field_23D80_ : &field_23D84_;
        const int half = sprite_a_.width() / 2;
        int position = x - border_thickness_;
        if (position < half) position = half;
        const int limit = client_rect_.right - client_rect_.left -
                          border_thickness_ * 2 - half;
        if (position > limit) position = limit;
        *target = static_cast<uint32_t>(position);
        update(0, ConsoleDragLineY, dib_.bmiHeader.biWidth, 1, nullptr);
        update(ConsoleDragLineX, 0, 1,
               -static_cast<int>(dib_.bmiHeader.biHeight), nullptr);
    } else if (field_23D88_ == dragging || field_23D8C_ == dragging) {
        uint32_t *const target = field_23D88_ == dragging
            ? &field_23D88_ : &field_23D8C_;
        const int bottom_border = bottom_border_thickness_ >= 0
            ? bottom_border_thickness_ : border_thickness_;
        const int track_offset = client_rect_.top - client_rect_.bottom -
            outer_rect_.top + outer_rect_.bottom + bottom_border;
        const int half = sprite_b_.height() / 2;
        int position = track_offset + y;
        if (position < half) position = half;
        const int limit = client_rect_.bottom - client_rect_.top -
                          caption_height_ - bottom_border - half;
        if (position > limit) position = limit;
        *target = static_cast<uint32_t>(position);
        update(0, ConsoleDragLineY, dib_.bmiHeader.biWidth, 1, nullptr);
        update(ConsoleDragLineX, 0, 1,
               -static_cast<int>(dib_.bmiHeader.biHeight), nullptr);
    }

    if (static_cast<int32_t>(field_23D88_) >
        static_cast<int32_t>(field_23D8C_)) {
        const uint32_t value = field_23D88_;
        field_23D88_ = field_23D8C_;
        field_23D8C_ = value;
    }
    if (static_cast<int32_t>(field_23D80_) >
        static_cast<int32_t>(field_23D84_)) {
        const uint32_t value = field_23D80_;
        field_23D80_ = field_23D84_;
        field_23D84_ = value;
    }
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005169F0 ?end_my_turn@Console@@QAEXXZ 0x005169F0-0x00516C4D
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005169f0.cpp: retained complete original identity/spans and seed evidence", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions through sequence27", "Source owner interpretation preserves mapped original accesses, lifetime calls and branch roles; source-bound members and globals replace the seed offsets/address aliases. Historical seeds and claims remain independent restoration obligations."], "reason": "Recover end-turn into the genuine Console owner using existing pane/timer/lifecycle members, shared faction waiting state and the independently read original popup captions. Preserve remote-turn and ready-vehicle decisions, including separate timer clear rather than InfoWin::reset. No matching or game-acceptance claim."}
void Console::end_my_turn() {
    TutWinBaseWindow.cancel_zoom_wait();
    ConsoleInfoWin.clear_pending_timer();
    TutWinGlobal->reset();
    ConsoleStatusWin.reset();
    g_FX.play(0xF);

    if (ConsoleControlTurnActive != 0) return;
    if (edit_lock() != 0) {
        NetMsgPrimary.pop("PROTECTED", 0x1388, 0, nullptr);
        return;
    }

    const uint8_t game_flags = static_cast<uint8_t>(GameDataStateStorage.fields_.game_state_);
    if (IsMultiplayerNet != 0) {
        if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10) != 0 &&
            GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) {
            const int faction = GameDataStateStorage.fields_.net_turn_faction_;
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[faction].title_leader, -1, -1);
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[faction].name_leader, -1, -1);
            GenderDefault = Players[faction].noun_gender;
            PluralityDefault = Players[faction].is_noun_plural;
            parse_says(2, Players[faction].noun_faction, -1, -1);
            const int process_id = NetDaemonNet.who_2_pid(faction);
            parse_says(3, NetDaemonNet.get_player_name(process_id), -1, -1);
            NetMsgPrimary.pop("SIMULWHOSE", 0x1388, 0, nullptr);
            return;
        }
        if ((game_flags & 2) != 0 && (GameDataStateStorage.fields_.net_turn_flags_ & 0x10) == 0) {
            const int faction_bit = 1 << ConsoleGlobal.faction_id_;
            if ((ConsoleFactionsWaitingMask & faction_bit) != 0 &&
                ConsoleFactionsWaitingMask != 0xFF) {
                NetMsgPrimary.pop("ATTEMPTTOCANCEL", 0x1388, 0, nullptr);
                message_data(0x2302, 0, GameDataStateStorage.fields_.turn_current_num_, 0, 0, 0);
                return;
            }
        }
    }

    if (field_23BE8_ == 0) return;
    if ((game_flags & 2) == 0) {
        set_move(1);
        if (ConsoleGlobal.cursor_active_ != 0) {
            if (selected_vehicle_ >= 0 &&
                (veh_ready(selected_vehicle_) != 0 ||
                 ConsoleGlobal.ready_search(selected_vehicle_) >= 0) &&
                X_pop("REALLYOVER", nullptr) == 0) {
                return;
            }
        }
    }

    GameDataStateStorage.fields_.game_state_ |= 2u;
    const int current_selection = selected_vehicle_;
    field_23BE4_ = 0;
    field_23BE8_ = 0;
    focus_vehicle_ = current_selection;
}

// Shared no-map interruption state observed in menu/key input and FLIC timer.
// Start-only binding: original allocation extent and lifetime remain unverified.
// RECOVERY_OBJECT: v1 0x0093A944 ?ConsoleNoMapOverrideLatch@@3HA
int ConsoleNoMapOverrideLatch;

extern int ConsoleNoMapOverrideLatch;
extern int __cdecl pop_wait();

void Console::reset_turn_view() {
    ambience(0x1E);
    cursor_active_ = 0;
    field_23BF8_ = 1;
    field_23BFC_ = 1;
    if (survey_mode_ != 0) {
        survey_mode_ = 0;
        MapWinTable[0]->draw_map(1);
    }
    ::draw_cursor();
    ConsoleInfoWin.change(0);
    ConsoleStatusWin.redraw();
    MapWinTable[0]->main_caption();
    alpha_menu_.check(selected_vehicle_);
}

// ORIGINAL: 0x0051BC70 ?menu_proc@Console@@QAEXH@Z 0x0051BC70-0x0051D23E
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0051bc70.cpp"], "reason": "Recover complete menu dispatch using genuine Console members, typed owners and original command branches."}
// TRIED: full transcription of every case in the ~150-target dispatch (mnemonic_similarity 0.45, 834/1610 original mnemonics in common); first divergence is the Win::set_cursor call through the vbtable adjustor near the top loop (mov/add ordering), and downstream case bodies pick up extra instructions from real vtable dispatch (VCall) and MapWin/Console member calls the original inlined more cheaply than /O2 does here.
// symbol    ?menu_proc@Console@@QAEXH@Z
void Console::menu_proc(int command) {
    if (MapBaseDisplayOverride == 0) {
        ConsoleNoMapOverrideLatch = 1;
        return;
    }
    if (ExitTurnLoop != 0) return;

    TutWinGlobal->reset();
    go_reset();
    field_23D10_ = 0;

    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == nullptr || (slot != 0 && window->field_1DD74_ == 0)) {
            continue;
        }
        if (window->cursor_sprite_ != nullptr ||
            window->cursor_resource_id_ != 0x7F00) {
            window->set_cursor(0x7F00);
            window->cursor_resource_id_ = 0x7F00;
            window->cursor_sprite_ = nullptr;
        }
        const uint32_t dirty = window->field_1DD70_ & 0x00C40000u;
        if (dirty != 0) {
            window->field_1DD70_ &= ~0x00C40000u;
            window->draw_map(1);
        }
        do_non_input();
    }

    if (IsMultiplayerNet != 0) {
        const int command_group = (command >> 8) & 0xFF;
        if ((ConsoleControlTurnActive != 0 ||
             NetDaemonNet.lock_state_[2].payload_.flags != 0 ||
             NetDaemonNet.lock_state_[0].leading_words_[3] != 0) &&
            command_group != 0 &&
            command_group != 7 && command_group != 8) {
            return;
        }
        log_say_hex("Menu Event", command, 0, 0);
    }

    if (ConsoleGlobal.cursor_active_ != 0) {
        if (edit_lock() != 0) {
            reset_turn_view();
            return;
        }
        if (selected_vehicle_ < 0 && command != 0x5D) return;
    }

    if (command > 0x8FF && command < 0x913) {
        terraform(selected_vehicle_, command - 0x900);
    }

    if (command > 0x103) {
        if (command < 0x201) {
            switch (command) {
            case 0x105: social_select(ConsoleGlobal.faction_id_); break;
            case 0x106: {
                if ((GameDataStateStorage.fields_.game_rules_ & 0x200) == 0) break;
                PlayerData &player = PlayersData[ConsoleGlobal.faction_id_];
                int &popup_flags = PopupWriteCheckState;
                popup_flags = (player.ai_growth != 0 ? 1 : 0) |
                    (player.ai_tech != 0 ? 2 : 0) |
                    (player.ai_wealth != 0 ? 4 : 0) |
                    (player.ai_power != 0 ? 8 : 0);
                if (x_pop_short("TECHRANDOM", 1, 0) >= 0) {
                    const unsigned char flags =
                        static_cast<unsigned char>(popup_flags);
                    player.ai_growth = flags & 1;
                    player.ai_tech = (flags >> 1) & 1;
                    player.ai_wealth = (flags >> 2) & 1;
                    player.ai_power = (flags >> 3) & 1;
                    synch_ai(ConsoleGlobal.faction_id_);
                }
                break;
            }
            case 0x107:
                if ((GameDataStateStorage.fields_.game_rules_ & 0x200) == 0) set_tech(ConsoleGlobal.faction_id_);
                break;
            case 0x108: help_topic(2, 0); break;
            case 0x109: report_labs(); break;
            case 0x10A: report_energy(); break;
            case 0x10B: report_ops(); break;
            case 0x10C: report_projects(); break;
            case 0x10D: report_sat(); break;
            case 0x10E: report_security(); break;
            case 0x10F: report_score(0); break;
            case 0x110: monument(0); break;
            case 0x111: hall_of_fame(0); break;
            case 0x113:
                if (MultiWindowOpen == 0) MultiWindow.show();
                else MultiWindow.hide();
                break;
            case 0x114: corner_the_market(); break;
            case 0x115: show_scen_objectives(); break;
            }
        } else if (command < 0x324) {
            switch (command) {
            case 0x200:
                if (edit_lock() == 0) set_move(0);
                else NetMsgPrimary.pop("PROTECTED", 5000, 0, nullptr);
                break;
            case 0x201:
                if (ConsoleGlobal.cursor_active_ != 0) reset_turn_view();
                break;
            case 0x203: case 0x204: case 0x205: case 0x206: case 0x207:
            case 0x208: zoom(command, 0); break;
            case 0x209:
                GameDataStateStorage.fields_.game_more_preferences_ ^= 0x10000;
                if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x10000) == 0)
                    alpha_menu_.uncheck_menu_item(2, 0x209);
                else alpha_menu_.check_menu_item(2, 0x209);
                AlphaIniPrefs.more_preferences = GameDataStateStorage.fields_.game_more_preferences_;
                prefs_save(0); ::draw_map(1); break;
            case 0x20A: case 0x20B: {
                const uint32_t bit = command == 0x20A ? 0x100 : 0x200;
                GameDataStateStorage.fields_.game_preferences_ ^= bit;
                if ((GameDataStateStorage.fields_.game_preferences_ & bit) == 0)
                    alpha_menu_.uncheck_menu_item(2, command);
                else alpha_menu_.check_menu_item(2, command);
                AlphaIniPrefs.preferences = GameDataStateStorage.fields_.game_preferences_;
                prefs_save(0); ::draw_map(1); break;
            }
            case 0x20C:
                MapWinTable[0]->field_1DD70_ ^= 0x40000;
                ::draw_map(1); break;
            case 0x20D:
                StatusFlags = 0;
                desktop_init(1); break;
            case 0x20F: survey(); break;
            case 0x210: {
                uint32_t prefs = GameDataStateStorage.fields_.game_more_preferences_;
                if ((prefs & 0x1000) == 0) {
                    if ((prefs & 0x2000) == 0) prefs |= 0x2000;
                    else prefs |= 0x1000;
                } else prefs &= ~0x3000u;
                GameDataStateStorage.fields_.game_more_preferences_ = prefs;
                AlphaIniPrefs.more_preferences = prefs;
                prefs_save(0); ::draw_map(1); break;
            }
            case 0x211: name_landmark(-1, -1); break;
            case 0x212: erase_landmark(-1, -1); break;
            case 0x213: go_to(-1, -2, 0); break;
            case 0x214: {
                const int index = ConsoleGlobal.cursor_history_index_;
                MapWinTable[0]->set_center(ConsoleGlobal.cursor_history_x_[index],
                    ConsoleGlobal.cursor_history_y_[index], 1);
                break;
            }
            case 0x215: {
                const uint32_t base_flag = MapWinTable[0]->field_1DD70_ & 0x400000;
                for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
                    MapWin *const window = MapWinTable[slot];
                    if (window == nullptr ||
                        (slot != 0 && window->field_1DD74_ == 0)) continue;
                    if (base_flag == 0) {
                        window->field_1DD70_ |= 0x400000;
                        window->draw_base_dest(1);
                    } else {
                        window->field_1DD70_ &= ~0x400000u;
                        window->draw_map(1);
                    }
                }
                break;
            }
            case 0x216:
                field_23BF8_ = 0;
                if (survey_mode_ != 0) { survey_mode_ = 0; MapWinTable[0]->draw_map(1); }
                MapWinTable[0]->draw_cursor();
                if (ConsoleGlobal.cursor_active_ != 0) reset_turn_view();
                if (--cursor_history_index_ < 0) cursor_history_index_ = 0x1F;
                {
                    const int history = ConsoleGlobal.cursor_history_index_;
                    const int x = ConsoleGlobal.cursor_history_x_[history];
                    const int y = ConsoleGlobal.cursor_history_y_[history];
                    ConsoleStatusWin.set_loc(x, y);
                    MapWinTable[0]->focus(x, y);
                }
                break;
            case 0x217: next_cursor(); break;
            case 0x300: case 0x301: new_base(selected_vehicle_); break;
            case 0x302: oblit(selected_vehicle_); break;
            case 0x304: {
                const Veh &vehicle = Vehs[selected_vehicle_];
                const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
                const bool air_combat =
                    Chassis[prototype.chassis_id].triad == TRIAD_AIR &&
                    Weapon[prototype.weapon_id].mode != 0;
                automate(selected_vehicle_, air_combat ? 0xC : 3);
                break;
            }
            case 0x305: arty(selected_vehicle_); break;
            case 0x306: automate(selected_vehicle_, 10); break;
            case 0x307: automate(selected_vehicle_, 0xB); break;
            case 0x308: convoy(selected_vehicle_); break;
            case 0x309: case 0x30A: use_ability(selected_vehicle_); break;
            case 0x310: go_to(selected_vehicle_, -3, 0); break;
            case 0x311: destroy(selected_vehicle_); break;
            case 0x312:
                if (ConsoleGlobal.cursor_active_ == 0)
                    disband(ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
                           ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_]);
                else disband(selected_vehicle_);
                break;
            case 0x313:
                if (edit_lock() == 0)
                    activate(ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
                             ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], 1);
                else NetMsgPrimary.pop("PROTECTED", 5000, 0, nullptr);
                break;
            case 0x314: automate(selected_vehicle_, 0); break;
            case 0x315: hold(selected_vehicle_, 0); break;
            case 0x316: case 0x317: go_home(selected_vehicle_); break;
            case 0x318: go_to(selected_vehicle_, 0x18, 0); break;
            case 0x319: go_to(selected_vehicle_, 0x18, 1); break;
            case 0x31A: assemble(); break;
            case 0x31B: explore(selected_vehicle_); break;
            case 0x31C: wait(selected_vehicle_); break;
            case 0x31D: unload(selected_vehicle_); break;
            case 0x31E: upgrade(selected_vehicle_); break;
            case 0x31F: give_veh(selected_vehicle_); break;
            case 0x320: designate(selected_vehicle_); break;
            case 0x321: sentry(selected_vehicle_, 0); break;
            case 0x322: sentry(selected_vehicle_, 10); break;
            case 0x323: skip(selected_vehicle_); break;
            }
        } else {
            switch (command) {
            case 0x324: set_patrol(selected_vehicle_); break;
            case 0x600: editor_on(); break;
            case 0x701: help_topic(2, 0); break;
            case 0x702: help_topic(0, 0); break;
            case 0x703: help_topic(1, 0); break;
            case 0x704: help_topic(0xE, 0); break;
            case 0x705: help_topic(4, 0); break;
            case 0x706: help_topic(6, 0); break;
            case 0x707: help_topic(7, 0); break;
            case 0x708: help_topic(5, 0); break;
            case 0x709: help_topic(8, 0); break;
            case 0x70A: help_topic(3, 0); break;
            case 0x70B: help_topic(10, 0); break;
            case 0x70C: help_topic(0xB, 0); break;
            case 0x70D: help_topic(0xC, 0); break;
            case 0x70E: help_topic(0xF, 0); break;
            case 0x720: TutWinGlobal->set_tour_stage(0); TutWinGlobal->tour(); break;
            case 0x721: TutWinGlobal->set_tour_stage(3); TutWinGlobal->tour(); break;
            case 0x722: TutWinGlobal->set_tour_stage(1); TutWinGlobal->tour(); break;
            case 0x723: TutWinGlobal->set_tour_stage(5); TutWinGlobal->tour(); break;
            case 0x724: TutWinGlobal->set_tour_stage(7); TutWinGlobal->tour(); break;
            case 0x725: TutWinGlobal->set_tour_stage(6); TutWinGlobal->tour(); break;
            case 0x750: show_credits(); break;
            case 0x751:
                parse_says(0, "RELEASE 2.0", -1, -1);
                parse_says(1, "20-Dec-1999", -1, -1);
                popp(ScriptPopupFile, "VERSIONCONTROLX",
                     0, "ac_logo_sm.pcx", 0);
                break;
            case 0x800: chat(0); break;
            case 0x801: X_pop("VOICEOVERDATA", &pop_wait); break;
            case 0x802: MultiWindow.on_button_clicked(2); break;
            case 0x803: custom_my_faction(); break;
            case 0x804: time_controls(); break;
            case 0x805: zoom_to_upkeep_base(); break;
            case 0x806: use_time_bonus(); break;
            case 0x807: case 0x808: end_my_turn(); break;
            case 0x9F0: road_to(selected_vehicle_, 0x1B); break;
            case 0x9F1: road_to(selected_vehicle_, 0x1C); break;
            case 0x9F2: automate(selected_vehicle_, 0); break;
            case 0x9F3: automate(selected_vehicle_, 1); break;
            case 0x9F4: automate(selected_vehicle_, 2); break;
            case 0x9F5: automate(selected_vehicle_, 7); break;
            case 0x9F6: automate(selected_vehicle_, 6); break;
            case 0x9F7: automate(selected_vehicle_, 3); break;
            case 0x9F8: automate(selected_vehicle_, 4); break;
            case 0x9F9: automate(selected_vehicle_, 5); break;
            }
        }
    } else if (command == 0x103) {
        design_new_veh(ConsoleGlobal.faction_id_, -1);
    } else {
        switch (command) {
        case 0x51: GameDataStateStorage.fields_.game_more_preferences_ |= 0x200000; alpha_menu_.check(selected_vehicle_); X_pop("MENUDETAIL", 0); break;
        case 0x52: GameDataStateStorage.fields_.game_more_preferences_ &= ~0x200000u; alpha_menu_.check(selected_vehicle_); X_pop("MENUSIMPLE", 0); break;
        case 0x53: set_preferences(); break;
        case 0x54: set_base_preferences(); break;
        case 0x55: set_adv_preferences(); break;
        case 0x56: set_auto_preferences(); break;
        case 0x57: set_audiovisual(); break;
        case 0x58: set_map_display(); break;
        case 0x59: save_game(0); break;
        case 0x5A: load_proc(0, 0); break;
        case 0x5B: retire_game(); break;
        case 0x5C: case 0x5D: {
            if (command == 0x5C && IsMultiplayerNet != 0) break;
            const char *const confirmation =
                IsMultiplayerNet == 0 && IsMultiplayerPBEM == 0
                    ? "REALLYQUIT"
                    : (bit_count(static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) > 1
                        ? "REALLYWIMPOUT"
                        : "REALLYQUIT");
            if (X_pop(confirmation, 0) != 0) {
                if (IsMultiplayerPBEM != 0) {
                    GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(1u << (ConsoleGlobal.faction_id_ & 0x1F));
                    save_game(0);
                }
                if ((GameDataStateStorage.fields_.game_rules_ & 0x400) != 0 && IsMultiplayerPBEM == 0) auto_save();
                if (IsMultiplayerNet != 0) {
                    NetDaemonNet.hang_up();
                    NetDaemonNet.reset();
                    IsMultiplayerNet = 0;
                }
                ExitTurnLoop = 1;
                log_say("Quit***********************************", 0, 0, 0);
            }
            if (command == 0x5C && ExitTurnLoop != 0) GameRestartQueued = 1;
            break;
        }
        }
    }

    if ((GameDataStateStorage.fields_.game_state_ & 0x40) == 0) return;
    if (command > 0xA00) {
        switch (command) {
        case 0xA01: editor_load(); return;
        case 0xA10: editor_paints(0); return;
        case 0xA11: editor_paints(1); return;
        case 0xA12: editor_paints(2); return;
        case 0xA13: editor_paints(3); return;
        case 0xA14: editor_paints(4); return;
        case 0xA15: editor_terrain(); return;
        case 0xA16: editor_paints(5); return;
        case 0xA17: editor_natural(1); return;
        case 0xA18: editor_natural(0); return;
        case 0xA19: editor_minor(); return;
        case 0xA20: MapWinTable[0]->editor(ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_], ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], 0); return;
        case 0xA21: MapWinTable[0]->editor(ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_], ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], 1); return;
        case 0xA24: editor_set_brush(); return;
        case 0xA30: editor_climate(); return;
        case 0xA31: editor_generate(); return;
        case 0xA32: editor_fast(); return;
        case 0xA36: editor_fungus(); return;
        case 0xA37: editor_rockiness(); return;
        case 0xA40: editor_randomize(); return;
        case 0xA41: editor_clear_random(); return;
        case 0xA48: editor_flat(); return;
        case 0xA50: editor_clear(); return;
        case 0xA51: editor_polar(); return;
        case 0xA52: editor_clear_vehicles(); return;
        case 0xA53: editor_clear_terrain(); return;
        case 0xA54: editor_lock2(); return;
        default: return;
        }
    }
    if (command == 0xA00) { editor_save(); return; }

    switch (command) {
    case 0x601:
        GameDataStateStorage.fields_.game_state_ ^= 0x80;
        if ((GameDataStateStorage.fields_.game_state_ & 0x80) == 0 || (GameDataStateStorage.fields_.game_state_ & 0x40) == 0)
            MainInterfaceGlobal.flat_button_[2].hide();
        else
            MainInterfaceGlobal.flat_button_[2].show(0);
        ::draw_map(1);
        WorldClimateWorldWin.GraphicWin::redraw();
        desktop_update();
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        MapWinTable[0]->main_caption();
        return;
    case 0x602: editor_veh(); return;
    case 0x603: editor_edit_veh(-1); return;
    case 0x604: tech_advance(ConsoleGlobal.faction_id_); return;
    case 0x605: editor_tech(); return;
    case 0x606: editor_view(); return;
    case 0x607: editor_diff(); return;
    case 0x608: editor_energy(); return;
    case 0x609: editor_date(); return;
    case 0x60A: editor_eliminate(); return;
    case 0x60B: editor_reload(); return;
    case 0x60C: editor_kill_vehicles(); return;
    case 0x60D: show_replay(); return;
    case 0x60E: show_movies(); return;
    case 0x60F: editor_reset_tech(); return;
    case 0x610: editor_reset_faction(); return;
    case 0x611: editor_diplomacy(); return;
    case 0x612: editor_personality(); return;
    case 0x613: editor_strategy(); return;
    case 0x614: editor_rules(); return;
    case 0x615: editor_scen_rules(); return;
    case 0x616: editor_scen_param(); return;
    case 0x617: editor_scen_victory(); return;
    case 0x618: editor_undo(); return;
    case 0x619: editor_redo(); return;
    case 0x61A: load_proc(1, 0); return;
    case 0x61B: save_game(1); return;
    default: return;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0051D720 ?menu_daemon@Console@@QAAXH@Z 0x0051D720-0x0051D733 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0051d720.cpp"], "reason": "Recover the callback as static cdecl: original forwards to ConsoleGlobal without reading an incoming receiver; retain catalogue identity separately from emitted symbol."}
// symbol    ?menu_daemon@Console@@SAXH@Z
void __cdecl Console::menu_daemon(int selection) {
    ConsoleGlobal.menu_proc(selection);
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E09B0 ?editor_kill_vehicles@@QAEXXZ 0x004E09B0-0x004E09D9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004e09b0.cpp"], "reason": "Recover editor undo and stack deletion through actual Console cursor history and existing stack_kill owner."}
// symbol    ?editor_kill_vehicles@Console@@QAEXXZ
void Console::editor_kill_vehicles() {
    auto_undo();
    const int index = ConsoleGlobal.cursor_history_index_;
    stack_kill(veh_at(ConsoleGlobal.cursor_history_x_[index],
                      ConsoleGlobal.cursor_history_y_[index]));
}

// Initialize the Console windows/menu and publish the fallback popup parent.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0051D890 ?init@Console@@QAEXH@Z 0x0051D890-0x0051D9CF
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0051d890.cpp"], "reason": "Complete genuine Console initialization/shared geometry from original receiver/member/global dataflow; native seed establishes catalogue identity and span. Source uses actual GraphicWin virtual base, AlphaMenu, shared RECT and typed fallback Win pointer. Original seed/claims retained; binding grants no matching or acceptance."}
// symbol    ?init@Console@@QAEXH@Z
void Console::init(int) {
    close();

    focus_vehicle_ = -1;
    field_23BEC_ = 0;

    if ((StatusFlags & 1u) == 0) {
        field_23BCC_ = WinScreenWidth;
        IfaceScreenWidth = WinScreenWidth;
        field_23BD0_ = WinScreenHeight - MainInterfaceGlobal.panel_height();
        IfaceScreenHeight = field_23BD0_;

        ConsoleMenuBounds.left = 0;
        ConsoleMenuBounds.top = 0;
        ConsoleMenuBounds.right = IfaceScreenWidth;
        ConsoleMenuBounds.bottom = IfaceScreenHeight;
        StatusFlags |= 1u;
    }

    char *const menu_caption = label_get(0x70);
    GraphicWin *const console_window = static_cast<GraphicWin *>(this);
    console_window->GraphicWin::init(
        &ConsoleMenuBounds, menu_caption, 0x02400000, 0,
        &alpha_menu_, 0);

    graphic_win_.init(0, 0, console_window->pixel_width(),
                      console_window->pixel_height(), 0, 0x01000022,
                      static_cast<Win *>(this), 0, 0);
    Win::set_def_focus(static_cast<Win *>(this));

    (void)alpha_menu_.menu_init();
    alpha_menu_.set_menu_proc(&Console::menu_daemon);
    load_cursors();

    BasePopDefaultField322C = static_cast<Win *>(this);
}

// Publish current screen geometry independently of init's once-only gate.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0051D760 sub_51d760 0x0051D760-0x0051D7B5
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0051d760.cpp"], "reason": "Complete genuine Console initialization/shared geometry from original receiver/member/global dataflow; native seed establishes catalogue identity and span. Source uses actual GraphicWin virtual base, AlphaMenu, shared RECT and typed fallback Win pointer. Original seed/claims retained; binding grants no matching or acceptance."}
// symbol    ?set_screen_geometry@Console@@QAEXXZ
void Console::set_screen_geometry() {
    const int width = WinScreenWidth;
    field_23BCC_ = width;
    IfaceScreenWidth = width;

    const int height = WinScreenHeight - MainInterfaceGlobal.panel_height();
    field_23BD0_ = height;
    IfaceScreenHeight = height;

    ConsoleMenuBounds.left = 0;
    ConsoleMenuBounds.top = 0;
    ConsoleMenuBounds.right = IfaceScreenWidth;
    ConsoleMenuBounds.bottom = height;
    StatusFlags |= 1u;
}

// ORIGINAL: 0x0048D0D0 ?veh_pick@@YAHHPBDH@Z 0x0048D0D0-0x0048D4EA;0x006585CB-0x006586B3
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x48d0d0 hot and cold spans; private full-body native-order review and actual-source callback/lifetime controls.", "git:decce96da7981b60ac91563a8323362cdab2e250:src/unrecovered/0048d0d0.cpp", "source:src/basepop.h:typed SpriteBox click callback at BasePop+3160; native CodeView and producer/consumer evidence."], "reason": "Recover the complete picker/supply operation through genuine Popup, Sprite, Veh, Base and network owners; preserve full original spans and seed findings without a new matching claim."}
// TRIED: declaring BOTH `Popup();` (matched: this disassembly calls Popup's own ctor at 0x4048a0, not BasePop+Scroll separately) AND `~Popup();` (declaration only, no body) on the Popup class lets `Popup pop;` be a real local without the compiler synthesising-and-EMITTING its own ~Popup() - the destructor declaration alone suppresses synthesis, so scope-exit just calls the external symbol once. This compiles clean (unlike 0x00405140, which hit the one-external-symbol error from an implicit ~Popup()). The extensive widget teardown after the `if (a3 != 0)` branch (FlatButton/BaseButton/Dialogs/Dialog/ Spot/Heap) is called through small raw-byte-buffer casts rather than real locals, since those types have no declared constructors in this scaffold and a real local would risk the same implicit-synthesis problem. Reaches MISMATCH #11 ('push' vs 'lea'); did not chase the exact field-store sequence (callback pointers, vtable resets) further.
int __cdecl veh_pick(int veh_id, const char *label, int flags) {
    // Native constructs Popup first at 0x48D0FB, then Sprite at 0x48D108.
    Popup popup;
    Sprite sprite;
    sprite.create_blank(0x64, 0x4B, WinFillColour);
    const int popup_flags = (flags != 0 ? 0x40 : 0) | 0x800;
    popup.start(PopupStartCaption, label, -1, 0, popup_flags, 0);

    int listed_count = 0;
    int listed_vehicle = veh_id;
    if (listed_vehicle >= 0) {
        int previous_vehicle = Vehs[listed_vehicle].prev_veh_id_stack;
        while (previous_vehicle >= 0) {
            listed_vehicle = previous_vehicle;
            previous_vehicle = Vehs[listed_vehicle].prev_veh_id_stack;
        }

        if (listed_vehicle >= 0) {
            do {
                say_morale(listed_vehicle, 0);
                StringTemp[0] = '\0';
                ::strcat(StringTemp, " ");
                const int prototype = Vehs[listed_vehicle].proto_id;
                ::strcat(StringTemp, VehPrototypes[prototype].veh_name);
                popup.dialogs().SpriteBox::sprite(
                    &sprite, StringTemp, listed_vehicle);

                // Native fetches the next stack link after the sprite call.
                const int next_vehicle = Vehs[listed_vehicle].next_veh_id_stack;
                ++listed_count;
                listed_vehicle = next_vehicle;
            } while (listed_vehicle >= 0);
        }
    }

    if (listed_count == 0) {
        // Native empty path closes the independent Sprite before Popup.
        sprite.close();
        popup.close();
        return -1;
    }

    popup.set_redraw_callback(veh_sprite_draw);
    popup.dialogs().SpriteBox::set_redraw_callback(veh_sprite_draw);
    popup.set_sprite_click_callback(veh_sprite_click);
    popup.set_sprite_double_click_callback(veh_sprite_double_click);

    PopupModalState = 6;
    const int result = popup.exec(0, 0);
    PopupModalState = 0;

    // Native successful path closes Popup before the local Sprite.
    popup.close();
    sprite.close();
    return result;
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
    if (X_text_open(ClimatePopupTitle[0], "SUPPLYMENU") != 0) {
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
        parse_say(0, reinterpret_cast<int>(Facility[facility_id].name), -1, -1);
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

    uint32_t &supply_action_flags = NetDaemonNet.lock_state_[1].leading_words_[3];
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

// The C++ compiler owns the scalar deleting destructor below. Do not
// add a hand-written deleting wrapper or explicit destructor cascade.
// Current source ownership is not original TU or COMDAT-selection proof.
// ORIGINAL: 0x0051DDF0 ??_GConsole@@UAEPAXI@Z 0x0051DDF0-0x0051DE26 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/0051ddf0.cpp", "task:window-generated-deletion-owners", "Pinned entry receives the Console virtual GraphicWin subobject at complete-object+0x23D94; own-part destructor, virtual-base destructor and conditional delete are separate native stages.", "Fresh emitted code and receiver/target controls remain required. Duplicate COMDAT emission does not establish historical translation-unit placement or retained link selection."], "reason": "The existing real class lifetime generates this deleting entry; bind compiler-owned code rather than transcribing receiver arithmetic or invoking member/base destructors twice."}

// Provisional original storage identities: access widths and initial values
// are observed, but no complete allocation/lifetime interpretation is claimed.
// Six individually stored bytes at 0x517930-0x51795D initially read as zero. Only the touched cells are known; original allocation extent, terminator and other consumers remain unverified.
// RECOVERY_OBJECT: v1 0x00939FE0 ?ConsoleKeyByteRegion@@3PAEA
unsigned char ConsoleKeyByteRegion[6] = {0};

// Interlude selection dword read/written at 0x518871/0x5188AB, initially zero. Its complete original allocation and lifetime remain unverified.
// RECOVERY_OBJECT: v1 0x00939FCC ?ConsoleInterludeSelection@@3HA
int ConsoleInterludeSelection = 0;

// Key-cycle dword read/written at 0x5185CE-0x518674, initially -2 in the pinned PE. Adjacent DiploKnownTechColor is independent; full allocation and lifetime remain unverified.
// RECOVERY_OBJECT: v1 0x0068A5C8 ?ConsoleKeyCycleState@@3HA
int ConsoleKeyCycleState = -2;

// ORIGINAL: 0x005178C0 ?on_key_click@Console@@QAEHHH@Z 0x005178C0-0x0051B66F;0x0065D4D0-0x0065D4E2
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 0x005178C0-0x0051B66F and EH span 0x0065D4D0-0x0065D4E2; receiver is the Console virtual GraphicWin subobject.", "Independent native entry/cursor/modal review: zero-tested mode-8 member, false-visible button handoff, normalized versus raw dispatch and unconditional confirmed single-player quit.", "Current VC6 emits the UAE virtual member and its own vtordisp thunk; retain the legacy QAE interpreted original name, not a hand-coded adjustment.", "MainInterfaceGlobal original7AE820; header button_group_[4] begins1E878 with stride94. Element3 is7CD254, its selection field84 is7CD2D8, and the following MainInterface field1EAC8 is7CD2E8.", "Pinned MainInterface constructor45EF10 constructs four ButtonGroups at+1E878 with ctor62B7C0 and dtor4325B0; destructor45EB80 tears down that same owned array. Noncompact init populates group3 with brush buttons1036..1041.", "Pinned MainInterface init4596CD clears its+1EAC8 member; Console key517A0B/517A93 reads that same original cell. The former ConsoleModalState7CD2E8 symbol retains its unbounded identity as a member binding next to MainInterfaceGlobal."], "reason": "Recover the complete Console key/event dispatcher in its genuine virtual MapWin/GraphicWin owner, with ordinary scoped Log lifetime and typed map, player, vehicle, modal-window and editor operations. Read the input gate from its genuine MainInterface member, not a duplicated standalone Console global; preserve independent reads, short-circuit order and clear_message behavior."}
// kind      game
// symbol    ?on_key_click@Console@@UAEHHH@Z
int Console::on_key_click(int a1, int a2) {

// ---- flattened from fragments/01-first-command-and-character-switches.inc ----
// Native body prelude and digit fallback, pinned offsets 0x5178C0..0x51803E.
// This section declares the method-local Log/latches, applies the first modal
// input router and joins the first byte dispatch in fragment 14.

// Common per-entry native return value is 1; every branch reaches the one
// cleanup edge at 0x51B648. `a1` is the character/key value at [ebp+8], and
// `a2` is the event command at [ebp+0xC]. Native 0x6466EF is a locale-aware
// lowercase conversion and is represented by `tolower(a1)`. Its keyboard
// trace call at 0x626350 is the source-owned log_say_hex overload.

int normalized_character;
Log native_handler_log; // native entry constructs at 0x625FB0; epilogue destroys it
char native_key_character[2];
char native_decimal_buffer[16];
int native_editor_lock;
int native_command_consumed = 0; // native stack local [ebp-0x28]
int native_original_key;
int native_mode_flag;

// Native entry guards 0x5178F3..0x517910 run after the local Log has been
// constructed and before the first sparse event table. The command mask is
// applied to the original a2 dword, not to its low byte.
if (ExitTurnLoop != 0 ||
    (static_cast<unsigned int>(a2) & 0xFFFCFFFFu) == 0x12u) {
    goto native_done;
}

// The first sparse event switch (0x51B690; a2 base 0x30041, 58 bytes) has
// six individual byte writes and the ConsoleTimer command. The neutral
// ConsoleKeyByteRegion owner is provisional as described beside its definition;
// its six byte cells cover observed stores only and assert no original
// allocation capacity or terminator.
switch (a2) {
case 0x30050: ConsoleKeyByteRegion[0] = 'p'; break;
case 0x30049: ConsoleKeyByteRegion[1] = 'i'; break;
case 0x30052: ConsoleKeyByteRegion[2] = 'r'; break;
case 0x30041: ConsoleKeyByteRegion[3] = 'a'; break;
case 0x30054: ConsoleKeyByteRegion[4] = 't'; break;
case 0x30045: ConsoleKeyByteRegion[5] = 'e'; break;
case 0x3007A:
    // Native reads ConsoleTimer.id_event_ at +4 (not count_), stops an active
    // timer and stores 0x10 to LockEnableMask's low byte, or starts the
    // callback with (param=0,count=100,resolution=5) and clears that byte.
    if (ConsoleTimer.has_timer()) {
        ConsoleTimer.stop();
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFFFFF00u) | 0x10u;
    } else {
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= 0xFFFFFF00u;
        ConsoleTimer.start(auto_play_callback, 0, 100, 5);
    }
    break;
default:
    break;
}

// After that event switch, native calls AlphaMenu::hide at 0x9380DC. This is
// the already modeled ConsoleGlobal.alpha_menu_ member (Console +0x22A2C).
// Then it tests MapBaseDisplayOverride and either sets
// ConsoleNoMapOverrideLatch or enters the later command router.

ConsoleGlobal.alpha_menu_.hide();
if (MapBaseDisplayOverride == 0) {
    ConsoleNoMapOverrideLatch = 1;
    goto native_done;
}

if (TutorialWindow.reset() != 0 && (a2 == 0x0D || a2 == 0x1B)) {
    goto native_done;
}
if (a2 == 0x91) {
    MainInterfaceGlobal.draw_edit_pane();
}

// Native Enter/Space panel handoff (0x5179F0..0x517AB7). Both the gate word
// and ButtonGroup belong to the live MainInterface object.
{
    bool enter_space_handoff = false;
    if (a2 == 0x0D || a1 == 0x20) {
        if (NetMsgPrimary.is_active() == 0 &&
            NetMsgSecondary.is_active() == 0 &&
            MainInterfaceGlobal.input_gate_state() == 0 &&
            MainInterfaceGlobal.button_group(0).field_84_ == 0x3E8 &&
            TutWinBaseWindow.is_visible() == 0) {
            enter_space_handoff = true;
        }
    }
    if (enter_space_handoff == false && a2 == 0x0D &&
        (ConsoleGlobal.alpha_menu_.is_visible() != 0 || MultiWindowOpen != 0)) {
        enter_space_handoff = true;
    }
    if (enter_space_handoff != false) {
        if (TutWinBaseWindow.is_visible() == 0) {
            MainInterfaceGlobal.button_group(0).button_click(0x3E9);
        }
        if (ConsoleGlobal.alpha_menu_.is_visible() != 0) {
            ConsoleGlobal.alpha_menu_.hide();
        }
        if (MultiWindowOpen != 0) {
            MultiWindow.hide();
        }
        if (MainInterfaceGlobal.input_gate_state() != 0) {
            MainInterfaceGlobal.clear_message();
        }
        if (NetMsgPrimary.is_active() != 0 ||
            NetMsgSecondary.is_active() != 0) {
            goto native_done;
        }
    }
}
ConsoleInfoWin.reset();
ConsoleStatusWin.reset();

if (a2 >= 0x60 && a2 <= 0x69) {
    a1 = 0;
}
if (a2 == 0x0D && (GameDataStateStorage.fields_.game_state_ & 0x80u) != 0 &&
    (MapWinTable[0]->field_1DD70_ & 0x4000u) != 0) {
    const int history = ConsoleGlobal.cursor_history_index_;
    int vehicle_id = veh_at(ConsoleGlobal.cursor_history_x_[history],
                            ConsoleGlobal.cursor_history_y_[history]);
    while (vehicle_id >= 0) {
        const Veh &vehicle = Vehs[vehicle_id];
        if (vehicle.order == 0x18) {
            Paths.move(vehicle_id, vehicle.faction_id);
            do_all_draws();
            goto native_done;
        }
        if (vehicle.next_veh_id_stack < 0) {
            do_all_draws();
            goto native_done;
        }
        vehicle_id = vehicle.next_veh_id_stack;
    }
    do_all_draws();
    goto native_done;
}

// Native 0x517B81..A2 normalizes the key and logs the raw key/event pair.
normalized_character = tolower(a1);
native_key_character[0] = static_cast<char>(a1);
native_key_character[1] = '\0';
log_say_hex("Key: ", native_key_character, a1, a2, 0);

// Native 0x517BA7..0x517BDF derives [ebp-0x2C] only for network games. A
// nonzero ConsoleControlTurnActive (0x93A938) sets the local latch and takes the immediate
// cleanup edge. With it clear, either nonzero lock-state sentinel sets the
// latch but continues; only when both are zero does the latch remain clear.
native_editor_lock = 0;
if (IsMultiplayerNet != 0) {
    if (ConsoleControlTurnActive != 0) {
        native_editor_lock = 1;
        goto native_done;
    } else if (NetDaemonNet.lock_state_[2].payload_.flags != 0 ||
               NetDaemonNet.lock_state_[0].leading_words_[3] != 0) {
        native_editor_lock = 1;
    }
}

// ---- flattened from fragments/00-map-cursor-pre-modal-corridor.inc ----
// Native 0x517BDF..0x517FB7. ESI is the Console virtual GraphicWin receiver;
// all accesses below are expressed through its real MapWin/Console/Veh owners.
// Native slot17 is MapWin::on_mouse_move; the earlier 0x5EC8A0 call is the
// inherited Win::get_mouse_pos operation.
{
    const uint32_t map_cursor_mode = field_23D10_;
    const int selected_map_slot = MapWinSelectedSlot;
    const int selected_vehicle_id = selected_vehicle_;
    if (map_cursor_mode == 1 && selected_map_slot >= 0 &&
        selected_vehicle_id >= 0 && ConsoleGlobal.cursor_active_ != 0) {
        MapWin &selected_map = *MapWinTable[selected_map_slot];
        if (normalized_character == 'j') {
            selected_map.field_1DD84_ =
                selected_map.field_1DD84_ == 0 ? 1u : 0u;
            int mouse_x = 0;
            int mouse_y = 0;
            selected_map.get_mouse_pos(&mouse_x, &mouse_y);
            selected_map.on_mouse_move(mouse_x, mouse_y);
            goto native_done;
        }
        if (normalized_character == 'g' || a1 == 0x20) {
            Veh &selected_vehicle = Vehs[selected_vehicle_id];
            if ((selected_vehicle.state & 0x01000200u) == 0x01000200u) {
                if (selected_vehicle.waypoint_count >= 3) {
                    selected_vehicle.waypoint_count = 0;
                } else {
                    ++selected_vehicle.waypoint_count;
                }
            } else if ((selected_vehicle.state & 0x02000000u) == 0) {
                selected_vehicle.state |= 0x02000000u;
                selected_vehicle.patrol_current_point = 0;
                selected_vehicle.waypoint_count = 1;
            } else {
                ++selected_vehicle.waypoint_count;
                if (selected_vehicle.waypoint_count >= 4) {
                    selected_vehicle.state &= ~0x02000000u;
                    selected_vehicle.waypoint_count = 0;
                }
            }
            int mouse_x = 0;
            int mouse_y = 0;
            selected_map.get_mouse_pos(&mouse_x, &mouse_y);
            selected_map.feedback_hover_y_ = -1;
            selected_map.field_1DE0C_ = static_cast<uint32_t>(-1);
            selected_map.on_mouse_move(mouse_x, mouse_y);
            goto native_done;
        }

        Veh &selected_vehicle = Vehs[selected_vehicle_id];
        selected_vehicle.state &= ~0x02000200u;
        selected_vehicle.order = -1;
    }

    // Map-mode 8 updates the active primary window's route-point counter.
    // ESI is Console+0x23D94 here; its -0x6014 read rebases to the existing
    // MapWin::field_1DD80_ at front offset 0x1DD80.
    if (field_23D10_ == 8 && field_1DD80_ != 0 &&
        selected_vehicle_ >= 0 &&
        ConsoleGlobal.cursor_active_ != 0 &&
        (normalized_character == ' ' || normalized_character == 'g')) {
        Veh &selected_vehicle = Vehs[selected_vehicle_];
        if (selected_vehicle.waypoint_count >= 3) {
            selected_vehicle.waypoint_count = 0;
        } else {
            ++selected_vehicle.waypoint_count;
        }
        MapWin &primary_map = *MapWinTable[0];
        int mouse_x = 0;
        int mouse_y = 0;
        primary_map.get_mouse_pos(&mouse_x, &mouse_y);
        primary_map.field_1DE0C_ = static_cast<uint32_t>(-1);
        primary_map.on_mouse_move(mouse_x, mouse_y);
        goto native_done;
    }

    // Pan commands in a flagged window consume the character and event before
    // the shared go_reset/cursor restoration path.
    for (size_t pan_window_slot = 0;
         pan_window_slot < MapWinTableSlots; ++pan_window_slot) {
        MapWin *const window = MapWinTable[pan_window_slot];
        if (window == 0 ||
            (pan_window_slot != 0 && window->field_1DD74_ == 0)) {
            continue;
        }
        if ((window->field_1DD70_ & 0x00400000u) != 0 &&
            (normalized_character == 'g' || normalized_character == 'p')) {
            a2 = 0;
            a1 = 0;
            normalized_character = 0;
        }
    }
    if (field_23D10_ != 0 &&
        (normalized_character == 'g' || normalized_character == 'p')) {
        a2 = 0;
        a1 = 0;
        normalized_character = 0;
    }

    go_reset();
    field_23D10_ = 0;
    for (size_t cursor_window_slot = 0;
         cursor_window_slot < MapWinTableSlots; ++cursor_window_slot) {
        MapWin *const window = MapWinTable[cursor_window_slot];
        if (window == 0 ||
            (cursor_window_slot != 0 && window->field_1DD74_ == 0)) {
            continue;
        }
        if (window->cursor_sprite_ != 0 ||
            window->cursor_resource_id_ != 0x7F00) {
            window->set_cursor(0x7F00);
            window->cursor_resource_id_ = 0x7F00;
            window->cursor_sprite_ = 0;
        }
        if ((window->field_1DD70_ & 0x00C40000u) != 0) {
            window->field_1DD70_ &= ~0x00C40000u;
            window->draw_map(1);
        }
        do_non_input();
    }

    if (a2 == 0x0D &&
        IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
        ReportIfGlobal.done();
        if (TutWinBaseWindow.is_visible() != 0) {
            TutWinBaseWindow.exit();
        }
        goto native_done;
    }
}

// Native 0x517FB8 tests TutWinBaseWindow::is_visible before the 0x51B6CC
// event table. The false edge targets 0x518179 and bypasses both the modal
// table and the numeric/raw-character handlers below.
if (TutWinBaseWindow.is_visible() == 0) {
    if (MainInterfaceGlobal.button_group(0).field_84_ == 0x3E8) {
        MainInterfaceGlobal.button_group(0).button_click(0x3E9);
    }
    goto native_after_basewindow_raw_map;
}

// Modal BaseWin event routing (native table 0x51B6CC, index a2-0x0D,
// count 0x5D), only after the native visible-window guard:
switch (a2) {
case 0x0D: // Enter
case 0x1B: // Escape
    TutWinBaseWindow.exit();
    goto native_done;
case 0x21: // Page Up
case 0x25: // Left
case 0x64: // keypad 4
case 0x69: // keypad 9
    TutWinBaseWindow.prev();
    goto native_done;
case 0x22: // Page Down
case 0x27: // Right
case 0x63: // keypad 3
case 0x66: // keypad 6
    TutWinBaseWindow.next();
    goto native_done;
default:
    break;
}

// Native numeric-key fallback after modal routing. The digit is the normalized
// character [ebp-0x24], not a2. Native guards are editor/network-sensitive:
// edit-lock state must be clear; in editor mode UnkGlobal0093A934 must be set;
// and NetDaemonNet's faction mask bit for ConsoleGlobal.faction_id_ must be
// clear. Once those have been established by the translated common prelude:
if (normalized_character >= '0' && normalized_character <= '9') {
    const int digit = normalized_character - '0';
    if (native_editor_lock != 0 &&
        (UnkGlobal0093A934 == 0 ||
         (NetDaemonNet.lock_state_[0].trailing_words_[8] &
          (1u << faction_id_)) != 0)) {
        goto native_done;
    }
    TutWinBaseWindow.production(digit, 0);
    goto native_done;
}


// ---- flattened from fragments/14-base-window-character-routes.inc ----
// INCOMPLETE C++ body fragment for the first normalized-character map
// (dispatch table at 0x51B73C, lookup bytes at 0x51B764; index is
// [ebp-0x24]-0x2B, where [ebp-0x24] is tolower(a1)). This precedes the
// separate raw-a1 0x51B7AC character table.
// Receiver 0x006A7628 is the source-owned TutWinBaseWindow object.

// The first raw-key map covers every input in [0x2B,0x72]. Its default table
// entry is not a fallthrough: native 0x518170 sets the command-consumed local
// and continues to the 0x2000D/0x20020 command tests, then the scenario gate.
if (normalized_character >= 0x2B && normalized_character <= 0x72) {
switch (normalized_character) {
case 'p':
case 'c':
    if (!native_editor_lock ||
        (UnkGlobal0093A934 != 0 &&
         (NetDaemonNet.lock_state_[0].trailing_words_[8] &
          (1u << faction_id_)) == 0)) {
        TutWinBaseWindow.production(0, 0);
    }
    goto native_done;
case 'h':
    if (!native_editor_lock) {
        TutWinBaseWindow.hurry();
    }
    goto native_done;
case ',':
case '<':
    TutWinBaseWindow.prev();
    goto native_done;
case '.':
case '>':
case 'n':
    TutWinBaseWindow.next();
    goto native_done;
case 'r':
    TutWinBaseWindow.rename();
    goto native_done;
case 'e':
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x40u) != 0) {
        TutWinBaseWindow.base_editor();
    }
    goto native_done;
case '-':
case '_':
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x40u) != 0) {
        TutWinBaseWindow.focus();
        if (BaseCurrent->population_size > 1) {
            --BaseCurrent->population_size;
            TutWinBaseWindow.recompute(0);
        }
    }
    native_command_consumed = 1;
    goto native_after_base_character;
case '+':
case '=':
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x40u) != 0) {
        TutWinBaseWindow.focus();
        if (BaseCurrent->population_size < 0x63) {
            ++BaseCurrent->population_size;
            TutWinBaseWindow.recompute(0);
        }
    }
    native_command_consumed = 1;
    goto native_after_base_character;
default:
    native_command_consumed = 1;
    goto native_after_base_character;
}
} else {
    // Native unsigned range failure at 0x518044 also falls to 0x518170.
    native_command_consumed = 1;
}

// Native false-visible edge enters 0x518179 here, after bypassing the raw
// BaseWin key map at 0x51803E..0x518170. Both edges share these command-ID
// tests and then the native command-consumed/secondary-character routing.
native_after_basewindow_raw_map:
native_after_base_character:
if (a2 == 0x2000D) {
    end_my_turn();
    goto native_done;
}
if (a2 == 0x20020) {
    use_time_bonus();
    goto native_done;
}
if (native_command_consumed != 0) {
    goto native_scenario_editor_gate;
}

// Evidence from the 0x51B764 map: normalized ','/'<' target 0x5180D3 prev;
// '.'/'>'/'n' target 0x5180E2 next; 'r' renames; normalized 'e' targets
// 0x518100 and preference-gates BaseWin::base_editor;
// 0x51811C/140 call BaseWin::focus, then modify BaseCurrent.population_size
// at Base+6 (bounds 1..99) and call BaseWin::recompute(0). The six mapped
// character groups above are mechanically derived from the PeImage table.
// Native 0x518059/080 are BaseWin::production(0,0), gated by the preserved
// editor latch, UnkGlobal0093A934 and lock_state_[0].trailing_words_[8];
// 0x5180BB is BaseWin::hurry(), with the native editor-latch guard.


// ---- flattened from fragments/01b-secondary-character-table.inc ----
// Native secondary character table 0x51B7AC, after the 0x51B764 BaseWin
// lookup and its command-consumed join.
switch (a1) {
case '+':
    custom_my_faction();
    goto native_done;

case ',':
    if (!native_editor_lock) {
        corner_the_market();
    }
    goto native_done;

case '=':
    if ((GameDataStateStorage.fields_.game_state_ & 0x10u) != 0 || ScenarioSave.objective_req_victory < 0x2328 ||
        ScenarioSave.objectives_sudden_death_victory < 0x2328) {
        show_scen_objectives();
    }
    goto native_done;

case 'D':
    if (!native_editor_lock) {
        if (ConsoleGlobal.cursor_active_ != 0 &&
            ConsoleGlobal.selected_vehicle_ >= 0) {
            disband(ConsoleGlobal.selected_vehicle_);
        } else {
            const int cursor = ConsoleGlobal.cursor_history_index_;
            disband(ConsoleGlobal.cursor_history_x_[cursor],
                    ConsoleGlobal.cursor_history_y_[cursor]);
        }
    }
    goto native_done;

case 'X': zoom(0x206, 0); go_reset(); goto native_done;
case 'Z': zoom(0x205, 0); go_reset(); goto native_done;
case 'x': zoom(0x204, 0); go_reset(); goto native_done;
case 'z': zoom(0x203, 0); go_reset(); goto native_done;

case 'c': {
    const int cursor = ConsoleGlobal.cursor_history_index_;
    MapWinTable[0]->set_center(ConsoleGlobal.cursor_history_x_[cursor],
                               ConsoleGlobal.cursor_history_y_[cursor], 1);
    goto native_done;
}
case 'e':
    if (!native_editor_lock) {
        social_select(ConsoleGlobal.faction_id_);
    }
    goto native_done;
case 't': survey(); goto native_done;
case 'u':
    if (!native_editor_lock) {
        design_new_veh(ConsoleGlobal.faction_id_, -1);
    }
    goto native_done;
default:
    break;
}


// ---- flattened from fragments/02-uppercase-and-scenario-switches.inc ----
// Native key-handler dispatch region.
// Depends on first-command fragment state and still omits the later command
// tables/shared tails; this is not an independent member definition.

// The native case-table 0x51B830 is keyed by a1 in [0x41,0x76]. It is only
// entered when Console::edit_lock() is false. Exact active character groups:
// A=0, P=1, T=2, '^'=3, J=4, j=5, R=6, v=7; all other table bytes route to
// the shared 0x5185B0 branch.
// Native first calls edit_lock(); when blocked, only `v` or `a` produces the
// synchronous PROTECTED notification (literal 0x68AAA4). It calls edit_lock()
// again before entering the key table, so preserve both reads around callback.
if (edit_lock() != 0 && (a1 == 'v' || a1 == 'a')) {
    NetMsgPrimary.pop("PROTECTED", 5000, 0, 0);
}

if (edit_lock() == 0) {
switch (a1) {
case 'A':
    if (!native_editor_lock) { // previously established native [ebp-0x2C]
        automate(selected_vehicle_, 0);
    }
    goto native_shared_after_key;
case 'P':
    if (IsMultiplayerNet) {
        MultiWindow.on_button_clicked(2);
    }
    goto native_shared_after_key;
case 'T':
    time_controls();
    goto native_shared_after_key;
case '^':
    use_time_bonus();
    goto native_shared_after_key;
case 'J':
    assemble();
    goto native_shared_after_key;
case 'j':
    go_to(selected_vehicle_, 0x18, 1);
    goto native_shared_after_key;
case 'R':
    {
    // The route is independently transcribed in the typed fragment; inline
    // it here during final assembly so all route outcomes share the common
    // post-key continuation.
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x200u) != 0) {
        const int faction = faction_id_;
        int *const toggles = PlayersData[faction].unk_38 + 2;
        PopupWriteCheckState = toggles[0] != 0 ? 1 : 0;
        if (toggles[1] != 0) {
            PopupWriteCheckState |= 2;
        } else {
            PopupWriteCheckState &= ~2;
        }
        popup_set_write_check_bit(2, toggles[2]);
        popup_set_write_check_bit(3, toggles[3]);
        if (x_pop_short(StrTechRandom, 1, 0) >= 0) {
            const char answer = static_cast<char>(PopupWriteCheckState);
            toggles[0] = answer & 1;
            toggles[1] = (answer & 2) >> 1;
            toggles[2] = (answer & 4) >> 2;
            toggles[3] = (answer & 8) >> 3;
            synch_ai(faction);
        }
    } else {
        social_select(faction_id_);
    }
    goto native_shared_after_key;
    }
case 'v':
    {
    if (!native_editor_lock) {
        if (ConsoleGlobal.cursor_active_ == 0) {
            set_move(0);
        } else {
            ambience(0x1E);
            field_23BF8_ = 1;
            field_23BFC_ = 1;
            const int had_survey = survey_mode_;
            survey_mode_ = 0;
            cursor_active_ = 0;
            if (had_survey != 0) {
                MapWinTable[0]->draw_map(1);
            }
            ::draw_cursor();
            ConsoleInfoWin.change(0);
            ConsoleStatusWin.redraw();
            MapWinTable[0]->main_caption();

            alpha_menu_.check(selected_vehicle_);
            const int cursor = ConsoleGlobal.cursor_history_index_;
            focus(ConsoleGlobal.cursor_history_x_[cursor],
                  ConsoleGlobal.cursor_history_y_[cursor], faction_id_);
        }
    }
    goto native_shared_after_key;
    }
default:
    break;
}
}

native_shared_after_key:
// Native 0x5185B0 handles the character produced by the normalization path.
if (normalized_character == ':' && ConsoleKeyCycleState == -1) {
    ConsoleKeyCycleState = 0;
}

if (normalized_character == '~' && (GameDataStateStorage.fields_.game_state_ & 0x60u) == 0) {
    int cycle = ConsoleKeyCycleState;
    if (cycle == -1) {
        if (!IsMultiplayerNet) {
            ++cycle;
        }
    } else if (cycle < 4) {
        ++cycle;
    }
    ConsoleKeyCycleState = cycle;

    const int threshold = IsMultiplayerNet ? 4 : 0;
    if (cycle >= threshold) {
        GameDataStateStorage.fields_.game_state_ ^= 0x1000u;
        if (IsMultiplayerNet && (GameDataStateStorage.fields_.game_state_ & 0x1000u) != 0 &&
            (GameDataStateStorage.fields_.game_state_ & 0x20u) == 0) {
            GameDataStateStorage.fields_.game_state_ |= 0xA0u;
        }

        set_dirty();
        draw_map(1);
        WorldClimateWorldWin.redraw();
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        MapWinTable[0]->main_caption();
    }
}

// When GameState has STATE_SCENARIO_EDITOR and the earlier event was not
// handled, the native 0x51B88C switch (input = normalized_character-0x21,
// count 0x59) has five active non-default entries. The `y` route's MainInterface button
// callback is typed in fragment 10; other active routes follow below.
if (normalized_character == 'y' &&
    (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) != 0 &&
    native_command_consumed == 0) {
    GameDataStateStorage.fields_.game_state_ ^= STATE_OMNISCIENT_VIEW;
    if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0 &&
        (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
        MainInterfaceGlobal.flat_button_[2].show(0);
    } else {
        MainInterfaceGlobal.flat_button_[2].hide();
    }
    set_dirty();
    ::draw_map(1);
    WorldClimateWorldWin.redraw();
    desktop_update();
    ConsoleInfoWin.change(0);
    ConsoleStatusWin.redraw();
    MapWinTable[0]->main_caption();
    goto native_editor_command_dispatch;
}

// Remaining active native entries (verified mechanically from
// byte_lookups[0x0051B8A4] in dispatch-map.json):
//   '@' / 0x40 -> 0x51872B random/retry loop; CRT rand plus do_task;
//   ';' / 0x3B -> 0x518863 scenario-mask/string helper, then interlude;
//   '&' / 0x26 -> 0x5188C7 vehicle/map bounds and linked-stack repair;
//   '!' / 0x21 -> 0x518BB1 checksum/base/report construction (not interlude);
//   'y' / 0x79 -> 0x5186AE omniscient toggle and flat_button_[2] show/hide;
//   default -> 0x519F9C command-number dispatch.
// The byte-index map is authoritative; in particular, the native interlude
// call is in `;` (0x3B), not `!` (0x21). Route fragments 09-12 contain the
// source-typed R/y/;/@ behavior; final assembly must preserve their exact
// incoming guards and shared continuation targets.


// ---- flattened from fragments/23-scenario-character-routes.inc ----
// Assembled 0x51B88C character-table routes. This file is included from the
// method body after fragment 02 and before command dispatch fragment 03.
// The table is reached only for the native scenario-editor state and only
// after native_command_consumed remains clear; keep each table target's own
// switch route and its continuation distinct.

if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) != 0 &&
    native_command_consumed == 0) {
    switch (normalized_character) {
    case '@':
        {

// ---- flattened from fragments/12-at-hotkey-random-pump.inc ----
// Native key-handler dispatch region.
// from 0x51B8A4 input 0x40/index 3 to target 0x51872B. This native route has
// no exit edge: it continually samples rand() and pumps the typed do_task()
// message loop. Counter locals whose increments never affect a branch are
// omitted; every controlling counter and every random/pump call is retained.

{
    int range = 0xA0;
    int passes = 1;
    int samples_per_round = 0x14;
    int successes_remaining = 0x1E;
    int widening_rounds = 4;
    int widened = 0;

    for (;;) {
        if (successes_remaining != 0 && samples_per_round != 0) {
            while (successes_remaining != 0 && samples_per_round != 0) {
                int range_sample = 0;
                if (range - 1 > 0) {
                    range_sample = rand() % range;
                }
                const int comparison_sample = rand() % 0x30;
                if (range_sample > comparison_sample) {
                    --samples_per_round;
                } else {
                    --successes_remaining;
                }
            }
        }

        if (successes_remaining == 0) {
            --passes;
            if (passes > 0) {
                successes_remaining = 0x1E;
                widened = 0;
                range = 0xA0;
                do_task();
                continue;
            }

            widening_rounds = 4;
            passes = 1;
            successes_remaining = 0x1E;
            samples_per_round = 0x14;
            widened = 0;
            range = 0xA0;
            do_task();
            continue;
        }

        if (range + 0x2F > 0) {
            const int widening_sample = rand() % (range + 0x30);
            if (widening_sample < 0x30 && widened == 0) {
                widened = 1;
                range += range / 2;
            }
        }

        --widening_rounds;
        if (widening_rounds > 0) {
            samples_per_round = 0x14;
            do_task();
            continue;
        }

        widening_rounds = 4;
        passes = 1;
        successes_remaining = 0x1E;
        samples_per_round = 0x14;
        widened = 0;
        range = 0xA0;
        do_task();
    }
}


        }
        break;
    case ';':

// ---- flattened from fragments/11-semicolon-interlude-prompt-owner-gap.inc ----
// Native key-handler dispatch region.
// from 0x51B8A4 input 0x3B/index 2 to native target 0x518863.
// Native guard is GameState bit 0x1000. The selector storage is a standalone
// zero-initialized dword touched only by this Console route in the pinned
// image. `ConsoleInterludeSelection` is a provisional source-owned name; the
// evidence proves a four-byte touched extent, not the complete original object
// extent or its original C++ spelling.

if ((GameDataStateStorage.fields_.game_state_ & 0x1000u) != 0) {
    // Native 0x518871 reads 0x939FCC as the current EDITINTERLUDE number,
    // and 0x5188AB stores the accepted result there.
    const int current_interlude_selection = ConsoleInterludeSelection;
    if (X_pop_ask_number("EDITINTERLUDE", current_interlude_selection,
                         0) == 0) {
        const int interlude_id = atoi(ParseStrBuffer[0].str);
        // After a successful popup the native write at 0x5188AB publishes
        // this value to the same selector cell.
        ConsoleInterludeSelection = interlude_id;
        const unsigned int bit =
            1u << (static_cast<unsigned int>(interlude_id) & 31u);
        GameDataStateStorage.fields_.game_interlude_flags_ &= ~bit;
        interlude(interlude_id, 0, 1, 0);
    }
}

// Continue to shared scenario-editor/gameplay dispatch.

        break;
    case '&':

// ---- flattened from fragments/13-ampersand-audit-route.inc ----
// Native key-handler dispatch region.
// The non-debug validator and stack-link repair paths below are translated
// from the native loop edges. The debug report's faction text is
// Players[faction].adj_name_faction: Players starts at 0x946A50, typed member
// offset +0x304, with Player's independently asserted native stride 0x59C.

if ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) == 0) {
    if (IsMultiplayerNet != 0) {
        goto native_editor_command_dispatch;
    }

    bool found_invalid_vehicle = false;
    for (int bounds_vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; bounds_vehicle_id >= 0;
         --bounds_vehicle_id) {
        const Veh &vehicle = Vehs[bounds_vehicle_id];
        if (vehicle.x < 0 || vehicle.x >= MapLongitudeBounds ||
            vehicle.y < 0 || vehicle.y >= MapLatitudeBounds) {
            veh_kill(bounds_vehicle_id);
            danger("Killed a baddy!", StringTable.get(0), 0, 0, 0);
            found_invalid_vehicle = true;
        }
    }

    for (int stack_vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; stack_vehicle_id >= 0;
         --stack_vehicle_id) {
        const Veh &vehicle = Vehs[stack_vehicle_id];
        if (vehicle.next_veh_id_stack == stack_vehicle_id ||
            vehicle.prev_veh_id_stack == stack_vehicle_id) {
            const int x = vehicle.x;
            const int y = vehicle.y;
            const int lifted_vehicle = veh_lift(stack_vehicle_id);
            veh_drop(lifted_vehicle, x, y);
            danger("Found a baddy!", StringTable.get(0), 0, 0, 0);
            found_invalid_vehicle = true;
        }
    }

    if (!found_invalid_vehicle) {
        danger("Game fixed!", StringTable.get(0), 0, 0, 0);
    }
    goto native_editor_command_dispatch;
}

if ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
    for (int debug_vehicle_id = 0; debug_vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++debug_vehicle_id) {
        const Veh &vehicle = Vehs[debug_vehicle_id];
        if (vehicle.next_veh_id_stack != debug_vehicle_id &&
            vehicle.prev_veh_id_stack != debug_vehicle_id) {
            continue;
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, Players[vehicle.faction_id].adj_name_faction);
        strcat(StringTemp, " ");
        strcat(StringTemp, VehPrototypes[vehicle.proto_id].veh_name);
        strcat(StringTemp, "(");
        char number[16];
        _itoa(vehicle.x, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ",");
        _itoa(vehicle.y, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ")");

        danger("Funny Unit", StringTemp, debug_vehicle_id, vehicle.x, vehicle.y);
    }
    goto native_editor_command_dispatch;
}

        break;
    case '!':
        if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) != 0) {

// ---- flattened from fragments/16-checksum-base-report.inc ----
// Native key-handler dispatch region.
// This is the checked-in, typed translation of the report filename/header
// and per-Base checksum loop only. Continue with later report families and
// the shared 0x519F9C dispatcher before assembling the member. The debug
// guard intentionally remains open for sequential fragment 17 and later
// report slices; close it only at the actual native branch continuation.

if ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
    StringTemp[0] = '\0';
    strcat(StringTemp, "Checksum");
    _itoa(faction_id_, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, ".TXT");

    native_handler_log.init(StringTemp);
    native_handler_log.reset();
    native_handler_log.say("BASE CHECKSUMS", 0, 0, 0, 0);

    for (int checksum_base_id = 0; checksum_base_id < GameDataStateStorage.fields_.base_current_count_; ++checksum_base_id) {
        Base &base = Bases[checksum_base_id];
        uint8_t value = checksum(reinterpret_cast<char *>(&base), 7, 0);
        value = checksum(reinterpret_cast<char *>(&base.nutrients_accumulated),
                          4, value);
        value = checksum(reinterpret_cast<char *>(&base.minerals_accumulated),
                          4, value);
        value = checksum(reinterpret_cast<char *>(&base.queue_size), 4, value);
        value = checksum(reinterpret_cast<char *>(&base.queue_production_id[0]),
                          4, value);
        value = checksum(reinterpret_cast<char *>(&base.resource_sqr_radius),
                          4, value);
        value = checksum(reinterpret_cast<char *>(&base.spec[0]),
                          8, value);
        value = checksum(reinterpret_cast<char *>(&base.facilities_built[0]),
                          9, value);

        StringTemp[0] = '\0';
        _itoa(value, native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Base ");
        _itoa(checksum_base_id, native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        native_handler_log.say(StringTemp, base.name_string, value, 0, 0);
    }

    native_handler_log.say(" ", 0, 0, 0, 0);
    native_handler_log.say("LEADER CHECKSUMS", 0, 0, 0, 0);
// Evidence: the active `!` table entry is input 0x21/index 0 -> 0x518BB1.
// This block is entered only with GameState bit 0x1000. It initializes the
// local Log with "Checksum" + faction_id + ".TXT", resets it, emits
// "BASE CHECKSUMS", then scans BaseCurrentCount records at stride 0x134.
// Each checksum input/length follows 0x518C48..0x518CC6 in order; Base layout
// fields and sizeof(Base)==0x134 are source-owned in base.h; the following
// leader, technology and player reports retain the same outer scan order.


// ---- flattened from fragments/17-player-diplomacy-report.inc ----
// Native key-handler dispatch region.
// faction loop. Native starts at faction 1 and stops when the next Player
// pointer reaches Players[8]+0x2FC; thus source bounds are 1 <= owner < 8.
// The six outer owner scalar/social/satellite groups precede these seven
// diplomacy rows and remain in the same native outer loop.

{
    for (int owner_faction = 1; owner_faction < MaxPlayerNum;
         ++owner_faction) {
    PlayerData &report_player = PlayersData[owner_faction];

    GenderDefault = Players[owner_faction].noun_gender;
    PluralityDefault = Players[owner_faction].is_noun_plural;
    native_handler_log.say(" ", Players[owner_faction].noun_faction,
                           0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(report_player.tech_accumulated, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Research");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(report_player.tech_id_researching, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Researching");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(report_player.planet_ecology, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Planet");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(report_player.energy_reserves, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Energy");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(report_player.integrity_blemishes, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Blacken");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    for (int social_index = 0; social_index < 4; ++social_index) {
        StringTemp[0] = '\0';
        _itoa(static_cast<int>(report_player.soc_category_pending.models_[social_index]),
              native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Social");
        _itoa(social_index, native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int satellite_index = 0; satellite_index < 4; ++satellite_index) {
        StringTemp[0] = '\0';
        _itoa(static_cast<int>(report_player.satellites_[satellite_index]),
              native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Satellites");
        _itoa(satellite_index, native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int treaty_faction = 0; treaty_faction < MaxPlayerNum; ++treaty_faction) {
        StringTemp[0] = '\0';
        _itoa(static_cast<int>(report_player.diplo_treaties[treaty_faction]),
              native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Treaties with ");
        GenderDefault = Players[treaty_faction].noun_gender;
        PluralityDefault = Players[treaty_faction].is_noun_plural;
        strcat(StringTemp, Players[treaty_faction].noun_faction);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int agenda_faction = 0; agenda_faction < MaxPlayerNum; ++agenda_faction) {
        StringTemp[0] = '\0';
        _itoa(report_player.diplo_agenda[agenda_faction], native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Agenda with ");
        GenderDefault = Players[agenda_faction].noun_gender;
        PluralityDefault = Players[agenda_faction].is_noun_plural;
        strcat(StringTemp, Players[agenda_faction].noun_faction);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int friction_faction = 0; friction_faction < MaxPlayerNum; ++friction_faction) {
        StringTemp[0] = '\0';
        _itoa(report_player.diplo_friction[friction_faction], native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Friction with ");
        GenderDefault = Players[friction_faction].noun_gender;
        PluralityDefault = Players[friction_faction].is_noun_plural;
        strcat(StringTemp, Players[friction_faction].noun_faction);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int patience_faction = 0; patience_faction < MaxPlayerNum; ++patience_faction) {
        StringTemp[0] = '\0';
        _itoa(static_cast<signed char>(report_player.diplo_patience[patience_faction]),
              native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Patience with ");
        GenderDefault = Players[patience_faction].noun_gender;
        PluralityDefault = Players[patience_faction].is_noun_plural;
        strcat(StringTemp, Players[patience_faction].noun_faction);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int merc_faction = 0; merc_faction < MaxPlayerNum; ++merc_faction) {
        StringTemp[0] = '\0';
        _itoa(report_player.diplo_merc[merc_faction], native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Merc with ");
        GenderDefault = Players[merc_faction].noun_gender;
        PluralityDefault = Players[merc_faction].is_noun_plural;
        strcat(StringTemp, Players[merc_faction].noun_faction);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int spoke_faction = 0; spoke_faction < MaxPlayerNum; ++spoke_faction) {
        StringTemp[0] = '\0';
        _itoa(report_player.diplo_spoke[spoke_faction], native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Spoke with ");
        GenderDefault = Players[spoke_faction].noun_gender;
        PluralityDefault = Players[spoke_faction].is_noun_plural;
        strcat(StringTemp, Players[spoke_faction].noun_faction);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int wrongs_faction = 0; wrongs_faction < MaxPlayerNum; ++wrongs_faction) {
        StringTemp[0] = '\0';
        _itoa(report_player.diplo_wrongs[wrongs_faction], native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Wrongs with ");
        GenderDefault = Players[wrongs_faction].noun_gender;
        PluralityDefault = Players[wrongs_faction].is_noun_plural;
        strcat(StringTemp, Players[wrongs_faction].noun_faction);
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }
    }
}

// Native ranges: [0x51904F,0x5190E4) diplo_treaties, [0x5190E4,0x519179)
// diplo_agenda, [0x519179,0x51920E) diplo_friction, [0x51920E,0x5192A2)
// signed-byte diplo_patience, [0x5192A2,0x519337) diplo_merc, and
// [0x519337,0x5193CC) diplo_spoke. Each scans the eight Player records at
// stride 0x59C; the source members and both grammatical defaults are typed.


// ---- flattened from fragments/18-vehicle-checksum-report.inc ----
// Native key-handler dispatch region.
// checksum section. Place after fragments 16 and 17, still inside the
// original GameState debug guard. Map checksums and record enumerations follow.

native_handler_log.say(" ", 0, 0, 0, 0);
native_handler_log.say("UNIT CHECKSUMS", 0, 0, 0, 0);
for (int checksum_vehicle_id = 0; checksum_vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++checksum_vehicle_id) {
    Veh &vehicle = Vehs[checksum_vehicle_id];
    uint8_t value = checksum(reinterpret_cast<char *>(&vehicle.x), 2, 0);
    value = checksum(reinterpret_cast<char *>(&vehicle.y), 2, value);
    value = checksum(reinterpret_cast<char *>(&vehicle.proto_id), 3, value);
    value = checksum(reinterpret_cast<char *>(&vehicle.morale), 1, value);
    value = checksum(reinterpret_cast<char *>(&vehicle.moves_expended), 1,
                      value);
    value = checksum(reinterpret_cast<char *>(&vehicle.home_base_id), 2,
                      value);

    StringTemp[0] = '\0';
    _itoa(value, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Veh ");
    _itoa(checksum_vehicle_id, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, ": ");
    GenderDefault = Players[vehicle.faction_id].noun_gender;
    PluralityDefault = Players[vehicle.faction_id].is_noun_plural;
    strcat(StringTemp, Players[vehicle.faction_id].adj_name_faction);
    strcat(StringTemp, " ");
    strcat(StringTemp, VehPrototypes[vehicle.proto_id].veh_name);
    native_handler_log.say(StringTemp, 0, value, 0, 0);
}

// Pinned native checks at [Veh+0, +2, +0xA, +0x24, +0x28, +0x2E) with byte
// lengths 2,2,3,1,1,2 and rolling uint8 seed; the Veh array stride is 0x34.
// The row appends the native vehicle ordinal, faction adjective, and prototype
// name, then passes the checksum as Log::say's first integer parameter.


// ---- flattened from fragments/19-map-checksum-report.inc ----
// Native key-handler dispatch region.
// checksum section. Follow unit checksums; continue with base/vehicle detail
// rows before closing the debug-report guard.

native_handler_log.say(" ", 0, 0, 0, 0);
native_handler_log.say("MAP CHECKSUMS", 0, 0, 0, 0);
for (int map_y = 0; map_y < MapLatitudeBounds; ++map_y) {
    int map_x = map_y & 1;
    for (; map_x < MapLongitudeBounds; map_x += 2) {
        Map &tile = MapTiles[MapLongitude * map_y + (map_x >> 1)];
        uint8_t value = checksum(reinterpret_cast<char *>(&tile.climate), 1, 0);
        value = checksum(reinterpret_cast<char *>(&tile.region), 1, value);
        value = checksum(reinterpret_cast<char *>(&tile.bit), 4, value);
        value = checksum(reinterpret_cast<char *>(&tile.contour), 1, value);
        value = checksum(reinterpret_cast<char *>(&tile.territory), 1, value);
        value = checksum(reinterpret_cast<char *>(&tile.bit_visible[0]), 2, value);

        StringTemp[0] = '\0';
        _itoa(value, native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " at: ");
        _itoa(map_x, native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, ", ");
        _itoa(map_y, native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, ": ");
        native_handler_log.say(StringTemp, 0, value, 0, 0);
    }
}

// Native report walks y=0..MapLatitudeBounds-1 and x=(y&1), +=2, with one
// Map record per pair of horizontal map coordinates. Its checksum covers
// offsets {0,3,8,1,7,0xC} for {1,1,4,1,1,2} bytes, then logs x and y. Map's
// source-owned stride is 0x2C. Confirmed source translation of the native
// formatting pieces matches the bounded literals in native-literals.json.


// ---- flattened from fragments/20-base-enumeration-report.inc ----
// Native key-handler dispatch region.
// enumeration. Follow map checksums; UNIT ENUMERATION follows this block.

native_handler_log.say(" ", 0, 0, 0, 0);
native_handler_log.say("BASES CHECKSUMS", 0, 0, 0, 0);
for (int enumerated_base_id = 0; enumerated_base_id < GameDataStateStorage.fields_.base_current_count_; ++enumerated_base_id) {
    Base &base = Bases[enumerated_base_id];

    StringTemp[0] = '\0';
    _itoa(enumerated_base_id, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    native_handler_log.say(StringTemp, base.name_string, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(base.x), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " X");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(base.y), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Y");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(base.population_size), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Size");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(base.faction_id_former), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Former");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(base.nutrients_accumulated, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Nutrient");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(base.minerals_accumulated, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Minerals");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(base.queue_size), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Num Making");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(base.queue_production_id[0], native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Making");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(base.resource_sqr_radius, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Radius");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    for (int specialist_word = 0; specialist_word < 2; ++specialist_word) {
        StringTemp[0] = '\0';
        _itoa(static_cast<int>(base.spec[specialist_word]), native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, specialist_word == 0 ? " Spec 0" : " Spec 1");
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }

    for (int facility_index = 0; facility_index < 9; ++facility_index) {
        StringTemp[0] = '\0';
        _itoa(static_cast<int>(base.facilities_built[facility_index]),
              native_decimal_buffer, 10);
        strcat(StringTemp, native_decimal_buffer);
        strcat(StringTemp, " Fac");
        native_handler_log.say(StringTemp, 0, 0, 0, 0);
    }
}

// Native source row offsets are Base+0,+2,+6,+5,+0x3C,+0x40,+0x4C,+0x50,
// +0x78,+0x84,+0x88,+0x8C..+0x94; source Base fields and 0x134 stride own
// every access. The fixed facility loop is exactly nine bytes, not the full
// source member's twelve-byte extent. Bounded source strings were retained in
// native-literals.json. Verify each call's pushed argument order when the
// report slices are assembled.


// ---- flattened from fragments/21-unit-enumeration-report.inc ----
// Native key-handler dispatch region.
// enumeration, followed by its final danger notification in fragment 22.

native_handler_log.say(" ", 0, 0, 0, 0);
native_handler_log.say("UNIT ENUMERATION", 0, 0, 0, 0);
for (int unit_vehicle_id = 0; unit_vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++unit_vehicle_id) {
    Veh &vehicle = Vehs[unit_vehicle_id];

    StringTemp[0] = '\0';
    _itoa(unit_vehicle_id, native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " ");
    GenderDefault = Players[vehicle.faction_id].noun_gender;
    PluralityDefault = Players[vehicle.faction_id].is_noun_plural;
    strcat(StringTemp, Players[vehicle.faction_id].adj_name_faction);
    strcat(StringTemp, " ");
    strcat(StringTemp, VehPrototypes[vehicle.proto_id].veh_name);
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(vehicle.x), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " X");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(vehicle.y), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " Y");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(vehicle.proto_id), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " proto");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(vehicle.faction_id), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " who");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(vehicle.dmg_incurred), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " damage");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(vehicle.morale), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " morale");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(vehicle.moves_expended), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " moves");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);

    StringTemp[0] = '\0';
    _itoa(static_cast<int>(vehicle.home_base_id), native_decimal_buffer, 10);
    strcat(StringTemp, native_decimal_buffer);
    strcat(StringTemp, " home");
    native_handler_log.say(StringTemp, 0, 0, 0, 0);
}

// Native receiver starts at Vehs[0]+0x0A (proto_id), then advances by the
// source-owned Veh stride 0x34. Signed word reads are x, y, proto_id and
// home_base_id; byte reads for faction, damage, morale and moves are zero
// extended. All listed members are source-owned in veh.h.


// ---- flattened from fragments/22-checksum-danger-tail.inc ----
// Native key-handler dispatch region.
// After all report sections, the native method presents this synchronous
// notification, then converges on command dispatch at 0x519F9C.

danger("Made CHECKSUM FILE", StringTable.get(0), 0, 0, 0);

        }
        // Close the outer GameState::STATE_SCENARIO_EDITOR guard separately
        // from the nested debug-report guard closed immediately above.
        }
        break;
    default:
        break;
    }
}

native_editor_command_dispatch:

// ---- flattened from fragments/03-editor-command-tables.inc ----
// Native key-handler dispatch region.
// These switches are selected by a2 command IDs after the main input mode
// transitions. They are not the same domain as the character switches.

switch (a2) {
// 0x51B900, a2 in [0x10070,0x1007B], native input base 0x10070.
case 0x10070: editor_veh(); break;
case 0x10071: tech_advance(ConsoleGlobal.faction_id_); break;
case 0x10072: editor_view(); break;
case 0x10073: editor_energy(); break;
case 0x10074: editor_date(); break;
case 0x10075: editor_eliminate(); break;
case 0x10076: show_replay(); break;
case 0x10077: show_movies(); break;
case 0x10078: editor_diplomacy(); break;
case 0x10079: editor_rules(); break;
case 0x1007A: editor_scen_rules(); break;
case 0x1007B: editor_scen_param(); break;

// 0x51B930, a2 in [0x20074,0x20078].
case 0x20074: editor_tech(); break;
case 0x20075: editor_save(); break;
case 0x20076:
    // Preserved dependency: this function remains seed-only and cannot be
    // replaced by an empty local definition. The native call passes slot 1.
    save_game(1);
    break;
case 0x20077: editor_kill_vehicles(); break;
case 0x20078: editor_reset_tech(); break;

// The neighboring command is tested outside the 0x51B930 table.
case 0x30070: editor_edit_veh(-1); break;

// 0x51B944, a2 in [0x30071,0x3007B]. The 0x30071 arm is a faction-specific
// 0x59-record scan. Native does not call tech_advance here: it tests the
// ownership predicate and prerequisite sentinels, marks every eligible
// GameTechAchieved bit, and adds two to tech_ranking per newly granted record.
case 0x30071:
    {
        const int faction = ConsoleGlobal.faction_id_;
        int granted_count = 0;
        for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
            if (has_tech_call(tech_id, faction) != 0) continue;
            if (Technology[tech_id].preq_tech_1 < TechNone) continue;
            if (Technology[tech_id].preq_tech_2 < TechNone) continue;

            int bit_offset;
            int bit_mask;
            bitmask_call(tech_id, &bit_offset, &bit_mask);
            GameDataStateStorage.fields_.game_tech_achieved_[tech_id] = static_cast<uint8_t>(
                GameDataStateStorage.fields_.game_tech_achieved_[tech_id] | (1u << faction));
            ++PlayersData[faction].tech_ranking;
            ++PlayersData[faction].tech_ranking;
            ++granted_count;
        }
        if (granted_count == 0) {
            PlayersData[faction].tech_ranking = 1;
            for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
                GameDataStateStorage.fields_.game_tech_achieved_[tech_id] = static_cast<uint8_t>(
                    GameDataStateStorage.fields_.game_tech_achieved_[tech_id] & ~(1u << faction));
            }
        }
    }
    break;
case 0x30072: editor_diff(); break;
case 0x30073: break; // target is the native no-op continuation 0x51A2C2
case 0x30074: editor_load(); break;
case 0x30075: editor_reload(); break;
case 0x30076: load_proc(1, 0); break;
case 0x30077: editor_reset_faction(); break;
case 0x30078: editor_strategy(); break;
case 0x30079: editor_lock2(); break;
case 0x3007A: break; // target is the native no-op continuation 0x51A2C2
case 0x3007B: editor_scen_victory(); break;
default:
    break;
}

// Keep the guarded single-id selection event distinct from table 0x51B944:
// a2 == 0x30070 enters editor_edit_veh(-1) directly at 0x51A0FF. The native
// branch at a2 == 0x40008 enters editor_undo() at 0x51A2B7.
if (a2 == 0x40008) {
    editor_undo();
}

// This fragment ends before the map-view hotkey and later faction/gameplay
// character tables; their code is in progress. Direct menu/owner methods above
// use the class declarations already present in console.h. The seed-only
// save_game body is an outside-owner dependency, not a forwarding stand-in.


// ---- flattened from fragments/04-scenario-editor-character-table.inc ----
// Native key-handler dispatch region.
// Native enters the 0x51B970 byte table only when GameState has
// STATE_SCENARIO_EDITOR and the earlier command-consumed local is clear.
native_scenario_editor_gate:
if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) != 0 &&
    native_command_consumed == 0) {
switch (normalized_character) {
case '$':
    if ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
        MapWinTable[0]->field_1DD70_ ^= 4;
    }
    break;
case '%':
    if ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
        MapWinTable[0]->field_1DD70_ ^= 0x2000;
    }
    break;
case '*':
    if ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
        MapWinTable[0]->field_1DD70_ ^= 0x10000;
    }
    break;
case '1': editor_paints(0); break;
case '2': editor_paints(1); break;
case '3': editor_paints(2); break;
case '4': editor_paints(3); break;
case '5': editor_paints(4); break;
case '6': editor_terrain(); break;
case '7': editor_set_brush(); break;
case '8': editor_climate(); break;
case '9': editor_generate(); break;
case '0': editor_fast(); break;
default:
    // The byte table sends every other normalized byte to the shared
    // 0x51A3F8 tail, which sets the WorldClimateWin repaint latch and refreshes
    // WorldClimateWorldWin. It does not consume arbitrary editor characters.
    break;
}
// The native explicit 0 key calls editor_fast() at 0x51A389; the 7/8/9 keys
// call editor_set_brush/editor_climate/editor_generate. Keep this as the
// exact table mapping rather than renumbering the editor tools by intuition.

// Shared 0x51A3F8 tail reached after this table, including default entries:
// draw_map(1), set WorldWin's redraw latch bit 4 and dispatch the world map.
::draw_map(1);
WorldClimateWorldWin.field_239BC_ |= 4;
WorldClimateWorldWin.set_world_map();
}


// ---- flattened from fragments/05-report-menu-and-enter-handlers.inc ----
// Native key-handler dispatch region.
// Candidate translation of the active 0x51B9C0 key table and adjacent direct
// commands. The common 0x10051/0x20051 confirmation path is represented below;
// coordinator review still owns whole-function integration and source binding.

// Native 0x51A53E compares the command before entering this table: exactly
// 0x20043 calls Console::chat(0), while larger IDs dispatch at 0x51AC11.
if (a2 == 0x20043) {
    chat(0);
    goto native_command_done;
}
if (a2 > 0x20043) {
    goto native_mode_command_tables;
}

if (a2 == 0x77) {
    report_score(0);
    goto native_command_done;
}

switch (a2) {
case 0x13: // Ctrl+S/native save menu action
    if (IsMultiplayerNet != 0 &&
        (NetDaemonNet.lock_state_[0].trailing_words_[4] &
         (1u << ConsoleGlobal.faction_id_)) != 0) {
        message_data(0x4C01, 0, 0, 0, 0, 0);
        MultiWindow.draw(0);
        goto native_command_done;
    }
    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
        // Native pushes target=null and channel=0 before duration and label;
        // these are all four explicit parameters of the owned NetMsg method.
        NetMsgPrimary.pop("SIMULNOT", 5000, 0, 0);
        MultiWindow.draw(0);
        goto native_command_done;
    }
    message_data(0x4C00, 0, 0, 0, 0, 0);
    MultiWindow.draw(0);
    goto native_command_done;
case 0x1B: // Escape
    {
        const char *confirmation = "REALLYQUIT";
        if ((IsMultiplayerNet != 0 || IsMultiplayerPBEM != 0) &&
            bit_count(static_cast<int>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu)) > 1) {
            confirmation = "REALLYWIMPOUT";
        }
        if (X_pop(confirmation, 0) != 0) {
            if (IsMultiplayerPBEM != 0) {
                GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(1u << ConsoleGlobal.faction_id_);
                save_game(0);
            }
            if ((GameDataStateStorage.fields_.game_rules_ & 0x400u) != 0 &&
                IsMultiplayerPBEM == 0) {
                auto_save();
            }
            if (IsMultiplayerNet != 0) {
                NetDaemonNet.hang_up();
                NetDaemonNet.reset();
                IsMultiplayerNet = 0;
            }
            ExitTurnLoop = 1;
            log_say("Quit***********************************", 0, 0, 0);
        }
    }
    goto native_command_done;
case 0x08: // Backspace: clear cursor selection/order and refresh UI state
    field_23BF8_ = 0;
    if (survey_mode_ != 0) {
        survey_mode_ = 0;
        MapWinTable[0]->draw_map(1);
    }
    MapWinTable[0]->draw_cursor();
    if (cursor_active_ != 0) {
        ambience(0x1E);
        cursor_active_ = 0;
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            MapWinTable[0]->draw_map(1);
        }
        ::draw_cursor();
        ConsoleInfoWin.change(0);
        ConsoleStatusWin.redraw();
        MapWinTable[0]->main_caption();
        alpha_menu_.check(selected_vehicle_);
    }
    --cursor_history_index_;
    if (cursor_history_index_ < 0) {
        cursor_history_index_ = 0x1F;
    }
    {
        const int history = cursor_history_index_;
        const int x = cursor_history_x_[history];
        const int y = cursor_history_y_[history];
        ConsoleStatusWin.set_loc(x, y);
        MapWinTable[0]->focus(x, y);
    }
    goto native_map_pan_route;
case 0x70: DatalinkMain.exec(0x0E, 0); goto native_command_done;
case 0x71: report_labs(); goto native_command_done;
case 0x72: report_energy(); goto native_command_done;
case 0x73: report_ops(); goto native_command_done;
case 0x74: report_projects(); goto native_command_done;
case 0x75: report_sat(); goto native_command_done;
case 0x76: report_security(); goto native_command_done;
case 0x78: hall_of_fame(0); goto native_command_done;
case 0x79: monument(0); goto native_command_done;
case 0x7A:
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x200000u) != 0) {
        GameDataStateStorage.fields_.game_more_preferences_ &= ~0x200000u;
        ConsoleGlobal.alpha_menu_.check(ConsoleGlobal.selected_vehicle_);
        X_pop("MENUSIMPLE", 0);
    } else {
        GameDataStateStorage.fields_.game_more_preferences_ |= 0x200000u;
        ConsoleGlobal.alpha_menu_.check(ConsoleGlobal.selected_vehicle_);
        X_pop("MENUDETAIL", 0);
    }
    goto native_command_done;
case 0x7B:
    MainInterfaceGlobal.on_iface_button_clicked(0x3EC);
    goto native_command_done;
case 0x0D:
case 0x1000D: // Enter/remote command: activate the current cursor base
    if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_2) != 0 && edit_lock() == 0) {
        field_23BE4_ = 0;
        field_23BE8_ = 0;
        goto native_command_done;
    }
    {
        const int history = cursor_history_index_;
        const int x = cursor_history_x_[history];
        const int y = cursor_history_y_[history];
        const int base_id = base_at(x, y);
        if (base_id >= 0) {
            const Base &base = Bases[base_id];
            const int owner = base.faction_id_current;
            const int local_faction = faction_id_;
            const bool omniscient = (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
            const bool visible =
                (base.visibility & (1u << local_faction)) != 0;
            if (owner == local_faction || visible || omniscient) {
                if (omniscient || owner == local_faction || spying(owner)) {
                    TutWinBaseWindow.zoom(base_id, 0);
                }
            }
        }
    }
    goto native_command_done;
case 0x10008: // advance cursor history, then share native map/cursor routing
    if (native_command_consumed == 0) { // native stack local [ebp-0x28]
        ConsoleGlobal.next_cursor();
        goto native_map_pan_route;
    }
    goto native_done;
default:
    break; // all other byte-table indexes reach native 0x51B078
}

// The native direct-command tail is deliberately kept separate from the
// 0x51B9C0 byte table. These IDs do not alias the later 0x200xx preferences.
if (a2 == 0x1004E) {
    if (native_command_consumed != 0) {
        goto native_done;
    }
    name_landmark(-1, -1);
    goto native_map_pan_route;
}
if (a2 == 0x20041) {
    set_auto_preferences();
    goto native_command_done;
}
if (a2 == 0x20042) {
    if (native_command_consumed != 0) {
        goto native_done;
    }
    go_to(-1, -2, 0);
    goto native_map_pan_route;
}
if (a2 == 0x10051) {
    retire_game();
    goto native_retirement_confirmation;
}
goto native_command_done;

// Native 0x10051 reaches this block after retire_game(); 0x20051 enters the
// same block directly from the 0x51BA60 table. They share this exact tail.
native_retirement_confirmation:
{
    // Native asks about ending the active multiplayer/PBEM game after the
    // optional preceding retire operation. Its tail is shared by both IDs.
    const char *quit_confirmation = "REALLYQUIT";
    if ((IsMultiplayerNet != 0 || IsMultiplayerPBEM != 0) &&
        bit_count(static_cast<int>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu)) > 1) {
        quit_confirmation = "REALLYWIMPOUT";
    }
    if (X_pop(quit_confirmation, 0) != 0) {
        if (IsMultiplayerPBEM != 0) {
            GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(1u << ConsoleGlobal.faction_id_);
            save_game(0);
        }
        if ((GameDataStateStorage.fields_.game_rules_ & 0x400u) != 0 && IsMultiplayerPBEM == 0) {
            auto_save();
        }
        if (IsMultiplayerNet != 0) {
            NetDaemonNet.hang_up();
            NetDaemonNet.reset();
            IsMultiplayerNet = 0;
        }
        ExitTurnLoop = 1;
        log_say("Quit***********************************", 0, 0, 0);
    }
}
goto native_command_done;

// Native 0x51B078 tests stack local [ebp-0x28] before entering 0x51B081.
// Most successful command handlers arrive with it clear and continue through
// the shared map/cursor router; the handful of earlier consumed-key paths
// take the common cleanup edge directly.
native_command_done:
if (native_command_consumed != 0) {
    goto native_done;
}
goto native_map_pan_route;

// Native adjacent command overrides represented here:
// - 0x77 calls report_score(0); 0x1000D enters Enter/base selection;
// - 0x10008 advances cursor history when the native consumed local is clear;
// - 0x1004E names the current landmark; 0x10051 retires then performs the
//   separate native post-retire transition; 0x20041 opens automation prefs;
//   0x20042 invokes go_to(-1,-2,0).


// ---- flattened from fragments/06-preference-and-mode-command-tables.inc ----
// Native late mode/preferences command families, native 0x51AC11..0x51B146.
// Includes 0x51BA60 (command IDs 0x20047..0x2005A) and 0x51BAB0 (byte-map
// domain 0x30041..0x30073); the substantial setting/network branches and the
// map-panel key layout cases continue in fragment 07.

native_mode_command_tables:

switch (a2) {
case 0x20047:
    GameDataStateStorage.fields_.game_preferences_ ^= 0x100;
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x100) != 0) {
        alpha_menu_.check_menu_item(2, 0x20A);
    } else {
        alpha_menu_.uncheck_menu_item(2, 0x20A);
    }
    AlphaIniPrefs.preferences = GameDataStateStorage.fields_.game_preferences_;
    prefs_save(0);
    ::draw_map(1);
    break;
case 0x20049: set_audiovisual(); break;
case 0x2004B: editor_on(); break;
case 0x2004C:
    // The native branch skips the load only when multiplayer is active and
    // GameState's STATE_DEBUG_MODE bit is clear (test at 0x51ACE5..0x51ACF5).
    if (IsMultiplayerNet == 0 || (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
        load_proc(0, 0);
    }
    break;
case 0x2004D: set_map_display(); break;
case 0x2004E:
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x1000) != 0) {
        GameDataStateStorage.fields_.game_more_preferences_ &= ~0x3000u;
    } else if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x2000) != 0) {
        GameDataStateStorage.fields_.game_more_preferences_ |= 0x1000;
    } else {
        GameDataStateStorage.fields_.game_more_preferences_ |= 0x2000;
    }
    AlphaIniPrefs.more_preferences = GameDataStateStorage.fields_.game_more_preferences_;
    prefs_save(0);
    ::draw_map(1);
    break;
case 0x2004F: set_adv_preferences(); break;
case 0x20050: set_preferences(); break;
case 0x20051:
    // Native 0x20051 branches directly to 0x51AA0D, unlike 0x10051 which
    // first calls retire_game() and then falls into this same tail.
    goto native_retirement_confirmation;
case 0x20053:
    save_game(0);
    break;
case 0x20057: set_base_preferences(); break;
case 0x20058: zoom(0x208, 0); go_reset(); break;
case 0x2005A: zoom(0x207, 0); go_reset(); break;
default:
    break;
}

// The 0x51BAB0 lookup is keyed by a2 codes in [0x30041,0x30073]. Its active
// bytes are exact and sparse: 0x30041, 0x30047, 0x3004C, 0x3004E, 0x30051,
// 0x30057, 0x30058, 0x30073; all other bytes use the native shared no-op.
switch (a2) {
case 0x30041:
    MapWinTable[0]->field_1DD70_ ^= 0x40000;
    ::draw_map(1);
    break;
case 0x30047:
    GameDataStateStorage.fields_.game_preferences_ ^= 0x200;
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) != 0) {
        alpha_menu_.check_menu_item(2, 0x20B);
    } else {
        alpha_menu_.uncheck_menu_item(2, 0x20B);
    }
    AlphaIniPrefs.preferences = GameDataStateStorage.fields_.game_preferences_;
    prefs_save(0);
    ::draw_map(1);
    break;
case 0x3004C:
    if (IsMultiplayerNet != 0 || (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) != 0) {
        ConsoleGlobal.load_proc(0, 0);
    }
    break;
case 0x3004E:
    if (native_editor_lock == 0) {
        ConsoleGlobal.editor_undo();
    }
    break;
case 0x30051:
    {
        if (IsMultiplayerNet != 0) {
            const char *confirmation = "REALLYQUIT";
            if (IsMultiplayerPBEM != 0 &&
                bit_count(static_cast<int>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu)) > 1) {
                confirmation = "REALLYWIMPOUT";
            }
            if (X_pop(confirmation, 0) != 0) {
                if (IsMultiplayerPBEM != 0) {
                    GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(1u << ConsoleGlobal.faction_id_);
                    save_game(0);
                }
                if ((GameDataStateStorage.fields_.game_rules_ & 0x400u) != 0 &&
                    IsMultiplayerPBEM == 0) {
                    auto_save();
                }
                if (IsMultiplayerNet != 0) {
                    NetDaemonNet.hang_up();
                    NetDaemonNet.reset();
                    IsMultiplayerNet = 0;
                }
                ExitTurnLoop = 1;
                log_say("Quit***********************************", 0, 0, 0);
            }
            if (ExitTurnLoop != 0) {
                GameRestartQueued = 1;
            }
        }
        break;
    }
case 0x30057:
    GameDataStateStorage.fields_.game_more_preferences_ ^= 1;
    ::draw_map(1);
    prefs_save(0);
    break;
case 0x30058:
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x10000u) != 0) {
        GameDataStateStorage.fields_.game_more_preferences_ &= ~0x10000u;
        alpha_menu_.uncheck_menu_item(2, 0x209);
    } else {
        GameDataStateStorage.fields_.game_more_preferences_ |= 0x10000u;
        alpha_menu_.check_menu_item(2, 0x209);
    }
    AlphaIniPrefs.more_preferences = GameDataStateStorage.fields_.game_more_preferences_;
    ::draw_map(1);
    prefs_save(0);
    break;
case 0x30073:
    show_credits();
    break;
default:
    break; // table value 8 routes to native 0x51B078
}

// The next three tables (0x51BB08, 0x51BB28, 0x51BB48) are not ordinary
// character actions. They select editor/map-panel layout modes. Their exact
// bases are respectively 0x21/count8, 0x62/count8, and 0x10026/count0x43.
// Targets 0x51B14D/1E9/248/2AC perform signed divide-by-3 viewport math on
// MapWin slot-zero fields 0x1DDCC..0x1DDD8, wrap X with map longitude and call
// MapWin::set_center. The common 0x51B1A4.. tail gates on cursor/selected
// vehicle, Console::edit_lock(), and updates Console cursor history or hover
// state. Keep signed divisions, edge clamping, longitude wrapping, and the
// selected-vehicle source distinct while translating the remaining lines.


// ---- flattened from fragments/07-layout-and-map-pan.inc ----
// Native key-handler dispatch region.
// Contains the mapped layout events, map pan, cursor movement, selected-unit
// mode handling and shared cursor/vehicle key continuation.

native_map_pan_route:

// 0x51B081 preserves the independent 0x20000 mode flag, and when set strips
// both high control bits before the byte/direct key lookups.
native_original_key = a2;
native_mode_flag = a2 & 0x20000;
if (native_mode_flag != 0) {
    a2 &= ~0x30000;
}

// The first layout table is indexed from 0x21 through 0x28 (8 entries).
// Native maps those eight keys to mode values 0,2,4,6,5,7,1,3 respectively.
// The second table is indexed from 0x62 through 0x69 and maps to
// 3,2,5, native-shared,1,6,7,0. Both later store this in the input slot.
// The mapped values are cursor direction modes and go to the distinct
// 0x51B324 movement/action block. Key 0x65 instead lands at shared 0x51B1A4.
switch (a2) {
case 0x21: a2 = 0; goto native_cursor_move_mode;
case 0x22: a2 = 2; goto native_cursor_move_mode;
case 0x23:
    a2 = 4;
    goto native_cursor_move_mode;
case 0x24: a2 = 6; goto native_cursor_move_mode;
case 0x25: a2 = 5; goto native_cursor_move_mode;
case 0x26: a2 = 7; goto native_cursor_move_mode;
case 0x27: a2 = 1; goto native_cursor_move_mode;
case 0x28: a2 = 3; goto native_cursor_move_mode;
case 0x61:
    // Native tests bit 0x10000 in the unmasked original event only when the
    // independent 0x20000 mode bit is present. In that combination this key
    // maps to mode 4 but the native target rejects it (only mode 1 dispatches
    // the special selection path there).
    if (native_mode_flag != 0 && (native_original_key & 0x10000) != 0) {
        goto native_done;
    }
    a2 = 4;
    goto native_cursor_move_mode;
case 0x62: a2 = 3; goto native_cursor_move_mode;
case 0x63: a2 = 2; goto native_cursor_move_mode;
case 0x64: a2 = 5; goto native_cursor_move_mode;
case 0x65:
    break;
case 0x66: a2 = 1; goto native_cursor_move_mode;
case 0x67: a2 = 6; goto native_cursor_move_mode;
case 0x68: a2 = 7; goto native_cursor_move_mode;
case 0x69: a2 = 0; goto native_cursor_move_mode;
default:
    break;
}

// The 0x51BB5C byte map's input domain is [0x10026, 0x10069], count 0x43;
// its active events are 0x10026/0x10068 (north), 0x10027/0x10066 (east),
// 0x10028/0x10062 (south), and 0x10064 (west). Direct 0x10025 also enters
// the west-pan target at 0x51B248. The native map is in dispatch-map.json.
{
    MapWin *map_window = MapWinTable[0];
    if (a2 == 0x10025 || a2 == 0x10026 || a2 == 0x10027 ||
        a2 == 0x10028 || a2 == 0x10062 || a2 == 0x10064 ||
        a2 == 0x10066 || a2 == 0x10068) {
        int destination_x = map_window->field_1DD9C_;
        int destination_y = map_window->field_1DDA0_;
        if (a2 == 0x10026 || a2 == 0x10068) {
            // Native signed multiply-high sequence is truncating signed / 3.
            int step = (map_window->field_1DDD0_ + 2) / 3;
            if (step < 0 || map_window->field_1DDA8_ < 0) {
                step = 0;
            } else if (step > map_window->field_1DDA8_) {
                step = map_window->field_1DDA8_;
            }
            destination_y -= step;
        } else if (a2 == 0x10028 || a2 == 0x10062) {
            int step = (map_window->field_1DDD0_ + 2) / 3;
            const int maximum = MapLatitudeBounds -
                2 * map_window->field_1DDD8_ - map_window->field_1DDA8_ + 1;
            if (step < 0 || maximum < 0) {
                step = 0;
            } else if (step > maximum) {
                step = maximum;
            }
            destination_y += step;
        } else {
            int step = (map_window->field_1DDCC_ + 2) / 3;
            // West/east use the signed horizontal step without a latitude
            // clamp; only the longitude wrap below belongs to these routes.
            if (a2 == 0x10025 || a2 == 0x10064) {
                destination_x -= step;
            } else {
                destination_x += step;
            }

            if ((MapIsFlat & 1) == 0) {
                if (destination_x < 0) {
                    destination_x += MapLongitudeBounds;
                } else if (destination_x >= MapLongitudeBounds) {
                    destination_x -= MapLongitudeBounds;
                }
            }
        }

        if (destination_x != map_window->field_1DD9C_ ||
            destination_y != map_window->field_1DDA0_) {
            map_window->set_center(destination_x, destination_y, 1);
        }
    }
}

goto native_cursor_common;

native_cursor_move_mode:
// 0x51B324 movement-mode handler. With no active cursor (or after native's
// editor-lock deactivation sequence), move one signed map step from history.
if (cursor_active_ != 0) {
    if (edit_lock() == 0) {
        if (native_editor_lock != 0) {
            goto native_done;
        }
        if (selected_vehicle_ < 0) {
            goto native_done;
        }
        if (native_mode_flag != 0) {
            Veh &vehicle = Vehs[selected_vehicle_];
            vehicle.order = 0x19;
            vehicle.waypoint_x[0] = static_cast<int16_t>(a2);
            vehicle.waypoint_x[1] = vehicle.x;
            vehicle.waypoint_y[1] = vehicle.y;
            goto native_done;
        }
        if (not_my_turn() != 0) {
            NetMsgPrimary.pop("SIMULNO", 5000, 0, 0);
            goto native_done;
        }
        if (veh_ready(selected_vehicle_) == 0) {
            NetMsgPrimary.pop("UNITMOVED", 5000, 0, 0);
            field_23BE4_ = 0;
            goto native_done;
        }
        NetDaemonNet.order_veh(selected_vehicle_, a2, 0);
        goto native_done;
    }
    ambience(0x1E);
    field_23BF8_ = 1;
    field_23BFC_ = 1;
    const uint32_t previous_survey_mode = survey_mode_;
    cursor_active_ = 0;
    if (previous_survey_mode != 0) {
        survey_mode_ = 0;
        MapWinTable[0]->draw_map(1);
    }
    ::draw_cursor();
    ConsoleInfoWin.change(0);
    ConsoleStatusWin.redraw();
    MapWinTable[0]->main_caption();
    alpha_menu_.check(selected_vehicle_);
}

{
    const int history = cursor_history_index_;
    int x = cursor_history_x_[history] + VehMoveDeltaX[a2];
    int y = cursor_history_y_[history] + VehMoveDeltaY[a2];
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) {
            x += MapLongitudeBounds;
        } else if (x >= MapLongitudeBounds) {
            x -= MapLongitudeBounds;
        }
    }
    if (y >= 0 && y < MapLatitudeBounds && x >= 0 &&
        x < MapLongitudeBounds) {
        field_23BF8_ = 0;
        MapWin *const map_window = MapWinTable[0];
        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            map_window->draw_map(1);
        }
        map_window->draw_cursor();
        cursor_next(x, y);
        ConsoleStatusWin.redraw();
        field_23BF8_ = 1;
        field_23BFC_ = 1;
        map_window->draw_cursor();
        map_window->focus(x, y);
    }
}
goto native_done;

native_cursor_common:
if (cursor_active_ == 0) {
    cursor_key(a1, normalized_character, a2);
    goto native_done;
}
if (edit_lock() == 0) {
    if (selected_vehicle_ < 0) {
        goto native_done;
    }
    veh_key(a1, normalized_character, a2);
    goto native_done;
}
ambience(0x1E);
field_23BF8_ = 1;
field_23BFC_ = 1;
{
    const uint32_t previous_survey_mode = survey_mode_;
    cursor_active_ = 0;
    if (previous_survey_mode != 0) {
        survey_mode_ = 0;
        MapWinTable[0]->draw_map(1);
    }
}
::draw_cursor();
ConsoleInfoWin.change(0);
ConsoleStatusWin.redraw();
MapWinTable[0]->main_caption();
alpha_menu_.check(selected_vehicle_);
cursor_key(a1, normalized_character, a2);
goto native_done;


// ---- flattened from fragments/15-common-epilogue.inc ----
// Shared return edge at native 0x51B648/0x51B64F.
// Declare one automatic `Log` in the enclosing member body: native constructs
// it at entry with Log::Log (0x625FB0) and destroys it on this common edge
// with Log::~Log (0x626020). Its scope is the whole handler, matching the
// native SEH lifetime. Keep the ordinary source-owned class lifetime instead
// of spelling calls through addresses.

native_done:
return 1;

}

// ORIGINAL: 0x0051DF00 ?on_nc_left_down@thunk1_Console@@QAEXHH@Z 0x0051DF00-0x0051DF08 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor span at 0x0050F460 stores table 0x0066EC18; exact cell 0x0066EC9C is 0x0051DF00.", "The complete original thunk bytes at 0x0051DF00 tail to pinned target 0x0050FB10.", "Fresh full build build-fpcqf1x8: ??_7Console@@6BWin@@@+0x84 COFF relocation selects ?on_nc_left_down@Console@@$4PPPPPPPM@A@AEXHH@Z; thunk bytes and its unique relative tail relocation target ?on_nc_left_down@Console@@UAEXHH@Z.", "The owner compile receipt inputs were unchanged and match current files at proposal emission; selected output is observed in the fresh MAP/PE but this proposal does not claim complete link-preservation acceptance.", "Historical source kind `game`, original identity/name, complete span, and parsed claims/lessons are checked unchanged after native-writer readback."], "reason": "The pinned Console constructor at 0x0050F460 installs the ??_7Console@@6BWin@@@ address point; its exact slot cell at +0x84 selects original identity 0x0051DF00. The fresh full-build owner COFF vftable relocation selects the compiler-generated thunk ?on_nc_left_down@Console@@$4PPPPPPPM@A@AEXHH@Z, whose unique physical relative tail targets ?on_nc_left_down@Console@@UAEXHH@Z. The original marker name and complete source history remain attached to this FILE identity."}
// kind      game
// symbol    ?on_nc_left_down@Console@@$4PPPPPPPM@A@AEXHH@Z

/*
 * RECOVERY_HISTORY: v1 address=0x0051DDE0 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=0521c4f5ad1301580c581a88b1479693698786bf2c22227d2ccafa4d64ac80fb
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Console's thunk1 subobject to the one
 * [ARCHIVE]          Console::scalar_delete expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x0051DDE0 ??_Gthunk1_Console@@UAEPAXI@Z 0x0051DDE0-0x0051DDE8
 * [ARCHIVE] // symbol    ?adjust_console1_scalar_delete@@YIPAXPAX0I@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void* (__thiscall ??_Gthunk1_Console@@UAEPAXI@Z)(Console* this, unsigned int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x0051DDE0 ??_Gthunk1_Console@@UAEPAXI@Z 0x0051DDE0-0x0051DDE8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class-view slot 0 selects the complete original scalar deleting adjustor at 0x0051DDE0.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded ??_EConsole@@$4PPPPPPPM@A@AEPAXI@Z in the actual console.cpp COFF view-table relocation; its adjustment prefix is byte-identical to the pinned thunk.", "That build's selected linked MAP/PE cell and PDB XFIXUP resolve the generated entry to ??_GConsole@@UAEPAXI@Z at 0x00407ED0; the physical COFF E primary is a per-class weak fallback to its G primary (search mode 2).", "That linked primary tests mode bit 0, conditionally calls ??3@YAXPAX@Z, returns the saved object pointer in EAX, and uses ret 4.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved scalar deleting-adjustor identity to the compiler-emitted class-view entry whose actual per-class E-to-G fallback and linked G body preserve the original receiver adjustment, deletion-flag contract, object return and stack cleanup."}
// symbol    ??_EConsole@@$4PPPPPPPM@A@AEPAXI@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x0051DF50 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=28abc097b47dd56ed1ee00d1e916a015f477f4889625fe90dd40a0b9eab886c9
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Console's thunk2 subobject to the one
 * [ARCHIVE]          Console::scalar_delete expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x0051DF50 ??_Gthunk2_Console@@UAEPAXI@Z 0x0051DF50-0x0051DF61
 * [ARCHIVE] // symbol    ?adjust_console2_scalar_delete@@YIPAXPAX0I@Z
 * [ARCHIVE] // size      17 bytes
 * [ARCHIVE] // prototype void* (__thiscall ??_Gthunk2_Console@@UAEPAXI@Z)(Console* this, unsigned int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x0051DF50 ??_Gthunk2_Console@@UAEPAXI@Z 0x0051DF50-0x0051DF61 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class-view slot 0 selects the complete original scalar deleting adjustor at 0x0051DF50.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded ??_EConsole@@$4PPPPPLLI@EEE@AEPAXI@Z in the actual console.cpp COFF view-table relocation; its adjustment prefix is byte-identical to the pinned thunk.", "That build's selected linked MAP/PE cell and PDB XFIXUP resolve the generated entry to ??_GConsole@@UAEPAXI@Z at 0x00407ED0; the physical COFF E primary is a per-class weak fallback to its G primary (search mode 2).", "That linked primary tests mode bit 0, conditionally calls ??3@YAXPAX@Z, returns the saved object pointer in EAX, and uses ret 4.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved scalar deleting-adjustor identity to the compiler-emitted class-view entry whose actual per-class E-to-G fallback and linked G body preserve the original receiver adjustment, deletion-flag contract, object return and stack cleanup."}
// symbol    ??_EConsole@@$4PPPPPLLI@EEE@AEPAXI@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x0051DEF0 source_sha256=15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f envelope_sha256=7937cba18d6e8aac8c8591100a989f43fdd77ca7f84573d36a51b64fa5368e86
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Console's thunk1 subobject to the one
 * [ARCHIVE]          Console::on_nc_mouse_move expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x0051DEF0 ?on_nc_mouse_move@thunk1_Console@@QAEXHH@Z 0x0051DEF0-0x0051DEF8
 * [ARCHIVE] // symbol    ?adjust_console1_on_nc_mouse_move@@YIXPAX0HH@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_nc_mouse_move@thunk1_Console@@QAEXHH@Z)(Console* this, int, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
/*
 * RECOVERY_HISTORY: v1 address=0x0051DF10 source_sha256=15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f envelope_sha256=37ddaa38b6a53d790767396d5fc2a32d2594f42d6df20cf9c5b4b9f51becf7f8
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Console's thunk1 subobject to the one
 * [ARCHIVE]          Console::on_nc_left_up expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x0051DF10 ?on_nc_left_up@thunk1_Console@@QAEXHH@Z 0x0051DF10-0x0051DF18
 * [ARCHIVE] // symbol    ?adjust_console1_on_nc_left_up@@YIXPAX0HH@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_nc_left_up@thunk1_Console@@QAEXHH@Z)(Console* this, int, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
/*
 * RECOVERY_HISTORY: v1 address=0x0051DF30 source_sha256=15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f envelope_sha256=44e403a97bb3aef98912293e94a5ac00a173c73c97f77849f3d6bbfdba9f9e09
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Console's thunk1 subobject to the one
 * [ARCHIVE]          Console::on_post_redraw_nc_buffer expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x0051DF30 ?on_post_redraw_nc_buffer@thunk1_Console@@QAEXPAUBuffer@@H@Z 0x0051DF30-0x0051DF38
 * [ARCHIVE] // symbol    ?adjust_console1_on_post_redraw_nc_buffer@@YIXPAX00H@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_post_redraw_nc_buffer@thunk1_Console@@QAEXPAUBuffer@@H@Z)(Console* this, Buffer*, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x0051DEF0 ?on_nc_mouse_move@thunk1_Console@@QAEXHH@Z 0x0051DEF0-0x0051DEF8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Console constructor 0x0050F460 installs Win address point 0x0066EC18 at 0x0050F562; slot 31 cell selects original identity 0x0051DEF0.", "Pinned Win primary table 0x0066FDD0 has the matching shared callback at slot 31; Win's named contract retains the original parameter shape.", "Pinned original thunk 0x0051DEF0 is `sub ecx,[ecx-4]; jmp 0x00510110`; the typed Console member is source-owned in src/console.cpp.", "The complete seed comment envelope, including original marker/name/spans/prototype/callers/kind/flags/calls/return/status/purpose/role and prose, is inertly archived in RECOVERY_HISTORY with source and envelope SHA-256 15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f/7937cba18d6e8aac8c8591100a989f43fdd77ca7f84573d36a51b64fa5368e86.", "The real Win virtual contract and Console override emit a compiler-owned vtordisp tail to the typed Console implementation, rather than a handwritten receiver-adjustment adapter. Source emitted-symbol facts identify the actual thunk and target separately; linked selection and matching require fresh verification."], "reason": "Bind the preserved Console Win-view callback identity to its pinned constructor-installed virtual slot and the natural compiler-generated entry for the typed Console override."}
// symbol    ?on_nc_mouse_move@Console@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0051DF10 ?on_nc_left_up@thunk1_Console@@QAEXHH@Z 0x0051DF10-0x0051DF18 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Console constructor 0x0050F460 installs Win address point 0x0066EC18 at 0x0050F562; slot 34 cell selects original identity 0x0051DF10.", "Pinned Win primary table 0x0066FDD0 has the matching shared callback at slot 34; Win's named contract retains the original parameter shape.", "Pinned original thunk 0x0051DF10 is `sub ecx,[ecx-4]; jmp 0x0050FE70`; the typed Console member is source-owned in src/console.cpp.", "The complete seed comment envelope, including original marker/name/spans/prototype/callers/kind/flags/calls/return/status/purpose/role and prose, is inertly archived in RECOVERY_HISTORY with source and envelope SHA-256 15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f/37ddaa38b6a53d790767396d5fc2a32d2594f42d6df20cf9c5b4b9f51becf7f8.", "The real Win virtual contract and Console override emit a compiler-owned vtordisp tail to the typed Console implementation, rather than a handwritten receiver-adjustment adapter. Source emitted-symbol facts identify the actual thunk and target separately; linked selection and matching require fresh verification."], "reason": "Bind the preserved Console Win-view callback identity to its pinned constructor-installed virtual slot and the natural compiler-generated entry for the typed Console override."}
// symbol    ?on_nc_left_up@Console@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0051DF30 ?on_post_redraw_nc_buffer@thunk1_Console@@QAEXPAUBuffer@@H@Z 0x0051DF30-0x0051DF38 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Console constructor 0x0050F460 installs Win address point 0x0066EC18 at 0x0050F562; slot 60 cell selects original identity 0x0051DF30.", "Pinned Win primary table 0x0066FDD0 has the matching shared callback at slot 60; Win's named contract retains the original parameter shape.", "Pinned original thunk 0x0051DF30 is `sub ecx,[ecx-4]; jmp 0x0050F960`; the typed Console member is source-owned in src/console.cpp.", "The complete seed comment envelope, including original marker/name/spans/prototype/callers/kind/flags/calls/return/status/purpose/role and prose, is inertly archived in RECOVERY_HISTORY with source and envelope SHA-256 15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f/44e403a97bb3aef98912293e94a5ac00a173c73c97f77849f3d6bbfdba9f9e09.", "The real Win virtual contract and Console override emit a compiler-owned vtordisp tail to the typed Console implementation, rather than a handwritten receiver-adjustment adapter. Source emitted-symbol facts identify the actual thunk and target separately; linked selection and matching require fresh verification."], "reason": "Bind the preserved Console Win-view callback identity to its pinned constructor-installed virtual slot and the natural compiler-generated entry for the typed Console override."}
// symbol    ?on_post_redraw_nc_buffer@Console@@$4PPPPPPPM@A@AEXPAVBuffer@@H@Z
// kind      game
