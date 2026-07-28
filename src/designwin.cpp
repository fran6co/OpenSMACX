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
#include "designwin.h"
#include "win.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 004345C0
Return Value: n/a
Status: Complete
*/
void DesignWin::UNK1() {
}

void __fastcall design_win_unk1_redirect(DesignWin *self, void *) {
    self->UNK1();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0043D330
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_left_down(int, int) {
}

void __fastcall design_win_on_iface_left_down_redirect(DesignWin *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0043D340
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_right_down(int, int) {
}

void __fastcall design_win_on_iface_right_down_redirect(DesignWin *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0043D400
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_selected(int, int) {
}

void __fastcall design_win_on_iface_selected_redirect(DesignWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0043D410
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_left_double_click(int, int) {
}

void __fastcall design_win_on_iface_left_double_click_redirect(DesignWin *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0043D420
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_right_double_click(int, int) {
}

void __fastcall design_win_on_iface_right_double_click_redirect(DesignWin *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0043D470
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_button_toggled(int, int) {
}

void __fastcall design_win_on_iface_button_toggled_redirect(DesignWin *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0043D480
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_group_clicked(int, int, int) {
}

void __fastcall design_win_on_iface_group_clicked_redirect(DesignWin *self, void *, int a1, int a2, int a3) {
    self->on_iface_group_clicked(a1, a2, a3);
}

/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
Original Offset: 00437BE0
Return Value: n/a
Status: Complete
*/
void DesignWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}

void __fastcall design_win_on_mouse_leave_redirect(DesignWin *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}
