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
Original Offset: 0045C170
Return Value: n/a
Status: Complete
*/
void MainInterface::clear_message() {
}

/*
Purpose: Update the desktop; the legacy implementation is intentionally empty.
Original Offset: 0058EE50
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_update() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0045C5B0
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0045EB60
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK3() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0045EB70
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK4() {
}

void __fastcall main_interface_clear_message_redirect(MainInterface *self, void *) {
    self->clear_message();
}

void __fastcall main_interface_unk2_redirect(MainInterface *self, void *) {
    self->UNK2();
}

void __fastcall main_interface_unk3_redirect(MainInterface *self, void *) {
    self->UNK3();
}

void __fastcall main_interface_unk4_redirect(MainInterface *self, void *) {
    self->UNK4();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0045C280
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK1(int) {
}

void __fastcall main_interface_unk1_redirect(MainInterface *self, void *, int a1) {
    self->UNK1(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0045CC80
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_scrolled(int, int) {
}

void __fastcall main_interface_on_iface_scrolled_redirect(MainInterface *self, void *, int a1, int a2) {
    self->on_iface_scrolled(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0045CC90
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_scrolling(int, int) {
}

void __fastcall main_interface_on_iface_scrolling_redirect(MainInterface *self, void *, int a1, int a2) {
    self->on_iface_scrolling(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0045CCA0
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_button_toggled(int, int) {
}

void __fastcall main_interface_on_iface_button_toggled_redirect(MainInterface *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 0045CDB0
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_right_double_click(int, int) {
}

void __fastcall main_interface_on_iface_right_double_click_redirect(MainInterface *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
Original Offset: 0045D160
Return Value: 0, always
Status: Complete
*/
int MainInterface::on_iface_button_pressed(int) {
    return 0;
}

int __fastcall main_interface_on_iface_button_pressed_redirect(MainInterface *self, void *, int a1) {
    return self->on_iface_button_pressed(a1);
}
