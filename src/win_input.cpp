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

// ===== homed from src/unrecovered/005f5fb0.cpp =====

// ORIGINAL: 0x005F5FB0 sub_5f5fb0 0x005F5FB0-0x005F622B
// RECOVERY_BINDING: v1 {"evidence": ["git:85fc7721f56c2a5cae754b993fe04fcfec74fec1:src/unrecovered/005f5fb0.cpp", "Pinned original5F5FB0..5F622B; direct global accesses5F60DD..5F619C"], "reason": "Restore the lost homing identity for the existing Win keyboard handler; its receiver, scroll dispatch and two stack words identify this method, not the separate string matcher at5F5F60."}
// symbol    ?on_char@Win@@UAEXDH@Z
// TRIED: extern "C" __stdcall(int,int) free function per IDA's guess - the spilled ecx is read back later ([esp+0x10] resolves to the "push ecx" slot itself, not a caller stack arg) and used as a Win* (scroll_vert_/scroll_horz_/key_click_event), so this is landed as Win::sub_5f5fb0(char, int) __thiscall instead; the unused 2nd stack dword explains `ret 8` with only 1 real read.
void Win::on_char(char param2, int param3) {
    char **cursor = &WinKeyRingCursor;
    **cursor = param2;

    bool matched = true;
    {
        // the LAST character; the loop below compares backward from it
        const char *pat = WinMdebugCode + 5;
        char *pos = *cursor;
        int count = 6;
        do {
            if (*pos != *pat) {
                matched = false;
                break;
            }
            pos--;
            pat--;
            if (pos < (char *)WinKeyRing) {
                pos = (char *)(WinKeyRing + 9);
            }
            count--;
        } while (count > 0);
    }

    if (matched) {
        Win &debug_window = WinBubbleWindow;
        if ((static_cast<uint8_t>(debug_window.iSomeFlag_) & 1) == 0 ||
            (debug_window.win_parent_ != 0 &&
             debug_window.win_parent_->is_visible() == 0)) {
            (&WinBubbleWindow)->show(0);
        } else if ((static_cast<uint8_t>(debug_window.iSomeFlag_) & 1) != 0) {
            if (WinBubbleCompanion == &WinBubbleWindow &&
                WinBubbleActive != 0) {
                WinBubbleCompanion = nullptr;
                WinBubbleActive = 0;
                (&WinBubbleWindow)->update_screen(&WinBubbleRect, 0);
                (&WinBubbleWindow)->flip(&WinBubbleRect);
            }

            Win *cand = WinPointerOwner1;
            if (cand == nullptr) {
                cand = WinPointerOwner2;
            }
            if (cand == &WinBubbleWindow) {
                WinPointerOwner1 = nullptr;
                WinPointerOwner2 = 0;
            } else if (cand != nullptr && debug_window.child_count_ > 0) {
                Win **list = debug_window.children_;
                int i = 0;
                do {
                    if (list[i] == cand || list[i]->is_descendant(cand)) {
                        WinPointerOwner1 = nullptr;
                        WinPointerOwner2 = 0;
                        break;
                    }
                    i++;
                } while (i < debug_window.child_count_);
            }

            if (WinPointerOwner3 == &WinBubbleWindow) {
                WinPointerOwner3 = 0;
                (&WinBubbleWindow)->on_lose_mouse_capture();
            }
            if (WinPointerOwner4 == &WinBubbleWindow) {
                WinPointerOwner4 = 0;
            }
            debug_window.iSomeFlag_ &= ~1;
            if (WinFocusWindow == &WinBubbleWindow) {
                (&WinBubbleWindow)->release_modal();
            }

            if ((JackalInitFlags & 1) != 0) {
                WinZOrderCount = 0;
                if (WinRootCount > 0) {
                    Win **entries = WinRootWindows;
                    Win *cur = WinZOrderWindow;
                    int j = 0;
                    do {
                        if (cur != 0 && cur == entries[j]) {
                            WinZOrderCount = 0;
                            WinZOrderFlag = 0;
                        }
                        if ((entries[j]->iSomeFlag_ & 1) != 0) {
                            recurse_zorder(entries[j]);
                            cur = WinZOrderWindow;
                        }
                        j++;
                    } while (j < WinRootCount);
                }

                InvalidateRect(HandleMain, 0, 0);

                Sound *w = debug_window.val_4_;
                if (w != 0 && w->is_playing() == 0) {
                    w->play();
                }
            }
        }
    }

    *cursor = *cursor + 1;
    if (*cursor > (char *)(WinKeyRing + 9)) {
        *cursor = (char *)WinKeyRing;
    }

    bool handled = false;
    if (scroll_vert_ != 0 &&
        scroll_vert_->on_key_click((int)param2, WinKeyModifiers) != 0) {
        handled = true;
    }
    if (!handled && scroll_horz_ != 0 &&
        scroll_horz_->on_key_click((int)param2, WinKeyModifiers) != 0) {
        handled = true;
    }
    if (!handled) {
        key_click_event((int)param2, WinKeyModifiers);
    }
}
