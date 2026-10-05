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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original destructor span and its direct close target.", "Original model arrays and renderer embedding EH cleanup distinguish loaded-model and instance lifetimes.", "Fresh VC6 observations under both configured flag sets identify this natural emission host and preserve normal versus EH call targets.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004210f0.cpp", "Historical seed spelling ??1Caviar@@QAE@XZ; current source mapping ??1Caviar@@QAE@XZ; emitted symbol ??1Caviar@@QAE@XZ."], "reason": "Genuine header-defined destructor in src/caviar.h; this translation unit naturally emits the original five-byte cleanup tail through real object lifetime use, without synthetic calls or forced exports. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// LEVER: thiscall shim - a synthetic class exposes close() as a member so "call close(); ret" folds into the original's bare 5-byte jmp
// body      src/caviar.h
#include "stdafx.h"
#include "base.h"
#include "battlewin.h"
#include "map.h"
#include "terraforming.h"
#include "original_seam.h"
#include "strings.h"  // StringTemp, 0x009B86A0
#include "statuswin.h"
#include "spritebox.h"
#include "font.h"
#include "infowin.h"
#include "win.h"
#include "graphicwin.h"
#include "mapwin.h"  // MainInterfaceGlobal
#include "alpha.h"
#include "basewin.h"
#include "console.h"
#include "datalink.h"
#include "game.h"
#include "netdaemon.h"
#include "popmenu.h"
#include "spying_recovery.h"
#include "tutwin.h"
#include "veh.h"

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
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/004b4bf0.cpp retained BYTE_EXACT seed; Vehs stride 0x34, VehPrototypes chassis_id offset 0x24, Chassis triad offset 0x49.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/004b4bf0.cpp"], "reason": "Home the destination vehicle route painter reached by MapWin recentering using Veh and Chassis records. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void __cdecl draw_my_dest(int vehicle_id) {
    if (vehicle_id < 0 || vehicle_id >= VehCurrentCount) {
        return;
    }
    Veh &vehicle = Vehs[vehicle_id];
    if (vehicle.faction_id != ConsoleGlobal.faction_id_ &&
        (GameState & STATE_OMNISCIENT_VIEW) == 0) {
        return;
    }
    if (vehicle.order != 0x18) {
        return;
    }
    CoverWindow.fill(9);
    const int prototype_id = vehicle.proto_id;
    const int chassis_id = VehPrototypes[prototype_id].chassis_id;
    const int route_color = Chassis[chassis_id].triad;
    ConsoleGlobal.dest_line(vehicle.x, vehicle.y,
                            vehicle.waypoint_x[0], vehicle.waypoint_y[0],
                            prototype_id, vehicle.faction_id, route_color,
                            0, 3, 5, 1);
    CoverWindow.update(static_cast<GraphicWin *>(0));
    DestinationVehicleId = vehicle_id;
}

// ORIGINAL: 0x004B4CD0 ?UNK2@StatusWin@@QAEXXZ 0x004B4CD0-0x004B4CE4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x004B4CD0-0x004B4CE4: store -1 at 0x006874C4, set ECX 0x00937118, tail-call Win::hide."], "reason": "Home the destination hide operation over the typed destination vehicle state and CoverWindow."}
void StatusWin::UNK2() {
    DestinationVehicleId = -1;
    CoverWindow.Win::hide();
}

// ORIGINAL: 0x004B9010 ?on_right_click@StatusWin@@QAEXHHH@Z 0x004B9010-0x004B9DF8;0x00659AA0-0x00659BCA
// RECOVERY_BINDING: v1 {"evidence": ["image:0x4b9010"], "reason": "Recover the full original operation using owned C++ members and base lifetimes; retain all original branches and expose unresolved callees instead of executing original-address forwarders."}
void StatusWin::on_right_click(int x, int y, int) {
    PullDown menu;
    PopMenu display_menu;
    menu.init(0);
    int object_index = -1;
    int menu_id = -1;
    if (spot_.check(x, y, &object_index, &menu_id) < 0) {
        return;
    }
    if (menu_id == 3) {
        display_menu.init();
        display_menu.dialogs().item(label_get(0xFEC / 4), 0);
        display_menu.dialogs().item(label_get(0xFF0 / 4), 1);
        display_menu.dialogs().item(label_get(0xFF4 / 4), 2);
        int screen_x, screen_y;
        MainInterfaceGlobal.get_mouse_pos(&screen_x, &screen_y);
        MainInterfaceGlobal.client_to_screen(&screen_x, &screen_y);
        int choice = display_menu.exec(screen_x, screen_y, 0);
        if (choice == 0 || choice == 1) {
            field_15A8_ = choice == 0 ? 0 : -1;
            draw_status(-1, -1, -1, -1, 0);
        } else if (choice == 2) {
            DatalinkMain.exec(9, 0);
        }
        return;
    }

    int base_id = -1;
    if (menu_id != 2) {
        if (menu_id == 0 || menu_id == 10) {
            if (menu_id == 10) object_index = ConsoleGlobal.selected_vehicle_;
            menu.add_item(6, label_get(0x958 / 4));
            menu.add_item(7, label_get(0x112C / 4));
            if (GameState & STATE_SCENARIO_EDITOR) {
                menu.add_item(90, label_get(0xDFC / 4));
            }
            menu.add_separator(99);
        } else {
            base_id = object_index;
            int owner = Bases[base_id].faction_id_current;
            if ((GameState & STATE_OMNISCIENT_VIEW) || owner == ConsoleGlobal.faction_id_
                || spying(owner) || (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[owner] & 1)) {
                menu.add_item(5, label_get(0x894 / 4));
            }
            if (owner == ConsoleGlobal.faction_id_ || (GameState & STATE_OMNISCIENT_VIEW)) {
                for (int action = 51; action <= 56; ++action) {
                    StringTemp[0] = '\0';
                    strcat(StringTemp, " ");
                    strcat(StringTemp, label_get(0x898 / 4 + action - 51));
                    if (action == 52) {
                        strcat(StringTemp, " ");
                        int production = Bases[base_id].queue_production_id[0];
                        // This caller passes the facility name through Strings::get.
                        // RulesFacility still types its stored string offset as LPSTR;
                        // that shared type/loader disagreement needs coupled recovery.
                        char *name = production < 0
                            ? StringTable.get(reinterpret_cast<int>(Facility[-production].name))
                            : VehPrototypes[production].veh_name;
                        strcat(StringTemp, name);
                    }
                    menu.add_item(action, StringTemp);
                }
                menu.add_separator(99);
            }
        }
    }
    menu.add_item(20, label_get(0x8FC / 4));
    MainInterfaceGlobal.client_to_screen(&x, &y);
    menu.show(x, y, 0);
    int selected = menu.get_selected_id();
    bool network_blocked = IsMultiplayerNet &&
        (ConsoleControlTurnActive || NetDaemonNet.lock_state_[2].payload_.flags
         || NetDaemonNet.lock_state_[0].leading_words_[3]);
    bool faction_blocked = IsMultiplayerNet &&
        (NetDaemonNet.lock_state_[0].trailing_words_[7] & (1U << ConsoleGlobal.faction_id_));

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
    case 51: case 52: case 53: case 54: case 55: case 56:
        if (!network_blocked && !faction_blocked) {
            TutWinBaseWindow.soft_zoom(base_id);
            if (selected == 51) TutWinBaseWindow.production(0, 0);
            else if (selected == 52) TutWinBaseWindow.hurry();
            else TutWinBaseWindow.gov_on(0x1000000 << (selected - 53));
        }
        break;
    case 90:
        ConsoleGlobal.editor_edit_veh(object_index);
        break;
    }
    if (selected != 5 && selected != 6 && (field_15C4_ >= 0 || field_15C8_ >= 0)) {
        field_15C4_ = field_15C8_ = -1;
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
// ORIGINAL: 0x004BA120 ??1StatusWin@@QAE@XZ 0x004BA120-0x004BA19E BYTE_EXACT
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
        sub_interface_global()->release_iface_mode();
    }
}


// Three fixed-address RECTs redraw() unions together, no established
// identity beyond what this one function shows. Now defined in statuswin.h,
// where the internal linkage folds the address to the immediate in every TU.

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
void StatusWin::redraw() {
    int sizeVal = (WinScreenWidth != 0x320) ? 0xC : 0xA;
    if (sizeVal != field_15AC_) {
        field_15AC_ = sizeVal;
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

void __fastcall status_win_redraw_redirect(StatusWin *self, void *) {
    self->redraw();
}

// The process-wide status window, 0x008C5568 in the image - REAL STORAGE, not
// the `StatusWin *const ConsoleStatusWin` binding that named terranx.exe data
// (see console.h). The image's dynamic initializer at 0x004B3FA0 runs the
// real StatusWin constructor against it and registers the deleting destructor
// with atexit; here the same recovered constructor runs through this build's
// own startup and the destructor closes it at exit.
// RECOVERY_OBJECT: v1 0x008C5568 ?ConsoleStatusWin@@3VStatusWin@@A
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
void StatusWin::draw_status(int a1, int a2, int a3, int a4, int a5) {
    field_15DC_ = 0;
    if (a1 >= 0 && a2 >= 0) {
        g_BattleWin.battle_report(a1, a2, a3, a4, a5);
        return;
    }
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0) return;

    for (int type = 0; type < 4; ++type) spot_.kill_type(type);
    spot_.kill_type(10);
    spot_.kill_type(11);

    font1_.width(const_cast<char *>("#"));
    a2 = 1;
    a3 = font1_.unk_1_ >= 0
        ? font1_.pixel_height() + font1_.unk_1_ : font1_.line_height();
    Buffer *const canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    canvas->set_font(&font1_, &font3_, &font2_, 0);

    int selected_vehicle = ConsoleGlobal.selected_vehicle_;
    int selected_base = -1;
    int base_under_cursor = -1;
    int tile_x = iTileX_;
    int tile_y = iTileY_;
    if (ConsoleGlobal.cursor_active_ != 0 && selected_vehicle >= 0 &&
        TurnCurrentNum != 0) {
        tile_x = Vehs[selected_vehicle].x;
        tile_y = Vehs[selected_vehicle].y;
        if (tile_x != iTileX_ || tile_y != iTileY_) goto resolve_panel_selection;
    } else {
resolve_panel_selection:
        tile_x = iTileX_;
        tile_y = iTileY_;
        selected_vehicle = TurnCurrentNum != 0 ? veh_at(tile_x, tile_y) : -1;
        base_under_cursor = base_at(tile_x, tile_y);
        if (selected_vehicle >= 0) {
            const int original_vehicle = selected_vehicle;
            int top_vehicle = selected_vehicle;
            while (Vehs[top_vehicle].prev_veh_id_stack >= 0)
                top_vehicle = Vehs[top_vehicle].prev_veh_id_stack;
            const int original_owner = Vehs[original_vehicle].faction_id;
            const int viewer = ConsoleGlobal.faction_id_;
            if (original_owner != viewer &&
                (GameState & STATE_OMNISCIENT_VIEW) == 0) {
                const Map &tile = MapTiles[tile_y * MapLongitude + (tile_x >> 1)];
                if ((PlayersData[viewer].flags & 0x200) == 0 &&
                    (tile.visibility & (1U << (viewer & 31))) == 0) {
                    selected_vehicle = -1;
                } else if (base_under_cursor >= 0) {
                    const Base &base = Bases[base_under_cursor];
                    bool can_see_base =
                        (PlayersData[base.faction_id_current]
                             .diplo_treaties[viewer] & 1) != 0 ||
                        spying(base.faction_id_current) != 0;
                    for (int neighbor = 0; !can_see_base && neighbor < 8; ++neighbor) {
                        const int x = base.x + VehMoveDeltaX[neighbor];
                        const int y = base.y + VehMoveDeltaY[neighbor];
                        if (y < 0 || y >= MapLatitudeBounds) continue;
                        int wrapped_x = x;
                        if ((MapIsFlat & 1) == 0) {
                            if (wrapped_x < 0) wrapped_x += MapLongitudeBounds;
                            else if (wrapped_x >= MapLongitudeBounds)
                                wrapped_x -= MapLongitudeBounds;
                        }
                        if (wrapped_x < 0 || wrapped_x >= MapLongitudeBounds) continue;
                        const Map &nearby = MapTiles[y * MapLongitude +
                                                    (wrapped_x >> 1)];
                        if ((nearby.bit & BIT_VEH_IN_TILE) != 0 &&
                            ((nearby.val2 & 0x0f) == viewer)) can_see_base = true;
                    }
                    if ((base.faction_id_current == viewer ||
                         (GameState & STATE_OMNISCIENT_VIEW) != 0 ||
                         (base.visibility & (1U << (viewer & 31))) != 0) &&
                        can_see_base) {
                        // Visibility of the base keeps the selected vehicle;
                        // the city panel selection is resolved from iCityIndex_.
                    } else {
                        selected_vehicle = -1;
                    }
                } else {
                    const Veh &stack_top = Vehs[top_vehicle];
                    if ((stack_top.faction_id == viewer &&
                         (stack_top.flags & 0x440) != 0x440) ||
                        (stack_top.visibility & (1U << (viewer & 31))) != 0) {
                        // The selected stack remains available when its top
                        // vehicle is visible to the current faction.
                    } else {
                        selected_vehicle = -1;
                    }
                }
            }
        }
    }

    selected_base = -1;
    if (iUnitIndex_ >= 0) {
        const Veh &selected = Vehs[iUnitIndex_];
        if (selected.x == tile_x && selected.y == tile_y) selected_vehicle = iUnitIndex_;
    } else if (iCityIndex_ >= 0) {
        const Base &selected = Bases[iCityIndex_];
        if (selected.x == tile_x && selected.y == tile_y)
            selected_base = iCityIndex_;
    }
    field_15DC_ = selected_vehicle >= 0;
    if (selected_base >= 0 && (GameState & STATE_OMNISCIENT_VIEW) == 0) {
        const Base &base = Bases[selected_base];
        if (base.faction_id_current != ConsoleGlobal.faction_id_ &&
            (base.visibility & (1U << (ConsoleGlobal.faction_id_ & 31))) == 0) {
            selected_base = -1;
            iCityIndex_ = -1;
        }
    }

    RECT work = MainInterfaceGlobal.status_right_rect();
    canvas->box_sprite(&work, &InterfaceBoxStyles[21]);
    work.top += 3; work.right -= 3; work.left += 3; work.bottom -= 3;
    canvas->box_sprite(&work, &InterfaceBoxStyles[18]);
    work.left += 3; work.right -= 3; work.top += 3; work.bottom -= 3;
    canvas->set_clip(&work);
    canvas->set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);

    int text_x = work.left;
    int text_y = (work.right - work.left) / 2 + 3 + work.top;
    int map_index = tile_y * MapLongitude + (tile_x >> 1);
    const int viewer = ConsoleGlobal.faction_id_;
    const Map &tile = MapTiles[map_index];
    char number[84];
    const bool visible = (GameState & STATE_OMNISCIENT_VIEW) != 0 ||
        (PlayersData[viewer].flags & 0x200) != 0 ||
        (tile.visibility & (1U << (viewer & 31))) != 0;

    if (visible) {
        MapWinCaptionScale = (work.right - work.left) - 4;
        MapWinCaptionDivisor = 100;
        const int map_faction = (GameState & STATE_OMNISCIENT_VIEW)
            ? -1 : viewer;
        gen_terrain_poly(canvas, 0, 0, map_faction, 0, 0x621,
                         tile_x, tile_y, text_x + 2, work.top + 1);
        MapWinCaptionScale = -1;
        MapWinCaptionDivisor = -1;

        int resource_text_y = text_y - 4;
        if (field_15A8_ != 0) {
            canvas->set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);
            const int crop = crop_yield(viewer, -1, tile_x, tile_y, 0);
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(
                static_cast<int>(reinterpret_cast<intptr_t>(Resource[0].name_plural))));
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            itoa(crop, number, 10);
            strcat(StringTemp, number);
            canvas->write_l(StringTemp, text_x, resource_text_y,
                            strlen(StringTemp));
            const Map &resource_tile = MapTiles[map_index];
            const uint32_t resource_flags = resource_tile.bit;
            const bool crop_bonus_visible =
                (resource_tile.val3 & 0xc0) <= 0x7f &&
                (!(resource_flags & BIT_FUNGUS) ||
                 (resource_tile.climate & 0xe0) <= 0x3f) &&
                (resource_flags & 0xa000) == 0 &&
                (crop <= 1 || has_tech(Rules.tech_three_nutrients_sqr, viewer) ||
                 bonus_at(tile_x, tile_y, 0) == 1 ||
                 (resource_flags & BIT_CONDENSER) != 0);
            resource_text_y += a3;
            if (crop_bonus_visible) {
                StringTemp[0] = '\0';
                strcat(StringTemp, "  ");
                strcat(StringTemp, label_get(0xfc4 / 4));
                strcat(StringTemp, ": ");
                strcat(StringTemp, " ");
                itoa(crop + 1, number, 10);
                strcat(StringTemp, number);
                canvas->write_l(StringTemp, text_x, resource_text_y,
                                strlen(StringTemp));
            }
            resource_text_y += a3;
            const int minerals = mine_yield(viewer, -1, tile_x, tile_y, 0);
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(
                static_cast<int>(reinterpret_cast<intptr_t>(Resource[1].name_plural))));
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            itoa(minerals, number, 10);
            strcat(StringTemp, number);
            canvas->write_l(StringTemp, text_x, resource_text_y,
                            strlen(StringTemp));
            const bool mineral_bonus_visible =
                (resource_flags & 0x1002000) == 0 &&
                (resource_flags & 0x10) == 0 &&
                (!(resource_flags & BIT_FUNGUS) ||
                 (resource_tile.climate & 0xe0) <= 0x3f) &&
                minerals < mine_yield(viewer, -1, tile_x, tile_y, 1);
            resource_text_y += a3;
            if (mineral_bonus_visible) {
                StringTemp[0] = '\0';
                strcat(StringTemp, "  ");
                strcat(StringTemp, label_get(0xfc8 / 4));
                strcat(StringTemp, ": ");
                strcat(StringTemp, " ");
                itoa(mine_yield(viewer, -1, tile_x, tile_y, 1), number, 10);
                strcat(StringTemp, number);
                canvas->write_l(StringTemp, text_x, resource_text_y,
                                strlen(StringTemp));
            }
            resource_text_y += a3;
            const int energy = energy_yield(viewer, -1, tile_x, tile_y, 0);
            StringTemp[0] = '\0';
            strcat(StringTemp, StringTable.get(
                static_cast<int>(reinterpret_cast<intptr_t>(Resource[2].name_plural))));
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            itoa(energy, number, 10);
            strcat(StringTemp, number);
            canvas->write_l(StringTemp, text_x, resource_text_y,
                            strlen(StringTemp));
            const bool energy_bonus_visible =
                (resource_flags & 0x1002000) == 0 &&
                (resource_flags & 0x40) == 0 &&
                (!(resource_flags & BIT_FUNGUS) ||
                 (resource_tile.climate & 0xe0) <= 0x3f);
            resource_text_y += a3;
            if (energy_bonus_visible) {
                StringTemp[0] = '\0';
                strcat(StringTemp, "  ");
                strcat(StringTemp, label_get(0xfcc / 4));
                strcat(StringTemp, ": ");
                strcat(StringTemp, " ");
                itoa(energy_yield(viewer, -1, tile_x, tile_y, 1), number, 10);
                strcat(StringTemp, number);
                canvas->write_l(StringTemp, text_x, resource_text_y,
                                strlen(StringTemp));
            }
            text_y = resource_text_y + a3;
        }
    } else {
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x300 / 4));
        canvas->write_cent_l(StringTemp, &work, strlen(StringTemp));
    }

    if (visible && field_15A8_ == 0) {
        const int terrain_kind = static_cast<int>(tile.climate >> 5) - 3;
        if (terrain_kind < 0) {
            StringTemp[0] = '\0';
            int water_label = -1;
            if (terrain_kind == -3) water_label = 0x568;
            else if (terrain_kind == -2) water_label = 0x304;
            else if (terrain_kind == -1) water_label = 0x564;
            if (water_label >= 0) {
                strcat(StringTemp, label_get(water_label / 4));
                const int written = canvas->write_l(StringTemp, text_x,
                                                     text_y - 4,
                                                     strlen(StringTemp));
                if (written >= 0) a1 = written;
                text_y += a3;
            }
        }

        StringTemp[0] = '\0';
        const int elevation = abs(elev_at(tile_x, tile_y));
        strcat(StringTemp, label_get((terrain_kind < 0 ? 0x30 : 0x2c) / 4));
        strcat(StringTemp, ": ");
        itoa(elevation, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, label_get(0x34 / 4));
        const int terrain_label_y = text_y;
        const int terrain_text_width = canvas->write_l(
            StringTemp, text_x, terrain_label_y, strlen(StringTemp));
        text_y += a3;

        const int altitude = tile.climate & 0xe0;
        if (terrain_kind >= 0) {
            if (ClimateSeaLevelDelta >= 1 &&
                static_cast<int>(tile.contour) - MapSeaLevel <
                    static_cast<int>(AltNatural[3]) + ClimateSeaLevelDelta) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0x7fc / 4));
                canvas->set_text_color(0xf9, -1, 1, 1);
                canvas->write_l(StringTemp, text_x, text_y, strlen(StringTemp));
                text_y += a3;
            }
        } else if (ClimateSeaLevelDelta <= -1 &&
                   static_cast<int>(AltNatural[3]) + ClimateSeaLevelDelta <=
                       static_cast<int>(tile.contour) - MapSeaLevel) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0x7fc / 4));
            canvas->set_text_color(0xf9, -1, 1, 1);
            canvas->write_l(StringTemp, text_x, text_y, strlen(StringTemp));
            text_y += a3;
        }
        canvas->set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);

        if (terrain_kind >= 0) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get((0x164 + (tile.val3 >> 6) * 4) / 4));
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, "&");
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, label_get((0x50 + ((tile.climate & 0x18) >> 3) * 4) / 4));
            const int altitude_rainfall_width = canvas->write_l(
                StringTemp, text_x, text_y, strlen(StringTemp));
            if (terrain_text_width > altitude_rainfall_width)
                a1 = terrain_text_width;
            else a1 = altitude_rainfall_width;
            text_y += a3;

            StringTemp[0] = '\0';
            if ((tile.bit2 & 0x80000002U) == BIT2_VOLCANO) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[1].name_short))));
            }
            if ((tile.bit2 & 0x80000001U) == BIT2_CRATER) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[0].name_short))));
            }
            if ((tile.bit2 & 0x80000004U) == BIT2_JUNGLE &&
                altitude >= 0x60) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[2].name_short))));
            }
            if ((tile.bit2 & 0x80000008U) == BIT2_URANIUM) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[3].name_short))));
            }
            if ((tile.bit2 & 0x80000040U) == BIT2_DUNES) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[6].name_short))));
            }
            if ((tile.bit2 & 0x80000100U) == BIT2_MESA) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[8].name_short))));
            }
            if ((tile.bit2 & 0x80000200U) == BIT2_CANYON) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[9].name_short))));
            }
            if ((tile.bit2 & 0x80000800U) == BIT2_RIDGE) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[11].name_short))));
            }
            if ((tile.bit2 & 0x80002000U) == BIT2_NEXUS) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[13].name_short))));
            }
            if (ExpansionEnabled != 0 &&
                (tile.bit2 & 0x80004000U) == BIT2_UNITY) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[14].name_short))));
            }
            if (StringTemp[0] != '\0') {
                canvas->write_l(StringTemp, text_x, text_y, strlen(StringTemp));
                text_y += a3;
            }

            StringTemp[0] = '\0';
            const bool has_river = (tile.bit & BIT_RIVER) != 0;
            if (has_river) strcat(StringTemp, label_get(0x3a8 / 4));
            if ((tile.bit & BIT_FUNGUS) != 0 && altitude >= 0x40) {
                if (StringTemp[0] != '\0') strcat(StringTemp, ", ");
                strcat(StringTemp, label_get(0x21c / 4));
            } else if ((tile.bit & 0x200000) != 0) {
                if (StringTemp[0] != '\0') strcat(StringTemp, ", ");
                strcat(StringTemp, label_get(0x48c / 4));
            }
            if (StringTemp[0] != '\0') {
                canvas->write_l(StringTemp, text_x, text_y, strlen(StringTemp));
                text_y += a3;
            }
        } else {
            StringTemp[0] = '\0';
            if (ExpansionEnabled != 0 &&
                (tile.bit2 & 0x80008000U) == BIT2_FOSSIL)
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[15].name_short))));
            if ((tile.bit2 & BIT2_FRESH) != 0 && altitude <= 0x5f) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[7].name_short))));
            }
            if ((tile.bit2 & 0x80000400U) == BIT2_GEOTHERMAL) {
                strcat(StringTemp, StringTable.get(
                    static_cast<int>(reinterpret_cast<intptr_t>(Natural[10].name_short))));
            }
            if ((tile.bit & BIT_FUNGUS) != 0 && altitude >= 0x40) {
                if (StringTemp[0] != '\0') strcat(StringTemp, ", ");
                strcat(StringTemp, label_get(0x398 / 4));
            }
            if (StringTemp[0] != '\0') {
                canvas->write_l(StringTemp, text_x, text_y, strlen(StringTemp));
                text_y += a3;
            }
        }

        const int bonus = bonus_at(tile_x, tile_y, 0);
        if (bonus >= 1) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get((0x16c + bonus * 4) / 4));
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x17c / 4));
            canvas->write_l(StringTemp, text_x, text_y, strlen(StringTemp));
            text_y += a3;
        }

        StringTemp[0] = '\0';
        bool has_terraforming_label = false;
        for (int terrain = 0; terrain < 0x13; ++terrain) {
            const uint32_t terrain_bit = Terraforming[terrain].bit;
            if (terrain_bit == 0 || (tile.bit & terrain_bit) == 0 ||
                terrain == TERRA_FOREST || terrain == TERRA_REMOVE_FUNGUS ||
                terrain == TERRA_PLANT_FUNGUS) continue;
            if (terrain == 0 && (tile.bit & 0x80000)) continue;
            if (terrain == 5 && (tile.bit & 0x8)) continue;
            if (has_terraforming_label) strcat(StringTemp, ", ");
            const LPSTR label = (tile.climate & 0xe0) < 0x60
                ? Terraforming[terrain].name_sea : Terraforming[terrain].name;
            strcat(StringTemp,
                   StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(label))));
            has_terraforming_label = true;
        }
        if ((tile.bit & BIT_MONOLITH) != 0) {
            if (has_terraforming_label) strcat(StringTemp, ", ");
            strcat(StringTemp, label_get(0x238 / 4));
        }
        if (StringTemp[0] != '\0') {
            RECT wrap_rect = work;
            wrap_rect.top = text_y;
            text_y = canvas->wrap(StringTemp, &wrap_rect);
        }
    }

    if ((GameState & STATE_SCENARIO_EDITOR) != 0 &&
        (GameState & STATE_DEBUG_MODE) != 0) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "(");
        itoa(static_cast<int>(static_cast<int8_t>(tile.region)), number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ")");
        canvas->set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);
        canvas->write_l(StringTemp, text_x + 2, work.bottom - a3,
                        strlen(StringTemp));
    }
    if (((GameState & STATE_SCENARIO_EDITOR) != 0 &&
         (GameState & STATE_DEBUG_MODE) != 0) ||
        text_y < work.bottom - a3) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "(");
        itoa(tile_x, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, " ");
        strcat(StringTemp, ", ");
        itoa(tile_y, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ")");
        canvas->set_text_color(TextColourTable[18], TextColourTable[19], 1, 1);
        canvas->write_right_l(StringTemp, work.right - 2,
                              work.bottom - a3, strlen(StringTemp));
    }

    canvas->restore_clip();
    RECT status_right = MainInterfaceGlobal.status_right_rect();
    spot_.add(0, 3, status_right.left, status_right.top,
              status_right.right - status_right.left,
              status_right.bottom - status_right.top);
    MainInterfaceGlobal.soft_update(&status_right);

    field_15C8_ = -1;
    field_15C4_ = -1;
    field_15CC_ = selected_vehicle;
    field_15D0_ = selected_base;
    draw_active(selected_vehicle, selected_base, 1);

    RECT unit_panel = MainInterfaceGlobal.status_hit_rect();
    canvas->box_sprite(&unit_panel, &InterfaceBoxStyles[21]);
    unit_panel.top += 3; unit_panel.right -= 3;
    unit_panel.bottom -= 3; unit_panel.left += 3;
    canvas->box_sprite(&unit_panel, &InterfaceBoxStyles[18]);
    unit_panel.top += 3; unit_panel.right -= 3;
    unit_panel.bottom -= 3; unit_panel.left += 3;
    if (TurnCurrentNum == 0) return;

    int base_id = base_at(tile_x, tile_y);
    if (base_id >= 0 && (GameState & STATE_OMNISCIENT_VIEW) == 0 &&
        Bases[base_id].faction_id_current != viewer &&
        (Bases[base_id].visibility & (1U << (viewer & 31))) == 0) base_id = -1;
    int item_count = base_id >= 0 ? 1 : 0;
    if (selected_vehicle >= 0) {
        int top = selected_vehicle;
        while (Vehs[top].prev_veh_id_stack >= 0)
            top = Vehs[top].prev_veh_id_stack;
        for (int unit = top; unit >= 0; unit = Vehs[unit].next_veh_id_stack)
            ++item_count;
    }
    int item_width = (MapWinCaptionScale >= 1
        ? MapWinCaptionScale * 100 / MapWinCaptionDivisor : 0x32) + 0xb;
    const int visible_items = (unit_panel.right - unit_panel.left) /
        (item_width + 5);
    if (item_count <= visible_items) {
        MainInterfaceGlobal.flat_button(8).hide();
        MainInterfaceGlobal.flat_button(9).hide();
        MainInterfaceGlobal.flat_button(10).hide();
    } else {
        const RECT &stack_buttons = MainInterfaceGlobal.stack_button_rect();
        const RECT &adjacent_right = MainInterfaceGlobal.adjacent_panel_right_rect();
        item_width = (adjacent_right.left - stack_buttons.right - 2) /
            visible_items - 5;
        MainInterfaceGlobal.flat_button(8).show(0);
        MainInterfaceGlobal.flat_button(10).show(0);
    }

    const RECT &stack_buttons = MainInterfaceGlobal.stack_button_rect();
    int row_x = unit_panel.left;
    if (item_count > visible_items) row_x = stack_buttons.right + 1;
    const int row_top = unit_panel.top;
    const int row_bottom = unit_panel.bottom;
    const int row_step = item_width + 5;
    int drawn = 0;
    if (base_id >= 0) {
        RECT row_clip;
        row_clip.left = row_x - 2; row_clip.right = row_x + item_width + 2;
        row_clip.top = row_top - 2; row_clip.bottom = row_bottom + 2;
        canvas->set_clip(&row_clip);
        base_draw(canvas, base_id, row_x + 11, row_top - 1, -8, 0x40);
        base_draw(canvas, base_id, row_x, row_top - 1, 0, 0x1800);
        spot_.add(base_id, 11, row_x, row_top,
                  item_width + 5, row_bottom - row_top);
        if (base_id == selected_base) {
            RECT outline;
            outline.left = row_x - 2; outline.right = row_x + item_width + 2;
            outline.top = row_top - 2; outline.bottom = row_bottom + 2;
            canvas->box(&outline, 0xae, 0xae);
        }
        row_x += row_step;
        drawn = 1;
    }

    if (MainInterfaceStatusRedrawFlag == 0 && selected_vehicle > -1) {
        stack_sort_2(selected_vehicle);
    }
    if (selected_vehicle >= 0) {
        int top = selected_vehicle;
        while (Vehs[top].prev_veh_id_stack >= 0)
            top = Vehs[top].prev_veh_id_stack;
        for (int unit = top; unit >= 0 && drawn < visible_items;
             unit = Vehs[unit].next_veh_id_stack, ++drawn) {
            RECT row;
            row.left = row_x - 2; row.right = row_x + item_width + 2;
            row.top = row_top - 2; row.bottom = row_bottom + 2;
            canvas->set_clip(&row);
            veh_draw(canvas, unit, row_x + 11, row_top - 1, -8, 0x400, 0);
            veh_draw(canvas, unit, row_x, row_top - 1, 0, 0x1800, 0);
            canvas->set_font(&font1_, 0, 0, 0);
            StringTemp[0] = '\0';
            const Veh &vehicle = Vehs[unit];
            if (vehicle.order >= 4 && vehicle.order <= 0x17 &&
                (vehicle.faction_id == viewer ||
                 (GameState & STATE_OMNISCIENT_VIEW) != 0 ||
                 (PlayersData[viewer].diplo_treaties[vehicle.faction_id] & 1))) {
                const int turns = action_terraform(unit, vehicle.order - 4, 0);
                itoa(turns + 1, number, 10);
                strcat(StringTemp, number);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get((turns == 1 ? 0xc0 : 0xc4) / 4));
                canvas->set_text_color(0xfb, 3, 1, 1);
            } else {
                say_stats_3(StringTemp, vehicle.proto_id);
                if ((vehicle.flags & 0x80) != 0) strcat(StringTemp, "#");
                if ((GameState & STATE_SCENARIO_EDITOR) != 0 &&
                    (GameState & STATE_DEBUG_MODE) != 0) {
                    strcat(StringTemp, " ");
                    strcat(StringTemp, "(");
                    itoa(unit, number, 10);
                    strcat(StringTemp, number);
                    strcat(StringTemp, ")");
                }
                canvas->set_text_color(TextColourTable[20], TextColourTable[21], 1, 1);
            }
            canvas->write_cent_l(StringTemp, row_x,
                                 font1_.descent() - a3 + row_bottom + 3,
                                 item_width, strlen(StringTemp));
            if (unit == selected_vehicle && selected_base < 0) {
                RECT outline = row;
                canvas->box(&outline, 0xae, 0xae);
            }
            spot_.add(unit, 0, row_x, row_top,
                      item_width + 5, row_bottom - row_top);
            row_x += row_step;
        }
    }

    find_font(0x14, 0);
    RECT status_hit = MainInterfaceGlobal.status_hit_rect();
    MainInterfaceGlobal.soft_update(&status_hit);
    canvas->restore_clip();
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
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004b4cf0.cpp retained full transcription and original span", "MainInterfaceGlobal Buffer and status_left_rect_ layout", "Veh and Base records, destination state and box styles in source"], "reason": "Home the active-status panel's frame, unit/base identity display, selection spots and destination lifecycle in their real owners; retain omitted detail branches as restoration obligations."}
// TRIED: full-function transcription (~1843 instructions, sim 0.46 vs original at /O2 /Oy-); diverges at instr #3. Both the vehicle- display and base-display halves are transcribed from Ghidra with the Buffer receiver confirmed as the fixed global g_007aec64 (checked against raw disasm at two call sites); the repeated chassis-speed/range mini-blocks were copied by pattern from the first verified instance rather than each re-checked individually against the disassembly.
// The retained seed is the restoration reference for the colour palette,
// punctuation and disputed row layout. Compiling this body does not establish
// exact display behavior or byte restoration.
void StatusWin::draw_active(int vehicle_id, int base_id, int add_spot) {
    Buffer *screen = static_cast<Buffer *>(&MainInterfaceGlobal);
    screen->set_font(&font1_, &font3_, &font2_, 0);

    // The image copies the left panel rectangle, draws two inset frames,
    // then reserves another three pixels on every side for active content.
    RECT area = MainInterfaceGlobal.status_left_rect();
    screen->box_sprite(&area, &InterfaceBoxStyles[21]);
    area.bottom -= 3;
    area.left += 3;
    area.right -= 3;
    area.top += 3;
    screen->box_sprite(&area, &InterfaceBoxStyles[18]);
    area.left += 3;
    area.right -= 3;
    area.top += 3;
    area.bottom -= 3;
    const int content_x = area.left;
    const int content_y = area.top;
    const int content_width = area.right - area.left;
    const int sprite_width = MapWinCaptionScale < 1 ? 75
        : MapWinCaptionScale * 100 / MapWinCaptionDivisor;
    const int sprite_x = ((area.right - sprite_width) - area.left) / 2 + content_x;
    const int title_offset = MapWinCaptionScale < 1 ? 56
        : MapWinCaptionScale * 75 / MapWinCaptionDivisor;

    if (base_id < 0 && vehicle_id >= 0 && vehicle_id < VehCurrentCount) {
        Veh &vehicle = Vehs[vehicle_id];
        screen->set_clip(&area);
        veh_draw(screen, vehicle_id, sprite_x, content_y, -4, 0x48, 0);
        veh_draw(screen, vehicle_id, content_x, content_y, 0, 0x1800, 0);
        screen->set_font(&font1_, 0, 0, 0);
        StringTemp[0] = '\0';
        strcat(StringTemp, VehPrototypes[vehicle.proto_id].veh_name);
        if ((GameState & STATE_SCENARIO_EDITOR) &&
            (GameState & STATE_DEBUG_MODE)) {
            char digits[80];
            strcat(StringTemp, "(");
            _itoa(vehicle_id, digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, ")");
        }
        int line_y = screen->wrap_cent(&font3_, StringTemp, content_x,
                                       content_y + title_offset + 8, content_width);
        if (vehicle.faction_id != 0 &&
            vehicle.faction_id != ConsoleGlobal.faction_id_) {
            StringTemp[0] = '\0';
            strcat(StringTemp, Players[vehicle.faction_id].adj_name_faction);
            line_y = screen->wrap_cent(StringTemp, content_x, line_y,
                                       content_width);
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, "(");
        say_orders(StringTemp, vehicle_id);
        if (vehicle.state & VSTATE_DESIGNATE_DEFENDER) {
            strcat(StringTemp, "#");
        }
        for (int ability = 0; ability < MaxAbilityNum; ++ability) {
            if (has_abil(vehicle.proto_id, 1 << ability)) {
                char *name = StringTable.get(
                    reinterpret_cast<int>(Ability[ability].abbreviation));
                if (*name != '\0') {
                    strcat(StringTemp, ", ");
                    strcat(StringTemp, name);
                }
            }
        }
        strcat(StringTemp, ")");
        line_y = screen->wrap_cent(StringTemp, content_x, line_y, content_width);
        const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
        const bool own_or_omniscient =
            vehicle.faction_id == ConsoleGlobal.faction_id_ ||
            (GameState & STATE_OMNISCIENT_VIEW) != 0;
        const bool known_orders = own_or_omniscient ||
            (PlayersData[ConsoleGlobal.faction_id_]
                 .diplo_treaties[vehicle.faction_id] & 1) != 0;
        if ((vehicle.faction_id != 0 && prototype.plan < 8) ||
            (prototype.plan == PLAN_INFO_WARFARE &&
             morale_veh(vehicle_id, 1, 0) > 1)) {
            StringTemp[0] = '\0';
            say_morale(StringTemp, vehicle_id, 0);
            line_y = own_or_omniscient
                ? screen->wrap(StringTemp, content_x, line_y, content_width)
                : screen->wrap_cent(StringTemp, content_x, line_y, content_width);
        }
        if (own_or_omniscient) {
            StringTemp[0] = '\0';
            if (vehicle.home_base_id >= 0 && vehicle.home_base_id < BaseCurrentCount) {
                strcat(StringTemp, Bases[vehicle.home_base_id].name_string);
            } else {
                strcat(StringTemp, label_get(0x308 / 4));
            }
            line_y = screen->wrap(StringTemp, content_x, line_y, content_width);
            if (vehicle.faction_id == ConsoleGlobal.faction_id_) {
                const int maximum = speed(vehicle_id, 0);
                const int remaining = maximum - vehicle.moves_expended;
                StringTemp[0] = '\0';
                if (remaining < 0 || remaining == 0) {
                    strcat(StringTemp, label_get(0x9e4 / 4));
                    screen->set_text_color(0xf9, -1, 1, 1);
                } else {
                    char digits[80];
                    const int clamped = range(remaining, 0, 999);
                    strcat(StringTemp, label_get(0x1c / 4));
                    strcat(StringTemp, ": ");
                    _itoa(clamped / Rules.move_rate_roads, digits, 10);
                    strcat(StringTemp, digits);
                    const int fraction = clamped % Rules.move_rate_roads;
                    if (fraction != 0) {
                        strcat(StringTemp, " ");
                        _itoa(fraction, digits, 10);
                        strcat(StringTemp, digits);
                        strcat(StringTemp, "/");
                        _itoa(Rules.move_rate_roads, digits, 10);
                        strcat(StringTemp, digits);
                    }
                }
                line_y = screen->wrap(StringTemp, content_x, line_y,
                                      content_width);
            }
            const RulesChassis &chassis = Chassis[prototype.chassis_id];
            if (chassis.range != 0) {
                char digits[80];
                const int movement = speed(vehicle_id, 0);
                const int remaining = range(movement - vehicle.moves_expended,
                                            0, 999);
                const int current =
                    (movement * (chassis.range + (remaining == 0 ? 1 : 0)) -
                     vehicle.terraforming_turns - vehicle.moves_expended) /
                    Rules.move_rate_roads;
                const int maximum = movement * chassis.range /
                                    Rules.move_rate_roads;
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0x30c / 4));
                strcat(StringTemp, ": ");
                const int next_x = screen->write_l(StringTemp, content_x,
                                                   line_y, strlen(StringTemp));
                _itoa(current, digits, 10);
                StringTemp[0] = '\0';
                strcat(StringTemp, digits);
                strcat(StringTemp, "/");
                _itoa(maximum, digits, 10);
                strcat(StringTemp, digits);
                screen->set_text_color(0xfa, -1, 1, 1);
                if ((vehicle.terraforming_turns != 0 &&
                     chassis.range / 2 <= vehicle.terraforming_turns) ||
                    (chassis.range == 1 && remaining <= movement / 2)) {
                    screen->set_text_color(0xfb, -1, 1, 1);
                }
                if (action_home(vehicle_id, -1) != 0) {
                    screen->set_text_color(0xf9, -1, 1, 1);
                }
                screen->write_l(StringTemp, next_x, line_y, strlen(StringTemp));
                line_y += font1_.line_advance();
            }
            const int capacity = veh_cargo(vehicle_id);
            if (capacity != 0) {
                char count_text[32];
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0x360 / 4));
                strcat(StringTemp, ": ");
                _itoa(capacity - stack_veh(vehicle_id, 0), count_text, 10);
                strcat(StringTemp, count_text);
                strcat(StringTemp, "/");
                _itoa(capacity, count_text, 10);
                strcat(StringTemp, count_text);
                line_y = screen->wrap(StringTemp, content_x, line_y,
                                      content_width);
            }
        }
        if (vehicle.dmg_incurred != 0) {
            const int hit_points = prototype.plan == PLAN_ALIEN_ARTIFACT ? 1
                : range(prototype.reactor_id, 1, 100) * 10;
            const int remaining = range(hit_points - vehicle.dmg_incurred,
                                        0, 9999);
            char percent_text[32];
            _itoa((hit_points - remaining) * 100 / hit_points,
                  percent_text, 10);
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0x628 / 4));
            strcat(StringTemp, ": ");
            const int next_x = screen->write_l(StringTemp, content_x,
                                               line_y, strlen(StringTemp));
            screen->set_text_color(0xfa, -1, 1, 1);
            if (hit_points / 3 <= vehicle.dmg_incurred) {
                screen->set_text_color(0xfb, -1, 1, 1);
            }
            if (hit_points * 2 / 3 <= vehicle.dmg_incurred) {
                screen->set_text_color(0xf9, -1, 1, 1);
            }
            StringTemp[0] = '\0';
            strcat(StringTemp, percent_text);
            strcat(StringTemp, "%");
            screen->write_l(StringTemp, next_x, line_y, strlen(StringTemp));
            line_y += font1_.line_advance();
        }
        if ((vehicle.state & VSTATE_DESIGNATE_DEFENDER) && known_orders) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0x1048 / 4));
            screen->set_text_color(0xfb, -1, 1, 1);
            screen->write_l(StringTemp, content_x, line_y, strlen(StringTemp));
            line_y += font1_.line_advance();
        }
        if (known_orders) {
            StringTemp[0] = '\0';
            say_orders(StringTemp, vehicle_id);
            line_y = screen->wrap(StringTemp, content_x, line_y,
                                  content_width);
            if (vehicle.order > 3 && vehicle.order < 0x18) {
                char digits[80];
                const int turns = action_terraform(vehicle_id,
                                                    vehicle.order - 4, 0);
                StringTemp[0] = '\0';
                strcat(StringTemp, "[");
                strcat(StringTemp, label_get(0x71c / 4));
                strcat(StringTemp, ": ");
                _itoa(turns + 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, "]");
                screen->set_text_color(0xfb, -1, 1, 1);
                screen->write_l(StringTemp, content_x, line_y,
                                strlen(StringTemp));
            }
        }
        if (add_spot != 0) {
            spot_.add(vehicle_id, 10, area.left, area.top,
                      content_width, area.bottom - area.top);
        }
        const bool show_destination =
            (vehicle.faction_id == ConsoleGlobal.faction_id_ ||
             (GameState & STATE_OMNISCIENT_VIEW)) &&
            vehicle.order == 0x18 &&
            (GameMorePreferences & MPREF_MAP_HIDE_ACTIVE_VEH_GOTO_PATH) == 0;
        if (show_destination) {
            draw_my_dest(vehicle_id);
            CoverWindow.Win::show(0);
        } else {
            UNK2();
        }
        base_id = -1;
    } else {
        UNK2();
    }

    screen->restore_clip();
    if (base_id >= 0 && base_id < BaseCurrentCount) {
        Base &base = Bases[base_id];
        base_draw(screen, base_id, sprite_x, content_y, -4, 0x40);
        base_draw(screen, base_id, sprite_x, content_y, 0, 0x1800);
        screen->set_font(&font1_, 0, 0, 0);
        StringTemp[0] = '\0';
        strcat(StringTemp, base.name_string);
        int line_y = screen->wrap_cent(&font3_, StringTemp, content_x,
                                       content_y + title_offset + 8, content_width);
        if (base.faction_id_current != ConsoleGlobal.faction_id_) {
            StringTemp[0] = '\0';
            const Player &owner = Players[base.faction_id_current];
            if (Language == 0) {
                strcat(StringTemp, owner.adj_name_faction);
            } else {
                GenderDefault = owner.noun_gender;
                PluralityDefault = owner.is_noun_plural;
                strcat(StringTemp, "(");
                strcat(StringTemp, owner.noun_faction);
                strcat(StringTemp, ")");
            }
            line_y = screen->wrap_cent(StringTemp, content_x, line_y,
                                       content_width);
            StringTemp[0] = '\0';
            const int condition_label = has_fac_built(FAC_HEADQUARTERS,
                                                       base_id) ? 0x5c4 :
                base.population_size < 2 ? 0x2f0 :
                base.population_size > static_cast<int>(Rules.pop_limit_sans_hab_complex)
                    ? 0x570 : 0x2f4;
            strcat(StringTemp, label_get(condition_label / 4));
            line_y = screen->wrap_cent(StringTemp, content_x, line_y,
                                       content_width);
            StringTemp[0] = '\0';
            const int facility_id = has_fac_built(FAC_RECREATION_COMMONS,
                                                  base_id)
                ? FAC_RECREATION_COMMONS
                : has_fac_built(FAC_TACHYON_FIELD, base_id)
                    ? FAC_TACHYON_FIELD : 0;
            if (facility_id != 0) {
                strcat(StringTemp, StringTable.get(
                    reinterpret_cast<int>(Facility[facility_id].name)));
                line_y = screen->wrap_cent(StringTemp, content_x, line_y,
                                           content_width);
            }
            if (base.state & BSTATE_DRONE_RIOTS_ACTIVE) {
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0xb04 / 4));
                screen->set_text_color(0xf9, -1, 1, 1);
                line_y = screen->wrap_cent(StringTemp, content_x, line_y,
                                           content_width);
            }
            if (base.nerve_staple_turns_left != 0) {
                char digits[80];
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(0xb08 / 4));
                strcat(StringTemp, ": ");
                _itoa(base.nerve_staple_turns_left, digits, 10);
                strcat(StringTemp, digits);
                screen->set_text_color(0xfb, -1, 1, 1);
                line_y = screen->wrap_cent(StringTemp, content_x, line_y,
                                           content_width);
            }
        }
        const int owner_id = base.faction_id_current;
        if (owner_id == ConsoleGlobal.faction_id_ ||
            (GameState & STATE_OMNISCIENT_VIEW) ||
            (PlayersData[ConsoleGlobal.faction_id_]
                 .diplo_treaties[owner_id] & 1) != 0 || spying(owner_id)) {
            const int production = base.queue_production_id[0];
            char digits[80];
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0x798 / 4));
            strcat(StringTemp, ": ");
            int production_cost;
            if (production < 0) {
                strcat(StringTemp, StringTable.get(
                    reinterpret_cast<int>(Facility[-production].name)));
                production_cost = Facility[-production].cost;
            } else {
                strcat(StringTemp, VehPrototypes[production].veh_name);
                production_cost = VehPrototypes[production].cost;
            }
            if (production != -69) {
                strcat(StringTemp, " (");
                _itoa(base.minerals_accumulated, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, "/");
                _itoa(cost_factor(owner_id, 1, -1) * production_cost,
                      digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, ")");
            }
            if (StringTemp[0] != '\0') {
                screen->wrap_cent(StringTemp, content_x, line_y,
                                  content_width);
            }
        }
        if (add_spot != 0) {
            spot_.add(base_id, 1, area.left, area.top,
                      content_width, area.bottom - area.top);
        }
    }
    if (add_spot == 0) {
        MainInterfaceGlobal.update(&area, static_cast<GraphicWin *>(0));
    }
}

// ORIGINAL: 0x004B8E40 ?on_left_click@StatusWin@@QAEXHH@Z 0x004B8E40-0x004B8FD3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004b8e40.cpp", "StatusWin spot, click and toggle members in src/statuswin.h", "Console, NetDaemon, Bases and PlayersData owners in src/"], "reason": "Recover the click branches through owned C++ objects and preserve the historical original span."}
void StatusWin::on_left_click(int x, int y) {
    if (fInOnClick_ == 0) {
        ConsoleInfoWin.reset();
        if (field_15D4_ != 0) {
            field_15D4_ = 0;
            sub_interface_global()->release_iface_mode();
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
            const int owner = Bases[veh_id].faction_id_current;
            if ((GameState & STATE_OMNISCIENT_VIEW) != 0
                || owner == ConsoleGlobal.faction_id_
                || spying(owner)
                || (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[owner] & 1) != 0) {
                TutWinBaseWindow.zoom(veh_id, 0);
                return;
            }
            break;
        }
        case 3:
            field_15A8_ = ~field_15A8_;
            draw_status(-1, -1, -1, -1, 0);
            break;
        }
    }
}

// ORIGINAL: 0x004B4A20 ?set_title@StatusWin@@QAEXH@Z 0x004B4A20-0x004B4BE3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004b4a20.cpp", "StringTemp, label_get and vehicle/console declarations in src/", "StatusWin tile members in src/statuswin.h"], "reason": "Recover title assembly using source-owned string, label, console and vehicle state."}
void StatusWin::set_title(int title_id) {
    static const char *const SpaceText = " ";
    static const char *const ColonSpaceText = ": ";
    const unsigned int flags = static_cast<unsigned int>(title_id);
    StringTemp[0] = '\0';

    if (flags != 0) {
        int label_offset;
        if (flags & 1) {
            label_offset = 0x6B4;
        } else if (flags & 0x10) {
            label_offset = 0x6F8;
        } else if (flags & 8) {
            label_offset = 0x6FC;
        } else if (flags & 2) {
            label_offset = 0x6B8;
        } else {
            label_offset = 0x250;
        }
        strcat(StringTemp, label_get(label_offset / 4));
        return;
    }

    strcat(StringTemp, label_get(0x8C / 4));
    if (TurnCurrentNum != 0) {
        strcat(StringTemp, ColonSpaceText);
        int x;
        int y;
        if (ConsoleGlobal.cursor_active_ == 0) {
            strcat(StringTemp, label_get(0xC / 4));
            x = iTileX_;
            y = iTileY_;
        } else {
            strcat(StringTemp, label_get(8 / 4));
            const int veh_id = ConsoleGlobal.selected_vehicle_;
            if (veh_id < 0) {
                return;
            }
            strcat(StringTemp, SpaceText);
            const Veh &veh = Vehs[veh_id];
            x = veh.x;
            y = veh.y;
            strcat(StringTemp, VehPrototypes[veh.proto_id].veh_name);
            strcat(StringTemp, SpaceText);
        }
        strcat(StringTemp, SpaceText);
        say_loc(StringTemp, x, y, ConsoleGlobal.cursor_active_,
                ConsoleGlobal.cursor_active_ == 0, ConsoleGlobal.cursor_active_);
    }
}

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

static const char *vehicle_order_text_id(int id) {
    return StringTable.get(id);
}

static const char *vehicle_order_rule_text(LPSTR encoded_id) {
    return vehicle_order_text_id(static_cast<int>(
        reinterpret_cast<intptr_t>(encoded_id)));
}

// ORIGINAL: 0x004B43E0 ?say_orders@@YAXPADH@Z 0x004B43E0-0x004B496D
// RECOVERY_BINDING: v1 {"evidence": ["git:eff92f20c0edb9c702224c574918f09a95579182:src/recovered/units/004b43e0.cpp: complete original identity/span and historical scaffolding measurement", "Pinned4B5C17 passes StatusWin::draw_active vehicle_id to4B43E0", "96C9B0 is Order[26].order; original ORDERS parser populates this shared record", "Seed g_96c878x/stride12 is Order; g_94615cx/stride8 is Resource.name_plural; g_945d48x/stride4 is parsed Compass", "recovery/tasks/say-orders-owner-oct5.json", "recovery/tests/test_say_orders_actual_source.py"], "reason": "Recover complete vehicle order text builder through genuine vehicle, base, map and parsed rules owners; replace only the old executable forwarder. Native caller passes vehicle_id, not prototype_id. Preserve original branch, buffer and index behavior; parsed Compass/Resource pointer-shaped string IDs retain their current representation pending separate type cleanup."}
// Historical seed observation: complete scaffold measured MISMATCH at3;
// its obsolete tool instructions are retired, not a current body claim.
void __cdecl say_orders(char *text, int veh_id) {
    Veh &vehicle = Vehs[veh_id];
    const uint32_t state = vehicle.state;

    const bool prefix_state = vehicle.waypoint_count <= vehicle.patrol_current_point;
    if ((state & 0x00000200u) != 0
        && (state & 0x01000200u) != 0x01000200u
        && ((state & 0x02000200u) != 0x02000200u || prefix_state)) {
        ::strcat(text, "(");
        if (vehicle.order_auto_type == 10) {
            char prefix[80];
            prefix[0] = '\0';
            ::strcat(prefix, label_get(0x6F0 / 4));
            const int base_id = base_at(vehicle.waypoint_x[1],
                                        vehicle.waypoint_y[1]);
            if (base_id >= 0) {
                parse_says(0, Bases[base_id].name_string, -1, -1);
                char suffix[80];
                parse_string(prefix, suffix);
                ::strcat(text, suffix);
            }
        } else if (vehicle.order_auto_type == 11) {
            ::strcat(text, label_get(0x6F4 / 4));
        } else if (VehPrototypes[vehicle.proto_id].plan == 9
                   && vehicle.order_auto_type != 0) {
            ::strcat(text, label_get((0x62C
                + vehicle.order_auto_type * 4) / 4));
        } else {
            ::strcat(text, label_get(0x62C / 4));
        }
        ::strcat(text, ") ");
    }

    if ((state & 0x00004000u) != 0) {
        ::strcat(text, vehicle_order_rule_text(Order[26].order));
        return;
    }

    if ((state & 0x01000200u) == 0x01000200u) {
        ::strcat(text, label_get(0x65C / 4));
        ::strcat(text, ": ");
        const int count = vehicle.waypoint_count + 1;
        for (int index = 0; index < count; ++index) {
            say_loc(text, vehicle.waypoint_x[index],
                    vehicle.waypoint_y[index], 0, 2, 0);
            if (index < vehicle.waypoint_count) {
                ::strcat(text, " ");
                ::strcat(text, label_get(0x660 / 4));
                ::strcat(text, " ");
            }
        }
        return;
    }

    const int tile_index = vehicle.y * static_cast<int>(MapLongitude)
                        + (vehicle.x >> 1);
    const uint8_t altitude = MapTiles[tile_index].climate & 0xE0;
    const int terrain_row = altitude < 0x60 ? 1 : 0;
    const unsigned int order_index =
        static_cast<unsigned char>(vehicle.order) & 0x3F;
    const RulesOrder &order = Order[order_index];
    const LPSTR order_label = terrain_row ? order.order_sea : order.order;
    ::strcat(text, vehicle_order_rule_text(order_label));

    if (vehicle.order == ORDER_CONVOY) {
        ::strcat(text, " ");
        ::strcat(text, StringTable.get(static_cast<int>(
            reinterpret_cast<intptr_t>(
                Resource[vehicle.order_auto_type].name_plural))));
        const int base_id = base_at(vehicle.x, vehicle.y);
        if (base_id >= 0) {
            ::strcat(text, " ");
            ::strcat(text, label_get(0x6C8 / 4));
            ::strcat(text, " ");
            say_base(text, vehicle.home_base_id);
        } else {
            ::strcat(text, "(");
            const int yield = resource_yield(
                vehicle.order_auto_type, vehicle.faction_id,
                vehicle.home_base_id, vehicle.x, vehicle.y);
            char number[80];
            _itoa(yield, number, 10);
            ::strcat(text, number);
            ::strcat(text, label_get(0x5EC / 4));
            ::strcat(text, ") ");
        }
    }

    if ((static_cast<unsigned char>(vehicle.order) & 0x3F) == ORDER_MOVE_TO) {
        ::strcat(text, " ");
        const bool has_patrol_route =
            (state & 0x02000200u) == 0x02000200u
            && vehicle.patrol_current_point < vehicle.waypoint_count;
        const int location_mode = has_patrol_route ? 2 : 1;
        say_loc(text, vehicle.waypoint_x[0], vehicle.waypoint_y[0],
                0, location_mode, 0);
        if (has_patrol_route && vehicle.waypoint_count != 0) {
            for (int index = 0; index < vehicle.waypoint_count; ++index) {
                ::strcat(text, " ");
                ::strcat(text, label_get(0x664 / 4));
                ::strcat(text, " ");
                say_loc(text, vehicle.waypoint_x[index + 1],
                        vehicle.waypoint_y[index + 1], 0, 2, 0);
            }
        }
    }
    if (vehicle.order == ORDER_MOVE) {
        ::strcat(text, " ");
        ::strcat(text, compass_name(vehicle.waypoint_x[0]));
    }
    if (vehicle.order == ORDER_ROAD_TO || vehicle.order == ORDER_MAGTUBE_TO) {
        ::strcat(text, " ");
        say_loc(text, vehicle.waypoint_x[0], vehicle.waypoint_y[0],
                0, 2, 0);
    }
}

// ORIGINAL: 0x004B6530 ?reset_active@StatusWin@@QAEXXZ 0x004B6530-0x004B6569 BYTE_EXACT
// LEVER: 2026-09-05 homed from 004b6530.cpp once field_15C4_/15C8_ became
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
    if (field_15C4_ >= 0 || field_15C8_ >= 0) {
        int draw_arg = field_15CC_;
        field_15C8_ = -1;
        field_15C4_ = -1;
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
    char *self = reinterpret_cast<char *>(this);
    int size = (WinScreenWidth != 800) ? 12 : 10;
    if (size != *reinterpret_cast<int *>(self + 0x15ac)) {
        *reinterpret_cast<int *>(self + 0x15ac) = size;
        reinterpret_cast<Font *>(self + 0x1530)->init((char *)DefaultFontFace, size, 0);
        reinterpret_cast<Font *>(self + 0x1558)->init((char *)DefaultFontFace, size, 2);
        reinterpret_cast<Font *>(self + 0x1580)->init((char *)DefaultFontFace, size, 1);
    }
    if (WinScreenWidth != 800) {
        *reinterpret_cast<int *>(self + 0x1530) = 4;
        *reinterpret_cast<int *>(self + 0x1558) = 4;
        *reinterpret_cast<int *>(self + 0x1580) = 4;
    }
}

// Fixed-slot bindings carried from 004b8890.cpp. All of them already have
// names in the tree - DefaultFontFace and WinScreenWidth in font.h/win.h, the
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
    char *self = reinterpret_cast<char *>(this);
    int fontSize = ((WinScreenWidth != 0x320) ? 2 : 0) + 10;

    if (fontSize != *reinterpret_cast<int *>(self + 0x15ac)) {
        *reinterpret_cast<int *>(self + 0x15ac) = fontSize;
        reinterpret_cast<Font *>(self + 0x1530)->init(DefaultFontFace, fontSize, 0);
        reinterpret_cast<Font *>(self + 0x1558)->init(DefaultFontFace, fontSize, 2);
        reinterpret_cast<Font *>(self + 0x1580)->init(DefaultFontFace, fontSize, 1);
    }

    if (WinScreenWidth != 0x320) {
        *reinterpret_cast<int *>(self + 0x1530) = 4;
        *reinterpret_cast<int *>(self + 0x1558) = 4;
        *reinterpret_cast<int *>(self + 0x1580) = 4;
    }

    reinterpret_cast<StatusWin *>(self)->draw_status(-1, -1, -1, -1, 0);

    RECT rect;
    UnionRect(&rect, &MainInterfaceGlobal.status_hit_rect(),
              &MainInterfaceGlobal.status_right_rect());
    UnionRect(&rect, &rect, &MainInterfaceGlobal.status_left_rect());

    MainInterfaceGlobal.soft_update(&rect);
}

/*
// ORIGINAL: 0x004B9E90 ?UNK4@StatusWin@@QAEXXZ 0x004B9E90-0x004B9E97
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

// ORIGINAL: 0x004B8DF0 ?UNK3@StatusWin@@QAEXXZ 0x004B8DF0-0x004B8E31 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The historical body tests StatusWin offsets +0x15BC and +0x15C0; statuswin.h maps these to iUnitIndex_ and iCityIndex_.", "The store at 0x0093928C is ConsoleGlobal.selected_vehicle_ (Console offset +0x23BDC), and the remaining dispatches match veh_wake, Console::set_move, and TutWinBaseWindow.zoom.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/004b8df0.cpp"], "reason": "Home the compact selected-unit/base action in StatusWin using the recovered members and process-owned C++ objects. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void StatusWin::UNK3() {
    if (iUnitIndex_ >= 0) {
        veh_wake(iUnitIndex_);
        ConsoleGlobal.selected_vehicle_ = iUnitIndex_;
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
// ORIGINAL: 0x004B89C0 ?arrow_left@StatusWin@@QAEXXZ 0x004B89C0-0x004B8BC5
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004b89c0.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_statuswin_remaining_behavior.py"], "reason": "Home the complete typed StatusWin arrow_left operation. Retain the original seed and historical obligations; this product has no new measured claim."}
void StatusWin::arrow_left() {
    if (iUnitIndex_ == -1) {
        return;
    }

    const int previous = Vehs[iUnitIndex_].prev_veh_id_stack;
    if (previous >= 0) {
        iUnitIndex_ = previous;
    } else {
        const int base_id = base_at(iTileX_, iTileY_);
        if (base_id < 0) {
            return;
        }
        iCityIndex_ = base_id;
        iUnitIndex_ = -1;
    }

    const int font_height = WinScreenWidth == 0x320 ? 10 : 12;
    if (font_height != field_15AC_) {
        field_15AC_ = font_height;
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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004B8BD0 ?arrow_right@StatusWin@@QAEXXZ 0x004B8BD0-0x004B8DF0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004b8bd0.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_statuswin_remaining_behavior.py"], "reason": "Home the complete typed StatusWin arrow_right operation. Retain the original seed and historical obligations; this product has no new measured claim. Historical seed obligation is carried by this product; current restoration is outstanding."}
void StatusWin::arrow_right() {
    bool changed = false;
    if (iCityIndex_ >= 0) {
        const int vehicle_id = veh_at(iTileX_, iTileY_);
        if (field_15DC_ != 0 && vehicle_id >= 0) {
            iCityIndex_ = -1;
            iUnitIndex_ = vehicle_id;
            changed = true;
        }
    } else if (iUnitIndex_ >= 0) {
        const int next = Vehs[iUnitIndex_].next_veh_id_stack;
        if (next >= 0) {
            iUnitIndex_ = next;
            changed = true;
        }
    }
    if (!changed) {
        return;
    }

    const int font_height = WinScreenWidth == 0x320 ? 10 : 12;
    if (font_height != field_15AC_) {
        field_15AC_ = font_height;
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
    field_15C8_ = -1;
    field_15C4_ = -1;
    field_15D0_ = -1;
    field_15CC_ = -1;
    field_15AC_ = -1;
    field_15D4_ = 0;

    if ((StatusFlags & 0x20u) == 0) {
        StatusFlags |= 0x20u;
    }

    spot_.init(0x18);
    caviar_.init();
    caviar_.set_field_A4(0);

    camera_angles_.values[0] = 0.0f;
    const uint32_t initial_angle_bits = 0x3F48F5C3u;
    memcpy(&camera_angles_.values[1], &initial_angle_bits,
           sizeof(initial_angle_bits));
    camera_angles_.values[2] = 0.0f;

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

    caviar_.light_vector().values[0] = 0.0f;
    caviar_.light_vector().values[1] = 0.0f;
    caviar_.light_vector().values[2] = 1.0f;
    caviar_.shadow_vector().values[0] = 0.0f;
    caviar_.shadow_vector().values[1] = 0.0f;
    caviar_.shadow_vector().values[2] = 1.0f;
}

// ORIGINAL: 0x0050F630 sub_50f630 0x0050F630-0x0050F640 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:5167ec56aaa9031de73f42c05fbc3bdd0bad0201:src/recovered/0050f630.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x50f630 complete original body; independent source-owner review.", "source:src/statuswin.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_50f630@@YGXH@Z
void __stdcall sub_50f630(int unused) {
    (void)unused;
    ConsoleStatusWin.redraw();
}

// ORIGINAL: 0x00515430 sub_515430 0x00515430-0x00515443 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/00515430.cpp", "task:owned-helper-coverage-batch", "StatusFlags939ED8 is already owned by statuswin.cpp; native515430 clears it before desktop_init(1)."], "reason": "Recover a complete native helper through genuine source owners and SDK calls, preserving original identity, extent and historical obligations. Source home is not proof of original TU or complete-game acceptance."}
// symbol    ?reset_status_and_initialize_desktop@@YAHXZ
int __cdecl reset_status_and_initialize_desktop() {
    StatusFlags = 0;
    return desktop_init(1);
}
