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
#include "temp.h" // MsgStatus, HandleMain
#include "time.h"

Time *Time::TimeModal;
int Time::TimeInitCount;

/*
Purpose: Initialize an instance of the class with a single parameter callback.
ORIGINAL: 0x00616260
// name      ?init@Time@@QAEXP6AXH@ZHHH@Z
// size      110 bytes
// spans     0x00616260-0x006162CE
// prototype void (__thiscall ?init@Time@@QAEXP6AXH@ZHHH@Z)(Time* this, void (__cdecl *)(int this), int, int, int)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061627E 0x0061628C
Return Value: n/a
Status: Complete
*/
void Time::init(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res) {
    stop();
    unk_tgl_ = 0;
    callback1_ = callback;
    callback2_ = 0;
    cb_param2_ = 0;
    cb_param1_ = param;
    count_ = cnt;
    unk_1_ = 0;
    resolution_ = res;
    unk_2_ = 0;
}

/*
Purpose: Initialize an instance of the class with a two parameter callback.
ORIGINAL: 0x006162D0
// name      ?init@Time@@QAEXP6AXHH@ZHHHH@Z
// size      114 bytes
// spans     0x006162D0-0x00616342
// prototype void (__thiscall ?init@Time@@QAEXP6AXHH@ZHHHH@Z)(Time* this, void (__cdecl *)(int this, int), int, int, int, int)
// callers   5   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x006162EE 0x006162FC
Return Value: n/a
Status: Complete
*/
void Time::init(void(__cdecl *callback)(int, int), int param, int param2, uint32_t cnt, 
                uint32_t res) {
    stop();
    unk_tgl_ = 0;
    callback1_ = 0;
    callback2_ = callback;
    cb_param2_ = param2;
    cb_param1_ = param;
    count_ = cnt;
    unk_1_ = 0;
    resolution_ = res;
    unk_2_ = 0;
}

/*
Purpose: Start an instance of the class with a single parameter callback.
ORIGINAL: 0x00616350
// name      ?start@Time@@QAEHP6AXH@ZHHH@Z
// size      181 bytes
// spans     0x00616350-0x00616405
// prototype int (__thiscall ?start@Time@@QAEHP6AXH@ZHHH@Z)(Time* this, void (__cdecl *)(int this), int, int, int uDelay)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061636E 0x0061637C 0x006163D8 0x006163ED
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::start(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res) {
    init(callback, param, cnt, res);
    if (!callback) {
        return 7;
    }
    unk_tgl_ = 0;
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc, 
            (UINT_PTR)this, TIME_PERIODIC);
    } else {
        id_event_ = SetTimer(*HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start an instance of the class with a two parameter callback.
ORIGINAL: 0x00616410
// name      ?start@Time@@QAEXP6AXHH@ZHHHH@Z
// size      186 bytes
// spans     0x00616410-0x006164CA
// prototype void (__thiscall ?start@Time@@QAEXP6AXHH@ZHHHH@Z)(Time* this, void (__cdecl *)(int this, int), int, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061642E 0x0061643C 0x0061649C 0x006164B2
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::start(void(__cdecl *callback)(int, int), int param, int param2, uint32_t cnt, 
                     uint32_t res) {
    init(callback, param, param2, cnt, res);
    if (!callback) {
        return 7;
    }
    unk_tgl_ = 0;
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc, 
            (UINT_PTR)this, TIME_PERIODIC);
    } else {
        id_event_ = SetTimer(*HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start a pulse instance of the class with a single parameter callback.
ORIGINAL: 0x006164D0
// name      ?pulse@Time@@QAEHP6AXH@ZHHH@Z
// size      181 bytes
// spans     0x006164D0-0x00616585
// prototype int (__thiscall ?pulse@Time@@QAEHP6AXH@ZHHH@Z)(Time* this, void (__cdecl *)(int this), int, int, int uDelay)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x006164EE 0x006164FC 0x00616558 0x0061656D
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::pulse(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res) {
    init(callback, param, cnt, res);
    if (!callback) {
        return 7;
    }
    unk_tgl_ = 0;
    if (count_ < 50) {
        // Bug fix: Original code had fuEvent param set to TIME_PERIODIC, the same as start().
        // Based on how the other pulse() functions work, this should be TIME_ONESHOT. It appears
        // this code branch is never used in the original game.
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc, 
            (UINT_PTR)this, TIME_ONESHOT);
    } else {
        id_event_ = SetTimer(*HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start a pulse instance of the class with a two parameter callback.
ORIGINAL: 0x00616590
// name      ?pulse@Time@@QAEXP6AXHH@ZHHHH@Z
// size      189 bytes
// spans     0x00616590-0x0061664D
// prototype void (__thiscall ?pulse@Time@@QAEXP6AXHH@ZHHHH@Z)(Time* this, void (__cdecl *)(int this, int), int, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x006165AE 0x006165BC 0x00616619 0x00616635
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::pulse(void(__cdecl *callback)(int, int), int param, int param2, uint32_t cnt, 
                     uint32_t res) {
    init(callback, param, param2, cnt, res);
    if (!callback) {
        return 7;
    }
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc, 
            (UINT_PTR)this, TIME_ONESHOT);
    } else {
        unk_tgl_ = 1;
        id_event_ = SetTimer(*HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start an instance of the timer.
ORIGINAL: 0x00616650
// name      ?start@Time@@QAEHXZ
// size      106 bytes
// spans     0x00616650-0x006166BA
// prototype int (__thiscall ?start@Time@@QAEHXZ)(Time* this)
// callers   11   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00616684 0x006166A6
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::start() {
    if (!callback1_ && !callback2_) {
        return 7;
    }
    unk_tgl_ &= ~1;
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc,
            (UINT_PTR)this, TIME_PERIODIC);
    } else {
        id_event_ = SetTimer(*HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start an instance of the pulse timer.
ORIGINAL: 0x006166C0
// name      ?pulse@Time@@QAEHXZ
// size      108 bytes
// spans     0x006166C0-0x0061672C
// prototype int (__thiscall ?pulse@Time@@QAEHXZ)(Time* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x006166ED 0x00616718
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::pulse() {
    if (!callback1_ && !callback2_) {
        return 7;
    }
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc, 
            (UINT_PTR)this, TIME_ONESHOT);
    } else {
        unk_tgl_ = (unk_tgl_ & ~2) | 1;
        id_event_ = SetTimer(*HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Stop an instance of the class.
ORIGINAL: 0x00616730
// name      ?stop@Time@@QAEXXZ
// size      66 bytes
// spans     0x00616730-0x00616772
// prototype void (__thiscall ?stop@Time@@QAEXXZ)(Time* this)
// callers   30   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x0061674B 0x00616759
Return Value: n/a
Status: Complete
*/
void Time::stop() {
    if (id_event_) {
        if (count_ < 50) {
            timeKillEvent(id_event_);
        } else {
            KillTimer(*HandleMain, id_event_);
        }
        id_event_ = 0;
    }
    if (~unk_tgl_ & 1) {
        flush_timer();
    }
}

/*
Purpose: Close out an instance of the class.
ORIGINAL: 0x00616780
// name      ?close@Time@@QAEXXZ
// size      96 bytes
// spans     0x00616780-0x006167E0
// prototype void (__thiscall ?close@Time@@QAEXXZ)(Time* this)
// callers   16   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061679E 0x006167AC
Return Value: n/a
Status: Complete
*/
void Time::close() {
    stop();
    unk_tgl_ = 0;
    callback1_ = 0;
    callback2_ = 0;
    cb_param2_ = 0;
    cb_param1_ = 0;
    count_ = 0;
    unk_1_ = 0;
    resolution_ = 5;
    unk_2_ = 0;
}

/*
Purpose: Callback function that processes WM_TIMER messages (SetTimer).
ORIGINAL: 0x006167E0
// name      ?TimerProc@Time@@QAEXPAXIIK@Z
// size      60 bytes
// spans     0x006167E0-0x0061681C
// prototype void (__thiscall ?TimerProc@Time@@QAEXPAXIIK@Z)(Time* this, HWND hwnd, uint32_t msg, uint32_t* idEvent, uint32_t dwTime)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x0061680B
Return Value: n/a
Status: Complete
*/
void Time::TimerProc(HWND UNUSED(hwnd), uint32_t UNUSED(msg), UINT_PTR id_timer, 
                     DWORD UNUSED(elapsed)) {
    if (id_timer && (!TimeModal || id_timer == (UINT_PTR)TimeModal)
        && !reinterpret_cast<Time *>(id_timer)->unk_1_) {
        PostMessageA(*HandleMain, WM_USER + 1, id_timer, 0);
        reinterpret_cast<Time *>(id_timer)->unk_1_ = 1;
    }
}

/*
Purpose: Callback function for the timeSetEvent. Effectively the same as TimerProc.
ORIGINAL: 0x00616820
// name      ?MultimediaProc@Time@@QAEXIIKKK@Z
// size      60 bytes
// spans     0x00616820-0x0061685C
// prototype void (__thiscall ?MultimediaProc@Time@@QAEXIIKKK@Z)(Time* this, UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0061684B
Return Value: n/a
Status: Complete
*/
void Time::MultimediaProc(uint32_t UNUSED(timer_id), uint32_t UNUSED(msg), DWORD_PTR user,
                          DWORD_PTR UNUSED(dw1), DWORD_PTR UNUSED(dw2)) {
    if (user && (!TimeModal || user == (DWORD_PTR)TimeModal) 
        && !reinterpret_cast<Time *>(user)->unk_1_) {
        PostMessageA(*HandleMain, WM_USER + 1, user, 0);
        reinterpret_cast<Time *>(user)->unk_1_ = 1;
    }
}

// global
Time *TurnTimer = (Time *)0x00915628;
Time *LineTimer = (Time *)0x00915658;
Time *BlinkTimer = (Time *)0x00915688;
Time *Blink2Timer = (Time *)0x00939EB0;
Time *GoTimer = (Time *)0x00939E60;
Time *ConsoleTimer = (Time *)0x00939E88;

/*
Purpose: Start global timers.
ORIGINAL: 0x0050F3D0
// name      ?start_timers@@YAXXZ
// size      97 bytes
// spans     0x0050F3D0-0x0050F431
// prototype 
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616350
Return Value: n/a
Status: Complete
*/
void __cdecl start_timers() {
    BlinkTimer->start(blink_timer, 1, 150, 150);
    Blink2Timer->start(blink2_timer, 2, 100, 100);
    LineTimer->start(line_timer, 3, 100, 100);
    TurnTimer->start(turn_timer, 4, 500, 500);
}

/*
Purpose: Stop global timers.
ORIGINAL: 0x0050F440
// name      ?stop_timers@@YAXXZ
// size      30 bytes
// spans     0x0050F440-0x0050F45E
// prototype 
// callers   6   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616780
Return Value: n/a
Status: Complete
*/
void __cdecl stop_timers() {
    Blink2Timer->close();
    BlinkTimer->close();
    LineTimer->close();
    // missing MP TurnTimer. TODO: In future determine if timer should be closed on stop
}

/*
Purpose: Clear timer message queue.
ORIGINAL: 0x005FD370
// name      ?flush_timer@@YAXXZ
// size      130 bytes
// spans     0x005FD370-0x005FD3F2
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
    *MsgStatus |= 0x3F;
    MSG msg;
    while (PeekMessage(&msg, NULL, WM_USER + 1, WM_USER + 1, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    *MsgStatus = 0;
}
