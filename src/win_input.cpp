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
#include "win.h"
#include "sound.h"
#include "multidebug.h"
#include "jackal.h"
#include "scroll.h"

/*
Purpose: Drain the keyboard queue, clear the two pointer-owner slots and let
         the network layer poll, then do the same for the mouse queue - each
         PeekMessage loop PEELED, one check outside before the do-while, as
         the image has it.
// ORIGINAL: 0x005FD120 ?flush_input@@YAXXZ 0x005FD120-0x005FD1BA BYTE_EXACT
// size      154 bytes
// prototype void (__cdecl ?flush_input@@YAXXZ)()
// callers   17   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062D5D0
// indirect  0x005FD13F 0x005FD157 0x005FD180 0x005FD198
Return Value: n/a
Status: Complete
*/
void __cdecl flush_input() {
    MSG msg;
    if (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
        do {
        } while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE));
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
    if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
        do {
        } while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
                             PM_REMOVE));
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
    check_net();
}

/*
Purpose: Drain every pending WM_CHAR, pumping video and net between polls.
         HOMED from src/recovered/005fcfe0.cpp on 2026-08-25. Its three
         USER32 entry points came from tools/iat_names.py - the PE import
         table names them exactly - and 0x009B7B9C is temp.cpp's MsgStatus.
*/
// ORIGINAL: 0x005FCFE0 ?do_all_chars@@YAXXZ 0x005FCFE0-0x005FD04B BYTE_EXACT
// symbol    ?do_all_chars@@YAXXZ
// size      108 bytes
// kind      game
// flags     hidden;sp_ready;purged_ok
void __cdecl do_all_chars() {
    MSG msg;  // the artifact spelled this `int msg[7]`; MSG is those 28 bytes

    MsgStatus = 4;


    int has_message;
    do {
        do_video();
        check_net();
        has_message = PeekMessageA(&msg, 0, 0x102, 0x102, 1);
        if (has_message) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    } while (has_message);

    MsgStatus = 0;
    do_net();
}
