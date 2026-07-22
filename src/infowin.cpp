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
#include "infowin.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 004562F0
Return Value: n/a
Status: Complete
*/
void InfoWin::UNK1() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00458910
Return Value: n/a
Status: Complete
*/
void InfoWin::on_redraw() {
}

void __fastcall info_win_unk1_redirect(InfoWin *self, void *) {
    self->UNK1();
}

void __fastcall info_win_on_redraw_redirect(InfoWin *self, void *) {
    self->on_redraw();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00458900
Return Value: n/a
Status: Complete
*/
void InfoWin::change(int) {
}

void __fastcall info_win_change_redirect(InfoWin *self, void *, int a1) {
    self->change(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00458920
Return Value: n/a
Status: Complete
*/
void InfoWin::UNK3(int, int) {
}

void __fastcall info_win_unk3_redirect(InfoWin *self, void *, int a1, int a2) {
    self->UNK3(a1, a2);
}

func_info_win_timer_proc *InfoWinOriginalTimerProc =
    (func_info_win_timer_proc *)0x00459150;

/*
Purpose: Cancel the pending info-window timer, if one is outstanding.
Original Offset: 00459280
Return Value: n/a
Status: Complete
*/
void InfoWin::reset() {
    if (field_A8_) {
        field_A8_ = 0;
        InfoWinOriginalTimerProc(this, 1);
    }
}

void __fastcall info_win_reset_redirect(InfoWin *self, void *) {
    self->reset();
}
