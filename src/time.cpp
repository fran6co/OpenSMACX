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
#include "mapwin.h"      // MapWinTable, go_reset's window walk
#include "console.h"     // ConsoleStatusWin
#include "statuswin.h"   // StatusWin::set_title, on the object above
#include "win.h"         // WinPointerOwner3/4
#include "tutwin.h"        // TutWinBaseWindow, TutWinSocWindow
#include "game.h"          // ExitTurnLoop, IsMultiplayerNet
#include "map.h"           // WorldClimateSkipTerrainClear
#include "planwin.h"       // PlanWin::on_redraw, MapWinTable
#include "hypothesis_layouts.h"  // MultiWin::draw

// The timer callbacks' shared state; declarations in temp.h beside the
// callbacks themselves. All four are zero-initialised .data in the image.
int TimerHoldFlag;      // 0x00915620
int Blink2Counter;      // 0x009392B4
int PlanLineCounter;    // 0x00939FD4
int TurnRedrawPending;  // 0x00703DE0

// The PlanWin the line timer repaints; the image loads the receiver as an
// immediate (`mov ecx, 0x856dc0`), which a header binding folds to.
static PlanWin *const LineTimerPlanWindow = (PlanWin *)0x00856DC0;
// The MultiWin the turn timer redraws (`mov ecx, 0x7fd648`). game.cpp binds
// the same address as its GraphicWin front for desktop_close.
static MultiWin *const TurnTimerMultiWindow = (MultiWin *)0x007FD648;

Time *Time::TimeModal;
int Time::TimeInitCount;

/*
Purpose: Initialize an instance of the class with a single parameter callback.
// ORIGINAL: 0x00616260 ?init@Time@@QAEXP6AXH@ZHHH@Z 0x00616260-0x006162CE BYTE_EXACT
// symbol    ?init@Time@@QAEXP6AXH@ZHII@Z
// size      110 bytes
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
    // IN THE IMAGE'S STORE ORDER at 0x006162AA: callback1_, then the four
    // zeroed fields out of declaration order (oneshot_state_, tick_posted_,
    // unk_2_, callback2_, cb_param2_), then the three arguments.
    stop();
    callback1_ = callback;
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback2_ = 0;
    cb_param2_ = 0;
    cb_param1_ = param;
    count_ = cnt;
    resolution_ = res;
}

/*
Purpose: Initialize an instance of the class with a two parameter callback.
// ORIGINAL: 0x006162D0 ?init@Time@@QAEXP6AXHH@ZHHHH@Z 0x006162D0-0x00616342 BYTE_EXACT
// symbol    ?init@Time@@QAEXP6AXHH@ZHHII@Z
// size      114 bytes
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
    // IN THE IMAGE'S STORE ORDER at 0x0061631A, which is not the one-argument
    // overload's: the two callback fields come first here.
    stop();
    callback2_ = reinterpret_cast<void(__cdecl *)(int, Palette *)>(callback);
    cb_param2_ = reinterpret_cast<Palette *>(param2);
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback1_ = 0;
    cb_param1_ = param;
    count_ = cnt;
    resolution_ = res;
}

/*
Purpose: Start an instance of the class with a single parameter callback.
// ORIGINAL: 0x00616350 ?start@Time@@QAEHP6AXH@ZHHH@Z 0x00616350-0x00616405
// symbol    ?start@Time@@QAEIP6AXH@ZHII@Z
// CORRECTED from ?start@Time@@QAEXP6AXH@ZHHH@Z
//   tail is `and al, 0xfe; add eax, 2; ret 0x10` - a computed return;
//   0x0063C340 tests the result at 0x0063C356 with `test eax, eax`
// size      181 bytes
// prototype int (__thiscall ?start@Time@@QAEHP6AXH@ZHHH@Z)(Time* this, void (__cdecl *)(int this), int, int, int uDelay)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061636E 0x0061637C 0x006163D8 0x006163ED
// LEVER: WRONG CALLEE - init() (0x00616260, BYTE_EXACT as its own
//        out-of-line function) is hand-inlined here: the image writes its
//        whole body out at this call site (flush_timer(), reached via
//        stop(), is the only call it keeps), rather than calling
//        0x00616260. MNEMONIC_ONLY, 65/69, 1.000 similar under /c /O2 /Gy
//        /GR- /GX - `semantic` REFUSES on a displacement, not a register.
// TRIED: store_order.py shows the image storing cb_param1_ (offset
//        0x14) BEFORE oneshot_state_/tick_posted_/unk_2_/callback1_ here -
//        a DIFFERENT order than the declaration order that keeps init()
//        itself BYTE_EXACT. Reordering this copy's assignments to match
//        (cb_param1_ first) made it WORSE (MISMATCH, 57/69, best 0.971
//        similar) - the compiler's actual scheduling here is driven by
//        operand availability at this specific inlined call site, not
//        source order, and is not reachable by reordering the source.
//        Left in init()'s own declaration order.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::start(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res) {
    // init() (0x00616260, BYTE_EXACT as its own out-of-line function) is
    // hand-inlined here: the image writes its whole body out at this call
    // site (init() itself inlines stop(), whose flush_timer() call is the
    // only one that survives), rather than calling 0x00616260.
    stop();
    callback1_ = callback;
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback2_ = 0;
    cb_param2_ = 0;
    cb_param1_ = param;
    count_ = cnt;
    resolution_ = res;
    if (!callback) {
        return 7;
    }
    oneshot_state_ = 0;
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc, 
            (UINT_PTR)this, TIME_PERIODIC);
    } else {
        id_event_ = SetTimer(HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start an instance of the class with a two parameter callback.
// ORIGINAL: 0x00616410 ?start@Time@@QAEXP6AXHH@ZHHHH@Z 0x00616410-0x006164CA
// symbol    ?start@Time@@QAEIP6AXHH@ZHHII@Z
// size      186 bytes
// prototype void (__thiscall ?start@Time@@QAEXP6AXHH@ZHHHH@Z)(Time* this, void (__cdecl *)(int this, int), int, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061642E 0x0061643C 0x0061649C 0x006164B2
// LEVER: WRONG CALLEE - init() (the two-parameter-callback overload,
//        BYTE_EXACT as its own out-of-line function) is hand-inlined here:
//        the image writes its whole body out at this call site (stop()'s
//        flush_timer() call is the only one that survives), rather than
//        calling init(), matching the single-parameter `start` sibling above.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::start(void(__cdecl *callback)(int, int), int param, int param2, uint32_t cnt,
                     uint32_t res) {
    // init() (the two-parameter-callback overload, BYTE_EXACT as its own
    // out-of-line function) is hand-inlined here: the image writes its whole
    // body out at this call site (init() itself inlines stop(), whose
    // flush_timer() call is the only one that survives), rather than calling
    // it as init(callback, param, param2, cnt, res).
    stop();
    callback2_ = reinterpret_cast<void(__cdecl *)(int, Palette *)>(callback);
    cb_param2_ = reinterpret_cast<Palette *>(param2);
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback1_ = 0;
    cb_param1_ = param;
    count_ = cnt;
    resolution_ = res;
    if (!callback) {
        return 7;
    }
    oneshot_state_ = 0;
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc,
            (UINT_PTR)this, TIME_PERIODIC);
    } else {
        id_event_ = SetTimer(HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start a pulse instance of the class with a single parameter callback.
// ORIGINAL: 0x006164D0 ?pulse@Time@@QAEHP6AXH@ZHHH@Z 0x006164D0-0x00616585
// symbol    ?pulse@Time@@QAEIP6AXH@ZHII@Z
// CORRECTED from ?pulse@Time@@QAEXP6AXH@ZHHH@Z
//   tail is `and al, 0xfe; add eax, 2; ret 0x10`, the same computed
//   return as its `start` sibling
// size      181 bytes
// prototype int (__thiscall ?pulse@Time@@QAEHP6AXH@ZHHH@Z)(Time* this, void (__cdecl *)(int this), int, int, int uDelay)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x006164EE 0x006164FC 0x00616558 0x0061656D
// LEVER: WRONG CALLEE - same fix as the `start` sibling immediately above:
//        init() hand-inlined here. MNEMONIC_ONLY, 63/69, 1.000 similar.
//        See that note for the store-order TRIED (measured identically
//        here).
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::pulse(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res) {
    // init() (0x00616260, BYTE_EXACT as its own out-of-line function) is
    // hand-inlined here: the image writes its whole body out at this call
    // site (init() itself inlines stop(), whose flush_timer() call is the
    // only one that survives), rather than calling 0x00616260.
    stop();
    callback1_ = callback;
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback2_ = 0;
    cb_param2_ = 0;
    cb_param1_ = param;
    count_ = cnt;
    resolution_ = res;
    if (!callback) {
        return 7;
    }
    oneshot_state_ = 0;
    if (count_ < 50) {
        // Bug fix: Original code had fuEvent param set to TIME_PERIODIC, the same as start().
        // Based on how the other pulse() functions work, this should be TIME_ONESHOT. It appears
        // this code branch is never used in the original game.
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc, 
            (UINT_PTR)this, TIME_ONESHOT);
    } else {
        id_event_ = SetTimer(HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start a pulse instance of the class with a two parameter callback.
// ORIGINAL: 0x00616590 ?pulse@Time@@QAEXP6AXHH@ZHHHH@Z 0x00616590-0x0061664D
// symbol    ?pulse@Time@@QAEIP6AXHH@ZHHII@Z
// size      189 bytes
// prototype void (__thiscall ?pulse@Time@@QAEXP6AXHH@ZHHHH@Z)(Time* this, void (__cdecl *)(int this, int), int, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x006165AE 0x006165BC 0x00616619 0x00616635
// LEVER: WRONG CALLEE - init() (the two-parameter-callback overload,
//        BYTE_EXACT as its own out-of-line function) is hand-inlined here,
//        matching Time::start's two-parameter-callback overload above.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::pulse(void(__cdecl *callback)(int, int), int param, int param2, uint32_t cnt,
                     uint32_t res) {
    // init() (the two-parameter-callback overload, BYTE_EXACT as its own
    // out-of-line function) is hand-inlined here: the image writes its whole
    // body out at this call site, rather than calling it directly.
    stop();
    callback2_ = reinterpret_cast<void(__cdecl *)(int, Palette *)>(callback);
    cb_param2_ = reinterpret_cast<Palette *>(param2);
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback1_ = 0;
    cb_param1_ = param;
    count_ = cnt;
    resolution_ = res;
    if (!callback) {
        return 7;
    }
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc, 
            (UINT_PTR)this, TIME_ONESHOT);
    } else {
        oneshot_state_ = TimeOneShot;
        id_event_ = SetTimer(HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start an instance of the timer.
// ORIGINAL: 0x00616650 ?start@Time@@QAEHXZ 0x00616650-0x006166BA BYTE_EXACT
// symbol    ?start@Time@@QAEIXZ
// CORRECTED from ?start@Time@@QAEXXZ
//   tail is `neg eax; sbb eax, eax; and al, 0xfe; add eax, 2; ret`
// size      106 bytes
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
    oneshot_state_ &= ~TimeOneShot;
    if (count_ < 50) {
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc,
            (UINT_PTR)this, TIME_PERIODIC);
    } else {
        id_event_ = SetTimer(HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start an instance of the pulse timer.
// ORIGINAL: 0x006166C0 ?pulse@Time@@QAEHXZ 0x006166C0-0x0061672C BYTE_EXACT
// symbol    ?pulse@Time@@QAEIXZ
// CORRECTED from ?pulse@Time@@QAEXXZ
//   tail is `neg eax; sbb eax, eax; and al, 0xfe; add eax, 2; ret`
// size      108 bytes
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
        oneshot_state_ = (oneshot_state_ & ~TimeOneShotFired) | TimeOneShot;
        id_event_ = SetTimer(HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Stop an instance of the class.
// ORIGINAL: 0x00616730 ?stop@Time@@QAEXXZ 0x00616730-0x00616772 BYTE_EXACT
// body      src/time.h
// size      66 bytes
// prototype void (__thiscall ?stop@Time@@QAEXXZ)(Time* this)
// callers   30   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x0061674B 0x00616759
Return Value: n/a
Status: Complete
*/


/*
Purpose: Close out an instance of the class.
// ORIGINAL: 0x00616780 ?close@Time@@QAEXXZ 0x00616780-0x006167E0 BYTE_EXACT
// body      src/time.h
// size      96 bytes
// prototype void (__thiscall ?close@Time@@QAEXXZ)(Time* this)
// callers   16   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061679E 0x006167AC
Return Value: n/a
Status: Complete
*/


/*
Purpose: Callback function that processes WM_TIMER messages (SetTimer).
// ORIGINAL: 0x006167E0 ?TimerProc@Time@@QAEXPAXIIK@Z 0x006167E0-0x0061681C BYTE_EXACT
// symbol    ?TimerProc@Time@@SGXPAUHWND__@@IKK@Z
// size      60 bytes
// prototype void (__thiscall ?TimerProc@Time@@QAEXPAXIIK@Z)(Time* this, HWND hwnd, uint32_t msg, uint32_t* idEvent, uint32_t dwTime)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x0061680B
Return Value: n/a
Status: Complete
*/
void CALLBACK Time::TimerProc(HWND UNUSED(hwnd), uint32_t UNUSED(msg), UINT_PTR id_timer, 
                     DWORD UNUSED(elapsed)) {
    if (id_timer && (!TimeModal || id_timer == (UINT_PTR)TimeModal)
        && !reinterpret_cast<Time *>(id_timer)->tick_posted_) {
        PostMessageA(HandleMain, WM_USER + 1, id_timer, 0);
        reinterpret_cast<Time *>(id_timer)->tick_posted_ = 1;
    }
}

/*
Purpose: Callback function for the timeSetEvent. Effectively the same as TimerProc.
// ORIGINAL: 0x00616820 ?MultimediaProc@Time@@QAEXIIKKK@Z 0x00616820-0x0061685C BYTE_EXACT
// symbol    ?MultimediaProc@Time@@SGXIIKKK@Z
// size      60 bytes
// prototype void (__thiscall ?MultimediaProc@Time@@QAEXIIKKK@Z)(Time* this, UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0061684B
Return Value: n/a
Status: Complete
*/
void CALLBACK Time::MultimediaProc(uint32_t UNUSED(timer_id), uint32_t UNUSED(msg), DWORD_PTR user,
                          DWORD_PTR UNUSED(dw1), DWORD_PTR UNUSED(dw2)) {
    if (user && (!TimeModal || user == (DWORD_PTR)TimeModal) 
        && !reinterpret_cast<Time *>(user)->tick_posted_) {
        PostMessageA(HandleMain, WM_USER + 1, user, 0);
        reinterpret_cast<Time *>(user)->tick_posted_ = 1;
    }
}

// global

// The full-screen window the go mode drops over the desktop, at a fixed
// address. game.cpp binds the same address as `CoverWindow` with internal
// linkage, which time.cpp cannot name, and a second variable binding for one
// address fails the duplicate-globals ceiling - so the address comes back
// from an inline function instead, which VC6 folds to the same
// `mov ecx, imm32` receiver at the call site (the world_climate_window
// idiom, game.cpp).
inline Win *go_cover_window() {
    return (Win *)0x00937118;
}

/*
Purpose: Cancel the go mode: hide the cover window, stop the go timer, then
         reset the mode state and cursor of every live map window and of the
         selected one.
// ORIGINAL: 0x0050EF50 ?go_reset@@YAXXZ 0x0050EF50-0x0050F166
// size      534 bytes
// prototype
// callers   7   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B4A20 0x005EC7C0 0x005EDCD0 0x00616780
// indirect  0x0050EFF6 0x0050F014 0x0050F126 0x0050F144
// MEASURED 2026-08-29: /O2 best at 10/137 (0.842 similar). Image
//   instructions 0..7 agree - prologue, the cover window and
//   its direct `call Win::hide`. The counted divergence is the catalogued
//   MEASURED-inline ceiling: `GoTimer->close()` is inlined here in full
//   (stop()'s two kill branches plus the nine field clears, writing
//   0x939E60..0x939E84 directly) where the image makes one plain
//   `call 0x616780`. time.h keeps close() in-class BECAUSE ~Time must
//   inline it, so an out-of-line spelling would regress that claim; VC6
//   honours no per-site noinline. Everything after is that block's
//   register-allocation shadow (the walking cursor lands in edi instead of
//   esi), not a spelling defect this file can reach.
// TRIED: the dead cursor guard as a cached local (`cursor_sprite != 0`)
//   - same 10/137. The image loads the sprite value once for the two
//   equality compares (3D imm at 0x0050F097/0x0050F09E) but re-reads the
//   FIELD for the dead arm (39 at 0x0050F0A5), so the body re-reads it.
Return Value: n/a
Status: Complete
*/
void __cdecl go_reset() {
    go_cover_window()->Win::hide();
    GoTimer->close();

    int index = 0;
    int cursor = reinterpret_cast<int>(MapWinTable);
    const int end = cursor + static_cast<int>(MapWinTableSlots) * 4;
    do {
        MapWin *const window = *reinterpret_cast<MapWin *const *>(cursor);
        if (window != nullptr &&
            (cursor == reinterpret_cast<int>(MapWinTable) ||
             *reinterpret_cast<const volatile uint32_t *>(
                 reinterpret_cast<const uint8_t *>(window) +
                 MapWinActiveOffset) != 0) &&
            index != MapWinSelectedSlot &&
            window->field_1DD80_ != 0) {
            // The FIRST store runs off the window the tests loaded; the rest
            // re-read the slot each time, as the image re-reads [esi] for
            // every store after the first (0x0050EFB6 on).
            window->field_1DD88_ = 1;
            (*reinterpret_cast<MapWin *const *>(cursor))->field_1DD80_ = 0;
            (*reinterpret_cast<MapWin *const *>(cursor))->field_1DD8C_ = 0;
            (*reinterpret_cast<MapWin *const *>(cursor))->field_1DD84_ = 0;
            (*reinterpret_cast<MapWin *const *>(cursor))->field_1DE0C_ = -1;

            // The GraphicWin virtual base the dispatches run through, as a
            // Win* so the owner comparisons are same-type. Each use
            // re-derives the vbtable walk - the image computes the adjusted
            // receiver twice (0x0050EFDA, 0x0050F007) rather than keeping
            // one.
            const Win *const base = static_cast<GraphicWin *>(window);
            if (WinPointerOwner3 == base) {
                WinPointerOwner3 = nullptr;
                window->vslot_04();
            }
            if (WinPointerOwner4 == base) {
                WinPointerOwner4 = nullptr;
            }
            window->vslot_63();
            ConsoleStatusWin->set_title(0);
        }
        ++index;
        cursor += 4;
    } while (cursor < end);

    // Slot 0's cursor first, unconditionally.
    MapWin *const primary = MapWinTable[0];
    if (primary->field_1EF4C_ != 0 || primary->field_1EF50_ != 0x7F00) {
        primary->GraphicWin::set_cursor(0x7F00);
        primary->field_1EF50_ = 0x7F00;
        primary->field_1EF4C_ = 0;
    }

    if (MapWinSelectedSlot >= 0) {
        // The slot walks two registers in the image - the window loaded once
        // for the cursor work, the slot's own address kept for the reset
        // stores below (`lea esi, [eax*4 + 0x7d3c3c]` at 0x0050F08A).
        MapWin *const window = MapWinTable[MapWinSelectedSlot];
        MapWin **const slot = &MapWinTable[MapWinSelectedSlot];
        // Only the two go-mode cursor sprites are replaced by the default.
        // The sprite value is loaded once for the two equality tests, but
        // the dead `!= 0` arm re-reads the FIELD from memory (0x0050F0A5
        // compares [edi + 0x1ef4c] again, not the register) - the same
        // re-read-the-lvalue shape as the reset stores below. Reproduced.
        const uint32_t cursor_sprite = window->field_1EF4C_;
        if ((cursor_sprite == 0x0093AA9C || cursor_sprite == 0x0093ABFC) &&
            (window->field_1EF4C_ != 0 || window->field_1EF50_ != 0x7F00)) {
            window->GraphicWin::set_cursor(0x7F00);
            window->field_1EF50_ = 0x7F00;
            window->field_1EF4C_ = 0;
        }
        (*slot)->field_1DD88_ = 1;
        (*slot)->field_1DD80_ = 0;
        (*slot)->field_1DD8C_ = 0;
        (*slot)->field_1DD84_ = 0;
        (*slot)->field_1DE0C_ = -1;

        const Win *const base = static_cast<GraphicWin *>((*slot));
        if (WinPointerOwner3 == base) {
            WinPointerOwner3 = nullptr;
            (*slot)->vslot_04();
        }
        if (WinPointerOwner4 == base) {
            WinPointerOwner4 = nullptr;
        }
        (*slot)->vslot_63();
        ConsoleStatusWin->set_title(0);
        MapWinSelectedSlot = -1;
    }
}

/*
Purpose: Blink timer tick: skip while any window is up or the game holds the
         timers, then advance the blink counter and repaint the interface.
// ORIGINAL: 0x0050EE30 ?blink2_timer@@YAXH@Z 0x0050EE30-0x0050EE7D BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ee30 archive unit. Five
//   sequential early-return `if`s (NOT one chained `&&`): the image jumps to a
//   SHARED `ret` epilogue from every test. The counter increment and the
//   MainInterface receiver load interleave in the image (`mov eax,[ctr]; mov
//   ecx,0x7ae820; inc; mov [ctr],eax`) exactly as the two-statement spelling
//   schedules, and the redraw_complete call is the last statement, which VC6
//   tail-jumps (`jmp 0x45c3a0`).
// size      77 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
Return Value: n/a
Status: Complete
*/
void __cdecl blink2_timer(int a1) {
    if (TutWinBaseWindow->is_visible() != 0) {
        return;
    }
    if (TutWinSocWindow->is_visible() != 0) {
        return;
    }
    if (TimerHoldFlag != 0) {
        return;
    }
    if (ExitTurnLoop != 0) {
        return;
    }
    if (WorldClimateSkipTerrainClear != 0) {
        return;
    }
    ++Blink2Counter;
    MainInterfaceVar->redraw_complete();
}

/*
Purpose: Plan-line timer tick: count the line counter down and repaint the
         plan lines on every live map window.
// ORIGINAL: 0x0050EE80 ?line_timer@@YAXH@Z 0x0050EE80-0x0050EF01 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ee80 archive unit. The slot
//   walk is a POINTER walk (`slot < &MapWinTable[8]`), not an index loop -
//   the image keeps the slot pointer in esi (`cmp esi, 0x7d3c5c; jl`), and
//   "slot == &MapWinTable[0]" is the primary-window exemption
//   (`cmp esi, 0x7d3c3c; je`), as in draw_tile. The PlanWin::on_redraw call
//   sits between the counter store and the loop with the receiver loaded as
//   an immediate; the two flag reads are plain (non-volatile) dwords off the
//   window pointer, which VC6 keeps in ecx across the guards.
// size      129 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00467970 0x0048AF30 0x005F7E90
Return Value: n/a
Status: Complete
*/
void __cdecl line_timer(int a1) {
    if (WorldClimateSkipTerrainClear != 0) {
        return;
    }
    if (TimerHoldFlag != 0) {
        return;
    }
    if (TutWinBaseWindow->is_visible() != 0) {
        return;
    }
    if (TutWinSocWindow->is_visible() != 0) {
        return;
    }
    --PlanLineCounter;
    LineTimerPlanWindow->on_redraw();

    for (MapWin *const *slot = &MapWinTable[0];
         reinterpret_cast<int32_t>(slot) <
         reinterpret_cast<int32_t>(&MapWinTable[MapWinTableSlots]);
         ++slot) {
        MapWin *const window = *slot;
        if (window == nullptr) {
            continue;
        }
        if (slot != &MapWinTable[0]) {
            const uint32_t active = *reinterpret_cast<const uint32_t *>(
                reinterpret_cast<const uint8_t *>(window) +
                MapWinActiveOffset);
            if (active == 0) {
                continue;
            }
        }
        const uint32_t draw_flags = *reinterpret_cast<const uint32_t *>(
            reinterpret_cast<const uint8_t *>(window) + 0x1DD70);
        if ((draw_flags & 0xC40000) != 0) {
            window->draw_base_dest(1);
        }
    }
}

/*
Purpose: Network turn timer tick: pulse the turn window while a multiplayer
         game is running and the timers are not held.
// ORIGINAL: 0x0050EF10 ?turn_timer@@YAXH@Z 0x0050EF10-0x0050EF42 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ef10 archive unit. Three
//   nested `if`s, not chained conditions - the image tests each in turn with
//   its own jump to the shared `ret`. TurnTimerMultiWindow->draw(1) is the
//   last statement, so VC6 tails it after loading the receiver immediate.
// size      50 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00479330 0x005D5A70
Return Value: n/a
Status: Complete
*/
void __cdecl turn_timer(int a1) {
    if (IsMultiplayerNet != 0) {
        if (TimerHoldFlag == 0) {
            if (TurnRedrawPending != 0) {
                TurnRedrawWindow->redraw();
            }
            TurnTimerMultiWindow->draw(1);
        }
    }
}

/*
Purpose: Start global timers.
// ORIGINAL: 0x0050F3D0 ?start_timers@@YAXXZ 0x0050F3D0-0x0050F431 BYTE_EXACT
// size      97 bytes
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
// ORIGINAL: 0x0050F440 ?stop_timers@@YAXXZ 0x0050F440-0x0050F45E BYTE_EXACT
// size      30 bytes
// prototype 
// callers   6   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616780
Return Value: n/a
Status: Complete
*/
// BODY IN time.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


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

// ---------------------------------------------------------------------------
// Merged from the retired src/time_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x006161D0 ??0Time@@QAE@XZ 0x006161D0-0x006161F6 BYTE_EXACT
// size      38 bytes
// prototype void (__thiscall ??0Time@@QAE@XZ)(Time* this)
// callers   19   call targets   0
// kind      
// flags     
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
Time::Time() {
    // IMAGE ORDER. An initialiser list runs in DECLARATION order,
    // and the image does not write the fields in that order.
    id_event_ = 0;
    callback1_ = nullptr;
    callback2_ = nullptr;
    cb_param2_ = 0;
    cb_param1_ = 0;
    count_ = 0;
    tick_posted_ = 0;
    resolution_ = 5;
    unk_2_ = 0;
    oneshot_state_ = 0;
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
    close();
}

/*
Purpose: Make this timer the modal one.
// ORIGINAL: 0x00616860 ?set_modal@Time@@QAAXXZ 0x00616860-0x00616867 BYTE_EXACT
// symbol    ?set_modal@Time@@QAEXXZ
// size      7 bytes
// prototype void (__thiscall ?set_modal@Time@@QAAXXZ)(Time* this)
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// `QAE`, not the catalogued `QAA`: this takes a receiver, and the tree emits
// the thiscall spelling. See `init_class` below for the same correction.
Return Value: n/a
Status: Complete
*/
void Time::set_modal() {
    TimeModal = this;
}

/*
Purpose: Clear the modal timer.
// ORIGINAL: 0x00616870 ?release_modal@Time@@QAAXXZ 0x00616870-0x0061687B BYTE_EXACT
// symbol    ?release_modal@Time@@QAEXXZ
// size      11 bytes
// prototype void (__thiscall ?release_modal@Time@@QAAXXZ)(Time* this)
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// The body was already here and carried no marker.
Return Value: n/a
Status: Complete
*/
void Time::release_modal() {
    TimeModal = nullptr;
}

/*
Purpose: Count one more user of the timer class.
// ORIGINAL: 0x00616880 ?init_class@Time@@QAAHXZ 0x00616880-0x0061688E BYTE_EXACT
// symbol    ?init_class@Time@@SAHXZ
// size      15 bytes
// prototype int (__cdecl ?init_class@Time@@QAAHXZ)()
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// THE BODY WAS ALREADY HERE and carried no marker, so the catalogue only knew
// it through `src/unrecovered/00616880.cpp` - a since-deleted artifact the
// build never compiled. That is a third shape of the same defect: not a missing body, a
// missing CLAIM on a body that was always in the tree.
Return Value: 0, always
Status: Complete
*/
int __cdecl Time::init_class() {
    TimeInitCount = static_cast<int>(static_cast<uint32_t>(TimeInitCount) + 1U);
    return 0;
}

/*
Purpose: Count one fewer user of the timer class.
// ORIGINAL: 0x00616890 ?close_class@Time@@QAAXXZ 0x00616890-0x00616897 BYTE_EXACT
// symbol    ?close_class@Time@@SAXXZ
// size      7 bytes
// prototype void (__cdecl ?close_class@Time@@QAAXXZ)()
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// Same as `init_class` above: the body was always here, the claim was not.
Return Value: n/a
Status: Complete
*/
void __cdecl Time::close_class() {
    TimeInitCount = static_cast<int>(static_cast<uint32_t>(TimeInitCount) - 1U);
}
