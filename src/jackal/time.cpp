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

// Timer SDK dependencies; original import slots are derived from the pinned PE.
// RECOVERY_IMPORT: v1 __imp__SetTimer@16 USER32.dll name:SetTimer
// RECOVERY_IMPORT: v1 __imp__timeSetEvent@20 WINMM.dll name:timeSetEvent

// Original 0x00616860 stores this; 0x00616870 clears it; callbacks load the Time* cell, not its pointee.
// RECOVERY_OBJECT: v1 0x009B90D0 ?TimeModal@Time@@2PAV1@A
Time *Time::TimeModal;
// Original 0x00616880/0x00616886 increments this class user count; 0x00616890 decrements; start only.
// RECOVERY_OBJECT: v1 0x009B90CC ?TimeInitCount@Time@@2HA
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
    stop_pending_events();
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
// symbol    ?init@Time@@QAEXP6AXHPAX@ZH0II@Z
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
void Time::init(void(__cdecl *callback)(int, void *), int param, void *param2, uint32_t cnt,
                uint32_t res) {
    // IN THE IMAGE'S STORE ORDER at 0x0061631A, which is not the one-argument
    // overload's: the two callback fields come first here.
    stop_pending_events();
    callback2_ = callback;
    cb_param2_ = param2;
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
// ORIGINAL: 0x00616350 ?start@Time@@QAEHP6AXH@ZHHH@Z 0x00616350-0x00616405 BYTE_EXACT
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
//        0x00616260.
// LEVER: this copy's store ladder is NOT init()'s. init() keeps
//        callback1_ first; here the image emits oneshot_state_/
//        tick_posted_/unk_2_ FIRST and callback1_ fourth
//        (0x006163A3..0x006163AB), which is what lets VC6 defer the
//        callback argument's register past the `cmp ecx, ebx` that tests
//        it at 0x006163A1. Writing the three zero stores before the
//        callback store took this 65/69 MNEMONIC_ONLY -> 69/69 BYTE_EXACT
//        in one move.
// TRIED: cb_param1_ first, as the image's own first store (0x0061639A)
//        reads - MISMATCH 57/69, so the image's emitted order is its
//        scheduler's, not the source's; the zeros-first spelling is the
//        one it reproduces from.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::start(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res) {
    // init() (0x00616260, BYTE_EXACT as its own out-of-line function) is
    // hand-inlined here: the image writes its whole body out at this call
    // site (init() itself inlines the cancellation core, whose flush_timer() call is the
    // only one that survives), rather than calling 0x00616260. The store
    // ladder is this copy's own - see the LEVER above.
    stop_pending_events();
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback1_ = callback;
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
// ORIGINAL: 0x00616410 ?start@Time@@QAEXP6AXHH@ZHHHH@Z 0x00616410-0x006164CA BYTE_EXACT
// symbol    ?start@Time@@QAEIP6AXHPAX@ZH0II@Z
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
// LEVER: same store ladder as that sibling - oneshot_state_/tick_posted_/
//        unk_2_ FIRST, then callback1_, then the callback store, then the
//        two cb_param fields. The image's own emitted order here is
//        cb_param2_, cb_param1_, the three zeros, callback1_, callback2_
//        (0x0061645A..0x00616475) - its scheduler hoists the two argument
//        stores, and the spelling it reproduces that from is the
//        zeros-first one, not the emitted one. 65/70 MNEMONIC_ONLY ->
//        70/70 BYTE_EXACT.
// TRIED: cb_param2_/cb_param1_ first, matching the emitted order literally
//        - MISMATCH 58/70: the ladder is right but VC6 loads `cnt` instead
//        of `callback` into its third preload register. The source has to
//        keep the callback store AFTER callback1_; the rest of the ladder
//        follows init()'s.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::start(void(__cdecl *callback)(int, void *), int param, void *param2, uint32_t cnt,
                     uint32_t res) {
    // init() (the two-parameter-callback overload, BYTE_EXACT as its own
    // out-of-line function) is hand-inlined here: the image writes its whole
    // body out at this call site (init() itself inlines the cancellation core, whose
    // flush_timer() call is the only one that survives), rather than calling
    // it as init(callback, param, param2, cnt, res).
    stop_pending_events();
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback1_ = 0;
    callback2_ = callback;
    cb_param2_ = param2;
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
// ORIGINAL: 0x006164D0 ?pulse@Time@@QAEHP6AXH@ZHHH@Z 0x006164D0-0x00616585 BYTE_EXACT
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
//        init() hand-inlined here.
// LEVER: the same store ladder as that sibling - oneshot_state_/tick_posted_/
//        unk_2_ FIRST, callback1_ fourth - is what took this 63/69
//        MNEMONIC_ONLY to 69/69 BYTE_EXACT. The image's emitted order here
//        (0x0061651A..0x0061652B) is cb_param1_, the three zeros, then
//        callback1_; the spelling VC6 produces that from is zeros-first,
//        and moving cb_param1_ to the front as the emitted order reads
//        measures WORSE (see the `start` sibling's TRIED).
// TRIED: TIME_ONESHOT in the multimedia arm, which the upstream OpenSMACX
//        "bug fix" comment asks for - 68/69, the one differing instruction
//        being the image's `push 1` at 0x0061654E. TIME_PERIODIC is what
//        shipped; see the BUG IN THE ORIGINAL note in the body.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::pulse(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res) {
    // init() (0x00616260, BYTE_EXACT as its own out-of-line function) is
    // hand-inlined here: the image writes its whole body out at this call
    // site (init() itself inlines the cancellation core, whose flush_timer() call is the
    // only one that survives), rather than calling 0x00616260.
    stop_pending_events();
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback1_ = callback;
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
        // BUG IN THE ORIGINAL: fuEvent is TIME_PERIODIC here, copy-pasted
        // from start() - the image pushes 1 at 0x0061654E, not 0. The
        // two-parameter pulse below and the parameterless pulse both push
        // TIME_ONESHOT, so this one really is the odd one out and the
        // multimedia path of this overload is periodic like start()'s.
        // Reproduced deliberately: TIME_ONESHOT (0) is a different program,
        // and the upstream OpenSMACX "bug fix" comment that changed it is
        // not what shipped.
        id_event_ = timeSetEvent(count_, resolution_, (LPTIMECALLBACK)MultimediaProc,
            (UINT_PTR)this, TIME_PERIODIC);
    } else {
        id_event_ = SetTimer(HandleMain, (UINT_PTR)this, count_, (TIMERPROC)TimerProc);
    }
    return id_event_ ? 0 : 2;
}

/*
Purpose: Start a pulse instance of the class with a two parameter callback.
// ORIGINAL: 0x00616590 ?pulse@Time@@QAEXP6AXHH@ZHHHH@Z 0x00616590-0x0061664D BYTE_EXACT
// symbol    ?pulse@Time@@QAEIP6AXHPAX@ZH0II@Z
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
// LEVER: and the same store ladder as that overload - oneshot_state_/
//        tick_posted_/unk_2_ first, then callback1_, THEN the callback
//        store, then cb_param2_/cb_param1_. Both two-parameter bodies
//        wanted the callback store in fifth position and refused it
//        anywhere earlier (58/70 from every spelling that hoisted it); the
//        parameterless-ladder sibling is the one that named the shape.
// TRIED: cb_param2_/cb_param1_ hoisted to the front to read like the
//        emitted order - MISMATCH 58/70, the third preload register going
//        to `cnt` instead of `callback`. Same wall as the `start`
//        two-parameter sibling; the zeros-first ladder is the only spelling
//        of the two that reproduces both.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
uint32_t Time::pulse(void(__cdecl *callback)(int, void *), int param, void *param2, uint32_t cnt,
                     uint32_t res) {
    // init() (the two-parameter-callback overload, BYTE_EXACT as its own
    // out-of-line function) is hand-inlined here: the image writes its whole
    // body out at this call site, rather than calling it directly.
    stop_pending_events();
    oneshot_state_ = 0;
    tick_posted_ = 0;
    unk_2_ = 0;
    callback1_ = 0;
    callback2_ = callback;
    cb_param2_ = param2;
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
// body      src/jackal/time.cpp
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

void Time::stop() {
    stop_pending_events();
}


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
// symbol    ?release_modal@Time@@SAXXZ
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
// Pinned 0x00616870 reads no receiver and only clears TimeModal. Credits and
// Replay call it after virtual set_modal; both DesignWin paths call it after
// stop, which may clobber ECX. The static owner API models that observed ABI;
// original static spelling versus an optimized unused receiver remains unknown.
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
    TimeInitCount = TimeInitCount + 1;
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

