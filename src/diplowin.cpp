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
#include "diplowin.h"
#include "vtable_shim.h"
// DiploWin::show reaches the SubInterface subobject the original places at
// +0xA14 by casting a hand-computed address, so the type must be complete.
#include "subinterface.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00442A00 ?UNK5@DiploWin@@QAEXXZ 0x00442A00-0x00442A01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK5@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK5() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004429F0 ?UNK4@DiploWin@@QAEXH@Z 0x004429F0-0x004429F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK4@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK4(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443D80 ?on_iface_left_click@DiploWin@@QAEXHH@Z 0x00443D80-0x00443D83 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443D90 ?on_iface_left_up@DiploWin@@QAEXHH@Z 0x00443D90-0x00443D93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_up@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_up(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DA0 ?on_iface_right_click@DiploWin@@QAEXHH@Z 0x00443DA0-0x00443DA3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DB0 ?on_iface_left_down@DiploWin@@QAEXHH@Z 0x00443DB0-0x00443DB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DC0 ?on_iface_right_down@DiploWin@@QAEXHH@Z 0x00443DC0-0x00443DC3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DD0 ?on_iface_mouse_move@DiploWin@@QAEXHH@Z 0x00443DD0-0x00443DD3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DE0 ?on_iface_mouse_leave@DiploWin@@QAEXHH@Z 0x00443DE0-0x00443DE3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DF0 ?on_iface_selected@DiploWin@@QAEXHH@Z 0x00443DF0-0x00443DF3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E00 ?on_iface_left_double_click@DiploWin@@QAEXHH@Z 0x00443E00-0x00443E03 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E10 ?on_iface_right_double_click@DiploWin@@QAEXHH@Z 0x00443E10-0x00443E13 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E20 ?on_iface_button_clicked@DiploWin@@QAEXH@Z 0x00443E20-0x00443E23 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_button_clicked(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E30 ?on_iface_button_toggled@DiploWin@@QAEXHH@Z 0x00443E30-0x00443E33 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00443E40 ?on_iface_dialog_back_draw@DiploWin@@QAEHH@Z 0x00443E40-0x00443E45 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?on_iface_dialog_back_draw@DiploWin@@QAEHH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int DiploWin::on_iface_dialog_back_draw(int) {
    return 0;
}


/*
Purpose: Clear the two cached values at 0xA24 and 0xA28.
// ORIGINAL: 0x004413B0 ?UNK2@DiploWin@@QAEXXZ 0x004413B0-0x004413BE
// size      14 bytes
// prototype void (__thiscall ?UNK2@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK2() {
    field_A24_ = 0;
    field_A28_ = 0;
}


/*
Purpose: Clear one entry of the array at 0xA1C, then clear the two words at
         0xA24 and 0xA28.

             mov dword [ecx+eax*4+0xA1C],0
             lea eax,[ecx+0xA24] / xor ecx,ecx
             mov [eax],ecx / mov [eax+4],ecx

         The index is NOT bounds checked - the original stores through
         `eax*4` whatever the caller passed - so this does not add a guard the
         program does not have.

         Note the array at 0xA1C overlaps the two words cleared afterwards:
         index 2 IS the word at 0xA24, and index 3 the one at 0xA28. So calling
         with 2 or 3 writes the same slot twice, which is visible only because
         the fixture checks exact bytes rather than the cleared entry alone.
// ORIGINAL: 0x004413C0 ?UNK3@DiploWin@@QAEXH@Z 0x004413C0-0x004413E2
// size      34 bytes
// prototype void (__thiscall ?UNK3@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK3(int a1) {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    *reinterpret_cast<int32_t *>(self + 0xA1C + a1 * 4) = 0;
    int32_t *const pair = reinterpret_cast<int32_t *>(self + 0xA24);
    pair[0] = 0;
    pair[1] = 0;
}


/*
// ORIGINAL: 0x00443C30 ?show@DiploWin@@QAEXH@Z 0x00443C30-0x00443C4B BYTE_EXACT
// symbol    ?show@DiploWin@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void DiploWin::show(int visible) {
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(reinterpret_cast<char *>(this) + 0xa14)->set_iface_mode();
    }
}

/*
// ORIGINAL: 0x00443C50 ?hide@DiploWin@@QAEXXZ 0x00443C50-0x00443C68 BYTE_EXACT
// symbol    ?hide@DiploWin@@UAEXXZ
// size      24 bytes
// prototype void (__thiscall ?hide@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// indirect  0x00443C60
Status: Complete
*/
void DiploWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        release_modal();
    }
}
