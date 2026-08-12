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
#include "messagewin.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004713A0 BYTE_EXACT
// name      ?UNK1@MessageWin@@QAEXHHHHH@Z
// size      3 bytes
// spans     0x004713A0-0x004713A3
// prototype void (__thiscall ?UNK1@MessageWin@@QAEXHHHHH@Z)(MessageWin* this, int, int, int, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void MessageWin::UNK1(int, int, int, int, int) {
}

void __fastcall message_win_unk1_redirect(MessageWin *self, void *, int a1, int a2, int a3, int a4, int a5) {
    self->UNK1(a1, a2, a3, a4, a5);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00471D40 BYTE_EXACT
// name      ?UNK4@MessageWin@@QAEXH@Z
// size      3 bytes
// spans     0x00471D40-0x00471D43
// prototype void (__thiscall ?UNK4@MessageWin@@QAEXH@Z)(MessageWin* this, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void MessageWin::UNK4(int) {
}

void __fastcall message_win_unk4_redirect(MessageWin *self, void *, int a1) {
    self->UNK4(a1);
}
