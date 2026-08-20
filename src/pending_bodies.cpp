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
#include "sprite.h"
#include "time.h"
#include "win.h"
#include "map.h"
#include "mapwin.h"
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

// ?jackal_close@@YAXXZ at 0x0062D500 - body in src/recovered/0062d500.cpp
void __cdecl jackal_close() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0062D500, pending)();
}

// ?init_class@Caviar@@QAAHXZ at 0x006185A0
//             body in src/unrecovered/006185a0.cpp
int Caviar::init_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x006185A0, pending)();
}

// ?close_class@Caviar@@QAAXXZ at 0x00618D20
//             body in src/recovered/units/00618d20.cpp
void Caviar::close_class() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x00618D20, pending)();
}

// ---------------------------------------------------------------------------
// PROMOTED OUT OF temp.h, where they were function POINTERS. A pointer
// binding compiles `call dword ptr [draw_map]` where the image has
// `call rel32`, so every caller of one reads as a mismatch however right its
// body is - that is what held `Console::editor_polar` and
// `Console::editor_climate` off the ratchet. The address is the same one
// src/temp.cpp bound, so nothing about runtime changed; only the call shape.
// ---------------------------------------------------------------------------

// ?draw_map@@YAXH@Z at 0x0046B190 - body in src/recovered/units/0046b190.cpp
void __cdecl draw_map(int draw_type) {
    typedef void(__cdecl *pending)(int);
    PENDING_BODY(0x0046B190, pending)(draw_type);
}

// ?world_climate@@YAXXZ at 0x005C5A30 - body in src/unrecovered/005c5a30.cpp
void __cdecl world_climate() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x005C5A30, pending)();
}

// ?custom_planet@@YAHHH@Z at 0x0058C2A0 - body in src/unrecovered/0058c2a0.cpp
int __cdecl custom_planet(int a, int b) {
    typedef int(__cdecl *pending)(int, int);
    return PENDING_BODY(0x0058C2A0, pending)(a, b);
}

// ?control_game@@YAXXZ at 0x0052AA30 - body in src/unrecovered/0052aa30.cpp
void __cdecl control_game() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0052AA30, pending)();
}

// ??0BasePop@@QAE@XZ at 0x00600860 - body in src/unrecovered/00600860.cpp
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

// ?set_display_mode@Win@@QAAHHHHH@Z at 0x005F2C40
int Win::set_display_mode(int width, int height, int depth, int tgl) {
    typedef int(__cdecl *pending)(int, int, int, int);
    return PENDING_BODY(0x005F2C40, pending)(width, height, depth, tgl);
}

// ?flip@Win@@QAAXPAURECT@@@Z at 0x005EFD20
void Win::flip(RECT *area) {
    typedef void(__cdecl *pending)(RECT *);
    PENDING_BODY(0x005EFD20, pending)(area);
}

// What `Win::window_proc` routes to, now that it is promoted into
// src/win.cpp. These six are what its recovery still stands on, and the
// first thing that faults if you move the mouse or press a key.
//
// ?recurse_zorder@@YAXPAUWin@@@Z at 0x005F4EC0
void __cdecl recurse_zorder(Win *window) {
    typedef void(__cdecl *pending)(Win *);
    PENDING_BODY(0x005F4EC0, pending)(window);
}

// ?get_key_window@Win@@QAGHXZ at 0x005F6A50
Win *Win::get_key_window() {
    typedef Win *(__stdcall *pending)();
    return PENDING_BODY(0x005F6A50, pending)();
}

// ?screen_to_client@Win@@QAEXPAHPAH@Z at 0x005ED2D0 - BYTE_EXACT already in
// src/unrecovered/005ed2d0.cpp, and 133 bytes, so this edge is cheap to
// close next.
void Win::screen_to_client(int *x, int *y) {
    typedef void(__fastcall *pending)(Win *, void *, int *, int *);
    PENDING_BODY(0x005ED2D0, pending)(this, nullptr, x, y);
}

// ?get_mouse_window_recurse@@YAHPAUWin@@PAHPAH@Z at 0x005F6AB0 - the tree walk
// `get_mouse_window` delegates to, 1110 bytes.
Win *__cdecl get_mouse_window_recurse(Win *window, int *x, int *y) {
    typedef Win *(__cdecl *pending)(Win *, int *, int *);
    return PENDING_BODY(0x005F6AB0, pending)(window, x, y);
}

// ?update_cursor@Win@@QAAHPAUWin@@H@Z at 0x005F1820
int Win::update_cursor(Win *window, int tgl) {
    typedef int(__cdecl *pending)(Win *, int);
    return PENDING_BODY(0x005F1820, pending)(window, tgl);
}

// ?update_screen@Win@@QAAHPAURECT@@PAVWin@@@Z at 0x005F7320
int Win::update_screen(RECT *area, Win *window) {
    typedef int(__cdecl *pending)(RECT *, Win *);
    return PENDING_BODY(0x005F7320, pending)(area, window);
}

// ?do_tracking@Win@@QAEXHH@Z at 0x005F7580 - the one __thiscall member of the
// set, so the forwarder hands the receiver over explicitly.
void Win::do_tracking(int x, int y) {
    typedef void(__fastcall *pending)(Win *, void *, int, int);
    PENDING_BODY(0x005F7580, pending)(this, nullptr, x, y);
}

// sub_5f86a0 at 0x005F86A0 - byte-exact in src/recovered/005f86a0.cpp, which
// is in no build, so the edge is still pending here.
extern "C" void __stdcall sub_5f86a0(int a1) {
    typedef void(__stdcall *pending)(int);
    PENDING_BODY(0x005F86A0, pending)(a1);
}

// ?load_pcx@Buffer@@QAEHPAEKPAVPalette@@HH@Z at 0x005E2690 - 1127 bytes, body
// in src/unrecovered/005e2690.cpp. THE FRONTIER MOVED HERE when the by-name
// `Buffer::load_pcx` landed byte-exact in src/buffer.cpp: that one only maps
// the file and fixes up the extension, and hands the bytes to this decoder.
int Buffer::load_pcx(BYTE *data, DWORD size, Palette *palette, int tgl,
                     int height) {
    typedef int(__fastcall *pending)(Buffer *, void *, BYTE *, DWORD,
                                     Palette *, int, int);
    return PENDING_BODY(0x005E2690, pending)(this, nullptr, data, size,
                                             palette, tgl, height);
}

// ?init_cursor_class@Cursor@@QAAXXZ at 0x0063B910 - `int` here, see cursor.h
int Cursor::init_cursor_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0063B910, pending)();
}

// ?trig_init@@YAHXZ at 0x0063B940
int __cdecl trig_init() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0063B940, pending)();
}
