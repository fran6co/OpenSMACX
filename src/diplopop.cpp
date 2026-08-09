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
#include "diplopop.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004404C0
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_click(int, int) {
}

void __fastcall diplo_pop_on_iface_left_click_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004404D0
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_click(int, int) {
}

void __fastcall diplo_pop_on_iface_right_click_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_right_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004404E0
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_down(int, int) {
}

void __fastcall diplo_pop_on_iface_left_down_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004404F0
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_down(int, int) {
}

void __fastcall diplo_pop_on_iface_right_down_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00440500
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_mouse_move(int, int) {
}

void __fastcall diplo_pop_on_iface_mouse_move_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00440510
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_mouse_leave(int, int) {
}

void __fastcall diplo_pop_on_iface_mouse_leave_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00440520
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_selected(int, int) {
}

void __fastcall diplo_pop_on_iface_selected_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00440530
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_double_click(int, int) {
}

void __fastcall diplo_pop_on_iface_left_double_click_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00440540
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_double_click(int, int) {
}

void __fastcall diplo_pop_on_iface_right_double_click_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00440610
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_button_toggled(int, int) {
}

void __fastcall diplo_pop_on_iface_button_toggled_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

/*
ORIGINAL: 0x0043F170
Status: Complete
*/
void DiploPop::hide() {
    Win *self = reinterpret_cast<Win *>(this);
    if (self->is_visible()) {
        self->hide();
    }
}
