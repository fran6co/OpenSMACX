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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original destructor span and its direct close target.", "Original model arrays and renderer embedding EH cleanup distinguish loaded-model and instance lifetimes.", "Fresh VC6 observations under both configured flag sets identify this natural emission host and preserve normal versus EH call targets."], "reason": "Genuine header-defined destructor in src/caviar.h; this translation unit naturally emits the original five-byte cleanup tail through real object lifetime use, without synthetic calls or forced exports."}
// body      src/caviar.h
#include "stdafx.h"
#include "base.h"
#include "original_seam.h"
#include "strings.h"  // StringTemp, 0x009B86A0
#include "statuswin.h"
#include "spritebox.h"
#include "font.h"
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

void __cdecl help_any();

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
        (ConsoleControlTurnActive || NetDaemonNet.lock_state_[2].dword_[4]
         || NetDaemonNet.lock_state_[0].dword_[3]);
    bool faction_blocked = IsMultiplayerNet &&
        (NetDaemonNet.lock_state_[0].dword_[12] & (1U << ConsoleGlobal.faction_id_));

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

void StatusWin::draw_status(int a1, int a2, int a3, int a4, int a5) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

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

// Two StatusWin members the image's click handlers need by name, both still
// original bodies this build does not own: draw_active (0x004B4CF0, 6203
// bytes, src/unrecovered/004b4cf0.cpp) and on_left_click (0x004B8E40, 403
// bytes with a jump table, src/recovered/units/004b8e40.cpp). Neither carries
// a BYTE_EXACT claim, so a forwarder is allowed; homing them retires these.
// reset_active and on_left_double_click call them BY NAME.

void StatusWin::draw_active(int first, int second, int flag) {  // 0x004B4CF0
    typedef void(__fastcall *pending)(StatusWin *, void *, int, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x004B4CF0))(
        this, nullptr, first, second, flag);
}

void StatusWin::on_left_click(int x, int y) {  // 0x004B8E40
    typedef void(__fastcall *pending)(StatusWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x004B8E40))(
        this, nullptr, x, y);
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

// ?say_orders@@YAXPADH@Z at 0x004B43E0, 1421 bytes - still an original body
// (src/recovered/units/004b43e0.cpp), forwarded here because the int form
// above calls it by name. It carries no claim, so a forwarder is allowed.
void say_orders(char *text, int veh_id) {  // 0x004B43E0
    typedef void(__cdecl *pending)(char *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x004B43E0))(text,
                                                                     veh_id);
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
//        hid its meaning. Named here as StatusFlagsPtr (statuswin.h).
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
    *StatusFlagsPtr |= 0x20;
}

// ORIGINAL: 0x004B8DF0 ?UNK3@StatusWin@@QAEXXZ 0x004B8DF0-0x004B8E31
// RECOVERY_BINDING: v1 {"evidence": ["The historical body tests StatusWin offsets +0x15BC and +0x15C0; statuswin.h maps these to iUnitIndex_ and iCityIndex_.", "The store at 0x0093928C is ConsoleGlobal.selected_vehicle_ (Console offset +0x23BDC), and the remaining dispatches match veh_wake, Console::set_move, and TutWinBaseWindow.zoom."], "reason": "Home the compact selected-unit/base action in StatusWin using the recovered members and process-owned C++ objects."}
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
