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
// ORIGINAL: 0x004404C0 ?on_iface_left_click@DiploPop@@QAEXHH@Z 0x004404C0-0x004404C3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004404D0 ?on_iface_right_click@DiploPop@@QAEXHH@Z 0x004404D0-0x004404D3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004404E0 ?on_iface_left_down@DiploPop@@QAEXHH@Z 0x004404E0-0x004404E3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004404F0 ?on_iface_right_down@DiploPop@@QAEXHH@Z 0x004404F0-0x004404F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00440500 ?on_iface_mouse_move@DiploPop@@QAEXHH@Z 0x00440500-0x00440503 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00440510 ?on_iface_mouse_leave@DiploPop@@QAEXHH@Z 0x00440510-0x00440513 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00440520 ?on_iface_selected@DiploPop@@QAEXHH@Z 0x00440520-0x00440523 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00440530 ?on_iface_left_double_click@DiploPop@@QAEXHH@Z 0x00440530-0x00440533 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00440540 ?on_iface_right_double_click@DiploPop@@QAEXHH@Z 0x00440540-0x00440543 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00440610 ?on_iface_button_toggled@DiploPop@@QAEXHH@Z 0x00440610-0x00440613 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_button_toggled(int, int) {
}

void __fastcall diplo_pop_on_iface_button_toggled_redirect(DiploPop *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

/*
// ORIGINAL: 0x0043F170 ?hide@DiploPop@@QAEXXZ 0x0043F170-0x0043F185 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?hide@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EDCD0 0x005F7E90
Status: Complete
*/
void DiploPop::hide() {
    Win *self = reinterpret_cast<Win *>(this);
    if (self->is_visible()) {
        self->hide();
    }
}
