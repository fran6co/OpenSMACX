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
#include "mapwin.h"      // MapWinTable, go_reset's window walk
#include "console.h"     // ConsoleStatusWin
#include "statuswin.h"   // StatusWin::set_title, on the object above
#include "win.h"         // WinPointerOwner3/4
#include "tutwin.h"        // TutWinBaseWindow, TutWinSocWindow, TutorialWindow
#include "game.h"          // ExitTurnLoop, IsMultiplayerNet
#include "map.h"           // WorldClimateSkipTerrainClear
#include "planwin.h"       // PlanWin::on_redraw, MapWinTable
#include "multiwin.h"
#include "councwin.h"      // CouncilWindow, the turn redraw receiver

// The timer callbacks' shared state; declarations in time.h beside the
// callbacks themselves. All four are zero-initialised .data in the image.
int TimerHoldFlag;      // 0x00915620
int Blink2Counter;      // 0x009392B4
int PlanLineCounter;    // 0x00939FD4
int TurnRedrawPending;  // 0x00703DE0

// ORIGINAL: 0x0050EA40 ?blink_timer@@YAXH@Z 0x0050EA40-0x0050EE25 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:371deca46d2a09538c2887fcf5c729de7add0065:src/recovered/units/0050ea40.cpp: full original identity/spans and historical source-form findings", "Independent full seed/native semantic review; original three USER32 IAT targets resolve GetAsyncKeyState/GetFocus/GetWindowLongA with GWL_HINSTANCE.", "Win primary virtual slot12 is on_redraw; PlanWin::blink now uses genuine virtual dispatch, not manual vbase/table indexing.", "recovery/tasks/blink-timer-owner-oct5.json", "recovery/tests/test_blink_timer_actual_source.py"], "reason": "Complete timer callback through existing genuine tutorial/Plan/MainInterface/Console/MapWin owners and USER32 APIs. Preserve original guards, parity/wrap geometry, half-open bounds and cursor update ordering. Source family home is not asserted original TU ownership."}
void __cdecl blink_timer(int a1) {
    (void)a1;
    if (WorldClimateSkipTerrainClear != 0) return;
    if (TimerHoldFlag != 0) return;

    if (TutWinBaseWindow.is_visible() != 0) {
        TutorialWindow.draw_arrow();
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
        TutorialWindow.draw_arrow();
        return;
    }

    PlanWindow.blink();
    MainInterfaceGlobal.string_box_.clip_ids(0x96);

    const int t1 = ConsoleGlobal.field_23BE8_;
    const unsigned char game_state = static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
    const int multiplayer = IsMultiplayerNet;
    if (t1 == 0 && (multiplayer == 0 || (game_state & 2) == 0)) {
        ConsoleGlobal.field_23BFC_ = 0;
        return;
    }
    if (ExitTurnLoop != 0) {
        ConsoleGlobal.field_23BFC_ = 0;
        return;
    }
    if (ConsoleGlobal.field_23BE4_ == 0
        && (multiplayer == 0 || (game_state & 2) == 0)) {
        return;
    }

    ConsoleGlobal.field_23BF8_ = ConsoleGlobal.field_23BF8_ == 0 ? 1 : 0;
    draw_cursor();

    const int blink_col = (GetAsyncKeyState(1) >> 8) & 0xFF;
    if (reinterpret_cast<HINSTANCE>(GetWindowLongA(GetFocus(), GWL_HINSTANCE))
        != WinInstance) {
        return;
    }
    if (TutorialWindow.is_visible() != 0) return;
    if (ConsoleGlobal.field_23D80_ == -1) return;
    if (ConsoleGlobal.field_23D84_ == -1) return;
    if (ConsoleGlobal.field_23D88_ == -1) return;
    if (ConsoleGlobal.field_23D8C_ == -1) return;

    const unsigned char preferences_high =
        static_cast<unsigned char>(GameDataStateStorage.fields_.game_preferences_ >> 8);
    if ((preferences_high & 0x10) == 0 && blink_col == 0
        && WinModalDepth == 0) {
        return;
    }

    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == 0) continue;
        int col = blink_col;

        if (slot != 0 && window->field_1DD74_ == 0) continue;
        if (col != 0 && window->field_1DD80_ == 0) continue;
        if (window->field_1DDD4_ + window->field_1DDCC_
            >= MapLongitudeBounds) {
            continue;
        }

        int row = static_cast<int32_t>(window->field_1DDA0_);
        col = static_cast<int32_t>(window->field_1DD9C_);

        int mouse_x;
        int mouse_y;
        window->get_mouse_pos(&mouse_x, &mouse_y);
        window->client_to_screen(&mouse_x, &mouse_y);

        const unsigned char flat = static_cast<unsigned char>(MapIsFlat);
        if (mouse_x == 0) {
            col -= (window->field_1DDCC_ + 2) / 3;
            if ((flat & 1) == 0) {
                if (col < 0) {
                    col += MapLongitudeBounds;
                } else {
                    const int width = MapLongitudeBounds;
                    if (col >= width) col -= width;
                }
            }
        }

        if (mouse_x >= WinScreenWidth - 0xA
            && mouse_x < WinScreenWidth + 0xA) {
            int next_col = (window->field_1DDCC_ + 2) / 3 + col;
            if ((flat & 1) != 0) {
                col = next_col;
            } else if (next_col < 0) {
                col = MapLongitudeBounds + next_col;
            } else {
                const int width = MapLongitudeBounds;
                if (next_col >= width) next_col -= width;
                col = next_col;
            }
        }

        if (mouse_y == 0 && window->field_1DDA8_ > -2) {
            const int max_shift = window->field_1DDA8_ + 2;
            const int requested = (window->field_1DDD0_ + 2) / 3;
            int delta;
            if (requested < 0 || max_shift < 0) {
                delta = 0;
            } else if (requested <= max_shift) {
                delta = requested;
            } else {
                delta = max_shift;
            }
            row -= delta;
        }

        if (mouse_y >= WinScreenHeight - 0xA
            && mouse_y < WinScreenHeight + 0xA) {
            const int limit = MapLatitudeBounds + 2;
            const int count = static_cast<int32_t>(window->field_1DDD8_);
            const int start = window->field_1DDA8_;
            if (start + count * 2 - 2 < limit) {
                const int max_shift = (2 - count) * 2 - start
                                    + MapLatitudeBounds;
                const int requested = (window->field_1DDD0_ + 2) / 3;
                int delta;
                if (requested < 0 || max_shift < 0) {
                    delta = 0;
                } else if (requested > max_shift) {
                    delta = max_shift;
                } else {
                    delta = requested;
                }
                row += delta;
            }
        }

        if ((col & 1) != 0) col -= 1;
        if ((row & 1) != 0) col += 1;

        if (col == static_cast<int32_t>(window->field_1DD9C_)
            && row == static_cast<int32_t>(window->field_1DDA0_)) {
            continue;
        }
        if (row < 0) continue;
        if (row >= MapLatitudeBounds) continue;
        if (col < 0) continue;
        if (col >= MapLongitudeBounds) continue;

        if (window == static_cast<MapWin *>(&ConsoleGlobal)) {
            window->field_21A44_ = 1;
        }
        window->set_center(col, row, 1);
        window->field_21A44_ = 0;
    }
}

// PlanWindow's virtual GraphicWin base begins at the timer's historical
// receiver 0x00856DC0. The real PlanWin object owns that base and lifetime.
// The MultiWin the turn timer redraws (`mov ecx, 0x7fd648`) is the REAL
// object MultiWindow in game.cpp; game.cpp closes its GraphicWin front for
// desktop_close - the same fixed address, one object.


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
// body      src/time.cpp
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

// global

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
// MEASURED 2026-09-03, after the four start/pulse ladders landed: now
//   9/137 (0.864) at /O2. Re-derived the ceiling from scratch and it holds,
//   with one new fact - the image DOES inline other helpers into this body
//   (`go_cover_window()` folds to `mov ecx, 0x937118` at 0x0050EF57, and
//   `TutWinBaseWindow.is_visible()` expands inline in every caller), so
//   VC6's refusal here is specific to `Time::close`, not a general
//   no-inlining TU. The shape that explains all four sites - close()
//   inlined into ~Time, called out-of-line from stop_timers AND from here -
//   is that go_reset's original .cpp never saw close()'s body. This tree
//   merged the two units into src/time.cpp, so the body is always visible
//   and VC6 always expands it (+34 instructions, 155 against 137).
// TRIED: `#pragma inline_depth(0)` ahead of go_reset - it suppressed the
//   close() expansion all right but is a per-function switch, so it also
//   un-inlined go_cover_window() (the folded `mov ecx, 0x937118` became a
//   real call pair) and every is_visible() expansion, landing at 3/137.
//   Not a per-site lever, and not source. Reverted.
// TRIED: making close() an out-of-line member defined below go_reset in
//   this file - would hide the body from go_reset while leaving it visible
//   to ~Time, but it also hides it from console.cpp:875 and netmsg.cpp:77,
//   which are not mine to regress, and it does nothing for the prologue
//   below. Not taken.
//   The second, independent wall is the prologue: the image opens
//   `push ebp / mov ebp, esp / push ecx` (catalogue flag `frame`) and this
//   build's /O2 omits the frame pointer, so instructions 0..1 differ before
//   anything else. No flag set in the axis reaches both - the /Oy- sets fix
//   the prologue and lose the body (17/137 best).
Return Value: n/a
Status: Complete
*/
void __cdecl go_reset() {
    CoverWindow.Win::hide();
    GoTimer.close();

    for (int index = 0; index < static_cast<int>(MapWinTableSlots); ++index) {
        MapWin **const slot = &MapWinTable[index];
        MapWin *const window = *slot;
        if (window != nullptr &&
            (index == 0 || window->field_1DD74_ != 0) &&
            index != MapWinSelectedSlot &&
            window->field_1DD80_ != 0) {
            // The FIRST store runs off the window the tests loaded; the rest
            // re-read the slot each time, as the image re-reads [esi] for
            // every store after the first (0x0050EFB6 on).
            window->field_1DD88_ = 1;
            (*slot)->field_1DD80_ = 0;
            (*slot)->field_1DD8C_ = 0;
            (*slot)->field_1DD84_ = 0;
            (*slot)->field_1DE0C_ = -1;

            // Capture-loss dispatch may replace the slot. Owner4 is still
            // compared with the old base, but the next dispatch reloads it
            // (50EFF9 versus 50F007 in the original).
            Win *const base = static_cast<GraphicWin *>(*slot);
            if (WinPointerOwner3 == base) {
                WinPointerOwner3 = nullptr;
                base->on_lose_mouse_capture();
            }
            if (WinPointerOwner4 == base) {
                WinPointerOwner4 = nullptr;
            }
            (*slot)->on_update();
            ConsoleStatusWin.set_title(0);
        }
    }

    // Slot 0's cursor first, unconditionally.
    MapWin *const primary = MapWinTable[0];
    if (primary->cursor_sprite_ != 0 || primary->cursor_resource_id_ != 0x7F00) {
        primary->GraphicWin::set_cursor(0x7F00);
        primary->cursor_resource_id_ = 0x7F00;
        primary->cursor_sprite_ = 0;
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
        Sprite *const cursor_sprite = window->cursor_sprite_;
        if ((cursor_sprite == &g_CURSOR_SPRITES[1] ||
             cursor_sprite == &g_CURSOR_SPRITES[9]) &&
            (window->cursor_sprite_ != 0 || window->cursor_resource_id_ != 0x7F00)) {
            window->GraphicWin::set_cursor(0x7F00);
            window->cursor_resource_id_ = 0x7F00;
            window->cursor_sprite_ = 0;
        }
        (*slot)->field_1DD88_ = 1;
        (*slot)->field_1DD80_ = 0;
        (*slot)->field_1DD8C_ = 0;
        (*slot)->field_1DD84_ = 0;
        (*slot)->field_1DE0C_ = -1;

        Win *const base = static_cast<GraphicWin *>(*slot);
        if (WinPointerOwner3 == base) {
            WinPointerOwner3 = nullptr;
            base->on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == base) {
            WinPointerOwner4 = nullptr;
        }
        (*slot)->on_update();
        ConsoleStatusWin.set_title(0);
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
    if (TutWinBaseWindow.is_visible() != 0) {
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
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
    MainInterfaceGlobal.redraw_complete();
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
// The pinned loop bounds 0x007D3C3C..0x007D3C5C and four-byte stride
//   identify eight ordered slots. A signed index preserves the live table
//   walk without converting its slot pointers to integers.
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
    if (TutWinBaseWindow.is_visible() != 0) {
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
        return;
    }
    --PlanLineCounter;
    // The image makes this a DIRECT call (0x50EEBF -> 0x48AF30
    // ?on_redraw@PlanWin@@QAEXXZ), not a slot-12 dispatch; the qualified
    // spelling suppresses the virtual call the shared name now implies.
    PlanWindow.PlanWin::on_redraw();

    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == nullptr) {
            continue;
        }
        if (slot != 0) {
            const uint32_t active = window->field_1DD74_;
            if (active == 0) {
                continue;
            }
        }
        const uint32_t draw_flags = window->field_1DD70_;
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
//   its own jump to the shared `ret`. MultiWindow.draw(1) is the
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
                CouncilWindow.GraphicWin::redraw();
            }
            MultiWindow.draw(1);
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
// LEVER (2026-08-29, timer conversion): the six timers are real `Time`
// objects (time.h/time.cpp) instead of `Time *const` bindings into their
// image addresses - the image's own receivers are the objects' ADDRESSES
// (`mov ecx, 0x915688` here), which `BlinkTimer.start(...)` on a real
// object reproduces; the claim re-measured BYTE_EXACT after, as did
// stop_timers, the three timer callbacks and game.cpp's inlined stop
// expansions.
Return Value: n/a
Status: Complete
*/
void __cdecl start_timers() {
    BlinkTimer.start(blink_timer, 1, 150, 150);
    Blink2Timer.start(blink2_timer, 2, 100, 100);
    LineTimer.start(line_timer, 3, 100, 100);
    TurnTimer.start(turn_timer, 4, 500, 500);
}

/*
Purpose: Stop global timers.
// ORIGINAL: 0x0050F440 ?stop_timers@@YAXXZ 0x0050F440-0x0050F45E BYTE_EXACT
// body      src/time.h
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
// Historical experiment below: stop_timers now calls Time::close directly;
// its implementation is hidden in the separate timer lifecycle TU.
// LEVER: the `time_close_call` forwarder (declared in time.h, defined in
//   flatbutton.cpp). 0 of 6 agreeing, 90 instructions against the image's 6,
//   because close()'s in-class body was expanded at all three sites - the
//   image emits three `mov ecx, <addr>` + `call Time::close`, the last a tail
//   `jmp`. The body is in a header, so it is the same tokens in time.cpp and
//   in game.cpp, and the image wants the two OPPOSITE things from those two
//   units: `call` here, `expand` in system_init 0x58F040 and game_reload
//   0x58F450. No declaration of close() can give both - moving close()'s
//   definition out of the class into this .cpp (TRIED, see 0x00616780) left
//   this unit still able to see it and changed nothing here. A forwarder
//   whose body lives in a THIRD unit breaks the symmetry: this COMDAT emits
//   the real `call`, and the two game.cpp sites are unchanged by it (35 of
//   135 and 19 of 44 agreeing, before and after). 6 of 6, BYTE_EXACT.


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

// These are behavior views over observed fields, not names or complete
// allocation claims for either original class. No connection between their
// construction/use/lifetime is inferred from the nearby addresses.
class RangePrefixInitializationView {
public:
    RangePrefixInitializationView();
    unsigned int unknown_dword_0_;
    unsigned int zeroed_dword_4_;
    unsigned int unknown_dword_8_;
    unsigned int unknown_dword_C_;
};

// RECOVERY_ROLE: SCAFFOLD Provisional four-word initialization view; original containing class, complete extent and constructor-to-instance lifetime flow remain unproved. Matching the endpoint views' offsets is not a receiver ownership link.
// ORIGINAL: 0x004C8110 sub_4c8110 0x004C8110-0x004C813B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8110-0x004C813B: complete thiscall constructor first zeroes four bytes [this+4] through memset, then stores dword [this], dword [this+8], and dword [this+0xC] in this order; [this+4] is zeroed by the actual four-byte memset call at 0x004C811B to 0x006465F0", "git:aee589a5dd0fca997773070668af4e1ff9026684:src/recovered/004c8110.cpp: historical-only seed hint // symbol ??0LegacyBlock@@QAE@XZ; this deleted seed-file spelling is not emitted as the current symbol and is not independently established as the original class", "pinned scan: no direct caller edge or absolute-address data reference to 4C8110 was found"], "reason": "Cover only the constructor's concrete four-dword initialization through a typed source view. Preserve its original span and claim; keep field roles, complete original class name, allocation extent and runtime lifetime unknown. The view is not merged with the separate time-range receiver."}
// symbol    ??0RangePrefixInitializationView@@QAE@XZ
RangePrefixInitializationView::RangePrefixInitializationView() {
    memset(&zeroed_dword_4_, 0, sizeof(zeroed_dword_4_));
    unknown_dword_0_ = 0;
    unknown_dword_8_ = 0;
    unknown_dword_C_ = 0;
}

class TimeRangeReceiverView {
public:
    // Endpoint operations and sample share only the observed fields;
    // original class and lifetime ownership are not established.
    void set_lower_endpoint(int value);
    void set_upper_endpoint(int value);
    int sample();
private:
    unsigned char unobserved_prefix_0_[4];
    unsigned char enabled_flags_4_;
    unsigned char unobserved_bytes_5_7_[3];
    int lower_bound_8_;
    int upper_bound_C_;
};


// The paired operations update the two bounds in their original order. Their
// unsigned comparisons are pinned behavior, not a claim about endpoint meaning.
/*
 * RECOVERY_HISTORY: v1 address=0x004C80C0 source_sha256=e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee envelope_sha256=e0d6eb0a4d32e64334cc1f4cded43e7d701b510f32d7a1a14fb4d75f7ad5a11f
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Clamp field 0xc against an argument, then store unconditionally.
 * [ARCHIVE]          Emitted by tools/generate_field_accessors.py from
 * [ARCHIVE]
 * [ARCHIVE]              push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 0xc] / cmp edx, eax / jae 0x4c80d0 / mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 8], eax / pop ebp / ret 4
 * [ARCHIVE]
 * [ARCHIVE]          The name is the ADDRESS, not an invention, and `self` is a void
 * [ARCHIVE]          pointer because this body needs `this` to be nothing more than a
 * [ARCHIVE]          pointer and an offset - the class it belongs to is not established.
 * [ARCHIVE]          The adapter declares 1 stack dword(s) so it
 * [ARCHIVE]          cleans 4 bytes, taken from the body's own `ret` -
 * [ARCHIVE]          the only statement of this function's arity that exists, since it has
 * [ARCHIVE]          no mangled name. Declaring fewer would leave them on the caller's
 * [ARCHIVE]          stack.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
 * [ARCHIVE] // original marker: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 BYTE_EXACT
 * [ARCHIVE] // symbol    ?field_accessor_004c80c0_redirect@@YIXPAX0H@Z
 * [ARCHIVE] // size      23 bytes
 * [ARCHIVE] // prototype
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C80C0-0x004C80D7: ECX receiver, one stack dword, unsigned endpoint compare and ordered stores at +8/+C", "source:src/time.cpp mapped 0x004C8350 reads the same +8/+C signed bounds and byte +4 flag through TimeRangeReceiverView", "pinned-analysis limitation: no caller, constructor, allocation extent, or runtime lifetime witness is available for these endpoints"], "reason": "Place the paired endpoint operation in the same provisional observed-field view as 0x004C8350 because the pinned endpoint stores and mapped sample reads address the same +8/+C fields, while the sample also reads the +4 flag. This supports shared field API closure, not original complete class identity, object extent, constructor, caller set, or lifetime. Preserve the pinned unsigned comparisons and store order."}
// symbol    ?set_lower_endpoint@TimeRangeReceiverView@@QAEXH@Z
// kind      game
void TimeRangeReceiverView::set_lower_endpoint(int value) {
    if (static_cast<DWORD>(upper_bound_C_) < static_cast<DWORD>(value)) {
        upper_bound_C_ = value;
    }
    lower_bound_8_ = value;
}

/*
 * RECOVERY_HISTORY: v1 address=0x004C80E0 source_sha256=e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee envelope_sha256=8c15061494d8d9b690e02e41f5aa7054bafa4a713979ab8f6841a65deb438b95
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Clamp field 0x8 against an argument, then store unconditionally.
 * [ARCHIVE]          Emitted by tools/generate_field_accessors.py from
 * [ARCHIVE]
 * [ARCHIVE]              push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 8] / cmp edx, eax / jbe 0x4c80f0 / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax / pop ebp / ret 4
 * [ARCHIVE]
 * [ARCHIVE]          The name is the ADDRESS, not an invention, and `self` is a void
 * [ARCHIVE]          pointer because this body needs `this` to be nothing more than a
 * [ARCHIVE]          pointer and an offset - the class it belongs to is not established.
 * [ARCHIVE]          The adapter declares 1 stack dword(s) so it
 * [ARCHIVE]          cleans 4 bytes, taken from the body's own `ret` -
 * [ARCHIVE]          the only statement of this function's arity that exists, since it has
 * [ARCHIVE]          no mangled name. Declaring fewer would leave them on the caller's
 * [ARCHIVE]          stack.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
 * [ARCHIVE] // original marker: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 BYTE_EXACT
 * [ARCHIVE] // symbol    ?field_accessor_004c80e0_redirect@@YIXPAX0H@Z
 * [ARCHIVE] // size      23 bytes
 * [ARCHIVE] // prototype
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C80E0-0x004C80F7: ECX receiver, one stack dword, unsigned endpoint compare and ordered stores at +8/+C", "source:src/time.cpp mapped 0x004C8350 reads the same +8/+C signed bounds and byte +4 flag through TimeRangeReceiverView", "pinned-analysis limitation: no caller, constructor, allocation extent, or runtime lifetime witness is available for these endpoints"], "reason": "Place the paired endpoint operation in the same provisional observed-field view as 0x004C8350 because the pinned endpoint stores and mapped sample reads address the same +8/+C fields, while the sample also reads the +4 flag. This supports shared field API closure, not original complete class identity, object extent, constructor, caller set, or lifetime. Preserve the pinned unsigned comparisons and store order."}
// symbol    ?set_upper_endpoint@TimeRangeReceiverView@@QAEXH@Z
// kind      game
void TimeRangeReceiverView::set_upper_endpoint(int value) {
    if (static_cast<DWORD>(lower_bound_8_) > static_cast<DWORD>(value)) {
        lower_bound_8_ = value;
    }
    upper_bound_C_ = value;
}

// Historical TRIED: the seed candidate calculated range before the clock call;
// the compiler scheduled the call earlier, retaining both endpoints and using
// another callee-saved register. A volatile range instead spilled through a
// frame. Those were compiler-shape findings, not evidence for a source class.
// The original adds the unsigned remainder to the reloaded upper endpoint.
// memcpy retains x86 32-bit wrap without C++ signed-overflow undefined behavior.
static int range_view_wrapped_upper_add(int upper, DWORD remainder) {
    const DWORD bits = static_cast<DWORD>(upper) + remainder;
    int result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

// Historical seed called this an unknown Subject::roll and kept an IAT alias;
// that is not pinned class/API evidence. The actual source uses the SDK clock.
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C8350 sub_4c8350 0x004C8350-0x004C8396 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8350-0x004C8396: thiscall reads byte[this+4]&1; signed-compares dwords+8/+C; forms a wrapped unsigned inclusive difference before timeGetTime; divides unsigned; reloads +C after the call and adds the remainder", "pinned calls at 0x004C836D and 0x004C8384 target PE import slot 0x00669368, which uniquely resolves to WINMM timeGetTime; the normal SDK API is used, not an executable-address alias", "promoted historical seed src/recovered/units/004c8350.cpp retained the complete70-byte span and MISMATCH state; prior range-before-clock source-shape observation is preserved here for later restoration", "pinned scan: no direct caller edge or absolute-address data reference to 4C8350 was found", "source:src/time.cpp provisional endpoint member operations at 0x004C80C0/0x004C80E0 update the same +8/+C fields read by sample"], "reason": "Implement the observed time-based range read through a provisional receiver view, paired with 0x004C80C0/0x004C80E0 because those endpoint operations write the same +8/+C fields this body reads. Keep the separate RangePrefixInitializationView at 0x004C8110 unmerged. Preserve signed branch comparison, unsigned 32-bit range wrap, zero-divisor failure, the post-clock upper-field reload, and wrapped return addition. This relation does not prove original complete class identity, object extent, constructor, callers, or lifetime."}
// symbol    ?sample@TimeRangeReceiverView@@QAEHXZ
int TimeRangeReceiverView::sample() {
    if ((enabled_flags_4_ & 1) == 0) {
        return 0;
    }

    const int lower = lower_bound_8_;
    const int upper = upper_bound_C_;
    if (lower > upper) {
        const DWORD range = static_cast<DWORD>(lower)
                          - static_cast<DWORD>(upper) + 1u;
        const DWORD tick = timeGetTime();
        const DWORD remainder = tick % range;
        return range_view_wrapped_upper_add(upper_bound_C_, remainder);
    }

    const DWORD range = static_cast<DWORD>(upper)
                      - static_cast<DWORD>(lower) + 1u;
    const DWORD tick = timeGetTime();
    const DWORD remainder = tick % range;
    return range_view_wrapped_upper_add(upper_bound_C_, remainder);
}

// Pump redraws before non-input work at least once. The original captures the
// WINMM timeGetTime import once and compares unsigned elapsed time, including
// wraparound and negative arguments interpreted as unsigned durations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050DD90 ?clock_wait@@YAXH@Z 0x0050DD90-0x0050DDBA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/0050dd90.cpp retained exact seed and original extent; preserved without copying historical claim", "Pinned PE import669368 resolves WINMM.dll timeGetTime; full disassembly50DD90..50DDBA captures import once, calls5FCBB0 then5FCA30 and uses unsigned jb elapsed compare"], "reason": "Home cooperative unsigned elapsed-time wait to timer integration; preserve import capture, redraw/non-input ordering, mandatory first iteration and modular DWORD subtraction, without fixed import address."}
void __cdecl clock_wait(int milliseconds) {
    DWORD (WINAPI *const read_clock)() = timeGetTime;
    const DWORD started = read_clock();
    do {
        do_all_draws();
        do_non_input();
    } while (read_clock() - started < static_cast<DWORD>(milliseconds));
}
