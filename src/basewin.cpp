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
#include "original_seam.h"
#include "basewin.h"
#include "win.h"
#include "subinterface.h"
#include <cstdint>
#include <cstring>

const uint32_t BaseWinSubInterfaceVtableInitial = 0x0066A6E4;
const uint32_t BaseWinSubInterfaceVtable = 0x0066A3B8;
const uint32_t BaseWinPrimaryVtable = 0x0066A40C;
const uint32_t BaseWinBufferVtable = 0x0066A404;

/*
Purpose: Construct BaseWin's large aggregate of sub-windows: a ProdPicker, a
         second GraphicWin, several Sprite runs, a FlatButton, a Buffer, a
         FlatButton[22], a Time, a Caviar, a MapWin, a Font, two Spots, a
         second Buffer and three Scrolls.
// ORIGINAL: 0x00408490 ??0BaseWin@@QAE@XZ 0x00408490-0x0040870D;0x004210D0-0x004210E7;0x00651380-0x00651567
// LEVER: real, correctly-offset, correctly-typed members (already modelled
//        in basewin.h) constructed IMPLICITLY, in declaration order, took
//        this from a SHARED_TAIL isolated-scaffold measurement straight to
//        120/144 agreeing instructions here - far better than the explicit
//        raw-offset placement-new this batch's other five constructors
//        needed for ordering control. First divergence at instruction 10:
//        this compiled body reserves an extra `push edi` and calls through
//        an extra local thunk where the image calls GraphicWin::construct
//        directly - not chased further given the SHARED_TAIL caveat above.
// size      1147 bytes
// prototype void (__thiscall ??0BaseWin@@QAE@XZ)(BaseWin* this)
// callers   1   call targets   15
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00420F90 0x004626E0 0x00492420 0x005D4CF0 0x005D4DD0 0x005D7210 0x005E37E0 0x005FA860 0x006051D0 0x00607CF0 0x006161D0 0x00616DA0 0x00618EA0 0x006456E4 0x006457C2
*/
// This address is catalogued SHARED_TAIL: its cleanup path runs through an
// EH-funclet cluster shared with other functions (COMDAT-folded), so no
// per-function byte verdict is well defined here regardless of source form -
// see the deleted src/recovered/units/00408490.cpp, which measured the same
// finding from an isolated scaffold. Bodied for COVERAGE: every sub-object
// below is a real, correctly-offset, correctly-typed member already modelled
// in basewin.h, constructed implicitly in declaration order (matching the
// image's own order field-for-field) except MapWin, whose own constructor
// is the empty inline stub - its real work is `MapWin::construct(int)`,
// the established seam for a virtual-base constructor reached through a
// pointer-to-member elsewhere in this tree.
BaseWin::BaseWin() {
    GraphicWin::construct();

    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0xA14 / 4] = BaseWinSubInterfaceVtableInitial;

    mapWin_.construct(1);

    object[0x000 / 4] = BaseWinPrimaryVtable;
    object[0x444 / 4] = BaseWinBufferVtable;
    object[0xA14 / 4] = BaseWinSubInterfaceVtable;
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00408710 ?close@BaseWin@@QAEXXZ 0x00408710-0x00408711 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::close() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00416DD0 ?UNK4@BaseWin@@QAEXXZ 0x00416DD0-0x00416DD1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK4@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0041AF20 ?UNK6@BaseWin@@QAEXXZ 0x0041AF20-0x0041AF21 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0041AF30 ?UNK7@BaseWin@@QAEXXZ 0x0041AF30-0x0041AF31 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK7@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK7() {
}

void __fastcall base_win_close_redirect(BaseWin *self, void *) {
    self->close();
}

void __fastcall base_win_unk4_redirect(BaseWin *self, void *) {
    self->UNK4();
}

void __fastcall base_win_unk6_redirect(BaseWin *self, void *) {
    self->UNK6();
}

void __fastcall base_win_unk7_redirect(BaseWin *self, void *) {
    self->UNK7();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041A610 ?UNK5@BaseWin@@QAEXH@Z 0x0041A610-0x0041A613 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK5@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK5(int) {
}

void __fastcall base_win_unk5_redirect(BaseWin *self, void *, int a1) {
    self->UNK5(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041B4E0 ?on_button_toggled@BaseWin@@QAEXHH@Z 0x0041B4E0-0x0041B4E3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_button_toggled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_button_toggled(int, int) {
}

void __fastcall base_win_on_button_toggled_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_button_toggled(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041DF40 ?on_iface_right_down@BaseWin@@QAEXHH@Z 0x0041DF40-0x0041DF43 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_down(int, int) {
}

void __fastcall base_win_on_iface_right_down_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041E4C0 ?on_iface_selected@BaseWin@@QAEXHH@Z 0x0041E4C0-0x0041E4C3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_selected(int, int) {
}

void __fastcall base_win_on_iface_selected_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}


/*
Purpose: Report a left click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
// ORIGINAL: 0x0041DCD0 ?on_iface_left_click@BaseWin@@QAEXHH@Z 0x0041DCD0-0x0041DCEE BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_iface_left_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_left_click(int a1, int a2) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->iface_click(a1, a2, 0, 0);
}

/*
Purpose: Report a right click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
// ORIGINAL: 0x0041DCF0 ?on_iface_right_click@BaseWin@@QAEXHH@Z 0x0041DCF0-0x0041DD0E BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_iface_right_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_click(int a1, int a2) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->iface_click(a1, a2, 1, 0);
}

/*
Purpose: Report a left double-click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
// ORIGINAL: 0x0041E4D0 ?on_iface_left_double_click@BaseWin@@QAEXHH@Z 0x0041E4D0-0x0041E4EE BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_iface_left_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_left_double_click(int a1, int a2) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->iface_click(a1, a2, 0, 1);
}

/*
Purpose: Report a right double-click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
// ORIGINAL: 0x0041E4F0 ?on_iface_right_double_click@BaseWin@@QAEXHH@Z 0x0041E4F0-0x0041E50E BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_iface_right_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_double_click(int a1, int a2) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->iface_click(a1, a2, 1, 1);
}

void __fastcall base_win_on_iface_left_click_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_left_click(a1, a2);
}

void __fastcall base_win_on_iface_right_click_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_right_click(a1, a2);
}

void __fastcall base_win_on_iface_left_double_click_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

void __fastcall base_win_on_iface_right_double_click_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}


/*
Purpose: Handle an interface scroll, but only for scroll kind 2 - stash the
         new position at 0x40100 (interface-relative) and redraw the supported
         markers. Like the click handlers, `this` arrives at the interface
         subobject and is adjusted back to the BaseWin for that redraw.
// ORIGINAL: 0x0041DC80 ?on_iface_scrolled@BaseWin@@QAEXHH@Z 0x0041DC80-0x0041DCA5
// size      37 bytes
// prototype void (__thiscall ?on_iface_scrolled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0040C850
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_scrolled(int a1, int a2) {
    if (a1 != 2) {
        return;
    }
    std::memcpy(reinterpret_cast<uint8_t *>(this) + 0x40100, &a2, sizeof(a2));
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->draw_supported(1);
}

void __fastcall base_win_on_iface_scrolled_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_scrolled(a1, a2);
}


/*
Purpose: Report a left click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AF70 ?on_left_click@BaseWin@@QAEXHH@Z 0x0041AF70-0x0041AF88 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_left_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_click(int a1, int a2) {
    click(a1, a2, 0, 0);
}

/*
Purpose: Report a right click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AF90 ?on_right_click@BaseWin@@QAEXHH@Z 0x0041AF90-0x0041AFA8 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_right_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_right_click(int a1, int a2) {
    click(a1, a2, 1, 0);
}

/*
Purpose: Report a left double-click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AFB0 ?on_left_double_click@BaseWin@@QAEXHH@Z 0x0041AFB0-0x0041AFC8 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_left_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_double_click(int a1, int a2) {
    click(a1, a2, 0, 1);
}

void __fastcall base_win_on_left_click_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

void __fastcall base_win_on_right_click_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_right_click(a1, a2);
}

void __fastcall base_win_on_left_double_click_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_left_double_click(a1, a2);
}

/*
Purpose: Interface timer tick; the legacy implementation ignores it and
         returns.
// ORIGINAL: 0x0041AF40 ?timer_callback@BaseWin@@QAAXHH@Z 0x0041AF40-0x0041AF41 BYTE_EXACT
// symbol    ?timer_callback@BaseWin@@SAXHH@Z
// size      1 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::timer_callback(int, int) {
}

void __cdecl base_win_timer_callback_redirect(int a1, int a2) {
    BaseWin::timer_callback(a1, a2);
}

/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x0041B1E0 ?on_mouse_leave@BaseWin@@QAEXHH@Z 0x0041B1E0-0x0041B1ED BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?on_mouse_leave@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void BaseWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}

void __fastcall base_win_on_mouse_leave_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}

/*
// ORIGINAL: 0x0041E890 ?show@BaseWin@@QAEXH@Z 0x0041E890-0x0041E8AB BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?show@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void BaseWin::show(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (!reinterpret_cast<Win *>(self)->is_visible()) {
        reinterpret_cast<SubInterface *>(self + 0xa14)->set_iface_mode();
    }
}

/*
// ORIGINAL: 0x0040B120 ?UNK2@BaseWin@@QAEXHHHH@Z 0x0040B120-0x0040B13C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?UNK2@BaseWin@@QAEXHHHH@Z)(BaseWin* this, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0040B140
Status: Complete
*/
void BaseWin::UNK2(int a1, int a2, int a3, int a4) {
    garrison_click(a1, a2, a3, a4);
}

/*
// ORIGINAL: 0x0041B4A0 ?on_scrolled@BaseWin@@QAEXHH@Z 0x0041B4A0-0x0041B4BD BYTE_EXACT
// size      29 bytes
// prototype void (__thiscall ?on_scrolled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0040FCC0
Status: Complete
*/
void BaseWin::on_scrolled(int a1, int a2) {
    switch (a1) {
        case 1:
            *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x40b10) = a2;
            draw_facilities(1);
            break;
    }
}


