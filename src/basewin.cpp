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

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x00408710 BYTE_EXACT
// name      ?close@BaseWin@@QAEXXZ
// size      1 bytes
// spans     0x00408710-0x00408711
// prototype void (__thiscall ?close@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   3   call targets   0
Return Value: n/a
Status: Complete
*/
void BaseWin::close() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x00416DD0 BYTE_EXACT
// name      ?UNK4@BaseWin@@QAEXXZ
// size      1 bytes
// spans     0x00416DD0-0x00416DD1
// prototype void (__thiscall ?UNK4@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0041AF20 BYTE_EXACT
// name      ?UNK6@BaseWin@@QAEXXZ
// size      1 bytes
// spans     0x0041AF20-0x0041AF21
// prototype void (__thiscall ?UNK6@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0041AF30 BYTE_EXACT
// name      ?UNK7@BaseWin@@QAEXXZ
// size      1 bytes
// spans     0x0041AF30-0x0041AF31
// prototype void (__thiscall ?UNK7@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
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
ORIGINAL: 0x0041A610 BYTE_EXACT
// name      ?UNK5@BaseWin@@QAEXH@Z
// size      3 bytes
// spans     0x0041A610-0x0041A613
// prototype void (__thiscall ?UNK5@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   0
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
ORIGINAL: 0x0041B4E0 BYTE_EXACT
// name      ?on_button_toggled@BaseWin@@QAEXHH@Z
// size      3 bytes
// spans     0x0041B4E0-0x0041B4E3
// prototype void (__thiscall ?on_button_toggled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x0041DF40 BYTE_EXACT
// name      ?on_iface_right_down@BaseWin@@QAEXHH@Z
// size      3 bytes
// spans     0x0041DF40-0x0041DF43
// prototype void (__thiscall ?on_iface_right_down@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x0041E4C0 BYTE_EXACT
// name      ?on_iface_selected@BaseWin@@QAEXHH@Z
// size      3 bytes
// spans     0x0041E4C0-0x0041E4C3
// prototype void (__thiscall ?on_iface_selected@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_selected(int, int) {
}

void __fastcall base_win_on_iface_selected_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

func_base_win_iface_click BaseWinIfaceClick =
    original_method<func_base_win_iface_click>(0x004160F0);

/*
Purpose: Report a left click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
ORIGINAL: 0x0041DCD0
// name      ?on_iface_left_click@BaseWin@@QAEXHH@Z
// size      30 bytes
// spans     0x0041DCD0-0x0041DCEE
// prototype void (__thiscall ?on_iface_left_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_left_click(int a1, int a2) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    (ORIGINAL(base)->*BaseWinIfaceClick)(a1, a2, 0, 0);
}

/*
Purpose: Report a right click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
ORIGINAL: 0x0041DCF0
// name      ?on_iface_right_click@BaseWin@@QAEXHH@Z
// size      30 bytes
// spans     0x0041DCF0-0x0041DD0E
// prototype void (__thiscall ?on_iface_right_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_click(int a1, int a2) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    (ORIGINAL(base)->*BaseWinIfaceClick)(a1, a2, 1, 0);
}

/*
Purpose: Report a left double-click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
ORIGINAL: 0x0041E4D0
// name      ?on_iface_left_double_click@BaseWin@@QAEXHH@Z
// size      30 bytes
// spans     0x0041E4D0-0x0041E4EE
// prototype void (__thiscall ?on_iface_left_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_left_double_click(int a1, int a2) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    (ORIGINAL(base)->*BaseWinIfaceClick)(a1, a2, 0, 1);
}

/*
Purpose: Report a right double-click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
ORIGINAL: 0x0041E4F0
// name      ?on_iface_right_double_click@BaseWin@@QAEXHH@Z
// size      30 bytes
// spans     0x0041E4F0-0x0041E50E
// prototype void (__thiscall ?on_iface_right_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_double_click(int a1, int a2) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    (ORIGINAL(base)->*BaseWinIfaceClick)(a1, a2, 1, 1);
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

func_base_win_draw_supported BaseWinDrawSupported =
    original_method<func_base_win_draw_supported>(0x0040C850);

/*
Purpose: Handle an interface scroll, but only for scroll kind 2 - stash the
         new position at 0x40100 (interface-relative) and redraw the supported
         markers. Like the click handlers, `this` arrives at the interface
         subobject and is adjusted back to the BaseWin for that redraw.
ORIGINAL: 0x0041DC80
// name      ?on_iface_scrolled@BaseWin@@QAEXHH@Z
// size      37 bytes
// spans     0x0041DC80-0x0041DCA5
// prototype void (__thiscall ?on_iface_scrolled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
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
    (ORIGINAL(base)->*BaseWinDrawSupported)(1);
}

void __fastcall base_win_on_iface_scrolled_redirect(BaseWin *self, void *, int a1, int a2) {
    self->on_iface_scrolled(a1, a2);
}

func_base_win_click BaseWinClick = original_method<func_base_win_click>(0x004165D0);

/*
Purpose: Report a left click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
ORIGINAL: 0x0041AF70
// name      ?on_left_click@BaseWin@@QAEXHH@Z
// size      24 bytes
// spans     0x0041AF70-0x0041AF88
// prototype void (__thiscall ?on_left_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_click(int a1, int a2) {
    (ORIGINAL(this)->*BaseWinClick)(a1, a2, 0, 0);
}

/*
Purpose: Report a right click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
ORIGINAL: 0x0041AF90
// name      ?on_right_click@BaseWin@@QAEXHH@Z
// size      24 bytes
// spans     0x0041AF90-0x0041AFA8
// prototype void (__thiscall ?on_right_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void BaseWin::on_right_click(int a1, int a2) {
    (ORIGINAL(this)->*BaseWinClick)(a1, a2, 1, 0);
}

/*
Purpose: Report a left double-click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
ORIGINAL: 0x0041AFB0
// name      ?on_left_double_click@BaseWin@@QAEXHH@Z
// size      24 bytes
// spans     0x0041AFB0-0x0041AFC8
// prototype void (__thiscall ?on_left_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_double_click(int a1, int a2) {
    (ORIGINAL(this)->*BaseWinClick)(a1, a2, 0, 1);
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
ORIGINAL: 0x0041AF40 BYTE_EXACT
// name      ?timer_callback@BaseWin@@QAAXHH@Z
// size      1 bytes
// spans     0x0041AF40-0x0041AF41
// prototype 
// callers   0   call targets   0
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
ORIGINAL: 0x0041B1E0
// name      ?on_mouse_leave@BaseWin@@QAEXHH@Z
// size      13 bytes
// spans     0x0041B1E0-0x0041B1ED
// prototype void (__thiscall ?on_mouse_leave@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
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
ORIGINAL: 0x0041E890 BYTE_EXACT
// name      ?show@BaseWin@@QAEXH@Z
// size      27 bytes
// spans     0x0041E890-0x0041E8AB
// prototype void (__thiscall ?show@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   2
Status: Complete
*/
void BaseWin::show(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (!reinterpret_cast<Win *>(self)->is_visible()) {
        reinterpret_cast<SubInterface *>(self + 0xa14)->set_iface_mode();
    }
}

/*
ORIGINAL: 0x0040B120 BYTE_EXACT
// name      ?UNK2@BaseWin@@QAEXHHHH@Z
// size      28 bytes
// spans     0x0040B120-0x0040B13C
// prototype void (__thiscall ?UNK2@BaseWin@@QAEXHHHH@Z)(BaseWin* this, int, int, int, int)
// callers   0   call targets   1
Status: Complete
*/
void BaseWin::UNK2(int a1, int a2, int a3, int a4) {
    garrison_click(a1, a2, a3, a4);
}

/*
ORIGINAL: 0x0041B4A0 BYTE_EXACT
// name      ?on_scrolled@BaseWin@@QAEXHH@Z
// size      29 bytes
// spans     0x0041B4A0-0x0041B4BD
// prototype void (__thiscall ?on_scrolled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
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

func_base_win_draw_facilities BaseWinDrawFacilities =
    original_method<func_base_win_draw_facilities>(0x0040FCC0);
func_base_win_garrison_click BaseWinGarrisonClick =
    original_method<func_base_win_garrison_click>(0x0040B140);

/*
Purpose: Redraw the base's facility list. The body at 0x0040FCC0 is NOT
         recovered; this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void BaseWin::draw_facilities(int a1) {
    (ORIGINAL(this)->*BaseWinDrawFacilities)(a1);
}

/*
Purpose: Handle a click on the base's garrison list. The body at 0x0040B140 is
         NOT recovered; this is a seam to the original image, not a recovery,
         and deliberately carries no `Original Offset:` line so the catalogue
         does not mistake it for one.
Status: Forwarded to the original image
*/
void BaseWin::garrison_click(int vehID, int a2, int a3, int a4) {
    (ORIGINAL(this)->*BaseWinGarrisonClick)(vehID, a2, a3, a4);
}
