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
#include "time.h"
#include "win_message_state.h"

// Timer-message draining is a separate call boundary for setup, stop
// and lifecycle callers. This source home does not establish the
// original filename or translation-unit boundary.

/*
Purpose: Clear timer message queue.
// ORIGINAL: 0x005FD370 ?flush_timer@@YAXXZ 0x005FD370-0x005FD3F2 BYTE_EXACT
// size      130 bytes
// prototype 
// callers   8   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x005FD39C 0x005FD3B5 0x005FD3BC 0x005FD3D1
Return Value: n/a
Status: Complete
*/
void __cdecl flush_timer() {
    MsgStatus |= 0x3F;
    MSG msg;
    while (PeekMessage(&msg, NULL, WM_USER + 1, WM_USER + 1, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    MsgStatus = 0;
}
