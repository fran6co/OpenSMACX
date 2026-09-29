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
#include "maininterface.h"
#include "messagewin.h"
#include "worldwin.h"
#include "graphicwin.h"
#include "filewin.h"
// Homed MainInterface bodies call the free helpers the archive binds by
// address: say_year (game.h), base_at (base.h). veh.h is left out on
// purpose - its `range` collides with general.h's, and the two headers
// cannot both be read here.
#include "game.h"
#include "base.h"
#include "map.h"  // WorldClimateSkipTerrainClear
#include "statuswin.h"
#include "infowin.h"
#include "console.h"   // ConsoleStatusWin, 0x008C5568
#include "alpha.h"     // label_get, the Labels.strings_ptr walk

 /*
 Purpose: Set the flat button's bubble text.
 Original Offset: n/a
 Return Value: Zero on success, non-zero on error
 Status: Complete
 */
int MainInterface::set_bubble_text(uint32_t button_id, LPCSTR text) {
    return flat_button_[button_id].set_bubble_text(text);
}

/*
Purpose: Clear the interface message; the legacy implementation is intentionally empty.
// ORIGINAL: 0x0045C170 ?clear_message@MainInterface@@QAEXXZ 0x0045C170-0x0045C171 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?clear_message@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void MainInterface::clear_message() {
}

/*
Purpose: Update the desktop; the legacy implementation is intentionally empty.
// ORIGINAL: 0x0058EE50 ?desktop_update@@YAXXZ 0x0058EE50-0x0058EE51 BYTE_EXACT
// size      1 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_update() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045C5B0 ?UNK2@MainInterface@@QAEXXZ 0x0045C5B0-0x0045C5B1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK2@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045EB60 ?UNK3@MainInterface@@QAEXXZ 0x0045EB60-0x0045EB61 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK3@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK3() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045EB70 ?UNK4@MainInterface@@QAEXXZ 0x0045EB70-0x0045EB71 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK4@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK4() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045C280 ?UNK1@MainInterface@@QAEXH@Z 0x0045C280-0x0045C283 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK1@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK1(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CC80 ?on_iface_scrolled@MainInterface@@QAEXHH@Z 0x0045CC80-0x0045CC83 BYTE_EXACT
// symbol    ?on_iface_scrolled@MainInterface@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_scrolled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_scrolled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CC90 ?on_iface_scrolling@MainInterface@@QAEXHH@Z 0x0045CC90-0x0045CC93 BYTE_EXACT
// symbol    ?on_iface_scrolling@MainInterface@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_scrolling@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_scrolling(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CCA0 ?on_iface_button_toggled@MainInterface@@QAEXHH@Z 0x0045CCA0-0x0045CCA3 BYTE_EXACT
// symbol    ?on_iface_button_toggled@MainInterface@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CDB0 ?on_iface_right_double_click@MainInterface@@QAEXHH@Z 0x0045CDB0-0x0045CDB3 BYTE_EXACT
// symbol    ?on_iface_right_double_click@MainInterface@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x0045D160 ?on_iface_button_pressed@MainInterface@@QAEHH@Z 0x0045D160-0x0045D165 BYTE_EXACT
// symbol    ?on_iface_button_pressed@MainInterface@@UAEHH@Z
// size      5 bytes
// prototype int (__thiscall ?on_iface_button_pressed@MainInterface@@QAEHH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int MainInterface::on_iface_button_pressed(int) {
    return 0;
}

// ORIGINAL: 0x0045CD80 ?on_iface_left_double_click@MainInterface@@QAEXHH@Z 0x0045CD80-0x0045CDB0 BYTE_EXACT
// symbol    ?on_iface_left_double_click@IfaceRegistryEntry@@UAEXHH@Z
// Original boundary evidence: the return at 0x0045CDAD ends at 0x0045CDB0;
// the separate right-double-click entry at 0x0045CDB0 has its own return.
// The former exclusive end 0x0045CDB1 cut into that next instruction.
// LEVER: 2026-09-05 homed from 0045cd80.cpp, the second of the wave-4
//   handoffs: it needs StatusWin::on_left_double_click (0x004B8990) as a real
//   member, which statuswin.cpp now owns. `mov ecx, 0x8c5568` falls out of
//   naming ConsoleStatusWin (console.h, real storage in statuswin.cpp) and
//   `push 0x7af50c` out of StatusWinRedrawRect1 (statuswin.h).
// size      48 bytes
// prototype void (__thiscall ?on_iface_left_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B8990 0x005FA7E0
void IfaceRegistryEntry::on_iface_left_double_click(int x, int y) {
    if (in_box(x, y, &MainInterfaceGlobal.status_hit_rect())) {
        ConsoleStatusWin.on_left_double_click(x, y);
    }
}

// THE RECEIVER MAP, RESOLVED. Every body below this line until the next
// non-iface method takes the IfaceRegistryEntry subobject as `this`, not the
// window front - so the members it names compile base-relative:
//
//     spot_                  ->  [esi + 0x24620]   (front + 0x25034)
//     iface_redraw_pending_  ->  [esi + 0x20CD8]   (front + 0x216EC)
//     redraw_complete()      ->  lea ecx,[esi - 0xA14] then call
//
// That is not a spelling this tree chose. VC6 12.00.8168 compiles an override
// of a SECONDARY base's virtual with the base subobject as `this`, puts the
// override's raw address in the secondary vftable, and emits no adjustor
// thunk - probed against a GraphicWin-sized first base under /O2 /Gy /GR- /GX
// on 2026-09-05. The image is the same shape: the handlers read the entry-
// relative offsets above, the table at 0x66C544 holds the raw bodies, and
// .text has no `sub ecx,0xA14` thunk anywhere. See maininterface.h.

// Fixed-slot bindings carried from the 0045c* artifacts, read out of the
// shipped image. The pair 0x008C6B30 / 0x008C6B2C is the status bar's hover
// state: veh.cpp already names 0x008C6B2C StatusHoverVehId (it is the id the
// map cursor is over, and veh_kill compacts it), so that name is repeated here
// rather than a second one being invented for the same dword. 0x008C6B48 is
// the second Spot the handler tests the cursor against.
static int *const StatusHoverBaseId = (int *)0x008C6B30;   // hovered base id
static int *const StatusHoverVehId = (int *)0x008C6B2C;    // hovered vehicle id
static Spot *const IfaceCursorSpot = (Spot *)0x008C6B48;   // the status Spot


// ORIGINAL: 0x0045D110 ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z 0x0045D110-0x0045D13F BYTE_EXACT
// symbol    ?on_iface_mouse_leave@MainInterface@@UAEXHH@Z
// LEVER: 2026-09-05 the receiver map. This was the body that named the wall:
//   as a MainInterface method with field_A14_/field_A18_ standing in for the
//   SubInterface subobject there was no spelling for `cmp [esi+0x20cd8],1` at
//   all - a downcast folded the displacement to 0x216EC and cost the match.
//   Declaring IfaceRegistryEntry as MainInterface's second base is what makes
//   it fall out: `this` is the entry, iface_redraw_pending_ is entry-relative,
//   and the unqualified redraw_complete() call is where VC6's own -0xA14 goes.
//   Neither argument is read. ConsoleStatusWin.reset_active is the real
//   0x004B6530, homed in statuswin.cpp.
// size      47 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045C3A0 0x004B6530
void MainInterface::on_iface_mouse_leave(int, int) {
    ConsoleStatusWin.reset_active();
    if (iface_redraw_pending_ == 1) {
        iface_redraw_pending_ = 0;
        redraw_complete();
    }
}

// PENDING 0x0045CF40 ?on_iface_left_down@MainInterface@@QAEXHH@Z 0x0045CF40-0x0045CF75
// symbol    ?on_iface_left_down@MainInterface@@UAEXHH@Z
// PENDING BODY - the receiver is solved (spot_ is entry+0x24620, so `add ecx,
//   0x24620` is `spot_.check(...)`) and the body is one line and a guard, but
//   it ends in Console::end_my_turn (0x005169F0) and console.h does not
//   declare that method yet. HANDOFF to console.h: name end_my_turn and this
//   becomes `int result = spot_.check(a1, a2, &a2, 0); if (result != -1 &&
//   a2 == 5) ConsoleGlobal.end_my_turn();`
// size      53 bytes
// prototype void (__thiscall ?on_iface_left_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005169F0 0x005FAB00
void MainInterface::on_iface_left_down(int a1, int a2) {
    typedef void(__fastcall *pending)(MainInterface *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0045CF40))(
        this, nullptr, a1, a2);
}

// ORIGINAL: 0x0045CDC0 ?on_iface_left_click@MainInterface@@QAEXHH@Z 0x0045CDC0-0x0045CE72 BYTE_EXACT
// symbol    ?on_iface_left_click@MainInterface@@UAEXHH@Z
// The first three hit regions belong to MainInterfaceGlobal; the last
// is InfoWin's union rectangle, established by its init at 0x004592A0.
// The original interface-receiver +0x1DEE8 is complete-object +0x1E8FC:
// button_group_[0] at +0x1E878, with its still-unnamed word at +0x84.
// size      178 bytes
// prototype void (__thiscall ?on_iface_left_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458930 0x004B8E40 0x005FA7E0
void MainInterface::on_iface_left_click(int x, int y) {
    if (in_box(x, y, &MainInterfaceGlobal.status_left_rect())) {
        ConsoleStatusWin.on_left_click(x, y);
        return;
    }
    if (in_box(x, y, &MainInterfaceGlobal.status_hit_rect())) {
        ConsoleStatusWin.on_left_click(x, y);
        return;
    }
    if (in_box(x, y, &MainInterfaceGlobal.status_right_rect())) {
        ConsoleStatusWin.on_left_click(x, y);
        return;
    }
    if (in_box(x, y, &ConsoleInfoWin.bounds()) &&
        button_group_[0].field_84_ == 0x3E9) {
        ConsoleInfoWin.on_left_click(x, y);
    }
}

// ORIGINAL: 0x0045CE80 ?on_iface_right_click@MainInterface@@QAEXHH@Z 0x0045CE80-0x0045CF38 BYTE_EXACT
// symbol    ?on_iface_right_click@MainInterface@@UAEXHH@Z
// Each hit returns independently. The middle StatusWin region selects 1;
// its two neighbors select 0. InfoWin owns the final hit rectangle.
// The ret 8 at 0x0045CF35 ends at 0x0045CF38, not the old pending extent.
// size      184 bytes
// prototype void (__thiscall ?on_iface_right_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458D40 0x004B9010 0x005FA7E0
void MainInterface::on_iface_right_click(int x, int y) {
    if (in_box(x, y, &MainInterfaceGlobal.status_left_rect())) {
        ConsoleStatusWin.on_right_click(x, y, 0);
        return;
    }
    if (in_box(x, y, &MainInterfaceGlobal.status_hit_rect())) {
        ConsoleStatusWin.on_right_click(x, y, 1);
        return;
    }
    if (in_box(x, y, &MainInterfaceGlobal.status_right_rect())) {
        ConsoleStatusWin.on_right_click(x, y, 0);
        return;
    }
    if (in_box(x, y, &ConsoleInfoWin.bounds()) &&
        button_group_[0].field_84_ == 0x3E9) {
        ConsoleInfoWin.on_right_click(x, y);
    }
}

// PENDING 0x0045CCB0 ?on_iface_group_clicked@MainInterface@@QAEXHHH@Z 0x0045CCB0-0x0045CD75
// symbol    ?on_iface_group_clicked@MainInterface@@UAEXHHH@Z
// PENDING BODY - the receiver is solved and the switch is transcribable, but
//   its `a1 == 0` arm calls the front's PRIMARY vtable slot 12, which is
//   on_redraw (0x0045BC50, 2274 lines, unhomed) and the call is
//   `mov edx,[ecx-0xa14] / add ecx,-0xA14 / call [edx+0x30]` - a dispatch, so
//   on_redraw would have to be virtual to reproduce it. HANDOFF: home
//   on_redraw first, decide virtuality second; this then spells directly.
//   Everything else in the arm is named - ConsoleGlobal.editor_paints,
//   MainInterfaceGlobal.draw_edit_pane, WorldWin's two map setters.
// size      197 bytes
// prototype void (__thiscall ?on_iface_group_clicked@MainInterface@@QAEXHHH@Z)(MainInterface* this, int, int, int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045BC50 0x0045B550 0x004C3990 0x004C3950 0x004E0A50
void MainInterface::on_iface_group_clicked(int a1, int a2, int a3) {
    typedef void(__fastcall *pending)(MainInterface *, void *, int, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0045CCB0))(
        this, nullptr, a1, a2, a3);
}

// ORIGINAL: 0x0045CF80 ?on_iface_right_down@MainInterface@@QAEXHH@Z 0x0045CF80-0x0045CFFD
// SPAN CORRECTED from the catalogue's 0x0045D0F5 (size 0x175): that span runs
// over on_iface_mouse_move's own record at 0x0045D000 - two catalogue entries
// cannot overlap, and the image's right_down body ends at 0x0045CFFD with the
// rest alignment padding into the next function. The old span could never
// measure BYTE_EXACT because its tail compared against mouse_move's bytes.
// symbol    ?on_iface_right_down@MainInterface@@UAEXHH@Z
// LEVER: 2026-09-05 the receiver map. spot_ is entry-relative; the bubble text
//   goes through the FRONT, which is what `Win *front = this` is for - inside
//   an override of a secondary base's virtual the conversion emits the
//   compiler's own -0xA14. Labels [+0xC4C] is the string id, StringTable the
//   table it indexes.
// size      373 bytes
// prototype void (__thiscall ?on_iface_right_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00 0x00617940 0x00617C70
void MainInterface::on_iface_right_down(int a1, int a2) {
    // BUG IN THE ORIGINAL: spotType is handed to Spot::check UNINITIALIZED -
    // the image's `lea eax,[ebp-4]` takes its address before any store, and
    // only the callee's write makes the `cmp dword ptr [ebp-4],5` below mean
    // anything. Spelling the `= 0` a natural reading wants adds a store the
    // image does not have, so it is left exactly as shipped.
    int spotType;
    int result = spot_.check(a1, a2, &spotType, 0);
    if (result != -1 && spotType == 5) {
        Win::client_to_screen(&a1, &a2);
        RECT rect;
        rect.left = a1;
        rect.top = a2;
        rect.right = a1;
        rect.bottom = a2;
        Win::set_bubble_text(label_get(0x313), &rect);
    }
}

// ORIGINAL: 0x0045D000 ?on_iface_mouse_move@MainInterface@@QAEXHH@Z 0x0045D000-0x0045D10F
// symbol    ?on_iface_mouse_move@MainInterface@@UAEXHH@Z
// LEVER: 2026-09-05 the receiver map. Both spot_ and iface_redraw_pending_ are
//   entry-relative and the two redraw_complete calls are the compiler's own
//   -0xA14. The status bar's two ids are the 0x008C6B2C/0x008C6B30 pair, and
//   the second Spot is at 0x008C6B48.
// size      271 bytes
// prototype void (__thiscall ?on_iface_mouse_move@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00 0x004B4CF0 0x0045C3A0 0x004B6530
void MainInterface::on_iface_mouse_move(int a1, int a2) {
    if (spot_.check(a1, a2, &a2, &a1) != -1 && a2 == 5) {
        if (iface_redraw_pending_ != 0) {
            return;
        }
        iface_redraw_pending_ = 1;
        redraw_complete();
        return;
    }

    int check2 = IfaceCursorSpot->check(a1, a2, &a2, &a1);
    if (check2 >= 0 && a1 == 0xb) {
        if (a2 != *StatusHoverBaseId) {
            *StatusHoverBaseId = a2;
            *StatusHoverVehId = -1;
            ConsoleStatusWin.draw_active(-1, a2, 0);
        }
    } else if (check2 >= 0 && a1 == 0) {
        if (a2 != *StatusHoverVehId) {
            *StatusHoverBaseId = -1;
            *StatusHoverVehId = a2;
            ConsoleStatusWin.draw_active(a2, -1, 0);
        }
    } else {
        ConsoleStatusWin.reset_active();
    }

    if (iface_redraw_pending_ == 1) {
        iface_redraw_pending_ = 0;
        redraw_complete();
    }
}

// PENDING 0x0045CA40 ?on_status@MainInterface@@QAEXH@Z 0x0045CA40-0x0045CC78
// symbol    ?on_status@MainInterface@@UAEXH@Z
// PENDING BODY - slot 0 of the interface table, 568 bytes, and the reason
//   IfaceRegistryEntry's slot 0 links at all. It takes the ENTRY as `this`
//   like the rest of the table: `lea esi,[edi+0x3ec]` is flat_button_[0] and
//   `lea ecx,[edi-0xa14]` is the front, both read off edi = the subobject.
//   The artifact at src/recovered/units/0045ca40.cpp is a raw-offset
//   transcription written against the front, so it cannot be dropped in
//   until its offsets are rebased onto the entry.
// size      568 bytes
// prototype void (__thiscall ?on_status@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00403830 0x0045BC50 0x0050F440 0x005D6310 0x00616730
void MainInterface::on_status(int a1) {
    typedef void(__fastcall *pending)(MainInterface *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0045CA40))(this,
                                                                      nullptr,
                                                                      a1);
}

// 0x0045D170 ?on_iface_button_clicked@MainInterface@@QAEXH@Z stays in
// pending_bodies.cpp, which already defines the override as a forwarder: the
// 257-byte switch over the five button ids (0x3EB menu, 0x3EC multiplayer,
// 0x3F0/0x3F1 demote, 0x3F2 promote) reaches three globals this tree has not
// named yet (0x009380DC, 0x007FD648, 0x00952858), and veh.h cannot be read
// here - its `range` collides with general.h's. It touches no member, so the
// entry-vs-front receiver question does not arise.

// Eight faction foreground colors, independently paired with eight shadows.
// load_faction_art4537D9/4537E8 writes the two banks and load_vehicles
// 45443C/454446 publishes entry0. The following 68FA70 string is NOT an
// array tail. Original chat caller537AD5 passes the packet faction index;
// malformed packet indices remain unvalidated, not a larger storage owner.
// Other original-only color users and load_faction_art still need recovery.
// RECOVERY_OBJECT: v2 0x0068FA30 0x0068FA50 ?ChatItemIdTable@@3PAHA 0x00454270
int ChatItemIdTable[8] = {
    249, 111, 252, 255, 251, 65, 102, 187,
};
// RECOVERY_OBJECT: v2 0x0068FA50 0x0068FA70 ?ChatShadowColors@@3PAHA 0x00454270
int ChatShadowColors[8] = {
    134, 177, 172, 55, 146, 51, 145, 169,
};

// ORIGINAL: 0x0045C0F0 ?set_message@MainInterface@@QAEXPADHHHH@Z 0x0045C0F0-0x0045C163;0x00655745-0x00655757 BYTE_EXACT
// LEVER: 2026-09-05 homed from 0045c0f0.cpp, exactly as that record's own
//   LEVER line says: the LOCAL `Font font;` is what earns the SEH frame
//   (`push -1 / push 0x65574d / mov fs:[0]`) - a non-trivially-destructible
//   automatic under /GX - and direct calls to string_box_.add and the real
//   InterfaceMessageWin reproduce the rest. Nothing names the three ints.
//   `ret 0x14` pops this plus four arguments, so the signature is four ints
//   after the text, exactly as the mangled name spells - none of them read.
// size      114 bytes
// prototype void (__thiscall ?set_message@MainInterface@@QAEXPADHHHH@Z)(MainInterface* this, int8*, int, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00471530 0x00618EA0 0x00618EE0 0x00629710
void MainInterface::set_message(char *text, int, int, int, int) {
    Font font;
    string_box_.add(text, 0x1388, 2);
    InterfaceMessageWin.add_message(text, 0);
}

// ORIGINAL: 0x0045C0B0 ?chat@MainInterface@@QAEXPADH@Z 0x0045C0B0-0x0045C0E6 BYTE_EXACT
// LEVER: 2026-09-05 homed from 0045c0b0.cpp. No local Font here and so no SEH
//   frame - the body is one straight line. `mov ecx, 0x7f67f8` is the real
//   InterfaceMessageWin, and `[eax*4 + 0x68fa30]` is ChatItemIdTable indexed
//   by the channel, which VC6 folds into the SIB addressing on its own.
// size      53 bytes
// prototype void (__thiscall ?chat@MainInterface@@QAEXPADH@Z)(MainInterface* this, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00471530 0x00629710
void MainInterface::chat(char *text, int channel) {
    string_box_.add(text, 0x2710, 2);
    InterfaceMessageWin.add_message(text, ChatItemIdTable[channel]);
}

// Fixed-slot bindings carried from 0045c290.cpp
static int *const InterfaceBuffer = (int *)0x007AEC64;
static int *const InterfaceClipRect = (int *)0x007AEC94;
// RECOVERY_OBJECT: v2 0x007D3930 0x007D3934 ?MainInterfaceTileFlag@@3HA 0x0045C290
int MainInterfaceTileFlag;

// ORIGINAL: 0x0045C290 ?restore_back@MainInterface@@QAEXH@Z 0x0045C290-0x0045C393 FILE
// size      259 bytes
// prototype void (__thiscall ?restore_back@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   12   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DA860
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C290
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c290/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?restore_back@MainInterface@@QAEXH@Z  at 0x0045C290  (259 bytes)
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


void MainInterface::restore_back(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 != 0) {
        reinterpret_cast<Buffer *>(InterfaceBuffer)->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));
    }

    {
        int *r = reinterpret_cast<int *>(self + 0xd80);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(InterfaceBuffer)->tile(InterfaceBoxStyles[9].mask_, x, y, x, y, w, h);
    }

    {
        int *r = MainInterfaceTileFlag != 0
            ? reinterpret_cast<int *>(self + 0xdd0)
            : reinterpret_cast<int *>(self + 0xdb0);
        int x = r[0];
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(InterfaceBuffer)->tile(InterfaceBoxStyles[9].mask_, x, y, x, y, w, h);
    }

    {
        int *r = MainInterfaceTileFlag != 0
            ? reinterpret_cast<int *>(self + 0xdc0)
            : reinterpret_cast<int *>(self + 0xda0);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(InterfaceBuffer)->tile(InterfaceBoxStyles[9].mask_, x, y, x, y, w, h);
    }

    {
        int *r = reinterpret_cast<int *>(self + 0xd90);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(InterfaceBuffer)->tile(InterfaceBoxStyles[9].mask_, x, y, x, y, w, h);
    }
}


/*
// ORIGINAL: 0x0045C180 ?timer_callback_daemon@MainInterface@@QAEXH@Z 0x0045C180-0x0045C1F4 FILE
// TRIED: the scaffold bound the IAT slot as `static int *const g_00669328`
// and dispatched through its own `UnionRectProc` typedef; the slot is
// USER32!UnionRect (tools/iat_names.py), and windows.h already declares it, so
// TRIED: 2026-09-03 homing pass. The body is faithful - alert_/time1_/time2_
//        by name, the switch and the UnionRect call all land - and it still
//        measures MISMATCH 6/42, because the image CALLS Time::stop
//        (0x00616730) here while time.h keeps `stop` in-class MEASURED so
//        that `close` and `~Time` inline it. VC6 has no __declspec(noinline),
//        so the one source cannot serve both call sites: this tree expands
//        the whole teardown (the `not edi / and edi, 1` flush test) in both
//        switch arms, 80 instructions against the image's 42. Same ceiling
//        time.h already documents for port_to_port / has_tech /
//        do_all_non_input. Fixing it means splitting `stop` into an in-class
//        inline for `close`/`~Time` and an out-of-line definition for the
//        daemon callers - time.h, not this TU.
// TRIED: the scaffold bound the IAT slot as `static int *const g_00669328`
// `call dword ptr [__imp_UnionRect]`. The offsets the scaffold cast through
// are the declared members alert_ / time1_ / time2_
// (tools/header_offsets.py: 0x216f0 / 0x25040 / 0x25068), so the body reads
// them by name now.
*/
void MainInterface::timer_callback_daemon(int a1) {
    RECT local;
    switch (a1) {
    case 0:
        time1_.stop();
        break;
    case 1:
        if (alert_ == -1) {
            time2_.stop();
            return;
        }
        alert_ += 1;
        UnionRect(&local,
                  reinterpret_cast<RECT *>(&field_DE0_),
                  reinterpret_cast<RECT *>(&field_DF0_));
        GraphicWin::soft_update(&local);
        break;
    }
}

// Fixed-slot bindings carried from 0045c5c0.cpp. IfaceRegistry is typed as
// the registry entry the receiver map in maininterface.h names, so the
// dispatch is a call by name instead of a shim.
// The original id-to-slot run ends where the separate registry begins.
// Bind its start; the exact historical allocation remains to be checked.
// RECOVERY_OBJECT: v1 0x007AE778 ?IfaceIdToSlotStorage@@3PAHA
int IfaceIdToSlotStorage[30];
static int *const IfaceRegistry = (int *)0x007AE7F0;
// RECOVERY_OBJECT: v1 0x007D392C ?ActiveIfaceIdStorage@@3HA
int ActiveIfaceIdStorage;

bool __cdecl active_interface_slot_occupied() {
    return IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0;
}

// The MessageWin the interface events are delivered to is now the real global
// InterfaceMessageWin (messagewin.h / messagewin.cpp) - game.cpp's system_init
// builds the same object through messagewin_init, and the image's handlers
// reach it by its address, `mov ecx, 0x7f67f8` before the call.

// ORIGINAL: 0x0045D140 ?on_iface_selected@MainInterface@@QAEXHH@Z 0x0045D140-0x0045D159 BYTE_EXACT
// symbol    ?on_iface_selected@IfaceRegistryEntry@@UAEXHH@Z
// LEVER: 2026-09-05 homed from 0045d140.cpp once InterfaceMessageWin became a
//   real global (messagewin.h / messagewin.cpp). The body is one line - the
//   handoff to the desktop's MessageWin - and the receiver never touches its
//   own SubInterface subobject, so it is byte-exact homed on the registry
//   entry exactly as it was as a MainInterface method. `mov ecx, 0x7f67f8`
//   falls out of naming the global: a const pointer binding costs the call
//   its `E8` (the brief's first lever) and a real object restores it. The
//   `symbol` fact is the mangled name this tree emits - slot 10 of the
//   interface table maininterface.h declares, hence `U` (virtual) on
//   IfaceRegistryEntry rather than `Q` on MainInterface - and the `fact`
//   value is one token by contract, so the explanation lives here.
// size      25 bytes
// prototype void (__thiscall ?on_iface_selected@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004718C0
void IfaceRegistryEntry::on_iface_selected(int code, int mode) {
    InterfaceMessageWin.on_selected(code, mode);
}


// ORIGINAL: 0x0045C5C0 ?on_scrolling@MainInterface@@QAEXHH@Z 0x0045C5C0-0x0045C5F4 FILE BYTE_EXACT
// LEVER: 2026-09-04 every dispatch in this file went from a VCall shim to a
//   named virtual - IfaceRegistryEntry (maininterface.h) declares the
//   nineteen slots the concrete interface table at 0x66C544 installs, so
//   `target->on_iface_left_click(a1, a2)` is the image's
//   `mov edx,[ecx] / call dword ptr [edx+0xNN]` with no shim. All sixteen
//   bodies that dispatch re-measured BYTE_EXACT through the change. Binding
//   IfaceRegistry as `IfaceRegistryEntry *const *const` instead of
//   `int *const` costs the match: VC6 then materialises the table pointer
//   with its own `mov reg,[imm]` instead of folding the index into the
//   address, so the binding stays `int *const` and the cast happens at the
//   use.
// size      52 bytes
// prototype void (__thiscall ?on_scrolling@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C5ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C5C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c5c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_scrolling@MainInterface@@QAEXHH@Z  at 0x0045C5C0  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_scrolling(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->on_iface_scrolling(a1, a2);
        }
    }
}


// ORIGINAL: 0x0045C640 ?on_left_double_click@MainInterface@@QAEXHH@Z 0x0045C640-0x0045C674 FILE BYTE_EXACT
// symbol    ?on_left_double_click@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_left_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C66D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C640
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c640/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_double_click@MainInterface@@QAEXHH@Z  at 0x0045C640  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_left_double_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->on_iface_left_double_click(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C680 ?on_right_double_click@MainInterface@@QAEXHH@Z 0x0045C680-0x0045C6B4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_right_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C6AD
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C680
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c680/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_double_click@MainInterface@@QAEXHH@Z  at 0x0045C680  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_right_double_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[interface_index];
        if (target != 0) {
            target->on_iface_right_double_click(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C6C0 ?on_left_up@MainInterface@@QAEXHH@Z 0x0045C6C0-0x0045C6F4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_left_up@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C6ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C6C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c6c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_up@MainInterface@@QAEXHH@Z  at 0x0045C6C0  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_left_up(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[group];
        if (target) {
            target->on_iface_left_up(a1, a2);
        }
    }
}


// ORIGINAL: 0x0045C700 ?on_left_click@MainInterface@@QAEXHH@Z 0x0045C700-0x0045C734 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_left_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C72D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C700
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@MainInterface@@QAEXHH@Z  at 0x0045C700  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_left_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->on_iface_left_click(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C740 ?on_right_click@MainInterface@@QAEXHH@Z 0x0045C740-0x0045C774 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_right_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C76D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C740
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c740/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_click@MainInterface@@QAEXHH@Z  at 0x0045C740  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_right_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[interface_index];
        if (target != 0) {
            target->on_iface_right_click(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C780 ?on_left_down@MainInterface@@QAEXHH@Z 0x0045C780-0x0045C7B4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_left_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C7AD
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C780
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c780/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_down@MainInterface@@QAEXHH@Z  at 0x0045C780  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_left_down(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[group];
        if (target) {
            target->on_iface_left_down(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C7C0 ?on_right_down@MainInterface@@QAEXHH@Z 0x0045C7C0-0x0045C7F4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_right_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C7ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C7C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c7c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_down@MainInterface@@QAEXHH@Z  at 0x0045C7C0  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

void MainInterface::on_right_down(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->on_iface_right_click(a1, a2);
        }
    }
}

// Fixed-slot bindings carried from 0045c830.cpp


// ORIGINAL: 0x0045C830 ?on_button_pressed@MainInterface@@QAEHH@Z 0x0045C830-0x0045C866 FILE BYTE_EXACT
// size      54 bytes
// prototype int (__thiscall ?on_button_pressed@MainInterface@@QAEHH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C859
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C830
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c830/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_pressed@MainInterface@@QAEHH@Z  at 0x0045C830  (54 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int MainInterface::on_button_pressed(int a1) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index1 = ActiveIfaceIdStorage;
        int index2 = IfaceIdToSlotStorage[index1];
        IfaceRegistryEntry *obj =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[index2];
        if (obj != 0) {
            return obj->on_iface_button_pressed(a1);
        }
    }
    return 0;
}

// ORIGINAL: 0x0045C870 ?on_button_toggled@MainInterface@@QAEXHH@Z 0x0045C870-0x0045C8A4 FILE BYTE_EXACT
// symbol    ?on_button_toggled@MainInterface@@UAEXHH@Z
//           Q->U 2026-09-06, Win's front slots 44/45 taking the image's
//           own names (on_button_clicked / on_button_toggled): the body is
//           now an override of the window's slot, so VC6 mangles it virtual
//           and enters it on the front table, while the image spells the
//           same body Q and its own table holds the raw address.
// size      52 bytes
// prototype void (__thiscall ?on_button_toggled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C89D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C870
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c870/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_toggled@MainInterface@@QAEXHH@Z  at 0x0045C870  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_button_toggled(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[interface_index];
        if (target != 0) {
            target->on_iface_button_toggled(a1, a2);
        }
    }
}


// ORIGINAL: 0x0045C8B0 ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z 0x0045C8B0-0x0045C8E6 FILE BYTE_EXACT
// symbol    ?on_dialog_back_draw@MainInterface@@UAEHPAVGraphicWin@@@Z
// symbol    ?on_dialog_back_draw@MainInterface@@QAEHPAVGraphicWin@@@Z
// size      54 bytes
// prototype int (__thiscall ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z)(MainInterface* this, GraphicWin*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C8D9
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C8B0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c8b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z  at 0x0045C8B0  (54 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int MainInterface::on_dialog_back_draw(GraphicWin* a1) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index2 = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *obj =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[index2];
        if (obj != 0) {
            return obj->on_dialog_back_draw(a1);
        }
    }
    return 0;
}

// ORIGINAL: 0x0045C8F0 ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x0045C8F0-0x0045C932 FILE BYTE_EXACT
// symbol    ?on_dialog_item_back_draw@MainInterface@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// symbol    ?on_dialog_item_back_draw@MainInterface@@QAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// size      66 bytes
// prototype int (__thiscall ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z)(MainInterface* this, GraphicWin*, int, int, RECT*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C925
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C8F0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c8f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z  at 0x0045C8F0  (66 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int MainInterface::on_dialog_item_back_draw(GraphicWin* a1, int a2, int a3, RECT* a4) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int idx1 = ActiveIfaceIdStorage;
        int idx2 = IfaceIdToSlotStorage[idx1];
        IfaceRegistryEntry *obj =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[idx2];
        if (obj != 0) {
            return obj->on_dialog_item_back_draw(a1, a2, a3, a4);
        }
    }
    return 0;
}


// ORIGINAL: 0x0045C980 ?on_mouse_move@MainInterface@@QAEXHH@Z 0x0045C980-0x0045C9B4 BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_mouse_move@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C9AD
// 0x0045C980  ?on_mouse_move@MainInterface@@QAEXHH@Z  ->  ?on_mouse_move@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MainInterface::on_mouse_move(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int mid = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *obj = reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[mid];
        if (obj != 0) {
            obj->on_iface_mouse_move(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C940 ?on_group_clicked@MainInterface@@QAEXHHH@Z 0x0045C940-0x0045C978 FILE BYTE_EXACT
// size      56 bytes
// prototype void (__thiscall ?on_group_clicked@MainInterface@@QAEXHHH@Z)(MainInterface* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C971
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C940
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c940/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_group_clicked@MainInterface@@QAEXHHH@Z  at 0x0045C940  (56 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_group_clicked(int a1, int a2, int a3) {
    if (WorldClimateSkipTerrainClear != 1u) {
        IfaceRegistryEntry *target = reinterpret_cast<IfaceRegistryEntry **>(
            IfaceRegistry)[IfaceIdToSlotStorage[ActiveIfaceIdStorage]];
        if (target != 0) {
            target->on_iface_group_clicked(a1, a2, a3);
        }
    }
}


// ORIGINAL: 0x0045C9C0 ?on_mouse_leave@MainInterface@@QAEXHH@Z 0x0045C9C0-0x0045C9F4 BYTE_EXACT
// symbol    ?on_mouse_leave@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_mouse_leave@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C9ED
// 0x0045C9C0  ?on_mouse_leave@MainInterface@@QAEXHH@Z  ->  ?on_mouse_leave@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MainInterface::on_mouse_leave(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->on_iface_mouse_leave(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045CA00 ?on_selected@MainInterface@@QAEXHH@Z 0x0045CA00-0x0045CA34 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_selected@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045CA2D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045CA00
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045ca00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_selected@MainInterface@@QAEXHH@Z  at 0x0045CA00  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_selected(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[interface_index];
        if (target != 0) {
            target->on_iface_selected(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045D2A0 ?set_alert@MainInterface@@QAEXH@Z 0x0045D2A0-0x0045D2D1 BYTE_EXACT
// size      49 bytes
// prototype void (__thiscall ?set_alert@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00616650
// 0x0045D2A0  ?set_alert@MainInterface@@QAEXH@Z  ->  ?set_alert@MainInterface@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MainInterface::set_alert(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 != 0) {
        *reinterpret_cast<int *>(self + 0x216f0) = 0;
        reinterpret_cast<Time *>(self + 0x25068)->start();
        return;
    }
    *reinterpret_cast<int *>(self + 0x216f0) = -1;
}

// Fixed-slot bindings carried from 0045c600.cpp

// ORIGINAL: 0x0045C600 ?on_scrolled@MainInterface@@QAEXHH@Z 0x0045C600-0x0045C634 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_scrolled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C62D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C600
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c600/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_scrolled@MainInterface@@QAEXHH@Z  at 0x0045C600  (52 bytes)
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
//    a thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention. An indirect VIRTUAL
//    call is spelled by the name of the method, which is what makes
//    `call dword ptr [eax+0xNN]`.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void MainInterface::on_scrolled(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[group];
        if (target) {
            target->on_iface_scrolled(a1, a2);
        }
    }
}


/*
// ORIGINAL: 0x0045C200 ?timer_callback_daemon@MainInterface@@QAAXHH@Z 0x0045C200-0x0045C273
// symbol    ?timer_callback_daemon@@YAXHH@Z
// TRIED: both stack parameters are read directly ([ebp+8], [ebp+0xc]) with no
//        `this` anywhere, so the scaffold's MainInterface member cannot be
//        defined out-of-line - that adds an implicit `this` ahead of them and
//        shifts both to +0xc/+0x10. It is a free function here.
// TRIED: the same `Time::stop` ceiling as 0x0045C180. The image CALLS
//        0x00616730; time.h keeps `stop` in-class MEASURED so `close`/`~Time`
//        inline it, and VC6 has no noinline. Demoted rather than mismatched
//        forever; split `stop` in time.h and both bodies re-earn.
*/
void __cdecl timer_callback_daemon(int a1, int a2) {
    if (a2 == 0) return;

    char *self = reinterpret_cast<char *>(a2);
    switch (a1) {
    case 0:
        reinterpret_cast<Time *>(self + 0x25040)->stop();
        break;
    case 1: {
        int *count = reinterpret_cast<int *>(self + 0x216f0);
        if (*count == -1) {
            reinterpret_cast<Time *>(self + 0x25068)->stop();
            return;
        }
        *count = *count + 1;

        RECT rc;
        UnionRect(&rc, reinterpret_cast<RECT *>(self + 0xde0),
                  reinterpret_cast<RECT *>(self + 0xdf0));
        reinterpret_cast<GraphicWin *>(self)->soft_update(&rc);
        break;
    }
    }
}

/*
Purpose: Hand the clicked button id to whichever interface mode is active.
// ORIGINAL: 0x0045C800 ?on_button_clicked@MainInterface@@QAEXH@Z 0x0045C800-0x0045C830 BYTE_EXACT
// size      48 bytes
// prototype void (__thiscall ?on_button_clicked@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C829
// LEVER: homed 2026-09-04 by the receiver map. The receiver is never touched,
//   so the body is the two guards and the dispatch - and the dispatch is now
//   a call by name: IfaceRegistryEntry declares the nineteen slots the
//   concrete table at 0x66C544 installs, slot 13 (offset 0x34) being
//   on_iface_button_clicked, so `target->on_iface_button_clicked(a1)` is the
//   image's `mov edx,[ecx] / call dword ptr [edx+0x34]` and the VCall shim
//   that used to stand in for it is gone. receiver stays unspelled because
//   the registry entries are SubInterface subobjects (front+0xA14), not
//   windows, and this method's own `this` never reaches the body.
// symbol    ?on_button_clicked@MainInterface@@UAEXH@Z
//           was `QAEXH@Z - unchanged; this one is a MainInterface member and
//           stays one`, until 2026-09-06 when Win's front slots 44/45 took
//           the image's own names: this body now overrides Win's slot 44, so
//           VC6 mangles it virtual and enters it on the front table
//           (0x66C648 holds it) while the image still spells the body Q.
Return Value: n/a
Status: homed from src/recovered/0045c800.cpp on this pass.
*/
void MainInterface::on_button_clicked(int a1) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = ActiveIfaceIdStorage;
        int subindex = IfaceIdToSlotStorage[index];
        IfaceRegistryEntry *target =
            reinterpret_cast<IfaceRegistryEntry **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->vslot_13(a1);
        }
    }
}
