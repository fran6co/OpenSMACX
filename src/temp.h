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
func5 *const load_faction_art = (func5 *)0x00453710;
func5 *const wave_it = (func5 *)0x004455F0;
typedef void *func6(char const *, int(*)(void));
typedef int func7(LPCSTR, LPCSTR, int, LPCSTR, int(__cdecl *)());
func7 *const popp = (func7 *)0x0048C0A0;
typedef void *func9(void);
func9 *const fixup_landmarks = (func9 *)0x00592940;
func9 *const mapwin_terrain_fixup = (func9 *)0x00471240;
func9 *const world_rainfall = (func9 *)0x005C4470;
// A POINTER BINDING COSTS ITS CALLERS THE RATCHET. It compiles
// `call dword ptr [name]` where the image has `call rel32`, so every caller
// reads as a mismatch however right its body is - which is what held both of
// the scenario editor's Console commands off the ratchet. `draw_map`,
// `world_climate` and `custom_planet` left here for that reason: they are
// declared properly now and forwarded in src/pending_bodies.cpp, which emits
// the `E8`. Prefer a forwarder to a pointer for anything a recovered body
// calls BY NAME; a pointer is only right for a genuine indirection.
typedef BOOL func11(int, int, int);
func11 *const wants_to_attack_ = (func11 *)0x0055BC80;
typedef uint32_t func15(LPCSTR);
// 0x005A94F0, a pending_bodies forwarder.
uint32_t __cdecl save_daemon(LPCSTR a1);
typedef uint32_t func16(LPCSTR, BOOL);
// 0x005A9760, a pending_bodies forwarder.
uint32_t __cdecl load_daemon(LPCSTR a1, BOOL a2);
typedef void func19(uint32_t);
func19 *const social_set = (func19 *)0x005B4600;
func19 *const consider_designs = (func19 *)0x00581260;
typedef int func20(LPCSTR, int, int, LPCSTR, Sprite *);
// 0x0048C650, a pending_bodies forwarder.
int __cdecl popb(LPCSTR a1, int a2, int a3, LPCSTR a4, Sprite * a5);
typedef void func21(int, int, int, int);
// 0x0046AEF0, a pending_bodies forwarder.
void __cdecl draw_radius(int a1, int a2, int a3, int a4);
typedef void func22(int, int, uint32_t);
// 0x00591290, a pending_bodies forwarder.
void __cdecl alt_set(int a1, int a2, uint32_t a3);

// Time
typedef void func30(int);
func30 *const blink_timer = (func30 *)0x0050EA40;
func30 *const blink2_timer = (func30 *)0x0050EE30;
func30 *const line_timer = (func30 *)0x0050EE80;
func30 *const turn_timer = (func30 *)0x0050EF10;

// testing
typedef int *func8(LPSTR, LPSTR);
func8 *const parse_string_OG = (func8 *)0x00625880;
char256 *const ParseStrBuffer = (char256 *)0x009BB5E8;

extern int BufferStrHeight;
int *const ParseNumTable = (int *)0x009BB598;
int *const ParseStrPlurality = (int *)0x009BB570;
int *const ParseStrGender = (int *)0x009BB5C0;
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
MainInterface *const MainInterfaceVar = (MainInterface *)0x007AE820;

int __cdecl tester();

// tech_val output
typedef int func13(int, int, BOOL);
func13 *const tech_val_OG = (func13 *)0x005BCBE0;
void __cdecl tech_calc_output();

// message handling testing
typedef void *func_msg(void);
func_msg *const do_video = (func_msg *)0x00636300;
func_msg *const check_net = (func_msg *)0x0062D5D0;
func_msg *const do_net = (func_msg *)0x0062D5B0;
func_msg *const do_non_input_ = (func_msg *)0x005FCA30;

/*
 * AN OBJECT, NOT A `uint32_t *` TO A FIXED ADDRESS. The pointer form costs a
 * load the image does not perform: `MsgStatus |= 0x3F` is
 * `mov ecx, [0x9b7b9c]` in the image and `mov eax, [MsgStatus]` followed by
 * `mov ecx, [eax]` through a pointer, which is what held `flush_timer` at
 * 4 of 44 instructions. It also cannot work outside the shipped image - the
 * address is in terranx.exe's data, unmapped in a standalone build, so every
 * use through it is a wild pointer.
 */
extern uint32_t MsgStatus;

BOOL __cdecl do_non_input();
BOOL __cdecl do_draw();
void __cdecl do_all_draws();
BOOL __cdecl do_keyboard();
void __cdecl do_all_keyboard();

MEASURED inline void __cdecl do_all_non_input() {
    do {
        MsgStatus = 32;
    } while (do_non_input_());
    MsgStatus = 0;
    do_net();
    check_net();
}
