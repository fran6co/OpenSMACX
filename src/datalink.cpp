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
#include "datalink.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0042BEA0
Return Value: n/a
Status: Complete
*/
void Datalink::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0042BF00
Return Value: n/a
Status: Complete
*/
void Datalink::UNK8() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00431DB0
Return Value: n/a
Status: Complete
*/
void Datalink::UNK9() {
}

void __fastcall datalink_unk6_redirect(Datalink *self, void *) {
    self->UNK6();
}

void __fastcall datalink_unk8_redirect(Datalink *self, void *) {
    self->UNK8();
}

void __fastcall datalink_unk9_redirect(Datalink *self, void *) {
    self->UNK9();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00431EB0
Return Value: n/a
Status: Complete
*/
void Datalink::on_left_click(int, int) {
}

void __fastcall datalink_on_left_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00431FB0
Return Value: n/a
Status: Complete
*/
void Datalink::on_group_clicked(int, int) {
}

void __fastcall datalink_on_group_clicked_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_group_clicked(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432000
Return Value: n/a
Status: Complete
*/
void Datalink::on_mouse_move(int, int) {
}

void __fastcall datalink_on_mouse_move_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432160
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_click(int, int) {
}

void __fastcall datalink_on_iface_left_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432170
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_click(int, int) {
}

void __fastcall datalink_on_iface_right_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_right_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432180
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_down(int, int) {
}

void __fastcall datalink_on_iface_left_down_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432190
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_down(int, int) {
}

void __fastcall datalink_on_iface_right_down_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321A0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_mouse_move(int, int) {
}

void __fastcall datalink_on_iface_mouse_move_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321B0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_mouse_leave(int, int) {
}

void __fastcall datalink_on_iface_mouse_leave_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321C0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_selected(int, int) {
}

void __fastcall datalink_on_iface_selected_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321D0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_double_click(int, int) {
}

void __fastcall datalink_on_iface_left_double_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321E0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_double_click(int, int) {
}

void __fastcall datalink_on_iface_right_double_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321F0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_button_clicked(int) {
}

void __fastcall datalink_on_iface_button_clicked_redirect(Datalink *self, void *, int a1) {
    self->on_iface_button_clicked(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432200
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_button_toggled(int, int) {
}

void __fastcall datalink_on_iface_button_toggled_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}
