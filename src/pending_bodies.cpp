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
#include "caviar.h"
#include "font.h"
#include "game.h"
#include "general.h"
#include "palette.h"
#include "basepop.h"
#include "buffer.h"
#include "checkbox.h"
#include "checkbutton.h"
#include "cursor.h"
#include "filewin.h"
#include "radiobutton.h"
#include "time.h"
#include "win.h"
#include "popup.h"
#include "sound.h"

/*
 * THE FRONTIER.
 *
 * Every function the compiled tree CALLS BY NAME but has not promoted into
 * the build yet gets one forwarder here, aimed at its address in the shipped
 * image. Nothing else belongs in this file.
 *
 * WHY IT EXISTS. Adding the recovered `WinMain` to the link added six
 * undefined symbols, and their transitive closure is 1,489 catalogued
 * addresses - the whole remaining recovery, arriving in one link error. The
 * alternative to a forwarder was to keep spelling callees as raw addresses at
 * the call site, the way the injected DLL did:
 *
 *     typedef int(__cdecl fn)(Palette *, Font *, LPSTR, int, int, int, int);
 *     ((fn *)0x0062D3A0)(&g_PALETTE1, &g_JACKAL_FONT, ...);
 *
 * which compiles a `call dword ptr [...]` where the image has `call rel32`,
 * and leaves the caller reading as finished when it is not. A forwarder keeps
 * the call site honest C++, emits the `E8` the image emits, and puts every
 * not-yet-promoted edge in ONE place that can be counted.
 *
 * HOW ONE LEAVES. When a body lands in a compiled unit, DELETE its forwarder.
 * The linker enforces that: two definitions of one symbol is LNK2005, so a
 * stale forwarder cannot survive the promotion it was waiting for. That is
 * the whole maintenance rule, and it is mechanical rather than remembered.
 *
 * WHAT THEY DO AT RUNTIME. Jump into an address that means nothing in this
 * process, and fault. That is deliberate: a forwarder that returned a
 * plausible success would let the startup sequence run past the first thing
 * that is missing, and the point of an entry point is to say exactly how far
 * the recovery reaches.
 */

#define PENDING_BODY(address, signature) \
    reinterpret_cast<signature>(static_cast<unsigned long>(address))

// 0x0062D500  ?jackal_close@@YAXXZ  body in src/recovered/0062d500.cpp
void __cdecl jackal_close() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0062D500, pending)();
}

// 0x006185A0  ?init_class@Caviar@@QAAHXZ
//             body in src/unrecovered/006185a0.cpp
int Caviar::init_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x006185A0, pending)();
}

// 0x00618D20  ?close_class@Caviar@@QAAXXZ
//             body in src/recovered/units/00618d20.cpp
void Caviar::close_class() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x00618D20, pending)();
}

// 0x004C5CE0  ?init_sound@@YAHPAXK@Z  body in src/unrecovered/004c5ce0.cpp
int __cdecl init_sound(void *window, unsigned long backends) {
    typedef int(__cdecl *pending)(void *, unsigned long);
    return PENDING_BODY(0x004C5CE0, pending)(window, backends);
}

// 0x0052AA30  ?control_game@@YAXXZ  body in src/unrecovered/0052aa30.cpp
void __cdecl control_game() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0052AA30, pending)();
}

// 0x00404FB0  ?alloc@Popup@@QAAHXZ  body in src/unrecovered/00404fb0.cpp
//
// Reached for its ADDRESS rather than by call: WinMain stores it into the
// popup allocator hook. A forwarder still has to exist, because the address
// of a function that nothing defines does not link.
int Popup::alloc() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x00404FB0, pending)();
}

// 0x00600860  ??0BasePop@@QAE@XZ  body in src/unrecovered/00600860.cpp
//
// THE FRONTIER MOVED HERE when basepop_alloc was recovered into
// src/basepop.cpp: `new BasePop()` needs a constructor to link against, and
// basepop.h no longer defines one inline. The forwarder that used to sit here
// for basepop_alloc itself is gone - the linker enforces that, since two
// definitions of one symbol is LNK2005.
BasePop::BasePop() {
    typedef void(__cdecl *pending)(void *);
    PENDING_BODY(0x00600860, pending)(this);
}

// ---------------------------------------------------------------------------
// THE FRONTIER AFTER jackal_init_real, which was recovered into
// src/general.cpp on 2026-08-15. It calls nineteen functions; five were
// already compiled and these fourteen were not, so promoting one body moved
// the edge outward by fourteen. Each is one `#include` and one body away from
// deleting its line here.
// ---------------------------------------------------------------------------

// 0x005FE330  ?init@Palette@@QAEXXZ
void Palette::init() {
    typedef void(__cdecl *pending)(void *);
    PENDING_BODY(0x005FE330, pending)(this);
}

// 0x005FE460  ?set@Palette@@QAEHXZ
int Palette::set() {
    typedef int(__cdecl *pending)(void *);
    return PENDING_BODY(0x005FE460, pending)(this);
}

// 0x005F01F0  ?init_class@Win@@QAAHPAD@Z
int Win::init_class(LPSTR window_name) {
    typedef int(__cdecl *pending)(LPSTR);
    return PENDING_BODY(0x005F01F0, pending)(window_name);
}

// 0x005F2C40  ?set_display_mode@Win@@QAAHHHHH@Z
int Win::set_display_mode(int width, int height, int depth, int tgl) {
    typedef int(__cdecl *pending)(int, int, int, int);
    return PENDING_BODY(0x005F2C40, pending)(width, height, depth, tgl);
}

// 0x005EFD20  ?flip@Win@@QAAXPAURECT@@@Z
void Win::flip(RECT *area) {
    typedef void(__cdecl *pending)(RECT *);
    PENDING_BODY(0x005EFD20, pending)(area);
}

// 0x0060FC60  ?init_class@CheckBox@@QAAHXZ
int CheckBox::init_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0060FC60, pending)();
}

// 0x0060E4D0  ?init_class@RadioButton@@QAAHXZ
int RadioButton::init_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0060E4D0, pending)();
}

// 0x00604590  ?init_class@BasePop@@QAAHXZ
int BasePop::init_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x00604590, pending)();
}

// 0x00614D90  ?init_class@FileWin@@QAAHXZ
int FileWin::init_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x00614D90, pending)();
}

// 0x0063B910  ?init_cursor_class@Cursor@@QAAXXZ - `int` here, see cursor.h
int Cursor::init_cursor_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0063B910, pending)();
}

// 0x0063B940  ?trig_init@@YAHXZ
int __cdecl trig_init() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0063B940, pending)();
}

// 0x0063CE20  no catalogued name
int __cdecl sub_63ce20() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0063CE20, pending)();
}
