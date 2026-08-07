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
#include <cstdint>
#include <cstring>

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00408710
Return Value: n/a
Status: Complete
*/
void BaseWin::close() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00416DD0
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0041AF20
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0041AF30
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
Original Offset: 0041A610
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
Original Offset: 0041B4E0
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
Original Offset: 0041DF40
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
Original Offset: 0041E4C0
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
Original Offset: 0041DCD0
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
Original Offset: 0041DCF0
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
Original Offset: 0041E4D0
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
Original Offset: 0041E4F0
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
Original Offset: 0041DC80
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
Original Offset: 0041AF70
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_click(int a1, int a2) {
    (ORIGINAL(this)->*BaseWinClick)(a1, a2, 0, 0);
}

/*
Purpose: Report a right click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
Original Offset: 0041AF90
Return Value: n/a
Status: Complete
*/
void BaseWin::on_right_click(int a1, int a2) {
    (ORIGINAL(this)->*BaseWinClick)(a1, a2, 1, 0);
}

/*
Purpose: Report a left double-click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
Original Offset: 0041AFB0
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
Original Offset: 0041AF40
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
Original Offset: 0041B1E0
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
Original Offset: 0041E890
Status: Complete
*/
void BaseWin::show(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (!reinterpret_cast<Win *>(self)->is_visible()) {
        reinterpret_cast<SubInterface *>(self + 0xa14)->set_iface_mode();
    }
}
