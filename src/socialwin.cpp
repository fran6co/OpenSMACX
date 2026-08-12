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
ORIGINAL: 0x004AEF00 BYTE_EXACT
// name      ?UNK2@SocialWin@@QAEXXZ
// size      1 bytes
// spans     0x004AEF00-0x004AEF01
// prototype void (__thiscall ?UNK2@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void SocialWin::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x004B1790 BYTE_EXACT
// name      ?UNK3@SocialWin@@QAEXXZ
// size      1 bytes
// spans     0x004B1790-0x004B1791
// prototype void (__thiscall ?UNK3@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   0   call targets   0
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
ORIGINAL: 0x004B2400 BYTE_EXACT
// name      ?on_iface_left_down@SocialWin@@QAEXHH@Z
// size      3 bytes
// spans     0x004B2400-0x004B2403
// prototype void (__thiscall ?on_iface_left_down@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x004B2410 BYTE_EXACT
// name      ?on_iface_right_down@SocialWin@@QAEXHH@Z
// size      3 bytes
// spans     0x004B2410-0x004B2413
// prototype void (__thiscall ?on_iface_right_down@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x004B2440 BYTE_EXACT
// name      ?on_iface_selected@SocialWin@@QAEXHH@Z
// size      3 bytes
// spans     0x004B2440-0x004B2443
// prototype void (__thiscall ?on_iface_selected@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x004B2450 BYTE_EXACT
// name      ?on_iface_left_double_click@SocialWin@@QAEXHH@Z
// size      3 bytes
// spans     0x004B2450-0x004B2453
// prototype void (__thiscall ?on_iface_left_double_click@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x004B2460 BYTE_EXACT
// name      ?on_iface_right_double_click@SocialWin@@QAEXHH@Z
// size      3 bytes
// spans     0x004B2460-0x004B2463
// prototype void (__thiscall ?on_iface_right_double_click@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x004B2670 BYTE_EXACT
// name      ?on_iface_button_toggled@SocialWin@@QAEXHH@Z
// size      3 bytes
// spans     0x004B2670-0x004B2673
// prototype void (__thiscall ?on_iface_button_toggled@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x004B2680 BYTE_EXACT
// name      ?on_iface_group_clicked@SocialWin@@QAEXHHH@Z
// size      3 bytes
// spans     0x004B2680-0x004B2683
// prototype void (__thiscall ?on_iface_group_clicked@SocialWin@@QAEXHHH@Z)(SocialWin* this, int, int, int)
// callers   0   call targets   0
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
ORIGINAL: 0x004B2420
// name      ?on_iface_mouse_move@SocialWin@@QAEXHH@Z
// size      13 bytes
// spans     0x004B2420-0x004B242D
// prototype void (__thiscall ?on_iface_mouse_move@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
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
ORIGINAL: 0x004B2430
// name      ?on_iface_mouse_leave@SocialWin@@QAEXHH@Z
// size      13 bytes
// spans     0x004B2430-0x004B243D
// prototype void (__thiscall ?on_iface_mouse_leave@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
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
ORIGINAL: 0x004B2690 BYTE_EXACT
// name      ?show@SocialWin@@QAEXH@Z
// size      27 bytes
// spans     0x004B2690-0x004B26AB
// prototype void (__thiscall ?show@SocialWin@@QAEXH@Z)(SocialWin* this, int)
// callers   1   call targets   2
Status: Complete
*/
void SocialWin::show(int a1) {
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        SubInterface::set_iface_mode();
    }
}

/*
ORIGINAL: 0x004B26B0 BYTE_EXACT
// name      ?hide@SocialWin@@QAEXXZ
// size      25 bytes
// spans     0x004B26B0-0x004B26C9
// prototype void (__thiscall ?hide@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   1   call targets   2
Status: Complete
*/
void SocialWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        SubInterface::release_iface_mode();
    }
}
