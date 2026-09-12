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

// System timer cancellation imports used by the shared stop implementation.
// RECOVERY_IMPORT: v1 __imp__timeKillEvent@4 WINMM.dll name:timeKillEvent
// RECOVERY_IMPORT: v1 __imp__KillTimer@8 USER32.dll name:KillTimer

// Timer teardown belongs together. Other TUs see only the close declaration;
// close and destruction share a private teardown implementation in this TU.
// The retained TRIED notes below describe the former mixed-TU arrangement.

inline void Time::close_impl() {
    stop();
    callback1_ = 0;
    oneshot_state_ = 0;
    callback2_ = 0;
    cb_param2_ = 0;
    cb_param1_ = 0;
    count_ = 0;
    resolution_ = 5;
    tick_posted_ = 0;
    unk_2_ = 0;
}

/*
Purpose: Close out an instance of the class.
// ORIGINAL: 0x00616780 ?close@Time@@QAEXXZ 0x00616780-0x006167E0 BYTE_EXACT
// size      96 bytes
// prototype void (__thiscall ?close@Time@@QAEXXZ)(Time* this)
// callers   16   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061679E 0x006167AC
// TRIED: moving the definition out of the class into this file (declared
//   MEASURED inline in time.cpp, ahead of ~Time) so that the units that CALL
//   it - stop_timers 0x50F440, go_reset 0x50EF50, Console::close 0x51D873,
//   EditBox::close 0x614FC3, NetMsg::close, FlatButton's `time_close_call` -
//   would stop expanding it. It did not reach stop_timers: this unit still
//   sees the body, and VC6's inline pass is not order-sensitive, so
//   stop_timers expanded it exactly as before (0 of 6, 90 instructions both
//   ways). Reverted, because the in-class body is what game.cpp's two
//   inlined stop_timers expansions (0x58F040, 0x58F450) need - the image
//   expands close() THERE and calls it in stop_timers, which no single
//   declaration can give both. See the LEVER note under 0x0050F440.
Return Value: n/a
Status: Complete
*/


void Time::close() {
    close_impl();
}

/*
// ORIGINAL: 0x00616200 ??1Time@@QAE@XZ 0x00616200-0x00616260 BYTE_EXACT
// size      96 bytes
// prototype void (__thiscall ??1Time@@QAE@XZ)(Time* this)
// callers   30   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061621E 0x0061622C
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
Time::~Time() {
    close_impl();
}
