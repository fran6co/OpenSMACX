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

/*
Purpose: The window class's WM_SYSKEYDOWN/WM_SYSCHAR router. It resolves
         which window is active - the modal focus if one is set, else the
         focus window, else the input focus, else the default - records that
         in WinActiveWindow, and hands the key to that window's owner
         through vtable slot 80. HOMED from src/recovered/units/005f16d0.cpp
         on 2026-08-25.

         The catalogued name spells a `Win` receiver (`QAA`) this body does
         not have: `a1` is the HWND the window procedure was called with and
         is never read. The `// symbol` fact records the free-function form
         the build emits.
*/
// ORIGINAL: 0x005F16D0 ?OnSysKey@Win@@QAAXPAXIJHI@Z 0x005F16D0-0x005F1745 FILE BYTE_EXACT
// symbol    ?OnSysKey@@YAXPAXIJHI@Z
// size      118 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
void __cdecl OnSysKey(void *hwnd, unsigned int key, long flags, int repeat,
                      unsigned int scan) {
    Win *active;
    if (WinFocusWindow != nullptr) {
        active = WinModalFocus;
        if (active == 0) {
            active = WinFocusWindow;
        }
    } else {
        active = WinInputFocus;
        if (active == 0) {
            active = WinDefaultFocus;
        }
    }
    WinActiveWindow = active;
    if (active != 0) {
        // 0xA8 is poWinBase_, which win.h already documents as the window that
// owns a position - a third witness after Win::construct storing `this`
// there and Win::get_mouse_window returning it.
        Win *owner = active->poWinBase_;
        if (owner != 0) {
            if (owner->iSomeFlag_ & 1) {
                Win *inner = owner->win_parent_;
                if (inner != 0) {
                    if (!inner->is_visible()) {
                        return;
                    }
                }
                owner->on_sys_key(key, flags, repeat, scan);
            }
        }
    }
}

// ===== homed from src/recovered/005f1660.cpp =====

// ORIGINAL: 0x005F1660 ?OnChar@Win@@QAAXPAXDH@Z 0x005F1660-0x005F16CB BYTE_EXACT
// symbol    ?OnChar@@YAXPAXDH@Z
// symbol    ?OnChar@Win@@QAAXPAXHH@Z
// notes     `H` where the catalogue guessed `D`: the image loads a dword
//           for that parameter, so it is an int.
// size      107 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// indirect  0x005F16C3
// 0x005F1660  ?OnChar@Win@@QAAXPAXDH@Z  ->  ?OnChar@Win@@QAAXPAXDH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
// FREE, not a member, and the shift is the whole finding. The catalogue
// spells this `?OnChar@Win@@QAAXPAXDH@Z`; a QAA name invents a receiver, and
// that invented `this` IS the HWND. Every parameter therefore sat one slot
// late: what the member form called `hwnd` is the character and what it
// called `ch` is the flags, which is why the body used to read
// `on_char(hwnd, ch)` - dispatching the window handle as the typed
// character. The stack slots it reads are unchanged, so the bytes are
// unchanged; only the names were wrong. Its siblings OnNCHitTest and
// OnSysKey were homed as free functions for the same reason.
void __cdecl OnChar(void *hwnd, char ch, int flags) {
    Win *active;
    if (WinFocusWindow != nullptr) {
        active = WinModalFocus;
        if (active == nullptr) active = WinFocusWindow;
    } else {
        active = WinInputFocus;
        if (active == nullptr) active = WinDefaultFocus;
    }
    WinActiveWindow = active;
    if (active != nullptr) {
        Win *const base_window = active->poWinBase_;
        if (base_window != nullptr
            && (base_window->iSomeFlag_ & 1U) != 0) {
            Win *const parent = base_window->win_parent_;
            if (parent == nullptr || parent->is_visible() != 0) {
                base_window->on_char(ch, flags);
            }
        }
    }
}

// ORIGINAL: 0x005F15C0 ?OnKey@Win@@QAAXPAXIJHI@Z 0x005F15C0-0x005F1657 FILE BYTE_EXACT
// symbol    ?OnKey@Win@@SAXPAXIJHI@Z
// kind game
// TRIED: MISMATCH #1 test/push at the ternary-vs-if for the first global select; body verified structurally correct otherwise
// ===== homed from src/unrecovered/005f15c0.cpp =====

void __cdecl Win::OnKey(void * hwnd, unsigned int key, long flags, int repeat, unsigned int scan) {
    Win *chosen;
    if (WinFocusWindow != nullptr) {
        chosen = WinModalFocus;
        if (chosen == nullptr) chosen = WinFocusWindow;
    } else {
        chosen = WinInputFocus;
        if (chosen == nullptr) chosen = WinDefaultFocus;
    }
    WinActiveWindow = chosen;

    if (chosen != nullptr) {
        Win *const inner = chosen->poWinBase_;
        if (inner != nullptr && (inner->iSomeFlag_ & 1U) != 0) {
            Win *const parent = inner->win_parent_;
            int visible = 1;
            if (parent != nullptr) visible = parent->is_visible();
            if (visible != 0) inner->on_key(key, flags, repeat, scan);
        }
    }

    if (WinKeyHook != nullptr && flags != 0) WinKeyHook(key);
    if (WinMessageHook != nullptr) WinMessageHook();
}
