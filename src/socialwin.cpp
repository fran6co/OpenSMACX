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
#include "win.h"
#include "subinterface.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004AEF00 ?UNK2@SocialWin@@QAEXXZ 0x004AEF00-0x004AEF01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK2@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004B1790 ?UNK3@SocialWin@@QAEXXZ 0x004B1790-0x004B1791 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK3@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004B2400 ?on_iface_left_down@SocialWin@@QAEXHH@Z 0x004B2400-0x004B2403 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004B2410 ?on_iface_right_down@SocialWin@@QAEXHH@Z 0x004B2410-0x004B2413 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004B2440 ?on_iface_selected@SocialWin@@QAEXHH@Z 0x004B2440-0x004B2443 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004B2450 ?on_iface_left_double_click@SocialWin@@QAEXHH@Z 0x004B2450-0x004B2453 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004B2460 ?on_iface_right_double_click@SocialWin@@QAEXHH@Z 0x004B2460-0x004B2463 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004B2670 ?on_iface_button_toggled@SocialWin@@QAEXHH@Z 0x004B2670-0x004B2673 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004B2680 ?on_iface_group_clicked@SocialWin@@QAEXHHH@Z 0x004B2680-0x004B2683 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_group_clicked@SocialWin@@QAEXHHH@Z)(SocialWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_group_clicked(int, int, int) {
}

void __fastcall social_win_on_iface_group_clicked_redirect(SocialWin *self, void *, int a1, int a2, int a3) {
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
// ORIGINAL: 0x004B2420 ?on_iface_mouse_move@SocialWin@@QAEXHH@Z 0x004B2420-0x004B242D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?on_iface_mouse_move@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_mouse_move(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}

void __fastcall social_win_on_iface_mouse_move_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
}

/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x004B2430 ?on_iface_mouse_leave@SocialWin@@QAEXHH@Z 0x004B2430-0x004B243D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}

void __fastcall social_win_on_iface_mouse_leave_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

/*
// ORIGINAL: 0x004B2690 ?show@SocialWin@@QAEXH@Z 0x004B2690-0x004B26AB BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?show@SocialWin@@QAEXH@Z)(SocialWin* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void SocialWin::show(int a1) {
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        SubInterface::set_iface_mode();
    }
}

/*
// ORIGINAL: 0x004B26B0 ?hide@SocialWin@@QAEXXZ 0x004B26B0-0x004B26C9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?hide@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005F7E90
Status: Complete
*/
void SocialWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        SubInterface::release_iface_mode();
    }
}
