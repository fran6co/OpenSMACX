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
#include "socialwin.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 004AEF00
Return Value: n/a
Status: Complete
*/
void SocialWin::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 004B1790
Return Value: n/a
Status: Complete
*/
void SocialWin::UNK3() {
}

void __fastcall social_win_unk2_redirect(SocialWin *self, void *) {
    self->UNK2();
}

void __fastcall social_win_unk3_redirect(SocialWin *self, void *) {
    self->UNK3();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004B2400
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_left_down(int, int) {
}

void __fastcall social_win_on_iface_left_down_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004B2410
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_right_down(int, int) {
}

void __fastcall social_win_on_iface_right_down_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004B2440
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_selected(int, int) {
}

void __fastcall social_win_on_iface_selected_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004B2450
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_left_double_click(int, int) {
}

void __fastcall social_win_on_iface_left_double_click_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004B2460
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_right_double_click(int, int) {
}

void __fastcall social_win_on_iface_right_double_click_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004B2670
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_button_toggled(int, int) {
}

void __fastcall social_win_on_iface_button_toggled_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004B2680
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_group_clicked(int, int, int) {
}

void __fastcall social_win_on_iface_group_clicked_redirect(SocialWin *self, void *, int a1, int a2, int a3) {
    self->on_iface_group_clicked(a1, a2, a3);
}
