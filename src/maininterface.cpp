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
ORIGINAL: 0x0045C170 BYTE_EXACT
// name      ?clear_message@MainInterface@@QAEXXZ
// size      1 bytes
// spans     0x0045C170-0x0045C171
// prototype void (__thiscall ?clear_message@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void MainInterface::clear_message() {
}

/*
Purpose: Update the desktop; the legacy implementation is intentionally empty.
ORIGINAL: 0x0058EE50 BYTE_EXACT
// name      ?desktop_update@@YAXXZ
// size      1 bytes
// spans     0x0058EE50-0x0058EE51
// prototype 
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_update() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0045C5B0 BYTE_EXACT
// name      ?UNK2@MainInterface@@QAEXXZ
// size      1 bytes
// spans     0x0045C5B0-0x0045C5B1
// prototype void (__thiscall ?UNK2@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0045EB60 BYTE_EXACT
// name      ?UNK3@MainInterface@@QAEXXZ
// size      1 bytes
// spans     0x0045EB60-0x0045EB61
// prototype void (__thiscall ?UNK3@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK3() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0045EB70 BYTE_EXACT
// name      ?UNK4@MainInterface@@QAEXXZ
// size      1 bytes
// spans     0x0045EB70-0x0045EB71
// prototype void (__thiscall ?UNK4@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0045C280 BYTE_EXACT
// name      ?UNK1@MainInterface@@QAEXH@Z
// size      3 bytes
// spans     0x0045C280-0x0045C283
// prototype void (__thiscall ?UNK1@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0045CC80 BYTE_EXACT
// name      ?on_iface_scrolled@MainInterface@@QAEXHH@Z
// size      3 bytes
// spans     0x0045CC80-0x0045CC83
// prototype void (__thiscall ?on_iface_scrolled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0045CC90 BYTE_EXACT
// name      ?on_iface_scrolling@MainInterface@@QAEXHH@Z
// size      3 bytes
// spans     0x0045CC90-0x0045CC93
// prototype void (__thiscall ?on_iface_scrolling@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0045CCA0 BYTE_EXACT
// name      ?on_iface_button_toggled@MainInterface@@QAEXHH@Z
// size      3 bytes
// spans     0x0045CCA0-0x0045CCA3
// prototype void (__thiscall ?on_iface_button_toggled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0045CDB0 BYTE_EXACT
// name      ?on_iface_right_double_click@MainInterface@@QAEXHH@Z
// size      3 bytes
// spans     0x0045CDB0-0x0045CDB3
// prototype void (__thiscall ?on_iface_right_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x0045D160 BYTE_EXACT
// name      ?on_iface_button_pressed@MainInterface@@QAEHH@Z
// size      5 bytes
// spans     0x0045D160-0x0045D165
// prototype int (__thiscall ?on_iface_button_pressed@MainInterface@@QAEHH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int MainInterface::on_iface_button_pressed(int) {
    return 0;
}

int __fastcall main_interface_on_iface_button_pressed_redirect(MainInterface *self, void *, int a1) {
    return self->on_iface_button_pressed(a1);
}

func_main_interface_on_iface_button_clicked MainInterfaceOnIfaceButtonClicked =
    original_method<func_main_interface_on_iface_button_clicked>(0x0045D170);

/*
Purpose: Act on a click of the interface button `a1`.
         Body unrecovered; forwards to the original image.
Forwards To: 0045D170
Return Value: n/a
Status: Forwarder
*/
void MainInterface::on_iface_button_clicked(int a1) {
    (ORIGINAL(this)->*MainInterfaceOnIfaceButtonClicked)(a1);
}
