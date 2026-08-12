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
#include "councwin.h"
#include "subinterface.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00426EB0
Return Value: n/a
Status: Complete
*/
void CouncWin::on_group_clicked(int, int) {
}

void __fastcall counc_win_on_group_clicked_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_group_clicked(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00426F80
Return Value: n/a
Status: Complete
*/
void CouncWin::on_button_passover(int, int) {
}

void __fastcall counc_win_on_button_passover_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_button_passover(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00427060
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_click(int, int) {
}

void __fastcall counc_win_on_iface_left_click_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00427070
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_click(int, int) {
}

void __fastcall counc_win_on_iface_right_click_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_right_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00427080
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_down(int, int) {
}

void __fastcall counc_win_on_iface_left_down_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00427090
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_down(int, int) {
}

void __fastcall counc_win_on_iface_right_down_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004270A0
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_mouse_move(int, int) {
}

void __fastcall counc_win_on_iface_mouse_move_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004270B0
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_mouse_leave(int, int) {
}

void __fastcall counc_win_on_iface_mouse_leave_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004270C0
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_selected(int, int) {
}

void __fastcall counc_win_on_iface_selected_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004270D0
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_double_click(int, int) {
}

void __fastcall counc_win_on_iface_left_double_click_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004270E0
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_double_click(int, int) {
}

void __fastcall counc_win_on_iface_right_double_click_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00427110
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_button_toggled(int, int) {
}

void __fastcall counc_win_on_iface_button_toggled_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00427120
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_group_clicked(int, int, int) {
}

void __fastcall counc_win_on_iface_group_clicked_redirect(CouncWin *self, void *, int a1, int a2, int a3) {
    self->on_iface_group_clicked(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
ORIGINAL: 0x00426F90
Return Value: 0
Status: Complete
*/
int CouncWin::on_button_text_draw(int, int, int) {
    return 0;
}

int __fastcall counc_win_on_button_text_draw_redirect(
        CouncWin *self, void *, int a1, int a2, int a3) {
    return self->on_button_text_draw(a1, a2, a3);
}

/*
ORIGINAL: 0x00426EC0
Status: Complete
*/
void CouncWin::on_mouse_leave(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0xa34);
    if (v != -1) {
        *reinterpret_cast<int *>(self + 0xa34) = -1;
        draw_leader(v);
    }
}

/*
ORIGINAL: 0x00425D90
Status: Complete
*/
void CouncWin::draw_all_leaders() {
    for (int i = 1; i < 8; i++) {
        draw_leader(i);
    }
}

/*
ORIGINAL: 0x00427130
Status: Complete
*/
int CouncWin::show(int a1) {
    int visible = reinterpret_cast<Win *>(this)->is_visible();
    if (visible) {
        return visible;
    }
    SubInterface::set_iface_mode();
}

// The seam and definition for the unrecovered draw_leader, at the end of the
// file so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness. Left alone, VC6 expands this
// one-line forwarder into draw_all_leaders (0x00425D90) and on_mouse_leave,
// which reported it as C4711 at councwin.cpp(215) and (225). Both callers are
// BYTE_EXACT today precisely because the original emits `call rel32` there, and
// an inlined `mov ecx / call [seam]` would break them. OPENSMACX_NOINLINE is
// the tree's spelling for this, but it expands to nothing on VC6 - the pragma
// is the only form cl 12.00 honours.
#pragma auto_inline(off)
func_counc_win_draw_leader CouncWinOriginalDrawLeader =
    original_method<func_counc_win_draw_leader>(0x00425DB0);

void CouncWin::draw_leader(int factionID) {
    (ORIGINAL(this)->*CouncWinOriginalDrawLeader)(factionID);
}
#pragma auto_inline(on)
