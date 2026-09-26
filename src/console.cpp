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

// Console constructor-installed Win branch: capture4, resize13, mouse17, update63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x0066EC18 ? ??_7Console@@6BWin@@@ 0x0050F460
#include "original_seam.h"
#include "console.h"
#include "dipedit.h"  // editor_diplomacy's stack DipEdit
#include "game.h"
#include "general.h"
#include "popup.h"  // editor_diplomacy's modal faction picker
#include "infowin.h"
#include "map.h"
#include "mapwin.h"
#include "maininterface.h"
#include "statuswin.h"
#include "temp.h"
#include "fx.h"
#include "hypothesis_layouts.h"
#include "graphicwin.h"
#include "spritebox.h"
#include "net_class.h"
#include "menu.h"
#include "sprite.h"
#include "time.h"
#include "texture.h"
#include "vector_teardown.h"
#include "veh.h"  // Vehs - the group table is the Vehs[i].state run
#include "faction.h"  // Players/PlayersData - the per-faction records editor_energy and set_ai walk
#include "xpops.h"  // X_pop - editor_on and friends raise the message pops
#include "lock.h"  // LockMapTable - editor_clear_vehicles rewrites its records
#include "log.h"  // log_say - the chat / message handlers report through it
#include "netdaemon.h"  // synch_veh - the turn handlers resync vehicles

#include "netmsg.h"  // NetMsgPopup - the editor's ALREADYDONE/PROTECT* pops
#include "tutwin.h"  // TutWinGlobal - survey2 raises the map tutorial on it
#include "spying_recovery.h"  // spying - the espionage order
#include "technology.h"  // has_tech - editor_tech and the tech orders gate on it
#include "random.h"  // random - the editor's randomizer and the AI orders
#include "alpha.h"  // AlphaIniPrefs - the toggles mirror the prefs into it
#include "worldwin.h"  // WorldClimateWorldWin - editor_flat / editor_clear redraw it
#include "terraforming.h"  // Terraforming - destroy's option list reads its bit/label rows
#include "buttongroup.h"  // ButtonGroup::set - the editor brush/paint selectors
#include <cstring>

// Link-only placeholders; movement selection and vehicle editing are unfinished.
// ORIGINAL: 0x00511520 ?set_move@Console@@QAEXH@Z 0x00511520-0x005117FD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00511520.cpp", "source:src/console.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU. Empty link-only placeholder; original behavior is not implemented."}
void Console::set_move(int) {
    // BODY GOES HERE.
}

// ORIGINAL: 0x004DDA50 ?editor_edit_veh@Console@@QAEXH@Z 0x004DDA50-0x004DE95B;0x0065B97E-0x0065BC13
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004dda50.cpp", "source:src/console.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU. Empty link-only placeholder; original behavior is not implemented."}
void Console::editor_edit_veh(int) {
    // BODY GOES HERE.
}

// Shared by the turn loop and network lock guards. The original writers
// are control_turn, net_end_of_turn and net_control_turn (see console.h).
// RECOVERY_OBJECT: v2 0x0093A938 0x0093A93C ?ConsoleControlTurnActive@@3HA 0x00530D50
int ConsoleControlTurnActive;

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
void __cdecl action_destroy(int veh_id, int choice, int a3, int a4);  // 0x004CAA50 ?action_destroy@@YAXHHHH@Z
void __cdecl draw_cursor();                               // 0x0046AE00 ?draw_cursor@@YAXXZ
int __cdecl tut_check(int veh_id);                            // 0x004BA5F0 ?tut_check@@YAHH@Z
int __cdecl tut_check2(int veh_id);                           // 0x004BA650 ?tut_check2@@YAHH@Z
int __cdecl save_game(int save_slot);                            // 0x005A9EB0 ?save_game@@YAHH@Z
void __cdecl base_kill(int base_id);                      // 0x004E5250 ?base_kill@@YAXH@Z
int __cdecl end_of_game(int victory_kind);                          // 0x00524870 ?end_of_game@@YAHH@Z
int __cdecl load_game(int save_slot, int faction_id);                    // 0x005AAAB0 ?load_game@@YAHHH@Z
int __cdecl load_map();                                   // 0x005AB600 ?load_map@@YAHXZ
void __cdecl save_map();                                  // 0x005ABB90 ?save_map@@YAXXZ
// a2 is 0 at all three call sites and a3 is -1 at two of them; nothing in
// the tree names either, so the forward declaration leaves them unnamed.
int __cdecl tech_pick(int, int, int, LPCSTR);            // 0x005BEB70 ?tech_pick@@YAHHHHPAD@Z
void __cdecl set_dirty();                                 // 0x00592A80 ?set_dirty@@YAXXZ
int __cdecl size_of_planet(int planet_size);                       // 0x0058CED0 ?size_of_planet@@YAHH@Z
int __cdecl supply_options(int base_id, int flags);               // 0x00593A00 ?supply_options@@YAHHH@Z
void __cdecl time_controls_dialog(Popup *popup);          // 0x00589330 ?time_controls_dialog@@YAXPAUPopup@@@Z
void __cdecl veh_kill(int veh_id);                        // 0x005C08C0 ?veh_kill@@YAXH@Z
int __cdecl veh_pick(int faction_id, const char *label, int flags);  // 0x0048D0D0 ?veh_pick@@YAHHPBDH@Z
void __cdecl world_build();                               // 0x005C86E0 ?world_build@@YAXXZ
void __cdecl world_fungus();                              // 0x005C3440 ?world_fungus@@YAXXZ
int __cdecl veh_selectable(int veh_id);                       // 0x005C1A20 ?veh_selectable@@YAHH@Z
int __cdecl eliminate_player(int faction_id, int is_eliminated);             // 0x005B3380 ?eliminate_player@@YAHHH@Z
int __cdecl veh_unmoved(int veh_id);                          // 0x005C1AB0 ?veh_unmoved@@YAHH@Z
char *__cdecl get_pact_hood(int faction_id, int faction_id_with);              // 0x0058FB00 ?get_pact_hood@@YAPADHH@Z
void __cdecl diplomacy_ends(int faction_id, int faction_id_with);              // 0x0053E470 ?diplomacy_ends@@YAXHH@Z
int __cdecl diplo_lock(int faction_id);                           // 0x00539820 ?diplo_lock@@YAHH@Z
int __cdecl break_treaty(int faction_id, int faction_id_with, int treaty);         // 0x0055F770 ?break_treaty@@YAHHHH@Z
void __cdecl double_cross(int faction_id, int faction_id_with, int treaty);        // 0x0050BCC0 ?double_cross@@YAXHHH@Z
void __cdecl spot_all(int x, int y);                    // 0x005B6260 ?spot_all@@YAXHH@Z
int __cdecl upgrade_cost(int facility_id, int base_id, int flags);         // 0x004EFD50 ?upgrade_cost@@YAHHHH@Z
int __cdecl resource_yield(int x, int y, int resource_kind, int faction_id, int a5); // 0x004E7DC0 ?resource_yield@@YAHHHHHH@Z
int __cdecl can_convoy(int faction_id, int veh_id);                   // 0x00564D90 ?can_convoy@@YAHHH@Z
void __cdecl hall_of_fame(int is_retire);                        // 0x0044AE50 ?hall_of_fame@@YAXH@Z
void __cdecl show_credits();                              // 0x00428DF0 ?show_credits@@YAXXZ
void __cdecl show_replay();                               // 0x005ADBF0 ?show_replay@@YAXXZ
int __cdecl tech_advance(int tech_id);                         // 0x005BE530 ?tech_advance@@YAHH@Z
void __cdecl tech_achieved(int faction_id, int tech_id, int is_stolen, int a4); // 0x005BB000 ?tech_achieved@@YAXHHHH@Z
void __cdecl social_select(int faction_id);                       // 0x005B5620 ?social_select@@YAXH@Z
void __cdecl design_new_veh(int faction_id, int proto_id);              // 0x00583CB0 ?design_new_veh@@YAXHH@Z
void __cdecl draw_tile_fixup(int x, int y, int a3, int a4); // 0x0046AF90 ?draw_tile_fixup@@YAXHHHH@Z
int __cdecl enemy_veh(int veh_id);                            // 0x00579240 ?enemy_veh@@YAHH@Z
void __cdecl message_veh(int veh_id, int a2, int a3, int a4); // 0x00592E10 ?message_veh@@YAXHHHH@Z
void __cdecl message_base(int base_id, int a2, char *text, int a4); // 0x00592E70 ?message_base@@YAXHHPADH@Z
void __cdecl message_chat(int a1, int a2, char *text);    // 0x00593190 ?message_chat@@YAXHHPAD@Z
void __cdecl message_landmark(int landmark_id, int a2, char *text, int a4, int a5, int a6, int a7); // 0x00593110
void __cdecl monument(int base_id);                            // 0x00476A50 ?monument@@YAXH@Z
void __cdecl quayle(int base_id);                              // 0x00496670 ?quayle@@YAXH@Z
void __cdecl report_score(int faction_id);                        // 0x004ACB80 ?report_score@@YAXH@Z
char *__cdecl popt(char *caption, const char *label, int a3, char *a4, int a5); // 0x0048C320 ?popt@@YAHPADPBDHPADH@Z
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
    const int32_t zero = 0;
    std::memcpy(reinterpret_cast<uint8_t *>(this) + 0x23D1C, &zero, sizeof(zero));
    const int32_t count = *ConsoleGroupCount;
    for (int32_t index = 0; index < count; ++index) {
        // 0x0095282C is Vehs + 4: the group table is the state dword of every
        // Veh record (see console.h), so the walk is an offset of Vehs, not a
        // separate object.
        uint8_t *const entry =
            reinterpret_cast<uint8_t *>(&Vehs[index].state);
        uint32_t value;
        std::memcpy(&value, entry, sizeof(value));
        value &= 0xF7FFFFFFu;
        std::memcpy(entry, &value, sizeof(value));
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
    if (!(GameState & STATE_SCENARIO_EDITOR)) {
        return 0;
    }
    if (((ConsoleEditKeyStateSlot())(VK_SCROLL) & 1) ||
        (GameState & STATE_EDITOR_ONLY_MODE)) {
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
        if ((GameState & STATE_DEBUG_MODE) != 0) {
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
            const uint32_t active = *reinterpret_cast<const volatile uint32_t *>(
                reinterpret_cast<const uint8_t *>(window) + MapWinActiveOffset);
            if (active == 0) {
                continue;
            }
        }
        // 0x00510902 reads the window's wide flag dword, one dword ahead of the
        // activity field mapwin.h already names. Kept as a documented literal
        // rather than a constant in another class's header: MapWin owns the
        // field, and console.h is not the place to name it. 0x00510908
        // `test ecx, edx` is a bitwise AND against the tag bits, not an
        // equality, so a window carrying 0x60000000 matches either mask.
        const uint32_t flags = *reinterpret_cast<const volatile uint32_t *>(
            reinterpret_cast<const uint8_t *>(window) + 0x1DD70);
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
            reinterpret_cast<Dialogs *>(reinterpret_cast<uint8_t *>(&popup)
                                            + 0x21d0)
                ->Dialogs::item(Players[i].noun_faction, i);
        }

        // BasePop's default answer: when the player accepts without picking,
        // the popup hands back the faction already being played.
        if (faction >= 0) {
            *reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(&popup)
                                              + 0xa44) =
                static_cast<uint32_t>(faction);
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
Time g_CONSOLE_TIMER;

// ===== The console domain's fixed windows, REAL OBJECTS at their image
// addresses (0x008578D8, 0x007AD2A0, 0x009156B0 - see console.h). The image's
// ??__E dynamic initializers construct the InfoWin and the Console's
// neighbours at their fixed addresses before WinMain (measured: `mov ecx,
// 0x7ad2a0; call InfoWin::InfoWin` at 0x004562C0 with an atexit thunk);
// ConsoleGlobal also has a real constructor; its static zero-initialization
// supplies the scalar/history storage that constructor leaves untouched.
PrefWin ConsolePrefWin;   // 0x008578D8
// RECOVERY_OBJECT: v1 0x007AD2A0 ?ConsoleInfoWin@@3VInfoWin@@A
InfoWin ConsoleInfoWin;
// 0x009156B0 - the bare declaration is what the image has: its ??__E
// dynamic initialiser pushes the hidden most-derived flag (push 1 at
// 0x0050E852) and the compiler emits that push for a most-derived global
// on its own. The old explicit `ConsoleGlobal(1)` was the flag spelled as
// a real int - the exact doubling Console(int) carried.
// Absolute cursor loads at 0x00510A01..0x00510A19 address this object's
// history members. The following stores instead use the method receiver.
// RECOVERY_OBJECT: v1 0x009156B0 ?ConsoleGlobal@@3VConsole@@A
Console ConsoleGlobal;

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
// size      796 bytes
// prototype void (__thiscall ?disband@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   5   call targets   14
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A7A0 0x0047A890 0x0048C0A0 0x004CD2F0 0x004E3A50 0x004E4430 0x00531020 0x005310F0 0x00531300 0x00592E10 0x00593250 0x005C0B00 0x00625EC0 0x00645470
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004D2AC0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d2ac0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?disband@Console@@QAEXH@Z  at 0x004D2AC0  (796 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void Console::disband(int veh_id) {
    return;
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

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

// .rdata labels the order handlers raise through NetMsgPopup, read out of the
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

// THE TIME-CONTROLS STATE the CUSTOMTIME dialog drives: 0x009A64E0 is the
// preselected row (start's value slot copies it into the popup) and
// 0x0094F25C..0x0094F274 are alpha.h's TimeControl[5].turn/.base/.unit/
// .event/.extra/.refresh/.accumulated - the six parse_says'd fields plus the
// seventh stored on accept.
static int *const TimeControlsSelection = (int *)0x009A64E0;

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

// THE SCENARIO-EDITOR BRUSH STATE, unbound anywhere in the tree: 0x00945AE4
// is the current brush id (the BRUSH picker seeds its list from it and the
// answer is stored back), 0x007CD254 is the editor palette's ButtonGroup and
// 0x007CD2D8 (group + 0x84) the id of the palette button currently checked -
// editor_set_brush re-checks the new brush's button only when it differs.
static int *const EditorBrushId = (int *)0x00945AE4;
static ButtonGroup *const EditorBrushButtonGroup = (ButtonGroup *)0x007CD254;
static int *const EditorBrushButtonGroupSelection = (int *)0x007CD2D8;

// THE GAME-IN-PROGRESS FLAG at 0x009B2074, between game.h's ControlTurnPhase
// (0x009B2070) and SaveNameBuffer (0x009B2078). load_proc reopens a running
// game through game_reload when it is set and re-initialises the desktop when
// it is clear. No tree binding owns it yet.
static int *const GameSessionActive = (int *)0x009B2074;

// THE TUTORIAL-PENDING FLAG at 0x008CC248, the dword beside tutwin.cpp's
// TutWinShownFlag (0x008CC244). The turn boundary consumes it: when set it
// clears and TutWin::tour (0x004BC6E0) launches.
static int *const TutWinTourPending = (int *)0x008CC248;

// TutWin::tour (0x004BC6E0) is still original and tutwin.h declares no seam
// for it yet, so end_of_turn reaches it through the pinned member-pointer
// form - the receiver rides in ECX, which is the whole point of
// original_seam.h's machinery.
typedef void (OriginalObject::*func_tut_win_tour)(void);
static func_tut_win_tour TutWinTour = original_method<func_tut_win_tour>(0x004BC6E0);

// break_treaty (0x0055F770) and action_destroy (0x004CAA50) are still
// original and have no pending_bodies forwarder yet, so destroy reaches them
// through local __cdecl seams - plain function pointers, since no receiver is
// involved (original_seam.h's member-pointer machinery is for __thiscall).
typedef void (__cdecl *func_action_destroy)(int veh_id, int choice, int a3, int a4);
static func_action_destroy ActionDestroy = (func_action_destroy)0x004CAA50;
typedef int (__cdecl *func_break_treaty)(int faction_id, int faction_id_with, int treaty);
static func_break_treaty BreakTreaty = (func_break_treaty)0x0055F770;

// BaseWin::on_redraw (0x0041E790) and BaseWin::exit (0x0041A4C0) are declared
// in basewin.h but not recovered - like its draw_facilities/garrison_click
// stand-ins, they are reached through the pinned member-pointer form until
// their bodies land.
typedef void (OriginalObject::*func_base_win_on_redraw)(void);
static func_base_win_on_redraw BaseWinOnRedraw =
    original_method<func_base_win_on_redraw>(0x0041E790);
typedef void (OriginalObject::*func_base_win_exit)(void);
static func_base_win_exit BaseWinExit = original_method<func_base_win_exit>(0x0041A4C0);

// ORIGINAL: 0x004DBB40 ?editor_reset_faction@Console@@QAEXXZ 0x004DBB40-0x004DBC32 FILE
// TRIED: nothing yet - MISMATCH #16 mov/push (94% mnemonic sim), a push/store reordering around the `compute_faction_modifiers` call; not chased further. `this` is unused throughout (thiscall receiver never touched).
// size      242 bytes
// prototype void (__thiscall ?editor_reset_faction@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B0D70 0x005B44D0 0x005BF310
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x004DBB40
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?editor_reset_faction@Console@@QAEXXZ  at 0x004DBB40  (242 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void Console::editor_reset_faction() {
    // THE WALKER. `faction_state` starts at PlayerData[1] + 0x324 and steps
    // one whole PlayerData (0x833 dwords == 0x20CC bytes, the stride
    // faction.h pins) per faction, so the raw offsets below are all relative
    // to that one fixed point inside the record. Spelled against the real
    // array rather than an anonymous 0x0096EDD0 base: the stride and the
    // start-at-faction-1 are what prove the identity.
    int *faction_state = reinterpret_cast<int *>(&PlayersData[1]) + 0xC9;
    if (X_pop(StrResetAllFac, (NullCallback)0) != 0) {
        unsigned int factionId = 1;
        uint32_t *rules_flag = TectonicDetonationCount + 1;
        do {
            faction_state[1] = 5;
            *faction_state = 0;
            compute_faction_modifiers(factionId);

            int *puVar2 = faction_state - 0x3f;
            int i = 4;
            do {
                puVar2[4] = 0;
                *puVar2 = 0;
                ++puVar2;
                --i;
            } while (i != 0);

            social_upkeep(factionId);
            int count = BaseCurrentCount;
            faction_state[-0x53] = 0;
            faction_state[-0x54] = 0;
            *rules_flag = 0;
            faction_state[-0x80] = 0;

            // Bases owned by this faction lose their governor flag and their
            // production queue. &Bases[0].governor is 0x0097D078; the faction
            // byte is Base::faction_id_current at 0x4, i.e. governor - 0x34.
            if (count > 0) {
                int *base_governor = reinterpret_cast<int *>(&Bases[0].governor);
                int n = count;
                do {
                    if ((unsigned char)base_governor[-0xd] == factionId) {
                        *base_governor &= 0x7fffffff;
                        base_governor[5] = 0;
                    }
                    base_governor += 0x4d;
                    --n;
                } while (n != 0);
            }
            // Vehs owned by this faction are emptied. &Vehs[0].state is
            // 0x0095282C (see clear_group's note in console.h) and the
            // 0xd-dword step is the 0x34-byte Veh stride.
            if (VehCurrentCount > 0) {
                int *veh_state = reinterpret_cast<int *>(&Vehs[0].state);
                int n = VehCurrentCount;
                do {
                    if (*((unsigned char *)veh_state + 0xA) == factionId) {
                        *((unsigned char *)veh_state + 0xD) = 0;
                        *veh_state &= 0xfcffbdff;
                    }
                    veh_state += 0xd;
                    --n;
                } while (n != 0);
            }
            ++factionId;
            ++rules_flag;
            faction_state += 0x833;
        } while (rules_flag < TectonicDetonationCount + 8);
    }
}


// ORIGINAL: 0x004E1300 ?editor_clear_vehicles@Console@@QAEXXZ 0x004E1300-0x004E137F FILE
// working copy - scaffold materialised by --work
// size      127 bytes
// prototype void (__thiscall ?editor_clear_vehicles@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x00523DD0 0x005ABF20

void Console::editor_clear_vehicles() {
    auto_undo();
    GameState |= 4;
    int count = 0;
    VehCurrentCount = 0;
    BaseCurrentCount = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    if (MapArea > 0) {
        unsigned int *rec = reinterpret_cast<unsigned int *>(LockMapTable + 8);
        do {
            rec[0] &= 0xfffffffc;
            unsigned int val = rec[0];
            unsigned int *dst = rec + 2;
            for (int i = 0; i < 7; i++) {
                dst[i] = val;
            }
            count++;
            rec += 0xb;
        } while (count < MapArea);
    }
    reset_territory();
    ::draw_map(1);
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
// RECOVERY_BINDING: v1 {"evidence": ["Original5105C0..5107D5, with src/unrecovered/005105c0.cpp retained as a seed.", "Original early mode1/mode0 returns skip pumping. Ordinary cursor mode is captured after valid_patrol; each called receiver is retained for cache writeback while subsequent table slots and hotspots are read live.", "Original68FAF0 is MapLongitude, the half-width used by the wrap test; valid initialized vehicle/window state and representable coordinate arithmetic are required by the original."], "reason": "Console owns selection-mode admission and the shared map-window cursor update. Recover genuine receivers and cursor globals; the range and patrol refusals both select loaded sprite5."}
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
// TRIED: manual vbase-adjust pointer arith for Win::set_cursor `this` (obj + *(int*)(*(int*)obj+4)); #0 push/mov is frame-setup register order, not a logic mismatch
// working copy - scaffold materialised by --work
// size      185 bytes
// prototype void (__thiscall ?selected@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550 0x005EC7C0 0x005FCA30

void Console::selected() {
    if (field_23D10_ != 0) {
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
    int first = reinterpret_cast<int>(MapWinTable[0]);
    int mask = *reinterpret_cast<int *>(first + 0x1dd70) & 0x400000;
    int *p = reinterpret_cast<int *>(MapWinTable);
    do {
        int obj = *p;
        if (obj != 0 && (p == reinterpret_cast<int *>(MapWinTable) ||
                         *reinterpret_cast<int *>(obj + MapWinActiveOffset) != 0)) {
            if (mask == 0) {
                *reinterpret_cast<unsigned int *>(obj + 0x1dd70) |= 0x400000;
                reinterpret_cast<MapWin *>(obj)->draw_base_dest(1);
            } else {
                *reinterpret_cast<unsigned int *>(obj + 0x1dd70) &= 0xffbfffff;
                reinterpret_cast<MapWin *>(obj)->draw_map(1);
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
    GameMorePreferences &= 0xffdfffff;
    alpha_menu_.check(selected_vehicle_);
    X_pop(StrMenuSimple, 0);
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
        int vehCount = VehCurrentCount;

        if ((IsMultiplayerNet != 0) && (outerIdx == 0) && (focus_vehicle_ >= 0) &&
            (focus_vehicle_ < vehCount) &&
            (Vehs[focus_vehicle_].faction_id == ConsoleGlobal.faction_id_) &&
            (veh_ready(focus_vehicle_) != 0)) {
            result = focus_vehicle_;
            focus_vehicle_ = -1;
            goto searchDone;
        }

        vehCount = VehCurrentCount;
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
            vehCount = VehCurrentCount;
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
    int flags = GameState;
    flags |= 4;
    VehCurrentCount = 0;
    BaseCurrentCount = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    GameState = flags;
    draw_map(1);
    ConsoleInfoWin.set_redraw_phase(7);
    ConsoleInfoWin.on_redraw();
}

// Fixed-slot bindings carried from 004e0fd0.cpp

class VCall { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used, called with arg 0
    virtual void slot002();     // <-- used, called with no args
};

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
    Map *tile = reinterpret_cast<Map *>(LockMapTable);

    VehCurrentCount = 0;
    BaseCurrentCount = 0;
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
    GameState = GameState | 4;
    // 0x004E1033 loads slot zero before accessing the window's +0x1DD98
    // member. The array itself is neither the receiver nor member storage.
    MapWinTable[0]->field_1DD98_ = static_cast<uint32_t>(-14);
    MapWinTable[0]->draw_map(1);

    int flags = GameState;
    int wasSet = flags & 0x80;
    flags |= 0x80;
    GameState = flags;
    world_build();
    ::draw_map(1);
    if (wasSet == 0) {
        int f2 = GameState;
        f2 &= ~0x80;
        GameState = f2;
        ::draw_map(1);
    }

    // MainInterface construction at 0x0045EF10 creates 42 FlatButtons at
    // +0xE00 with stride 0xB4C. Element 2 is the original 0x007B0CB8
    // receiver; its table selects Win::show and BaseButton::hide.
    unsigned char flagByte = static_cast<unsigned char>(GameState);
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
    int *record = reinterpret_cast<int *>((*LockMapTable)) + 2;
    if (0 < guard) {
        do {
            *record &= 0xcfeffbff;
            int *dest = record + 2;
            int n = 7;
            do {
                *dest = *record;
                ++dest;
            } while (--n);
            ++i;
            record += 0xb;
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
        int base = (*LockMapTable);
        char *ptr = (char *)base + 8;
        int i = 0;
        do {
            uint32_t v = *(uint32_t *)ptr;
            if ((v & 0x400) != 0 && (v & 0x20000000) == 0) {
                v &= 0xdffffbff;
                *(uint32_t *)ptr = v;
                uint32_t *dst = (uint32_t *)(ptr + 8);
                int count = 7;
                do {
                    *dst = v;
                    dst++;
                    count--;
                } while (count != 0);
            }
            i++;
            ptr += 0x2c;
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
    const uint32_t flags = GameState;
    const uint32_t lock_mask = 0x8000000;
    if (flags & lock_mask) {
        GameState = flags & ~lock_mask;
        NetMsgPopup->pop(StrProtectOff, 5000, 0, 0);
        MainInterfaceGlobal.draw_edit_pane();
        return;
    }
    GameState = flags | lock_mask;
    NetMsgPopup->pop(StrProtectOn, 5000, 0, 0);
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
    const int flags = GameMorePreferences;
    int new_flags;
    if (flags & 0x1000) {
        new_flags = flags & 0xFFFFCFFF;
    } else if (flags & 0x2000) {
        new_flags = flags | 0x1000;
    } else {
        new_flags = flags | 0x2000;
    }
    GameMorePreferences = AlphaIniPrefs.more_preferences = new_flags;
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
    const int flags = GameMorePreferences ^ 0x10000;
    GameMorePreferences = flags;
    if (flags & 0x10000) {
        alpha_menu_.check_menu_item(2, 0x209);
    } else {
        alpha_menu_.uncheck_menu_item(2, 0x209);
    }
    AlphaIniPrefs.more_preferences = GameMorePreferences;
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
    const int flags = GamePreferences ^ 0x100;
    GamePreferences = flags;
    if (flags & 0x100) {
        alpha_menu_.check_menu_item(2, 0x20A);
    } else {
        alpha_menu_.uncheck_menu_item(2, 0x20A);
    }
    AlphaIniPrefs.preferences = GamePreferences;
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
    const int flags = GamePreferences ^ 0x200;
    GamePreferences = flags;
    if (flags & 0x200) {
        alpha_menu_.check_menu_item(2, 0x20B);
    } else {
        alpha_menu_.uncheck_menu_item(2, 0x20B);
    }
    AlphaIniPrefs.preferences = GamePreferences;
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
// console.cpp's own CursorVehActive (0x00939288) and CursorVehIndex
// (0x0093928C). 0x007B0CB8 is the same singleton editor_generate dispatches
// through, so the body reuses console.cpp's file-scope VCall shim rather
// than carrying a second copy of it.
// LEVER: the loop bound is `static_cast<int>(MapArea)` and not MapArea
// itself. MapArea is uint32_t (map.h) and the emitter's `veh_count` was
// int, so the plain spelling compiled `jbe` against the image's `jl` -
// signedness, not scheduling, and the only divergence in the body.


void Console::editor_fast() {
    auto_undo();
    unsigned int state = GameState | 4;
    unsigned char *base = reinterpret_cast<unsigned char *>(map_tiles());
    VehCurrentCount = 0;
    BaseCurrentCount = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.selected_vehicle_ = -1;
    GameState = state;
    for (int i = 0; i < static_cast<int>(MapArea); i++) {
        *reinterpret_cast<unsigned int *>(base + i * 0x2c + 8) &= 0xfffffffc;
    }
    unsigned int flag80 = GameState & 0x80;
    GameState = GameState & 0xffffff7f;
    world_build();
    if (flag80 != 0) {
        GameState = GameState | 0x80;
    }
    unsigned char al = *reinterpret_cast<unsigned char *>(&GameState);
    if ((al & 0x80) != 0 && (al & 0x40) != 0) {
        reinterpret_cast<VCall *>(0x007B0CB8)->slot001(0);
    } else {
        reinterpret_cast<VCall *>(0x007B0CB8)->slot002();
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

// Fixed-slot bindings carried from 004e0a50.cpp
// EditorPaintId: use-site evidence, see editor_paints
static int *const EditorPaintId = (int *)0x00945AE0;
// EditorPaintSlotsLow: use-site evidence, see editor_paints
static int *const EditorPaintSlotsLow = (int *)0x00691888;
// EditorPaintSlotsHigh: use-site evidence, see editor_paints
static int *const EditorPaintSlotsHigh = (int *)0x00691AE8;
// PaintCycleGroup: use-site evidence, see editor_paints
static ButtonGroup *const PaintCycleGroup = (ButtonGroup *)0x007CD1C0;
// PaintCycleSelection: use-site evidence, see editor_paints
static int *const PaintCycleSelection = (int *)0x007CD244;

// ORIGINAL: 0x004E0A50 ?editor_paints@Console@@QAEXH@Z 0x004E0A50-0x004E0ACF FILE
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
    // Use-site names (the census rule): 0x00945AE0 is written here and read
    // by every editor_* painter as the current paint id; 0x00691AE8..0x00691888
    // is the paint-availability walk's table (8-int stride, high end down);
    // 0x007CD1C0/0x007CD244 are the map-paint cycle ButtonGroup and its
    // selected-slot int (set(result, 1) only when different).
    InfoWin *const win = &ConsoleInfoWin;
    *EditorPaintId = paint_id;
    ConsoleInfoWin.set_redraw_phase(7);
    win->on_redraw();
    MainInterfaceGlobal.draw_edit_pane();
    int result = paint_id + 0x3f3;
    if (paint_id >= 6 && paint_id < 0x1a) {
        int counter = paint_id - 6;
        int limit = 0x13;
        int *p = EditorPaintSlotsHigh;
        do {
            if (*p == 0 && limit <= counter) {
                --counter;
            }
            p -= 8;
            --limit;
        } while (p >= EditorPaintSlotsLow);
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
// popup is netmsg.h's NetMsgPopup, and the two labels read DESIGNATED /
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
                NetMsgPopup->pop(StrDesignated, 5000, 0, 0);
                return;
            }
            NetMsgPopup->pop(StrUndesignated, 5000, 0, 0);
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
// callback - and on_key_click is not homed yet, so nothing in this build
// reaches the body either way.
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
            unsigned char state = *reinterpret_cast<unsigned char *>(&GameState);
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
// symbol    ?on_mouse_feedback@MapWin@@$4PPPPPPPM@CDCI@AEXHH@Z
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
            (ORIGINAL(&TutWinBaseWindow)->*BaseWinOnRedraw)();
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
        x_pop_value_flags(StrBrush, *EditorBrushId, 0x40, (NullCallback)0);
    PopupModalState = 0;
    if (answer >= 0) {
        *EditorBrushId = answer;
    }
    ConsoleInfoWin.set_redraw_phase(7);
    ConsoleInfoWin.on_redraw();
    MainInterfaceGlobal.draw_edit_pane();
    if (WinScreenWidth != 0x320) {
        const int brush_button = *EditorBrushId + 0x40C;
        if (*EditorBrushButtonGroupSelection != brush_button) {
            EditorBrushButtonGroup->set(brush_button, 1);
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
    (ORIGINAL(&TutWinBaseWindow)->*BaseWinExit)();
    if (IsMultiplayerNet != 0 && (GameState & 0x1000) == 0) {
        return;
    }
    if (IsMultiplayerPBEM != 0 && (GameState & 0x1000) == 0) {
        return;
    }
    if (load_game(save_slot, faction_id) != 0) {
        return;
    }
    if (*GameSessionActive == 0) {
        desktop_init(0);
    } else if (game_reload(1, 0) != 0) {
        exit(3);
    }
    if (save_slot != 0) {
        GameState |= 0xE0;
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
    const int saved_group_count = *ConsoleGroupCount;

    if (ConsoleGlobal.cursor_active_ == 0) {
        for (;;) {
            if (field_23BE4_ == 0) break;
            if (field_23BE8_ == 0) break;
            if (ExitTurnLoop != 0) break;

            if (end_of_game(0) != 0) break;
            if (saved_group_count != *ConsoleGroupCount) break;

            if (IsMultiplayerNet != 0) {
                if ((GameState & 0x800) != 0) break;
            }

            wait_loop();
            if (*TutWinTourPending != 0) {
                *TutWinTourPending = 0;
                (ORIGINAL(TutWinGlobal)->*TutWinTour)();
            }
            if (ConsoleGlobal.cursor_active_ != 0) break;
        }
    }

    field_23BE4_ = 0;
}

// 0x004D9AD0 ?editor_who@Console@@QAEHHH@Z - which faction/personality the
// scenario editor is working on. The catalogue, the partition and
// editor_energy's own call list all name this address (span
// 0x004D9AD0-0x004D9DB4); the console.h declaration carries the corrected
// address too.
// Link-only placeholder: the seed (src/unrecovered/004d9ad0.cpp) is preserved
// and its transcription is still blocked on the private-member access the
// console.h note records.
// ORIGINAL: 0x004D9AD0 ?editor_who@Console@@QAEHHH@Z 0x004D9AD0-0x004D9DB4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004d9ad0.cpp", "source:src/console.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU. Empty link-only placeholder; original behavior is not implemented."}
int Console::editor_who(int, int) {
    // BODY GOES HERE.
    // This definition provides build continuity only - editor_energy calls
    // it; the seed body is preserved, not transcribed.
    return 0;
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
        NetMsgPopup->pop(StrNoBomb, 5000, 0, 0);
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

    parse_field_number(0, ObjectiveReqVictory);
    parse_field_number(1, ObjectivesSuddenDeathVictory);
    parse_field_number(2, ObjectiveAchievePts);
    parse_field_number(3, VictoryAchieveBonusPts);
    parse_field_number(4, StartingMissionYear);
    parse_field_number(5, EndingMissionYear);
    parse_field_number(6, SunspotDuration);

    popup.start(PopupStartCaption, StrScenParam, -1, nullptr, 0x44, nullptr);

    if (popup.exec(0, 0) < 0) {
        popup.close();
        return;
    }

    // The parser parked each field's text in ParseStrBuffer; read the edited
    // values back out.
    ObjectiveReqVictory = atoi(ParseStrBuffer[0].str);
    ObjectivesSuddenDeathVictory = atoi(ParseStrBuffer[1].str);
    ObjectiveAchievePts = atoi(ParseStrBuffer[2].str);
    VictoryAchieveBonusPts = atoi(ParseStrBuffer[3].str);
    StartingMissionYear = atoi(ParseStrBuffer[4].str);
    EndingMissionYear = atoi(ParseStrBuffer[5].str);
    SunspotDuration = atoi(ParseStrBuffer[6].str);

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
        NetMsgPopup->pop(StrSimulNo, 5000, 0, 0);
        return;
    }
    if (veh_ready(veh_id) == 0) {
        NetMsgPopup->pop(StrUnitMoved, 5000, 0, 0);
        return;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, -1) != 0) {
        return;
    }
    if (not_my_turn()) {
        NetMsgPopup->pop(StrSimulNo2, 5000, 0, 0);
        NetDaemonNet.unlock_veh();
        return;
    }
    if (veh_ready(veh_id) == 0) {
        NetMsgPopup->pop(StrUnitMoved2, 5000, 0, 0);
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
        NetMsgPopup->pop(StrOnlyCombat, 5000, 0, 0);
        NetDaemonNet.unlock_veh();
        return;
    }
    if (((tile->bit & 1) != 0) && ((tile->val2 & 0xF) < 8) &&
        ((tile->val2 & 0xF) >= 0)) {
        NetDaemonNet.unlock_veh();
        NetMsgPopup->pop(StrNotBase, 5000, 0, 0);
        return;
    }

    const int triad = Chassis[VehPrototypes[proto_id].chassis_id].triad;
    if ((triad != 2) && (static_cast<int>(not_ocean) != (triad == 1))) {
        NetDaemonNet.unlock_veh();
        NetMsgPopup->pop(StrNotInTriad, 5000, 0, 0);
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
        NetMsgPopup->pop(StrNotHere, 5000, 0, 0);
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
        if (BreakTreaty(faction, territory, 7) != 0) {
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
        ActionDestroy(veh_id, result, -1, -1);
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

    const unsigned char state = static_cast<unsigned char>(GameState);
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
        NetMsgPopup->pop(StrHostTime, 5000, 0, 0);
        popup.close();
        return;
    }

    time_controls_dialog(&popup);
    // BasePop's value slot (basepop.h's private value_ at 0xA44): the row the
    // dialog opens preselected. editor_diplomacy spells the same write the
    // same way.
    *reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(&popup) + 0xa44) =
        static_cast<uint32_t>(*TimeControlsSelection);
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
