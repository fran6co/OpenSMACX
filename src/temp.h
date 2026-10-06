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
#pragma once
#include "faction.h"
#include "general.h"
#include "maininterface.h"
#include "win.h"

 /*
  * Temp functions and offset pointers. This allows interfacing with parts of the exe not yet
  * decompiled. Eventually, this will be removed.
  */

typedef struct char256 { char str[256]; } char256;

// built-in functions

//typedef void *func12(void *, size_t);
//extern func12 *_realloc;
//typedef int *func18(FILE *);
//extern func18 *_fclose;
//typedef LPSTR *func19(LPSTR, int, FILE *);
//extern func19 *_fgets;

// other
typedef void func5(int);
typedef void *func6(char const *, int(*)(void));
typedef int func7(LPCSTR, LPCSTR, int, LPCSTR, int(__cdecl *)());
// 0x0048C0A0, implemented in popup.cpp.
int __cdecl popp(LPCSTR a1, LPCSTR a2, int a3, LPCSTR a4, int(__cdecl *a5)());
typedef void *func9(void);
// A POINTER BINDING COSTS ITS CALLERS THE RATCHET. It compiles
// `call dword ptr [name]` where the image has `call rel32`, so every caller
// reads as a mismatch however right its body is - which is what held both of
// the scenario editor's Console commands off the ratchet. The direct
// source-owned functions `draw_map` (src/mapwin.cpp), `world_climate` (src/map.cpp),
// and `custom_planet` (src/newgame.cpp) are called by name, emitting the `E8`.
// Prefer a direct declaration for anything a recovered body calls BY NAME;
// a function pointer is only right for a genuine indirection.
typedef BOOL func11(int, int, int);
typedef uint32_t func15(LPCSTR);
// 0x005A94F0, recovered in game.cpp with the original mutable-pointer ABI.
// A SAVE PATH, from the callers: general.cpp passes
// "saves\\auto\\Alpha Centauri Autosave 1", and scenario.cpp passes
// State.save_path to the load side.
int __cdecl save_daemon(char *save_path);
typedef int func16(LPCSTR, int);
// 0x005A9760, recovered in game.cpp.
int __cdecl load_daemon(LPCSTR save_path, int linearize_contours);
typedef void func19(uint32_t);
typedef int func20(LPCSTR, int, int, LPCSTR, Sprite *);
// 0x0048C650, implemented in popup.cpp.
int __cdecl popb(LPCSTR a1, int a2, int a3, LPCSTR a4, Sprite * a5);
typedef void func21(int, int, int, int);
// 0x0046AEF0, defined at the end of src/mapwin.cpp.
// mapwin.h:314 declares this call's own typedef with three of the four
// named: `(int x_coord, int y_coord, int a3, int draw_type)`.
void __cdecl draw_radius(int x_coord, int y_coord, int a3, int draw_type);

// Time
typedef void func30(int);
// The four callbacks below are implemented in time.cpp.
// They were bound as pointers here, which cost `start_timers` the image's `E8`
// at each of its four calls - see the note there.
// 0x0050EA40, implemented in time.cpp beside the other timer callbacks.
void __cdecl blink_timer(int a1);
// 0x0050EE30, implemented in time.cpp.
void __cdecl blink2_timer(int a1);
// 0x0050EE80, implemented in time.cpp.
void __cdecl line_timer(int a1);
// 0x0050EF10, implemented in time.cpp.
void __cdecl turn_timer(int a1);

// Timer-domain state, defined in time.cpp beside the callbacks. Every one of
// these is zero-initialised .data in the image (image_data.py).
// The hold flag: blink_timer, blink2_timer, line_timer and turn_timer all
// return early while it is nonzero (0x0050EA40, 0x0050EE30, 0x0050EE80,
// 0x0050EF10) and so does the start side at 0x0050F170.
extern int TimerHoldFlag;     // 0x00915620
// blink2_timer's tick counter, incremented once per accepted tick.
extern int Blink2Counter;     // 0x009392B4
// line_timer counts it down once per accepted tick before repainting the plan
// lines; the plan-line draw bodies pass the dword to Buffer::line as a color.
extern int PlanLineCounter;   // 0x00939FD4
// turn_timer repaints the network turn window only while it is nonzero; the
// send-side loop at 0x00515770 reads it in the same breath as its turn flags.
extern int TurnRedrawPending;  // 0x00703DE0
// The window TurnRedrawPending gates: turn_timer redraws it (GraphicWin::redraw
// at 0x005D5A70) before MultiWin::draw on 0x007FD648.
GraphicWin *const TurnRedrawWindow = (GraphicWin *)0x006FEC80;

// testing
typedef int *func8(LPSTR, LPSTR);
// The four parse-string tables, REAL OBJECTS since 2026-08-29 - they were
// bindings on 0x009BB570..0x009BB5E8, and every parse_says/parse_num call
// wrote through them into unmapped image memory. Ten entries each, the
// `id > 9` guard every family member enforces; the image lays them out
// contiguously in this order (plurality, num, gender, then the 0xA00-byte
// string buffer ending at GenderDefault's 0x009BBFEC).
extern char256 ParseStrBuffer[10];

extern int BufferStrHeight;
extern int ParseNumTable[10];
extern int ParseStrPlurality[10];
extern int ParseStrGender[10];
// The main window. AN OBJECT, for the reason in game.h on
// ExpansionEnabled: 0x009B7B28 is zero-fill, so the pointer form carried
// no information and cost a load at every use.
extern HWND HandleMain;

extern uint32_t UnkBitfield1;

/*
 * AN OBJECT, NOT A `Filefind *`. The image takes the address of a member as
 * an IMMEDIATE - `cmp esi, 0x9b8398` and `mov eax, 0x9b8398` in
 * `filefind_get`, which is `&FilefindPath.last_path` at 0x009B8198 + 0x200 -
 * where a pointer variable compiles a load and an add. It is also five
 * kilobytes of terranx.exe's data, unmapped in a standalone build, so every
 * path this tree read through it was a wild pointer.
 */
extern Filefind FilefindPath;
// 0x007AE820 is MainInterfaceGlobal (mapwin.cpp) - the same object this file
// used to bind as `MainInterface *const MainInterfaceVar`. Uses name the
// object directly.

int __cdecl tester();

// tech_val output
typedef int func13(int, int, BOOL);
void __cdecl tech_calc_output();



// DECLARED, NOT BOUND. A `T *const` binding compiles `call dword ptr [...]` at
// every call site where the image emits `call rel32`, which costs the CALLER
// its claim - see the pump family in this file, and the vector iterators in
// `vector_teardown.h`. Each of these is forwarded in `pending_bodies.cpp`
// until its body lands.
void __cdecl wave_it(int a1);
void __cdecl fixup_landmarks();
void __cdecl mapwin_terrain_fixup();
void __cdecl world_rainfall();
void __cdecl consider_designs(int faction_id);
