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
#include "replaywin.h"

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x005AD3D0 BYTE_EXACT
// name      ?on_left_double_click@ReplayWin@@QAEXHH@Z
// size      3 bytes
// spans     0x005AD3D0-0x005AD3D3
// prototype void (__thiscall ?on_left_double_click@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x005AD3E0 BYTE_EXACT
// name      ?on_right_double_click@ReplayWin@@QAEXHH@Z
// size      3 bytes
// spans     0x005AD3E0-0x005AD3E3
// prototype void (__thiscall ?on_right_double_click@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_right_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x005AD3F0 BYTE_EXACT
// name      ?on_mouse_move@ReplayWin@@QAEXHH@Z
// size      3 bytes
// spans     0x005AD3F0-0x005AD3F3
// prototype void (__thiscall ?on_mouse_move@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x005AD400 BYTE_EXACT
// name      ?on_right_down@ReplayWin@@QAEXHH@Z
// size      3 bytes
// spans     0x005AD400-0x005AD403
// prototype void (__thiscall ?on_right_down@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_right_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x005AD410 BYTE_EXACT
// name      ?on_left_down@ReplayWin@@QAEXHH@Z
// size      3 bytes
// spans     0x005AD410-0x005AD413
// prototype void (__thiscall ?on_left_down@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_left_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x005AD420 BYTE_EXACT
// name      ?on_left_up@ReplayWin@@QAEXHH@Z
// size      3 bytes
// spans     0x005AD420-0x005AD423
// prototype void (__thiscall ?on_left_up@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_left_up(int, int) {
}

void __fastcall replay_win_on_left_double_click_redirect(ReplayWin *self, void *, int a1, int a2) {
    self->on_left_double_click(a1, a2);
}

void __fastcall replay_win_on_right_double_click_redirect(ReplayWin *self, void *, int a1, int a2) {
    self->on_right_double_click(a1, a2);
}

void __fastcall replay_win_on_mouse_move_redirect(ReplayWin *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

void __fastcall replay_win_on_right_down_redirect(ReplayWin *self, void *, int a1, int a2) {
    self->on_right_down(a1, a2);
}

void __fastcall replay_win_on_left_down_redirect(ReplayWin *self, void *, int a1, int a2) {
    self->on_left_down(a1, a2);
}

void __fastcall replay_win_on_left_up_redirect(ReplayWin *self, void *, int a1, int a2) {
    self->on_left_up(a1, a2);
}

/*
ORIGINAL: 0x005ADBD0 BYTE_EXACT
// name      ?timer_callback_daemon@ReplayWin@@QAAXH@Z
// size      17 bytes
// spans     0x005ADBD0-0x005ADBE1
// prototype 
// callers   0   call targets   1
Status: Complete
*/
void __cdecl fn_005adbd0(ReplayWin* a1) {
    if (a1) {
        a1->timer_callback();
    }
}

func_replay_win_timer_callback ReplayWinTimerCallback =
    original_method<func_replay_win_timer_callback>(0x005AD9E0);

/*
Purpose: Advance the replay one tick. The body at 0x005AD9E0 is NOT recovered;
         this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void ReplayWin::timer_callback() {
    (ORIGINAL(this)->*ReplayWinTimerCallback)();
}
