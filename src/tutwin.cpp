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
#include "tutwin.h"

// The tutorial's "already shown" marker, cleared alongside the window's own
// state. Bound at its canonical address because nothing here owns it yet.
uint32_t *TutWinShownFlag = (uint32_t *)0x008CC244;

/*
Purpose: Reset the tutorial window - four fields to zero, four to -1, and the
         shared shown-marker cleared with them.
Original Offset: 004BA720
Return Value: n/a
Status: Complete
*/
void TutWin::UNK1() {
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    *TutWinShownFlag = 0;
}

/*
Purpose: Store a single value at 0x53D4.
Original Offset: 004BDDD0
Return Value: n/a
Status: Complete
*/
void TutWin::UNK3(int a1) {
    field_53D4_ = a1;
}

void __fastcall tut_win_unk1_redirect(TutWin *self, void *) {
    self->UNK1();
}

void __fastcall tut_win_unk3_redirect(TutWin *self, void *, int a1) {
    self->UNK3(a1);
}
