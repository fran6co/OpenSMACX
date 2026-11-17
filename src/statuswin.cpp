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
// ORIGINAL: 0x004210F0 ??1Caviar@@QAE@XZ 0x004210F0-0x004210F5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original destructor span and its direct close target.", "Original model arrays and renderer embedding EH cleanup distinguish loaded-model and instance lifetimes.", "Fresh VC6 observations under both configured flag sets identify this natural emission host and preserve normal versus EH call targets.", "git:0e184947304cbb843b26264d5145e9b9b452cf16:src/unrecovered/004210f0.cpp", "Historical seed spelling ??1Caviar@@QAE@XZ; current source mapping ??1Caviar@@QAE@XZ; emitted symbol ??1Caviar@@QAE@XZ."], "reason": "Genuine header-defined destructor in src/caviar.h; this translation unit naturally emits the original five-byte cleanup tail through real object lifetime use, without synthetic calls or forced exports. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// LEVER: thiscall shim - a synthetic class exposes close() as a member so "call close(); ret" folds into the original's bare 5-byte jmp
// body      src/caviar.h
// ORIGINAL: 0x004B3FA0 _$E21 0x004B3FA0-0x004B3FB6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825f4 selects original initializer0x4b3fa0..0x4b3fb6, registering fini0x4b3fc0..0x4b3fca; complete independent decode establishes exclusive boundaries.", "Source-owned object?ConsoleStatusWin@@3VStatusWin@@A identifies original storage0x8c5568. Startup arguments/calls: call 0x4ba1a0; push 0x4b3fc0. Cleanup receiver/calls: mov ecx, 0x8c5568; jmp 0x4ba120. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/statuswin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?ConsoleStatusWin@@3VStatusWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E16
// ORIGINAL: 0x004B3FC0 _$E19 0x004B3FC0-0x004B3FCA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825f4 selects original initializer0x4b3fa0..0x4b3fb6, registering fini0x4b3fc0..0x4b3fca; complete independent decode establishes exclusive boundaries.", "Source-owned object?ConsoleStatusWin@@3VStatusWin@@A identifies original storage0x8c5568. Startup arguments/calls: call 0x4ba1a0; push 0x4b3fc0. Cleanup receiver/calls: mov ecx, 0x8c5568; jmp 0x4ba120. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/statuswin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?ConsoleStatusWin@@3VStatusWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E14
#define BUFFER_WRITE_RIGHT_L3  // guarded Buffer::write_right_l(text, x, y)
#define SPOT_ADD_RECT_INLINE  // stand-in: see Spot::add(int, int, RECT *) in spot.h
#include "stdafx.h"
#include "interface_fonts.h"
#include "base.h"
#include "battlewin.h"
#include "map.h"
#include "terraforming.h"
#include "technology.h"  // has_tech is declared by its technology owner.
#include "strings.h"  // StringTemp, 0x009B86A0
#include "statuswin.h"
#include "spritebox.h"
#include "font.h"
#include "font_metrics.h"
#include "infowin.h"
#include "win.h"
#include "graphicwin.h"
#include "mapwin.h"  // MainInterfaceGlobal
#include "alpha.h"
#include "basewin.h"
#include "console.h"
#include "datalink.h"
#include "game.h"
#include "messagewin.h"
#include "interface_palette.h"
#include "faction.h"  // spying() is owned by the faction API
#include "netdaemon.h"
#include "popmenu.h"
#include "tutwin.h"
#include "veh.h"

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];

// Shared separator objects the image pushes instead of unit literals.
extern const char BufferJoinSpace[];
extern char SocialWinColonSpaceText[];
extern char SocialWinCommaSpaceText[];

// Insert in src/statuswin.cpp after the project headers; palette.cpp owns storage.
extern int TextColourTable[195];

void __cdecl help_any();

// The status and information-panel initialization latch is shared across
// three window owners. The pinned image places this zero-filled word here.
// RECOVERY_OBJECT: v2 0x00939ED8 0x00939EDC ?StatusFlags@@3IA 0x00458D60
uint32_t StatusFlags;

// The route destination renderer is called by MapWin::set_center. Its
// retained seed indexes genuine Veh and chassis records, then publishes the
// signed vehicle ID shared with StatusWin's selection lifecycle.
// ORIGINAL: 0x004B4BF0 ?draw_my_dest@@YAXH@Z 0x004B4BF0-0x004B4CC5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/unrecovered/004b4bf0.cpp retained BYTE_EXACT seed; Vehs stride 0x34, VehPrototypes chassis_id offset 0x24, Chassis triad offset 0x49.", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/unrecovered/004b4bf0.cpp"], "reason": "Home the destination vehicle route painter reached by MapWin recentering using Veh and Chassis records. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void __cdecl draw_my_dest(int vehicle_id) {
    if (vehicle_id < 0 || vehicle_id >= GameDataStateStorage.fields_.vehicle_current_count_) {
        return;
    }
    if (Vehs[vehicle_id].faction_id != ConsoleGlobal.faction_id_ &&
        (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0) {
        return;
    }
    if (Vehs[vehicle_id].order != 0x18) {
        return;
    }
    ConsoleGlobal.cover_window_.fill(9);
    Veh &vehicle = Vehs[vehicle_id];
    const int prototype_id = vehicle.proto_id;
    const int chassis_id = VehPrototypes[prototype_id].chassis_id;
    const int route_color = Chassis[chassis_id].triad;
    ConsoleGlobal.dest_line(vehicle.x, vehicle.y,
                            vehicle.waypoint_x[0], vehicle.waypoint_y[0],
                            prototype_id, vehicle.faction_id, route_color,
                            0, 3, 5, 1);
    ConsoleGlobal.cover_window_.update(static_cast<GraphicWin *>(0));
    DestinationVehicleId = vehicle_id;
}

// ORIGINAL: 0x004B4CD0 ?UNK2@StatusWin@@QAEXXZ 0x004B4CD0-0x004B4CE4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x004B4CD0-0x004B4CE4: store -1 at 0x006874C4, set ECX 0x00937118, tail-call Win::hide."], "reason": "Home the destination hide operation over the typed destination vehicle state and ConsoleGlobal.cover_window_."}
void StatusWin::UNK2() {
    DestinationVehicleId = -1;
    ConsoleGlobal.cover_window_.Win::hide();
}

// RECOVERY_ASSOCIATED: v1 switch_selector 0x004B9010 0x004B9E30 0x004B9E8A 0x004B9659
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004B9010 0x004B9DF8 0x004B9E30 0x004B965F
// ORIGINAL: 0x004B9010 ?on_right_click@StatusWin@@QAEXHHH@Z 0x004B9010-0x004B9DF8;0x00659AA0-0x00659BCA
// RECOVERY_BINDING: v1 {"evidence": ["image:0x4b9010"], "reason": "Recover the full original operation using owned C++ members and base lifetimes; retain all original branches and expose unresolved callees instead of executing original-address forwarders."}
void StatusWin::on_right_click(int x, int y, int) {
    PullDown menu;
    PopMenu display_menu;
    menu.init(0);
    // Neither out-parameter is seeded; a missed spot still offers Help.
    int object_index;
    int menu_id;
    int base_id;
    int screen_y, screen_x;
    if (spot_.check(x, y, &object_index, &menu_id) >= 0) {
        if (menu_id == 3) {
            display_menu.init();
            char *text = label_get(0xFEC / 4);
            display_menu.dialogs().item(text, 0);
            text = StringTable.get(Labels.strings_ptr[0xFF0 / 4]);
            display_menu.dialogs().item(text, 1);
            text = label_get(0xFF4 / 4);
            display_menu.dialogs().item(text, 2);
            MainInterfaceGlobal.get_mouse_pos(&screen_x, &screen_y);
            MainInterfaceGlobal.client_to_screen(&screen_x, &screen_y);
            const int choice = display_menu.exec(screen_x, screen_y, 0);
            if (choice >= 0) {
                switch (choice) {
                case 0:
                    field_15A8_ = 0;
                    draw_status(-1, -1, -1, -1, 0);
                    break;
                case 1:
                    field_15A8_ = -1;
                    draw_status(-1, -1, -1, -1, 0);
                    break;
                case 2:
                    DatalinkMain.exec(9, 0);
                    break;
                }
            }
            return;
        }
        if (menu_id != 2) {
            if (menu_id == 0 || menu_id == 10) {
                if (menu_id == 10) object_index = ConsoleGlobal.selected_vehicle_;
                menu.add_item(6, label_get(0x958 / 4));
                menu.add_item(7, label_get(0x112C / 4));
                if (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) {
                    menu.add_item(90, label_get(0xDFC / 4));
                }
                menu.add_separator(99);
            } else {
                base_id = object_index;
                if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
                    || Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
                    || spying(Bases[base_id].faction_id_current)
                    || (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[Bases[base_id].faction_id_current] & 1)) {
                    menu.add_item(5, StringTable.get(Labels.strings_ptr[0x894 / 4]));
                }
                if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
                    || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
                ::clear();
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x898 / 4]));
                menu.add_item(51, StringTemp);
                ::clear();
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x89C / 4]));
                strcat(StringTemp, BufferJoinSpace);
                // This caller passes the facility name through Strings::get.
                // RulesFacility still types its stored string offset as LPSTR;
                // that shared type/loader disagreement needs coupled recovery.
                const int production = Bases[base_id].queue_production_id[0];
                strcat(StringTemp, production < 0
                    ? StringTable.get(Facility[-production].name)
                    : VehPrototypes[production].veh_name);
                menu.add_item(52, StringTemp);
                ::clear();
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0x8A0 / 4));
                menu.add_item(53, StringTemp);
                ::clear();
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0x8A4 / 4));
                menu.add_item(54, StringTemp);
                ::clear();
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0x8A8 / 4));
                menu.add_item(55, StringTemp);
                ::clear();
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0x8AC / 4));
                menu.add_item(56, StringTemp);
                menu.add_separator(99);
                }
            }
        }
    }
    menu.add_item(20, label_get(0x8FC / 4));
    MainInterfaceGlobal.client_to_screen(&x, &y);
    menu.show(x, y, 0);
    // Only the signed upper bound is tested, as in the image (0x004B95EB).
    const int slot = menu.get_selected();
    const int selected = slot < 64 ? menu.item_id(slot) : -1;
    int network_blocked;
    int faction_blocked;
    if (IsMultiplayerNet) {
        network_blocked = ConsoleControlTurnActive || NetDaemonNet.field_1BB4_
            || NetDaemonNet.primary_state_.leading_words_[2];
        faction_blocked = (1 << ConsoleGlobal.faction_id_)
            & NetDaemonNet.primary_state_.trailing_words_[6];
    } else {
        faction_blocked = 0;
        network_blocked = 0;
    }

    switch (selected) {
    case 1:
        iUnitIndex_ = -1;
        iCityIndex_ = base_id;
        redraw();
        break;
    case 2:
        iCityIndex_ = -1;
        iUnitIndex_ = object_index;
        redraw();
        break;
    case 5:
        TutWinBaseWindow.zoom(base_id, 0);
        break;
    case 51:
        if (!network_blocked && !faction_blocked) {
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.production(0, 0);
        }
        break;
    case 52:
        if (!network_blocked && !faction_blocked) {
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.hurry();
        }
        break;
    case 53:
        if (!network_blocked && !faction_blocked) {
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.gov_on(0x1000000);
        }
        break;
    case 54:
        if (!network_blocked && !faction_blocked) {
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.gov_on(0x2000000);
        }
        break;
    case 55:
        if (!network_blocked && !faction_blocked) {
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.gov_on(0x4000000);
        }
        break;
    case 56:
        if (!network_blocked && !faction_blocked) {
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.gov_on(0x8000000);
        }
        break;
    case 6:
        if (veh_selectable(object_index)) {
            ConsoleGlobal.selected_vehicle_ = object_index;
            if (NetDaemonNet.lock_veh(&object_index, 0, -1, -1, 0) == 0) {
                veh_wake(object_index);
                synch_veh(object_index);
                NetDaemonNet.await_synch();
                NetDaemonNet.unlock_veh();
            }
            ConsoleGlobal.set_move(1);
        }
        break;
    case 7:
        iUnitIndex_ = object_index;
        redraw();
        break;
    case 20:
        help_any();
        break;
    case 90:
        ConsoleGlobal.editor_edit_veh(object_index);
        break;
    }
    if (selected != 5 && selected != 6 && (hover_vehicle_id_ >= 0 || hover_base_id_ >= 0)) {
        hover_vehicle_id_ = hover_base_id_ = -1;
        draw_active(field_15CC_, field_15D0_, 0);
    }
}

/*
Purpose: Compose a StatusWin from its Caviar, three Font and one Spot
         subobjects. All four are ordinary typed members now (see the class
         comment in statuswin.h), so the implicit member-init sequence -
         built in declaration order, with no null-pointer guard around any
         of it - reproduces the image's four real constructor calls with an
         empty body.
// ORIGINAL: 0x004BA1A0 ??0StatusWin@@QAE@XZ 0x004BA1A0-0x004BA217;0x00659A60-0x00659A9F BYTE_EXACT
// LEVER: a placement-new version of this body (since replaced) measured 7/32 - matching the SEH prologue through instruction 6, then diverging on register allocation. Every `new (self+off) T()` VC6 guards with a null-pointer `cmp`/`je`, which the image's real member-init sequence never emits; real typed members in declaration order drop the guard entirely.
// size      182 bytes
// prototype void (__thiscall ??0StatusWin@@QAE@XZ)(StatusWin* this)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FA860 0x00616DA0 0x00618EA0
Return Value: n/a
Status: Complete
*/
StatusWin::StatusWin() {
}

/*
Purpose: Tear down a StatusWin. All five subobjects are ordinary typed
         members (see the class comment in statuswin.h), so the implicit
         destruction sequence - reverse declaration order: Spot, font3,
         font2, font1, then Caviar's own destructor, which forwards to
         close() - reproduces the image's exact five-call sequence with an
         empty body.
// ORIGINAL: 0x004BA120 ??1StatusWin@@QAE@XZ 0x004BA120-0x004BA19E;0x00659A20-0x00659A5F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:prolog 0x4BA125 registers handler 0x659A55; FuncInfo 0x678CC0 and unwind map 0x678CE0 contain four HIGHLOW-backed actions in the completely decoded cold region 0x659A20..0x659A5F."], "reason": "Preserve the historical destructor claim and strengthen its original mapping with the independently owned cleanup region; this does not assert that the newly included generated bytes currently match."}
// LEVER: an explicit-call version of this body (since replaced) measured 0/32 - the image opens `push ebp; mov ebp,esp; push -1; push 0x659a55`, a real SEH frame protecting the four-subobject unwind, which straight-line explicit `~Font()`/`~Spot()`/`close()` calls never reproduced. The compiler's own implicit per-member destruction gets that protection for free.
// size      126 bytes
// prototype void (__thiscall ??1StatusWin@@QAE@XZ)(StatusWin* this)
// callers   0   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
Return Value: n/a
Status: Complete
*/
StatusWin::~StatusWin() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004B9F80 ?close@StatusWin@@QAEXXZ 0x004B9F80-0x004B9F81 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void StatusWin::close() {
}


/*
Purpose: Place the status window, invalidating the two cached values that
         follow its position.
// ORIGINAL: 0x004B9F90 ?set_loc@StatusWin@@QAEXHH@Z 0x004B9F90-0x004B9FB8 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?set_loc@StatusWin@@QAEXHH@Z)(StatusWin* this, int, int)
// callers   8   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void StatusWin::set_loc(int x, int y) {
    iTileX_ = x;
    iTileY_ = y;
    iCityIndex_ = -1;
    iUnitIndex_ = -1;
}



/*
Purpose: Release the status window's hold on the interface mode, if it holds
         one. The original tail-jumps into the release, so nothing follows it.
// ORIGINAL: 0x004B8970 ?reset@StatusWin@@QAEXXZ 0x004B8970-0x004B898F BYTE_EXACT
// size      31 bytes
// prototype void (__thiscall ?reset@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void StatusWin::reset() {
    if (field_15D4_) {
        field_15D4_ = 0;
        g_BattleWin.release_iface_mode();
    }
}


// Three fixed-address RECTs redraw() unions together, no established
// identity beyond what this one function shows. Now defined in statuswin.h,
// where the internal linkage folds the address to the immediate in every TU.

// The process-wide status window, 0x008C5568 in the image - REAL STORAGE, not
// the `StatusWin *const ConsoleStatusWin` binding that named terranx.exe data
// (see console.h). The image's dynamic initializer at 0x004B3FA0 runs the
// real StatusWin constructor against it and registers the deleting destructor
// with atexit; here the same recovered constructor runs through this build's
// own startup and the destructor closes it at exit.
// The 0x15EC-byte StatusWin ends at 0x008C6B54, a separate word that the
// free function say_morale (0x004B3FD0) reads and writes absolutely.
// RECOVERY_OBJECT: v2 0x008C5568 0x008C6B54 ?ConsoleStatusWin@@3VStatusWin@@A 0x004B3FA0
StatusWin ConsoleStatusWin;  // 0x008C5568

// Fixed-slot bindings carried from 004b6570.cpp

// ORIGINAL: 0x004B6570 ?draw_status@StatusWin@@QAEXHHHHH@Z 0x004B6570-0x004B8825 FILE
// DEFERRED: genuinely more tractable than this pass reached - frame is only 0xD8 bytes (no embedded C++ objects the way the other six addresses in this batch have; this is plain locals only), and the member map shows this function itself touches exactly ONE StatusWin field (field_15DC_, a plain store at 0x004B6588 - every other offset the tool lists belongs to a SIBLING StatusWin method shown for context, not to draw_status). The remaining size (8885B, 2569 instructions, 829 Ghidra lines) is volume, not structural depth: it is a base-stats HUD renderer, dominated by _strcat(86)/?get@Strings@@(35)/_strlen(28)/?write_l@Buffer@@(15)/ __itoa(12)/?set_text_color@Buffer@@(9) - build a label string, draw it - interleaved with ~25 one-off game-data queries (?mine_yield@@, ?energy_yield@@, ?base_at@@, ?veh_at@@, ?bonus_at@@, ?has_tech@@, ?spying@@, ?base_draw@@, ?kill_type@Spot@@, ...) each needing its own argument set read off the disassembly. Ghidra also reports five "Removing unreachable block" warnings, meaning its own CFG differs from the raw disassembly's in at least five places - a real structural question (dead branches from an inlined constant vs. a genuine goto target this decompile mis-collapsed), not yet checked.
// UNBLOCKER: because per-call risk is genuinely low here (a wrong string
//     ID or draw position does not cascade into unrelated fields, unlike
//     the faction/tech bit arithmetic seen in the other addresses in this
//     batch), this is the best remaining candidate for a bulk transcribe-
//     from-Ghidra pass - AFTER first resolving the five "unreachable
//     block" addresses (0x004B7A25, 0x004B75AE, 0x004B7240, 0x004B731B,
//     0x004B7BE1) against the raw disassembly, since those are exactly
//     where a silent control-flow divergence would hide.
// working copy - scaffold materialised by --work
// size      8885 bytes
// prototype void (__thiscall ?draw_status@StatusWin@@QAEXHHHHH@Z)(StatusWin* this, int, int, int, int, int)
// callers   7   call targets   36
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00421B50 0x004632D0 0x004B4CF0 0x004C9B00 0x004E3A50 0x004E6E50 0x004E7310 0x004E7750 0x005594F0 0x0055AF20 0x0055BC00 0x0057D7D0 0x005882F0 0x005919C0 0x00592030 0x005B8C90 0x005B9F20 0x005BFE90 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DCEA0 0x005DD020 0x005DD130 0x005DD3B0 0x005DD530 0x005E2DD3 0x005E3203 0x005FA960 0x005FAA90 0x006169A0 0x00619280 0x006453E0 0x00645470 0x0064FC88
// indirect  0x004B8246 0x004B8254 0x004B8261 0x004B829D 0x004B82AD

// Typed StatusWin::draw_status recovery from the pinned native body.
// Retained KUNA pseudocode was used only as a control-flow aid; all data and
// calls below are expressed through current source owners.
// The dead battle-report parameters are reused: the image keeps the vehicle
// in [ebp+8] and the tile x in [ebp+0x14]; separate locals pack differently.
void StatusWin::draw_status(int veh, int a2, int a3, int x, int a5) {
    field_15DC_ = 0;
    if (veh >= 0 && a2 >= 0) {
        g_BattleWin.battle_report(veh, a2, a3, x, a5);
        return;
    }
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage]) {
        return;
    }
    spot_.kill_type(0);
    spot_.kill_type(1);
    spot_.kill_type(2);
    spot_.kill_type(3);
    spot_.kill_type(10);
    spot_.kill_type(11);
    font1_.width(const_cast<char *>("-"));
    int line_y = 1;
    int row_pitch = font_line_advance(font1_);
    MainInterfaceGlobal.set_font(&font1_, &font3_, &font2_, 0);

    char digits[80];
    // 0x004B665E: max_width lives in [ebp-0x28] with dead stores kept; that
    // slot later holds item_x + item_width (0x004B82C9, read 0x004B8401).
    // WORKAROUND(volatile): the image keeps max_width and alt in the frame
    // ([ebp-0x28], [ebp-0x1c]) and keeps max_width's dead max() stores
    // (0x004B726B, 0x004B75DC); a plain int is enregistered or dead-store
    // eliminated, which also moves `this` out of [ebp-0x18], drops the edi
    // zero register at entry and shrinks the frame to 0xD4. The original
    // reason these two stay in memory is unknown.
    volatile int max_width = 0;
    // Shared image slots: [ebp-0x14] holds the text widths and the visible
    // item count (0x004B731E, 0x004B8237); [ebp-0x24] holds the stack row
    // height (0x004B8179). One reused variable each.
    int col;
    int row;
    volatile int alt;
    veh = ConsoleGlobal.selected_vehicle_;
    int y;
    if (ConsoleGlobal.cursor_active_ && veh >= 0
        && GameDataStateStorage.fields_.turn_current_num_
        && iTileX_ == Vehs[veh].x && iTileY_ == Vehs[veh].y) {
        x = Vehs[veh].x;
        y = Vehs[veh].y;
    } else {
        x = iTileX_;
        y = iTileY_;
        veh = GameDataStateStorage.fields_.turn_current_num_ ? veh_at(x, y) : -1;
        int base_id = base_at(x, y);
        int top = veh_top(veh);
        if (top >= 0 && Vehs[veh].faction_id != ConsoleGlobal.faction_id_
            && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
            if (!(PlayersData[ConsoleGlobal.faction_id_].flags & 0x200)
                && !(map_loc(x, y)->visibility & (1 << ConsoleGlobal.faction_id_))) {
                veh = -1;
            } else if (base_id >= 0) {
                // The dead a5 argument is reused as the "seen" flag: the
                // image keeps it in [ebp+0x18] and tests it after the
                // loop instead of threading the constants (0x004B6821).
                // 0x004B67BF/0x004B67CC: both tests jump forward to the
                // shared a5 = 1 block, which the image places after the
                // first jmp inside the loop (a queued else arm).
                if (!(PlayersData[Bases[base_id].faction_id_current]
                          .diplo_treaties[ConsoleGlobal.faction_id_] & 1)
                    && !spying(Bases[base_id].faction_id_current)) {
                    a5 = 0;
                    for (int i = 0; i < 8; i++) {
                        int x2 = wrap_map_x(Bases[base_id].x + VehMoveDeltaX[i]);
                        int y2 = Bases[base_id].y + VehMoveDeltaY[i];
                        if (on_map(x2, y2)
                            && veh_who(x2, y2) == ConsoleGlobal.faction_id_) {
                            a5 = 1;
                            break;
                        }
                    }
                } else {
                    a5 = 1;
                }
                if (!((Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
                       || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
                       || (Bases[base_id].visibility & (1 << ConsoleGlobal.faction_id_)))
                      && a5)) {
                    veh = -1;
                }
            } else if (!((Vehs[top].faction_id == ConsoleGlobal.faction_id_
                          && (Vehs[top].flags & 0x440) != 0x440)
                         || (Vehs[top].visibility & (1 << ConsoleGlobal.faction_id_)))) {
                veh = -1;
            }
        }
    }

    int sel_base = -1;
    if (iUnitIndex_ >= 0) {
        if (Vehs[iUnitIndex_].x == x && Vehs[iUnitIndex_].y == y) {
            veh = iUnitIndex_;
        }
    } else if (iCityIndex_ >= 0) {
        if (Bases[iCityIndex_].x == x && Bases[iCityIndex_].y == y) {
            sel_base = iCityIndex_;
        }
    }
    field_15DC_ = veh >= 0;
    if (sel_base >= 0 && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
        && Bases[sel_base].faction_id_current != ConsoleGlobal.faction_id_
        && !(Bases[sel_base].visibility & (1 << ConsoleGlobal.faction_id_))) {
        sel_base = -1;
        iCityIndex_ = -1;
    }

    RECT area = MainInterfaceGlobal.status_right_rect();
    MainInterfaceGlobal.box_sprite(&area, &InterfaceBoxStyles[21]);
    area.left += 3;
    area.right -= 3;
    area.top += 3;
    area.bottom -= 3;
    MainInterfaceGlobal.box_sprite(&area, &InterfaceBoxStyles[18]);
    area.left += 3;
    area.right -= 3;
    area.top += 3;
    area.bottom -= 3;
    MainInterfaceGlobal.set_clip(&area);
    MainInterfaceGlobal.set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);

    if (!(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
        && !(PlayersData[ConsoleGlobal.faction_id_].flags & 0x200)
        && !(map_loc(x, y)->visibility & (1 << ConsoleGlobal.faction_id_))) {
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x300 / 4));
        MainInterfaceGlobal.write_cent_l(StringTemp, &area);
    } else {
        MapWinCaptionScale = area.right - area.left - 4;
        MapWinCaptionDivisor = 100;
        gen_terrain_poly(&MainInterfaceGlobal, 0, 0,
                         (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
                             ? -1 : ConsoleGlobal.faction_id_,
                         0, 0x621, x, y, area.left + 2, area.top + 1);
        MapWinCaptionScale = -1;
        MapWinCaptionDivisor = -1;
        int text_x = area.left;
        line_y = (area.right - area.left) / 2 + area.top + 3;
        if (field_15A8_) {
            line_y -= 4;
            MainInterfaceGlobal.set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(Resource[0].name_plural));
            strcat(StringTemp, SocialWinColonSpaceText);
            strcat(StringTemp, BufferJoinSpace);
            int crop = crop_yield(ConsoleGlobal.faction_id_, -1, x, y, 0);
            _itoa(crop, digits, 10);
            strcat(StringTemp, digits);
            MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
            line_y += row_pitch;
            if ((map_loc(x, y)->val3 & ~0x3F) < 0x80
                && (!(map_loc(x, y)->bit & BIT_FUNGUS)
                    || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF)
                && !(map_loc(x, y)->bit & (BIT_FARM | BIT_MONOLITH))
                && (crop < 2
                    || has_tech(Rules.tech_three_nutrients_sqr, ConsoleGlobal.faction_id_)
                    || bonus_at(x, y, 0) == 1
                    || (map_loc(x, y)->bit & BIT_CONDENSER))) {
                StringTemp[0] = '\0';
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0xfc4 / 4));
                strcat(StringTemp, SocialWinColonSpaceText);
                strcat(StringTemp, BufferJoinSpace);
                _itoa(crop + 1, digits, 10);
                strcat(StringTemp, digits);
                MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
            }
            line_y += row_pitch;
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(Resource[1].name_plural));
            strcat(StringTemp, SocialWinColonSpaceText);
            strcat(StringTemp, BufferJoinSpace);
            int minerals = mine_yield(ConsoleGlobal.faction_id_, -1, x, y, 0);
            _itoa(minerals, digits, 10);
            strcat(StringTemp, digits);
            MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
            line_y += row_pitch;
            if (!(map_loc(x, y)->bit & (BIT_THERMAL_BORE | BIT_MONOLITH))
                && !(map_loc(x, y)->bit & BIT_MINE)
                && (!(map_loc(x, y)->bit & BIT_FUNGUS)
                    || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF)
                && mine_yield(ConsoleGlobal.faction_id_, -1, x, y, 1) > minerals) {
                StringTemp[0] = '\0';
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0xfc8 / 4));
                strcat(StringTemp, SocialWinColonSpaceText);
                strcat(StringTemp, BufferJoinSpace);
                int mine_bonus = mine_yield(ConsoleGlobal.faction_id_, -1, x, y, 1);
                _itoa(mine_bonus, digits, 10);
                strcat(StringTemp, digits);
                MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
            }
            line_y += row_pitch;
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(Resource[2].name_plural));
            strcat(StringTemp, SocialWinColonSpaceText);
            strcat(StringTemp, BufferJoinSpace);
            int energy = energy_yield(ConsoleGlobal.faction_id_, -1, x, y, 0);
            _itoa(energy, digits, 10);
            strcat(StringTemp, digits);
            MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
            line_y += row_pitch;
            if (!(map_loc(x, y)->bit & (BIT_THERMAL_BORE | BIT_MONOLITH))
                && !(map_loc(x, y)->bit & BIT_SOLAR_TIDAL)
                && (!(map_loc(x, y)->bit & BIT_FUNGUS)
                    || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF)) {
                StringTemp[0] = '\0';
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0xfcc / 4));
                strcat(StringTemp, SocialWinColonSpaceText);
                strcat(StringTemp, BufferJoinSpace);
                int energy_bonus = energy_yield(ConsoleGlobal.faction_id_, -1, x, y, 1);
                _itoa(energy_bonus, digits, 10);
                strcat(StringTemp, digits);
                MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
            }
            line_y += row_pitch;
        } else {
            alt = alt_at(x, y) - 3;
            MainInterfaceGlobal.set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);
            if (alt < 0) {
                StringTemp[0] = '\0';
                switch (alt) {
                case -1:
                    strcat(StringTemp, label_get(0x564 / 4));
                    break;
                case -2:
                    strcat(StringTemp, label_get(0x304 / 4));
                    break;
                case -3:
                    strcat(StringTemp, label_get(0x568 / 4));
                    break;
                }
                int ocean_width = MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
                max_width = max_width > ocean_width ? max_width : ocean_width;
                line_y += row_pitch;
            }
            StringTemp[0] = '\0';
            // 0x004B727D: two branch loads joined before one get call.
            strcat(StringTemp, StringTable.get(alt >= 0 ? Labels.strings_ptr[0x2c / 4]
                                                        : Labels.strings_ptr[0x30 / 4]));
            strcat(StringTemp, SocialWinColonSpaceText);
            int elevation = elev_at(x, y);
            if (elevation < 0) {
                elevation = -elevation;
            }
            _itoa(elevation, digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, label_get(0x34 / 4));
            col = MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
            line_y += row_pitch;
            if (alt < 0) {
                if (GameDataStateStorage.fields_.climate_sea_level_delta_ < 0
                    && alt_detail_at(x, y) - MapSeaLevel
                        >= (int)AltNatural[3]
                            + GameDataStateStorage.fields_.climate_sea_level_delta_) {
                    StringTemp[0] = '\0';
                    strcat(StringTemp, label_get(0x7fc / 4));
                    MainInterfaceGlobal.set_text_color(0xf9, -1, 1, 1);
                    MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
                    line_y += row_pitch;
                }
            } else if (GameDataStateStorage.fields_.climate_sea_level_delta_ > 0
                       && alt_detail_at(x, y) - MapSeaLevel
                           < (int)AltNatural[3]
                               + GameDataStateStorage.fields_.climate_sea_level_delta_) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0x7fc / 4));
                MainInterfaceGlobal.set_text_color(0xf9, -1, 1, 1);
                MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
                line_y += row_pitch;
            }
            MainInterfaceGlobal.set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);
            int rocky = map_loc(x, y)->val3 >> 6;
            if (alt >= 0) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0x164 / 4 + rocky));
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, "&");
                strcat(StringTemp, BufferJoinSpace);
            }
            max_width = max_width > col ? max_width : col;
            if (alt >= 0) {
                strcat(StringTemp,
                       label_get(0x50 / 4 + ((map_loc(x, y)->climate >> 3) & 3)));
                int rain_width = MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
                max_width = max_width > rain_width ? max_width : rain_width;
                line_y += row_pitch;
                StringTemp[0] = '\0';
                if ((map_loc(x, y)->bit2 & 0x80000002) == BIT2_VOLCANO) {
                    strcat(StringTemp, StringTable.get(Natural[1].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80000001) == BIT2_CRATER) {
                    strcat(StringTemp, StringTable.get(Natural[0].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80000004) == BIT2_JUNGLE
                    && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
                    strcat(StringTemp, StringTable.get(Natural[2].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80000008) == BIT2_URANIUM) {
                    strcat(StringTemp, StringTable.get(Natural[3].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80000040) == BIT2_DUNES) {
                    strcat(StringTemp, StringTable.get(Natural[6].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80000100) == BIT2_MESA) {
                    strcat(StringTemp, StringTable.get(Natural[8].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80000200) == BIT2_CANYON) {
                    strcat(StringTemp, StringTable.get(Natural[9].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80000800) == BIT2_RIDGE) {
                    strcat(StringTemp, StringTable.get(Natural[11].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80002000) == BIT2_NEXUS) {
                    strcat(StringTemp, StringTable.get(Natural[13].name_short));
                }
                if (ExpansionEnabled
                    && (map_loc(x, y)->bit2 & 0x80004000) == BIT2_UNITY) {
                    strcat(StringTemp, StringTable.get(Natural[14].name_short));
                }
                if (strlen(StringTemp)) {
                    MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
                    line_y += row_pitch;
                }
                StringTemp[0] = '\0';
                if (map_loc(x, y)->bit & BIT_RIVER) {
                    strcat(StringTemp, label_get(0x3a8 / 4));
                }
                if ((map_loc(x, y)->bit & BIT_FUNGUS)
                    && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                    if (strlen(StringTemp)) {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    strcat(StringTemp, label_get(0x21c / 4));
                } else if (map_loc(x, y)->bit & BIT_FOREST) {
                    if (strlen(StringTemp)) {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    strcat(StringTemp, label_get(0x48c / 4));
                }
                if (strlen(StringTemp)) {
                    col = MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
                    line_y += row_pitch;
                }
            } else {
                StringTemp[0] = '\0';
                if (ExpansionEnabled
                    && (map_loc(x, y)->bit2 & 0x80008000) == BIT2_FOSSIL) {
                    strcat(StringTemp, StringTable.get(Natural[15].name_short));
                }
                if ((map_loc(x, y)->bit2 & BIT2_FRESH)
                    && (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
                    strcat(StringTemp, StringTable.get(Natural[7].name_short));
                }
                if ((map_loc(x, y)->bit2 & 0x80000400) == BIT2_GEOTHERMAL) {
                    strcat(StringTemp, StringTable.get(Natural[10].name_short));
                }
                if ((map_loc(x, y)->bit & BIT_FUNGUS)
                    && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                    if (strlen(StringTemp)) {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    strcat(StringTemp, label_get(0x398 / 4));
                }
                if (strlen(StringTemp)) {
                    col = MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
                    line_y += row_pitch;
                }
            }
            int bonus = bonus_at(x, y, 0);
            if (bonus > 0) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0x16c / 4 + bonus));
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0x17c / 4));
                MainInterfaceGlobal.write_l(StringTemp, text_x, line_y);
                line_y += row_pitch;
            }
            StringTemp[0] = '\0';
            uint32_t bits = map_loc(x, y)->bit;
            for (int i = 0; i < 19; i++) {
                if (Terraforming[i].bit && (bits & Terraforming[i].bit)
                    && i != TERRA_FOREST && i != TERRA_REMOVE_FUNGUS
                    && i != TERRA_PLANT_FUNGUS
                    && !(i == TERRA_ROAD && (bits & BIT_MAGTUBE))
                    && !(i == TERRA_FARM && (bits & BIT_SOIL_ENRICHER))) {
                    if (strlen(StringTemp)) {
                        strcat(StringTemp, SocialWinCommaSpaceText);
                    }
                    // The image indexes the name pair by the is_ocean result
                    // (lea [ocean + 8*i]).
                    strcat(StringTemp,
                           StringTable.get(Terraforming[i].name[is_ocean(x, y)]));
                }
            }
            if (map_loc(x, y)->bit & BIT_MONOLITH) {
                if (strlen(StringTemp)) {
                    strcat(StringTemp, SocialWinCommaSpaceText);
                }
                strcat(StringTemp, label_get(0x238 / 4));
            }
            if (strlen(StringTemp)) {
                area.top = line_y;
                line_y = MainInterfaceGlobal.wrap(StringTemp, &area);
            }
        }
    }

    if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR)
        && (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE)) {
        StringTemp[0] = '\0';
        strcat(StringTemp, SocialWinOpenParenText);
        _itoa(region_at(x, y), digits, 10);
        strcat(StringTemp, digits);
        strcat(StringTemp, SocialWinCloseParenText);
        MainInterfaceGlobal.set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);
        MainInterfaceGlobal.write_l(StringTemp, area.left + 2, area.bottom - row_pitch);
    }
    if (((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR)
         && (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE))
        || line_y < area.bottom - row_pitch) {
        StringTemp[0] = '\0';
        strcat(StringTemp, SocialWinOpenParenText);
        _itoa(x, digits, 10);
        strcat(StringTemp, digits);
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, SocialWinCommaSpaceText);
        _itoa(y, digits, 10);
        strcat(StringTemp, digits);
        strcat(StringTemp, SocialWinCloseParenText);
        MainInterfaceGlobal.set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);
        MainInterfaceGlobal.write_right_l(StringTemp, area.right - 2,
                                          area.bottom - row_pitch);
    }
    MainInterfaceGlobal.restore_clip();
    spot_.add(0, 3, MainInterfaceGlobal.status_right_rect().left,
              MainInterfaceGlobal.status_right_rect().top,
              MainInterfaceGlobal.status_right_rect().right
                  - MainInterfaceGlobal.status_right_rect().left,
              MainInterfaceGlobal.status_right_rect().bottom
                  - MainInterfaceGlobal.status_right_rect().top);
    MainInterfaceGlobal.soft_update(
        const_cast<RECT *>(&MainInterfaceGlobal.status_right_rect()));
    hover_base_id_ = -1;
    hover_vehicle_id_ = -1;
    field_15CC_ = veh;
    field_15D0_ = sel_base;
    draw_active(veh, sel_base, 1);

    area = MainInterfaceGlobal.status_hit_rect();
    MainInterfaceGlobal.box_sprite(&area, &InterfaceBoxStyles[21]);
    area.left += 3;
    area.right -= 3;
    area.top += 3;
    area.bottom -= 3;
    MainInterfaceGlobal.box_sprite(&area, &InterfaceBoxStyles[18]);
    area.left += 3;
    area.right -= 3;
    area.top += 3;
    area.bottom -= 3;
    if (!GameDataStateStorage.fields_.turn_current_num_) {
        return;
    }
    int item_width = (MapWinCaptionScale > 0
        ? MapWinCaptionScale * 100 / MapWinCaptionDivisor : 50) + 11;
    row = MainInterfaceGlobal.status_hit_rect().bottom
        - MainInterfaceGlobal.status_hit_rect().top - 12;
    int count = 0;
    int base_id = base_at(x, y);
    if (base_id >= 0) {
        if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            || Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
            || (Bases[base_id].visibility & (1 << ConsoleGlobal.faction_id_))) {
            count = 1;
        } else {
            base_id = -1;
        }
    }
    for (int stacked = veh_top(veh); stacked >= 0;
         stacked = veh_next_stack_id(stacked)) {
        count++;
    }
    col = (area.right - area.left) / (item_width + 5);
    if (count <= col) {
        MainInterfaceGlobal.flat_button(8).hide();
        MainInterfaceGlobal.flat_button(9).hide();
        MainInterfaceGlobal.flat_button(10).hide();
    } else {
        area.left = MainInterfaceGlobal.stack_button_rect().right + 1;
        area.right = MainInterfaceGlobal.adjacent_panel_right_rect().left - 1;
        item_width = (area.right - area.left) / col - 5;
        MainInterfaceGlobal.flat_button(8).show(0);
        MainInterfaceGlobal.flat_button(10).show(0);
    }
    int item_x = area.left;
    int item_y = area.top;
    max_width = item_x + item_width;
    area.right = max_width;
    area.left -= 2;
    area.right += 2;
    area.top -= 2;
    area.bottom = item_y + row + 2;
    MainInterfaceGlobal.set_clip(&area);
    int drawn = 0;
    area.left += 2;
    area.right -= 2;
    area.top += 2;
    area.bottom -= 2;
    if (base_id >= 0) {
        drawn = 1;
        base_draw(&MainInterfaceGlobal, base_id, item_x + 11, item_y - 1, -8, 0x40);
        base_draw(&MainInterfaceGlobal, base_id, item_x, item_y - 1, 0, 0x1800);
        RECT spot_rect = area;
        spot_rect.right += 5;
        spot_.add(base_id, 11, &spot_rect);
        if (base_id == sel_base) {
            area.left -= 2;
            area.right += 2;
            area.top -= 2;
            area.bottom += 2;
            MainInterfaceGlobal.box(&area, 0xae, 0xae);
            area.left += 2;
            area.right -= 2;
            area.top += 2;
            area.bottom -= 2;
        }
        area.left += item_width + 5;
        area.right += item_width + 5;
        item_x = max_width + 5;
    }
    if (!MainInterfaceStatusRedrawFlag && veh >= 0) {
        stack_sort_2(veh);
    }
    for (int v = veh_top(veh); v >= 0 && drawn < col;
         v = veh_next_stack_id(v), drawn++) {
        area.left -= 2;
        area.top -= 2;
        area.right += 2;
        area.bottom += 2;
        MainInterfaceGlobal.set_clip(&area);
        area.left += 2;
        area.top += 2;
        area.right -= 2;
        area.bottom -= 2;
        veh_draw(&MainInterfaceGlobal, v, item_x + 11, item_y - 1, -8, 0x400, 0);
        veh_draw(&MainInterfaceGlobal, v, item_x, item_y - 1, 0, 0x1800, 0);
        MainInterfaceGlobal.set_font(&font1_, 0, 0, 0);
        StringTemp[0] = '\0';
        if (Vehs[v].order >= 4 && Vehs[v].order < 0x18
            && (Vehs[v].faction_id == ConsoleGlobal.faction_id_
                || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
                || (PlayersData[ConsoleGlobal.faction_id_]
                        .diplo_treaties[Vehs[v].faction_id] & 1))) {
            int turns = action_terraform(v, Vehs[v].order - 4, 0);
            _itoa(turns + 1, digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, BufferJoinSpace);
            // 0x004B85E5: two branch loads joined before one get call.
            strcat(StringTemp, StringTable.get(turns != 1 ? Labels.strings_ptr[0xc4 / 4]
                                                          : Labels.strings_ptr[0xc0 / 4]));
            MainInterfaceGlobal.set_text_color(0xfb, 3, 1, 1);
        } else {
            say_stats_3(StringTemp, Vehs[v].proto_id);
            if (Vehs[v].state & VSTATE_DESIGNATE_DEFENDER) {
                strcat(StringTemp, "#");
            }
            if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR)
                && (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE)) {
                char id_digits[80];
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, SocialWinOpenParenText);
                _itoa(v, id_digits, 10);
                strcat(StringTemp, id_digits);
                strcat(StringTemp, SocialWinCloseParenText);
            }
            MainInterfaceGlobal.set_text_color(TextColourTable[20], TextColourTable[21], 1, 1);
        }
        MainInterfaceGlobal.write_cent_text(StringTemp, item_x,
                                            font1_.descent() - row_pitch + area.bottom + 1,
                                            item_width);
        if (v == veh && iCityIndex_ < 0) {
            area.left -= 2;
            area.right += 2;
            area.top -= 2;
            area.bottom += 2;
            MainInterfaceGlobal.box(&area, 0xae, 0xae);
            area.left += 2;
            area.right -= 2;
            area.top += 2;
            area.bottom -= 2;
        }
        RECT spot_rect = area;
        spot_rect.right += 5;
        spot_.add(v, 0, &spot_rect);
        item_x += item_width + 5;
        area.left += item_width + 5;
        area.right += item_width + 5;
    }
    find_font(0x14, 0);
    MainInterfaceGlobal.soft_update(
        const_cast<RECT *>(&MainInterfaceGlobal.status_hit_rect()));
    MainInterfaceGlobal.restore_clip();
}

// ORIGINAL: 0x004B8830 ?draw@StatusWin@@QAEXHHHHH@Z 0x004B8830-0x004B888A BYTE_EXACT
// LEVER: 2026-09-05 homed from 004b8830.cpp. UnionRect is spelled as the plain
//   Win32 call, NOT the archive's local function-pointer variable holding
//   `*0x669328`: that address is UnionRect's own import slot, VC6 reads it
//   into esi and calls it twice, and an ordinary call is what builds that.
//   redraw() (above) already reproduces the identical tail byte-exactly.
// size      90 bytes
// prototype void (__thiscall ?draw@StatusWin@@QAEXHHHHH@Z)(StatusWin* this, int, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B6570 0x005D5930
// indirect  0x004B8864 0x004B8873
void StatusWin::draw(int tile_x, int tile_y, int city_index, int unit_index,
                     int mode) {
    RECT localRect;
    draw_status(tile_x, tile_y, city_index, unit_index, mode);
    UnionRect(&localRect, &MainInterfaceGlobal.status_hit_rect(),
              &MainInterfaceGlobal.status_right_rect());
    UnionRect(&localRect, &localRect, &MainInterfaceGlobal.status_left_rect());
    MainInterfaceGlobal.soft_update(&localRect);
}

// ORIGINAL: 0x004B4CF0 ?draw_active@StatusWin@@QAEXHHH@Z 0x004B4CF0-0x004B652B
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004b4cf0.cpp retained full transcription and original span", "MainInterfaceGlobal Buffer and status_left_rect_ layout", "Veh and Base records, destination state and box styles in source"], "reason": "Home the active-status panel's frame, unit/base identity display, selection spots and destination lifecycle in their real owners; retain omitted detail branches as restoration obligations."}
// TRIED: full-function transcription (~1843 instructions, sim 0.46 vs original at /O2 /Oy-); diverges at instr #3. Both the vehicle- display and base-display halves are transcribed from Ghidra with the Buffer receiver confirmed as the fixed global g_007aec64 (checked against raw disasm at two call sites); the repeated chassis-speed/range mini-blocks were copied by pattern from the first verified instance rather than each re-checked individually against the disassembly.
// The retained seed is the restoration reference for the colour palette,
// punctuation and disputed row layout. Compiling this body does not establish
// exact display behavior or byte restoration.
void StatusWin::draw_active(int vehicle_id, int base_id, int add_spot) {
    char digits[80];
    const int row_pitch = font_line_advance(font1_);
    MainInterfaceGlobal.set_font(&font1_, &font3_, &font2_, 0);

    // The image copies the left panel rectangle, draws two inset frames,
    // then reserves another three pixels on every side for active content.
    RECT area = MainInterfaceGlobal.status_left_rect();
    MainInterfaceGlobal.box_sprite(&area, &InterfaceBoxStyles[21]);
    area.left += 3;
    area.right -= 3;
    area.top += 3;
    area.bottom -= 3;
    MainInterfaceGlobal.box_sprite(&area, &InterfaceBoxStyles[18]);
    area.left += 3;
    area.right -= 3;
    area.top += 3;
    area.bottom -= 3;
    const int content_x = area.left;
    const int content_y = area.top;
    int line_y;
    int next_x;

    if (base_id < 0 && vehicle_id >= 0) {
        MainInterfaceGlobal.set_clip(&area);
        Buffer *buffer = &MainInterfaceGlobal;
        veh_draw(buffer, vehicle_id,
                 (area.right - (MapWinCaptionScale > 0
                     ? MapWinCaptionScale * 100 / MapWinCaptionDivisor : 75)
                  - area.left) / 2 + content_x,
                 content_y, -4, 0x48, 0);
        veh_draw(&MainInterfaceGlobal, vehicle_id, content_x, content_y, 0,
                 0x1800, 0);
        MainInterfaceGlobal.set_font(&font1_, 0, 0, 0);
        line_y = (MapWinCaptionScale > 0
            ? MapWinCaptionScale * 75 / MapWinCaptionDivisor : 56) + content_y + 8;
        StringTemp[0] = '\0';
        strcat(StringTemp, VehPrototypes[Vehs[vehicle_id].proto_id].veh_name);
        if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) &&
            (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE)) {
            strcat(StringTemp, SocialWinOpenParenText);
            _itoa(vehicle_id, digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, SocialWinCloseParenText);
        }
        MainInterfaceGlobal.set_text_color(TextColourTable[12], TextColourTable[13], 1, 1);
        line_y = MainInterfaceGlobal.wrap_cent(&font3_, StringTemp, content_x, line_y,
                                   area.right - area.left);
        MainInterfaceGlobal.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
        if (Vehs[vehicle_id].faction_id != 0 &&
            Vehs[vehicle_id].faction_id != ConsoleGlobal.faction_id_) {
            StringTemp[0] = '\0';
            if (Language == 0) {
                strcat(StringTemp,
                       Players[Vehs[vehicle_id].faction_id].adj_name_faction);
            } else {
                strcat(StringTemp, SocialWinOpenParenText);
                parse_set(Players[Vehs[vehicle_id].faction_id].noun_gender,
                          Players[Vehs[vehicle_id].faction_id].is_noun_plural);
                strcat(StringTemp,
                       Players[Vehs[vehicle_id].faction_id].noun_faction);
                strcat(StringTemp, SocialWinCloseParenText);
            }
            line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                       area.right - area.left);
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, SocialWinOpenParenText);
        say_stats_3(StringTemp, Vehs[vehicle_id].proto_id);
        if (Vehs[vehicle_id].state & VSTATE_DESIGNATE_DEFENDER) {
            strcat(StringTemp, "#");
        }
        for (int ability = 0; ability < MaxAbilityNum; ++ability) {
            if (has_abil(Vehs[vehicle_id].proto_id, 1 << ability)
                && strlen(StringTable.get(Ability[ability].abbreviation))) {
                strcat(StringTemp, SocialWinCommaSpaceText);
                strcat(StringTemp, StringTable.get(Ability[ability].abbreviation));
            }
        }
        strcat(StringTemp, SocialWinCloseParenText);
        line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                   area.right - area.left);
        if ((Vehs[vehicle_id].faction_id != 0
             && VehPrototypes[Vehs[vehicle_id].proto_id].plan < 8)
            || (VehPrototypes[Vehs[vehicle_id].proto_id].plan == PLAN_INFO_WARFARE
                && morale_veh(vehicle_id, 1, 0) > 1)) {
            StringTemp[0] = '\0';
            say_morale(StringTemp, vehicle_id, 0);
            if (Vehs[vehicle_id].faction_id != ConsoleGlobal.faction_id_
                && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
                line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                           area.right - area.left);
            } else {
                line_y = MainInterfaceGlobal.wrap(StringTemp, content_x, line_y,
                                      area.right - area.left);
            }
        }
        if (Vehs[vehicle_id].faction_id == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
            int home_base_id = Vehs[vehicle_id].home_base_id;
            StringTemp[0] = '\0';
            if (home_base_id < 0) {
                strcat(StringTemp, label_get(0x308 / 4));
            } else {
                strcat(StringTemp, Bases[home_base_id].name_string);
            }
            line_y = MainInterfaceGlobal.wrap(StringTemp, content_x, line_y,
                                  area.right - area.left);
            if (Vehs[vehicle_id].faction_id == ConsoleGlobal.faction_id_) {
                if (!veh_moves(vehicle_id)) {
                    StringTemp[0] = '\0';
                    strcat(StringTemp, label_get(0x9e4 / 4));
                    MainInterfaceGlobal.set_text_color(0xf9, -1, 1, 1);
                    line_y = MainInterfaceGlobal.wrap(StringTemp, content_x, line_y,
                                          area.right - area.left);
                } else {
                    StringTemp[0] = '\0';
                    strcat(StringTemp, label_get(0x1c / 4));
                    strcat(StringTemp, SocialWinColonSpaceText);
                    int moves = veh_moves(vehicle_id);
                    int fraction = moves % Rules.move_rate_roads;
                    _itoa(moves / Rules.move_rate_roads, digits, 10);
                    strcat(StringTemp, digits);
                    if (fraction) {
                        strcat(StringTemp, BufferJoinSpace);
                        _itoa(fraction, digits, 10);
                        strcat(StringTemp, digits);
                        strcat(StringTemp, "/");
                        _itoa(Rules.move_rate_roads, digits, 10);
                        strcat(StringTemp, digits);
                    }
                    line_y = MainInterfaceGlobal.wrap(StringTemp, content_x, line_y,
                                          area.right - area.left);
                }
            }
            if (Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0x30c / 4));
                strcat(StringTemp, SocialWinColonSpaceText);
                next_x = MainInterfaceGlobal.write_l(StringTemp, content_x, line_y);
                StringTemp[0] = '\0';
                int moves = veh_moves(vehicle_id);
                int fuel = (speed(vehicle_id, 0)
                    * (Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range
                       + (moves == 0) - Vehs[vehicle_id].terraforming_turns)
                    - Vehs[vehicle_id].moves_expended) / Rules.move_rate_roads;
                _itoa(fuel, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, "/");
                fuel = speed(vehicle_id, 0)
                    * Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range
                    / Rules.move_rate_roads;
                _itoa(fuel, digits, 10);
                strcat(StringTemp, digits);
                MainInterfaceGlobal.set_text_color(0xfa, -1, 1, 1);
                if (Vehs[vehicle_id].terraforming_turns
                    && Vehs[vehicle_id].terraforming_turns
                        >= Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range / 2) {
                    MainInterfaceGlobal.set_text_color(0xfb, -1, 1, 1);
                }
                if (Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range == 1
                    && veh_moves(vehicle_id) <= speed(vehicle_id, 0) / 2) {
                    MainInterfaceGlobal.set_text_color(0xfb, -1, 1, 1);
                }
                if (action_home(vehicle_id, -1)) {
                    MainInterfaceGlobal.set_text_color(0xf9, -1, 1, 1);
                }
                MainInterfaceGlobal.write_l(StringTemp, next_x, line_y);
                line_y += row_pitch;
            }
            MainInterfaceGlobal.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
            if (veh_cargo(vehicle_id)) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0x360 / 4));
                strcat(StringTemp, SocialWinColonSpaceText);
                int cargo = veh_cargo(vehicle_id) - stack_veh(vehicle_id, 0);
                _itoa(cargo, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, "/");
                cargo = veh_cargo(vehicle_id);
                _itoa(cargo, digits, 10);
                strcat(StringTemp, digits);
                MainInterfaceGlobal.write_l(StringTemp, content_x, line_y);
                line_y += row_pitch;
            }
        }
        if (Vehs[vehicle_id].dmg_incurred) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0x628 / 4));
            strcat(StringTemp, SocialWinColonSpaceText);
            next_x = MainInterfaceGlobal.write_l(StringTemp, content_x, line_y);
            MainInterfaceGlobal.set_text_color(0xfa, -1, 1, 1);
            if (Vehs[vehicle_id].dmg_incurred >= proto_power(vehicle_id) / 3) {
                MainInterfaceGlobal.set_text_color(0xfb, -1, 1, 1);
            }
            if (Vehs[vehicle_id].dmg_incurred >= proto_power(vehicle_id) * 2 / 3) {
                MainInterfaceGlobal.set_text_color(0xf9, -1, 1, 1);
            }
            StringTemp[0] = '\0';
            _itoa((proto_power(vehicle_id)
                   - range(proto_power(vehicle_id) - Vehs[vehicle_id].dmg_incurred,
                           0, 9999)) * 100 / proto_power(vehicle_id),
                  digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, "%");
            MainInterfaceGlobal.write_l(StringTemp, next_x, line_y);
            MainInterfaceGlobal.set_text_color(MessageWinTextColor, MessageWinTextBackground, 1, 1);
            line_y += row_pitch;
        }
        if ((Vehs[vehicle_id].faction_id == ConsoleGlobal.faction_id_
             || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
             || (PlayersData[ConsoleGlobal.faction_id_]
                     .diplo_treaties[Vehs[vehicle_id].faction_id] & 1))
            && (Vehs[vehicle_id].state & VSTATE_DESIGNATE_DEFENDER)) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0x1048 / 4));
            MainInterfaceGlobal.set_text_color(0xfb, -1, 1, 1);
            MainInterfaceGlobal.write_l(StringTemp, content_x, line_y);
            line_y += row_pitch;
        }
        MainInterfaceGlobal.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
        if (Vehs[vehicle_id].faction_id == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            || (PlayersData[ConsoleGlobal.faction_id_]
                    .diplo_treaties[Vehs[vehicle_id].faction_id] & 1)) {
            StringTemp[0] = '\0';
            say_orders(StringTemp, vehicle_id);
            line_y = MainInterfaceGlobal.wrap(StringTemp, content_x, line_y,
                                  area.right - area.left);
            if (Vehs[vehicle_id].order >= 4 && Vehs[vehicle_id].order < 0x18) {
                int turns = action_terraform(vehicle_id, Vehs[vehicle_id].order - 4, 0);
                StringTemp[0] = '\0';
                strcat(StringTemp, "[");
                strcat(StringTemp, label_get(0x71c / 4));
                strcat(StringTemp, SocialWinColonSpaceText);
                _itoa(turns + 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, "]");
                MainInterfaceGlobal.set_text_color(0xfb, -1, 1, 1);
                MainInterfaceGlobal.write_l(StringTemp, content_x, line_y);
            }
        }
        if (add_spot) {
            spot_.add(vehicle_id, 10, area.left, area.top,
                      area.right - area.left, area.bottom - area.top);
        }
        base_id = -1;
        if ((Vehs[vehicle_id].faction_id == ConsoleGlobal.faction_id_
             || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW))
            && Vehs[vehicle_id].order == 0x18
            && !(GameDataStateStorage.fields_.game_more_preferences_
                 & MPREF_MAP_HIDE_ACTIVE_VEH_GOTO_PATH)) {
            draw_my_dest(vehicle_id);
            ConsoleGlobal.cover_window_.Win::show(0);
        } else {
            UNK2();
        }
    } else {
        UNK2();
    }

    MainInterfaceGlobal.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
    MainInterfaceGlobal.restore_clip();
    if (base_id >= 0) {
        // 0x4B5DFF/0x4B5E54: each base_draw's Buffer* conversion (neg/sbb/and)
        // is computed before the caption-scale ternary, as the vehicle half's
        // first veh_draw does with its named pointer: a fresh named value per
        // call, not the argument expression.
        Buffer *buffer = &MainInterfaceGlobal;
        base_draw(buffer, base_id,
                  (area.right - (MapWinCaptionScale > 0
                      ? MapWinCaptionScale * 100 / MapWinCaptionDivisor : 75)
                   - area.left) / 2 + content_x,
                  content_y, -4, 0x40);
        buffer = &MainInterfaceGlobal;
        base_draw(buffer, base_id,
                  (area.right - (MapWinCaptionScale > 0
                      ? MapWinCaptionScale * 100 / MapWinCaptionDivisor : 75)
                   - area.left) / 2 + content_x,
                  content_y, 0, 0x1800);
        MainInterfaceGlobal.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
        MainInterfaceGlobal.set_font(&font1_, 0, 0, 0);
        line_y = (MapWinCaptionScale > 0
            ? MapWinCaptionScale * 75 / MapWinCaptionDivisor : 56) + content_y + 8;
        StringTemp[0] = '\0';
        strcat(StringTemp, Bases[base_id].name_string);
        if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) &&
            (GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE)) {
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, SocialWinOpenParenText);
            _itoa(base_id, digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, SocialWinCloseParenText);
        }
        line_y = MainInterfaceGlobal.wrap_cent(&font3_, StringTemp, content_x, line_y,
                                   area.right - area.left);
        if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_) {
            StringTemp[0] = '\0';
            if (Language == 0) {
                strcat(StringTemp,
                       Players[Bases[base_id].faction_id_current].adj_name_faction);
            } else {
                strcat(StringTemp, SocialWinOpenParenText);
                parse_set(Players[Bases[base_id].faction_id_current].noun_gender,
                          Players[Bases[base_id].faction_id_current].is_noun_plural);
                strcat(StringTemp,
                       Players[Bases[base_id].faction_id_current].noun_faction);
                strcat(StringTemp, SocialWinCloseParenText);
            }
            line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                       area.right - area.left);
            if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_) {
                StringTemp[0] = '\0';
            } else {
                strcat(StringTemp, BufferJoinSpace);
            }
            strcat(StringTemp, has_fac(FAC_HEADQUARTERS, base_id, 0)
                ? label_get(0x5c4 / 4)
                : Bases[base_id].population_size <= 1 ? label_get(0x2f0 / 4)
                : Bases[base_id].population_size <= Rules.pop_limit_sans_hab_complex
                    ? label_get(0x2f4 / 4) : label_get(0x570 / 4));
            line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                       area.right - area.left);
            StringTemp[0] = '\0';
            if (has_fac(FAC_TACHYON_FIELD, base_id, 0)) {
                strcat(StringTemp, StringTable.get(Facility[FAC_TACHYON_FIELD].name));
            } else if (has_fac(FAC_PERIMETER_DEFENSE, base_id, 0)) {
                strcat(StringTemp, StringTable.get(Facility[FAC_PERIMETER_DEFENSE].name));
            }
            if (strlen(StringTemp)) {
                line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                           area.right - area.left);
            }
            if (Bases[base_id].state & BSTATE_DRONE_RIOTS_ACTIVE) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0xb04 / 4));
                MainInterfaceGlobal.set_text_color(0xf9, -1, 1, 1);
                line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                           area.right - area.left);
            }
            if (Bases[base_id].nerve_staple_turns_left) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0xb08 / 4));
                strcat(StringTemp, SocialWinColonSpaceText);
                _itoa(Bases[base_id].nerve_staple_turns_left, digits, 10);
                strcat(StringTemp, digits);
                MainInterfaceGlobal.set_text_color(0xfb, -1, 1, 1);
                line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                           area.right - area.left);
            }
        }
        MainInterfaceGlobal.set_text_color(TextColourTable[14], TextColourTable[15], 1, 1);
        if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            || (PlayersData[ConsoleGlobal.faction_id_]
                    .diplo_treaties[Bases[base_id].faction_id_current] & 1)
            || spying(Bases[base_id].faction_id_current)) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0x798 / 4));
            strcat(StringTemp, SocialWinColonSpaceText);
            int cost;
            if (Bases[base_id].queue_production_id[0] >= 0) {
                strcat(StringTemp,
                       VehPrototypes[Bases[base_id].queue_production_id[0]].veh_name);
                cost = VehPrototypes[Bases[base_id].queue_production_id[0]].cost;
            } else {
                strcat(StringTemp, StringTable.get(
                    Facility[-Bases[base_id].queue_production_id[0]].name));
                cost = Facility[-Bases[base_id].queue_production_id[0]].cost;
            }
            if (Bases[base_id].queue_production_id[0] != -69) {
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, SocialWinOpenParenText);
                _itoa(Bases[base_id].minerals_accumulated, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, "/");
                cost *= cost_factor(Bases[base_id].faction_id_current, 1, -1);
                _itoa(cost, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SocialWinCloseParenText);
            }
            if (strlen(StringTemp)) {
                line_y = MainInterfaceGlobal.wrap_cent(StringTemp, content_x, line_y,
                                           area.right - area.left);
            }
        }
        if (add_spot) {
            spot_.add(base_id, 1, area.left, area.top,
                      area.right - area.left, area.bottom - area.top);
        }
    }
    if (!add_spot) {
        MainInterfaceGlobal.update(&area, static_cast<GraphicWin *>(0));
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004B8E40 0x004B8FD4 0x004B9004 0x004B8EB9
// ORIGINAL: 0x004B8E40 ?on_left_click@StatusWin@@QAEXHH@Z 0x004B8E40-0x004B8FD3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004b8e40.cpp", "StatusWin spot, click and toggle members in src/statuswin.h", "Console, NetDaemon, Bases and PlayersData owners in src/"], "reason": "Recover the click branches through owned C++ objects and preserve the historical original span."}
void StatusWin::on_left_click(int x, int y) {
    if (fInOnClick_ == 0) {
        ConsoleInfoWin.reset();
        if (field_15D4_ != 0) {
            field_15D4_ = 0;
            g_BattleWin.release_iface_mode();
        }
        TutWinBaseWindow.cancel_zoom_wait();

        int veh_id;
        int kind;
        if (spot_.check(x, y, &veh_id, &kind) < 0) {
            return;
        }
        switch (kind) {
        case 0:
        case 10:
            if (veh_selectable(veh_id)) {
                ConsoleGlobal.selected_vehicle_ = veh_id;
                if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) == 0) {
                    veh_wake(veh_id);
                    synch_veh(veh_id);
                    NetDaemonNet.await_synch();
                    NetDaemonNet.unlock_veh();
                }
                ConsoleGlobal.set_move(1);
                return;
            }
            break;
        case 1:
        case 11: {
            // The image copies the base id into edi and re-reads the owner
            // byte at each use (0x004B8F3E, 0x004B8F64): no owner local.
            int base_id = veh_id;
            if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0
                || Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
                || spying(Bases[base_id].faction_id_current)
                || (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[Bases[base_id].faction_id_current] & 1) != 0) {
                TutWinBaseWindow.zoom(base_id, 0);
                return;
            }
            break;
        }
        case 3:
            field_15A8_ = ~field_15A8_;
            draw_status(-1, -1, -1, -1, 0);
            return;
        // Explicit empty cases: the image's 12-entry jump table at 0x004B8FD4
        // is direct; without them VC6 emits a byte index table instead.
        case 2:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            return;
        }
    }
}

// ORIGINAL: 0x004B4A20 ?set_title@StatusWin@@QAEXH@Z 0x004B4A20-0x004B4BE3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004b4a20.cpp", "StringTemp, label_get and vehicle/console declarations in src/", "StatusWin tile members in src/statuswin.h"], "reason": "Recover title assembly using source-owned string, label, console and vehicle state."}
#pragma optimize("y", off)
void StatusWin::set_title(int title_id) {
    const unsigned int flags = static_cast<unsigned int>(title_id);
    StringTemp[0] = '\0';

    if (flags != 0) {
        if (flags & 1) {
            strcat(StringTemp, label_get(0x6B4 / 4));
        } else if (flags & 0x10) {
            strcat(StringTemp, label_get(0x6F8 / 4));
        } else if (flags & 8) {
            strcat(StringTemp, label_get(0x6FC / 4));
        } else if (flags & 2) {
            strcat(StringTemp, label_get(0x6B8 / 4));
        } else {
            strcat(StringTemp, label_get(0x250 / 4));
        }
        return;
    }

    strcat(StringTemp, label_get(0x8C / 4));
    if (GameDataStateStorage.fields_.turn_current_num_ != 0) {
        strcat(StringTemp, SocialWinColonSpaceText);
        int x;
        int y;
        if (ConsoleGlobal.cursor_active_ == 0) {
            strcat(StringTemp, label_get(0xC / 4));
            x = iTileX_;
            y = iTileY_;
        } else {
            strcat(StringTemp, label_get(8 / 4));
            if (ConsoleGlobal.selected_vehicle_ < 0) {
                return;
            }
            strcat(StringTemp, BufferJoinSpace);
            x = Vehs[ConsoleGlobal.selected_vehicle_].x;
            y = Vehs[ConsoleGlobal.selected_vehicle_].y;
            strcat(StringTemp,
                   VehPrototypes[Vehs[ConsoleGlobal.selected_vehicle_].proto_id].veh_name);
            strcat(StringTemp, BufferJoinSpace);
        }
        strcat(StringTemp, BufferJoinSpace);
        say_loc(StringTemp, x, y, ConsoleGlobal.cursor_active_,
                ConsoleGlobal.cursor_active_ == 0, ConsoleGlobal.cursor_active_);
    }
}
#pragma optimize("", on)

// ORIGINAL: 0x004B4970 ?say_orders@@YAXH@Z 0x004B4970-0x004B4987 BYTE_EXACT
// LEVER: 2026-09-05 homed from 004b4970.cpp. StringTemp (strings.h,
//   0x009B86A0) is the buffer the image pushes, so `push 0x9b86a0` is the
//   array's own address rather than a bound pointer's load.
// size      22 bytes
// prototype void (__cdecl ?say_orders@@YAXH@Z)(int vehID)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B43E0
void say_orders(int veh_id) {
    say_orders(StringTemp, veh_id);
}

static const char *vehicle_order_rule_text(LPSTR encoded_id) {
    return StringTable.get(encoded_id);
}

// has_fac_built_call() (real bitmask() call, general.h bitmask_call) is now
// declared in base.h, which this file already includes - no local
// redeclaration needed.

/*
Purpose: Generate an output string for the specified unit's morale.
// ORIGINAL: 0x004B3FD0 ?say_morale@@YAXPADHH@Z 0x004B3FD0-0x004B43BF BYTE_EXACT
// LEVER: std::string -> strcat, matching the say_stats family (calls went
//        from 16, mostly basic_string internals, to 10; strcat count now
//        exactly 7, matching the image). TRIED: crèche/brood-pit was
//        already the correct asymmetric if/else-if here, unlike
//        get_basic_offense - no change needed.
// LEVER: the ternary feeding the first strcat's `Strings::get()` argument
//        rewritten as an if/else with the strcat duplicated into each arm -
//        the image has 2 static call sites (one per branch) and the ternary
//        let the compiler merge them to 1. call_diff went from 10 (image 14)
//        to 11; best flag set's exact-instruction count nearly doubled,
//        13/313 -> 26/313 at /O2 /GR- /Oy- /GX.
// TRIED: `bitmask_call` forwarder for the crèche/brood-pit
//        `has_fac_built()` sites (general.h, the same lever that helped
//        base.cpp's pop_goal) - tried on the sibling morale_veh (0x5C0E40)
//        first and reverted there, similarity fell at every flag set; not
//        retried here on that evidence. Still open: the same bitmask() x3
//        auto-inline wall (see get_basic_offense, 0x5015B0) accounts for the
//        remaining call-count gap (11 vs 14: 3x bitmask via has_fac_built).
// size      1007 bytes
// prototype void (__cdecl ?say_morale@@YAXPADHH@Z)(int8* moraleOutput, int vehID, int factionIDvsNative)
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050BA00 0x005C0E40 0x006169A0 0x00645470
Return Value: n/a
Status: Complete
*/
void __cdecl say_morale(LPSTR morale_output, int veh_id, int faction_id_vs_native) {
    // TODO: Look into inconsistencies related to morale display
    int morale = morale_veh(veh_id, false, faction_id_vs_native);
    int faction_id = Vehs[veh_id].faction_id;
    // LEVER: std::string -> raw strcat() straight onto morale_output, per
    // the say_stats family above. The image's own `calls` list is 7 strcat
    // sites, not a string builder: this fill, the drone-riot " (-)", " "
    // and "(" as TWO separate strcat calls (not one " (" literal), the
    // "+" loop body, ")", and the designate-defender "(d)".
    if (Vehs[veh_id].proto_id < MaxVehProtoFactionNum &&
        (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0 || Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER)) {
        char *name = Morale[morale].name_lifecycle;
        strcat(morale_output, StringTable.get(name));
    } else {
        char *name = Morale[morale].name;
        strcat(morale_output, StringTable.get(name));
    }
    if (VehPrototypes[Vehs[veh_id].proto_id].plan < PLAN_COLONIZATION) {
        int morale_penalty = 0;
        int home_base_id = Vehs[veh_id].home_base_id;
        if (home_base_id >= 0 && Bases[home_base_id].state & BSTATE_DRONE_RIOTS_ACTIVE && morale > 0
            && !(Players[faction_id].rule_flags & RFLAG_MORALE)) {
            strcat(morale_output, " (-)");
            morale_penalty = 1;
        }
        VehMoraleModifierCount = 0;
        int base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        if (base_id >= 0 && morale < 6 && has_fac(FAC_CHILDREN_CRECHE, base_id, 0)) {
            VehMoraleModifierCount++;
            int morale_active = range(PlayersData[faction_id].soc_effect_active.values_[SocialEffect::MORALE], -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            if (morale_active < 0) {
                int morale_cap = morale + VehMoraleModifierCount;
                while (1) {
                    if (morale_cap >= 6) {
                        break;
                    }
                    VehMoraleModifierCount++;
                    morale_cap++;
                    if (++morale_active >= 0) {
                        break;
                    }
                }
            }
            if(has_fac(FAC_BROOD_PIT, base_id, 0) && Vehs[veh_id].proto_id < MaxVehProtoFactionNum
                && (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0 || Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER)) {
                VehMoraleModifierCount++;
            }
        } else if (base_id >= 0 && morale < 6 && has_fac(FAC_BROOD_PIT, base_id, 0)
            && Vehs[veh_id].proto_id < MaxVehProtoFactionNum
            && (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0 || Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER)) {
            VehMoraleModifierCount++;
            int morale_active = range(PlayersData[faction_id].soc_effect_active.values_[SocialEffect::MORALE], -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            if (morale_active < 0) {
                int morale_cap = morale + VehMoraleModifierCount;
                while (1) {
                    if (morale_cap >= 6) {
                        break;
                    }
                    VehMoraleModifierCount++;
                    morale_cap++;
                    if (++morale_active >= 0) {
                        break;
                    }
                }
            }
        }
        int morale_pending = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE];
        if (morale_pending == 2 || morale_pending == 3) {
            VehMoraleModifierCount++;
        }
        if (!morale && !VehMoraleModifierCount) {
            VehMoraleModifierCount = 1;
        }
        if(VehMoraleModifierCount) {
            strcat(morale_output, BufferJoinSpace);
            strcat(morale_output, SocialWinOpenParenText);
            for (int i = 0; i < VehMoraleModifierCount; i++) {
                strcat(morale_output, "+");
            }
            strcat(morale_output, SocialWinCloseParenText);
        }
        VehMoraleModifierCount -= morale_penalty;
        if (Vehs[veh_id].state & VSTATE_DESIGNATE_DEFENDER) {
            strcat(morale_output, "(d)");
        }
    }
    // TODO: assumes at least 1032 char nulled buffer (stringTemp), eventually remove
}

/*
Purpose: Get the morale string for the specified unit and store it in the stringTemp buffer.
// ORIGINAL: 0x004B43C0 ?say_morale@@YAXHH@Z 0x004B43C0-0x004B43DA BYTE_EXACT
// size      26 bytes
// prototype void (__cdecl ?say_morale@@YAXHH@Z)(int vehID, int factionID)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004B3FD0
Return Value: n/a
Status: Complete
*/
void __cdecl say_morale(int veh_id, int faction_id_vs_native) {
    say_morale(StringTemp, veh_id, faction_id_vs_native);
}

// ORIGINAL: 0x004B43E0 ?say_orders@@YAXPADH@Z 0x004B43E0-0x004B496D
// RECOVERY_BINDING: v1 {"evidence": ["git:038351919772808e976813a96cd7a6d820dc1450:src/recovered/units/004b43e0.cpp: complete original identity/span and historical scaffolding measurement", "Pinned4B5C17 passes StatusWin::draw_active vehicle_id to4B43E0", "96C9B0 is Order[26].order[0]; original ORDERS parser populates this shared record", "Seed g_96c878x/stride12 is Order; g_94615cx/stride8 is Resource.name_plural; g_945d48x/stride4 is parsed Compass", "recovery/tasks/say-orders-owner-oct5.json", "recovery/tests/test_say_orders_actual_source.py"], "reason": "Recover complete vehicle order text builder through genuine vehicle, base, map and parsed rules owners; replace only the old executable forwarder. Native caller passes vehicle_id, not prototype_id. Preserve original branch, buffer and index behavior; parsed Compass/Resource pointer-shaped string IDs retain their current representation pending separate type cleanup."}
// Historical seed observation: complete scaffold measured MISMATCH at3;
// its obsolete tool instructions are retired, not a current body claim.
void __cdecl say_orders(char *text, int veh_id) {
    char number[80];
    char suffix[80];
    char prefix[80];
    const uint32_t state = Vehs[veh_id].state;

    const bool prefix_state = Vehs[veh_id].patrol_current_point >= Vehs[veh_id].waypoint_count;
    if ((state & 0x00000200u) != 0
        && (state & 0x01000200u) != 0x01000200u
        && ((state & 0x02000200u) != 0x02000200u || prefix_state)) {
        ::strcat(text, SocialWinOpenParenText);
        if (Vehs[veh_id].order_auto_type == 10) {
            prefix[0] = '\0';
            ::strcat(prefix, label_get(0x6F0 / 4));
            const int base_id = base_at(Vehs[veh_id].waypoint_x[1],
                                        Vehs[veh_id].waypoint_y[1]);
            if (base_id >= 0) {
                parse_says(0, Bases[base_id].name_string, -1, -1);
                parse_string(prefix, suffix);
                ::strcat(text, suffix);
            }
        } else if (Vehs[veh_id].order_auto_type == 11) {
            ::strcat(text, label_get(0x6F4 / 4));
        } else if (VehPrototypes[Vehs[veh_id].proto_id].plan == 9
                   && Vehs[veh_id].order_auto_type != 0) {
            ::strcat(text, label_get(0x62C / 4 + Vehs[veh_id].order_auto_type));
        } else {
            ::strcat(text, label_get(0x62C / 4));
        }
        ::strcat(text, SocialWinCloseParenText);
        ::strcat(text, BufferJoinSpace);
    }

    const uint32_t state_after_prefix = Vehs[veh_id].state;
    if ((state_after_prefix & 0x00004000u) != 0) {
        ::strcat(text, vehicle_order_rule_text(Order[26].order[0]));
        return;
    }

    if ((state_after_prefix & 0x01000200u) == 0x01000200u) {
        ::strcat(text, label_get(0x65C / 4));
        ::strcat(text, SocialWinColonSpaceText);
        for (int index = 0; index < Vehs[veh_id].waypoint_count + 1; ++index) {
            say_loc(text, Vehs[veh_id].waypoint_x[index],
                    Vehs[veh_id].waypoint_y[index], 0, 2, 0);
            if (index < Vehs[veh_id].waypoint_count) {
                ::strcat(text, BufferJoinSpace);
                ::strcat(text, label_get(0x660 / 4));
                ::strcat(text, BufferJoinSpace);
            }
        }
        return;
    }

    // The sea flag indexes the order's land/sea label pair directly.
    const int tile_index = Vehs[veh_id].y * MapLongitude
                        + (Vehs[veh_id].x >> 1);
    const int sea = (MapTiles[tile_index].climate & ~0x1F) < 0x60;
    ::strcat(text, vehicle_order_rule_text(Order[Vehs[veh_id].order & 0x3F].order[sea]));

    if (Vehs[veh_id].order == ORDER_CONVOY) {
        ::strcat(text, BufferJoinSpace);
        char *name = Resource[Vehs[veh_id].order_auto_type].name_plural;
        ::strcat(text, StringTable.get(name));
        const int base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        if (base_id >= 0) {
            ::strcat(text, BufferJoinSpace);
            ::strcat(text, label_get(0x6C8 / 4));
            ::strcat(text, BufferJoinSpace);
            say_base(text, Vehs[veh_id].home_base_id);
        } else {
            ::strcat(text, SocialWinOpenParenText);
            const int yield = resource_yield(
                Vehs[veh_id].order_auto_type, Vehs[veh_id].faction_id,
                Vehs[veh_id].home_base_id, Vehs[veh_id].x, Vehs[veh_id].y);
            _itoa(yield, number, 10);
            ::strcat(text, number);
            ::strcat(text, label_get(0x5EC / 4));
            ::strcat(text, SocialWinCloseParenText);
            ::strcat(text, BufferJoinSpace);
        }
    }

    if ((static_cast<unsigned char>(Vehs[veh_id].order) & 0x3F) == ORDER_MOVE_TO) {
        ::strcat(text, BufferJoinSpace);
        say_loc(text, Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0], 0,
                ((Vehs[veh_id].state & 0x02000200u) == 0x02000200u
                 && Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count)
                    ? 2 : 1,
                0);
        if ((Vehs[veh_id].state & 0x02000200u) == 0x02000200u &&
            Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count) {
            for (int index = 0; index < Vehs[veh_id].waypoint_count; ++index) {
                ::strcat(text, BufferJoinSpace);
                ::strcat(text, label_get(0x664 / 4));
                ::strcat(text, BufferJoinSpace);
                say_loc(text, Vehs[veh_id].waypoint_x[index + 1],
                        Vehs[veh_id].waypoint_y[index + 1], 0, 2, 0);
            }
        }
    }
    if (Vehs[veh_id].order == ORDER_MOVE) {
        ::strcat(text, BufferJoinSpace);
        ::strcat(text, compass_name(Vehs[veh_id].waypoint_x[0]));
    }
    if (Vehs[veh_id].order == ORDER_ROAD_TO || Vehs[veh_id].order == ORDER_MAGTUBE_TO) {
        ::strcat(text, BufferJoinSpace);
        say_loc(text, Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0],
                0, 2, 0);
    }
}

// ORIGINAL: 0x004B6530 ?reset_active@StatusWin@@QAEXXZ 0x004B6530-0x004B6569 BYTE_EXACT
// LEVER: 2026-09-05 homed from 004b6530.cpp once hover_vehicle_id_/15C8_ became
//   signed. The guard is `a >= 0 || b >= 0`, tested with the image's own
//   `jge`/`jl` - unsigned members would give `jae`/`jb` and -1 would stop
//   meaning "nothing active". field_15CC_ is read into a local BEFORE the
//   two -1 stores, which is where the image's `mov edx,[ecx+0x15cc]` sits.
// size      57 bytes
// prototype void (__thiscall ?reset_active@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004B4CF0
void StatusWin::reset_active() {
    if (hover_vehicle_id_ >= 0 || hover_base_id_ >= 0) {
        int draw_arg = field_15CC_;
        hover_base_id_ = -1;
        hover_vehicle_id_ = -1;
        draw_active(draw_arg, field_15D0_, 0);
    }
}

// ORIGINAL: 0x004B8990 ?on_left_double_click@StatusWin@@QAEXHH@Z 0x004B8990-0x004B89BE BYTE_EXACT
// LEVER: 2026-09-05 homed from 004b8990.cpp. fInOnClick_ brackets the
//   on_left_click call, and on_left_click must stay out of line for the
//   image's `call 0x4b8e40` - it is 403 bytes with a jump table, so it never
//   inlines, but the forwarder in this file is what makes the call a real
//   member call rather than a bound pointer.
// size      46 bytes
// prototype void (__thiscall ?on_left_double_click@StatusWin@@QAEXHH@Z)(StatusWin* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B8E40
void StatusWin::on_left_double_click(int x, int y) {
    fInOnClick_ = 1;
    on_left_click(x, y);
    fInOnClick_ = 0;
}

// Fixed-slot bindings carried from 004b4990.cpp

// ORIGINAL: 0x004B4990 ?UNK1@StatusWin@@QAEXXZ 0x004B4990-0x004B4A1E FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      142 bytes
// prototype void (__thiscall ?UNK1@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00618F40

void StatusWin::UNK1() {
    int size = (WinScreenWidth != 800) ? 12 : 10;
    if (size != font_size_) {
        font_size_ = size;
        font1_.init(DefaultFontFace, size, 0);
        font2_.init(DefaultFontFace, size, 2);
        font3_.init(DefaultFontFace, size, 1);
    }
    if (WinScreenWidth != 800) {
        font1_.unk_1_ = 4;
        font2_.unk_1_ = 4;
        font3_.unk_1_ = 4;
    }
}

// Fixed-slot bindings carried from 004b8890.cpp. All of them already have
// names in the tree - DefaultFontFace and WinScreenWidth in interface_fonts.h/win.h, the
// three redraw RECTs in statuswin.h, MainInterfaceGlobal in mapwin.h - so the
// carried lines are dropped and the uses read those instead.

typedef int (__stdcall *UnionRectFn)(RECT *, const RECT *, const RECT *);

// ORIGINAL: 0x004B8890 ?on_redraw@StatusWin@@QAEXXZ 0x004B8890-0x004B896B FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      219 bytes
// prototype void (__thiscall ?on_redraw@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B6570 0x005D5930 0x00618F40
// indirect  0x004B8946 0x004B8955


void StatusWin::on_redraw() {
    int fontSize = ((WinScreenWidth != 0x320) ? 2 : 0) + 10;

    if (fontSize != font_size_) {
        font_size_ = fontSize;
        font1_.init(DefaultFontFace, fontSize, 0);
        font2_.init(DefaultFontFace, fontSize, 2);
        font3_.init(DefaultFontFace, fontSize, 1);
    }

    if (WinScreenWidth != 0x320) {
        font1_.unk_1_ = 4;
        font2_.unk_1_ = 4;
        font3_.unk_1_ = 4;
    }

    draw_status(-1, -1, -1, -1, 0);

    RECT rect;
    UnionRect(&rect, &MainInterfaceGlobal.status_hit_rect(),
              &MainInterfaceGlobal.status_right_rect());
    UnionRect(&rect, &rect, &MainInterfaceGlobal.status_left_rect());

    MainInterfaceGlobal.soft_update(&rect);
}

/*
// ORIGINAL: 0x004B9E90 ?UNK4@StatusWin@@QAEXXZ 0x004B9E90-0x004B9E9D BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Include the StatusFlags writeback and return in the complete original body; preserve existing behavior and historical matching notes without granting a claim.","evidence":["pinned:0x004B9E90 reads0x00939ED8, ORs bit0x20, writes back at0x004B9E97 and returns at0x004B9E9C; padding starts0x004B9E9D","source:StatusWin::UNK4 updates the source-owned StatusFlags object"]}
// TRIED: the scaffold bound the word as `static int *const g_00939ed8`, which
//        hid its meaning. It is now the source-owned StatusFlags object.
// TRIED: 2026-09-03 homing pass, 4 spellings (declared `int` storage, inline
//        pointer accessor, inline reference accessor, named const-pointer
//        binding). All four agree with the image on every mnemonic and
//        operand - `mov eax,[0x939ed8] / or al,0x20 / ...` - and all four
//        still measure MISMATCH, because in this tree the two address
//        operands are RELOCATIONS the comparison masks, leaving 0 bytes to
//        score; the image's bytes carry the literal address. Only the
//        scaffold's anonymous `g_<address>` spelling scored, and that name is
//        what the anonymous-global ceiling exists to remove. Demoted as
//        unverifiable rather than mismatches forever: the body is the
//        faithful 3 instructions.
*/
void StatusWin::UNK4() {
    StatusFlags |= 0x20;
}

/*
Purpose: Re-init the three status fonts when the screen size changed enough
         to want a different point size, force default character spacing at
         low resolution, redraw the whole status area, then invalidate the
         union of three fixed-address rects on the main interface window.
// ORIGINAL: 0x004B9EA0 ?redraw@StatusWin@@QAEXXZ 0x004B9EA0-0x004B9F7B BYTE_EXACT
// size      219 bytes
// prototype void (__thiscall ?redraw@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   17   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B6570 0x005D5930 0x00618F40
// indirect  0x004B9F56 0x004B9F65
Return Value: n/a
Status: Complete
*/
// WORKAROUND(inline): on_right_click (0x004B9010) expands this body at all
// three of its call sites, while every other caller (console, battle, mapwin)
// calls the copy at 0x004B9EA0. A plain member is not auto-inlined there
// (too large, whether defined before or after); the explicit inline member
// is, even defined here at its image position after UNK4. Measured
// 2026-11-12: a header inline is refuted, since console.cpp then expands all
// 26 of its sites. Nothing in the image needs the copy from this unit, so
// the redirect below is what emits it.
inline void StatusWin::redraw() {
    int sizeVal = (WinScreenWidth != 0x320) ? 0xC : 0xA;
    if (sizeVal != font_size_) {
        font_size_ = sizeVal;
        font1_.init(DefaultFontFace, sizeVal, 0);
        font2_.init(DefaultFontFace, sizeVal, 2);
        font3_.init(DefaultFontFace, sizeVal, 1);
    }
    if (WinScreenWidth != 0x320) {
        // font.h's `int unk_1_; // height offset? set outside of class
        // function` - and THIS IS that outside setter. The offsets are the
        // embedded Fonts' own starts (font1_ 0x1530, font2_ 0x1558, font3_
        // 0x1580), so the int written is each Font's first member.
        font1_.unk_1_ = 4;
        font2_.unk_1_ = 4;
        font3_.unk_1_ = 4;
    }
    draw_status(-1, -1, -1, -1, 0);
    RECT localRect;
    UnionRect(&localRect, &MainInterfaceGlobal.status_hit_rect(),
              &MainInterfaceGlobal.status_right_rect());
    UnionRect(&localRect, &localRect, &MainInterfaceGlobal.status_left_rect());
    MainInterfaceGlobal.soft_update(&localRect);
}

// WORKAROUND(noinline): non-original; its real call under inline_depth(0)
// emits redraw's out-of-line copy for the other units.
#pragma inline_depth(0)
void __fastcall status_win_redraw_redirect(StatusWin *self, void *) {
    self->redraw();
}
#pragma inline_depth(8)

// ORIGINAL: 0x004B8DF0 ?UNK3@StatusWin@@QAEXXZ 0x004B8DF0-0x004B8E31 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The historical body tests StatusWin offsets +0x15BC and +0x15C0; statuswin.h maps these to iUnitIndex_ and iCityIndex_.", "The store at 0x0093928C is ConsoleGlobal.selected_vehicle_ (Console offset +0x23BDC), and the remaining dispatches match veh_wake, Console::set_move, and TutWinBaseWindow.zoom.", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/004b8df0.cpp"], "reason": "Home the compact selected-unit/base action in StatusWin using the recovered members and process-owned C++ objects. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void StatusWin::UNK3() {
    const int unit_id = iUnitIndex_;
    if (unit_id >= 0) {
        veh_wake(unit_id);
        ConsoleGlobal.selected_vehicle_ = unit_id;
        ConsoleGlobal.set_move(1);
        return;
    }

    if (iCityIndex_ >= 0) {
        TutWinBaseWindow.zoom(iCityIndex_, 0);
    }
}

// Private candidate bodies for the admitted StatusWin owner. This file is a
// host-test extraction packet, not project source and carries no recovery
// annotations or historical claims.

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004B89C0 ?arrow_left@StatusWin@@QAEXXZ 0x004B89C0-0x004B8BC5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004b89c0.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_statuswin_remaining_behavior.py"], "reason": "Home the complete typed StatusWin arrow_left operation. Retain the original seed and historical obligations; this product has no new measured claim."}
void StatusWin::arrow_left() {
    if (iUnitIndex_ == -1) {
        return;
    }

    const int previous = Vehs[iUnitIndex_].prev_veh_id_stack;
    if (previous >= 0) {
        iUnitIndex_ = previous;

        const int font_height = WinScreenWidth == 0x320 ? 10 : 12;
        if (font_height != font_size_) {
            font_size_ = font_height;
            font1_.init(DefaultFontFace, font_height, 0);
            font2_.init(DefaultFontFace, font_height, 2);
            font3_.init(DefaultFontFace, font_height, 1);
        }
        if (WinScreenWidth != 0x320) {
            font1_.unk_1_ = 4;
            font2_.unk_1_ = 4;
            font3_.unk_1_ = 4;
        }

        draw_status(-1, -1, -1, -1, 0);
        RECT rect;
        UnionRect(&rect, &MainInterfaceGlobal.status_hit_rect(),
                  &MainInterfaceGlobal.status_right_rect());
        UnionRect(&rect, &rect, &MainInterfaceGlobal.status_left_rect());
        MainInterfaceGlobal.soft_update(&rect);
        return;
    } else {
        const int base_id = base_at(iTileX_, iTileY_);
        if (base_id < 0) {
            return;
        }
        iCityIndex_ = base_id;
        iUnitIndex_ = -1;

        const int font_height = WinScreenWidth == 0x320 ? 10 : 12;
        if (font_height != font_size_) {
            font_size_ = font_height;
            font1_.init(DefaultFontFace, font_height, 0);
            font2_.init(DefaultFontFace, font_height, 2);
            font3_.init(DefaultFontFace, font_height, 1);
        }
        if (WinScreenWidth != 0x320) {
            font1_.unk_1_ = 4;
            font2_.unk_1_ = 4;
            font3_.unk_1_ = 4;
        }

        draw_status(-1, -1, -1, -1, 0);
        RECT rect;
        UnionRect(&rect, &MainInterfaceGlobal.status_hit_rect(),
                  &MainInterfaceGlobal.status_right_rect());
        UnionRect(&rect, &rect, &MainInterfaceGlobal.status_left_rect());
        MainInterfaceGlobal.soft_update(&rect);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004B8BD0 ?arrow_right@StatusWin@@QAEXXZ 0x004B8BD0-0x004B8DF0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004b8bd0.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_statuswin_remaining_behavior.py"], "reason": "Home the complete typed StatusWin arrow_right operation. Retain the original seed and historical obligations; this product has no new measured claim. Historical seed obligation is carried by this product; current restoration is outstanding."}
void StatusWin::arrow_right() {
    if (iCityIndex_ >= 0) {
        const int vehicle_id = veh_at(iTileX_, iTileY_);
        if (field_15DC_ != 0 && vehicle_id >= 0) {
            iCityIndex_ = -1;
            iUnitIndex_ = vehicle_id;

            const int font_height = WinScreenWidth == 0x320 ? 10 : 12;
            if (font_height != font_size_) {
                font_size_ = font_height;
                font1_.init(DefaultFontFace, font_height, 0);
                font2_.init(DefaultFontFace, font_height, 2);
                font3_.init(DefaultFontFace, font_height, 1);
            }
            if (WinScreenWidth != 0x320) {
                font1_.unk_1_ = 4;
                font2_.unk_1_ = 4;
                font3_.unk_1_ = 4;
            }

            draw_status(-1, -1, -1, -1, 0);
            RECT rect;
            UnionRect(&rect, &MainInterfaceGlobal.status_hit_rect(),
                      &MainInterfaceGlobal.status_right_rect());
            UnionRect(&rect, &rect, &MainInterfaceGlobal.status_left_rect());
            MainInterfaceGlobal.soft_update(&rect);
            return;
        }
    } else if (iUnitIndex_ >= 0) {
        const int next = Vehs[iUnitIndex_].next_veh_id_stack;
        if (next >= 0) {
            iUnitIndex_ = next;

            const int font_height = WinScreenWidth == 0x320 ? 10 : 12;
            if (font_height != font_size_) {
                font_size_ = font_height;
                font1_.init(DefaultFontFace, font_height, 0);
                font2_.init(DefaultFontFace, font_height, 2);
                font3_.init(DefaultFontFace, font_height, 1);
            }
            if (WinScreenWidth != 0x320) {
                font1_.unk_1_ = 4;
                font2_.unk_1_ = 4;
                font3_.unk_1_ = 4;
            }

            draw_status(-1, -1, -1, -1, 0);
            RECT rect;
            UnionRect(&rect, &MainInterfaceGlobal.status_hit_rect(),
                      &MainInterfaceGlobal.status_right_rect());
            UnionRect(&rect, &rect, &MainInterfaceGlobal.status_left_rect());
            MainInterfaceGlobal.soft_update(&rect);
            return;
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004B9FC0 ?init@StatusWin@@QAEXXZ 0x004B9FC0-0x004BA11E
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004b9fc0.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_statuswin_remaining_behavior.py"], "reason": "Home the complete typed StatusWin init operation. Retain the original seed and historical obligations; this product has no new measured claim."}
void StatusWin::init() {
    field_15A8_ = 0;
    fInOnClick_ = 0;
    iTileX_ = 0;
    iTileY_ = 0;
    iUnitIndex_ = -1;
    iCityIndex_ = -1;
    hover_base_id_ = -1;
    hover_vehicle_id_ = -1;
    field_15D0_ = -1;
    field_15CC_ = -1;
    font_size_ = -1;
    field_15D4_ = 0;

    if ((StatusFlags & 0x20u) == 0) {
        StatusFlags |= 0x20u;
    }

    spot_.init(0x18);
    caviar_.init();
    caviar_.set_field_A4(0);

    camera_angles_.values[1] = 0.0f;
    camera_angles_.values[2] = 0.785f;
    camera_angles_.values[0] = 0.0f;

    caviar_apply_rotation(camera_angles_.values, &camera_rotation_);

    camera_rotation_.values[2][0] = -camera_rotation_.values[2][0];
    camera_rotation_.values[2][1] = -camera_rotation_.values[2][1];
    camera_rotation_.values[2][2] = -camera_rotation_.values[2][2];
    VOX_Vect direction;
    direction.values[0] = camera_rotation_.values[2][0] * -10000.0f;
    direction.values[1] = camera_rotation_.values[2][1] * -10000.0f;
    direction.values[2] = camera_rotation_.values[2][2] * -10000.0f;
    caviar_.set_camera_direct(&direction, &camera_rotation_);
    caviar_.set_scaling(0.18f);

    direction.values[0] = 0.0f;
    direction.values[1] = 0.0f;
    direction.values[2] = 1.0f;
    caviar_.shadow_vector() = direction;
    caviar_.light_vector() = direction;
}
