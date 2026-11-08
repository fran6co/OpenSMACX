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
// ORIGINAL: 0x0051DED0 ?on_key_click@thunk1_Console@@QAEHHH@Z 0x0051DED0-0x0051DED8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The pinned Console constructor 0x0050F460 stores Win-view table 0x0066EC18; slot 25 (byte offset 0x64) contains original thunk 0x0051DED0.", "The pinned Win primary slot 25 targets 0x00406A80; Win declares int on_key_click(int, int), and Console retains the same int callback contract.", "Original thunk 0x0051DED0 adjusts the vtordisp receiver and tail-jumps to mapped Console::on_key_click at 0x005178C0.", "Seed history records `int (__thiscall ?on_key_click@thunk1_Console@@QAEHHH@Z)(Console* this, int, int)` and flags hidden/sp_ready/purged_ok; those source findings remain with this original identity."], "reason": "Bind the original Console Win-view receiver adjustment to the existing typed Console callback at 0051DED0; the pinned Win base contract and Console constructor-installed view establish the ownership relation."}
// symbol    ?on_key_click@Console@@$4PPPPPPPM@A@AEHHH@Z
// kind game
// ORIGINAL: 0x0051DF40 ?on_nc_hittest@thunk1_Console@@QAEXHH@Z 0x0051DF40-0x0051DF48 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The pinned Console constructor 0x0050F460 stores Win-view table 0x0066EC18; slot 75 (byte offset 0x12C) contains original thunk 0x0051DF40.", "The pinned Win primary slot 75 targets 0x005F5AD0; Win::on_nc_hittest and the existing Console::on_nc_hittest source method both have int(int,int) contracts.", "The seed catalogue's thunk prototype says void, but the original thunk preserves EAX while tail-jumping to 0x0050F680; that body sets EAX to 0 or 1 on its exits. The source owner keeps the int result; no source callback or base slot is renamed.", "Seed history records `void (__thiscall ?on_nc_hittest@thunk1_Console@@QAEXHH@Z)(Console* this, int, int)` and flags hidden/sp_ready/purged_ok; those source findings remain with this original identity."], "reason": "Bind the original Console Win-view receiver adjustment to the existing typed Console callback at 0051DF40; the pinned Win base contract and Console constructor-installed view establish the ownership relation."}
// symbol    ?on_nc_hittest@Console@@$4PPPPPPPM@A@AEHHH@Z
// kind game
// ORIGINAL: 0x0050E850 _$E21 0x0050E850-0x0050E868 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682620 selects original initializer0x50e850..0x50e868, registering fini0x50e870..0x50e884; complete independent decode establishes exclusive boundaries.", "Source-owned object?ConsoleGlobal@@3VConsole@@A identifies original storage0x9156b0. Startup arguments/calls: push 1; call 0x50f460; push 0x50e870. Cleanup receiver/calls: mov ecx, 0x939444; call 0x51d9f0; mov ecx, 0x939444; jmp 0x5d4dd0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/console.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?ConsoleGlobal@@3VConsole@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E16
// ORIGINAL: 0x0050E870 _$E19 0x0050E870-0x0050E884 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682620 selects original initializer0x50e850..0x50e868, registering fini0x50e870..0x50e884; complete independent decode establishes exclusive boundaries.", "Source-owned object?ConsoleGlobal@@3VConsole@@A identifies original storage0x9156b0. Startup arguments/calls: push 1; call 0x50f460; push 0x50e870. Cleanup receiver/calls: mov ecx, 0x939444; call 0x51d9f0; mov ecx, 0x939444; jmp 0x5d4dd0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/console.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?ConsoleGlobal@@3VConsole@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E14
// ORIGINAL: 0x0050E980 sub_50e980 0x0050E980-0x0050E996 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682624 selects startup 0x50e980..0x50e996 and registers cleanup 0x50e9a0..0x50e9aa. Complete root decode proves the exclusive RET/JMP endpoints.", "The source-owned object ?ConsoleTimer@@3VTime@@A is selected by physical COFF constructor/destructor relocations; current emitted initializer is identified by the adjacent symbol fact. Original receiver 0x939e88 belongs to this real global; preserve its class lifetime and registration.", "Buffer relative declaration order is PCX, interface array, middle, vehicle, special-text owner, Sprite, shared Buffer. Generated helpers are compiler output, not hand-authored address callbacks. Selected same-owner order does not establish full CRT order, dependency provider, linked preservation or runtime/game acceptance. Guarded cleanup is a separate unfinished obligation."], "reason": "Bind original init operation to the genuine C++ lifetime of ?ConsoleTimer@@3VTime@@A; retain original behavior and remaining matching obligations."}
// symbol    _$E21
// ORIGINAL: 0x0050E9A0 sub_50e9a0 0x0050E9A0-0x0050E9AA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682624 selects startup 0x50e980..0x50e996 and registers cleanup 0x50e9a0..0x50e9aa. Complete root decode proves the exclusive RET/JMP endpoints.", "The source-owned object ?ConsoleTimer@@3VTime@@A is selected by physical COFF constructor/destructor relocations; current emitted finalizer is identified by the adjacent symbol fact. Original receiver 0x939e88 belongs to this real global; preserve its class lifetime and registration.", "Buffer relative declaration order is PCX, interface array, middle, vehicle, special-text owner, Sprite, shared Buffer. Generated helpers are compiler output, not hand-authored address callbacks. Selected same-owner order does not establish full CRT order, dependency provider, linked preservation or runtime/game acceptance. Guarded cleanup is a separate unfinished obligation."], "reason": "Bind original fini operation to the genuine C++ lifetime of ?ConsoleTimer@@3VTime@@A; retain original behavior and remaining matching obligations."}
// symbol    _$E19
#include "stdafx.h"

// Distinct writable string cells observed at the original call sites.
// These role names do not claim original source names or allocation bounds.
// RECOVERY_OBJECT: v1 0x0068A9A4 _EraseLandmarkDeniedKey
static char EraseLandmarkDeniedKey[] = "LANDMARKNO";

// Original auto_play_callback at 0x0050E890 uses this named USER32 export; PE imports supply its slot.
// RECOVERY_IMPORT: v1 __imp__SetCursorPos@8 USER32.dll name:SetCursorPos
#include "prefwin.h"  // Console owns preference-window calls.
#include "interlude.h"  // interlude is declared by its owner.
#include "text.h" // TextBufferGetPtr is reloaded after parser callbacks.
#include "alphamovie.h"

// Console constructor-installed Win branch: capture4, resize13, mouse17, update63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x0066EC18 ? ??_7Console@@6BWin@@@ 0x0050F460
#include "console.h"
#include "console_commands.h"
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
#include "win_pointer.h"  // go_timer inlines Win::grab_pointer
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
#include "tutwin.h"  // TutorialWindow - survey2 raises the map tutorial on it
#include "technology.h"  // has_tech - editor_tech and the tech orders gate on it
#include "random.h"  // random - the editor's randomizer and the AI orders
#include "alpha.h"  // AlphaIniPrefs - the toggles mirror the prefs into it
#include "terraforming.h"  // Terraforming rules drive the scenario terrain picker
#include "planwin.h"
#include "councwin.h"  // CouncilWindow - turn_timer redraws it
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
#include "worldwin.h"  // WorldClimateWorldWin - editor_flat / editor_clear redraw it
#include "terraforming.h"  // Terraforming - destroy's option list reads its bit/label rows
#include "buttongroup.h"  // ButtonGroup::set - the editor brush/paint selectors
#include <cstdlib>
#include <cstring>

void __cdecl draw_cursor();                               // 0x0046AE00
void __cdecl ambience_veh(int vehicle);

// ORIGINAL: 0x00511520 ?set_move@Console@@QAEXH@Z 0x00511520-0x005117FD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963abb2ef8a11892e1d754d80934bd50b12e:src/recovered/units/00511520.cpp", "source:src/console.h", "source:src/veh.h", "source:src/game.h", "source:src/mapwin.h", "pinned:0x00511520..0x005117FD sequential multiplayer gates, ready/ambience selection split, visibility call and callback-sensitive coordinate reads", "source:src/netdaemon.h:NetDaemon::primary_state_"], "reason": "Complete the original Console move workflow through its genuine ready, ambience_veh and spot_all helpers. Preserve the observed ordered guards, selection recheck, offline visibility condition and post-callback field reads. The historical faction waiting-mask identity now binds to NetDaemonNet.primary_state_.trailing_words_[6]; the singleton owns that shared storage."}
void Console::set_move(int force_update) {
    int did_reset = 0;
    if (IsMultiplayerNet != 0) {
        if (ConsoleControlTurnActive != 0) {
            return;
        }
        if (NetDaemonNet.channels_[1].payload_.flags != 0) {
            return;
        }
        if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
            if (not_my_turn() != 0) {
                return;
            }
        } else {
            const int faction_bit = 1 << faction_id_;
            if ((NetDaemonNet.factions_waiting_mask() & faction_bit) != 0) {
                return;
            }
            if (NetDaemonNet.primary_state_.trailing_words_[3] != 0) {
                return;
            }
        }
    }

    if (ConsoleGlobal.cursor_active_ == 0 || force_update != 0) {
        cursor_active_ = 0;
        field_23BF8_ = 0;
        field_23BFC_ = 1;
        did_reset = 1;

        if (survey_mode_ != 0) {
            survey_mode_ = 0;
            console_map_win()->draw_map(1);
        }
        ::draw_cursor();
        cursor_active_ = 1;
        field_23BE4_ = 0;
    }

    if (veh_selectable(selected_vehicle_) == 0) {
        ready();
        did_reset = 0;
    } else {
        ambience_veh(selected_vehicle_);
    }

    if (selected_vehicle_ >= 0 && veh_selectable(selected_vehicle_) != 0) {
        const int visible_vehicle_id = selected_vehicle_;
        if (Vehs[visible_vehicle_id].moves_expended == 0 && IsMultiplayerNet == 0) {
            spot_all(visible_vehicle_id, 0);
        }
        const int coordinate_vehicle_id = selected_vehicle_;
        const int tile_y = Vehs[coordinate_vehicle_id].y;
        const int tile_x = Vehs[coordinate_vehicle_id].x;
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
            const int focus_vehicle_id = selected_vehicle_;
            const int history_index = ConsoleGlobal.cursor_history_index_;
            const int focus_faction = Vehs[focus_vehicle_id].faction_id;
            const int focus_y = ConsoleGlobal.cursor_history_y_[history_index];
            const int focus_x = ConsoleGlobal.cursor_history_x_[history_index];
            focus(focus_x, focus_y, focus_faction);
            alpha_menu_.check(selected_vehicle_);
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

    const int loc_index = ConsoleGlobal.cursor_history_index_;
    const int loc_y = ConsoleGlobal.cursor_history_y_[loc_index];
    const int loc_x = ConsoleGlobal.cursor_history_x_[loc_index];
    ConsoleStatusWin.set_loc(loc_x, loc_y);
    const int focus_index = ConsoleGlobal.cursor_history_index_;
    const int focus_faction = faction_id_;
    const int focus_y = ConsoleGlobal.cursor_history_y_[focus_index];
    const int focus_x = ConsoleGlobal.cursor_history_x_[focus_index];
    focus(focus_x, focus_y, focus_faction);
}



// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00511800 ?turn_hints@Console@@QAEXXZ 0x00511800-0x00511BA8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00511800; retained seed src/recovered/units/00511800.cpp", "Existing TutWin guard at8CC210 is member+53A8; stage8CC228 is+53C0. Typed global Console history, PlayerData prototype/base/population fields and diplomacy arrays replace seed address arithmetic. Caption pointer691B14 targets TUTOR691B8C.", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions retain the original task baseline"], "reason": "Recover the complete Console command into its genuine owner using typed state and existing lifecycle/network APIs. Preserve original spans and retained seed evidence; this binding is not a byte, reference or runtime acceptance claim."}
void Console::turn_hints() { // 0x00511800
    if ((GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_TUTORIAL_MSGS) == 0 ||
        IsMultiplayerNet != 0 || TutorialWindow.tutorial_blocked()) {
        return;
    }

    const int faction_id = ConsoleGlobal.faction_id_;
    if (GameDataStateStorage.fields_.turn_current_num_ == 1 &&
        tut_check(GameDataStateStorage.fields_.turn_current_num_) != 0) {
        const int base_id = base_find(
            ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
            ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], faction_id);
        if (base_id >= 0) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            if (selected_vehicle_ >= 0) {
                parse_says(1, VehPrototypes[Vehs[selected_vehicle_].proto_id].veh_name, -1, -1);
                TutorialWindow.set_tour_stage(0);
                TutorialWindow.tut_map("LANDFALL",
                    ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
                    ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], -1,
                    nullptr, 0x100, -1, -1);
            }
        }
    }

    bool second_base_tutorial_shown = false;
    if (GameDataStateStorage.fields_.turn_current_num_ >= 0x14 &&
        PlayersData[faction_id].current_num_bases == 1 &&
        PlayersData[faction_id].pop_total >= 2 &&
        PlayersData[faction_id].proto_id_active[0] == 0 &&
        PlayersData[faction_id].proto_id_queue[0] == 0 &&
        tut_check(0x100000) != 0) {
        second_base_tutorial_shown = true;
        const int base_id = base_find(
            ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
            ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], faction_id);
        if (base_id >= 0) {
            focus(Bases[base_id].x, Bases[base_id].y, faction_id);
            parse_says(0, Bases[base_id].name_string, -1, -1);
            TutorialWindow.tut_map("SECONDBASE", Bases[base_id].x, Bases[base_id].y, -2 - base_id,
                                  nullptr, 0, -1, -1);
            PlayersData[faction_id].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
        }
    }

    if (!second_base_tutorial_shown &&
        GameDataStateStorage.fields_.turn_current_num_ - PlayersData[faction_id].last_turn_new_base >= 0x18 &&
        PlayersData[faction_id].proto_id_active[0] == 0 &&
        PlayersData[faction_id].current_num_bases <= 5 &&
        PlayersData[faction_id].current_num_bases > 1) {
        const int base_id = base_find(
            ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
            ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], faction_id);
        if (base_id >= 0) {
            TutorialWindow.tut_map("BUILDMORE", Bases[base_id].x, Bases[base_id].y, -2 - base_id,
                                  nullptr, 0, -1, -1);
            PlayersData[faction_id].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
        }
    }

    for (int other_faction = 1; other_faction < MaxPlayerNum; ++other_faction) {
        if (faction_id != other_faction &&
            (PlayersData[faction_id].diplo_treaties[other_faction] & DTREATY_COMMLINK) &&
            (PlayersData[faction_id].diplo_treaties[other_faction] & DTREATY_VENDETTA)) {
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
            X_pop(TutorialPopupCaption, "VENDETTA", nullptr);
            return;
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00516DA0 ?governor_directives@Console@@QAEXXZ 0x00516DA0-0x00517033 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00516DA0; retained seed src/recovered/units/00516da0.cpp", "Native516DC7/516E0E/516E97 uses ConsoleGlobal faction for eligibility/base scanning. Native516EBB..516FC9 repeatedly writes BaseCurrent->governor, not each visited base. Advanced settings use receiver faction at+23BD4; preserve this original behavior rather than silently fixing it.", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions retain the original task baseline"], "reason": "Recover the complete Console command into its genuine owner using typed state and existing lifecycle/network APIs. Preserve original spans and retained seed evidence; this binding is not a byte, reference or runtime acceptance claim."}
void Console::governor_directives() { // 0x00516DA0
    if (ConsoleControlTurnActive != 0 || field_23BE8_ == 0 ||
        !is_human(ConsoleGlobal.faction_id_)) {
        return;
    }

    const int directive = x_pop_short("GOVERNORCONTROL", 0x40, 0);
    if (directive < 0) {
        return;
    }

    int first_owned_base;
    for (first_owned_base = 0;
         first_owned_base < GameDataStateStorage.fields_.base_current_count_;
         ++first_owned_base) {
        if (Bases[first_owned_base].faction_id_current == ConsoleGlobal.faction_id_) {
            break;
        }
    }
    if (first_owned_base >= GameDataStateStorage.fields_.base_current_count_) {
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
    // The image mutates BaseCurrent, the selected-base pointer, while
    // walking the owner's base records; it does not retarget it per record.
    for (int owned_base_id = 0; owned_base_id < GameDataStateStorage.fields_.base_current_count_; ++owned_base_id) {
        if (Bases[owned_base_id].faction_id_current == ConsoleGlobal.faction_id_) {
            switch (directive) {
            case 0:
                BaseCurrent->governor &= 0x7FFFFFFFu;
                BaseCurrent->governor &= 0xF0FFFFFFu;
                strcat(StringTemp, label_get(0xAF8 / 4));
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                BaseCurrent->governor |= 0x80000000u;
                BaseCurrent->governor &= 0xF0FFFFFFu;
                switch (directive) {
                case 1:
                    BaseCurrent->governor |= 0x01000000u;
                    strcat(StringTemp, label_get(0x824 / 4));
                    break;
                case 2:
                    BaseCurrent->governor |= 0x02000000u;
                    strcat(StringTemp, label_get(0x828 / 4));
                    break;
                case 3:
                    BaseCurrent->governor |= 0x04000000u;
                    strcat(StringTemp, label_get(0x82C / 4));
                    break;
                case 4:
                    BaseCurrent->governor |= 0x08000000u;
                    strcat(StringTemp, label_get(0x830 / 4));
                    break;
                }
                break;
            case 5:
                BaseCurrent->governor = static_cast<uint32_t>(
                    PlayersData[faction_id_].base_governor_adv);
                strcat(StringTemp, label_get(0xAFC / 4));
                break;
            }
        }
    }

    parse_says(0, StringTemp, -1, -1);
    NetMsgPrimary.pop("GOVERNORCONTROLDONE", 5000, 0, nullptr);
    diplo_unlock();
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

    NetDaemon &network = NetDaemonNet;
    if (ConsoleControlTurnActive &&
        (network.channels_[0].leading_words_[1] & (1 << ConsoleGlobal.faction_id_)) == 0 &&
        network.primary_state_.trailing_words_[3] == 0) {
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

    if (network.primary_state_.session_phase_ >= 2) {
        const unsigned int enabled = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xff;
        if ((enabled & network.channels_[1].trailing_words_[6]) != enabled) {
            for (int faction = 1; faction < 8; ++faction) {
                const int bit = 1 << faction;
                if (!(*reinterpret_cast<const unsigned char *>(
                        &GameDataStateStorage.fields_.lock_masks_.enable_mask_) & bit)) continue;
                if (target_faction > 0) {
                    if (faction != target_faction) continue;
                } else if (CommlinkPending) {
                    if (faction != CommlinkFaction) continue;
                } else if (!TurnRedrawPending &&
                           !(network.channels_[1].trailing_words_[6] & bit)) {
                    continue;
                }
                message_chat(0x1e00, faction, message);
                log_say("Sent CHAT", message, faction, 0, 0);
            }
            return;
        }
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
// RECOVERY_OBJECT: v1 0x00939FD0 ?AutoPlayCounter@@3HA
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

// The timer callbacks' shared state; declarations in time.h beside the
// callbacks themselves. All four are zero-initialised .data in the image.
// Original 0x0050EE4C/0x0050EE89/0x0050EF19 gate the timer callbacks on this shared value; start only.
// RECOVERY_OBJECT: v1 0x00915620 ?TimerHoldFlag@@3HA
int TimerHoldFlag;      // 0x00915620
// Blink2's counter at 0x009392B4 is ConsoleGlobal.blink2_counter_; its
// historical object identity is retained beside ConsoleGlobal in console.cpp.
// RECOVERY_OBJECT: v1 0x00939FD4 ?PlanLineCounter@@3HA
int PlanLineCounter;    // 0x00939FD4
// RECOVERY_OBJECT: v2 0x00703DE0 0x00703DE4 ?TurnRedrawPending@@3HA 0x00404C3B
int TurnRedrawPending;  // 0x00703DE0

// ORIGINAL: 0x0050EA40 ?blink_timer@@YAXH@Z 0x0050EA40-0x0050EE25 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:371deca46d2a09538c2887fcf5c729de7add0065:src/recovered/units/0050ea40.cpp: full original identity/spans and historical source-form findings", "Independent full seed/native semantic review; original three USER32 IAT targets resolve GetAsyncKeyState/GetFocus/GetWindowLongA with GWL_HINSTANCE.", "Win primary virtual slot12 is on_redraw; PlanWin::blink now uses genuine virtual dispatch, not manual vbase/table indexing.", "recovery/tasks/blink-timer-owner-oct5.json", "recovery/tests/test_blink_timer_actual_source.py"], "reason": "Complete timer callback through existing genuine tutorial/Plan/MainInterface/Console/MapWin owners and USER32 APIs. Preserve original guards, parity/wrap geometry, half-open bounds and cursor update ordering. Source family home is not asserted original TU ownership."}
void __cdecl blink_timer(int a1) {
    (void)a1;
    if (WorldClimateSkipTerrainClear != 0) return;
    if (TimerHoldFlag != 0) return;

    if (TutWinBaseWindow.is_visible() != 0) {
        TutorialWindow.draw_arrow();
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
        TutorialWindow.draw_arrow();
        return;
    }

    PlanWindow.blink();
    MainInterfaceGlobal.string_box_.clip_ids(0x96);

    // One clear-and-return exit serves both the idle and the turn-exit tests
    // (0x0050EADF stores the immediate zero once).
    if ((ConsoleGlobal.field_23BE8_ == 0
         && (IsMultiplayerNet == 0 || (GameDataStateStorage.fields_.game_state_ & 2) == 0))
        || ExitTurnLoop != 0) {
        ConsoleGlobal.field_23BFC_ = 0;
        return;
    }
    if (ConsoleGlobal.field_23BE4_ == 0
        && (IsMultiplayerNet == 0 || (GameDataStateStorage.fields_.game_state_ & 2) == 0)) {
        return;
    }

    ConsoleGlobal.field_23BF8_ = ConsoleGlobal.field_23BF8_ == 0 ? 1 : 0;
    draw_cursor();

    const int blink_col = HIBYTE(GetAsyncKeyState(1));
    if (reinterpret_cast<HINSTANCE>(GetWindowLongA(GetFocus(), GWL_HINSTANCE))
        != WinInstance) {
        return;
    }
    if (TutorialWindow.is_visible() != 0) return;
    if (ConsoleGlobal.field_23D80_ == -1) return;
    if (ConsoleGlobal.field_23D84_ == -1) return;
    if (ConsoleGlobal.field_23D88_ == -1) return;
    if (ConsoleGlobal.field_23D8C_ == -1) return;

    if ((GameDataStateStorage.fields_.game_preferences_ & 0x1000) == 0 && blink_col == 0
        && WinModalDepth == 0) {
        return;
    }

    // The walk keeps the slot pointer (0x007D3C3C..0x007D3C5C) and re-reads
    // the window through it after every call.
    for (MapWin **slot = &MapWinTable[0]; slot < &MapWinTable[MapWinTableSlots]; ++slot) {
        if (*slot == 0) continue;
        int col = blink_col;

        if (slot != &MapWinTable[0] && (*slot)->view_active_ == 0) continue;
        if (col != 0 && (*slot)->field_1DD80_ == 0) continue;
        if ((*slot)->field_1DDD4_ + (*slot)->field_1DDCC_ >= MapLongitudeBounds) {
            continue;
        }

        int row = (*slot)->center_y_;
        col = (*slot)->center_x_;

        int mouse_x;
        int mouse_y;
        (*slot)->get_mouse_pos(&mouse_x, &mouse_y);
        (*slot)->client_to_screen(&mouse_x, &mouse_y);

        if (mouse_x == 0) {
            col -= ((*slot)->field_1DDCC_ + 2) / 3;
            if ((MapIsFlat & 1) == 0) {
                if (col < 0) {
                    col += MapLongitudeBounds;
                } else if (col >= MapLongitudeBounds) {
                    col -= MapLongitudeBounds;
                }
            }
        }

        if (mouse_x >= WinScreenWidth - 0xA
            && mouse_x < WinScreenWidth + 0xA) {
            int next_col = ((*slot)->field_1DDCC_ + 2) / 3 + col;
            if ((MapIsFlat & 1) == 0) {
                if (next_col < 0) {
                    next_col += MapLongitudeBounds;
                } else if (next_col >= MapLongitudeBounds) {
                    next_col -= MapLongitudeBounds;
                }
            }
            col = next_col;
        }

        if (mouse_y == 0 && (*slot)->field_1DDA8_ > -2) {
            const int max_shift = (*slot)->field_1DDA8_ + 2;
            const int requested = ((*slot)->field_1DDD0_ + 2) / 3;
            int delta;
            if (requested < 0 || max_shift < 0) {
                delta = 0;
            } else if (requested <= max_shift) {
                delta = requested;
            } else {
                delta = max_shift;
            }
            row -= delta;
        }

        if (mouse_y >= WinScreenHeight - 0xA
            && mouse_y < WinScreenHeight + 0xA) {
            const int count = (*slot)->field_1DDD8_;
            const int start = (*slot)->field_1DDA8_;
            if (start + count * 2 - 2 < MapLatitudeBounds + 2) {
                const int max_shift = (2 - count) * 2 - start
                                    + MapLatitudeBounds;
                const int requested = ((*slot)->field_1DDD0_ + 2) / 3;
                int delta;
                if (requested < 0 || max_shift < 0) {
                    delta = 0;
                } else if (requested > max_shift) {
                    delta = max_shift;
                } else {
                    delta = requested;
                }
                row += delta;
            }
        }

        if ((col & 1) != 0) col -= 1;
        if ((row & 1) != 0) col += 1;

        if (col == (*slot)->center_x_
            && row == (*slot)->center_y_) {
            continue;
        }
        if (row < 0) continue;
        if (row >= MapLatitudeBounds) continue;
        if (col < 0) continue;
        if (col >= MapLongitudeBounds) continue;

        if (*slot == static_cast<MapWin *>(&ConsoleGlobal)) {
            (*slot)->field_21A44_ = 1;
        }
        (*slot)->set_center(col, row, 1);
        (*slot)->field_21A44_ = 0;
    }
}

// PlanWindow's virtual GraphicWin base begins at the timer's historical
// receiver 0x00856DC0. The real PlanWin object owns that base and lifetime.
// The MultiWin the turn timer redraws (`mov ecx, 0x7fd648`) is the REAL
// object MultiWindow in game.cpp; game.cpp closes its GraphicWin front for
// desktop_close - the same fixed address, one object.



// global

/*
Purpose: Blink timer tick: skip while any window is up or the game holds the
         timers, then advance the blink counter and repaint the interface.
// ORIGINAL: 0x0050EE30 ?blink2_timer@@YAXH@Z 0x0050EE30-0x0050EE7D BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ee30 archive unit. Five
//   sequential early-return `if`s (NOT one chained `&&`): the image jumps to a
//   SHARED `ret` epilogue from every test. The counter increment and the
//   MainInterface receiver load interleave in the image (`mov eax,[ctr]; mov
//   ecx,0x7ae820; inc; mov [ctr],eax`) exactly as the two-statement spelling
//   schedules, and the redraw_complete call is the last statement, which VC6
//   tail-jumps (`jmp 0x45c3a0`).
// size      77 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
Return Value: n/a
Status: Complete
*/
void __cdecl blink2_timer(int a1) {
    if (TutWinBaseWindow.is_visible() != 0) {
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
        return;
    }
    if (TimerHoldFlag != 0) {
        return;
    }
    if (ExitTurnLoop != 0) {
        return;
    }
    if (WorldClimateSkipTerrainClear != 0) {
        return;
    }
    ++ConsoleGlobal.blink2_counter_;
    MainInterfaceGlobal.redraw_complete();
}

/*
Purpose: Plan-line timer tick: count the line counter down and repaint the
         plan lines on every live map window.
// ORIGINAL: 0x0050EE80 ?line_timer@@YAXH@Z 0x0050EE80-0x0050EF01 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ee80 archive unit. The slot
//   walk is a POINTER walk (`slot < &MapWinTable[8]`), not an index loop -
//   the image keeps the slot pointer in esi (`cmp esi, 0x7d3c5c; jl`), and
//   "slot == &MapWinTable[0]" is the primary-window exemption
//   (`cmp esi, 0x7d3c3c; je`), as in draw_tile. The PlanWin::on_redraw call
//   sits between the counter store and the loop with the receiver loaded as
//   an immediate; the two flag reads are plain (non-volatile) dwords off the
//   window pointer, which VC6 keeps in ecx across the guards.
// The pinned loop bounds 0x007D3C3C..0x007D3C5C and four-byte stride
//   identify eight ordered slots. A signed index preserves the live table
//   walk without converting its slot pointers to integers.
// size      129 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00467970 0x0048AF30 0x005F7E90
Return Value: n/a
Status: Complete
*/
void __cdecl line_timer(int a1) {
    if (WorldClimateSkipTerrainClear != 0) {
        return;
    }
    if (TimerHoldFlag != 0) {
        return;
    }
    if (TutWinBaseWindow.is_visible() != 0) {
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
        return;
    }
    --PlanLineCounter;
    // The image makes this a DIRECT call (0x50EEBF -> 0x48AF30
    // ?on_redraw@PlanWin@@QAEXXZ), not a slot-12 dispatch; the qualified
    // spelling suppresses the virtual call the shared name now implies.
    PlanWindow.PlanWin::on_redraw();

    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == nullptr) {
            continue;
        }
        if (slot != 0) {
            const uint32_t active = window->view_active_;
            if (active == 0) {
                continue;
            }
        }
        const uint32_t draw_flags = window->field_1DD70_;
        if ((draw_flags & 0xC40000) != 0) {
            window->draw_base_dest(1);
        }
    }
}

/*
Purpose: Network turn timer tick: pulse the turn window while a multiplayer
         game is running and the timers are not held.
// ORIGINAL: 0x0050EF10 ?turn_timer@@YAXH@Z 0x0050EF10-0x0050EF42 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ef10 archive unit. Three
//   nested `if`s, not chained conditions - the image tests each in turn with
//   its own jump to the shared `ret`. MultiWindow.draw(1) is the
//   last statement, so VC6 tails it after loading the receiver immediate.
// size      50 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00479330 0x005D5A70
Return Value: n/a
Status: Complete
*/
void __cdecl turn_timer(int a1) {
    if (IsMultiplayerNet != 0) {
        if (TimerHoldFlag == 0) {
            if (TurnRedrawPending != 0) {
                CouncilWindow.GraphicWin::redraw();
            }
            MultiWindow.draw(1);
        }
    }
}


/*
Purpose: Cancel the go mode: hide the cover window, stop the go timer, then
         reset the mode state and cursor of every live map window and of the
         selected one.
// ORIGINAL: 0x0050EF50 ?go_reset@@YAXXZ 0x0050EF50-0x0050F166 BYTE_EXACT
// size      534 bytes
// prototype
// callers   7   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B4A20 0x005EC7C0 0x005EDCD0 0x00616780
// indirect  0x0050EFF6 0x0050F014 0x0050F126 0x0050F144
// MEASURED 2026-08-29: /O2 best at 10/137 (0.842 similar). Image
//   instructions 0..7 agree - prologue, the cover window and
//   its direct `call Win::hide`. The counted divergence is the catalogued
//   MEASURED-inline ceiling: `GoTimer->close()` is inlined here in full
//   (stop()'s two kill branches plus the nine field clears, writing
//   0x939E60..0x939E84 directly) where the image makes one plain
//   `call 0x616780`. time.h keeps close() in-class BECAUSE ~Time must
//   inline it, so an out-of-line spelling would regress that claim; VC6
//   honours no per-site noinline. Everything after is that block's
//   register-allocation shadow (the walking cursor lands in edi instead of
//   esi), not a spelling defect this file can reach.
// TRIED: the dead cursor guard as a cached local (`cursor_sprite != 0`)
//   - same 10/137. The image loads the sprite value once for the two
//   equality compares (3D imm at 0x0050F097/0x0050F09E) but re-reads the
//   FIELD for the dead arm (39 at 0x0050F0A5), so the body re-reads it.
// MEASURED 2026-09-03, after the four start/pulse ladders landed: now
//   9/137 (0.864) at /O2. Re-derived the ceiling from scratch and it holds,
//   with one new fact - the image DOES inline other helpers into this body
//   (`go_cover_window()` folds to `mov ecx, 0x937118` at 0x0050EF57, and
//   `TutWinBaseWindow.is_visible()` expands inline in every caller), so
//   VC6's refusal here is specific to `Time::close`, not a general
//   no-inlining TU. The shape that explains all four sites - close()
//   inlined into ~Time, called out-of-line from stop_timers AND from here -
//   is that go_reset's original .cpp never saw close()'s body. This tree
//   merged the two units into src/time.cpp, so the body is always visible
//   and VC6 always expands it (+34 instructions, 155 against 137).
// TRIED: `#pragma inline_depth(0)` ahead of go_reset - it suppressed the
//   close() expansion all right but is a per-function switch, so it also
//   un-inlined go_cover_window() (the folded `mov ecx, 0x937118` became a
//   real call pair) and every is_visible() expansion, landing at 3/137.
//   Not a per-site lever, and not source. Reverted.
// TRIED: making close() an out-of-line member defined below go_reset in
//   this file - would hide the body from go_reset while leaving it visible
//   to ~Time, but it also hides it from console.cpp:875 and netmsg.cpp:77,
//   which are not mine to regress, and it does nothing for the prologue
//   below. Not taken.
//   The second, independent wall is the prologue: the image opens
//   `push ebp / mov ebp, esp / push ecx` (catalogue flag `frame`) and this
//   build's /O2 omits the frame pointer, so instructions 0..1 differ before
//   anything else. No flag set in the axis reaches both - the /Oy- sets fix
//   the prologue and lose the body (17/137 best).
Return Value: n/a
Status: Complete
*/
void __cdecl go_reset() {
    ConsoleGlobal.cover_window_.Win::hide();
    GoTimer.close();

    for (int index = 0; index < static_cast<int>(MapWinTableSlots); ++index) {
        MapWin **const slot = &MapWinTable[index];
        MapWin *const window = *slot;
        if (window != nullptr &&
            (index == 0 || window->view_active_ != 0) &&
            index != MapWinSelectedSlot &&
            window->field_1DD80_ != 0) {
            // The FIRST store runs off the window the tests loaded; the rest
            // re-read the slot each time, as the image re-reads [esi] for
            // every store after the first (0x0050EFB6 on).
            window->field_1DD88_ = 1;
            (*slot)->field_1DD80_ = 0;
            (*slot)->field_1DD8C_ = 0;
            (*slot)->field_1DD84_ = 0;
            (*slot)->field_1DE0C_ = -1;

            // Capture-loss dispatch may replace the slot; the next dispatch
            // reloads it (50EFF9 versus 50F007 in the original).
            (*slot)->release_pointer();
            (*slot)->on_update();
            ConsoleStatusWin.set_title(0);
        }
    }

    // Slot 0's cursor first, unconditionally.
    MapWin *const primary = MapWinTable[0];
    if (primary->cursor_sprite_ != 0 || primary->cursor_resource_id_ != 0x7F00) {
        primary->GraphicWin::set_cursor(0x7F00);
        primary->cursor_resource_id_ = 0x7F00;
        primary->cursor_sprite_ = 0;
    }

    if (MapWinSelectedSlot >= 0) {
        // The slot walks two registers in the image - the window loaded once
        // for the cursor work, the slot's own address kept for the reset
        // stores below (`lea esi, [eax*4 + 0x7d3c3c]` at 0x0050F08A).
        MapWin *const window = MapWinTable[MapWinSelectedSlot];
        MapWin **const slot = &MapWinTable[MapWinSelectedSlot];
        // Only the two go-mode cursor sprites are replaced by the default.
        // The sprite tests read it through the table expression, so the
        // dead `!= 0` arm re-reads the field (0x0050F0A5) instead of CSE.
        Sprite *const cursor_sprite = MapWinTable[MapWinSelectedSlot]->cursor_sprite_;
        if ((cursor_sprite == &g_CURSOR_SPRITES[1] ||
             cursor_sprite == &g_CURSOR_SPRITES[9]) &&
            (window->cursor_sprite_ != 0 || window->cursor_resource_id_ != 0x7F00)) {
            window->GraphicWin::set_cursor(0x7F00);
            window->cursor_resource_id_ = 0x7F00;
            window->cursor_sprite_ = 0;
        }
        (*slot)->field_1DD88_ = 1;
        (*slot)->field_1DD80_ = 0;
        (*slot)->field_1DD8C_ = 0;
        (*slot)->field_1DD84_ = 0;
        (*slot)->field_1DE0C_ = -1;

        (*slot)->release_pointer();
        (*slot)->on_update();
        ConsoleStatusWin.set_title(0);
        MapWinSelectedSlot = -1;
    }
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050F170 ?go_timer@@YAXH@Z 0x0050F170-0x0050F3C4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050f170.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
void __cdecl go_timer(int selection_mode) {
    GoTimer.close();
    if (MapWinSelectedSlot < 0 || TimerHoldFlag != 0) return;

    MapWin *&window = MapWinTable[MapWinSelectedSlot];
    int mouse_x;
    int mouse_y;
    window->get_mouse_pos(&mouse_x, &mouse_y);

    const uint8_t control_down = static_cast<uint8_t>(
        static_cast<uint16_t>(GetAsyncKeyState(VK_CONTROL)) >> 8);
    // The key branch defaults to cursor 1. With Control held and no active
    // cursor, a clear 0x23D1C selects cursor 9; map-wait mode also forces 9.
    int mode = control_down && ConsoleGlobal.cursor_active_ == 0 &&
               ConsoleGlobal.field_23D1C_ == 0 ? 9 : 1;

    if ((window->field_1DD70_ & 0x40000U) == 0) {
        if (mode == 1 && ConsoleGlobal.cursor_active_ == 0 &&
            ConsoleGlobal.field_23D1C_ == 0) {
            int vehicle = ConsoleGlobal.selected_vehicle_;
            if (vehicle < 0 ||
                ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_] != Vehs[vehicle].x ||
                Vehs[vehicle].y != ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_] ||
                veh_selectable(vehicle) == 0 || Vehs[vehicle].order != 0 ||
                (Vehs[vehicle].state & 0x4200U) != 0) {
                vehicle = veh_find(
                    ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
                    ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_],
                    ConsoleGlobal.faction_id_, -1);
                if (vehicle >= 0 && veh_selectable(vehicle) != 0 &&
                    Vehs[vehicle].order == 0 &&
                    (Vehs[vehicle].state & 0x200U) == 0 &&
                    (Vehs[vehicle].state & 0x4000U) != 0) {
                    ConsoleGlobal.selected_vehicle_ = vehicle;
                } else {
                    go_reset();
                    return;
                }
            }
        }
    } else {
        mode = 9;
    }

    MapWin *const target = window;
    Sprite *const cursor = &g_CURSOR_SPRITES[mode];
    if (target->cursor_sprite_ != cursor) {
        target->set_cursor(cursor, CursorHotspotX[mode], CursorHotspotY[mode]);
        target->cursor_sprite_ = cursor;
    }

    window->grab_pointer();
    ConsoleGlobal.field_23D10_ = mode;
    ConsoleGlobal.cover_window_.fill(9);
    ConsoleGlobal.cover_window_.show(0);
    window->field_1DD80_ = 1;
    window->field_1DD84_ =
        (ConsoleGlobal.cursor_active_ == 0 && ConsoleGlobal.field_23D1C_ != 0)
            ? 1U : 0U;
    window->on_mouse_move(mouse_x, mouse_y);
}

/*
Purpose: Start global timers.
// ORIGINAL: 0x0050F3D0 ?start_timers@@YAXXZ 0x0050F3D0-0x0050F431 BYTE_EXACT
// size      97 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616350
// LEVER (2026-08-29, timer conversion): the six timers are real `Time`
// objects (time.h/time.cpp) instead of `Time *const` bindings into their
// image addresses - the image's own receivers are the objects' ADDRESSES
// (`mov ecx, 0x915688` here), which `BlinkTimer.start(...)` on a real
// object reproduces; the claim re-measured BYTE_EXACT after, as did
// stop_timers, the three timer callbacks and game.cpp's inlined stop
// expansions.
Return Value: n/a
Status: Complete
*/
void __cdecl start_timers() {
    BlinkTimer.start(blink_timer, 1, 150, 150);
    Blink2Timer.start(blink2_timer, 2, 100, 100);
    LineTimer.start(line_timer, 3, 100, 100);
    TurnTimer.start(turn_timer, 4, 500, 500);
}

/*
Purpose: Stop global timers.
// ORIGINAL: 0x0050F440 ?stop_timers@@YAXXZ 0x0050F440-0x0050F45E BYTE_EXACT
// body      src/console.cpp
// size      30 bytes
// prototype 
// callers   6   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616780
Return Value: n/a
Status: Complete
*/
// Historical: an inline definition plus a third-TU forwarder once matched
// selected caller boundaries. The canonical implementation now lives here.
// LEVER: the `time_close_call` forwarder (declared in time.h, defined in
//   flatbutton.cpp). 0 of 6 agreeing, 90 instructions against the image's 6,
//   because close()'s in-class body was expanded at all three sites - the
//   image emits three `mov ecx, <addr>` + `call Time::close`, the last a tail
//   `jmp`. The body is in a header, so it is the same tokens in time.cpp and
//   in game.cpp, and the image wants the two OPPOSITE things from those two
//   units: `call` here, `expand` in system_init 0x58F040 and game_reload
//   0x58F450. No declaration of close() can give both - moving close()'s
//   definition out of the class into this .cpp (TRIED, see 0x00616780) left
//   this unit still able to see it and changed nothing here. A forwarder
//   whose body lives in a THIRD unit breaks the symmetry: this COMDAT emits
//   the real `call`, and the two game.cpp sites are unchanged by it (35 of
//   135 and 19 of 44 agreeing, before and after). Historical result: 6 of 6,
//   BYTE_EXACT.
void __cdecl stop_timers() {
    Blink2Timer.close();
    BlinkTimer.close();
    LineTimer.close();
    // missing MP TurnTimer. TODO: In future determine if timer should be closed on stop
}

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
    blink2_counter_ = 0;
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

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 4 bytes.
// ORIGINAL: 0x0050F640 nullsub_185 0x0050F640-0x0050F643 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0050F640-0x0050F643 return without modifying EAX; callee stack cleanup 4 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0050F630 sub_50f630 (src/statuswin.cpp) and 0x0050F650 ?clear_group@Console@@QAEXXZ (src/console.cpp); Console constructor 0x0050F460 and Console members after it bound the band"]}
// symbol    ?nullsub_185@@YGXH@Z
// kind      game
void __stdcall nullsub_185(int) {
}

/*
Purpose: Tear down the game console - its own Buffer/Time/Menu/Sprite
         members, its own extra embedded GraphicWin, then the MapWin
         fields it holds directly (no separate call into MapWin's own
         destructor).
// ORIGINAL: 0x0051D9F0 ??1Console@@QAE@XZ 0x0051D9F0-0x0051DC10;0x0065D0E0-0x0065D220 BYTE_EXACT
// symbol    ??1Console@@UAE@XZ
// body      src/console.h
//
// HOMED INTO THE CLASS 2026-08-30: the hand-spelled body this spot carried
// measured MISMATCH (7/123) because everything but `close()` is
// compiler-owned - the EH frame, the vtable/vtordisp re-stores on the
// vbase, the member teardown of the GraphicWin/Buffer/Time/Menu/Sprite
// members console.h now declares, and MapWin's own base teardown inlined
// (clear(0) and MapWin's carved members). Historically, the in-class
// `~Console() { close(); }` was interpreted as the original image's
// ??_DConsole@@QAEXXZ base-object phase (the catalogue's ??1 name sits on the
// base flavor; the original complete ??1Console phase is 0x0051DDF0,
// `lea esi,[ecx-0x23D94]` adjust, a call to this, then
// ??1GraphicWin on the vbase, then the conditional operator delete, with
// the deleting wrapper ??_GConsole at 0x0051DDE0 before it). The funclet
// span 0x0065D0E0-0x0065D220 is that historical EH phase's own. The current
// natural owner-TU compiler symbol is ??1Console@@UAE@XZ, as recorded by the
// unchanged source symbol override; this comment does not claim a current
// ??_DConsole emission.
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
// Original add_site independently passes the same zero-fill empty-string address as danger's second string argument; see its pinned call at 0x00579B95..0x00579B9F.
// RECOVERY_LITERAL: v1 0x00939FF0 0x00939FF1 ??_C@_00A@?$AA@ 0x00579B70
            danger(CursorNextDangerMsg1, "", x_coord,
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
                static_cast<const volatile MapWin *>(window)->view_active_;
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
// RECOVERY_ASSOCIATED: v1 switch_targets 0x005150D0 0x005153E0 0x005153F8 0x00515352
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
                primary->center_x_ = tile_x;
                primary->center_y_ = tile_y;
                primary_map_hit = true;
            }
        }

        if (!primary_map_hit) {
            for (int slot = 0; slot < static_cast<int>(MapWinTableSlots);
                 ++slot) {
                MapWin *const window = MapWinTable[slot];
                if (window == nullptr ||
                    (slot != 0 && window->view_active_ == 0) ||
                    window->zoom_offset_ >= 0) {
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
                window->center_x_ = candidate_x;
                window->center_y_ = candidate_y;
                selected_window = slot;
                break;
            }
        }
    }

    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == nullptr ||
            (slot != 0 && window->view_active_ == 0) ||
            (selected_window >= 0 && slot != selected_window)) {
            continue;
        }
        switch (zoom_command) {
        case 0x203:
            if (window->zoom_offset_ < 0x10) {
                window->zoom_offset_ += 2;
            }
            break;
        case 0x204:
            if (window->zoom_offset_ > -0xE) {
                window->zoom_offset_ -= 2;
            }
            break;
        case 0x205:
            window->zoom_offset_ = 0;
            break;
        case 0x206:
            window->zoom_offset_ = -6;
            break;
        case 0x207:
            window->zoom_offset_ = 0x10;
            break;
        case 0x208:
            window->zoom_offset_ = -0xE;
            break;
        default:
            break;
        }
        window->draw_map(1);
    }
    WorldClimateWorldWin.GraphicWin::redraw();
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
Purpose: Reset the console - clear its scalar state and history slot zero,
         then close the Buffer, Time and GraphicWin
         subobjects and clear the map.
The historical forwarding notes below predate the separate timer lifecycle TU;
the timer call now uses the existing Time member directly.
// ORIGINAL: 0x0051D7D0 ?close@Console@@QAEXXZ 0x0051D7D0-0x0051D88E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Console owns buffer_ at2247C, time_ at22A04 and alpha_menu_ at22A2C; AlphaMenu derives nonvirtually from Menu/GraphicWin.", "Pinned51D7D0 tail selects receiver+2247C for5D7470, receiver+22A04 for616780, receiver+22A2C for5D4E40, then clear(0); this closes members without substituting destruction."], "reason": "Select the constructed Console Buffer and AlphaMenu GraphicWin base through their real members; preserve scalar resets, repeated history-slot-zero writes and Buffer/Time/GraphicWin/clear ordering."}
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
    field_23BC8_ = 0xffffffff;
    cursor_active_ = 0;
    field_23BE4_ = 0;
    field_23BE8_ = 0;
    field_23BF0_ = 0;
    field_23BF4_ = 0;
    survey_mode_ = 0;
    blink2_counter_ = 0;
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
    buffer_.close();
    time_.close();
    static_cast<GraphicWin &>(alpha_menu_).close();
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
// Native 0x0068A5D0 is exactly the writable initialized text plus terminator;
// this constant-initialized source array adds no global constructor.
// RECOVERY_OBJECT: v2 0x0068A5D0 0x0068A5E0 ?CursorNextDangerMsg1@@3PADA 0x005109B0
char CursorNextDangerMsg1[16] = "Bad Cursor Move";
// The 0x247A8-byte Console runs to its virtual GraphicWin base's end:
// ??0Console builds that base at +0x23D94 (0x0050F494) and the 0xA14-byte
// GraphicWin ends at 0x00939E58. Formerly separate globals inside that
// extent are Console members, constructed or cleared by ??0Console:
// CoverWindow (+0x21A68, ??0GraphicWin at 0x0050F4B3), Blink2Counter
// (+0x23C04, cleared at 0x0050F5B6) and MessageWinMessageCount (+0x23D20,
// between constructor-initialized +0x23D18 and the +0x23D28 Sprite).
// RECOVERY_OBJECT: v2 0x009156B0 0x00939E58 ?ConsoleGlobal@@3VConsole@@A 0x0050E850
// RECOVERY_OBJECT: v3 0x00937118 ?CoverWindow@@3VGraphicWin@@A member ?ConsoleGlobal@@3VConsole@@A cover_window_
// RECOVERY_OBJECT: v3 0x009392B4 ?Blink2Counter@@3HA member ?ConsoleGlobal@@3VConsole@@A blink2_counter_
// RECOVERY_OBJECT: v3 0x009393D0 0x009393D4 ?MessageWinMessageCount@@3HA 0x00471530 member ?ConsoleGlobal@@3VConsole@@A message_count_
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
// Original grouped ctor at 0x0050E9CE and cleanup at 0x0050E9FA use the distinct Go timer receiver; start only.
// RECOVERY_OBJECT: v1 0x00939E60 ?GoTimer@@3VTime@@A
// Original 0x0050E9D8/0x0050E9F0 and start_timers at 0x0050F426 use this timer receiver; no extent/lifetime claim.
// RECOVERY_OBJECT: v1 0x00915628 ?TurnTimer@@3VTime@@A
Time BlinkTimer,   // 0x00915688
     Blink2Timer,  // 0x00939EB0
     LineTimer,    // 0x00915658
     GoTimer,      // 0x00939E60
     TurnTimer;    // 0x00915628

// Compiler-generated source-owned lifecycle entry; no handwritten wrapper.
/*
// ORIGINAL: 0x0050E9B0 sub_50e9b0 0x0050E9B0-0x0050E9EE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bcad94a018ff94737abb85df10d24ca7aa405b1a:src/recovered/0050e9b0.cpp", "pinned:terranx_original lifecycle0x50e9b0", "historical source owner before rehome:src/time.cpp"], "reason": "Generated complete five-Time-object initializer composes the real constructor group and destructor registration. Delegated generated helpers are current lowering, not a claim of one-to-one byte layout. Preserve complete original spans, catalogue names, claims and historical lessons; current compiler emission is not a fresh grant or linked selection proof."}
// symbol    _$E26
*/

// Historical seed finding: Native construction order: Blink, Blink2, Line, Go, Turn; registers the aggregate reverse teardown.
// Historical seed finding: ConsoleTimer startup50E980 is a separate original group and is not annexed by this binding.

// Compiler-generated source-owned lifecycle entry; no handwritten wrapper.
/*
// ORIGINAL: 0x0050E9F0 sub_50e9f0 0x0050E9F0-0x0050EA22 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bcad94a018ff94737abb85df10d24ca7aa405b1a:src/recovered/0050e9f0.cpp", "pinned:terranx_original lifecycle0x50e9f0", "historical source owner before rehome:src/time.cpp"], "reason": "Generated destructor for the single five-Time-object declaration preserves native reverse destruction and timer shutdown effects. Preserve complete original spans, catalogue names, claims and historical lessons; current compiler emission is not a fresh grant or linked selection proof."}
// symbol    _$E24
*/

// Historical seed finding: Native destruction order: Turn, Go, Line, Blink2, Blink.
// Historical seed finding: The former separate per-object declarations did not own the complete original aggregate.
int Console::game_data_private_word(FILE *file, int write_mode) {
    if (write_mode != 0) {
        message_count_ = 0;
    }
    return game_data_transfer_block(file, &message_count_, sizeof(message_count_));
}


// Shared four-field rectangle written by original Console51D760/51D890.
// Start binding only: no complete original allocation/lifetime claim.
// RECOVERY_OBJECT: v1 0x00939EE8 ?ConsoleMenuBounds@@3UtagRECT@@A
RECT ConsoleMenuBounds;

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

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x00515760 nullsub_186 0x00515760-0x00515761 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x00515760-0x00515761 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x00515740 ?toggle_dest@Console@@QAEXXZ (src/console.cpp) and 0x00515770 ?chat@Console@@QAEXH@Z (src/console.cpp)"]}
// symbol    ?nullsub_186@@YAXXZ
// kind      game
void __cdecl nullsub_186() {
}

// Original .data text inputs, with separate writable owners for each
// address even when another input has the same contents.
// RECOVERY_OBJECT: v2 0x0068A9CC 0x0068A9D7 ?ConsoleStrMenuSimple@@3PADA 0x00516970
char ConsoleStrMenuSimple[] = "MENUSIMPLE";  // 0x0068A9CC
// RECOVERY_OBJECT: v2 0x0068A9C0 0x0068A9CB ?ConsoleStrMenuDetail@@3PADA 0x00516930
char ConsoleStrMenuDetail[] = "MENUDETAIL";  // 0x0068A9C0
// The caption/label popup passes this separate body-key object at 0x0068AA0C.
// RECOVERY_OBJECT: v1 0x0068AA0C _StrScenarioObjectivesPopup
static const char StrScenarioObjectivesPopup[] = "OBJECTIVES";
// Its title argument uses this distinct writable object at 0x0068AA18.
// RECOVERY_OBJECT: v1 0x0068AA18 _StrScenarioObjectivesTitle
static char StrScenarioObjectivesTitle[] = "SCENARIO";
// One Console-owned confirmation string is shared by the original quit/new/menu/key callers.
// RECOVERY_OBJECT: v1 0x0068A7D4 _ConsoleReallyWimpOut
static const char ConsoleReallyWimpOut[] = "REALLYWIMPOUT";
// RECOVERY_OBJECT: v2 0x0068A8E0 0x0068A8ED ?ConsoleStrTerraSurvey2@@3PADA 0x00515530
char ConsoleStrTerraSurvey2[] = "TERRASURVEY2";  // 0x0068A8E0
// RECOVERY_OBJECT: v2 0x0068A8D4 0x0068A8DF ?ConsoleStrTechChange@@3PADA 0x00515010
char ConsoleStrTechChange[] = "TECHCHANGE";  // 0x0068A8D4
// RECOVERY_OBJECT: v2 0x0068AA7C 0x0068AA8B ?ConsoleStrAcLogoSm@@3PADA 0x00517180
char ConsoleStrAcLogoSm[] = "ac_logo_sm.pcx";  // 0x0068AA7C
// RECOVERY_OBJECT: v2 0x0068AA8C 0x0068AA9C ?ConsoleStrVersionControlX@@3PADA 0x00517180
char ConsoleStrVersionControlX[] = "VERSIONCONTROLX";  // 0x0068AA8C
// The pinned .data cells were mutable char* slots initialized to these
// distinct writable text arrays; callers share the pointer objects.
// RECOVERY_OBJECT: v2 0x00691B40 0x00691B4C ?ConsoleVersionReleaseText@@3PADA 0x00517180
char ConsoleVersionReleaseText[] = "RELEASE 2.0";
// RECOVERY_OBJECT: v2 0x00691B4C 0x00691B58 ?ConsoleVersionBuildDateText@@3PADA 0x00517180
char ConsoleVersionBuildDateText[] = "20-Dec-1999";
// RECOVERY_OBJECT: v2 0x00691870 0x00691874 ?ConsoleVersionRelease@@3PADA 0x00517180
char *ConsoleVersionRelease = ConsoleVersionReleaseText;
// RECOVERY_OBJECT: v2 0x00691874 0x00691878 ?ConsoleVersionBuildDate@@3PADA 0x00517180
char *ConsoleVersionBuildDate = ConsoleVersionBuildDateText;


// RECOVERY_OBJECT: v2 0x0068A8C8 0x0068A8D3 ?ConsoleStrTechRandom@@3PADA 0x00514F50
char ConsoleStrTechRandom[] = "TECHRANDOM";  // 0x0068A8C8
// RECOVERY_OBJECT: v2 0x0068A93C 0x0068A945 ?ConsoleStrHostTime@@3PADA 0x00515900
char ConsoleStrHostTime[] = "HOSTTIME";  // 0x0068A93C
// RECOVERY_OBJECT: v2 0x0068A948 0x0068A953 ?ConsoleStrCustomTime@@3PADA 0x00515900
char ConsoleStrCustomTime[] = "CUSTOMTIME";  // 0x0068A948

// THE TIME-CONTROLS STATE the CUSTOMTIME dialog drives: 0x009A64E0 is the
// preselected row (start's value slot copies it into the popup) and
// 0x0094F25C..0x0094F274 are alpha.h's TimeControl[5].turn/.base/.unit/
// .event/.extra/.refresh/.accumulated. All seven editable values are seeded
// before CUSTOMTIME and re-parsed on accept.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.


// Existing typed owner cells: NetDaemonNet's shared assignment word and
// Popup's full-width checkbox state.
inline unsigned int &console_turns_assigned() {
    return NetDaemonNet.primary_state_.leading_words_[0];
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


// ORIGINAL: 0x005105C0 ?selecting@Console@@QAEHHH@Z 0x005105C0-0x005107D5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 0x005105C0..0x005107D5 and its retired transcription preserved in Git history.", "Original early mode1/mode0 returns skip pumping. Ordinary cursor mode is captured after valid_patrol; each called receiver is retained for cache writeback while subsequent table slots and hotspots are read live.", "Original68FAF0 is MapLongitude, the half-width used by the wrap test; valid initialized vehicle/window state and representable coordinate arithmetic are required by the original."], "reason": "Console owns selection-mode admission and the shared map-window cursor update. Recover genuine receivers and cursor globals; the range and patrol refusals both select loaded sprite5."}
// The image repeats this map-window cursor walk at each of selecting's
// three exits, each followed by do_non_input.
static inline void selecting_cursor(int shape) {
    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window != nullptr && (slot == 0 || window->view_active_ != 0)) {
            Sprite *const sprite = &g_CURSOR_SPRITES[shape];
            if (window->cursor_sprite_ != sprite) {
                window->GraphicWin::set_cursor(sprite,
                    CursorHotspotX[shape], CursorHotspotY[shape]);
                window->cursor_sprite_ = sprite;
            }
        }
    }
    do_non_input();
}

int Console::selecting(int tile_x, int tile_y) {
    if (field_23D10_ == 1) {
        return 1;
    }
    if (field_23D10_ != 0) {
        if (field_23D14_ >= 0) {
            const int vehicle_y = Vehs[selected_vehicle_].y;
            const int vehicle_x = Vehs[selected_vehicle_].x;
            const int abs_dx = abs(tile_x - vehicle_x);
            const int dx = ((MapIsFlat & 1) == 0 && abs_dx > MapLongitude)
                ? MapLongitudeBounds - abs_dx : abs_dx;
            // The original SAR floors negative odd sums rather than truncating.
            if ((dx + abs(tile_y - vehicle_y)) >> 1 > field_23D14_) {
                selecting_cursor(5);
                return 1;
            }
        }
        if (field_23D10_ == 8 && !valid_patrol(selected_vehicle_, tile_x, tile_y)) {
            selecting_cursor(5);
            return 1;
        }
        selecting_cursor(field_23D10_);
        return 1;
    }
    return 0;
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
// RECOVERY_LITERAL: v1 0x0068A994 0x0068A9A1 ??_C@_0N@EBGE@LANDMARKNONE?$AA@ 0x00516490
        NetMsgPrimary.pop("LANDMARKNONE", 5000, 0, nullptr);
        return;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 0x80) == 0 &&
        valid_landmark(x_coord, y_coord, ConsoleGlobal.faction_id_) == 0) {
        NetMsgPrimary.pop(EraseLandmarkDeniedKey, 5000, 0, nullptr);
        return;
    }

    parse_says(0, MapLandmark[landmark_id].name, -1, -1);
// RECOVERY_LITERAL: v1 0x0068A9B0 0x0068A9BD ??_C@_0N@MOHD@LANDMARKKILL?$AA@ 0x00516490
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
        if (window != nullptr && (slot == 0 || window->view_active_ != 0)) {
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

// ORIGINAL: 0x005107E0 ?selected@Console@@QAEXXZ 0x005107E0-0x00510899 FILE BYTE_EXACT
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
            if (window != nullptr && (slot == 0 || window->view_active_ != 0)) {
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

// ORIGINAL: 0x00515EF0 ?toggle_patrol@Console@@QAEXXZ 0x00515EF0-0x00515F70 FILE BYTE_EXACT
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
    for (int slot = 0; slot < 8; ++slot) {
        if (MapWinTable[slot] != 0 &&
            (slot == 0 || MapWinTable[slot]->view_active_ != 0)) {
            if (mask == 0) {
                MapWinTable[slot]->field_1DD70_ |= 0x400000;
                MapWinTable[slot]->draw_base_dest(1);
            } else {
                MapWinTable[slot]->field_1DD70_ &= 0xffbfffff;
                MapWinTable[slot]->draw_map(1);
            }
        }
    }
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
    X_pop(ConsoleStrMenuSimple, 0);
}

// ORIGINAL: 0x00516930 ?menu_detail@Console@@QAEXXZ 0x00516930-0x00516961 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x00516930 seed sets GameMorePreferences bit 0x200000, calls the Console member at +0x22A2C using its current vehicle, and passes the image string at 0x0068A9C0 to X_pop.", "src/console.h identifies alpha_menu_ at Console +0x22A2C and selected_vehicle_; src/console.cpp already binds the matching MENUDETAIL text as ConsoleStrMenuDetail, and alpha.h names GameMorePreferences."], "reason": "Express menu detail activation through the existing preference flag, typed AlphaMenu member, Console selection state and established string binding."}
void Console::menu_detail() {
    GameDataStateStorage.fields_.game_more_preferences_ |= 0x200000;
    alpha_menu_.check(selected_vehicle_);
    X_pop(ConsoleStrMenuDetail, 0);
}

// ORIGINAL: 0x00516C50 ?show_scen_objectives@Console@@QAEXXZ 0x00516C50-0x00516D98 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 0x00516C50 seed parses ObjectiveReqVictory, ObjectivesSuddenDeathVictory and EndingMissionYear before processing faction-specific names.", "src/faction.h defines Players as 0x59C Player records: the seed's 0x00946A50, 0x00946A9C, 0x00946A84, 0x00946D4C, 0x00946D50 and 0x00946D34 references correspond to Player::is_leader_female, title_leader, name_leader, noun_gender, is_noun_plural and noun_faction.", "src/game.h owns GameRules and victory settings; general.h owns GenderDefault, PluralityDefault, parse_num, parse_says and say_year; strings.h owns StringTemp. The original popup keys at 0x0068AA0C and 0x0068AA18 are OBJECTIVES and SCENARIO, respectively.", "The seed's 0x005BF330 caption/label/callback call is represented by xpops.h's x_pop_caption_label, whose implementation forwards with the defaults (-1, no override, flags 0) to the typed full popup."], "reason": "Render scenario objectives through the existing faction, game-state, parser, scratch-buffer and popup owners, preserving the pinned OBJECTIVES and SCENARIO key values."}
void Console::show_scen_objectives() {
    parse_num(0, ScenarioSave.objective_req_victory);
    parse_num(1, ScenarioSave.objectives_sudden_death_victory);
    parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000));
    parse_num(3, ScenarioSave.ending_mission_year);

    GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
    GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
    PluralityDefault = 0;
    parse_says(2, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
    GenderDefault = Players[ConsoleGlobal.faction_id_].noun_gender;
    PluralityDefault = Players[ConsoleGlobal.faction_id_].is_noun_plural;
    parse_says(3, Players[ConsoleGlobal.faction_id_].noun_faction, -1, -1);

    StringTemp[0] = 0;
    say_year(StringTemp);
    parse_says(4, StringTemp, -1, -1);
    parse_says(0, StringTemp, -1, -1);
    X_pop(StrScenarioObjectivesTitle, StrScenarioObjectivesPopup, 0);
}

// Fixed-slot bindings carried from 00510ce0.cpp

// ORIGINAL: 0x00510CE0 ?ready_search@Console@@QAEHH@Z 0x00510CE0-0x00510F48 FILE BYTE_EXACT
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

    if (veh_id >= 0 && Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_) {
        coordA = Vehs[veh_id].x;
        coordB = Vehs[veh_id].y;
    } else {
        coordA = ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
        coordB = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
        veh_id = -1;
    }

    int outerIdx = (IsMultiplayerNet != 0) ? -1 : 0;

    while (outerIdx < 3) {
        int vehCount = GameDataStateStorage.fields_.vehicle_current_count_;

        if ((IsMultiplayerNet != 0) && (outerIdx == 0) && (focus_vehicle_ >= 0) &&
            (focus_vehicle_ < vehCount) &&
            (Vehs[focus_vehicle_].faction_id == ConsoleGlobal.faction_id_) &&
            (veh_ready(focus_vehicle_) != 0)) {
            result = focus_vehicle_;
            focus_vehicle_ = -1;
            goto searchDone;
        }

        for (int innerIdx = 0;
             innerIdx < GameDataStateStorage.fields_.vehicle_current_count_;
             innerIdx++) {
            Veh &vehicle = Vehs[innerIdx];

            if (veh_ready(innerIdx) == 0) continue;

            if (outerIdx >= 0 ||
                (vehicle.state & 0x200) != 0 ||
                (vehicle.state & 0x1000200) == 0x1000200 ||
                (vehicle.state & 0x4000) != 0 ||
                vehicle.order != 0) {

                if (vehicle.state & 2) continue;

                int vehX = vehicle.x;
                int vehY = vehicle.y;

                int score = vector_dist(x_dist(coordA, vehX), abs(coordB - vehY));

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
                if (outerIdx != 0 || clearIdx != selected_vehicle_ || veh_id < 0) {
                    Vehs[clearIdx].state &= 0xfffffffd;
                }
            }
        }

        outerIdx++;
    }

searchDone:
    field_23D24_ = IsMultiplayerNet != 0 && outerIdx < 0;
    return result;
}

// Fixed-slot bindings carried from 00510490.cpp

// ORIGINAL: 0x00510490 ?cursor_on@Console@@QAEXHH@Z 0x00510490-0x0051051D FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      141 bytes
// prototype void (__thiscall ?cursor_on@Console@@QAEXHH@Z)(Console* this, int, int)
// callers   5   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC740 0x005FCA30

void Console::cursor_on(int cursor_shape, int veh_id) {
    field_23D10_ = static_cast<uint32_t>(cursor_shape);
    field_23D14_ = veh_id;
    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window != nullptr && (slot == 0 || window->view_active_ != 0)) {
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
// Operands now follow VC6's load-b-first order (x: delta + coord, y: coord +
// delta), which fixes the loads; residual is esi/edi swapped (this vs new_x).


void Console::move_cursor(int direction) {
    int new_x = wrap_map_x(
        VehMoveDeltaX[direction] +
        ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_]);
    int new_y = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_] +
                VehMoveDeltaY[direction];
    if (!set_cursor(new_x, new_y)) {
        console_map_win()->focus(new_x, new_y);
    }
}

// ORIGINAL: 0x005147E0 ?set_cursor@Console@@QAEXHH@Z 0x005147E0-0x0051487D FILE BYTE_EXACT
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
    if (!on_map(x_coord, y_coord)) {
        return 1;
    }
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
        TutorialWindow.tut_map(ConsoleStrTerraSurvey2, ConsoleGlobal.cursor_history_x_[slot],
                               ConsoleGlobal.cursor_history_y_[slot], -1, nullptr, 0, -1, -1);
    }
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
    GameDataStateStorage.fields_.game_more_preferences_ = new_flags;
    AlphaIniPrefs.more_preferences = new_flags;
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
        if (window != nullptr && (slot == 0 || window->view_active_ != 0)) {
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
        parse_say(0, Technology[*researching].name, -1, -1);
        parse_num(0, *research);
        if (X_pop(ConsoleStrTechChange, (NullCallback)0) == 0) {
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
// Homed 2026-09-04. Its historical measurement was NO_COMPILE, not MISMATCH.
// The original writable .data pointer slots at 0x00691870/74 and their
// distinct text windows are now represented by shared source-owned objects.
// The caption slot at 0x00691B0C is ScriptPopupFile; the two popp strings are
// the existing source-owned arrays below.


void Console::show_version() {
    parse_says(0, ConsoleVersionRelease, -1, -1);
    parse_says(1, ConsoleVersionBuildDate, -1, -1);
    popp(ScriptPopupFile, ConsoleStrVersionControlX, 0, ConsoleStrAcLogoSm,
         (NullCallback)0);
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
            unsigned char state = static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
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
    // The pinned reads/writes at 96CDC0..96CDCC are the four named priority
    // fields at PlayersData + 0x3E0..0x3EC (VC6 CodeView offsets 992..1004).
    int flags = (PlayersData[faction_id].ai_growth != 0) ? 1 : 0;
    if (PlayersData[faction_id].ai_tech != 0) {
        flags |= 2;
    } else {
        flags &= ~2;
    }
    if (PlayersData[faction_id].ai_wealth != 0) {
        flags |= 4;
    } else {
        flags &= ~4;
    }
    if (PlayersData[faction_id].ai_power != 0) {
        flags |= 8;
    } else {
        flags &= ~8;
    }
    PopupWriteCheckState = static_cast<int>(flags);
    if (x_pop_short(ConsoleStrTechRandom, 1, (NullCallback)0) >= 0) {
        const signed char answer = static_cast<signed char>(PopupWriteCheckState);
        PlayersData[faction_id].ai_growth = answer & 1;
        PlayersData[faction_id].ai_tech = (answer & 2) >> 1;
        PlayersData[faction_id].ai_wealth = (answer & 4) >> 2;
        PlayersData[faction_id].ai_power = (answer & 8) >> 3;
        synch_ai(faction_id);
    }
}


/*
Purpose: Load a saved game: leave the base window, refuse outside an active
         network/PBEM session, then rebuild the desktop or reload the running
         game and redraw the world.
// ORIGINAL: 0x0051BBA0 ?load_proc@Console@@QAEXHH@Z 0x0051BBA0-0x0051BC62 FILE BYTE_EXACT
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
    if (GameSessionActive != 0) {
        if (game_reload(1, 0) != 0) {
            exit(3);
        }
    } else {
        desktop_init(0);
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
// ORIGINAL: 0x00513FC0 ?end_of_turn@Console@@QAEXXZ 0x00513FC0-0x0051404E FILE BYTE_EXACT
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

    while (ConsoleGlobal.cursor_active_ == 0 && field_23BE4_ != 0 &&
           field_23BE8_ != 0 && ExitTurnLoop == 0) {
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
    }

    field_23BE4_ = 0;
}

// ORIGINAL: 0x005165B0 ?zoom_to_upkeep_base@Console@@QAEXXZ 0x005165B0-0x005166C3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The retained 0x005165B0 seed selects the nearest own base carrying Base::state bit 0x10, using the current MapWin camera coordinates and wraps X on round maps.", "base.h names the 0x0097D040 array as Bases with 0x134-byte Base records; its x/y/faction/state members are at offsets 0/2/4/0x30, matching all seed reads.", "MapWin fields 0x1DD9C/0x1DDA0 supply the camera point; map.cpp binds MapLongitude/MapIsFlat/MapLongitudeBounds; tutwin.h binds the image's BaseWin at 0x006A7628 as TutWinBaseWindow."], "reason": "Choose the nearest base of ConsoleGlobal's current faction with the seed's wrapped-map distance calculation and pass its typed index to the established BaseWin zoom path."}
void Console::zoom_to_upkeep_base() {
    const MapWin *map_window = console_map_win();
    const int camera_x = map_window->center_x_;
    const int camera_y = map_window->center_y_;
    int best_distance = 99999;
    int best_base = -1;

    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        const Base &base = Bases[base_id];
        if (base.faction_id_current == ConsoleGlobal.faction_id_ &&
            (base.state & BSTATE_UNK_10) != 0) {
            const int base_x = base.x;
            const int base_y = base.y;
            const int distance =
                vector_dist(x_dist(camera_x, base_x), abs(camera_y - base_y));
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
        return;
    }

    if (console_turns_assigned() == 0) {
        NetMsgPrimary.pop(ConsoleStrHostTime, 5000, 0, 0);
        return;
    }

    time_controls_dialog(&popup);
    popup.set_value(GameDataStateStorage.fields_.time_controls_selection_);
    const int result = popup.exec(0, 0);
    if (result < 0) {
        return;
    }

    if (result == 5) {
        // The seven editable turn times go through the parser in this order,
        // then the accept path re-parses and clamps all seven fields of the
        // active TimeControl row.
        parse_field_number(0, TimeControl[5].base);
        parse_field_number(1, TimeControl[5].unit);
        parse_field_number(2, TimeControl[5].event);
        parse_field_number(3, TimeControl[5].turn);
        parse_field_number(4, TimeControl[5].extra);
        parse_field_number(result, TimeControl[5].refresh);
        parse_field_number(6, TimeControl[5].accumulated);

        popup.start(PopupStartCaption, ConsoleStrCustomTime, -1, nullptr, 0x44, nullptr);
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
}

// The splitter distances use a textual abs: its operand is expanded twice.
#define SPLIT_ABS(v) ((v) >= 0 ? (v) : -(v))

// True while any of the four splitter slots is armed (-1) for a drag. The
// image keeps each caller's `return 1` epilogue separate, which only an
// inline boundary around this test reproduces.
static inline int splitter_armed(int a, int b, int c, int d) {
    return a == -1 || b == -1 || c == -1 || d == -1;
}

// ORIGINAL: 0x0050F680 ?on_nc_hittest@Console@@QAEXHH@Z 0x0050F680-0x0050F951
// RECOVERY_BINDING: v1 {"evidence": ["The original 0x0050F680..0x0050F951 disassembly exits return EAX values 1 or 0; its retired transcription is preserved in Git history."], "reason": "Translate the Console non-client hit-test into its existing virtual method; the disassembly exits return EAX values 1 or 0, correcting the seed-only void prototype."}
// symbol    ?on_nc_hittest@Console@@UAEHHH@Z
int Console::on_nc_hittest(int x_coord, int y_coord) {
    // EAX=1 while a split line is armed, else 1 only over a caption box or a
    // splitter handle.
    if (splitter_armed(field_23D80_, field_23D84_, field_23D88_, field_23D8C_)) {
        return 1;
    }

    RECT caption_box = {0, 0xE, 0x26, 0x1F};
    caption_box.left = client_width() - 0x5b;
    caption_box.right = caption_box.left + 0x26;
    if (in_box(x_coord, y_coord, &caption_box)) {
        return 1;
    }
    if (x_coord > client_width() - border_thickness_) {
        if (y_coord >= track_height() - bottom_border() + caption_height_ &&
            y_coord <= client_height() - bottom_border()) {
            if ((y_coord >= track_height() - bottom_border() - sprite_b_.height() +
                     field_23D88_ + caption_height_ &&
                 y_coord <= track_height() - bottom_border() + field_23D88_ +
                     caption_height_ + sprite_b_.height()) ||
                (y_coord >= track_height() - bottom_border() - sprite_b_.height() +
                     field_23D8C_ + caption_height_ &&
                 y_coord <= track_height() - bottom_border() + field_23D8C_ +
                     caption_height_ + sprite_b_.height())) {
                return 1;
            }
        }
    } else if (y_coord > client_height() - bottom_border() &&
               x_coord >= border_thickness_ &&
               x_coord <= client_width() - border_thickness_) {
        if ((x_coord >= field_23D80_ - sprite_a_.width() + border_thickness_ &&
             x_coord <= field_23D80_ + border_thickness_ + sprite_a_.width()) ||
            (x_coord >= field_23D84_ - sprite_a_.width() + border_thickness_ &&
             x_coord <= field_23D84_ + border_thickness_ + sprite_a_.width())) {
            return 1;
        }
    }
    return 0;
}

// ORIGINAL: 0x0050F960 ?on_post_redraw_nc_buffer@Console@@QAEXPAUBuffer@@H@Z 0x0050F960-0x0050FB08
// RECOVERY_BINDING: v1 {"evidence": ["The original 0x0050F960..0x0050FB08 draws two splitter handles per orientation; its retired transcription remains in Git history."], "reason": "Render the two Console non-client splitter handles with their owned Sprite members and the Window client/outer geometry; the archived scaffold confirms two handles per orientation and the image buffer dimensions."}
// symbol    ?on_post_redraw_nc_buffer@Console@@UAEXPAVBuffer@@H@Z
void Console::on_post_redraw_nc_buffer(Buffer *buffer, int which) {
    if (buffer == nullptr) {
        return;
    }
    // The image recomputes the border and track geometry for every handle.
    if (which == 1) {
        Sprite *const handle = &sprite_b_;
        handle->draw(buffer, handle->transparent_index(),
                     buffer->dib_.bmiHeader.biWidth - handle->width(),
                     track_height() - sprite_b_.height() / 2 + field_23D88_ -
                         bottom_border(),
                     1, 1);
        handle->draw(buffer, handle->transparent_index(),
                     buffer->dib_.bmiHeader.biWidth - handle->width(),
                     track_height() - handle->height() / 2 + field_23D8C_ -
                         bottom_border(),
                     1, 1);
    } else if (which == 2) {
        Sprite *const handle = &sprite_a_;
        handle->draw(buffer, handle->transparent_index(),
                     field_23D80_ - sprite_a_.width() / 2 + border_thickness_,
                     -(buffer->dib_.bmiHeader.biHeight + handle->height()), 1, 1);
        handle->draw(buffer, handle->transparent_index(),
                     field_23D84_ - handle->width() / 2 + border_thickness_,
                     -(buffer->dib_.bmiHeader.biHeight + handle->height()), 1, 1);
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

    box.left = client_width() - 0x5b;
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

    // The nearer slot wins; each distance expands its operand twice.
    if (x_coord > client_width() - border_thickness_) {
        if (SPLIT_ABS(y_coord - (track_height() - bottom_border() + caption_height_ +
                                 field_23D88_)) <
            SPLIT_ABS(y_coord - (track_height() - bottom_border() + caption_height_ +
                                 field_23D8C_))) {
            field_23D88_ = -1;
        } else {
            field_23D8C_ = -1;
        }
        if (WinPointerOwner4 == 0) {
            take_pointer();
        }
    }

    if (y_coord > client_height() - bottom_border()) {
        if (SPLIT_ABS(x_coord - border_thickness_ - field_23D80_) <
            SPLIT_ABS(x_coord - border_thickness_ - field_23D84_)) {
            field_23D80_ = -1;
        } else {
            field_23D84_ = -1;
        }
        if (WinPointerOwner4 == 0) {
            take_pointer();
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
    int *target;

    // The clamps match on_nc_left_up; the dragged slot is redrawn without
    // its line, then the crossing drag line moves to the new position.
    if (field_23D80_ == -1 || field_23D84_ == -1) {
        target = field_23D80_ == -1 ? &field_23D80_ : &field_23D84_;
        *target = x_coord - border_thickness_;
        if (*target < sprite_a_.width() / 2) {
            *target = sprite_a_.width() / 2;
        }
        if (*target > client_width() - border_thickness_ * 2 -
                          sprite_a_.width() / 2) {
            *target = client_width() - border_thickness_ * 2 -
                      sprite_a_.width() / 2;
        }
        line_x = *target;
        line_y = field_23D88_;
        redraw_nc_buffer(2);
        *target = -1;
        update_nc_buffer(2);
    } else {
        // Both coordinates take x here, as in the image.
        line_x = x_coord;
        line_y = x_coord;
    }

    if (field_23D88_ == -1 || field_23D8C_ == -1) {
        target = field_23D88_ == -1 ? &field_23D88_ : &field_23D8C_;
        *target = y_coord - (track_height() - bottom_border() + caption_height_);
        if (*target < sprite_b_.height() / 2) {
            *target = sprite_b_.height() / 2;
        }
        if (*target > client_height() - caption_height_ - bottom_border() -
                          sprite_b_.height() / 2) {
            *target = client_height() - caption_height_ - bottom_border() -
                      sprite_b_.height() / 2;
        }
        line_x = field_23D80_;
        line_y = *target;
        redraw_nc_buffer(1);
        *target = -1;
        update_nc_buffer(1);
    }

    if (splitter_armed(field_23D80_, field_23D84_, field_23D88_, field_23D8C_)) {
        GraphicWin::update(0, ConsoleDragLineY,
                           static_cast<int>(dib_.bmiHeader.biWidth), 1, nullptr);
        GraphicWin::update(ConsoleDragLineX, 0, 1,
                           -static_cast<int>(dib_.bmiHeader.biHeight), nullptr);
        window_line_raw(static_cast<int>(dib_.bmiHeader.biWidth), line_y,
                        0, line_y, 0, 1, 0);
        window_line_raw(line_x, -static_cast<int>(dib_.bmiHeader.biHeight),
                        line_x, 0, 0, 1, 0);
        ConsoleDragLineX = line_x;
        ConsoleDragLineY = line_y;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x005166D0 0x005168B8 0x005168D0 0x005166F2
// RECOVERY_ASSOCIATED: v1 switch_selector 0x005166D0 0x005168D0 0x00516921 0x005166EC
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
        if (!NetDaemonNet.channels_[1].payload_.flags) {
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
        if (!ConsoleControlTurnActive && !NetDaemonNet.channels_[1].payload_.flags) {
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
            if (!window || (i && !window->view_active_)) continue;
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
// ORIGINAL: 0x00517060 ?use_time_bonus@Console@@QAEXXZ 0x00517060-0x00517173 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00517060.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::use_time_bonus() {
    if (ConsoleControlTurnActive) return;
    const int selection = GameDataStateStorage.fields_.time_controls_selection_;
    if (!selection) return;
    if (!IsMultiplayerNet) return;
    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10) &&
        GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) {
// RECOVERY_LITERAL: v1 0x0068AA48 0x0068AA50 ??_C@_07GHIC@SIMULNO?$AA@ 0x00517060
        NetMsgPrimary.pop("SIMULNO", 5000, 0, nullptr);
        return;
    }
    const int faction = ConsoleGlobal.faction_id_;
    if (PlayersData[faction].time_bonus_count <= 0) {
        parse_num(0, TimeControl[selection].refresh);
// RECOVERY_LITERAL: v1 0x0068AA50 0x0068AA5E ??_C@_0O@EABK@NOTIMEBONUSES?$AA@ 0x00517060
        NetMsgPrimary.pop("NOTIMEBONUSES", 5000, 0, nullptr);
        return;
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x800) {
// RECOVERY_LITERAL: v1 0x0068AA60 0x0068AA6D ??_C@_0N@FAMI@NOBONUSATEND?$AA@ 0x00517060
        NetMsgPrimary.pop("NOBONUSATEND", 5000, 0, nullptr);
        return;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 2) &&
        ((1u << faction_id_) & NetDaemonNet.primary_state_.trailing_words_[6])) {
// RECOVERY_LITERAL: v1 0x0068AA70 0x0068AA7C ??_C@_0M@IGKJ@NOBONUSDONE?$AA@ 0x00517060
        NetMsgPrimary.pop("NOBONUSDONE", 5000, 0, nullptr);
        return;
    }
    message_data(0x2306, 0, 0, 0, 0, 0);
}

namespace {
inline void console_close_network(unsigned int final_channel_mask) {
    NetDaemonNet.hang_up();
    NetDaemon &network = NetDaemonNet;
    network.primary_state_.leading_words_[0] = 0;
    network.primary_state_.leading_words_[1] = 0;
    network.primary_state_.leading_words_[2] = 0;
    network.primary_state_.flags_ = 0;
    network.primary_state_.trailing_words_[7] = 0;
    network.primary_state_.trailing_words_[6] = 0;
    network.channels_[0].leading_words_[1] = 0;
    network.channels_[0].leading_words_[0] = 0;
    network.channels_[0].trailing_words_[0] = 0;
    network.channels_[1].leading_words_[1] = 0;
    network.channels_[0].trailing_words_[9] = 0;
    network.channels_[1].leading_words_[0] = 0;
    network.channels_[1].trailing_words_[5] = 0;
    network.channels_[1].trailing_words_[4] = 0;
    network.primary_state_.trailing_words_[3] = 0;
    network.primary_state_.trailing_words_[4] = 0;
    network.primary_state_.trailing_words_[5] = 0;
    network.channels_[1].trailing_words_[3] = 0;
    network.channels_[1].trailing_words_[2] = 0;
    network.channels_[1].leading_words_[3] = 0;
    network.channels_[1].leading_words_[2] = 0;
    network.channels_[1].payload_.flags = 0;
    network.primary_state_.session_phase_ = 0;
    network.primary_state_.trailing_words_[0] = 0;
    network.channels_[1].trailing_words_[8] = 0;
    network.channels_[1].trailing_words_[7] = 0;
    network.primary_state_.trailing_words_[1] = 0;
    network.primary_state_.trailing_words_[2] = 0;
    network.channels_[1].trailing_words_[6] = final_channel_mask;
    network.channels_[0].payload_.vehicle_selection_follow = 0;
    for (int i = 6; i < 14; ++i) network.channels_[0].trailing_words_[i - 5] = 0;
    NetDaemonNet.locks_.clear();
    NetDaemonNet.AlphaNet::close();
    IsMultiplayerNet = 0;
}
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00514990 ?quit_game@Console@@QAEXXZ 0x00514990-0x00514B2C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00514990.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::quit_game() {
    const char *prompt;
    if ((IsMultiplayerNet || IsMultiplayerPBEM) &&
        bit_count(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 255) > 1) prompt = ConsoleReallyWimpOut;
// RECOVERY_LITERAL: v1 0x0068A7E4 0x0068A7EF ??_C@_0L@GOAC@REALLYQUIT?$AA@ 0x00514990
    else prompt = "REALLYQUIT";
    if (!X_pop(prompt, nullptr)) return;
    if (IsMultiplayerPBEM) {
        // The original modifies only the human-faction byte of this word.
        unsigned char &human_mask = reinterpret_cast<unsigned char &>(
            GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        human_mask &= static_cast<unsigned char>(~(1u << ConsoleGlobal.faction_id_));
        save_game(0);
    }
    if ((GameDataStateStorage.fields_.game_rules_ & 0x400) && !IsMultiplayerPBEM) auto_save();
    if (IsMultiplayerNet) console_close_network(255);
    ExitTurnLoop = 1;
// RECOVERY_LITERAL: v1 0x0068A7F0 0x0068A818 ??_C@_0CI@GLBI@Quit?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK@ 0x00514990
    log_say("Quit***********************************", 0, 0, 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00514D30 ?new_game@Console@@QAEXXZ 0x00514D30-0x00514EE2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00514d30.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::new_game() {
    if (IsMultiplayerNet) return;
    const char *prompt;
    if (IsMultiplayerPBEM && bit_count(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 255) > 1)
        prompt = ConsoleReallyWimpOut;
    else prompt = "REALLYQUIT";
    if (X_pop(prompt, nullptr)) {
        if (IsMultiplayerPBEM) {
            unsigned char &human_mask = reinterpret_cast<unsigned char &>(
                GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            human_mask &= static_cast<unsigned char>(~(1u << ConsoleGlobal.faction_id_));
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
// ORIGINAL: 0x005162A0 ?name_landmark@Console@@QAEXHH@Z 0x005162A0-0x0051648F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005162a0.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::name_landmark(int x, int y) {
    if (!on_map(x, y)) {
        x = ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
        y = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
    }
    const int landmark = find_landmark(x, y, 1);
    if (!(GameDataStateStorage.fields_.game_state_ & 0x80)) {
        const int valid = landmark >= 0
            ? valid_landmark(MapLandmark[landmark].x, MapLandmark[landmark].y,
                             ConsoleGlobal.faction_id_)
            : valid_landmark(x, y, ConsoleGlobal.faction_id_);
        if (!valid) {
            NetMsgPrimary.pop("LANDMARKNO", 5000, 0, nullptr);
            return;
        }
    }
    if (landmark >= 0) {
        if (X_pop_ask("LANDMARK", 31, MapLandmark[landmark].name, nullptr, 0)) return;
        purge_spaces(ParseStrBuffer[0].str);
        if (!strlen(ParseStrBuffer[0].str)) return;
        if (IsMultiplayerNet) {
            message_landmark(0x2e03, 0, ParseStrBuffer[0].str,
                MapLandmark[landmark].x, MapLandmark[landmark].y, 0, 0);
            return;
        }
        strcpy(MapLandmark[landmark].name, ParseStrBuffer[0].str);
        ::draw_map(1);
    } else {
        if (X_pop_ask("LANDMARK", 31, LandmarkDefaultName, nullptr, 0)) return;
        purge_spaces(ParseStrBuffer[0].str);
        if (!strlen(ParseStrBuffer[0].str)) return;
        if (IsMultiplayerNet) {
            message_landmark(0x2e01, 0, ParseStrBuffer[0].str, x, y, 0, 0);
            return;
        }
        if (new_landmark(x, y, ParseStrBuffer[0].str) < 0) {
            parse_num(0, 64);
            NetMsgPrimary.pop("LANDMARKTOOMANY", 5000, 0, nullptr);
        }
        ::draw_map(1);
    }
}

extern int IfaceIdToSlotStorage[30];
extern int ActiveIfaceIdStorage;

// Inlined copy of MapWin::tile_in_view (0x004216F0), which lives in another TU.
static inline int ready_tile_in_view(MapWin &win, int x, int y, int margin) {
    if (margin != 0 && y >= 0 && y < MapLatitudeBounds && x >= 0 &&
        x < MapLongitudeBounds) {
        if (alt_at(x, y) > 3 &&
            y - alt_at(x, y) + 3 < win.field_1DDA8_ + margin) {
            return 0;
        }
    }
    if (y < win.field_1DDA8_ + margin) {
        return 0;
    }
    if (y >= win.field_1DDD8_ + win.field_1DDD0_ - margin + win.field_1DDA8_) {
        return 0;
    }
    if ((MapIsFlat & 1) == 0) {
        if (x < win.field_1DDA4_ + margin) {
            x += MapLongitudeBounds;
        }
        if (x >= win.field_1DDD4_ + win.field_1DDCC_ - margin + win.field_1DDA4_) {
            x -= MapLongitudeBounds;
        }
    }
    if (x < win.field_1DDA4_ + margin) {
        return 0;
    }
    if (x >= win.field_1DDD4_ + win.field_1DDCC_ - margin + win.field_1DDA4_) {
        return 0;
    }
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00510F50 ?ready@Console@@QAEXXZ 0x00510F50-0x00511471 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00510f50.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::ready() {
    field_23BF8_ = 0;
    ::draw_cursor();
    selected_vehicle_ = ready_search(field_23D18_ ? -1 : selected_vehicle_);
    field_23D18_ = 0;
    if (selected_vehicle_ < 0 || !ConsoleGlobal.cursor_active_) return;
    if (!Vehs[selected_vehicle_].order || Vehs[selected_vehicle_].order >= 24) {
        const int y = Vehs[selected_vehicle_].y;
        const int x = Vehs[selected_vehicle_].x;
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
        if (!(GameDataStateStorage.fields_.game_preferences_ & 0x80000) ||
            (!Vehs[selected_vehicle_].order && !(Vehs[selected_vehicle_].state & 0x200) &&
             (Vehs[selected_vehicle_].state & 0x1000200) != 0x1000200)) {
            if (!focus(Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y,
                       Vehs[selected_vehicle_].faction_id)) {
                if (!ready_tile_in_view(*this, Vehs[selected_vehicle_].x,
                                        Vehs[selected_vehicle_].y, 3)) {
                    set_center(Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y, 1);
                }
            }
        }
        alpha_menu_.check(selected_vehicle_);
        flush_input();
        Vehs[selected_vehicle_].unk_6 = 0;
    }
    const int base = base_at(Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
    if (!Vehs[selected_vehicle_].order && !(Vehs[selected_vehicle_].state & 0x200) &&
        (Vehs[selected_vehicle_].state & 0x1000200) != 0x1000200 &&
        !(Vehs[selected_vehicle_].state & 0x4000) && base >= 0) {
        const int16_t destination = Bases[base].auto_forward_slots[
            Chassis[VehPrototypes[Vehs[selected_vehicle_].proto_id].chassis_id].triad];
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
        switch (Chassis[VehPrototypes[prototype].chassis_id].triad) {
        case 0:
            if (VehPrototypes[prototype].chassis_id == 0 && Weapon[VehPrototypes[prototype].weapon_id].offense_rating != 0) {
                g_FX.play(0x40, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
            } else if (Weapon[VehPrototypes[prototype].weapon_id].offense_rating != 0) {
                g_FX.play(0x41, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
            } else {
                g_FX.play(0x42, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
            }
            break;
        case 1:
            if (Weapon[VehPrototypes[prototype].weapon_id].offense_rating != 0) {
                g_FX.play(0x45, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
            } else {
                g_FX.play(0x46, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
            }
            break;
        case 2:
            if (!Chassis[VehPrototypes[prototype].chassis_id].range) {
                g_FX.play(0x43, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
            } else if (Chassis[VehPrototypes[prototype].chassis_id].range == 1) {
                g_FX.play(0x44, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
            } else {
                g_FX.play(0x47, Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y);
            }
            break;
        }
    }
    ambience_veh(selected_vehicle_);
    flush_input();
}

int __cdecl popv(char *caption, const char *label, int vehicle, int flags);
void __cdecl action_gate(int vehicle_id, int base_id);


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00514B30 ?retire_game@Console@@QAEXXZ 0x00514B30-0x00514D2B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00514B30; retained seed src/recovered/units/00514b30.cpp", "Native514B55..64 passes the low-byte greater-than-one boolean into bit_count. Native514D10 writes closed-network==0 to93A948 GameRestartQueued,514D16 sets9B2068 ExitTurnLoop. Existing typed close-network helper reproduces lock clear/transport close; scenario, PBEM, ironman and replay branches remain distinct.", "recovery/tasks/console-scenario-owner-wave.json; native scope decisions preserve the original baseline"], "reason": "Recover the complete Console command in its genuine owner, retaining original spans and seed evidence. Typed state and lifecycle/network APIs replace seed address arithmetic; this source binding is not byte, reference, structure or runtime acceptance."}
void Console::retire_game() { // 0x00514B30
    // This function owns a separate static string for the distinct retirement pointer.
// RECOVERY_OBJECT: v1 0x0068A818 _?retire_wimpout@?1??retire_game@Console@@QAEXXZ@4QBDB
    static const char retire_wimpout[] = "REALLYWIMPOUT";
    int closed_network = 0;
    if (GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) {
        auto_save();
    }

    const bool multiple_players =
        (IsMultiplayerNet || IsMultiplayerPBEM) &&
        bit_count(static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) > 1) != 0;
    const char *const prompt = multiple_players ? retire_wimpout :
// RECOVERY_LITERAL: v1 0x0068A828 0x0068A836 ??_C@_0O@JMAD@REALLYRETIRE2?$AA@ 0x00514B30
// RECOVERY_LITERAL: v1 0x0068A838 0x0068A845 ??_C@_0N@LNNN@REALLYRETIRE?$AA@ 0x00514B30
        (GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) ? "REALLYRETIRE2" : "REALLYRETIRE";
    if (!X_pop(prompt, nullptr)) {
        return;
    }

    if (IsMultiplayerPBEM) {
        unsigned char &human_mask = reinterpret_cast<unsigned char &>(
            GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        human_mask &= static_cast<unsigned char>(~(1u << ConsoleGlobal.faction_id_));
        save_game(0);
    }

    if (IsMultiplayerNet) {
        console_close_network(0xFF);
        closed_network = 1;
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
// RECOVERY_LITERAL: v1 0x0068A848 0x0068A86C ??_C@_0CE@JOCE@Retire?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK?$CK@ 0x00514B30
    log_say("Retire*****************************", 0, 0, 0);
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
            strcat(StringTemp, StringTable.get(Facility[facility_id].name));
            popup.dialogs().item(StringTemp, project_id);
        }
        popup.sort_item_ids(0);

        const int selected_project = popup.exec(0, nullptr);
        if (selected_project < 0) {
            return;
        }
        amovie_project(selected_project);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00513B90 ?veh_turn@Console@@QAEHXZ 0x00513B90-0x00513FB9 BYTE_EXACT
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
            field_23BE4_ = Vehs[selected_vehicle_].order == 0x18;
            field_23BF0_ = 0;
        }

        // Native reads 0x0093E8E0 here: NetDaemon + 0x1B50, the already
        // typed lock_state_[0].dword_[9]. Its wider semantic role is unknown.
        if (NetDaemonNet.primary_state_.trailing_words_[3] != 0) {
            if (ConsoleGlobal.cursor_active_ != 0) {
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
            result = 1;
            wait_loop();
            continue;
        }

        if (field_23D24_ != 0 &&
            (Vehs[selected_vehicle_].state & 0x200u) == 0 &&
            (Vehs[selected_vehicle_].state & 0x01000200u) != 0x01000200u &&
            (Vehs[selected_vehicle_].state & 0x4000u) == 0 &&
            Vehs[selected_vehicle_].order == 0) {
            goto clear_auto_order;
        }

        if (ConsoleGlobal.cursor_active_ != 0) {
            if (veh_ready(selected_vehicle_) != 0) {
                if (TutWinBaseWindow.is_visible() == 0 &&
                    ReportWinGlobal.is_visible() == 0) {
                    if ((Vehs[selected_vehicle_].state & 0x4200u) == 0) {
                        const uint8_t order = Vehs[selected_vehicle_].order;
                        if (order == 0) {
                            n = 0;
                        } else if (++n > (order == 0x18 ? 100 : 20)) {
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
                        if (NetDaemonNet.action(selected_vehicle_, 0) != 0) {
                            continue;
                        }
                        Vehs[selected_vehicle_].state |= 0x04000000u;
                    } else {
                        // Native target 0x00531150 is NetDaemon::automated_lock(int *, int).
                        if (NetDaemonNet.automated_lock(&selected_vehicle_, 4) != 0) {
                            goto finish_turn;
                        }
                        ++NetBusyCount;
                        if (++n > 20) {
                            veh_skip(selected_vehicle_);
                            if (IsMultiplayerNet != 0) {
                                synch_veh(selected_vehicle_);
                                NetDaemonNet.await_synch();
                                NetDaemonNet.unlock_veh();
                            }
                            --NetBusyCount;
                            continue;
                        }
                        if (enemy_veh(selected_vehicle_) != 0) {
                            goto enemy_vehicle_found;
                        }
                        --NetBusyCount;
                        continue;
                    }
                }
            } else if (Vehs[selected_vehicle_].order != 0 ||
                       (Vehs[selected_vehicle_].state & 0x4200u) != 0) {
                goto selected_vehicle_unavailable;
            }
        }

        result = 1;
        wait_loop();
        if (TutWinTourPending != 0) {
            TutWinTourPending = 0;
            TutorialWindow.tour();
        }
    }

clear_auto_order:
    field_23D24_ = 0;
selected_vehicle_unavailable:
    selected_vehicle_ = -1;
finish_turn:
    field_23BE4_ = 0;
    if (flag10 == 0) {
        selected_vehicle_ = -1;
    }
    return result;

enemy_vehicle_found:
    if (IsMultiplayerNet != 0) {
        NetDaemonNet.unlock_veh();
        selected_vehicle_ = -1;
    }
    --NetBusyCount;
    goto finish_turn;
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

// ORIGINAL: 0x00514050 ?human_turn@Console@@QAEXXZ 0x00514050-0x005147DA FILE BYTE_EXACT
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

        if (ConsoleGlobal.cursor_active_ != 0) {
            if (selected_vehicle_ < 0 ||
                GameDataStateStorage.fields_.vehicle_current_count_ != previous_vehicle_count ||
                (field_23BEC_ ? !veh_selectable(selected_vehicle_) : !veh_ready(selected_vehicle_))) {
                ready();
            } else if (field_23BF4_ == 0) {
                // The helper can run callbacks; native reloads the selection
                // after the eligibility callback and before reading its tile.
                const int y = Vehs[selected_vehicle_].y;
                const int x = Vehs[selected_vehicle_].x;
                field_23BF8_ = 0;
                if (survey_mode_ != 0) {
                    survey_mode_ = 0;
                    MapWinTable[0]->draw_map(1);
                }
                MapWinTable[0]->draw_cursor();
                cursor_next(x, y);
                // cursor_next may dispatch callbacks that change the live
                // selection; native reloads selected_vehicle_ afterward.
                if (field_23BF0_ == 0 || Vehs[selected_vehicle_].order != 0x18) {
                    focus(Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y,
                          Vehs[selected_vehicle_].faction_id);
                    ConsoleInfoWin.change(0);
                    ConsoleStatusWin.redraw();
                    MapWinTable[0]->main_caption();
                    alpha_menu_.check(selected_vehicle_);
                }
            }
        } else if (selected_vehicle_ < 0 ||
                   GameDataStateStorage.fields_.vehicle_current_count_ != previous_vehicle_count ||
                   (field_23BEC_ ? !veh_selectable(selected_vehicle_) : !veh_ready(selected_vehicle_))) {
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
                    NetMsgPrimary.pop(flush_cache == 0 ? "ENDOFTURN1" : "ENDOFTURN",
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

// ORIGINAL: 0x005171D0 ?veh_key@Console@@QAEXHHH@Z 0x005171D0-0x0051778A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005171d0.cpp", "image:terranx_original.exe:0x005171D0", "Pinned key selector tables5177E4/517844 and accelerator targets517898; native selection reloads between dispatch stages preserve callback effects."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
// TRIED: MISMATCH #3 push-vs-mov in the prologue; hoisting the two guard reads into locals before the branch did not change register allocation order. Full switch/goto structure otherwise matches.
void Console::veh_key(int key, int character, int command) {
    const int lock_flags = NetDaemonNet.channels_[1].payload_.flags;
    if (IsMultiplayerNet != 0 && character != 'a') {
        if (ConsoleControlTurnActive != 0) return;
        if (lock_flags != 0) return;
        if ((NetDaemonNet.primary_state_.trailing_words_[6] &
             (1 << ConsoleGlobal.faction_id_)) != 0) return;
    }

    // Every case reads the selected-vehicle member afresh.
    switch (key) {
    case 'a':
        if (lock_flags == 0) {
            activate(Vehs[selected_vehicle_].x, Vehs[selected_vehicle_].y, 1);
        }
        break;
    case 'b': {
        const int vehicle_id = selected_vehicle_;
        const uint8_t triad = Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].triad;
        if (triad == TRIAD_AIR && VehPrototypes[Vehs[vehicle_id].proto_id].plan != 8 &&
            Weapon[VehPrototypes[Vehs[vehicle_id].proto_id].weapon_id].offense_rating != 0) {
            automate(vehicle_id, 10);
        } else if (Weapon[VehPrototypes[Vehs[vehicle_id].proto_id].weapon_id].offense_rating != 0 &&
                   triad != TRIAD_AIR) {
            oblit(vehicle_id);
        } else if (VehPrototypes[Vehs[vehicle_id].proto_id].plan == 9) {
            terraform(vehicle_id, 14);
        } else if (VehPrototypes[Vehs[vehicle_id].proto_id].plan == 8) {
            // Both arms found a base; the image keeps the dead plan compare.
            new_base(vehicle_id);
        } else {
            new_base(vehicle_id);
        }
        break;
    }
    case 'q': terraform(selected_vehicle_, 15); break;
    case 'B': terraform(selected_vehicle_, 14); break;
    case 'E': terraform(selected_vehicle_, 13); break;
    case 'f':
        if (can_arty(Vehs[selected_vehicle_].proto_id, 1) != 0) {
            arty(selected_vehicle_);
        } else {
            terraform(selected_vehicle_, 0);
        }
        break;
    case 'F': terraform(selected_vehicle_, 4); break;
    case 'g': go_to(selected_vehicle_, 0x18, 0); break;
    case 'G': go_home(selected_vehicle_); break;
    case 'h': hold(selected_vehicle_, 0); break;
    case 'H': hold(selected_vehicle_, 10); break;
    case 'i': use_ability(selected_vehicle_); break;
    case 'I': go_to(selected_vehicle_, -3, 0); break;
    case 'n': terraform(selected_vehicle_, 12); break;
    case 'o':
        if (VehPrototypes[Vehs[selected_vehicle_].proto_id].plan == 10) {
            convoy(selected_vehicle_);
        } else {
            terraform(selected_vehicle_, 9);
        }
        break;
    case 'L': automate(selected_vehicle_, 11); break;
    case 'l': sentry(selected_vehicle_, 0); break;
    case 'p': set_patrol(selected_vehicle_); break;
    case 'P': {
        const uint32_t destination_flag = MapWinTable[0]->field_1DD70_ & 0x400000u;
        for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
            if (MapWinTable[slot] != 0 && (slot == 0 || MapWinTable[slot]->view_active_ != 0)) {
                if (destination_flag == 0) {
                    MapWinTable[slot]->field_1DD70_ |= 0x400000u;
                    MapWinTable[slot]->draw_base_dest(1);
                } else {
                    MapWinTable[slot]->field_1DD70_ &= 0xffbfffffu;
                    MapWinTable[slot]->draw_map(1);
                }
            }
        }
        break;
    }
    case 'U': unload(selected_vehicle_); break;
    case 'r': terraform(selected_vehicle_, 5); break;
    }

    switch (character) {
    case 'm': terraform(selected_vehicle_, 2); break;
    case 's': terraform(selected_vehicle_, 3); break;
    case 0x2f: explore(selected_vehicle_); break;
    case 0x20: skip(selected_vehicle_); break;
    case ']': terraform(selected_vehicle_, 16); break;
    case '[': terraform(selected_vehicle_, 17); break;
    case '_': terraform(selected_vehicle_, 18); break;
    case 'd': destroy(selected_vehicle_); break;
    case 'k': terraform(selected_vehicle_, 7); break;
    case 0x2e: terraform(selected_vehicle_, 8); break;
    }

    switch (command) {
    case 0x30046: automate(selected_vehicle_, 6); break;
    case 0x20052: road_to(selected_vehicle_, 0x1b); break;
    case 0x20044: designate(selected_vehicle_); break;
    case 0x20046: terraform(selected_vehicle_, 11); break;
    case 0x20048: go_home(selected_vehicle_); break;
    case 0x20054: road_to(selected_vehicle_, 0x1c); break;
    case 0x20055: upgrade(selected_vehicle_); break;
    case 0x30041: {
        const int vehicle_id = selected_vehicle_;
        if (Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].triad == TRIAD_AIR &&
            Weapon[VehPrototypes[Vehs[vehicle_id].proto_id].weapon_id].offense_rating != 0) {
            automate(vehicle_id, 12);
        } else {
            automate(vehicle_id, 3);
        }
        break;
    }
    case 0x3004d: automate(selected_vehicle_, 5); break;
    case 0x30053: automate(selected_vehicle_, 4); break;
    case 0x30052: automate(selected_vehicle_, 1); break;
    case 0x30054: automate(selected_vehicle_, 2); break;
    case 0x3004f: automate(selected_vehicle_, 7); break;
    case 0x30055: give_veh(selected_vehicle_); break;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050FE70 ?on_nc_left_up@Console@@QAEXHH@Z 0x0050FE70-0x0051010A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050fe70.cpp: retained complete original identity/spans and seed evidence", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions through sequence27", "Source owner interpretation preserves mapped original accesses, lifetime calls and branch roles; source-bound members and globals replace the seed offsets/address aliases. Historical seeds and claims remain independent restoration obligations."], "reason": "Recover splitter release into the genuine Console owner using source Win capture owners, real Win inheritance, embedded Sprite geometry and named splitter members. Preserve signed sentinel ordering and the original update operations; no fixed executable calls or self-offset access."}
// symbol    ?on_nc_left_up@Console@@UAEXHH@Z
void Console::on_nc_left_up(int x, int y) {
    release_pointer();

    // Each clamp re-reads the geometry: the stores go through `target`.
    if (field_23D80_ != -1 && field_23D84_ != -1) {
        if (field_23D88_ == -1 || field_23D8C_ == -1) {
            int *const target = field_23D88_ == -1 ? &field_23D88_ : &field_23D8C_;
            *target = y - (track_height() - bottom_border() + caption_height_);
            if (*target < sprite_b_.height() / 2) {
                *target = sprite_b_.height() / 2;
            }
            if (*target > client_height() - caption_height_ - bottom_border() -
                              sprite_b_.height() / 2) {
                *target = client_height() - caption_height_ - bottom_border() -
                          sprite_b_.height() / 2;
            }
            update(0, ConsoleDragLineY, dib_.bmiHeader.biWidth, 1, nullptr);
            update(ConsoleDragLineX, 0, 1, -static_cast<int>(dib_.bmiHeader.biHeight), nullptr);
        }
    } else {
        int *const target = field_23D80_ == -1 ? &field_23D80_ : &field_23D84_;
        *target = x - border_thickness_;
        if (*target < sprite_a_.width() / 2) {
            *target = sprite_a_.width() / 2;
        }
        if (*target > client_width() - border_thickness_ * 2 -
                          sprite_a_.width() / 2) {
            *target = client_width() - border_thickness_ * 2 -
                      sprite_a_.width() / 2;
        }
        update(0, ConsoleDragLineY, dib_.bmiHeader.biWidth, 1, nullptr);
        update(ConsoleDragLineX, 0, 1, -static_cast<int>(dib_.bmiHeader.biHeight), nullptr);
    }

    if (field_23D88_ > field_23D8C_) {
        swap(&field_23D88_, &field_23D8C_);
    }
    if (field_23D80_ > field_23D84_) {
        swap(&field_23D80_, &field_23D84_);
    }
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005169F0 ?end_my_turn@Console@@QAEXXZ 0x005169F0-0x00516C4D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005169f0.cpp: retained complete original identity/spans and seed evidence", "recovery/tasks/console-scenario-owner-wave.json; native scope expansions through sequence27", "Source owner interpretation preserves mapped original accesses, lifetime calls and branch roles; source-bound members and globals replace the seed offsets/address aliases. Historical seeds and claims remain independent restoration obligations."], "reason": "Recover end-turn into the genuine Console owner using existing pane/timer/lifecycle members, shared faction waiting state and the independently read original popup captions. Preserve remote-turn and ready-vehicle decisions, including separate timer clear rather than InfoWin::reset. No matching or game-acceptance claim."}
void Console::end_my_turn() {
    TutWinBaseWindow.cancel_zoom_wait();
    ConsoleInfoWin.clear_pending_timer();
    TutorialWindow.reset();
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
            GenderDefault = Players[GameDataStateStorage.fields_.net_turn_faction_].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[GameDataStateStorage.fields_.net_turn_faction_].title_leader, -1, -1);
            GenderDefault = Players[GameDataStateStorage.fields_.net_turn_faction_].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[GameDataStateStorage.fields_.net_turn_faction_].name_leader, -1, -1);
            GenderDefault = Players[GameDataStateStorage.fields_.net_turn_faction_].noun_gender;
            PluralityDefault = Players[GameDataStateStorage.fields_.net_turn_faction_].is_noun_plural;
            parse_says(2, Players[GameDataStateStorage.fields_.net_turn_faction_].noun_faction, -1, -1);
            parse_says(3, NetDaemonNet.get_player_name(NetDaemonNet.who_2_pid(
                              GameDataStateStorage.fields_.net_turn_faction_)), -1, -1);
            NetMsgPrimary.pop("SIMULWHOSE", 0x1388, 0, nullptr);
            return;
        }
        if ((game_flags & 2) != 0 && (GameDataStateStorage.fields_.net_turn_flags_ & 0x10) == 0) {
            const int faction_bit = 1 << ConsoleGlobal.faction_id_;
            if ((NetDaemonNet.factions_waiting_mask() & faction_bit) != 0 &&
                NetDaemonNet.factions_waiting_mask() != 0xFF) {
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

    TutorialWindow.reset();
    go_reset();
    field_23D10_ = 0;

    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == nullptr || (slot != 0 && window->view_active_ == 0)) {
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
             NetDaemonNet.channels_[1].payload_.flags != 0 ||
             NetDaemonNet.primary_state_.leading_words_[2] != 0) &&
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
                if (MultiWindow.panel_open() == 0) MultiWindow.show();
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
                        (slot != 0 && window->view_active_ == 0)) continue;
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
            case 0x720: TutorialWindow.set_tour_stage(0); TutorialWindow.tour(); break;
            case 0x721: TutorialWindow.set_tour_stage(3); TutorialWindow.tour(); break;
            case 0x722: TutorialWindow.set_tour_stage(1); TutorialWindow.tour(); break;
            case 0x723: TutorialWindow.set_tour_stage(5); TutorialWindow.tour(); break;
            case 0x724: TutorialWindow.set_tour_stage(7); TutorialWindow.tour(); break;
            case 0x725: TutorialWindow.set_tour_stage(6); TutorialWindow.tour(); break;
            case 0x750: show_credits(); break;
            case 0x751:
                parse_says(0, ConsoleVersionRelease, -1, -1);
                parse_says(1, ConsoleVersionBuildDate, -1, -1);
                popp(ScriptPopupFile, ConsoleStrVersionControlX, 0,
                     ConsoleStrAcLogoSm, (NullCallback)0);
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
                        ? ConsoleReallyWimpOut
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

// Initialize the Console windows/menu and publish the fallback popup parent.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0051D890 ?init@Console@@QAEXH@Z 0x0051D890-0x0051D9CF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0051d890.cpp"], "reason": "Complete genuine Console initialization/shared geometry from original receiver/member/global dataflow; native seed establishes catalogue identity and span. Source uses actual GraphicWin virtual base, AlphaMenu, shared RECT and typed fallback Win pointer. Original seed/claims retained; binding grants no matching or acceptance."}
// symbol    ?init@Console@@QAEXH@Z
void Console::init(int) {
    close();

    focus_vehicle_ = -1;
    field_23BEC_ = 0;

    if ((~StatusFlags & 1u) != 0) {
        field_23BCC_ = WinScreenWidth;
        IfaceScreenWidth = field_23BCC_;
        field_23BD0_ = WinScreenHeight - MainInterfaceGlobal.panel_height();
        IfaceScreenHeight = field_23BD0_;

        ConsoleMenuBounds.left = 0;
        ConsoleMenuBounds.top = 0;
        ConsoleMenuBounds.right = IfaceScreenWidth;
        ConsoleMenuBounds.bottom = IfaceScreenHeight;
        StatusFlags |= 1u;
    }

    char *const menu_caption = StringTable.get(Labels.strings_ptr[0x1C]);
    GraphicWin::init(&ConsoleMenuBounds, menu_caption, 0x02400000, 0,
                     &alpha_menu_, 0);

    cover_window_.init(0, 0, pixel_width(), pixel_height(), 0, 0x01000022,
                        static_cast<Win *>(this), 0, 0);
    Win::set_def_focus(static_cast<Win *>(this));

    (void)alpha_menu_.menu_init();
    alpha_menu_.set_menu_proc(&Console::menu_daemon);
    load_cursors();

    Win *const window = this;
    BasePopDefaultField322C = window;
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0051D9D0 nullsub_187 0x0051D9D0-0x0051D9D1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0051D9D0-0x0051D9D1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0051D890 ?init@Console@@QAEXH@Z (src/console.cpp) and 0x0051D9F0 ??1Console@@QAE@XZ (src/console.cpp)"]}
// symbol    ?nullsub_187@@YAXXZ
// kind      game
void __cdecl nullsub_187() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0051D9E0 nullsub_188 0x0051D9E0-0x0051D9E1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0051D9E0-0x0051D9E1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0051D890 ?init@Console@@QAEXH@Z (src/console.cpp) and 0x0051D9F0 ??1Console@@QAE@XZ (src/console.cpp)"]}
// symbol    ?nullsub_188@@YAXXZ
// kind      game
void __cdecl nullsub_188() {
}

// Publish current screen geometry independently of init's once-only gate.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0051D760 sub_51d760 0x0051D760-0x0051D7B5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0051d760.cpp"], "reason": "Complete genuine Console initialization/shared geometry from original receiver/member/global dataflow; native seed establishes catalogue identity and span. Source uses actual GraphicWin virtual base, AlphaMenu, shared RECT and typed fallback Win pointer. Original seed/claims retained; binding grants no matching or acceptance."}
// symbol    ?set_screen_geometry@Console@@QAEXXZ
void Console::set_screen_geometry() {
    field_23BCC_ = WinScreenWidth;
    IfaceScreenWidth = field_23BCC_;
    const int height = WinScreenHeight - MainInterfaceGlobal.panel_height();
    field_23BD0_ = height;
    IfaceScreenHeight = height;
    ConsoleMenuBounds.left = 0;
    ConsoleMenuBounds.top = 0;
    ConsoleMenuBounds.right = IfaceScreenWidth;
    ConsoleMenuBounds.bottom = height;
    StatusFlags |= 1u;
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
        (ConsoleGlobal.alpha_menu_.is_visible() != 0 || MultiWindow.panel_open() != 0)) {
        enter_space_handoff = true;
    }
    if (enter_space_handoff != false) {
        if (TutWinBaseWindow.is_visible() == 0) {
            MainInterfaceGlobal.button_group(0).button_click(0x3E9);
        }
        if (ConsoleGlobal.alpha_menu_.is_visible() != 0) {
            ConsoleGlobal.alpha_menu_.hide();
        }
        if (MultiWindow.panel_open() != 0) {
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
    } else if (NetDaemonNet.channels_[1].payload_.flags != 0 ||
               NetDaemonNet.primary_state_.leading_words_[2] != 0) {
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
        if (normalized_character == 'j') {
            MapWin *const toggle_map = MapWinTable[selected_map_slot];
            toggle_map->field_1DD84_ =
                toggle_map->field_1DD84_ == 0 ? 1u : 0u;
            int mouse_x = 0;
            int mouse_y = 0;
            MapWinTable[selected_map_slot]->get_mouse_pos(&mouse_x, &mouse_y);
            MapWin *const j_after_mouse_map = MapWinTable[selected_map_slot];
            j_after_mouse_map->on_mouse_move(mouse_x, mouse_y);
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
            MapWinTable[selected_map_slot]->get_mouse_pos(&mouse_x, &mouse_y);
            MapWin *const g_after_mouse_map = MapWinTable[selected_map_slot];
            g_after_mouse_map->feedback_hover_y_ = -1;
            g_after_mouse_map->field_1DE0C_ = static_cast<uint32_t>(-1);
            g_after_mouse_map->on_mouse_move(mouse_x, mouse_y);
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
        int mouse_x = 0;
        int mouse_y = 0;
        MapWinTable[0]->get_mouse_pos(&mouse_x, &mouse_y);
        MapWin *const mode8_after_mouse_map = MapWinTable[0];
        mode8_after_mouse_map->field_1DE0C_ = static_cast<uint32_t>(-1);
        MapWinTable[0]->on_mouse_move(mouse_x, mouse_y);
        goto native_done;
    }

    // Pan commands in a flagged window consume the character and event before
    // the shared go_reset/cursor restoration path.
    for (size_t pan_window_slot = 0;
         pan_window_slot < MapWinTableSlots; ++pan_window_slot) {
        MapWin *const window = MapWinTable[pan_window_slot];
        if (window == 0 ||
            (pan_window_slot != 0 && window->view_active_ == 0)) {
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
            (cursor_window_slot != 0 && window->view_active_ == 0)) {
            continue;
        }
        if (window->cursor_sprite_ != 0 ||
            window->cursor_resource_id_ != 0x7F00) {
            window->set_cursor(0x7F00);
            window->cursor_resource_id_ = 0x7F00;
            window->cursor_sprite_ = 0;
        }
        MapWin *const cursor_flags_window = MapWinTable[cursor_window_slot];
        if ((cursor_flags_window->field_1DD70_ & 0x00C40000u) != 0) {
            cursor_flags_window->field_1DD70_ &= ~0x00C40000u;
            MapWin *const cursor_redraw_window = MapWinTable[cursor_window_slot];
            cursor_redraw_window->draw_map(1);
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
         (NetDaemonNet.primary_state_.trailing_words_[7] &
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
         (NetDaemonNet.primary_state_.trailing_words_[7] &
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
        PopupWriteCheckState = PlayersData[faction].ai_growth != 0 ? 1 : 0;
        if (PlayersData[faction].ai_tech != 0) {
            PopupWriteCheckState |= 2;
        } else {
            PopupWriteCheckState &= ~2;
        }
        popup_set_write_check_bit(2, PlayersData[faction].ai_wealth);
        popup_set_write_check_bit(3, PlayersData[faction].ai_power);
        if (x_pop_short(ConsoleStrTechRandom, 1, 0) >= 0) {
            const int answer = PopupWriteCheckState;
            PlayersData[faction].ai_growth = answer & 1;
            PlayersData[faction].ai_tech = (answer & 2) >> 1;
            PlayersData[faction].ai_wealth = (answer & 4) >> 2;
            PlayersData[faction].ai_power = (answer & 8) >> 3;
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
            // The pinned key branch drains input again after focus's own conditional flush.
            flush_input();
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
        ::draw_map(1);
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
            danger("Killed a baddy!", "", 0, 0, 0);
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
            danger("Found a baddy!", "", 0, 0, 0);
            found_invalid_vehicle = true;
        }
    }

    if (!found_invalid_vehicle) {
        danger("Game fixed!", "", 0, 0, 0);
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

danger("Made CHECKSUM FILE", "", 0, 0, 0);

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
            if (has_tech(tech_id, faction) != 0) continue;
            if (Technology[tech_id].preq_tech_1 < TechNone) continue;
            if (Technology[tech_id].preq_tech_2 < TechNone) continue;

            int bit_offset;
            int bit_mask;
            bitmask(tech_id, &bit_offset, &bit_mask);
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
        (NetDaemonNet.primary_state_.trailing_words_[3] &
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
            confirmation = ConsoleReallyWimpOut;
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
        quit_confirmation = ConsoleReallyWimpOut;
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
// The original 0x51AC11 comparison dispatches this exact raw event before
// the later layout/key routing; after the popup, preserve the common tail.
case 0x20073:
    parse_says(0, ConsoleVersionRelease, -1, -1);
    parse_says(1, ConsoleVersionBuildDate, -1, -1);
    popp(ScriptPopupFile, ConsoleStrVersionControlX, 0,
         ConsoleStrAcLogoSm, (NullCallback)0);
    break;
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
                confirmation = ConsoleReallyWimpOut;
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
        int destination_x = map_window->center_x_;
        int destination_y = map_window->center_y_;
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

        if (destination_x != map_window->center_x_ ||
            destination_y != map_window->center_y_) {
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

// ORIGINAL: 0x0051DF00 ?on_nc_left_down@thunk1_Console@@QAEXHH@Z 0x0051DF00-0x0051DF08 FILE BYTE_EXACT
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
// ORIGINAL: 0x0051DDE0 ??_Gthunk1_Console@@UAEPAXI@Z 0x0051DDE0-0x0051DDE8 FILE BYTE_EXACT
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
// ORIGINAL: 0x0051DF50 ??_Gthunk2_Console@@UAEPAXI@Z 0x0051DF50-0x0051DF61 FILE BYTE_EXACT
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
// ORIGINAL: 0x0051DE40 ?on_redraw@thunk4_MapWin@@QAEXXZ 0x0051DE40-0x0051DE4E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0Console@@QAE@H@Z at 0x0050F460 stores the Console Win view view table at 0066EC18.", "Pinned view slot +0x30 contains original thunk VA 0051DE40.", "Original full-span receiver adjustment is sub ecx, dword ptr [ecx - 4]; sub ecx, 0x2328; its tail reaches 0046B1D0 (?on_redraw@MapWin@@QAEXXZ).", "The original tail target ?on_redraw@MapWin@@QAEXXZ is source-mapped in src/mapwin.cpp; the Console Win view class-view entry is physically emitted in src/console.cpp."], "reason": "Bind the original Console Win-view adjustor identity to the compiler-generated most-derived Console class-view entry in src/console.cpp, while retaining the inherited MapWin callback as its typed target in src/mapwin.cpp. No adapter, vtable, or hierarchy change is authored."}
// symbol    ?on_redraw@MapWin@@$4PPPPPPPM@CDCI@AEXXZ
// kind game
// ORIGINAL: 0x0051DE80 ?on_left_click@thunk4_MapWin@@QAEXHH@Z 0x0051DE80-0x0051DE8E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0Console@@QAE@H@Z at 0x0050F460 stores the Console Win view view table at 0066EC18.", "Pinned view slot +0x4C contains original thunk VA 0051DE80.", "Original full-span receiver adjustment is sub ecx, dword ptr [ecx - 4]; sub ecx, 0x2328; its tail reaches 0046EBA0 (?on_left_click@MapWin@@QAEXHH@Z).", "The original tail target ?on_left_click@MapWin@@QAEXHH@Z is source-mapped in src/mapwin.cpp; the Console Win view class-view entry is physically emitted in src/console.cpp."], "reason": "Bind the original Console Win-view adjustor identity to the compiler-generated most-derived Console class-view entry in src/console.cpp, while retaining the inherited MapWin callback as its typed target in src/mapwin.cpp. No adapter, vtable, or hierarchy change is authored."}
// symbol    ?on_left_click@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
// kind game
// ORIGINAL: 0x0051DE90 ?on_left_down@thunk4_MapWin@@QAEXHH@Z 0x0051DE90-0x0051DE9E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0Console@@QAE@H@Z at 0x0050F460 stores the Console Win view view table at 0066EC18.", "Pinned view slot +0x50 contains original thunk VA 0051DE90.", "Original full-span receiver adjustment is sub ecx, dword ptr [ecx - 4]; sub ecx, 0x2328; its tail reaches 0046ED30 (?on_left_down@MapWin@@QAEXHH@Z).", "The original tail target ?on_left_down@MapWin@@QAEXHH@Z is source-mapped in src/mapwin.cpp; the Console Win view class-view entry is physically emitted in src/console.cpp."], "reason": "Bind the original Console Win-view adjustor identity to the compiler-generated most-derived Console class-view entry in src/console.cpp, while retaining the inherited MapWin callback as its typed target in src/mapwin.cpp. No adapter, vtable, or hierarchy change is authored."}
// symbol    ?on_left_down@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
// kind game
// ORIGINAL: 0x0051DEA0 ?on_left_up@thunk4_MapWin@@QAEXHH@Z 0x0051DEA0-0x0051DEAE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0Console@@QAE@H@Z at 0x0050F460 stores the Console Win view view table at 0066EC18.", "Pinned view slot +0x54 contains original thunk VA 0051DEA0.", "Original full-span receiver adjustment is sub ecx, dword ptr [ecx - 4]; sub ecx, 0x2328; its tail reaches 0046EFF0 (?on_left_up@MapWin@@QAEXHH@Z).", "The original tail target ?on_left_up@MapWin@@QAEXHH@Z is source-mapped in src/mapwin.cpp; the Console Win view class-view entry is physically emitted in src/console.cpp."], "reason": "Bind the original Console Win-view adjustor identity to the compiler-generated most-derived Console class-view entry in src/console.cpp, while retaining the inherited MapWin callback as its typed target in src/mapwin.cpp. No adapter, vtable, or hierarchy change is authored."}
// symbol    ?on_left_up@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
// kind game
// ORIGINAL: 0x0051DEB0 ?on_right_click@thunk4_MapWin@@QAEXHH@Z 0x0051DEB0-0x0051DEBE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0Console@@QAE@H@Z at 0x0050F460 stores the Console Win view view table at 0066EC18.", "Pinned view slot +0x58 contains original thunk VA 0051DEB0.", "Original full-span receiver adjustment is sub ecx, dword ptr [ecx - 4]; sub ecx, 0x2328; its tail reaches 0046EBE0 (?on_right_click@MapWin@@QAEXHH@Z).", "The original tail target ?on_right_click@MapWin@@QAEXHH@Z is source-mapped in src/mapwin.cpp; the Console Win view class-view entry is physically emitted in src/console.cpp."], "reason": "Bind the original Console Win-view adjustor identity to the compiler-generated most-derived Console class-view entry in src/console.cpp, while retaining the inherited MapWin callback as its typed target in src/mapwin.cpp. No adapter, vtable, or hierarchy change is authored."}
// symbol    ?on_right_click@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
// kind game
// ORIGINAL: 0x0051DEC0 ?on_right_down@thunk4_MapWin@@QAEXHH@Z 0x0051DEC0-0x0051DECE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0Console@@QAE@H@Z at 0x0050F460 stores the Console Win view view table at 0066EC18.", "Pinned view slot +0x5C contains original thunk VA 0051DEC0.", "Original full-span receiver adjustment is sub ecx, dword ptr [ecx - 4]; sub ecx, 0x2328; its tail reaches 0046EC10 (?on_right_down@MapWin@@QAEXHH@Z).", "The original tail target ?on_right_down@MapWin@@QAEXHH@Z is source-mapped in src/mapwin.cpp; the Console Win view class-view entry is physically emitted in src/console.cpp."], "reason": "Bind the original Console Win-view adjustor identity to the compiler-generated most-derived Console class-view entry in src/console.cpp, while retaining the inherited MapWin callback as its typed target in src/mapwin.cpp. No adapter, vtable, or hierarchy change is authored."}
// symbol    ?on_right_down@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
// kind game
// ORIGINAL: 0x0051DEE0 ?on_left_double_click@thunk4_MapWin@@QAEXHH@Z 0x0051DEE0-0x0051DEEE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0Console@@QAE@H@Z at 0x0050F460 stores the Console Win view view table at 0066EC18.", "Pinned view slot +0x70 contains original thunk VA 0051DEE0.", "Original full-span receiver adjustment is sub ecx, dword ptr [ecx - 4]; sub ecx, 0x2328; its tail reaches 0046EBD0 (?on_left_double_click@MapWin@@QAEXHH@Z).", "The original tail target ?on_left_double_click@MapWin@@QAEXHH@Z is source-mapped in src/mapwin.cpp; the Console Win view class-view entry is physically emitted in src/console.cpp."], "reason": "Bind the original Console Win-view adjustor identity to the compiler-generated most-derived Console class-view entry in src/console.cpp, while retaining the inherited MapWin callback as its typed target in src/mapwin.cpp. No adapter, vtable, or hierarchy change is authored."}
// symbol    ?on_left_double_click@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
// kind game
// ORIGINAL: 0x0051DF20 ?on_button_clicked@thunk4_MapWin@@QAEXH@Z 0x0051DF20-0x0051DF2E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0Console@@QAE@H@Z at 0x0050F460 stores the Console Win view view table at 0066EC18.", "Pinned view slot +0xB0 contains original thunk VA 0051DF20.", "Original full-span receiver adjustment is sub ecx, dword ptr [ecx - 4]; sub ecx, 0x2328; its tail reaches 0046F8C0 (?on_button_clicked@MapWin@@QAEXH@Z).", "The original tail target ?on_button_clicked@MapWin@@QAEXH@Z is source-mapped in src/mapwin.cpp; the Console Win view class-view entry is physically emitted in src/console.cpp."], "reason": "Bind the original Console Win-view adjustor identity to the compiler-generated most-derived Console class-view entry in src/console.cpp, while retaining the inherited MapWin callback as its typed target in src/mapwin.cpp. No adapter, vtable, or hierarchy change is authored."}
// symbol    ?on_button_clicked@MapWin@@$4PPPPPPPM@CDCI@AEXH@Z
// kind game
// ORIGINAL: 0x0051DEF0 ?on_nc_mouse_move@thunk1_Console@@QAEXHH@Z 0x0051DEF0-0x0051DEF8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Console constructor 0x0050F460 installs Win address point 0x0066EC18 at 0x0050F562; slot 31 cell selects original identity 0x0051DEF0.", "Pinned Win primary table 0x0066FDD0 has the matching shared callback at slot 31; Win's named contract retains the original parameter shape.", "Pinned original thunk 0x0051DEF0 is `sub ecx,[ecx-4]; jmp 0x00510110`; the typed Console member is source-owned in src/console.cpp.", "The complete seed comment envelope, including original marker/name/spans/prototype/callers/kind/flags/calls/return/status/purpose/role and prose, is inertly archived in RECOVERY_HISTORY with source and envelope SHA-256 15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f/7937cba18d6e8aac8c8591100a989f43fdd77ca7f84573d36a51b64fa5368e86.", "The real Win virtual contract and Console override emit a compiler-owned vtordisp tail to the typed Console implementation, rather than a handwritten receiver-adjustment adapter. Source emitted-symbol facts identify the actual thunk and target separately; linked selection and matching require fresh verification."], "reason": "Bind the preserved Console Win-view callback identity to its pinned constructor-installed virtual slot and the natural compiler-generated entry for the typed Console override."}
// symbol    ?on_nc_mouse_move@Console@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0051DF10 ?on_nc_left_up@thunk1_Console@@QAEXHH@Z 0x0051DF10-0x0051DF18 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Console constructor 0x0050F460 installs Win address point 0x0066EC18 at 0x0050F562; slot 34 cell selects original identity 0x0051DF10.", "Pinned Win primary table 0x0066FDD0 has the matching shared callback at slot 34; Win's named contract retains the original parameter shape.", "Pinned original thunk 0x0051DF10 is `sub ecx,[ecx-4]; jmp 0x0050FE70`; the typed Console member is source-owned in src/console.cpp.", "The complete seed comment envelope, including original marker/name/spans/prototype/callers/kind/flags/calls/return/status/purpose/role and prose, is inertly archived in RECOVERY_HISTORY with source and envelope SHA-256 15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f/37ddaa38b6a53d790767396d5fc2a32d2594f42d6df20cf9c5b4b9f51becf7f8.", "The real Win virtual contract and Console override emit a compiler-owned vtordisp tail to the typed Console implementation, rather than a handwritten receiver-adjustment adapter. Source emitted-symbol facts identify the actual thunk and target separately; linked selection and matching require fresh verification."], "reason": "Bind the preserved Console Win-view callback identity to its pinned constructor-installed virtual slot and the natural compiler-generated entry for the typed Console override."}
// symbol    ?on_nc_left_up@Console@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0051DF30 ?on_post_redraw_nc_buffer@thunk1_Console@@QAEXPAUBuffer@@H@Z 0x0051DF30-0x0051DF38 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Console constructor 0x0050F460 installs Win address point 0x0066EC18 at 0x0050F562; slot 60 cell selects original identity 0x0051DF30.", "Pinned Win primary table 0x0066FDD0 has the matching shared callback at slot 60; Win's named contract retains the original parameter shape.", "Pinned original thunk 0x0051DF30 is `sub ecx,[ecx-4]; jmp 0x0050F960`; the typed Console member is source-owned in src/console.cpp.", "The complete seed comment envelope, including original marker/name/spans/prototype/callers/kind/flags/calls/return/status/purpose/role and prose, is inertly archived in RECOVERY_HISTORY with source and envelope SHA-256 15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f/44e403a97bb3aef98912293e94a5ac00a173c73c97f77849f3d6bbfdba9f9e09.", "The real Win virtual contract and Console override emit a compiler-owned vtordisp tail to the typed Console implementation, rather than a handwritten receiver-adjustment adapter. Source emitted-symbol facts identify the actual thunk and target separately; linked selection and matching require fresh verification."], "reason": "Bind the preserved Console Win-view callback identity to its pinned constructor-installed virtual slot and the natural compiler-generated entry for the typed Console override."}
// symbol    ?on_post_redraw_nc_buffer@Console@@$4PPPPPPPM@A@AEXPAVBuffer@@H@Z
// kind      game
// RECOVERY_GENERATED: v1 vbtable 0x0066EF04 0x0066EF0C ??_8Console@@7B@ 0x0050F460

// Console constructor 0x0050F56E stores this secondary table at Win+0x444;
// its first slot is the mapped Buffer-view deleting adjustor 0x0051DF50.
// The table address point is proven; its full extent remains unclaimed.
// RECOVERY_GENERATED: v1 vtable 0x0066EC10 ? ??_7Console@@6BBuffer@@@ 0x0050F460
