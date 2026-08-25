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
#include "worldwin.h"
#include "graphicwin.h"
#include "filewin.h"

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
// ORIGINAL: 0x0045C170 ?clear_message@MainInterface@@QAEXXZ 0x0045C170-0x0045C171 BYTE_EXACT
// size      1 bytes
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
// ORIGINAL: 0x0058EE50 ?desktop_update@@YAXXZ 0x0058EE50-0x0058EE51 BYTE_EXACT
// size      1 bytes
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
// ORIGINAL: 0x0045C5B0 ?UNK2@MainInterface@@QAEXXZ 0x0045C5B0-0x0045C5B1 BYTE_EXACT
// size      1 bytes
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
// ORIGINAL: 0x0045EB60 ?UNK3@MainInterface@@QAEXXZ 0x0045EB60-0x0045EB61 BYTE_EXACT
// size      1 bytes
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
// ORIGINAL: 0x0045EB70 ?UNK4@MainInterface@@QAEXXZ 0x0045EB70-0x0045EB71 BYTE_EXACT
// size      1 bytes
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





/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045C280 ?UNK1@MainInterface@@QAEXH@Z 0x0045C280-0x0045C283 BYTE_EXACT
// size      3 bytes
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


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CC80 ?on_iface_scrolled@MainInterface@@QAEXHH@Z 0x0045CC80-0x0045CC83 BYTE_EXACT
// size      3 bytes
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


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CC90 ?on_iface_scrolling@MainInterface@@QAEXHH@Z 0x0045CC90-0x0045CC93 BYTE_EXACT
// size      3 bytes
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


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CCA0 ?on_iface_button_toggled@MainInterface@@QAEXHH@Z 0x0045CCA0-0x0045CCA3 BYTE_EXACT
// size      3 bytes
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


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CDB0 ?on_iface_right_double_click@MainInterface@@QAEXHH@Z 0x0045CDB0-0x0045CDB3 BYTE_EXACT
// size      3 bytes
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


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x0045D160 ?on_iface_button_pressed@MainInterface@@QAEHH@Z 0x0045D160-0x0045D165 BYTE_EXACT
// size      5 bytes
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



