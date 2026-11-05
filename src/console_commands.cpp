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

// Console orders and scenario-editor operations; this semantic source home
// does not claim a recovered original translation-unit filename/boundary.
#include "stdafx.h"
// Original Console::edit_lock at 0x004E1F40 queries this USER32 export through its import pointer.
// RECOVERY_IMPORT: v1 __imp__GetKeyState@4 USER32.dll name:GetKeyState
#include "prefwin.h"
#include "interlude.h"
#include "text.h"
#include "alphamovie.h"
#include "console.h"
#include "reportif.h"
#include "datalink.h"
#include "worldwin.h"
#include "multiwin.h"
#include "dipedit.h"
#include "game.h"
#include "base.h"
#include "general.h"
#include "popup.h"
#include "basepop.h"
#include "infowin.h"
#include "map.h"
#include "path.h"
#include "mapwin.h"
#include "maininterface.h"
#include "statuswin.h"
#include "reportwin.h"
#include "win.h"
#include "fx.h"
#include "graphicwin.h"
#include "spritebox.h"
#include "net_class.h"
#include "menu.h"
#include "sprite.h"
#include "time.h"
#include "texture.h"
#include "strings.h"
#include "veh.h"
#include "faction.h"
#include "xpops.h"
#include "lock.h"
#include "log.h"
#include "netdaemon.h"
#include "netmsg.h"
#include "tutwin.h"
#include "technology.h"
#include "random.h"
#include "alpha.h"
#include "terraforming.h"
#include "planwin.h"
#include <cstdlib>
#include <cstring>
#include "buttongroup.h"
#include "console_commands.h"

void __cdecl base_kill(int base_id);
int __cdecl wants_to_speak(int faction_id, int other_faction_id);
void __cdecl action_oblit(int vehicle_id, int base_id);
int __cdecl tut_check(int marker);
int __cdecl diplo_lock(int code);
void __cdecl diplo_unlock();
void __cdecl kill(int veh_id);
void __cdecl read_faction(int player_id);
int __cdecl veh_pick(int veh_id, const char *label, int flags);
int __cdecl eliminate_player(int faction_id, int is_eliminated);
int __cdecl veh_init(int prototype, int faction, int x, int y);
void __cdecl draw_cursor();                               // 0x0046AE00
void __cdecl ambience_veh(int vehicle);
int __cdecl veh_ready(int veh_id);                        // 0x005C1B70 ?veh_ready@@YAHH@Z
int __cdecl tut_check(int veh_id);                            // 0x004BA5F0 ?tut_check@@YAHH@Z
int __cdecl tut_check2(int veh_id);                           // 0x004BA650 ?tut_check2@@YAHH@Z
int __cdecl save_game(int save_slot);                            // 0x005A9EB0 ?save_game@@YAHH@Z
void __cdecl base_kill(int base_id);                      // 0x004E5250 ?base_kill@@YAXH@Z
int __cdecl load_game(int save_slot, int faction_id);                    // 0x005AAAB0 ?load_game@@YAHHH@Z
int __cdecl load_map();                                   // 0x005AB600 ?load_map@@YAHXZ
void __cdecl save_map();                                  // 0x005ABB90 ?save_map@@YAXXZ
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
int __cdecl popv(char *caption, const char *label, int vehicle, int flags);
void __cdecl action_gate(int vehicle_id, int base_id);
extern int __cdecl pop_wait();

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

// RECOVERY_OBJECT: v2 0x00688F98 0x00688FA4 ?ConsoleStrResetAllFac@@3PADA 0x004DBB40
char ConsoleStrResetAllFac[] = "RESETALLFAC";  // 0x00688F98

// THE PER-FACTION DWORD RUN at 0x00946138 - game.h binds that address as
// TectonicDetonationCount, and editor_reset_faction is the reader that shows
// it is a RUN of eight, one per faction slot: it walks factions 1..7 and
// clears exactly one dword per pass, starting at 0x0094613C and stopping on
// the exclusive 0x00946158 where RulesResource Resource[] begins. The run
// sits between Citizen[9] and Resource[0], so it is not part of either array.

// Original .data string at 0x00688FA4 is "RESETALLTECH", the confirmation
// X_pop asks before editor_reset_tech throws every faction's
// research away.
// RECOVERY_OBJECT: v2 0x00688FA4 0x00688FB1 ?ConsoleStrResetAllTech@@3PADA 0x004DBC40
char ConsoleStrResetAllTech[] = "RESETALLTECH";  // 0x00688FA4

// Original .data strings passed by editor commands to x_pop_short / X_pop.
// RECOVERY_OBJECT: v2 0x00689174 0x0068917E ?ConsoleEditorRockinessLabel@@3PADA 0x004E14A0
char ConsoleEditorRockinessLabel[] = "ROCKOSITY";  // 0x00689174

// RECOVERY_OBJECT: v2 0x00689120 0x0068912C ?ConsoleStrEditorMinor@@3PADA 0x004E0AD0
char ConsoleStrEditorMinor[] = "EDITORMINOR";  // 0x00689120

// RECOVERY_OBJECT: v2 0x00689148 0x00689150 ?ConsoleStrMapFlat@@3PADA 0x004E0F30
char ConsoleStrMapFlat[] = "MAPFLAT";  // 0x00689148

// RECOVERY_OBJECT: v2 0x00689150 0x00689159 ?ConsoleStrMapRound@@3PADA 0x004E0F30
char ConsoleStrMapRound[] = "MAPROUND";  // 0x00689150

// RECOVERY_OBJECT: v2 0x006890F0 0x006890FB ?ConsoleStrEditorYear@@3PADA 0x004E0210
char ConsoleStrEditorYear[] = "EDITORYEAR";  // 0x006890F0

// RECOVERY_OBJECT: v2 0x0068915C 0x00689166 ?ConsoleStrFungosity@@3PADA 0x004E13E0
char ConsoleStrFungosity[] = "FUNGOSITY";  // 0x0068915C

// RECOVERY_OBJECT: v2 0x00689168 0x00689173 ?ConsoleStrFungmotize@@3PADA 0x004E13E0
char ConsoleStrFungmotize[] = "FUNGMOTIZE";  // 0x00689168

// RECOVERY_OBJECT: v2 0x00689198 0x006891A3 ?ConsoleStrProtectOff@@3PADA 0x004E1F70
char ConsoleStrProtectOff[] = "PROTECTOFF";  // 0x00689198

// RECOVERY_OBJECT: v2 0x006891A4 0x006891AE ?ConsoleStrProtectOn@@3PADA 0x004E1F70
char ConsoleStrProtectOn[] = "PROTECTON";  // 0x006891A4

// RECOVERY_OBJECT: v2 0x00688E44 0x00688E4F ?ConsoleStrDesignated@@3PADA 0x004D7CF0
char ConsoleStrDesignated[] = "DESIGNATED";  // 0x00688E44

// RECOVERY_OBJECT: v2 0x00688E50 0x00688E5D ?ConsoleStrUndesignated@@3PADA 0x004D7CF0
char ConsoleStrUndesignated[] = "UNDESIGNATED";  // 0x00688E50

// Original .data strings passed to the editor pop helpers.
// RECOVERY_OBJECT: v2 0x006890E0 0x006890ED ?ConsoleStrEditorEnergy@@3PADA 0x004E0120
char ConsoleStrEditorEnergy[] = "EDITORENERGY";  // 0x006890E0

// RECOVERY_OBJECT: v2 0x00689140 0x00689146 ?ConsoleStrBrush@@3PADA 0x004E0EA0
char ConsoleStrBrush[] = "BRUSH";  // 0x00689140

// Original .data labels sent to NetMsgPrimary. Duplicate texts remain distinct
// arrays because the image contains separate text objects for each use.
// RECOVERY_OBJECT: v2 0x00688C68 0x00688C70 ?ConsoleStrSimulNo@@3PADA 0x004D52A0
char ConsoleStrSimulNo[] = "SIMULNO";  // 0x00688C68

// RECOVERY_OBJECT: v2 0x00688C70 0x00688C7A ?ConsoleStrUnitMoved@@3PADA 0x004D52A0
char ConsoleStrUnitMoved[] = "UNITMOVED";  // 0x00688C70

// RECOVERY_OBJECT: v2 0x00688C7C 0x00688C84 ?ConsoleStrSimulNo2@@3PADA 0x004D52A0
char ConsoleStrSimulNo2[] = "SIMULNO";  // 0x00688C7C

// RECOVERY_OBJECT: v2 0x00688C84 0x00688C8E ?ConsoleStrUnitMoved2@@3PADA 0x004D52A0
char ConsoleStrUnitMoved2[] = "UNITMOVED";  // 0x00688C84

// RECOVERY_OBJECT: v2 0x00688C90 0x00688C9B ?ConsoleStrOnlyCombat@@3PADA 0x004D52A0
char ConsoleStrOnlyCombat[] = "ONLYCOMBAT";  // 0x00688C90

// RECOVERY_OBJECT: v2 0x00688C9C 0x00688CA4 ?ConsoleStrNotBase@@3PADA 0x004D52A0
char ConsoleStrNotBase[] = "NOTBASE";  // 0x00688C9C

// RECOVERY_OBJECT: v2 0x00688CA4 0x00688CAF ?ConsoleStrNotInTriad@@3PADA 0x004D52A0
char ConsoleStrNotInTriad[] = "NOTINTRIAD";  // 0x00688CA4

// RECOVERY_OBJECT: v2 0x00688E18 0x00688E29 _RoadToNeedTerraformers 0x004D7BE0
static char RoadToNeedTerraformers[] = "NEEDTERRAFORMERS";
// RECOVERY_OBJECT: v2 0x00688E2C 0x00688E37 _RoadToNotInTriad 0x004D7BE0
static char RoadToNotInTriad[] = "NOTINTRIAD";
// RECOVERY_OBJECT: v2 0x00688E38 0x00688E43 _RoadToNotInOcean 0x004D7BE0
static char RoadToNotInOcean[] = "NOTINOCEAN";
// RECOVERY_OBJECT: v2 0x00688EC8 0x00688ED3 _SetRoadToNotInOcean 0x004D8DC0
static char SetRoadToNotInOcean[] = "NOTINOCEAN";

// RECOVERY_OBJECT: v2 0x00688CB0 0x00688CBC ?ConsoleStrDestroyBomb@@3PADA 0x004D52A0
char ConsoleStrDestroyBomb[] = "DESTROYBOMB";  // 0x00688CB0

// RECOVERY_OBJECT: v2 0x00688CBC 0x00688CC4 ?ConsoleStrDestroy@@3PADA 0x004D52A0
char ConsoleStrDestroy[] = "DESTROY";  // 0x00688CBC

// RECOVERY_OBJECT: v2 0x00688CC4 0x00688CCC ?ConsoleStrNotHere@@3PADA 0x004D52A0
char ConsoleStrNotHere[] = "NOTHERE";  // 0x00688CC4

// RECOVERY_OBJECT: v2 0x00688EC0 0x00688EC7 ?ConsoleStrNoBomb@@3PADA 0x004D8CD0
char ConsoleStrNoBomb[] = "NOBOMB";  // 0x00688EC0

// RECOVERY_OBJECT: v2 0x00688FF4 0x00688FFE ?ConsoleStrScenParam@@3PADA 0x004DCCC0
char ConsoleStrScenParam[] = "SCENPARAM";  // 0x00688FF4

// RECOVERY_OBJECT: v2 0x00688F6C 0x00688F76 ?ConsoleStrEditorWhoListTitle@@3PADA 0x004D9AD0
char ConsoleStrEditorWhoListTitle[] = "EDITORWHO";  // 0x00688F6C

// RECOVERY_OBJECT: v2 0x00689000 0x0068900C ?ConsoleStrCustomNames@@3PADA 0x004DD1B0
char ConsoleStrCustomNames[] = "CUSTOMNAMES";  // 0x00689000

// The two disband dialog arms select distinct writable text objects. Equal
// text used by other commands has a separate original identity. The NUL bytes
// are known; full original allocation bounds and lifetime remain unproved.
// RECOVERY_OBJECT: v1 0x00688908 _console_disband_alien_picture
static char console_disband_alien_picture[] = "al_col_sm.pcx";
// RECOVERY_OBJECT: v1 0x00688918 _console_disband_alien_caption
static char console_disband_alien_caption[] = "DISBAND";
// RECOVERY_OBJECT: v1 0x00688920 _console_disband_human_picture
static char console_disband_human_picture[] = "colfoun_sm.pcx";
// RECOVERY_OBJECT: v1 0x00688930 _console_disband_human_caption
static char console_disband_human_caption[] = "DISBAND";

// RECOVERY_OBJECT: v1 0x0068834C ?GoToLastSelection@@3HA
int GoToLastSelection = -1;

// Distinct writable message cells consumed by the original artillery command.
// RECOVERY_OBJECT: v1 0x00688EF0 _ConsoleArtySimulNoText
static char ConsoleArtySimulNoText[] = "SIMULNO";
// RECOVERY_OBJECT: v1 0x00688EF8 _ConsoleArtyUnitMovedText
static char ConsoleArtyUnitMovedText[] = "UNITMOVED";
// RECOVERY_OBJECT: v1 0x00688F04 _ConsoleArtySeaText
static char ConsoleArtySeaText[] = "SEAARTY";

// RECOVERY_OBJECT: v1 0x00688568 _ConsoleUnloadNoText
static char ConsoleUnloadNoText[] = "UNLOADNO";
// RECOVERY_OBJECT: v1 0x00688574 _ConsoleUnloadNextText
static char ConsoleUnloadNextText[] = "UNLOADNEXT";

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
    if (Players[Bases[base].faction_id_current].rule_flags & 0x80) {
        if (!popp(ScriptPopupFile, console_disband_alien_caption, 0, console_disband_alien_picture, nullptr)) return;
    } else {
        if (!popp(ScriptPopupFile, console_disband_human_caption, 0, console_disband_human_picture, nullptr)) return;
    }
    const int faction = Bases[base].faction_id_current;
    base_kill(base);
    ::draw_map(1);
    ConsoleStatusWin.redraw();
    eliminate_player(faction, 0);
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
            LPSTR name = Morale[morale_id].name;
            const VehPrototype &listed_prototype = VehPrototypes[vehicle.proto_id];
            if (vehicle.proto_id < 0x40 &&
                (Weapon[listed_prototype.weapon_id].offense_rating < 0 || vehicle.proto_id == 0x0F)) {
                name = Morale[morale_id].name_lifecycle;
            }
            popup.dialogs().item(StringTable.get(name), morale_id);
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
            popup.dialogs().item(StringTemp, faction_id);
        }
        popup.set_value(vehicle.faction_id);
        const int faction = popup.exec(0, nullptr);
        if (faction < 0) {
            return;
        }
        vehicle.faction_id = static_cast<uint8_t>(faction);
        ::draw_tile(vehicle.x, vehicle.y, 1);
        ConsoleGlobal.update_data(0);
        return;
    }

    if (ConsoleGroupAction == 4) {
        popup.start(PopupStartCaption, "EDITVEHHOME", -1, nullptr, 0x42, nullptr);
        StringTemp[0] = '\0';
        ::strcat(StringTemp, label_get(25));
        popup.dialogs().item(StringTemp, 9999);
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current == vehicle.faction_id) {
                popup.dialogs().item(Bases[base_id].name_string, base_id);
            }
        }
        popup.set_value(vehicle.home_base_id < 0 ? 9999 : vehicle.home_base_id);
        int selected_base = popup.exec(0, nullptr);
        if (selected_base < 0) {
            return;
        }
        if (selected_base == 9999) selected_base = -1;
        vehicle.home_base_id = static_cast<int16_t>(selected_base);
        ::draw_tile(vehicle.x, vehicle.y, 1);
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
    ::draw_tile(Vehs[vehicle_id].x, Vehs[vehicle_id].y, 1);
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

// The original order at 0x004D7BE0 checks the rover's plan, chassis and
// current terrain before placing a road cursor.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D7BE0 ?road_to@Console@@QAEXHH@Z 0x004D7BE0-0x004D7CED BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d7be0.cpp"], "reason": "Seed 004d7be0 identifies the Console road cursor order and extent; typed vehicle, prototype, terrain and technology owners preserve the sequence."}
void Console::road_to(int veh_id, int cursor_mode) {
    const Veh &vehicle = Vehs[veh_id];
    const int prototype_id = Vehs[veh_id].proto_id;
    if (VehPrototypes[prototype_id].plan != 9) {
        NetMsgPrimary.pop(RoadToNeedTerraformers, 5000, 0, 0);
        return;
    }
    if (Chassis[VehPrototypes[prototype_id].chassis_id].triad != TRIAD_LAND) {
        NetMsgPrimary.pop(RoadToNotInTriad, 5000, 0, 0);
        return;
    }
    const Map &tile = map_tiles()[vehicle.y * MapLongitude + (vehicle.x >> 1)];
    if ((tile.climate & ~0x1F) < 0x60) {
        NetMsgPrimary.pop(RoadToNotInOcean, 5000, 0, 0);
        return;
    }
    if (cursor_mode != 0x1C ||
        has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0], vehicle.faction_id)) {
        ConsoleGlobal.cursor_on((cursor_mode == 0x1C) + 2, -1);
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
            if (chassis.triad != TRIAD_LAND || (tile.climate & ~0x1F) < 0x60) {
                return;
            }
        }
        if (order == 2) {
            if (!has_tech(Terraforming[TERRA_FARM].prerequisite_tech[0],
                          vehicle.faction_id)) {
                return;
            }
            direct_lock = true;
        }
        if (!direct_lock && (order == 4 || order == 5) &&
            base_who(vehicle.x, vehicle.y) >= 0) {
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
        Veh &locked_vehicle = Vehs[veh_id];
        const VehPrototype &locked_prototype =
            VehPrototypes[locked_vehicle.proto_id];
        const RulesChassis &locked_chassis = Chassis[locked_prototype.chassis_id];
        locked_vehicle.waypoint_x[1] = locked_vehicle.x;
        locked_vehicle.waypoint_y[1] = locked_vehicle.y;
        if (locked_chassis.range != 0) {
            action_home(veh_id, 0);
            if (Vehs[veh_id].order == 0x18) {
                Veh &after_home_vehicle = Vehs[veh_id];
                after_home_vehicle.waypoint_x[1] =
                    after_home_vehicle.waypoint_x[0];
                after_home_vehicle.waypoint_y[1] =
                    after_home_vehicle.waypoint_y[0];
            }
            Vehs[veh_id].order = 0;
        }
    }
    Veh &updated_vehicle = Vehs[veh_id];
    updated_vehicle.order_auto_type = static_cast<uint8_t>(order);
    updated_vehicle.order = 0;
    updated_vehicle.state = (updated_vehicle.state & 0xFCFFFFFF) | 0x200;
    updated_vehicle.waypoint_count = 0;
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
}

// A convoy either opens the supply panel at its base or chooses an extraction
// resource at its current site. The vehicle lock covers both interactions.
// Resource labels are RulesResource members. Before the base supply panel,
// the original sets the already-owned NetDaemon channel action word to 3.
// Keep this write ahead of the panel's callback-capable work.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D8FA0 ?convoy@Console@@QAEXH@Z 0x004D8FA0-0x004D916D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d8fa0.cpp", "source:src/alpha.h", "pinned:0x004D8FF4 dword value 3 store before supply_options at 0x004D8FFE", "source:src/netdaemon.h:channels_ at 0x1B68 and LockStateEntry::leading_words_[3] at 0xC", "source:src/console.cpp:supply_options reads the same source-owned action flags"], "reason": "Seed 004d8fa0 identifies the Console convoy order and lock lifecycle; typed Veh, Base and RulesResource records replace receiver and table offsets. The resource label run at 0x0094615C is Resource[0].name_plural, with the singular member selected for a yield of one. The original pre-panel dword value 3 write at 0x004D8FF4 targets the already-owned NetDaemonNet.channels_[0].leading_words_[3], whose singleton-relative offset 0x1B74 identifies 0x0093E904. Preserve that write before supply_options without adding duplicate storage or claiming full matching/acceptance."}
// Original convoy pushes at 004D9057/004D911D select distinct writable strings.
// Current NUL-terminated literals have separate complete COMDATs, not lifetime proof.
// RECOVERY_LITERAL: v1 0x00688ED4 0x00688EDF ??_C@_0L@OHNN@CANTCONVOY?$AA@ 0x004D8FA0
// RECOVERY_LITERAL: v1 0x00688EE0 0x00688EEE ??_C@_0O@EAKA@SUPPLYEXTRACT?$AA@ 0x004D8FA0
void Console::convoy(int veh_id) {
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return;
    }
    const int base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
    if (base_id >= 0) {
        NetDaemonNet.channels_[0].leading_words_[3] = 3;
        supply_options(veh_id, base_id);
        NetDaemonNet.unlock_veh();
        return;
    }
    const int home_base_id = Vehs[veh_id].home_base_id;
    if (home_base_id >= 0) {
        if (can_convoy(Vehs[veh_id].x, Vehs[veh_id].y) == 0) {
            NetMsgPrimary.pop("CANTCONVOY", 5000, 0, 0);
            NetDaemonNet.unlock_veh();
            return;
        }
        int amounts[3];
        for (int resource = 0; resource < 3; ++resource) {
            amounts[resource] = resource_yield(resource, faction_id_,
                                               Vehs[veh_id].home_base_id,
                                               Vehs[veh_id].x, Vehs[veh_id].y);
            parse_num(resource, amounts[resource]);
            LPSTR name = amounts[resource] == 1
                ? Resource[resource].name_singular
                : Resource[resource].name_plural;
            parse_say(resource, name, -1, -1);
        }
        parse_says(3, Bases[home_base_id].name_string, -1, -1);
        const int choice = x_pop_short("SUPPLYEXTRACT", 0x40, 0);
        if (choice >= 0) {
            Vehs[veh_id].order = 3;
            Vehs[veh_id].order_auto_type = static_cast<uint8_t>(choice);
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
        }
    }
    NetDaemonNet.unlock_veh();
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

// ORIGINAL: 0x004D0450 ?skip@Console@@QAEXH@Z 0x004D0450-0x004D0494 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004D0450-0x004D0494", "source:src/netdaemon.h", "source:src/veh.h", "source:src/game.h", "source:src/console.h"], "reason": "Home the skip order through the typed vehicle-ready predicate, turn guard and NetDaemon order API, and the already named Console skip-state member; remove the seed's receiver/table pointer arithmetic."}
// The seed's blocked callee at 0x00531780 is now NetDaemon::order_veh; its
// Historical seed warning: the header definition inlined not_my_turn here,
// unlike the original call at 0x0052DC70. Its ordinary definition is in game.cpp.
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
        NetMsgPrimary.pop(SetRoadToNotInOcean, 5000, 0, 0);
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
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return;
    }

    const Veh &vehicle = Vehs[veh_id];
    const int x_coord = vehicle.x;
    const int y_coord = vehicle.y;
    const int tile_index = MapLongitude * y_coord + (x_coord >> 1);
    const Map &tile = map_tiles()[tile_index];
    const RulesChassis &chassis = Chassis[VehPrototypes[vehicle.proto_id].chassis_id];

    bool should_hold = (tile.climate & ~0x1F) >= 0x60 || chassis.triad != 0;
    if (!should_hold) {
        if (base_who(x_coord, y_coord) < 0) {
            NetDaemonNet.unlock_veh();
            return;
        }
    }

    if (chassis.triad == TRIAD_AIR && chassis.range != 0) {
        if (base_who(x_coord, y_coord) < 0) {
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
    ::draw_tile(x_coord, y_coord, 1);

    if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_ALIEN_ARTIFACT) {
        return;
    }
    const int owner = base_who(x_coord, y_coord);
    if (owner == Vehs[veh_id].faction_id) {
        NetDaemonNet.order_veh(veh_id, -1, 0);
    }
}

// ORIGINAL: 0x004D04E0 ?unload@Console@@QAEXH@Z 0x004D04E0-0x004D06B6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004D04E0-0x004D06B6", "image:.data:0x00688568 contains UNLOADNO and .data:0x00688574 contains UNLOADNEXT", "source:src/veh.h", "source:src/netdaemon.h", "source:src/netmsg.h", "source:src/console.h"], "reason": "Recover cargo unloading through Veh stack links, chassis traits and source-owned movement/ability predicates; preserve the daemon lock/synchronization lifecycle, selected-vehicle Console state, and the image's localization keys without absolute-address aliases."}
void Console::unload(int veh_id) {
    int flags = 0;
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return;
    }
    if (veh_cargo(veh_id) == 0 &&
        has_abil(Vehs[veh_id].proto_id, 0x80) == 0) {
        NetDaemonNet.unlock_veh();
        return;
    }
    const int wanted_triad = has_abil(Vehs[veh_id].proto_id, 0x80) ? 2 : 0;
    int first_cargo = veh_id;
    if (first_cargo < 0) {
        first_cargo = -1;
    } else {
        while (Vehs[first_cargo].prev_veh_id_stack >= 0) {
            first_cargo = Vehs[first_cargo].prev_veh_id_stack;
        }
    }
    veh_id = first_cargo;
    while (veh_id >= 0) {
        if (Vehs[veh_id].order == 1) {
            Vehs[veh_id].order = 0;
            synch_veh(veh_id);
        }
        const RulesChassis &chassis = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id];
        if (chassis.triad == wanted_triad) {
            flags |= 1;
            if (veh_unmoved(veh_id) != 0 && Vehs[veh_id].order == 0) {
                ConsoleGlobal.selected_vehicle_ = veh_id;
                flags |= 2;
            }
        }
        if (veh_id < 0) break;
        veh_id = Vehs[veh_id].next_veh_id_stack;
    }
    NetDaemonNet.unlock_veh();
    if (flags == 0) {
        NetMsgPrimary.pop(ConsoleUnloadNoText, 0x1388, 0, nullptr);
        return;
    }
    if ((flags & 2) == 0) {
        NetMsgPrimary.pop(ConsoleUnloadNextText, 0x1388, 0, nullptr);
        return;
    }
    field_23BE4_ = 0;
    set_move(1);
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
    if (X_pop(ConsoleStrResetAllFac, (NullCallback)0) != 0) {
        PlayerData *faction = &PlayersData[1];
        unsigned int factionId = 1;
        uint32_t *rules_flag = TectonicDetonationCount + 1;
        do {
            faction->se_alloc_labs = 5;
            faction->se_alloc_psych = 0;
            compute_faction_modifiers(factionId);

            for (int i = 0; i < 4; ++i) {
                faction->soc_category_active.models_[i] = 0;
                faction->soc_category_pending.models_[i] = 0;
            }

            social_upkeep(factionId);
            int count = GameDataStateStorage.fields_.base_current_count_;
            faction->major_atrocities = 0;
            faction->atrocities = 0;
            *rules_flag = 0;
            faction->integrity_blemishes = 0;

            if (count > 0) {
                Base *base = Bases;
                do {
                    if (base->faction_id_current == factionId) {
                        base->governor &= 0x7fffffff;
                        base->queue_size = 0;
                    }
                    ++base;
                    --count;
                } while (count != 0);
            }
            int vehicle_count = GameDataStateStorage.fields_.vehicle_current_count_;
            if (vehicle_count > 0) {
                Veh *vehicle = Vehs;
                do {
                    if (vehicle->faction_id == factionId) {
                        vehicle->order = 0;
                        vehicle->state &= 0xfcffbdff;
                    }
                    ++vehicle;
                    --vehicle_count;
                } while (vehicle_count != 0);
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
    const int initial_area = MapArea;
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
        } while (count < MapArea);
    }
    reset_territory();
    ::draw_map(1);
}

// ORIGINAL: 0x004E1290 ?editor_clear@Console@@QAEXXZ 0x004E1290-0x004E12FB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:d5d0b2c01472baa3cf01dda67b73e270d64b4db7:src/recovered/units/004e1290.cpp;source:src/console_commands.cpp:Console::editor_clear_vehicles;source:src/map.h;source:src/path.h;source:src/worldwin.h;source:src/game.h;source:src/veh.h;source:src/base.h;source:src/console.h"], "reason": "Recover the editor map-reset coordinator through the typed GameState, vehicle/base counts, Console cursor state, map lifecycle, Path and WorldWin owners. The historical body establishes the ordered reset and redraw operations."}
void Console::editor_clear() {
    auto_undo();
    uint32_t pending_state = GameDataStateStorage.fields_.game_state_;
    pending_state |= STATE_UNK_4;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    GameDataStateStorage.fields_.game_state_ = pending_state;
    map_wipe();
    size_of_planet(2);
    map_shutdown();
    map_init();
    Paths.init();
    map_wipe();
    ::draw_map(1);
    WorldClimateWorldWin.GraphicWin::redraw();
}

// ORIGINAL: 0x004E0210 ?editor_date@Console@@QAEXXZ 0x004E0210-0x004E0282 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004e0210.cpp;source:src/console_commands.cpp:ConsoleStrEditorYear;source:src/game.h:game_year;source:src/game.h:TurnCurrentNum;source:src/game.h:StartingMissionYear;source:src/general.h:ParseNumTable;source:src/mapwin.h:MapWinTable;source:src/win.h:Win::on_redraw;source:src/basewin.h:BaseWin::on_redraw;source:src/console.h:Console::update_data"], "reason": "Recover the editor year prompt from its named EDITORYEAR string, the current turn/year fields, and the existing Console, MapWin and BaseWin owners. The seed’s indirect call through the primary MapWin pointer reaches the virtual GraphicWin base at Win::on_redraw slot 12."}
void Console::editor_date() {
    auto_undo();
    const int accepted = X_pop_ask_number(
        ConsoleStrEditorYear, static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_)), 0);
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

// Fixed-slot bindings carried from 004dbc40.cpp

// ORIGINAL: 0x004DBC40 ?editor_reset_tech@Console@@QAEXXZ 0x004DBC40-0x004DBD18 FILE
// RECOVERY_BINDING: v1 {"evidence": ["PlayersData96C9E0 stride20CC, rank35C:96EE08 is faction1 rank; seven advances reach97D39C, so faction0 remains untouched.", "Shared GameDataStateStorage9A6490 bitmap+1E0 is9A6670, exactly89 bytes; prototype9AB868 stride34 obsolete2C/flags30,512 records with flag-bit clearing only for IDs64..511.", "Vehs952828 stride34 signed proto_id+0A is952832; Console selected_vehicle_+23BDC is reread/decremented after veh_kill, which may clear selection and stop the walk at-2.", "Bases97D040 stride134 queue_size4C is97D08C, next dword is queue_production_id[0]; snapshot base count after the vehicle walk, clear queue_size then replace it with bit patternFFFFFFBB only for a signed queue item>=40.", "Pinned complete body4DBC40..4DBD18: popup acceptance precedes all resets; faction ranks,89-byte bitmap,512 prototypes,vehicle walk,base walk,draw_map(1) occur in that order.", "Pinned prototype loop clears flags bit0 with a byte-width AND; the named uint16_t flags expression preserves its final bits, while physical access width remains a strict compiler/matching obligation."], "reason": "Reset technology through genuine faction, shared game-state, prototype, vehicle and base owners. Preserve signed prototype tests, the kill callback's live selected-vehicle mutation, the post-kill base-count snapshot and original queue-size sentinel."}
// TRIED: `field_23BDC_` named field is WRONG - it lands at 0x245f0, not 0x23bdc (raw `self+0x23bdc` cast is correct, per the "scaffold's named fields can be wrong" caution). `reinterpret_cast<int>` on the three pointer loop bounds gets `jl` instead of `jb` (pointer `<` compiles unsigned). Landed at MNEMONIC_ONLY: the one remaining diff is the `*p=0` store and the `p[1]` load at the very last loop swapped order (compiler schedules the independent load before the store regardless of source order) - tried a named temp, a ternary store, and an if/else split; none reproduced store-then-load.
// working copy - scaffold materialised by --work
// size      216 bytes
// prototype void (__thiscall ?editor_reset_tech@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005BF310 0x005C08C0

void Console::editor_reset_tech() {
    if (X_pop(ConsoleStrResetAllTech, 0) != 0) {
        for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
            PlayersData[faction_id].tech_ranking = 0;
        }

        memset(GameDataStateStorage.fields_.game_tech_achieved_, 0, MaxTechnologyNum);

        for (int prototype_id = 0; prototype_id < MaxVehProtoNum; ++prototype_id) {
            VehPrototype &prototype = VehPrototypes[prototype_id];
            prototype.obsolete_factions = 0;
            if (prototype_id >= MaxVehProtoFactionNum) {
                unsigned char &flags_low = *reinterpret_cast<unsigned char *>(&prototype.flags);
                flags_low &= 0xfe;
            }
        }

        selected_vehicle_ = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
        if (selected_vehicle_ >= 0) {
            do {
                if (Vehs[selected_vehicle_].proto_id >= 0x40) {
                    veh_kill(selected_vehicle_);
                }
                // veh_kill may clear this selection: reread the live member.
                --selected_vehicle_;
            } while (selected_vehicle_ >= 0);
        }

        int base_count = GameDataStateStorage.fields_.base_current_count_;
        if (base_count > 0) {
            Base *base = Bases;
            do {
                base->queue_size = 0;
                const int next_queue_item = base->queue_production_id[0];
                if (next_queue_item >= 0x40) {
                    base->queue_size = 0xFFFFFFBBU;
                }
                ++base;
                --base_count;
            } while (base_count != 0);
        }
        ::draw_map(1);
    }
}

// Fixed-slot bindings carried from 004e0a00.cpp

// ORIGINAL: 0x004E0A00 ?editor_load@Console@@QAEXXZ 0x004E0A00-0x004E0A50 FILE BYTE_EXACT
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
    ::draw_map(1);
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
        } while (i < MapArea);
    }

    ConsoleGlobal.field_23BF8_ = 0;
    GameDataStateStorage.fields_.game_state_ = GameDataStateStorage.fields_.game_state_ | 4;
    // 0x004E1033 loads slot zero before accessing the window's +0x1DD98
    // member. The array itself is neither the receiver nor member storage.
    MapWinTable[0]->zoom_offset_ = -14;
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

// ORIGINAL: 0x004E1190 ?editor_randomize@Console@@QAEXXZ 0x004E1190-0x004E11F7 FILE BYTE_EXACT
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
    ::draw_map(1);
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
        ::draw_map(1);
        return;
    }
    const int initial_area = MapArea;
    Map *tile = MapTiles;
    MapRandSeed = 0;
    if (initial_area > 0) {
        int i = 0;
        do {
            uint32_t v = tile->bit;
            if ((v & 0x400) != 0 && (v & 0x20000000) == 0) {
                v &= 0xdffffbff;
                tile->bit = v;
                uint32_t *dst = tile->bit_visible;
                int count = 7;
                do {
                    *dst = tile->bit;
                    dst++;
                    count--;
                } while (count != 0);
            }
            i++;
            ++tile;
        } while (i < MapArea);
    }
    ::draw_map(1);
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
//   2. `ConsoleEditorRockinessLabel` is the image's .data string "ROCKOSITY" at
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
// is bound as `ConsoleEditorRockinessLabel` at 0x00689174. No forwarder was needed -
// world_rocky is a real definition in map.cpp, not a pending_bodies stub.


void Console::editor_rockiness() {
    auto_undo();
    if (x_pop_short(ConsoleEditorRockinessLabel, 0x40, 0) != 0) {
        world_rocky();
        ::draw_map(1);
    }
}

// ORIGINAL: 0x004E13E0 ?editor_fungus@Console@@QAEXXZ 0x004E13E0-0x004E149B FILE BYTE_EXACT
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
// Restoration note: `initial_area` models only the original guard snapshot;
// the loop tail below still reloads MapArea as the pinned body does. Hoisting
// that initial MapArea/map_tiles/counter setup also preserves its execution
// when the guard is nonpositive, unlike the earlier conditional placement.

// size      187 bytes
// prototype void (__thiscall ?editor_fungus@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x005BF3F0 0x005C3440


void Console::editor_fungus() {
    auto_undo();
    const int answer = x_pop_short(ConsoleStrFungosity, 0x40, 0);
    if (answer >= 0) {
        // The image carries no boolean here: it tests the FUNGMOTIZE answer
        // directly (`je 0x4e1472` at 0x004E141B skips the plant loop) and
        // falls into the loop on the answer == 0 path, where the second prompt
        // was never asked. Seeding `confirmed` with -1 makes one test spell
        // both of those.
        int confirmed = -1;
        if (answer > 0) {
            confirmed = x_pop_short(ConsoleStrFungmotize, 0x40, 0);
            if (confirmed < 0) {
                return;
            }
        }
        if (confirmed != 0) {
            // The initial signed MapArea and MapTiles loads, then the zero
            // loop counter, all precede the positive-area guard in the image.
            // The loop back edge separately reloads signed MapArea each pass.
            const int initial_area = MapArea;
            Map *tile = map_tiles();
            int i = 0;
            if (initial_area > 0) {
                do {
                    tile->bit &= 0xFFFFFFDFu;
                    faction_id_ = 1;
                    do {
                        tile->bit_visible[faction_id_ - 1] = tile->bit;
                        faction_id_ = faction_id_ + 1;
                    } while (faction_id_ < 8);
                    i++;
                    tile++;
                } while (i < MapArea);
            }
        }
        if (answer > 0) {
            uint32_t saved = MapNativeLifeForms;
            MapNativeLifeForms = answer - 1;
            world_fungus();
            MapNativeLifeForms = saved;
        }
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
        NetMsgPrimary.pop(ConsoleStrProtectOff, 5000, 0, 0);
        MainInterfaceGlobal.draw_edit_pane();
        return;
    }
    GameDataStateStorage.fields_.game_state_ = flags | lock_mask;
    NetMsgPrimary.pop(ConsoleStrProtectOn, 5000, 0, 0);
    if (ConsoleGlobal.cursor_active_ != 0) {
        set_view(1);
    }
    MainInterfaceGlobal.draw_edit_pane();
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
    for (; i < MapArea; i++, tile++) {
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
// console.cpp's ConsoleStrMapFlat/ConsoleStrMapRound, and 0x008E9F60 is map.cpp's
// WorldClimateWorldWin (declared here as a HANDOFF row until the shared
// extern lands in worldwin.h).


void Console::editor_flat() {
    auto_undo();
    MapIsFlat ^= 1;
    ::draw_map(1);
    WorldClimateWorldWin.GraphicWin::redraw();
    if (MapIsFlat & 1) {
        X_pop(ConsoleStrMapFlat, (NullCallback)0);
        return;
    }
    X_pop(ConsoleStrMapRound, (NullCallback)0);
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
    Map *base = map_tiles();
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    GameDataStateStorage.fields_.game_state_ = state;
    for (int i = 0; i < MapArea; i++) {
        base[i].bit &= 0xfffffffc;
    }
    unsigned int flag80 = GameDataStateStorage.fields_.game_state_ & 0x80;
    GameDataStateStorage.fields_.game_state_ = GameDataStateStorage.fields_.game_state_ & 0xffffff7f;
    world_build();
    if (flag80 != 0) {
        GameDataStateStorage.fields_.game_state_ = GameDataStateStorage.fields_.game_state_ | 0x80;
    }
    unsigned char al = static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
    if ((al & 0x80) != 0 && (al & 0x40) != 0) {
        MainInterfaceGlobal.flat_button_[2].show(0);
    } else {
        MainInterfaceGlobal.flat_button_[2].hide();
    }
    ::draw_map(1);
}

// ORIGINAL: 0x004E0A50 ?editor_paints@Console@@QAEXH@Z 0x004E0A50-0x004E0ACF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x004E0A50 scans rule rows from 19 down to 0 and reads each mutable Terraforming row's bit at base 0x00691878 + 0x10 + row*0x20.", "src/terraforming.h: RulesTerraforming is 0x20 bytes and declares bit at +0x10; src/alpha.cpp maps the constructed Terraforming array at 0x00691878.", "Pinned MainInterface constructor 0x0045EF10 constructs four ButtonGroups at +0x1E878 with stride 0x94; group 2 starts at 0x007CD1C0 and its field_84_ is 0x007CD244.", "MainInterface::init populates button_group_[2] with the 25 paint ids 1011..1035; ButtonGroup::set stores the selected id in field_84_.", "MainInterface exposes button_group(int) as the public accessor for its private button_group_ array; Console::editor_set_brush uses that accessor for group 3."], "reason": "Read paint availability from the source-owned Terraforming objects at the original row stride, and use MainInterfaceGlobal's public button_group(2) accessor for selected-id comparison and notification, preserving the original conditional set call."}
// TRIED: `a1 > 5` (jl/jle flip at #12, fixed by `a1 >= 6`); `counter = a1 - 6` as one expression (lea vs sub at #15, tried splitting into `counter=a1; counter-=6;`, no change). Same size as original (127/127), mismatch only at #15.
// size      127 bytes
// prototype void (__thiscall ?editor_paints@Console@@QAEXH@Z)(Console* this, int)
// callers   5   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458910 0x0045B550 0x0062B870
//
// ConsoleInfoWin owns the redraw phase; MainInterfaceGlobal owns the paint
// ButtonGroup and selection that the original accessed at 0x007CD1C0/244.


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
    // MainInterface's constructed group 2 owns the selected paint id and
    // receives set(result, 1) only when that id differs.
    InfoWin *const win = &ConsoleInfoWin;
    MapEditorPaintId = paint_id;
    ConsoleInfoWin.set_redraw_phase(7);
    win->on_redraw();
    MainInterfaceGlobal.draw_edit_pane();
    int result = paint_id + 0x3f3;
    if (paint_id >= 6 && paint_id < 0x1a) {
        paint_id -= 6;
        for (int limit = 19; limit >= 0; --limit) {
            if (Terraforming[limit].bit != 0) continue;
            if (paint_id < limit) continue;
            --paint_id;
        }
        result = paint_id + 0x3f9;
    }
    if (static_cast<int>(MainInterfaceGlobal.button_group(2).field_84_) != result) {
        MainInterfaceGlobal.button_group(2).set(result, 1);
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
// ConsoleStrEditorMinor.


void Console::editor_minor() {
    int paint = x_pop_short(ConsoleStrEditorMinor, 0x40, (NullCallback)0);
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
                NetMsgPrimary.pop(ConsoleStrDesignated, 5000, 0, 0);
                return;
            }
            NetMsgPrimary.pop(ConsoleStrUndesignated, 5000, 0, 0);
        }
    }
}

/*
Purpose: Ask the energy priority for one faction and refresh it - seeding the
         parser's per-faction defaults while the prompt loop runs.
// ORIGINAL: 0x004E0120 ?editor_energy@Console@@QAEXXZ 0x004E0120-0x004E020E FILE BYTE_EXACT
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
        // the faction label and that faction's current energy reserve to the
        // script parser. The pinned loop starts at PlayersData[1] + 0x220
        // (0x96ECCC) and advances by the 0x20CC faction-record stride.
        for (int loop_faction = 1; loop_faction < MaxPlayerNum; ++loop_faction) {
            GenderDefault = Players[loop_faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(loop_faction, Players[loop_faction].name_leader, -1, -1);
            parse_num(loop_faction, PlayersData[loop_faction].energy_reserves);
        }

        const int answer = X_pop_ask_number(
            ConsoleStrEditorEnergy, PlayersData[faction].energy_reserves,
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
// ORIGINAL: 0x004D7EE0 ?set_patrol@Console@@QAEXH@Z 0x004D7EE0-0x004D7F5F FILE BYTE_EXACT
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
    // 1..7 only when view_active_ says they hold a live view.
    for (int slot = 0; slot < 8; ++slot) {
        if (MapWinTable[slot] != 0 &&
            (slot == 0 || MapWinTable[slot]->view_active_ != 0)) {
            MapWinTable[slot]->field_1DD70_ |= 0x400000;
            MapWinTable[slot]->draw_base_dest(1);
        }
    }
    ConsoleGlobal.cursor_on(8, -1);
}

/*
Purpose: Ask which terrain-improvement brush the scenario editor paints with
         and refresh the interface panes around the answer.
// ORIGINAL: 0x004E0EA0 ?editor_set_brush@Console@@QAEXXZ 0x004E0EA0-0x004E0F22 FILE BYTE_EXACT
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
        x_pop_value_flags(ConsoleStrBrush, MapEditorBrushId, 0x40, (NullCallback)0);
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

// 0x004D9AD0 ?editor_who@Console@@QAEHHH@Z - which faction/personality the
// scenario editor is working on. The catalogue, the partition and
// editor_energy's own call list all name this address (span
// 0x004D9AD0-0x004D9DB4); the console.h declaration carries the corrected
// address too.
// The picker calls Dialogs::item on the Popup's +0x21D0 member: original
// popup stack slot -0x5388 plus 0x21D0 equals the item receiver -0x31B8.
// Its automatic destructor owns the historical teardown sequence.
// ORIGINAL: 0x004D9AD0 ?editor_who@Console@@QAEHHH@Z 0x004D9AD0-0x004D9DB4;0x0065B275-0x0065B347 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/004d9ad0.cpp", "source:src/console_commands.cpp:Console::editor_diplomacy", "source:src/faction.h:Player", "source:src/basepop.h:BasePop"], "reason": "Recover the editor faction picker through the typed popup, dialog member, faction nouns and post-start default value; C++ lifetime replaces the seed's explicit teardown. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// TRIED: a plain `Popup popup;` local reproduces the SEH frame, the __alloca_probe (Popup is ~21KB), and the whole close()-then- automatic-dtor cascade over its embedded scroll_/sprite_/ flat_button1_/flat_button2_/heap_ members for free - matched through instruction #11.
// TRIED: MISMATCH #11 'push' vs 'lea', not chased further (950B, heaviest object graph in this batch: Popup->BasePop-> {Scroll{2x FlatButton}, 2x FlatButton, Sprite, Heap} plus separate Dialogs/Spot locals and two `sub_4066c0`-typed objects the emitter could only give a nullary cdecl decl, handled via a `Sub4066c0Thunk` shim class for the thiscall). Exact interleaving of the inlined implicit-dtor chain against the separate Dialogs/Spot/mystery destructors is optimizer scheduling, not fully controlled from source.
int Console::editor_who(int default_faction, int first_faction) {
    Popup popup;
    PopupModalState = 6;
    popup.start(PopupStartCaption, ConsoleStrEditorWhoListTitle, -1, 0, 0x40, nullptr);

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
    popup.start(PopupStartCaption, "EDITORDIFF", -1, nullptr, 0x40, nullptr);
    for (int i = 0; i < MaxDiffNum; ++i) {
        popup.dialogs().item(StringTable.get(Difficulty[i]), i);
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
    popup.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40, nullptr);
    for (int faction_entry = 1; faction_entry < MaxPlayerNum; ++faction_entry) {
        Player &row = Players[faction_entry];
        const int plurality = row.is_noun_plural;
        const int gender = row.noun_gender;
        PluralityDefault = plurality;
        GenderDefault = gender;
        popup.dialogs().item(row.noun_faction, faction_entry);
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
    popup.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40, nullptr);
    for (int faction_entry = 1; faction_entry < MaxPlayerNum; ++faction_entry) {
        Player &row = Players[faction_entry];
        const int plurality = row.is_noun_plural;
        const int gender = row.noun_gender;
        PluralityDefault = plurality;
        GenderDefault = gender;
        popup.dialogs().item(row.noun_faction, faction_entry);
    }
    const int faction = popup.exec(0, nullptr);
    PopupModalState = 0;
    if (faction < 0) {
        return;
    }

    const unsigned char faction_bit = static_cast<unsigned char>(1u << faction);
    // The original loads both bytes, then stores human before alive; retain
    // the rest of the shared lock-mask DWORD unchanged.
    GameDataLockMaskBytes &status = GameDataStateStorage.fields_.lock_masks_.bytes_;
    const uint8_t human_mask = status.human_mask_;
    const uint8_t alive_mask = status.alive_mask_;
    status.human_mask_ = human_mask & static_cast<uint8_t>(~faction_bit);
    status.alive_mask_ = alive_mask & static_cast<uint8_t>(~faction_bit);
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
    popup.start(PopupStartCaption, "EDITORTERRAIN", -1, nullptr, 0, nullptr);
    for (int index = 0; index < MaxTerrainNum; ++index) {
        if (Terraforming[index].bit == 0) {
            continue;
        }
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Terraforming[index].name));
        if (strcmp(StringTable.get(Terraforming[index].name),
                   StringTable.get(Terraforming[index].name_sea)) != 0) {
            strcat(StringTemp, "/");
            strcat(StringTemp, StringTable.get(Terraforming[index].name_sea));
        }
        popup.dialogs().item(StringTemp, index);
        if (index == MapEditorPaintId - 6) {
            popup.set_value(index);
        }
    }
    const int result = popup.exec(0, nullptr);
    editor_paints(result + 6);
    PopupModalState = 0;
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
        popup.start(PopupStartCaption, "FACOBJ", -1, nullptr, 0, nullptr);
        for (int facility = 1; facility <= 64; ++facility) {
            popup.dialogs().item(
                StringTable.get(Facility[facility].name),
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
    popup.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40, nullptr);
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1u << faction)) == 0) {
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
            GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ =
                static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & 0xFEu);
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
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1u << selected)) == 0) {
        GenderDefault = Players[selected].noun_gender;
        PluralityDefault = Players[selected].is_noun_plural;
        parse_says(0, Players[selected].noun_faction, -1, -1);
        PopupModalState = 6;
        const int answer = X_pop("EDITORCONTROL", nullptr);
        GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ =
            answer != 0 ? static_cast<uint8_t>(1u << selected) : 0;
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
                -1, nullptr, 0x40, nullptr);
    for (int natural = 0; natural < MaxNaturalNum; ++natural) {
        popup.dialogs().item(
            StringTable.get(Natural[natural].name), natural);
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
                               StringTable.get(Natural[LM_FRESH].name)) == 0) {
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
        const int tile_x = wrap_map_x(x + RadiusOffsetX[probe_offset]);
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
        const int tile_x = wrap_map_x(x + RadiusOffsetX[clear_offset]);
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
                   StringTable.get(Natural[selected].name)) == 0) {
            kill_landmark(MapLandmark[landmark].x, MapLandmark[landmark].y);
        }
    }
    world_climate();
    ::draw_map(1);
}

// Historical seed finding: original 65BFBC is a borrowed-frame Popup cleanup at EBP-0x5398, not a standalone function; the isolated constant-null Popup pointer scaffold diverged at instruction 0 (7/11 bytes). Preserve as a failed standalone experiment, not candidate implementation.
// ORIGINAL: 0x0065BFBC sub_65bfbc 0x0065BFBC-0x0065BFC7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech prologue at 0x004DFADB registers handler 0x0065C0BE; its FuncInfo unwind row at 0x0067ABD8 names action 0x0065BFBC", "src:console_commands.cpp Console::editor_tech contains the branch-local automatic Popup who at the source location carrying this child identity", "src:Popup is a BasePop with its real automatic branch-scoped destructor; no standalone callable definition is supplied for the child"], "reason": "Retain 0x0065BFBC as the generated cleanup action of editor_tech's branch-local automatic Popup who; this borrowed-frame EH child is not a standalone function body, and generated-action matching remains an explicit obligation."}
// kind      game
// Historical seed finding: original 65C001 is a borrowed-frame BaseButton cleanup at EBP-0x14; the isolated constant-null BaseButton pointer scaffold diverged at instruction 0 (7/8 bytes). Preserve as a failed standalone experiment, not candidate implementation.
// ORIGINAL: 0x0065C001 sub_65c001 0x0065C001-0x0065C009 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech handler 0x0065C0BE uses FuncInfo unwind row 0x0067AC08: state 14, predecessor 12, action 0x0065C001", "pinned:0x004DFEB8 saves &who.scroll_.flat_button_right_ (EBP-0xB70) in [ebp-0x14]; action 0x0065C001 loads that saved pointer and transfers to BaseButton destructor 0x00607040", "pinned:Popup who receiver is EBP-0x5398; Popup constructs Scroll at +0x3230, whose constructor constructs flat_button_right_ at +0x15F8; the combined receiver is EBP-0xB70", "source:Popup::scroll_ owns Scroll::flat_button_right_; FlatButton directly derives from BaseButton and ordinary destruction owns its base teardown", "pinned:the later 0x004DFEF1 save of EBP-0x16BC instead belongs to state 15/action 0x0065C009 and the left Scroll button"], "reason": "Retain 0x0065C001 as the generated BaseButton cleanup for branch-local Popup who's typed member path scroll_.flat_button_right_; this borrowed-frame EH action is not a standalone function body, and exact generated-action/state correspondence remains an obligation."}
// kind      game
// Historical seed group (inactive): the 23 cleanup identities below were formerly EXCLUDED S2a because C++ EH unwind funclets borrow the enclosing editor_tech frame (docs/EXCLUSIONS.md section 2a). That was a generated-code categorization, not absent runtime lifetime behavior. The FILE bindings below are nonclaims; exact current generated-action and unwind-state correspondence remains pending.
// ORIGINAL: 0x0065BFB1 sub_65bfb1 0x0065BFB1-0x0065BFBC FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech constructs Popup popup at EBP-0xA714 in the 0x004DFAFB call", "pinned:Popup ctor 0x004048A0 constructs BasePop then Scroll at this+0x3230 at 0x004048C4", "source:src/console_commands.cpp Console::editor_tech owns the real automatic Popup popup; src/popup.h declares its real destructor and scroll_ member"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: Console::editor_tech automatic Popup popup; no standalone function or generated-symbol match is claimed."}
// kind      game
// ORIGINAL: 0x0065BFC7 sub_65bfc7 0x0065BFC7-0x0065BFD2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:editor_tech constructs Popup who at EBP-0x5398 in the 0x004DFDCB call", "pinned:Popup ctor 0x004048BF calls BasePop ctor on unchanged Popup this", "source:src/console_commands.cpp declares Popup who in the input-poll branch; src/popup.h Popup derives from BasePop"], "reason": "Bind this generated cleanup identity to the real nested C++ lifetime: branch-local Popup who BasePop base and owned-member teardown; no standalone function or generated-symbol match is claimed."}
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
// RECOVERY_BINDING: v1 {"evidence": ["pinned:FuncInfo unwind state 15 has predecessor 11 and names action 0x0065C009; the action loads the saved pointer from [ebp-0x14] and transfers to BaseButton destructor 0x00607040", "pinned:0x004DFEF1 saves EBP-0x16BC in [ebp-0x14]; state 15 protects FlatButton::close at 0x004DFF04 before normal-path BaseButton destruction at 0x004DFF13", "pinned:Popup who receiver EBP-0x5398 plus Popup::scroll_ 0x3230 plus Scroll::flat_button_left_ 0xAAC equals EBP-0x16BC", "source:Popup owns Scroll, Scroll owns both FlatButtons, and FlatButton directly derives from BaseButton; the right sibling uses the earlier state 14/action 0x0065C001"], "reason": "Bind 0x0065C009 to the generated BaseButton cleanup for branch-local Popup who.scroll_.flat_button_left_; preserve the borrowed-frame FILE identity without claiming a standalone function or exact current unwind-state correspondence."}
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
    if (faction < 0) {
        faction = ConsoleGlobal.faction_id_;
        ScenarioTechFaction = faction;
    }

    for (;;) {
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
            const int status = has_tech(tech_id, ScenarioTechFaction) ? 246 :
                tech_avail(tech_id, ScenarioTechFaction) ? 247 : 248;
            strcat(StringTemp, label_get(status));
            popup.dialogs().item(StringTemp, tech_id);
        }
        popup.sort_item_ids(0);
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
            const int who_faction = ScenarioTechFaction;
            int chosen;
            {
                Popup who;
                who.start(PopupStartCaption, "EDITORWHO", -1, nullptr, 0x40);
                for (int player = 1; player < MaxPlayerNum; ++player) {
                    GenderDefault = Players[player].noun_gender;
                    PluralityDefault = Players[player].is_noun_plural;
                    who.dialogs().item(Players[player].noun_faction, player);
                }
                if (who_faction >= 0) who.set_value(who_faction);
                PopupModalState = 6;
                chosen = who.exec(0, nullptr);
                PopupModalState = 0;
            }
            if (chosen > 0) {
                ScenarioTechFaction = chosen;
                faction = chosen;
            } else {
                faction = ScenarioTechFaction;
            }
            continue;
        }

        if (has_tech(selected, ScenarioTechFaction)) {
            int ignored_a;
            int ignored_b;
            bitmask(selected, &ignored_a, &ignored_b);
            faction = ScenarioTechFaction;
            GameDataStateStorage.fields_.game_tech_achieved_[selected] = static_cast<uint8_t>(
                GameDataStateStorage.fields_.game_tech_achieved_[selected] & ~(1u << faction));
            // The pin carries this post-bitmask faction value into ranking setup.
        } else {
            tech_achieved(ScenarioTechFaction, selected, 0, 0);
            faction = ScenarioTechFaction;
        }
        ScenarioTechSelection = selected;
        PlayersData[faction].tech_ranking = 0;
        int lookup_faction = faction;
        for (int owned_tech = 0; owned_tech < MaxTechnologyNum; ++owned_tech) {
            const int is_owned = has_tech(owned_tech, lookup_faction);
            const int ranking_faction = ScenarioTechFaction;
            if (is_owned) PlayersData[ranking_faction].tech_ranking += 2;
            lookup_faction = ranking_faction;
        }
        TutWinBaseWindow.redraw();
        faction = ScenarioTechFaction;
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
                popup.dialogs().item(StringTable.get(Morale[morale].name), morale);
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
            ::draw_tile(x, y, 1);
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
            GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ |=
                static_cast<uint8_t>(1u << ScenarioVehicleFaction);
        }
        return;
    }
}

/*
Purpose: Order one air unit on a bombing run against a tile: refuse unless the
         tile's base owner is at vendetta with us, lock the unit over the net,
         restate its order and resync.
// ORIGINAL: 0x004D8CD0 ?set_bombing_run@Console@@QAEXHHH@Z 0x004D8CD0-0x004D8DBE FILE BYTE_EXACT
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
        NetMsgPrimary.pop(ConsoleStrNoBomb, 5000, 0, 0);
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
// ORIGINAL: 0x004D7DE0 ?go_home@Console@@QAEXH@Z 0x004D7DE0-0x004D7EDA FILE BYTE_EXACT
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
        const int base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        if (base_id >= 0) {
            if (Bases[base_id].faction_id_current == Vehs[veh_id].faction_id) {
                const int old_base = Vehs[veh_id].home_base_id;
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
        } else {
            action_home(veh_id, 0);
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
        }
        NetDaemonNet.unlock_veh();
    }
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

    popup.start(PopupStartCaption, ConsoleStrScenParam, -1, nullptr, 0x44, nullptr);

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
// Superseded interpretation: the late native calls tear down Popup's owned
// Scroll, buttons and BasePop, not a separate confirmation dialog. Original
// Popup at EBP-53A0 owns the Dialogs at +21D0 and Scroll at +3230; use those
// members rather than constructing detached duplicate UI receivers.
// size      3307 bytes
// prototype void (__thiscall ?destroy@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   3   call targets   33
// calls     0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406820 0x00406880 0x00406910 0x00406E60 0x0047A890 0x004CAA50 0x004E3EF0 0x0050BCC0 0x0052DC70 0x00531020 0x005310F0 0x00531300 0x0055F770 0x00592E10 0x00592EE0 0x005C1B70 0x005D45B0 0x005D4DD0 0x005E3820 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00645550
*/
void Console::destroy(int veh_id) {
    // The original's UI cleanup addresses all belong to this one Popup.
    // Its genuine member/base lifetimes own normal and exceptional teardown.
    Popup popup;

    if (not_my_turn()) {
        NetMsgPrimary.pop(ConsoleStrSimulNo, 5000, 0, 0);
        return;
    }
    if (veh_ready(veh_id) == 0) {
        NetMsgPrimary.pop(ConsoleStrUnitMoved, 5000, 0, 0);
        return;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, -1) != 0) {
        return;
    }
    if (not_my_turn()) {
        NetMsgPrimary.pop(ConsoleStrSimulNo2, 5000, 0, 0);
        NetDaemonNet.unlock_veh();
        return;
    }
    if (veh_ready(veh_id) == 0) {
        NetMsgPrimary.pop(ConsoleStrUnitMoved2, 5000, 0, 0);
        NetDaemonNet.unlock_veh();
        return;
    }

    const int vy = Vehs[veh_id].y;
    const int vx = Vehs[veh_id].x;
    const uint8_t faction = Vehs[veh_id].faction_id;
    Map *const tile = map_loc(vx, vy);
    const bool is_ocean = (tile->climate & 0xE0) < 0x60;
    const int proto_id = Vehs[veh_id].proto_id;
    const int weapon_id = VehPrototypes[proto_id].weapon_id;

    if ((Weapon[weapon_id].offense_rating == 0) &&
        (Weapon[weapon_id].mode != WPN_MODE_TERRAFORMER)) {
        NetMsgPrimary.pop(ConsoleStrOnlyCombat, 5000, 0, 0);
        NetDaemonNet.unlock_veh();
        return;
    }
    if (((tile->bit & 1) != 0) && ((tile->val2 & 0xF) < 8) &&
        ((tile->val2 & 0xF) >= 0)) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop(ConsoleStrNotBase, 5000, 0, 0);
        return;
    }

    const int triad = Chassis[VehPrototypes[proto_id].chassis_id].triad;
    if ((triad != 2) && (static_cast<int>(is_ocean) != (triad == 1))) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop(ConsoleStrNotInTriad, 5000, 0, 0);
        return;
    }

    const uint32_t tile_flags = tile->bit;
    const char *title = (triad == 2) ? ConsoleStrDestroyBomb : ConsoleStrDestroy;
    // Native4D57E9 passes -1, not the tile flags used by the option filter.
    popup.start(PopupStartCaption, title, -1, nullptr, 0x40, nullptr);

    Dialogs &dialogs = popup.dialogs();
    int item_count = 0;
    for (int a = 0; a < 0x13; a++) {
        // Preserve the original exclusion branches before reading this row.
        if ((a == 5 && (tile_flags & 8) != 0) ||
            (a == 0 && (tile_flags & 0x80000) != 0) || a == 11) {
            continue;
        }
        const uint32_t abil_flag = Terraforming[a].bit;
        if (abil_flag == 0 || (tile_flags & abil_flag) == 0 || a == 10) {
            continue;
        }
        // Native4D5839 uses its saved ocean boolean, not the vehicle's x/2.
        dialogs.item(StringTable.get(is_ocean ? Terraforming[a].name_sea
                                             : Terraforming[a].name), abil_flag);
        item_count++;
    }

    if (item_count == 0) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop(ConsoleStrNotHere, 5000, 0, 0);
        return;
    }

    // A single admitted item takes the original zero-result fast path.
    int32_t result = 0;
    if (item_count >= 2) {
        result = popup.exec(0, 0);
    }
    if (result == -1) {
        NetDaemonNet.unlock_veh();
        return;
    }

    const int territory = whose_territory(faction, vx, vy, nullptr, 0);
    if ((territory >= 0) && (territory != faction)) {
        if (break_treaty(faction, territory, 7) != 0) {
            NetDaemonNet.unlock_veh();
            return;
        }
        if ((PlayersData[faction].diplo_treaties[territory] & DTREATY_VENDETTA) == 0) {
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
        const int saved_faction = ConsoleGlobal.faction_id_;
        {
            Popup popup2;
            PopupModalState = 6;
            popup2.start(PopupStartCaption, ConsoleStrEditorWhoListTitle,
                         -1, nullptr, 0x40, nullptr);
            // The who list walks factions 1..7 by noun record: each item's
            // text is the faction's noun, with its gender/plurality feeding
            // the parser defaults.
            for (int idx = 1; idx < MaxPlayerNum; ++idx) {
                Player &row = Players[idx];
                const int plurality = row.is_noun_plural;
                const int gender = row.noun_gender;
                PluralityDefault = plurality;
                GenderDefault = gender;
                popup2.dialogs().item(row.noun_faction, idx);
            }
            if (saved_faction >= 0) {
                popup2.set_value(saved_faction);
            }
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
    LPSTR *const label_keys = Labels.strings_ptr;
    const LPSTR key = (player.is_leader_female == 0)
        ? label_keys[0x2E2] : label_keys[0x2E3];
    strcpy(name_buf, StringTable.get(key));

    popup.start(PopupStartCaption, ConsoleStrCustomNames, -1, nullptr, 0x44, nullptr);

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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D6070 ?goto_routine@Console@@QAAXH@Z 0x004D6070-0x004D6196 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d6070.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
// symbol    ?goto_routine@Console@@SAXH@Z
void __cdecl Console::goto_routine(int base_id) {
    PlanWindow.clear_lines();
    if (base_id >= 0 && base_id < GameDataStateStorage.fields_.base_current_count_) {
        if (GoToPreviewActive) {
            PlanWindow.opticenter(GoToSourceX, GoToSourceY,
                             Bases[base_id].x, Bases[base_id].y, 0);
            if (base_id < GameDataStateStorage.fields_.base_current_count_) {
                PlanWindow.add_line(GoToSourceX, GoToSourceY,
                                Bases[base_id].x, Bases[base_id].y,
                                InfoFactionTextColors[ConsoleGlobal.faction_id_]);
                PlanWindow.on_redraw();
                return;
            }
        } else {
            PlanWindow.monocenter(Bases[base_id].x, Bases[base_id].y, 0);
            PlanWindow.add_line(Bases[base_id].x, Bases[base_id].y,
                        Bases[base_id].x, Bases[base_id].y,
                        InfoFactionTextColors[ConsoleGlobal.faction_id_]);
            PlanWindow.on_redraw();
            return;
        }
    } else {
        PlanWindow.back_redraw();
    }
    PlanWindow.on_redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D9170 ?arty@Console@@QAEXH@Z 0x004D9170-0x004D92CD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d9170.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
void Console::arty(int vehicle) {
    if (!can_arty(Vehs[vehicle].proto_id, 1)) return;
    if (not_my_turn()) {
        NetMsgPrimary.pop(ConsoleArtySimulNoText, 5000, 0, nullptr);
        return;
    }
    if (!veh_ready(vehicle)) {
        NetMsgPrimary.pop(ConsoleArtyUnitMovedText, 5000, 0, nullptr);
        return;
    }
    Map *const tile = map_loc(Vehs[vehicle].x, Vehs[vehicle].y);
    if ((tile->climate & ~0x1F) < 0x60 &&
        Chassis[VehPrototypes[Vehs[vehicle].proto_id].chassis_id].triad != 1 &&
        base_who(Vehs[vehicle].x, Vehs[vehicle].y) < 0) {
        NetMsgPrimary.pop(ConsoleArtySeaText, 5000, 0, nullptr);
        return;
    }
    ConsoleGlobal.cursor_on(10, static_cast<int>(Rules.artillery_max_rng));
    int pixel_x, pixel_y;
    MapWinTable[0]->get_mouse_pos(&pixel_x, &pixel_y);
    MapWinTable[0]->on_mouse_move(pixel_x, pixel_y);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004D9970 ?editor_on@Console@@QAEXXZ 0x004D9970-0x004D9AC5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004d9970.cpp: original identity and complete spans", "source:src/console.h: Console owner declarations and layout", "source:src/console.cpp: coupled navigation, selection and lifecycle caller relationships"], "reason": "The original Console operation is expressed by the corresponding real Console owner definition, using its recovered members and source-owned callees rather than the retained seed scaffold."}
// RECOVERY_LITERAL: v1 0x00688F3C 0x00688F46 ??_C@_09HDAC@EDITORNET?$AA@ 0x004D999D
// RECOVERY_LITERAL: v1 0x00688F48 0x00688F53 ??_C@_0L@KPKF@SCENEDITOR?$AA@ 0x004D99B4
// RECOVERY_LITERAL: v1 0x00688F54 0x00688F5D ??_C@_08FDNL@EDITORON?$AA@ 0x004D99F9
// RECOVERY_LITERAL: v1 0x00688F60 0x00688F6A ??_C@_09OGLF@EDITOROFF?$AA@ 0x004D9A11
void Console::editor_on() {
    if ((IsMultiplayerNet || IsMultiplayerPBEM) &&
        !(GameDataStateStorage.fields_.game_state_ & 0x61) &&
        !(GameDataStateStorage.fields_.game_state_ & 0x1000)) {
        NetMsgPrimary.pop("EDITORNET", 5000, 0, nullptr);
        return;
    }
    if (!(GameDataStateStorage.fields_.game_state_ & 0x60) && !X_pop("SCENEDITOR", nullptr)) return;
    GameDataStateStorage.fields_.game_state_ = (GameDataStateStorage.fields_.game_state_ ^ 0x40) | 0x20;
    if (GameDataStateStorage.fields_.game_rules_ & 0x400) auto_save();
    if (GameDataStateStorage.fields_.game_state_ & 0x40) {
        NetMsgPrimary.pop("EDITORON", 5000, 0, nullptr);
        GameDataStateStorage.fields_.game_state_ |= 0x80;
    } else {
        NetMsgPrimary.pop("EDITOROFF", 5000, 0, nullptr);
        GameDataStateStorage.fields_.game_state_ &= ~0x80u;
    }
    if (((GameDataStateStorage.fields_.game_state_ & 0x80) != 0 &&
         (GameDataStateStorage.fields_.game_state_ & 0x40) != 0) && !IfaceIdToSlotStorage[ActiveIfaceIdStorage])
        MainInterfaceGlobal.flat_button(2).show(0);
    else MainInterfaceGlobal.flat_button(2).hide();
    alpha_menu_.check(selected_vehicle_);
    if ((GameDataStateStorage.fields_.game_state_ & 0x40) != 0 &&
        (GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
        MainInterfaceGlobal.button_group(0).set(1002, 1);
    } else {
        MainInterfaceGlobal.button_group(0).set(1001, 1);
    }
    set_dirty();
    ::draw_map(1);
    WorldClimateWorldWin.GraphicWin::redraw();
    desktop_update();
    update_data(0);
}

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
            const int destination = i == 0 ? base.auto_forward_slots[Base::AutoForwardLand]
                : i == 1 ? base.auto_forward_slots[Base::AutoForwardSea]
                         : base.auto_forward_slots[Base::AutoForwardAir];
            strcpy(StringTemp, StringTable.get(Triad[i]));
            strcat(StringTemp, " (");
            say_base(StringTemp, destination);
            strcat(StringTemp, ")");
            popup.dialogs().item(StringTemp, i);
        }
        forward_triad = popup.exec(0, nullptr);
        if (forward_triad < 0) goto finish;
        triad = forward_triad;
        parse_say(0, Triad[triad], -1, -1);
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
    GoToSourceX = PlanWindow.center_x_ = source_x;
    GoToSourceY = PlanWindow.center_y_ = source_y;
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
        if (sorting) {
            const int bounds = GameDataStateStorage.fields_.base_current_count_ - 1;
            int changed;
            do {
                changed = 0;
                for (int j = 0; j < bounds; ++j) {
                    if (scores[j] < scores[j + 1]) {
                        changed = 1;
                        swap(&scores[j], &scores[j + 1]);
                        swap(&ids[j], &ids[j + 1]);
                    }
                }
            } while (changed != 0);
        } else {
            int changed;
            do {
                changed = 0;
                for (int j = 0; j < GameDataStateStorage.fields_.base_current_count_ - 1; ++j) {
                    if (_stricmp(Bases[ids[j]].name_string,
                                 Bases[ids[j + 1]].name_string) > 0) {
                        changed = 1;
                        swap(&ids[j], &ids[j + 1]);
                    }
                }
            } while (changed != 0);
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
                        ? StringTable.get(Facility[90].name) : label_get(0x4c4 / 4));
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
            if (order == -1) preferred = forward_triad == 0
                ? Bases[subject].auto_forward_slots[Base::AutoForwardLand]
                : forward_triad == 1 ? Bases[subject].auto_forward_slots[Base::AutoForwardSea]
                                     : Bases[subject].auto_forward_slots[Base::AutoForwardAir];
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
            if (!forward_triad)
                Bases[subject].auto_forward_slots[Base::AutoForwardLand] = destination;
            else if (forward_triad == 1)
                Bases[subject].auto_forward_slots[Base::AutoForwardSea] = destination;
            else Bases[subject].auto_forward_slots[Base::AutoForwardAir] = destination;
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
        if (!window || (slot && !window->view_active_)) continue;
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
        if (NetDaemonNet.channels_[1].leading_words_[2] ||
            NetDaemonNet.channels_[1].payload_.flags) return;
        const unsigned int faction_bit = 1u << ConsoleGlobal.faction_id_;
        if (((NetDaemonNet.primary_state_.trailing_words_[6] & faction_bit) &&
             !(GameDataStateStorage.fields_.net_turn_flags_ & 0x10)) || ConsoleControlTurnActive) {
            int vehicle = veh_at(x, y);
            if (stack_check(vehicle, 1, -1, -1, -1) != 1)
                vehicle = veh_pick(vehicle, "PICKUNIT", 1);
            if (vehicle >= 0) {
                selected_vehicle_ = vehicle;
                focus_vehicle_ = vehicle;
            }
            const bool ready_now = (NetDaemonNet.primary_state_.trailing_words_[6] &
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
        ::draw_tile(Vehs[vehicle].x, Vehs[vehicle].y, 1);
        if (!ConsoleGlobal.cursor_active_ && !(GameDataStateStorage.fields_.game_state_ & 2) &&
            (!IsMultiplayerNet || (!ConsoleControlTurnActive &&
             !NetDaemonNet.channels_[1].payload_.flags))) {
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
                       StringTable.get(Ability[ability_id].abbreviation));
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
                               StringTable.get(Ability[ability_id].abbreviation));
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
                       StringTable.get(Ability[ability_id].abbreviation));
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
                               StringTable.get(Ability[ability_id].abbreviation));
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
    ::draw_tile(Vehs[veh_id].x, Vehs[veh_id].y, 1);
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
            popup.dialogs().item(StringTable.get(Ability[ability_id].name),
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
        if (popp(ScriptPopupFile, prompt, 0x40, "bureau_sm.pcx", nullptr) < 0) {
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
            ::draw_map(1);
        }
        const int answer =
            X_pop_ask("NEWBASE", 0x17, base_name, nullptr, 0);
        MapFocusY = -99;
        MapFocusX = -99;
        if (answer != 0) {
            if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
                return;
            }
            ::draw_map(1);
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
        ::draw_map(1);
        return;
    }

    int locked_vehicle_id = vehicle_id;
    if (NetDaemonNet.lock_veh(&locked_vehicle_id, 4, -1, -1, 0) != 0) {
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        ::draw_map(1);
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
        ::draw_map(1);
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
        ::draw_map(1);
        return;
    }
    const Map *const locked_tile = map_loc(x, y);
    if ((locked_tile->bit & BIT_FUNGUS) != 0 && locked_altitude >= 0x40) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("FUNGUSDANGUS", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        ::draw_map(1);
        return;
    }
    if ((locked_tile->bit & BIT_MONOLITH) != 0) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("NOTONMONOLITH", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        ::draw_map(1);
        return;
    }
    if (!veh_ready(locked_vehicle_id)) {
        NetDaemonNet.unlock_veh();
        NetMsgPrimary.pop("UNITMOVED", 5000, 0, nullptr);
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) == 0) {
            return;
        }
        ::draw_map(1);
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
        ::draw_map(1);
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
        popp(ScriptPopupFile, "BASEFOUNDED", 0, picture, nullptr);
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
            (is_sea ? Terraforming[TERRA_MAGTUBE].prerequisite_tech[1]
                    : Terraforming[TERRA_MAGTUBE].prerequisite_tech[0]) >= -1) {
            terraform_id = TERRA_MAGTUBE;
        }
    } else if (terraform_id == TERRA_FARM) {
        if ((tile->bit & BIT_FUNGUS) != 0 && altitude >= 0x40) {
            terraform_id = TERRA_REMOVE_FUNGUS;
        } else if ((tile->bit & BIT_FARM) != 0 && altitude >= 0x60 &&
                   (is_sea ? Terraforming[TERRA_SOIL_ENR].prerequisite_tech[1]
                           : Terraforming[TERRA_SOIL_ENR].prerequisite_tech[0]) >= -1) {
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
        ? Terraforming[terraform_id].prerequisite_tech[1]
        : Terraforming[terraform_id].prerequisite_tech[0];
    if (selected_prerequisite < -1) {
        const int other_prerequisite = is_sea
            ? Terraforming[terraform_id].prerequisite_tech[0]
            : Terraforming[terraform_id].prerequisite_tech[1];
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

    if (!is_sea && (map_loc(x, y)->val3 & 0xC0) > 0x40 &&
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
                ? Terraforming[terraform_id].prerequisite_tech[1]
                : Terraforming[terraform_id].prerequisite_tech[0];
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
        if ((map_loc(x, y)->val3 & 0xC0) == 0) {
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

    if (NetDaemonNet.channels_[1].leading_words_[3] != 0) {
        log_say("I think I've already got my lock", 0, 0, 0);
    }
    if (NetDaemonNet.channels_[1].leading_words_[3] == 0) {
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
