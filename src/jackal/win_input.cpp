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

// The global message pumps, 0x005FC700..0x005FD36B, including do_sound. They
// form one code run apart from Win's class run (0x005EB350..0x005F8770), and
// flush_input expands flush_keyboard and flush_mouse, so they shared an
// original unit separate from win.cpp. Inferred boundary; the historical
// filename is unknown.
#include "stdafx.h"
#include "win.h"
#include "sound.h"
#include "multidebug.h"
#include "jackal.h"
#include "scroll.h"
#include "net_class.h"  // ActiveNet - do_task routes the 0x402 notification to it

void __cdecl do_all_chars();

// ORIGINAL: 0x005FC700 ?wait_task@@YAXXZ 0x005FC700-0x005FC7E4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005fc700.cpp", "image:terranx_original.exe:0x005FC700", "Pinned5FC74A..757 and5FC83A..847 both poll0xF..0xF. Only high-range402 messages route ActiveNet.handle_sys_msg; null network consumes them. do_all_tasks continues until empty and calls do_net after final MsgStatus=0.", "Coordinator-owned core-pump-secret-owner-20261001 scope; preserved original seeds and historical obligations, no matching/runtime claim from binding"], "reason": "Recover the native message pump using SDK MSG/USER32, existing MsgStatus and ActiveNet storage. Preserve paint-first and low/high queue priority, helper order and dedicated network notification dispatch."}
// TRIED: 260 vs 228 bytes; VC6's /O2 loop rotation duplicates the do_video/check_net/do_net top-of-loop block into a separate call sequence instead of the original's fall-through-then-jmp reuse of one copy. Tried both `for (;;)` and `do {} while(1)` for the poll loop; both rotate the same way.
void __cdecl wait_task() {
    MSG message;
    // Paint dispatch owns a separate native message record.
    MSG paint_message;
    MsgStatus = 0;
    do_video();
    do_net();
    check_net();
    BOOL (WINAPI *peek_message)(LPMSG, HWND, UINT, UINT, UINT) = PeekMessageA;
    BOOL (WINAPI *translate_message)(const MSG *) = TranslateMessage;
    LRESULT (WINAPI *dispatch_message)(const MSG *) = DispatchMessageA;
    MsgStatus = 1;
    do {
        do_video();
        check_net();
        do_net();
    } while (peek_message(&paint_message, 0, WM_PAINT, WM_PAINT, PM_REMOVE) &&
             (translate_message(&paint_message), dispatch_message(&paint_message), MsgStatus = 1, true));
    MsgStatus = 0;
    if (peek_message(&message, 0, 0, WM_USER, PM_REMOVE)) {
        translate_message(&message);
        dispatch_message(&message);
        return;
    }
    if (peek_message(&message, 0, WM_USER, 0xFFFF, PM_REMOVE)) {
        if (message.message == 0x402) {
            if (ActiveNet != 0) {
                ActiveNet->handle_sys_msg();
            }
        } else {
            translate_message(&message);
            dispatch_message(&message);
        }
    }
}

// Native task pumps drain paint messages first, then poll ordinary messages
// before the WM_USER-and-higher range. The network's 0x402 notification is
// consumed by its actual owner instead of being sent to a window procedure.
// ORIGINAL: 0x005FC7F0 ?do_task@@YAHXZ 0x005FC7F0-0x005FC8E8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005fc7f0.cpp", "image:terranx_original.exe:0x005FC7F0", "Pinned5FC74A..757 and5FC83A..847 both poll0xF..0xF. Only high-range402 messages route ActiveNet.handle_sys_msg; null network consumes them. do_all_tasks continues until empty and calls do_net after final MsgStatus=0.", "Coordinator-owned core-pump-secret-owner-20261001 scope; preserved original seeds and historical obligations, no matching/runtime claim from binding"], "reason": "Recover the native message pump using SDK MSG/USER32, existing MsgStatus and ActiveNet storage. Preserve paint-first and low/high queue priority, helper order and dedicated network notification dispatch."}
// TRIED: PeekMessageA/TranslateMessage/DispatchMessageA (loaded as plain function pointers from the IAT slots at 0x669358/0x66935c/ 0x669340, per the emitted globals) read into locals right after the first do_video/do_net/check_net() triple, matching the original's `mov esi,[...]` placement AFTER those three calls rather than before (moving it earlier cost the first 5 instructions). Best reached is 78.3% mnemonic similarity (#5 call vs push): the callee-register-save prologue still spills one extra value this body doesn't, not chased further. `handle_sys_msg` at 0x632db0 is a thiscall `Net` member with `this` read from 0x9be608 right before the call (`mov ecx,[0x9be608]`), not a bare cdecl call.
int __cdecl do_task() {
    MSG message;
    // Paint dispatch owns a separate native message record.
    MSG paint_message;
    MsgStatus = 0;
    do_video();
    do_net();
    check_net();
    BOOL (WINAPI *peek_message)(LPMSG, HWND, UINT, UINT, UINT) = PeekMessageA;
    BOOL (WINAPI *translate_message)(const MSG *) = TranslateMessage;
    LRESULT (WINAPI *dispatch_message)(const MSG *) = DispatchMessageA;
    MsgStatus = 1;
    do {
        do_video();
        check_net();
        do_net();
    } while (peek_message(&paint_message, 0, WM_PAINT, WM_PAINT, PM_REMOVE) &&
             (translate_message(&paint_message), dispatch_message(&paint_message), MsgStatus = 1, true));
    MsgStatus = 0;
    if (peek_message(&message, 0, 0, WM_USER, PM_REMOVE)) {
        translate_message(&message);
        dispatch_message(&message);
        return 1;
    }
    if (peek_message(&message, 0, WM_USER, 0xFFFF, PM_REMOVE)) {
        if (message.message == 0x402) {
            if (ActiveNet != 0) {
                ActiveNet->handle_sys_msg();
            }
        } else {
            translate_message(&message);
            dispatch_message(&message);
        }
        return 1;
    }
    return 0;
}

// ORIGINAL: 0x005FC8F0 ?do_all_tasks@@YAXXZ 0x005FC8F0-0x005FC9DE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005fc8f0.cpp", "image:terranx_original.exe:0x005FC8F0", "Pinned5FC74A..757 and5FC83A..847 both poll0xF..0xF. Only high-range402 messages route ActiveNet.handle_sys_msg; null network consumes them. do_all_tasks continues until empty and calls do_net after final MsgStatus=0.", "Coordinator-owned core-pump-secret-owner-20261001 scope; preserved original seeds and historical obligations, no matching/runtime claim from binding"], "reason": "Recover the native message pump using SDK MSG/USER32, existing MsgStatus and ActiveNet storage. Preserve paint-first and low/high queue priority, helper order and dedicated network notification dispatch."}
// TRIED: PeekMessageA/TranslateMessage/DispatchMessageA are resolved
// TRIED: could not reproduce the original's frameless, purely
void __cdecl do_all_tasks() {
    MSG message;
    // Native paint dispatch has its own MSG, separate from the ordinary queue.
    MSG paint_message;
    for (;;) {
        MsgStatus = 0;
        do_video();
        do_net();
        check_net();
        BOOL (WINAPI *peek_message)(LPMSG, HWND, UINT, UINT, UINT) = PeekMessageA;
        BOOL (WINAPI *translate_message)(const MSG *) = TranslateMessage;
        LRESULT (WINAPI *dispatch_message)(const MSG *) = DispatchMessageA;
        MsgStatus = 1;
        do {
            do_video();
            check_net();
            do_net();
        } while (peek_message(&paint_message, 0, WM_PAINT, WM_PAINT, PM_REMOVE) &&
                 (translate_message(&paint_message), dispatch_message(&paint_message), MsgStatus = 1, true));
        MsgStatus = 0;
        if (peek_message(&message, 0, 0, WM_USER, PM_REMOVE)) {
            translate_message(&message);
            dispatch_message(&message);
            continue;
        }
        if (!peek_message(&message, 0, WM_USER, 0xFFFF, PM_REMOVE)) {
            break;
        }
        if (message.message == 0x402) {
            if (ActiveNet != 0) {
                ActiveNet->handle_sys_msg();
            }
        } else {
            translate_message(&message);
            dispatch_message(&message);
        }
    }
    MsgStatus = 0;
    do_net();
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: zero EAX and return; callee stack cleanup 0 bytes.
// ORIGINAL: 0x005FC9E0 sub_5fc9e0 0x005FC9E0-0x005FC9E3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x005FC9E0-0x005FC9E3 zero EAX and return; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x005FC8F0 ?do_all_tasks@@YAXXZ (src/win.cpp) and 0x005FC9F0 ?do_all_tasks@@YAXPAUWin@@@Z (src/win.cpp)"]}
// symbol    ?sub_5fc9e0@@YAHXZ
// kind      game
int __cdecl sub_5fc9e0() {
    return 0;
}

// Original global Win message-pump family.
// ORIGINAL: 0x005FC9F0 ?do_all_tasks@@YAXPAUWin@@@Z 0x005FC9F0-0x005FCA23 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005FC9F0-0x005FCA23", "git:c1b8fa86:src/recovered/units/005fc9f0.cpp"], "reason": "Clear the real queued-window globals before polling do_net; the independently remeasured seed is retired."}
// symbol    ?do_all_tasks@@YAXPAVWin@@@Z
void __cdecl do_all_tasks(Win *window) {
    MsgStatus = 0;
    if (window) {
        Win *active = WinPointerOwner1 ? WinPointerOwner1 : WinPointerOwner2;
        if (window == active) {
            WinPointerOwner1 = 0;
            WinPointerOwner2 = 0;
        }
    }
    do_net();
}

/*
Purpose: Process non-input related message.
// ORIGINAL: 0x005FCA30 ?do_non_input@@YAXXZ 0x005FCA30-0x005FCB14 BYTE_EXACT
// symbol    ?do_non_input@@YAHXZ
// LEVER: A WRONG PROGRAM, not a wrong spelling. The three PeekMessage guards
//        were chained with `||`: `if (!A || !B || !C) return false;`, which
//        gives up unless ALL THREE ranges have a message waiting. The image
//        jumps AWAY on each success (`test eax, eax; jne 0x5FCAA7`) and only
//        falls through the third into `pop edi; add esp, 0x54; ret`, so it
//        gives up only when NONE of them does - `&&`. 33/84 -> 81/84, and the
//        recovered program now answers the message pump the way the image does.
// LEVER: 0x020A spelled out, because `WM_MOUSELAST` is not one constant.
//        winuser.h defines it 0x020A under WINVER >= 0x0400 and 0x0209
//        otherwise, and this build takes the 0x0209 arm while the image pushes
//        0x020A. Worth the 82nd instruction. `osmx semantic` is what found it -
//        it names the operand pair, "push immediate 0x20a against 0x209",
//        where `listing_diff` cannot: it masks every immediate before
//        comparing, so a wrong constant is invisible there.
// LEVER: init-decl order for strength-reduced loop offsets. VC6 emits the
//        byte-offset initialisations for the min-search loop in DECLARATION
//        ORDER: `int c = 1; int a = 0; for (; c < 3; c++)` puts `mov eax, 0x1c`
//        (c's offset) before `xor esi, esi` (a's offset), matching the image.
//        The old `int a = 0; for (int c = 1; ...)` had them swapped. 82/84 -> 84/84.
// size      228 bytes
// prototype
// callers   12   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCA6E 0x005FCA85 0x005FCA9C 0x005FCAEF 0x005FCAF6 0x005FCB01
Return Value: Is message available? true/false
Status: Complete - testing
*/
BOOL __cdecl do_non_input() {
    do_video();
    check_net();
    do_net();
    MSG msg[3];
    for (int i = 0; i < 3; i++) {
        msg[i].time = 0xFFFFFFFF;
    }
    // `&&`, NOT `||`. This tree had `if (!A || !B || !C) return false;`, which
    // demands a message in ALL THREE ranges; the image returns false only when
    // NONE of them has one - it jumps AWAY on each success and falls through
    // the third test into the epilogue. A different program, not a different
    // spelling. 33/84 -> 81/84 on the fix.
    if (!PeekMessage(&msg[0], NULL, WM_NULL, WM_INPUT, PM_NOREMOVE)
        && !PeekMessage(&msg[1], NULL, WM_UNICHAR, WM_KEYDOWN | WM_INPUT, PM_NOREMOVE)
        // 0x020A, NOT `WM_MOUSELAST`. winuser.h defines that name TWICE - 0x020A
        // when WINVER >= 0x0400, 0x0209 otherwise - and this build takes the
        // 0x0209 arm, while the image pushes 0x020A. `osmx semantic` names the
        // pair outright: "push immediate 0x20a against 0x209".
        && !PeekMessage(&msg[2], NULL, 0x020A, UNICODE_NOCHAR, PM_NOREMOVE)) {
        return false;
    }
    int c = 1;
    int a = 0;
    for (; c < 3; c++) {
        if (msg[c].time < msg[a].time) {
            a = c;
        }
    }
    PeekMessage(&msg[0], msg[a].hwnd, msg[a].message, msg[a].message, PM_REMOVE);
    TranslateMessage(&msg[0]);
    DispatchMessage(&msg[0]);
    return true;
}

/*
Purpose: Process all non-input related messages.
// ORIGINAL: 0x005FCB20 ?do_all_non_input@@YAXXZ 0x005FCB20-0x005FCB5A BYTE_EXACT
// size      58 bytes
// prototype
// callers   27   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FCA30 0x0062D5B0
Return Value: n/a
Status: Complete - testing
*/
// The original source-owned pump body lives at this marker. The failed
// forwarder experiment below was historical: it changed the callee identity
// and is not evidence against an ordinary definition of this function.
// HISTORICAL TRIED: an out-of-line `do_all_non_input_call` forwarder, using
// the `sleep_call`/`base_cost_call` idiom, was measured and reverted in
// path.cpp. Agreement fell from 16/206 to 1/206 and call_diff went from MORE
// to FEWER because a forwarder has a different address from the image's
// 0x005FCB20 target. That result applied to the forwarder, not this body.
void __cdecl do_all_non_input() {
    MsgStatus = 32;
    while (do_non_input()) {
        MsgStatus = 32;
    }
    MsgStatus = 0;
    do_net();
    check_net();
}

/*
Purpose: Process draw related message.
// ORIGINAL: 0x005FCB60 ?do_draw@@YAHXZ 0x005FCB60-0x005FCBAC BYTE_EXACT
// body      src/win.cpp
// size      76 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCB7F 0x005FCB92 0x005FCB9D
Return Value: Is message available? true/false
Status: Complete - testing
*/
// The body is defined below in this translation unit; the local pump expands
// it here, while NetDaemon::send_files has direct calls from another unit.

BOOL __cdecl do_draw() {
    do_video();
    check_net();
    do_net();
    MSG msg;
    if (!PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, WM_CREATE)) {
        return false;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return true;
}

/*
Purpose: Process all draw related messages.
// ORIGINAL: 0x005FCBB0 ?do_all_draws@@YAXXZ 0x005FCBB0-0x005FCC1F BYTE_EXACT
// size      111 bytes
// prototype
// callers   57   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCBEE 0x005FCBF9 0x005FCC00
Return Value: n/a
Status: Complete - testing
*/
void __cdecl do_all_draws() {
    MsgStatus = 1;
    while (do_draw()) {
        MsgStatus = 1;
    }
    MsgStatus = 0;
}

/*
Purpose: Process keyboard related message.
// ORIGINAL: 0x005FCC20 ?do_keyboard@@YAXXZ 0x005FCC20-0x005FCC6D BYTE_EXACT
// body      src/win.cpp
// symbol    ?do_keyboard@@YAHXZ
// size      77 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5D0 0x00636300
// indirect  0x005FCC40 0x005FCC53 0x005FCC5E
Return Value: Is message available? true/false
Status: Complete - testing
*/
// The body is defined below in this translation unit and is expanded by the
// local keyboard pump; no mapped direct caller is known.

BOOL __cdecl do_keyboard() {
    do_video();
    check_net();
    MSG msg;
    if (!PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYLAST, WM_CREATE)) {
        return false;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return true;
}

/*
Purpose: Process all keyboard related messages.
// ORIGINAL: 0x005FCC70 ?do_all_keyboard@@YAXXZ 0x005FCC70-0x005FCCE5 BYTE_EXACT
// size      117 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCCAF 0x005FCCBA 0x005FCCC1
Return Value: n/a
Status: Complete - testing
*/
void __cdecl do_all_keyboard() {
    MsgStatus = 2;
    while (do_keyboard()) {
        MsgStatus = 2;
    }
    MsgStatus = 0;
    do_net();
}

// ORIGINAL: 0x005FCCF0 sub_5fccf0 0x005FCCF0-0x005FCD3D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/unrecovered/005fccf0.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// LEVER: cast the three fixed IAT slots to __stdcall function-pointer types and called through them directly
// symbol    ?pump_key_message@@YAHXZ
int __cdecl pump_key_message() {
    MSG message;
    do_video();
    check_net();
    if (!PeekMessageA(&message, nullptr, WM_KEYDOWN, WM_KEYLAST, PM_REMOVE))
        return 0;
    TranslateMessage(&message);
    DispatchMessageA(&message);
    return 1;
}

// ORIGINAL: 0x005FCD40 sub_5fcd40 0x005FCD40-0x005FCD97 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/005fcd40.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?pump_all_key_messages@@YAXXZ
void __cdecl pump_all_key_messages() {
    MSG message;
    BOOL (WINAPI *translate_message)(const MSG *) = TranslateMessage;
    BOOL (WINAPI *peek_message)(LPMSG, HWND, UINT, UINT, UINT) = PeekMessageA;
    LRESULT (WINAPI *dispatch_message)(const MSG *) = DispatchMessageA;
    do {
        do_video();
        check_net();
    } while (peek_message(&message, nullptr, WM_KEYDOWN, WM_KEYLAST, PM_REMOVE) &&
             (translate_message(&message), dispatch_message(&message), true));
    do_net();
}

// ORIGINAL: 0x005FCDA0 sub_5fcda0 0x005FCDA0-0x005FCDE5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/005fcda0.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?pump_mouse_move_message@@YAHXZ
int __cdecl pump_mouse_move_message() {
    MSG message;
    if (PeekMessageA(&message, nullptr, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
        return 1;
    }
    return 0;
}

// ORIGINAL: 0x005FCDF0 sub_5fcdf0 0x005FCDF0-0x005FCEC0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005fcdf0.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation."}
// TRIED: per-call reinterpret_cast<Proc*>(g) load (5 call sites) instead of caching Peek/Translate/Dispatch ptrs in registers once
// symbol    ?pump_mouse_button_message@@YAHXZ
int __cdecl pump_mouse_button_message() {
    MSG message;
    do_video();
    check_net();
    if (!PeekMessageA(&message, nullptr, WM_MOUSEMOVE, WM_MBUTTONDBLCLK, PM_REMOVE))
        return 0;
    TranslateMessage(&message);
    DispatchMessageA(&message);
    if (message.message == WM_LBUTTONDOWN) {
        if (PeekMessageA(&message, message.hwnd, WM_LBUTTONUP, WM_LBUTTONUP, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
            return 1;
        }
    }
    if (message.message == WM_RBUTTONDOWN) {
        if (PeekMessageA(&message, message.hwnd, WM_RBUTTONUP, WM_RBUTTONUP, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    return 1;
}

// ===== homed from src/unrecovered/005fcec0.cpp =====
// The three USER32 entry points this pump caches, at the signatures the VC6
// headers themselves declare - `void *` for the window handle, which the
// older `PeekMessageFn` above spells, does not accept `&PeekMessageA`.
typedef int(__stdcall *PeekMessageProc)(MSG *, HWND, unsigned int, unsigned int,
                                       unsigned int);
typedef int(__stdcall *TranslateMessageProc)(const MSG *);
typedef LRESULT(__stdcall *DispatchMessageProc)(const MSG *);
// Keep one typed imported-function pointer snapshot per API, in the same
// capture order and location as the original entry-point slot reads.
// ORIGINAL: 0x005FCEC0 sub_5fcec0 0x005FCEC0-0x005FCF8C FILE BYTE_EXACT
// symbol    ?pump_mouse_messages@@YAXXZ
// size      204 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCEF5 0x005FCF04 0x005FCF0B 0x005FCF2D 0x005FCF38 0x005FCF3F 0x005FCF63 0x005FCF72 0x005FCF79
// NAMED from the body: the loop peeks the WM_MOUSEFIRST..WM_MOUSELAST range
// and then, for a WM_LBUTTONDOWN or WM_RBUTTONDOWN, peeks the matching
// WM_LBUTTONUP / WM_RBUTTONUP out of the queue before the next round - the
// mouse-message pump, as `do_char` is the keystroke one at 0x005FCF90.
//
// VOID, not the scaffold's `int` + `return 0`: the image's epilogue sets no
// eax after the do_net call, and an int return would have to zero it.
//
// Historical experiment below used the retired raw-IAT aliases. The current
// source instead takes typed API addresses; load timing needs fresh evidence.
// LEVER: the three USER32 entry points are read out of this module's own
// import table - `*(TranslateMessageProc *) IatTranslateMessage` - which is
// what makes VC6 hoist all three loads into the prologue, before the
// do_video/check_net calls, exactly as the image does. Taking the imported
// names as values (`= &PeekMessageA`) compiles the same instruction but VC6
// then treats it as rematerialisable and sinks each load to its first use.
// Slot addresses from tools/iat_names.py; the operands mask as relocations.
//
// TRIED: switch over msg.message (37/68), `&&`-chained guards (39/68),
// `continue` in the second arm as well (39/68), else-if chain (40/68, and it
// caches msg.message in eax where the image re-reads it). All of them still
// merge the two translate/dispatch tails into one shared copy - 77
// instructions against the image's 68, which emits the pair twice. The
// declaration order is load-bearing instead: reverse of declaration, ebx =
// translate, esi = peek, edi = dispatch, is what puts the prologue loads on
// the image's own registers. 39 of 68, similarity 0.910, and the divergence
// is that merge and nothing else.
void __cdecl pump_mouse_messages() {
    MSG msg;
    // Cache the SDK entry points before the first loop callbacks.
    BOOL (WINAPI *translate)(const MSG *) = TranslateMessage;
    BOOL (WINAPI *peek)(LPMSG, HWND, UINT, UINT, UINT) = PeekMessageA;
    LRESULT (WINAPI *dispatch)(const MSG *) = DispatchMessageA;
    do {
        do_video();
        check_net();
    } while (peek(&msg, 0, 0x200, 0x209, 1) &&
             (translate(&msg), dispatch(&msg),
              (msg.message == 0x201 && peek(&msg, msg.hwnd, 0x202, 0x202, 1))
                  ? (translate(&msg), dispatch(&msg), true)
                  : ((msg.message == 0x204 && peek(&msg, msg.hwnd, 0x205, 0x205, 1))
                         ? (translate(&msg), dispatch(&msg), true) : true)));
    do_net();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FCF90 ?do_char@@YAXXZ 0x005FCF90-0x005FCFDD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fcf90.cpp", "image:terranx_original.exe:0x005FCF90"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
// symbol    ?do_char@@YAHXZ
int __cdecl do_char() {
    do_video();
    check_net();
    MSG msg;
    if (!PeekMessageA(&msg, 0, WM_CHAR, WM_CHAR, PM_REMOVE)) {
        return 0;
    }
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
    return 1;
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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD050 ?do_mouse@@YAXXZ 0x005FD050-0x005FD09D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fd050.cpp", "image:terranx_original.exe:0x005FD050"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
// symbol    ?do_mouse@@YAHXZ
int __cdecl do_mouse() {
    do_video();
    check_net();
    MSG msg;
    // The original range ends at WM_MBUTTONDBLCLK, not newer SDK messages.
    if (!PeekMessageA(&msg, 0, WM_MOUSEMOVE, WM_MBUTTONDBLCLK, PM_REMOVE)) {
        return 0;
    }
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD0A0 ?do_all_mouse@@YAXXZ 0x005FD0A0-0x005FD115 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fd0a0.cpp", "image:terranx_original.exe:0x005FD0A0"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
void __cdecl do_all_mouse() {
    MSG msg;
    MsgStatus = 8;
    int has_message;
    do {
        do_video();
        check_net();
        has_message = PeekMessageA(&msg, 0, WM_MOUSEMOVE, WM_MBUTTONDBLCLK, PM_REMOVE);
        if (has_message) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            MsgStatus = 8;
        }
    } while (has_message);
    MsgStatus = 0;
    do_net();
}

/*
Purpose: Drain the keyboard queue, then the mouse queue, then let the
         network layer poll. Both flushers share this unit and /Ob2 expands
         them here; their peeled PeekMessage loops are the expansions.
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
    flush_keyboard();
    flush_mouse();
    check_net();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD1C0 ?flush_keyboard@@YAXXZ 0x005FD1C0-0x005FD21A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/005fd1c0.cpp", "image:terranx_original.exe:0x005FD1C0"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
void __cdecl flush_keyboard() {
    MSG msg;
    // Retain the original upper bound; WM_KEYLAST varies with SDK version.
    while (PeekMessageA(&msg, 0, WM_KEYFIRST, 0x108, PM_REMOVE)) {
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD220 ?flush_mouse@@YAXXZ 0x005FD220-0x005FD27A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fd220.cpp", "image:terranx_original.exe:0x005FD220"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
void __cdecl flush_mouse() {
    MSG msg;
    while (PeekMessageA(&msg, 0, WM_MOUSEMOVE, WM_MBUTTONDBLCLK, PM_REMOVE)) {
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x005FD280 nullsub_192 0x005FD280-0x005FD281 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x005FD280-0x005FD281 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x005FD220 ?flush_mouse@@YAXXZ (src/win.cpp) and 0x005FD2B0 ?do_sound@@YAHXZ (src/autosound.cpp); win.cpp functions also follow at 0x005FD2C0..0x005FD360"]}
// symbol    ?nullsub_192@@YAXXZ
// kind      game
void __cdecl nullsub_192() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x005FD290 nullsub_193 0x005FD290-0x005FD291 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x005FD290-0x005FD291 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x005FD220 ?flush_mouse@@YAXXZ (src/win.cpp) and 0x005FD2B0 ?do_sound@@YAHXZ (src/autosound.cpp); win.cpp functions also follow at 0x005FD2C0..0x005FD360"]}
// symbol    ?nullsub_193@@YAXXZ
// kind      game
void __cdecl nullsub_193() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x005FD2A0 nullsub_194 0x005FD2A0-0x005FD2A1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x005FD2A0-0x005FD2A1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x005FD220 ?flush_mouse@@YAXXZ (src/win.cpp) and 0x005FD2B0 ?do_sound@@YAHXZ (src/autosound.cpp); win.cpp functions also follow at 0x005FD2C0..0x005FD360"]}
// symbol    ?nullsub_194@@YAXXZ
// kind      game
void __cdecl nullsub_194() {
}

/*
Purpose: Legacy sound processing hook retained as a no-op.
// ORIGINAL: 0x005FD2B0 ?do_sound@@YAHXZ 0x005FD2B0-0x005FD2B3 BYTE_EXACT
// body      src/jackal/autosound.cpp
// CORRECTED from ?do_sound@@YAXXZ
//   body is `xor eax, eax; ret` - a void function would emit `ret` alone
// size      3 bytes
// prototype 
// callers   21   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: zero
Status: Complete
*/
// The pinned caller scan found 36 direct CALL edges to this hook in mapped
// spans. Its original body is the three-byte `xor eax, eax; ret` at 0x5FD2B0.
int __cdecl do_sound() {
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD2C0 sub_5fd2c0 0x005FD2C0-0x005FD308 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005fd2c0.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation."}
// symbol    ?pump_sound_message@@YAHXZ
int __cdecl pump_sound_message() {
    MSG message;
    if (!PeekMessageA(&message, nullptr, WM_USER + 1, WM_USER + 1, PM_REMOVE))
        return 0;
    TranslateMessage(&message);
    DispatchMessageA(&message);
    check_net();
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD310 sub_5fd310 0x005FD310-0x005FD35D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005fd310.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation."}
// symbol    ?pump_all_sound_messages@@YAXXZ
void __cdecl pump_all_sound_messages() {
    MSG message;
    BOOL (WINAPI *translate_message)(const MSG *) = TranslateMessage;
    BOOL (WINAPI *peek_message)(LPMSG, HWND, UINT, UINT, UINT) = PeekMessageA;
    LRESULT (WINAPI *dispatch_message)(const MSG *) = DispatchMessageA;
    do {
    } while (peek_message(&message, nullptr, WM_USER + 1, WM_USER + 1, PM_REMOVE) &&
             (translate_message(&message), dispatch_message(&message), check_net(), true));
}

// Historical catalogue name: this entry only clears the message status.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD360 ?do_all_sounds@@YAXXZ 0x005FD360-0x005FD36B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/005fd360.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation."}
// symbol    ?do_all_sounds@@YAXXZ
void __cdecl do_all_sounds() {
    MsgStatus = 0;
}
