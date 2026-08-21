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
#include "original_seam.h"
#include "atexit_thunks.h"
#include "ambience.h"
#include "battlewin.h"
#include "buffer.h"
#include "buttongroup.h"
#include "caviar.h"
#include "font.h"
#include "fx.h"
#include "sprite.h"
#include "texture.h"
#include "wave.h"
#include "basewin.h"
#include "councwin.h"
#include "datalink.h"
#include "designwin.h"
#include "diplopop.h"
#include "diplowin.h"
#include "maininterface.h"
#include "multidebug.h"
#include "netmsg.h"
#include "netwin.h"
#include "newtechwin.h"
#include "pickwin.h"
#include "reportwin.h"
#include "socialwin.h"
#include "tutwin.h"
#include "worldwin.h"
#include "hypothesis_layouts.h"

func_wave_destructor WaveOriginalDestructor =
    original_method<func_wave_destructor>(0x004C67C0);


// Per-element teardowns the array walks dispatch to. The Wave one binds
// the same address as WaveOriginalDestructor deliberately: it is the same
// original body, held separately so the array seam rebinds alone.
const void *const BufferElementTeardown = (const void *)0x005D7410;
const void *const CaviarDataElementTeardown = (const void *)0x00456100;
const void *const FactionArtElementTeardown = (const void *)0x00456010;
const void *const FontElementTeardown = (const void *)0x00618EE0;
const void *const SpriteElementTeardown = (const void *)0x00406850;
const void *const TextIndexElementTeardown = (const void *)0x005FDF60;
const void *const TextureElementTeardown = (const void *)0x00619660;
const void *const WaveElementTeardown = (const void *)0x004C67C0;

// The opaque-storage teardowns, each defaulting to the
// original destructor the thunk tail jumps to; the
// hybrid redirects it to a recovered body at run time.
func_thiscall_teardown InfoWinDtorTarget =
    original_method<func_thiscall_teardown>(0x004594A0);
func_thiscall_teardown MessageWinDtorTarget =
    original_method<func_thiscall_teardown>(0x00472020);
func_thiscall_teardown ReportIfDtorTarget =
    original_method<func_thiscall_teardown>(0x004ACDA0);
func_thiscall_teardown StatusWinDtorTarget =
    original_method<func_thiscall_teardown>(0x004BA120);
func_thiscall_teardown Wave_DeviceDtorTarget =
    original_method<func_thiscall_teardown>(0x004C4E60);
func_thiscall_teardown Midi_DeviceDtorTarget =
    original_method<func_thiscall_teardown>(0x004C5780);
func_thiscall_teardown Wave_In_DeviceDtorTarget =
    original_method<func_thiscall_teardown>(0x004C5980);
func_thiscall_teardown NetDaemonDtorTarget =
    original_method<func_thiscall_teardown>(0x00538D10);
func_thiscall_teardown PaletteDtorTarget =
    original_method<func_thiscall_teardown>(0x005FE2E0);
func_thiscall_teardown TimeDtorTarget =
    original_method<func_thiscall_teardown>(0x00616200);
func_thiscall_teardown StringsDtorTarget =
    original_method<func_thiscall_teardown>(0x006169C0);
func_thiscall_teardown CaviarCloseTarget =
    original_method<func_thiscall_teardown>(0x00617020);

Font g_JACKAL_FONT;  // 0x007D3948

/*
Purpose: Atexit teardown thunk for g_ALPHAMENU_WAVE.
// ORIGINAL: 0x00402F30 ??__Fg_ALPHAMENU_WAVE@@YAXXZ 0x00402F30-0x00402F3A
// symbol    ?destroy_alphamenu_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_alphamenu_wave() {
    (ORIGINAL(g_ALPHAMENU_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR02.
// ORIGINAL: 0x00404460 ??__Fg_UNUSED_SPRITE_VAR02@@YAXXZ 0x00404460-0x0040446A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var02@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var02() {
    g_UNUSED_SPRITE_VAR02->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR11.
// ORIGINAL: 0x00404490 ??__Fg_UNUSED_SPRITE_VAR11@@YAXXZ 0x00404490-0x0040449A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var11@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var11() {
    g_UNUSED_SPRITE_VAR11->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR06.
// ORIGINAL: 0x004044C0 ??__Fg_UNUSED_SPRITE_VAR06@@YAXXZ 0x004044C0-0x004044CA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var06@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var06() {
    g_UNUSED_SPRITE_VAR06->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR09.
// ORIGINAL: 0x004044F0 ??__Fg_UNUSED_SPRITE_VAR09@@YAXXZ 0x004044F0-0x004044FA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var09@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var09() {
    g_UNUSED_SPRITE_VAR09->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR21.
// ORIGINAL: 0x00404520 ??__Fg_UNUSED_SPRITE_VAR21@@YAXXZ 0x00404520-0x0040452A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var21@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var21() {
    g_UNUSED_SPRITE_VAR21->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR05.
// ORIGINAL: 0x00404550 ??__Fg_UNUSED_SPRITE_VAR05@@YAXXZ 0x00404550-0x0040455A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var05@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var05() {
    g_UNUSED_SPRITE_VAR05->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR08.
// ORIGINAL: 0x00404580 ??__Fg_UNUSED_SPRITE_VAR08@@YAXXZ 0x00404580-0x0040458A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var08@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var08() {
    g_UNUSED_SPRITE_VAR08->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR04.
// ORIGINAL: 0x004045B0 ??__Fg_UNUSED_SPRITE_VAR04@@YAXXZ 0x004045B0-0x004045BA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var04@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var04() {
    g_UNUSED_SPRITE_VAR04->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR01.
// ORIGINAL: 0x004045E0 ??__Fg_UNUSED_SPRITE_VAR01@@YAXXZ 0x004045E0-0x004045EA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var01@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var01() {
    g_UNUSED_SPRITE_VAR01->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR18.
// ORIGINAL: 0x00404610 ??__Fg_UNUSED_SPRITE_VAR18@@YAXXZ 0x00404610-0x0040461A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var18@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var18() {
    g_UNUSED_SPRITE_VAR18->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR03.
// ORIGINAL: 0x00404640 ??__Fg_UNUSED_SPRITE_VAR03@@YAXXZ 0x00404640-0x0040464A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var03@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var03() {
    g_UNUSED_SPRITE_VAR03->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR20.
// ORIGINAL: 0x00404670 ??__Fg_UNUSED_SPRITE_VAR20@@YAXXZ 0x00404670-0x0040467A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var20@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var20() {
    g_UNUSED_SPRITE_VAR20->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR16.
// ORIGINAL: 0x004046A0 ??__Fg_UNUSED_SPRITE_VAR16@@YAXXZ 0x004046A0-0x004046AA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var16@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var16() {
    g_UNUSED_SPRITE_VAR16->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR14.
// ORIGINAL: 0x004046D0 ??__Fg_UNUSED_SPRITE_VAR14@@YAXXZ 0x004046D0-0x004046DA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var14@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var14() {
    g_UNUSED_SPRITE_VAR14->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR22.
// ORIGINAL: 0x00404700 ??__Fg_UNUSED_SPRITE_VAR22@@YAXXZ 0x00404700-0x0040470A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var22@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var22() {
    g_UNUSED_SPRITE_VAR22->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR10.
// ORIGINAL: 0x00404730 ??__Fg_UNUSED_SPRITE_VAR10@@YAXXZ 0x00404730-0x0040473A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var10@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var10() {
    g_UNUSED_SPRITE_VAR10->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR15.
// ORIGINAL: 0x00404760 ??__Fg_UNUSED_SPRITE_VAR15@@YAXXZ 0x00404760-0x0040476A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var15@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var15() {
    g_UNUSED_SPRITE_VAR15->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR13.
// ORIGINAL: 0x00404790 ??__Fg_UNUSED_SPRITE_VAR13@@YAXXZ 0x00404790-0x0040479A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var13@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var13() {
    g_UNUSED_SPRITE_VAR13->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR17.
// ORIGINAL: 0x004047C0 ??__Fg_UNUSED_SPRITE_VAR17@@YAXXZ 0x004047C0-0x004047CA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var17@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var17() {
    g_UNUSED_SPRITE_VAR17->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR19.
// ORIGINAL: 0x004047F0 ??__Fg_UNUSED_SPRITE_VAR19@@YAXXZ 0x004047F0-0x004047FA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var19@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var19() {
    g_UNUSED_SPRITE_VAR19->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR12.
// ORIGINAL: 0x00404820 ??__Fg_UNUSED_SPRITE_VAR12@@YAXXZ 0x00404820-0x0040482A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var12@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var12() {
    g_UNUSED_SPRITE_VAR12->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR07.
// ORIGINAL: 0x00404850 ??__Fg_UNUSED_SPRITE_VAR07@@YAXXZ 0x00404850-0x0040485A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var07@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var07() {
    g_UNUSED_SPRITE_VAR07->close();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN.
// ORIGINAL: 0x004083D0 ??__Fg_BASEWIN@@YAXXZ 0x004083D0-0x004083DA BYTE_EXACT
// symbol    ?destroy_basewin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin() {
    static_cast<BaseWin *>(reinterpret_cast<void *>(0x006A7628))->BaseWin::~BaseWin();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN_WAVE.
// ORIGINAL: 0x00408400 ??__Fg_BASEWIN_WAVE@@YAXXZ 0x00408400-0x0040840A
// symbol    ?destroy_basewin_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin_wave() {
    (ORIGINAL(g_BASEWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_BattleWin.
// ORIGINAL: 0x004219D0 ??__Fg_BattleWin@@YAXXZ 0x004219D0-0x004219DA BYTE_EXACT
// symbol    ?destroy_battlewin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battlewin() {
    g_BattleWin->~BattleWin();
}

/*
Purpose: Atexit teardown thunk for g_COUNCWIN.
// ORIGINAL: 0x004243C0 ??__Fg_COUNCWIN@@YAXXZ 0x004243C0-0x004243CA BYTE_EXACT
// symbol    ?destroy_councwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_councwin() {
    static_cast<CouncWin *>(reinterpret_cast<void *>(0x006FEC80))->CouncWin::~CouncWin();
}

/*
Purpose: Atexit teardown thunk for g_CREDITS_WAVE.
// ORIGINAL: 0x00428770 ??__Fg_CREDITS_WAVE@@YAXXZ 0x00428770-0x0042877A
// symbol    ?destroy_credits_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_credits_wave() {
    (ORIGINAL(g_CREDITS_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_DATALINK.
// ORIGINAL: 0x00428FB0 ??__Fg_DATALINK@@YAXXZ 0x00428FB0-0x00428FBA BYTE_EXACT
// symbol    ?destroy_datalink@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_datalink() {
    static_cast<Datalink *>(reinterpret_cast<void *>(0x00703EA0))->Datalink::~Datalink();
}

/*
Purpose: Atexit teardown thunk for g_DESIGNWIN.
// ORIGINAL: 0x00432870 ??__Fg_DESIGNWIN@@YAXXZ 0x00432870-0x0043287A BYTE_EXACT
// symbol    ?destroy_designwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_designwin() {
    static_cast<DesignWin *>(reinterpret_cast<void *>(0x0071F2B0))->DesignWin::~DesignWin();
}

/*
Purpose: Atexit teardown thunk for g_DESIGNWIN_WAVE.
// ORIGINAL: 0x004328A0 ??__Fg_DESIGNWIN_WAVE@@YAXXZ 0x004328A0-0x004328AA
// symbol    ?destroy_designwin_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_designwin_wave() {
    (ORIGINAL(g_DESIGNWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_DIPLOPOP.
// ORIGINAL: 0x0043EFE0 ??__Fg_DIPLOPOP@@YAXXZ 0x0043EFE0-0x0043EFEA BYTE_EXACT
// symbol    ?destroy_diplopop@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_diplopop() {
    static_cast<DiploPop *>(reinterpret_cast<void *>(0x00733990))->DiploPop::~DiploPop();
}

/*
Purpose: Atexit teardown thunk for g_DIPLOWIN.
// ORIGINAL: 0x00440F20 ??__Fg_DIPLOWIN@@YAXXZ 0x00440F20-0x00440F2A BYTE_EXACT
// symbol    ?destroy_diplowin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_diplowin() {
    static_cast<DiploWin *>(reinterpret_cast<void *>(0x0073ACD8))->DiploWin::~DiploWin();
}

/*
Purpose: Atexit teardown thunk for g_CPU_WAVES.
// ORIGINAL: 0x00445480 ??__Fg_CPU_WAVES@@YAXXZ 0x00445480-0x00445494
// symbol    ?destroy_cpu_waves@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_cpu_waves() {
    VectorDtorIterator(g_CPU_WAVES, 0x6C, 45, WaveElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MENU_UP_WAVE.
// ORIGINAL: 0x004454C0 ??__Fg_MENU_UP_WAVE@@YAXXZ 0x004454C0-0x004454CA
// symbol    ?destroy_menu_up_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_menu_up_wave() {
    (ORIGINAL(g_MENU_UP_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_MENU_DOWN_WAVE.
// ORIGINAL: 0x004454F0 ??__Fg_MENU_DOWN_WAVE@@YAXXZ 0x004454F0-0x004454FA
// symbol    ?destroy_menu_down_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_menu_down_wave() {
    (ORIGINAL(g_MENU_DOWN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_SCOOT_WAVE.
// ORIGINAL: 0x00445520 ??__Fg_SCOOT_WAVE@@YAXXZ 0x00445520-0x0044552A
// symbol    ?destroy_scoot_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scoot_wave() {
    (ORIGINAL(g_SCOOT_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_OK_WAVE.
// ORIGINAL: 0x00445550 ??__Fg_OK_WAVE@@YAXXZ 0x00445550-0x0044555A
// symbol    ?destroy_ok_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ok_wave() {
    (ORIGINAL(g_OK_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_PASSOVER_WAVE.
// ORIGINAL: 0x00445580 ??__Fg_PASSOVER_WAVE@@YAXXZ 0x00445580-0x0044558A
// symbol    ?destroy_passover_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_passover_wave() {
    (ORIGINAL(g_PASSOVER_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_FX.
// ORIGINAL: 0x004455B0 ??__Fg_FX@@YAXXZ 0x004455B0-0x004455BA BYTE_EXACT
// symbol    ?destroy_fx@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fx() {
    g_FX->~FX();
}

/*
Purpose: Atexit teardown thunk for g_AMBIENCE.
// ORIGINAL: 0x004455E0 ??__Fg_AMBIENCE@@YAXXZ 0x004455E0-0x004455EA BYTE_EXACT
// symbol    ?destroy_ambience@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ambience() {
    g_AMBIENCE->~Ambience();
}

/*
Purpose: Atexit teardown thunk for g_FAMEWIN.
// ORIGINAL: 0x004483F0 ??__Fg_FAMEWIN@@YAXXZ 0x004483F0-0x004483FA BYTE_EXACT
// symbol    ?destroy_famewin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_famewin() {
    static_cast<FameWin *>(reinterpret_cast<void *>(0x0074DAF8))->FameWin::~FameWin();
}

/*
Purpose: Atexit teardown thunk for g_PCX_PARSE_TEMP_BUFFER1.
// ORIGINAL: 0x0044CC40 ??__Fg_PCX_PARSE_TEMP_BUFFER1@@YAXXZ 0x0044CC40-0x0044CC4A BYTE_EXACT
// symbol    ?destroy_pcx_parse_temp_buffer1@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_pcx_parse_temp_buffer1() {
    g_PCX_PARSE_TEMP_BUFFER1->Buffer::~Buffer();
}

/*
Purpose: Atexit teardown thunk for FactionArt.
// ORIGINAL: 0x0044CC80 ??__FFactionArt@@YAXXZ 0x0044CC80-0x0044CC97
// symbol    ?destroy_factionart@@YAXXZ
// size      23 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_factionart() {
    VectorDtorIterator(FactionArtGlobal, 0x65C, 8, FactionArtElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_CLOSE_X_SPRITES.
// ORIGINAL: 0x0044CCD0 ??__Fg_IFACE_CLOSE_X_SPRITES@@YAXXZ 0x0044CCD0-0x0044CCE4
// symbol    ?destroy_iface_close_x_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_close_x_sprites() {
    VectorDtorIterator(g_IFACE_CLOSE_X_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES1.
// ORIGINAL: 0x0044CD20 ??__Fg_IFACE_BOX_SPRITES1@@YAXXZ 0x0044CD20-0x0044CD34
// symbol    ?destroy_iface_box_sprites1@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites1() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES1, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES2.
// ORIGINAL: 0x0044CD70 ??__Fg_IFACE_BOX_SPRITES2@@YAXXZ 0x0044CD70-0x0044CD84
// symbol    ?destroy_iface_box_sprites2@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites2() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES2, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES3.
// ORIGINAL: 0x0044CDC0 ??__Fg_IFACE_BOX_SPRITES3@@YAXXZ 0x0044CDC0-0x0044CDD4
// symbol    ?destroy_iface_box_sprites3@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites3() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES3, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES4.
// ORIGINAL: 0x0044CE10 ??__Fg_IFACE_BOX_SPRITES4@@YAXXZ 0x0044CE10-0x0044CE24
// symbol    ?destroy_iface_box_sprites4@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites4() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES4, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES5.
// ORIGINAL: 0x0044CE60 ??__Fg_IFACE_BOX_SPRITES5@@YAXXZ 0x0044CE60-0x0044CE74
// symbol    ?destroy_iface_box_sprites5@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites5() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES5, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES6.
// ORIGINAL: 0x0044CEB0 ??__Fg_IFACE_BOX_SPRITES6@@YAXXZ 0x0044CEB0-0x0044CEC4
// symbol    ?destroy_iface_box_sprites6@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites6() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES6, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES7.
// ORIGINAL: 0x0044CF00 ??__Fg_IFACE_BOX_SPRITES7@@YAXXZ 0x0044CF00-0x0044CF14
// symbol    ?destroy_iface_box_sprites7@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites7() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES7, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES8.
// ORIGINAL: 0x0044CF50 ??__Fg_IFACE_BOX_SPRITES8@@YAXXZ 0x0044CF50-0x0044CF64
// symbol    ?destroy_iface_box_sprites8@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites8() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES8, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITE_BUFFERS.
// ORIGINAL: 0x0044CFA0 ??__Fg_IFACE_BOX_SPRITE_BUFFERS@@YAXXZ 0x0044CFA0-0x0044CFB7
// symbol    ?destroy_iface_box_sprite_buffers@@YAXXZ
// size      23 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprite_buffers() {
    VectorDtorIterator(g_IFACE_BOX_SPRITE_BUFFERS, 0x588, 51, BufferElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE.
// ORIGINAL: 0x0044CFE0 ??__Fg_IFACE_STD_POPUPS_TOP_LEFT_SPRITE@@YAXXZ 0x0044CFE0-0x0044CFEA BYTE_EXACT
// symbol    ?destroy_iface_std_popups_top_left_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_left_sprite() {
    g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE.
// ORIGINAL: 0x0044D010 ??__Fg_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE@@YAXXZ 0x0044D010-0x0044D01A BYTE_EXACT
// symbol    ?destroy_iface_std_popups_top_right_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_right_sprite() {
    g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE.
// ORIGINAL: 0x0044D040 ??__Fg_IFACE_STD_POPUPS_BOT_LEFT_SPRITE@@YAXXZ 0x0044D040-0x0044D04A BYTE_EXACT
// symbol    ?destroy_iface_std_popups_bot_left_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_left_sprite() {
    g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE.
// ORIGINAL: 0x0044D070 ??__Fg_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE@@YAXXZ 0x0044D070-0x0044D07A BYTE_EXACT
// symbol    ?destroy_iface_std_popups_bot_right_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_right_sprite() {
    g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_MID_SPRITE.
// ORIGINAL: 0x0044D0A0 ??__Fg_IFACE_STD_POPUPS_TOP_MID_SPRITE@@YAXXZ 0x0044D0A0-0x0044D0AA BYTE_EXACT
// symbol    ?destroy_iface_std_popups_top_mid_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_mid_sprite() {
    g_IFACE_STD_POPUPS_TOP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_MID_SPRITE.
// ORIGINAL: 0x0044D0D0 ??__Fg_IFACE_STD_POPUPS_BOT_MID_SPRITE@@YAXXZ 0x0044D0D0-0x0044D0DA BYTE_EXACT
// symbol    ?destroy_iface_std_popups_bot_mid_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_mid_sprite() {
    g_IFACE_STD_POPUPS_BOT_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MID_LEFT_SPRITE.
// ORIGINAL: 0x0044D100 ??__Fg_IFACE_STD_POPUPS_MID_LEFT_SPRITE@@YAXXZ 0x0044D100-0x0044D10A BYTE_EXACT
// symbol    ?destroy_iface_std_popups_mid_left_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_mid_left_sprite() {
    g_IFACE_STD_POPUPS_MID_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE.
// ORIGINAL: 0x0044D130 ??__Fg_IFACE_STD_POPUPS_MID_RIGHT_SPRITE@@YAXXZ 0x0044D130-0x0044D13A BYTE_EXACT
// symbol    ?destroy_iface_std_popups_mid_right_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_mid_right_sprite() {
    g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MIDDLE_BUFFER.
// ORIGINAL: 0x0044D160 ??__Fg_IFACE_STD_POPUPS_MIDDLE_BUFFER@@YAXXZ 0x0044D160-0x0044D16A BYTE_EXACT
// symbol    ?destroy_iface_std_popups_middle_buffer@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_middle_buffer() {
    g_IFACE_STD_POPUPS_MIDDLE_BUFFER->Buffer::~Buffer();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR1.
// ORIGINAL: 0x0044D190 ??__Fg_UNUSED_CAVIARDATA_VAR1@@YAXXZ 0x0044D190-0x0044D19A BYTE_EXACT
// symbol    ?destroy_unused_caviardata_var1@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var1() {
    g_UNUSED_CAVIARDATA_VAR1->close();
}

/*
Purpose: Atexit teardown thunk for g_SSF_CAVIARDATA.
// ORIGINAL: 0x0044D1C0 ??__Fg_SSF_CAVIARDATA@@YAXXZ 0x0044D1C0-0x0044D1CA BYTE_EXACT
// symbol    ?destroy_ssf_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ssf_caviardata() {
    g_SSF_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SDP_CAVIARDATA.
// ORIGINAL: 0x0044D1F0 ??__Fg_SDP_CAVIARDATA@@YAXXZ 0x0044D1F0-0x0044D1FA BYTE_EXACT
// symbol    ?destroy_sdp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sdp_caviardata() {
    g_SDP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SAS_CAVIARDATA.
// ORIGINAL: 0x0044D220 ??__Fg_SAS_CAVIARDATA@@YAXXZ 0x0044D220-0x0044D22A BYTE_EXACT
// symbol    ?destroy_sas_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sas_caviardata() {
    g_SAS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SCD_CAVIARDATA.
// ORIGINAL: 0x0044D250 ??__Fg_SCD_CAVIARDATA@@YAXXZ 0x0044D250-0x0044D25A BYTE_EXACT
// symbol    ?destroy_scd_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scd_caviardata() {
    g_SCD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SCJ_CAVIARDATA.
// ORIGINAL: 0x0044D280 ??__Fg_SCJ_CAVIARDATA@@YAXXZ 0x0044D280-0x0044D28A BYTE_EXACT
// symbol    ?destroy_scj_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scj_caviardata() {
    g_SCJ_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SAGS_CAVIARDATA.
// ORIGINAL: 0x0044D2B0 ??__Fg_SAGS_CAVIARDATA@@YAXXZ 0x0044D2B0-0x0044D2BA BYTE_EXACT
// symbol    ?destroy_sags_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sags_caviardata() {
    g_SAGS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SFT_CAVIARDATA.
// ORIGINAL: 0x0044D2E0 ??__Fg_SFT_CAVIARDATA@@YAXXZ 0x0044D2E0-0x0044D2EA BYTE_EXACT
// symbol    ?destroy_sft_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sft_caviardata() {
    g_SFT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHR_CAVIARDATA1.
// ORIGINAL: 0x0044D310 ??__Fg_VHR_CAVIARDATA1@@YAXXZ 0x0044D310-0x0044D31A BYTE_EXACT
// symbol    ?destroy_vhr_caviardata1@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhr_caviardata1() {
    g_VHR_CAVIARDATA1->close();
}

/*
Purpose: Atexit teardown thunk for g_SHT_CAVIARDATA.
// ORIGINAL: 0x0044D340 ??__Fg_SHT_CAVIARDATA@@YAXXZ 0x0044D340-0x0044D34A BYTE_EXACT
// symbol    ?destroy_sht_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sht_caviardata() {
    g_SHT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SRB_CAVIARDATA.
// ORIGINAL: 0x0044D370 ??__Fg_SRB_CAVIARDATA@@YAXXZ 0x0044D370-0x0044D37A BYTE_EXACT
// symbol    ?destroy_srb_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_srb_caviardata() {
    g_SRB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ASAS_CAVIARDATA.
// ORIGINAL: 0x0044D3A0 ??__Fg_ASAS_CAVIARDATA@@YAXXZ 0x0044D3A0-0x0044D3AA BYTE_EXACT
// symbol    ?destroy_asas_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_asas_caviardata() {
    g_ASAS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_RESLASER_CAVIARDATA.
// ORIGINAL: 0x0044D3D0 ??__Fg_RESLASER_CAVIARDATA@@YAXXZ 0x0044D3D0-0x0044D3DA BYTE_EXACT
// symbol    ?destroy_reslaser_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reslaser_caviardata() {
    g_RESLASER_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_RESBOLT_CAVIARDATA.
// ORIGINAL: 0x0044D400 ??__Fg_RESBOLT_CAVIARDATA@@YAXXZ 0x0044D400-0x0044D40A BYTE_EXACT
// symbol    ?destroy_resbolt_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_resbolt_caviardata() {
    g_RESBOLT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_FUNLOAD_CAVIARDATA.
// ORIGINAL: 0x0044D430 ??__Fg_FUNLOAD_CAVIARDATA@@YAXXZ 0x0044D430-0x0044D43A BYTE_EXACT
// symbol    ?destroy_funload_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_funload_caviardata() {
    g_FUNLOAD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_TECLOAD_CAVIARDATA.
// ORIGINAL: 0x0044D460 ??__Fg_TECLOAD_CAVIARDATA@@YAXXZ 0x0044D460-0x0044D46A BYTE_EXACT
// symbol    ?destroy_tecload_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tecload_caviardata() {
    g_TECLOAD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_DISSWAVE_CAVIARDATA.
// ORIGINAL: 0x0044D490 ??__Fg_SP_DISSWAVE_CAVIARDATA@@YAXXZ 0x0044D490-0x0044D49A BYTE_EXACT
// symbol    ?destroy_sp_disswave_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_disswave_caviardata() {
    g_SP_DISSWAVE_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_MARINED_CAVIARDATA.
// ORIGINAL: 0x0044D4C0 ??__Fg_SP_MARINED_CAVIARDATA@@YAXXZ 0x0044D4C0-0x0044D4CA BYTE_EXACT
// symbol    ?destroy_sp_marined_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_marined_caviardata() {
    g_SP_MARINED_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_NANOO_CAVIARDATA.
// ORIGINAL: 0x0044D4F0 ??__Fg_SP_NANOO_CAVIARDATA@@YAXXZ 0x0044D4F0-0x0044D4FA BYTE_EXACT
// symbol    ?destroy_sp_nanoo_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_nanoo_caviardata() {
    g_SP_NANOO_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_SOPORIFIC_CAVIARDATA.
// ORIGINAL: 0x0044D520 ??__Fg_SP_SOPORIFIC_CAVIARDATA@@YAXXZ 0x0044D520-0x0044D52A BYTE_EXACT
// symbol    ?destroy_sp_soporific_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_soporific_caviardata() {
    g_SP_SOPORIFIC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA01_CAVIARDATA.
// ORIGINAL: 0x0044D550 ??__Fg_AA01_CAVIARDATA@@YAXXZ 0x0044D550-0x0044D55A BYTE_EXACT
// symbol    ?destroy_aa01_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa01_caviardata() {
    g_AA01_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_ROVER_CAVIARDATA.
// ORIGINAL: 0x0044D580 ??__Fg_AA_ROVER_CAVIARDATA@@YAXXZ 0x0044D580-0x0044D58A BYTE_EXACT
// symbol    ?destroy_aa_rover_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_rover_caviardata() {
    g_AA_ROVER_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_WING_CAVIARDATA.
// ORIGINAL: 0x0044D5C0 ??__Fg_AA_WING_CAVIARDATA@@YAXXZ 0x0044D5C0-0x0044D5D4
// symbol    ?destroy_aa_wing_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_wing_caviardata() {
    VectorDtorIterator(g_AA_WING_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ACP_CAVIARDATA.
// ORIGINAL: 0x0044D610 ??__Fg_ACP_CAVIARDATA@@YAXXZ 0x0044D610-0x0044D624
// symbol    ?destroy_acp_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_acp_caviardata() {
    VectorDtorIterator(g_ACP_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AX_CAVIARDATA.
// ORIGINAL: 0x0044D650 ??__Fg_AX_CAVIARDATA@@YAXXZ 0x0044D650-0x0044D65A BYTE_EXACT
// symbol    ?destroy_ax_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ax_caviardata() {
    g_AX_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_CAVIARDATA.
// ORIGINAL: 0x0044D680 ??__Fg_AA_CAVIARDATA@@YAXXZ 0x0044D680-0x0044D68A BYTE_EXACT
// symbol    ?destroy_aa_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_caviardata() {
    g_AA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ACOLPOD_CAVIARDATA.
// ORIGINAL: 0x0044D6B0 ??__Fg_ACOLPOD_CAVIARDATA@@YAXXZ 0x0044D6B0-0x0044D6BA BYTE_EXACT
// symbol    ?destroy_acolpod_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_acolpod_caviardata() {
    g_ACOLPOD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AT_CAVIARDATA.
// ORIGINAL: 0x0044D6E0 ??__Fg_AT_CAVIARDATA@@YAXXZ 0x0044D6E0-0x0044D6EA BYTE_EXACT
// symbol    ?destroy_at_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_at_caviardata() {
    g_AT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VTA_CAVIARDATA.
// ORIGINAL: 0x0044D710 ??__Fg_VTA_CAVIARDATA@@YAXXZ 0x0044D710-0x0044D71A BYTE_EXACT
// symbol    ?destroy_vta_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vta_caviardata() {
    g_VTA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ATP_CAVIARDATA.
// ORIGINAL: 0x0044D740 ??__Fg_ATP_CAVIARDATA@@YAXXZ 0x0044D740-0x0044D74A BYTE_EXACT
// symbol    ?destroy_atp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_atp_caviardata() {
    g_ATP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SSFA_CAVIARDATA.
// ORIGINAL: 0x0044D770 ??__Fg_SSFA_CAVIARDATA@@YAXXZ 0x0044D770-0x0044D77A BYTE_EXACT
// symbol    ?destroy_ssfa_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ssfa_caviardata() {
    g_SSFA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SFTA_CAVIARDATA.
// ORIGINAL: 0x0044D7A0 ??__Fg_SFTA_CAVIARDATA@@YAXXZ 0x0044D7A0-0x0044D7AA BYTE_EXACT
// symbol    ?destroy_sfta_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sfta_caviardata() {
    g_SFTA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VW00_CAVIARDATA.
// ORIGINAL: 0x0044D7D0 ??__Fg_VW00_CAVIARDATA@@YAXXZ 0x0044D7D0-0x0044D7DA BYTE_EXACT
// symbol    ?destroy_vw00_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vw00_caviardata() {
    g_VW00_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTAWL_CAVIARDATA.
// ORIGINAL: 0x0044D800 ??__Fg_VIPTAWL_CAVIARDATA@@YAXXZ 0x0044D800-0x0044D80A BYTE_EXACT
// symbol    ?destroy_viptawl_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptawl_caviardata() {
    g_VIPTAWL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTASGN_CAVIARDATA.
// ORIGINAL: 0x0044D830 ??__Fg_VIPTASGN_CAVIARDATA@@YAXXZ 0x0044D830-0x0044D83A BYTE_EXACT
// symbol    ?destroy_viptasgn_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptasgn_caviardata() {
    g_VIPTASGN_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTAPSI_CAVIARDATA.
// ORIGINAL: 0x0044D860 ??__Fg_VIPTAPSI_CAVIARDATA@@YAXXZ 0x0044D860-0x0044D86A BYTE_EXACT
// symbol    ?destroy_viptapsi_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptapsi_caviardata() {
    g_VIPTAPSI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTR_CAVIARDATA.
// ORIGINAL: 0x0044D8A0 ??__Fg_VIPTR_CAVIARDATA@@YAXXZ 0x0044D8A0-0x0044D8B4
// symbol    ?destroy_viptr_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptr_caviardata() {
    VectorDtorIterator(g_VIPTR_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VIPTA_CAVIARDATA.
// ORIGINAL: 0x0044D8F0 ??__Fg_VIPTA_CAVIARDATA@@YAXXZ 0x0044D8F0-0x0044D904
// symbol    ?destroy_vipta_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vipta_caviardata() {
    VectorDtorIterator(g_VIPTA_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_PTMOD_CAVIARDATA.
// ORIGINAL: 0x0044D930 ??__Fg_PTMOD_CAVIARDATA@@YAXXZ 0x0044D930-0x0044D93A BYTE_EXACT
// symbol    ?destroy_ptmod_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ptmod_caviardata() {
    g_PTMOD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VB_CAVIARDATA.
// ORIGINAL: 0x0044D960 ??__Fg_VB_CAVIARDATA@@YAXXZ 0x0044D960-0x0044D96A BYTE_EXACT
// symbol    ?destroy_vb_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vb_caviardata() {
    g_VB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VBP_CAVIARDATA.
// ORIGINAL: 0x0044D990 ??__Fg_VBP_CAVIARDATA@@YAXXZ 0x0044D990-0x0044D99A BYTE_EXACT
// symbol    ?destroy_vbp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vbp_caviardata() {
    g_VBP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMC_CAVIARDATA.
// ORIGINAL: 0x0044D9C0 ??__Fg_VGMC_CAVIARDATA@@YAXXZ 0x0044D9C0-0x0044D9CA BYTE_EXACT
// symbol    ?destroy_vgmc_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmc_caviardata() {
    g_VGMC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMCP_CAVIARDATA.
// ORIGINAL: 0x0044D9F0 ??__Fg_VGMCP_CAVIARDATA@@YAXXZ 0x0044D9F0-0x0044D9FA BYTE_EXACT
// symbol    ?destroy_vgmcp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmcp_caviardata() {
    g_VGMCP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VLIGHTS_CAVIARDATA.
// ORIGINAL: 0x0044DA20 ??__Fg_VLIGHTS_CAVIARDATA@@YAXXZ 0x0044DA20-0x0044DA2A BYTE_EXACT
// symbol    ?destroy_vlights_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vlights_caviardata() {
    g_VLIGHTS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VPT_CAVIARDATA.
// ORIGINAL: 0x0044DA50 ??__Fg_VPT_CAVIARDATA@@YAXXZ 0x0044DA50-0x0044DA5A BYTE_EXACT
// symbol    ?destroy_vpt_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vpt_caviardata() {
    g_VPT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_A_CAVIARDATA.
// ORIGINAL: 0x0044DA80 ??__Fg_A_CAVIARDATA@@YAXXZ 0x0044DA80-0x0044DA8A BYTE_EXACT
// symbol    ?destroy_a_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_a_caviardata() {
    g_A_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_APWALL_CAVIARDATA.
// ORIGINAL: 0x0044DAB0 ??__Fg_APWALL_CAVIARDATA@@YAXXZ 0x0044DAB0-0x0044DABA BYTE_EXACT
// symbol    ?destroy_apwall_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_apwall_caviardata() {
    g_APWALL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ASGEN_CAVIARDATA.
// ORIGINAL: 0x0044DAE0 ??__Fg_ASGEN_CAVIARDATA@@YAXXZ 0x0044DAE0-0x0044DAEA BYTE_EXACT
// symbol    ?destroy_asgen_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_asgen_caviardata() {
    g_ASGEN_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_APSID_CAVIARDATA.
// ORIGINAL: 0x0044DB10 ??__Fg_APSID_CAVIARDATA@@YAXXZ 0x0044DB10-0x0044DB1A BYTE_EXACT
// symbol    ?destroy_apsid_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_apsid_caviardata() {
    g_APSID_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VA01_CAVIARDATA.
// ORIGINAL: 0x0044DB40 ??__Fg_VA01_CAVIARDATA@@YAXXZ 0x0044DB40-0x0044DB4A BYTE_EXACT
// symbol    ?destroy_va01_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_va01_caviardata() {
    g_VA01_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VR_CAVIARDATA.
// ORIGINAL: 0x0044DB80 ??__Fg_VR_CAVIARDATA@@YAXXZ 0x0044DB80-0x0044DB94
// symbol    ?destroy_vr_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vr_caviardata() {
    VectorDtorIterator(g_VR_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VHR_CAVIARDATA2.
// ORIGINAL: 0x0044DBC0 ??__Fg_VHR_CAVIARDATA2@@YAXXZ 0x0044DBC0-0x0044DBCA BYTE_EXACT
// symbol    ?destroy_vhr_caviardata2@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhr_caviardata2() {
    g_VHR_CAVIARDATA2->close();
}

/*
Purpose: Atexit teardown thunk for g_VRC_CAVIARDATA.
// ORIGINAL: 0x0044DC00 ??__Fg_VRC_CAVIARDATA@@YAXXZ 0x0044DC00-0x0044DC14
// symbol    ?destroy_vrc_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vrc_caviardata() {
    VectorDtorIterator(g_VRC_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VI_CAVIARDATA.
// ORIGINAL: 0x0044DC40 ??__Fg_VI_CAVIARDATA@@YAXXZ 0x0044DC40-0x0044DC4A BYTE_EXACT
// symbol    ?destroy_vi_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vi_caviardata() {
    g_VI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMT_CAVIARDATA.
// ORIGINAL: 0x0044DC70 ??__Fg_VGMT_CAVIARDATA@@YAXXZ 0x0044DC70-0x0044DC7A BYTE_EXACT
// symbol    ?destroy_vgmt_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmt_caviardata() {
    g_VGMT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMTP_CAVIARDATA.
// ORIGINAL: 0x0044DCA0 ??__Fg_VGMTP_CAVIARDATA@@YAXXZ 0x0044DCA0-0x0044DCAA BYTE_EXACT
// symbol    ?destroy_vgmtp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmtp_caviardata() {
    g_VGMTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR2.
// ORIGINAL: 0x0044DCD0 ??__Fg_UNUSED_CAVIARDATA_VAR2@@YAXXZ 0x0044DCD0-0x0044DCDA BYTE_EXACT
// symbol    ?destroy_unused_caviardata_var2@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var2() {
    g_UNUSED_CAVIARDATA_VAR2->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNTU_CAVIARDATA.
// ORIGINAL: 0x0044DD00 ??__Fg_VWNTU_CAVIARDATA@@YAXXZ 0x0044DD00-0x0044DD0A BYTE_EXACT
// symbol    ?destroy_vwntu_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwntu_caviardata() {
    g_VWNTU_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VT_CAVIARDATA.
// ORIGINAL: 0x0044DD30 ??__Fg_VT_CAVIARDATA@@YAXXZ 0x0044DD30-0x0044DD3A BYTE_EXACT
// symbol    ?destroy_vt_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vt_caviardata() {
    g_VT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_DROP_CAVIARDATA.
// ORIGINAL: 0x0044DD60 ??__Fg_DROP_CAVIARDATA@@YAXXZ 0x0044DD60-0x0044DD6A BYTE_EXACT
// symbol    ?destroy_drop_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_drop_caviardata() {
    g_DROP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_DROPLET_CAVIARDATA.
// ORIGINAL: 0x0044DD90 ??__Fg_DROPLET_CAVIARDATA@@YAXXZ 0x0044DD90-0x0044DD9A BYTE_EXACT
// symbol    ?destroy_droplet_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_droplet_caviardata() {
    g_DROPLET_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCL_CAVIARDATA.
// ORIGINAL: 0x0044DDC0 ??__Fg_VCL_CAVIARDATA@@YAXXZ 0x0044DDC0-0x0044DDCA BYTE_EXACT
// symbol    ?destroy_vcl_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcl_caviardata() {
    g_VCL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR3.
// ORIGINAL: 0x0044DE00 ??__Fg_UNUSED_CAVIARDATA_VAR3@@YAXXZ 0x0044DE00-0x0044DE14
// symbol    ?destroy_unused_caviardata_var3@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var3() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR3, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCLT00_CAVIARDATA.
// ORIGINAL: 0x0044DE40 ??__Fg_VCLT00_CAVIARDATA@@YAXXZ 0x0044DE40-0x0044DE4A BYTE_EXACT
// symbol    ?destroy_vclt00_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vclt00_caviardata() {
    g_VCLT00_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHT_VBP_CAVIARDATA.
// ORIGINAL: 0x0044DE70 ??__Fg_VHT_VBP_CAVIARDATA@@YAXXZ 0x0044DE70-0x0044DE7A BYTE_EXACT
// symbol    ?destroy_vht_vbp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vht_vbp_caviardata() {
    g_VHT_VBP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTP_CAVIARDATA.
// ORIGINAL: 0x0044DEA0 ??__Fg_VHTP_CAVIARDATA@@YAXXZ 0x0044DEA0-0x0044DEAA BYTE_EXACT
// symbol    ?destroy_vhtp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhtp_caviardata() {
    g_VHTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTA0_CAVIARDATA.
// ORIGINAL: 0x0044DEE0 ??__Fg_VHTA0_CAVIARDATA@@YAXXZ 0x0044DEE0-0x0044DEF4
// symbol    ?destroy_vhta0_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhta0_caviardata() {
    VectorDtorIterator(g_VHTA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VHTTP_CAVIARDATA.
// ORIGINAL: 0x0044DF20 ??__Fg_VHTTP_CAVIARDATA@@YAXXZ 0x0044DF20-0x0044DF2A BYTE_EXACT
// symbol    ?destroy_vhttp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhttp_caviardata() {
    g_VHTTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTTPA0_CAVIARDATA.
// ORIGINAL: 0x0044DF60 ??__Fg_VHTTPA0_CAVIARDATA@@YAXXZ 0x0044DF60-0x0044DF74
// symbol    ?destroy_vhttpa0_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhttpa0_caviardata() {
    VectorDtorIterator(g_VHTTPA0_CAVIARDATA, 0xC, 3, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VSP_CAVIARDATA.
// ORIGINAL: 0x0044DFA0 ??__Fg_VSP_CAVIARDATA@@YAXXZ 0x0044DFA0-0x0044DFAA BYTE_EXACT
// symbol    ?destroy_vsp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsp_caviardata() {
    g_VSP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VSPA0_CAVIARDATA.
// ORIGINAL: 0x0044DFE0 ??__Fg_VSPA0_CAVIARDATA@@YAXXZ 0x0044DFE0-0x0044DFF4
// symbol    ?destroy_vspa0_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vspa0_caviardata() {
    VectorDtorIterator(g_VSPA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VSPTF_CAVIARDATA.
// ORIGINAL: 0x0044E020 ??__Fg_VSPTF_CAVIARDATA@@YAXXZ 0x0044E020-0x0044E02A BYTE_EXACT
// symbol    ?destroy_vsptf_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsptf_caviardata() {
    g_VSPTF_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VSPTB_CAVIARDATA.
// ORIGINAL: 0x0044E050 ??__Fg_VSPTB_CAVIARDATA@@YAXXZ 0x0044E050-0x0044E05A BYTE_EXACT
// symbol    ?destroy_vsptb_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsptb_caviardata() {
    g_VSPTB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VFL_CAVIARDATA.
// ORIGINAL: 0x0044E080 ??__Fg_VFL_CAVIARDATA@@YAXXZ 0x0044E080-0x0044E08A BYTE_EXACT
// symbol    ?destroy_vfl_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vfl_caviardata() {
    g_VFL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGS_CAVIARDATA.
// ORIGINAL: 0x0044E0B0 ??__Fg_VGS_CAVIARDATA@@YAXXZ 0x0044E0B0-0x0044E0BA BYTE_EXACT
// symbol    ?destroy_vgs_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgs_caviardata() {
    g_VGS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGSP_CAVIARDATA.
// ORIGINAL: 0x0044E0E0 ??__Fg_VGSP_CAVIARDATA@@YAXXZ 0x0044E0E0-0x0044E0EA BYTE_EXACT
// symbol    ?destroy_vgsp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgsp_caviardata() {
    g_VGSP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VJTP_CAVIARDATA.
// ORIGINAL: 0x0044E110 ??__Fg_VJTP_CAVIARDATA@@YAXXZ 0x0044E110-0x0044E11A BYTE_EXACT
// symbol    ?destroy_vjtp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vjtp_caviardata() {
    g_VJTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VJT0_CAVIARDATA.
// ORIGINAL: 0x0044E150 ??__Fg_VJT0_CAVIARDATA@@YAXXZ 0x0044E150-0x0044E164
// symbol    ?destroy_vjt0_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vjt0_caviardata() {
    VectorDtorIterator(g_VJT0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCU_CAVIARDATA.
// ORIGINAL: 0x0044E190 ??__Fg_VCU_CAVIARDATA@@YAXXZ 0x0044E190-0x0044E19A BYTE_EXACT
// symbol    ?destroy_vcu_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcu_caviardata() {
    g_VCU_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCUP_CAVIARDATA.
// ORIGINAL: 0x0044E1C0 ??__Fg_VCUP_CAVIARDATA@@YAXXZ 0x0044E1C0-0x0044E1CA BYTE_EXACT
// symbol    ?destroy_vcup_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcup_caviardata() {
    g_VCUP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCUA0_CAVIARDATA.
// ORIGINAL: 0x0044E200 ??__Fg_VCUA0_CAVIARDATA@@YAXXZ 0x0044E200-0x0044E214
// symbol    ?destroy_vcua0_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcua0_caviardata() {
    VectorDtorIterator(g_VCUA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCUW_CAVIARDATA.
// ORIGINAL: 0x0044E240 ??__Fg_VCUW_CAVIARDATA@@YAXXZ 0x0044E240-0x0044E24A BYTE_EXACT
// symbol    ?destroy_vcuw_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcuw_caviardata() {
    g_VCUW_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCT_CAVIARDATA.
// ORIGINAL: 0x0044E270 ??__Fg_VCT_CAVIARDATA@@YAXXZ 0x0044E270-0x0044E27A BYTE_EXACT
// symbol    ?destroy_vct_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vct_caviardata() {
    g_VCT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCTP_CAVIARDATA.
// ORIGINAL: 0x0044E2A0 ??__Fg_VCTP_CAVIARDATA@@YAXXZ 0x0044E2A0-0x0044E2AA BYTE_EXACT
// symbol    ?destroy_vctp_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vctp_caviardata() {
    g_VCTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCTB_CAVIARDATA.
// ORIGINAL: 0x0044E2D0 ??__Fg_VCTB_CAVIARDATA@@YAXXZ 0x0044E2D0-0x0044E2DA BYTE_EXACT
// symbol    ?destroy_vctb_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vctb_caviardata() {
    g_VCTB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCT0_CAVIARDATA.
// ORIGINAL: 0x0044E310 ??__Fg_VCT0_CAVIARDATA@@YAXXZ 0x0044E310-0x0044E324
// symbol    ?destroy_vct0_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vct0_caviardata() {
    VectorDtorIterator(g_VCT0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VWNTT_CAVIARDATA.
// ORIGINAL: 0x0044E350 ??__Fg_VWNTT_CAVIARDATA@@YAXXZ 0x0044E350-0x0044E35A BYTE_EXACT
// symbol    ?destroy_vwntt_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwntt_caviardata() {
    g_VWNTT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNST_CAVIARDATA.
// ORIGINAL: 0x0044E380 ??__Fg_VWNST_CAVIARDATA@@YAXXZ 0x0044E380-0x0044E38A BYTE_EXACT
// symbol    ?destroy_vwnst_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwnst_caviardata() {
    g_VWNST_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNAA_CAVIARDATA.
// ORIGINAL: 0x0044E3B0 ??__Fg_VWNAA_CAVIARDATA@@YAXXZ 0x0044E3B0-0x0044E3BA BYTE_EXACT
// symbol    ?destroy_vwnaa_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwnaa_caviardata() {
    g_VWNAA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VW_CAVIARDATA.
// ORIGINAL: 0x0044E3F0 ??__Fg_VW_CAVIARDATA@@YAXXZ 0x0044E3F0-0x0044E404
// symbol    ?destroy_vw_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vw_caviardata() {
    VectorDtorIterator(g_VW_CAVIARDATA, 0xC, 16, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VM_CAVIARDATA.
// ORIGINAL: 0x0044E430 ??__Fg_VM_CAVIARDATA@@YAXXZ 0x0044E430-0x0044E43A BYTE_EXACT
// symbol    ?destroy_vm_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vm_caviardata() {
    g_VM_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VM13_CAVIARDATA.
// ORIGINAL: 0x0044E460 ??__Fg_VM13_CAVIARDATA@@YAXXZ 0x0044E460-0x0044E46A BYTE_EXACT
// symbol    ?destroy_vm13_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vm13_caviardata() {
    g_VM13_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VPBR0_CAVIARDATA.
// ORIGINAL: 0x0044E4A0 ??__Fg_VPBR0_CAVIARDATA@@YAXXZ 0x0044E4A0-0x0044E4B4
// symbol    ?destroy_vpbr0_caviardata@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vpbr0_caviardata() {
    VectorDtorIterator(g_VPBR0_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR4.
// ORIGINAL: 0x0044E4F0 ??__Fg_UNUSED_CAVIARDATA_VAR4@@YAXXZ 0x0044E4F0-0x0044E504
// symbol    ?destroy_unused_caviardata_var4@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var4() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR4, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_NW_CAVIARDATA.
// ORIGINAL: 0x0044E530 ??__Fg_NW_CAVIARDATA@@YAXXZ 0x0044E530-0x0044E53A BYTE_EXACT
// symbol    ?destroy_nw_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_nw_caviardata() {
    g_NW_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_NI_CAVIARDATA.
// ORIGINAL: 0x0044E560 ??__Fg_NI_CAVIARDATA@@YAXXZ 0x0044E560-0x0044E56A BYTE_EXACT
// symbol    ?destroy_ni_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ni_caviardata() {
    g_NI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_NLC_CAVIARDATA.
// ORIGINAL: 0x0044E590 ??__Fg_NLC_CAVIARDATA@@YAXXZ 0x0044E590-0x0044E59A BYTE_EXACT
// symbol    ?destroy_nlc_caviardata@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_nlc_caviardata() {
    g_NLC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR5.
// ORIGINAL: 0x0044E5D0 ??__Fg_UNUSED_CAVIARDATA_VAR5@@YAXXZ 0x0044E5D0-0x0044E5E4
// symbol    ?destroy_unused_caviardata_var5@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var5() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR5, 0xC, 5, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RADIUS1_TEXTURE.
// ORIGINAL: 0x0044E610 ??__Fg_RADIUS1_TEXTURE@@YAXXZ 0x0044E610-0x0044E61A BYTE_EXACT
// symbol    ?destroy_radius1_texture@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_radius1_texture() {
    g_RADIUS1_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_RADIUS2_TEXTURE.
// ORIGINAL: 0x0044E640 ??__Fg_RADIUS2_TEXTURE@@YAXXZ 0x0044E640-0x0044E64A BYTE_EXACT
// symbol    ?destroy_radius2_texture@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_radius2_texture() {
    g_RADIUS2_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_ROCKY_TEXTURES.
// ORIGINAL: 0x0044E680 ??__Fg_ROCKY_TEXTURES@@YAXXZ 0x0044E680-0x0044E694
// symbol    ?destroy_rocky_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rocky_textures() {
    VectorDtorIterator(g_ROCKY_TEXTURES, 0x70, 4, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_OCEAN_TEXTURES.
// ORIGINAL: 0x0044E6D0 ??__Fg_OCEAN_TEXTURES@@YAXXZ 0x0044E6D0-0x0044E6E4
// symbol    ?destroy_ocean_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ocean_textures() {
    VectorDtorIterator(g_OCEAN_TEXTURES, 0x70, 2, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FLAT_ARID_LAND_TEXTURE.
// ORIGINAL: 0x0044E710 ??__Fg_FLAT_ARID_LAND_TEXTURE@@YAXXZ 0x0044E710-0x0044E71A BYTE_EXACT
// symbol    ?destroy_flat_arid_land_texture@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_flat_arid_land_texture() {
    g_FLAT_ARID_LAND_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_MOIST_LAND_TEXTURES.
// ORIGINAL: 0x0044E750 ??__Fg_MOIST_LAND_TEXTURES@@YAXXZ 0x0044E750-0x0044E764
// symbol    ?destroy_moist_land_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_moist_land_textures() {
    VectorDtorIterator(g_MOIST_LAND_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINY_LAND_TEXTURES.
// ORIGINAL: 0x0044E7A0 ??__Fg_RAINY_LAND_TEXTURES@@YAXXZ 0x0044E7A0-0x0044E7B4
// symbol    ?destroy_rainy_land_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainy_land_textures() {
    VectorDtorIterator(g_RAINY_LAND_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_JUNGLE_LAND_TEXTURES.
// ORIGINAL: 0x0044E7F0 ??__Fg_JUNGLE_LAND_TEXTURES@@YAXXZ 0x0044E7F0-0x0044E804
// symbol    ?destroy_jungle_land_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_jungle_land_textures() {
    VectorDtorIterator(g_JUNGLE_LAND_TEXTURES, 0x70, 15, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_DUNE_LAND_TEXTURE.
// ORIGINAL: 0x0044E830 ??__Fg_DUNE_LAND_TEXTURE@@YAXXZ 0x0044E830-0x0044E83A BYTE_EXACT
// symbol    ?destroy_dune_land_texture@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_dune_land_texture() {
    g_DUNE_LAND_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_SUNNY_MESA_TEXTURES.
// ORIGINAL: 0x0044E870 ??__Fg_SUNNY_MESA_TEXTURES@@YAXXZ 0x0044E870-0x0044E884
// symbol    ?destroy_sunny_mesa_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sunny_mesa_textures() {
    VectorDtorIterator(g_SUNNY_MESA_TEXTURES, 0x70, 8, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_SINGLE_TILE_TEXTURES.
// ORIGINAL: 0x0044E8C0 ??__Fg_RAINFALL_SINGLE_TILE_TEXTURES@@YAXXZ 0x0044E8C0-0x0044E8D4
// symbol    ?destroy_rainfall_single_tile_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_single_tile_textures() {
    VectorDtorIterator(g_RAINFALL_SINGLE_TILE_TEXTURES, 0x70, 2, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ROAD_TEXTURES.
// ORIGINAL: 0x0044E910 ??__Fg_ROAD_TEXTURES@@YAXXZ 0x0044E910-0x0044E924
// symbol    ?destroy_road_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_road_textures() {
    VectorDtorIterator(g_ROAD_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MAGTUBE_TEXTURES.
// ORIGINAL: 0x0044E960 ??__Fg_MAGTUBE_TEXTURES@@YAXXZ 0x0044E960-0x0044E974
// symbol    ?destroy_magtube_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_magtube_textures() {
    VectorDtorIterator(g_MAGTUBE_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RIVER_TEXTURES.
// ORIGINAL: 0x0044E9B0 ??__Fg_RIVER_TEXTURES@@YAXXZ 0x0044E9B0-0x0044E9C4
// symbol    ?destroy_river_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_river_textures() {
    VectorDtorIterator(g_RIVER_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MOUNT_PLANET_TEXTURES.
// ORIGINAL: 0x0044EA00 ??__Fg_MOUNT_PLANET_TEXTURES@@YAXXZ 0x0044EA00-0x0044EA14
// symbol    ?destroy_mount_planet_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_mount_planet_textures() {
    VectorDtorIterator(g_MOUNT_PLANET_TEXTURES, 0x70, 3, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_GARLAND_CRATER_TEXTURES.
// ORIGINAL: 0x0044EA50 ??__Fg_GARLAND_CRATER_TEXTURES@@YAXXZ 0x0044EA50-0x0044EA64
// symbol    ?destroy_garland_crater_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_garland_crater_textures() {
    VectorDtorIterator(g_GARLAND_CRATER_TEXTURES, 0x70, 3, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FUNGUS_TEXTURES.
// ORIGINAL: 0x0044EAA0 ??__Fg_FUNGUS_TEXTURES@@YAXXZ 0x0044EAA0-0x0044EAB4
// symbol    ?destroy_fungus_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fungus_textures() {
    VectorDtorIterator(g_FUNGUS_TEXTURES, 0x70, 30, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FARM_TEXTURES.
// ORIGINAL: 0x0044EAF0 ??__Fg_FARM_TEXTURES@@YAXXZ 0x0044EAF0-0x0044EB04
// symbol    ?destroy_farm_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_farm_textures() {
    VectorDtorIterator(g_FARM_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FOREST_TEXTURES.
// ORIGINAL: 0x0044EB40 ??__Fg_FOREST_TEXTURES@@YAXXZ 0x0044EB40-0x0044EB54
// symbol    ?destroy_forest_textures@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_forest_textures() {
    VectorDtorIterator(g_FOREST_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_SINGLE_TILE_TEXTURE.
// ORIGINAL: 0x0044EB80 ??__Fg_RAINFALL_SINGLE_TILE_TEXTURE@@YAXXZ 0x0044EB80-0x0044EB8A BYTE_EXACT
// symbol    ?destroy_rainfall_single_tile_texture@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_single_tile_texture() {
    g_RAINFALL_SINGLE_TILE_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_TER1_WHITE_ORG_YEL_TILE_SPRITES.
// ORIGINAL: 0x0044EBC0 ??__Fg_TER1_WHITE_ORG_YEL_TILE_SPRITES@@YAXXZ 0x0044EBC0-0x0044EBD4
// symbol    ?destroy_ter1_white_org_yel_tile_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_white_org_yel_tile_sprites() {
    VectorDtorIterator(g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOTTOM_LEFT_TILE_SPRITES.
// ORIGINAL: 0x0044EC10 ??__Fg_TER1_BOTTOM_LEFT_TILE_SPRITES@@YAXXZ 0x0044EC10-0x0044EC24
// symbol    ?destroy_ter1_bottom_left_tile_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_bottom_left_tile_sprites() {
    VectorDtorIterator(g_TER1_BOTTOM_LEFT_TILE_SPRITES, 0x2C, 9, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNUSED_SPRITES2.
// ORIGINAL: 0x0044EC60 ??__Fg_TER1_UNUSED_SPRITES2@@YAXXZ 0x0044EC60-0x0044EC74
// symbol    ?destroy_ter1_unused_sprites2@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unused_sprites2() {
    VectorDtorIterator(g_TER1_UNUSED_SPRITES2, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_MINE_SPRITE.
// ORIGINAL: 0x0044ECA0 ??__Fg_TER1_MINE_SPRITE@@YAXXZ 0x0044ECA0-0x0044ECAA BYTE_EXACT
// symbol    ?destroy_ter1_mine_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_mine_sprite() {
    g_TER1_MINE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_SOLAR_COLLECTOR_SPRITE.
// ORIGINAL: 0x0044ECD0 ??__Fg_TER1_SOLAR_COLLECTOR_SPRITE@@YAXXZ 0x0044ECD0-0x0044ECDA BYTE_EXACT
// symbol    ?destroy_ter1_solar_collector_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_solar_collector_sprite() {
    g_TER1_SOLAR_COLLECTOR_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_TIDAL_HARNESS_SPRITE.
// ORIGINAL: 0x0044ED00 ??__Fg_TER1_TIDAL_HARNESS_SPRITE@@YAXXZ 0x0044ED00-0x0044ED0A BYTE_EXACT
// symbol    ?destroy_ter1_tidal_harness_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_tidal_harness_sprite() {
    g_TER1_TIDAL_HARNESS_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_MINING_PLATFORM_SPRITE.
// ORIGINAL: 0x0044ED30 ??__Fg_TER1_MINING_PLATFORM_SPRITE@@YAXXZ 0x0044ED30-0x0044ED3A BYTE_EXACT
// symbol    ?destroy_ter1_mining_platform_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_mining_platform_sprite() {
    g_TER1_MINING_PLATFORM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_TUT_BLANK_SPRITE.
// ORIGINAL: 0x0044ED60 ??__Fg_TER1_TUT_BLANK_SPRITE@@YAXXZ 0x0044ED60-0x0044ED6A BYTE_EXACT
// symbol    ?destroy_ter1_tut_blank_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_tut_blank_sprite() {
    g_TER1_TUT_BLANK_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_KELP_FARM_SPRITE.
// ORIGINAL: 0x0044ED90 ??__Fg_TER1_KELP_FARM_SPRITE@@YAXXZ 0x0044ED90-0x0044ED9A BYTE_EXACT
// symbol    ?destroy_ter1_kelp_farm_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_kelp_farm_sprite() {
    g_TER1_KELP_FARM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_CONDENSER_SPRITE.
// ORIGINAL: 0x0044EDC0 ??__Fg_TER1_CONDENSER_SPRITE@@YAXXZ 0x0044EDC0-0x0044EDCA BYTE_EXACT
// symbol    ?destroy_ter1_condenser_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_condenser_sprite() {
    g_TER1_CONDENSER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_ECHELON_MIRROR_SPRITE.
// ORIGINAL: 0x0044EDF0 ??__Fg_TER1_ECHELON_MIRROR_SPRITE@@YAXXZ 0x0044EDF0-0x0044EDFA BYTE_EXACT
// symbol    ?destroy_ter1_echelon_mirror_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_echelon_mirror_sprite() {
    g_TER1_ECHELON_MIRROR_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOREHOLE_SPRITE.
// ORIGINAL: 0x0044EE20 ??__Fg_TER1_BOREHOLE_SPRITE@@YAXXZ 0x0044EE20-0x0044EE2A BYTE_EXACT
// symbol    ?destroy_ter1_borehole_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_borehole_sprite() {
    g_TER1_BOREHOLE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOREHOLE_CLUSTER_SPRITE.
// ORIGINAL: 0x0044EE50 ??__Fg_TER1_BOREHOLE_CLUSTER_SPRITE@@YAXXZ 0x0044EE50-0x0044EE5A BYTE_EXACT
// symbol    ?destroy_ter1_borehole_cluster_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_borehole_cluster_sprite() {
    g_TER1_BOREHOLE_CLUSTER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_MANIFOLD_NEXUS_SPRITES.
// ORIGINAL: 0x0044EE90 ??__Fg_TER1_MANIFOLD_NEXUS_SPRITES@@YAXXZ 0x0044EE90-0x0044EEA4
// symbol    ?destroy_ter1_manifold_nexus_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_manifold_nexus_sprites() {
    VectorDtorIterator(g_TER1_MANIFOLD_NEXUS_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1WRECK_UNITY_WRECKAGE_SPRITES.
// ORIGINAL: 0x0044EEE0 ??__Fg_TER1WRECK_UNITY_WRECKAGE_SPRITES@@YAXXZ 0x0044EEE0-0x0044EEF4
// symbol    ?destroy_ter1wreck_unity_wreckage_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1wreck_unity_wreckage_sprites() {
    VectorDtorIterator(g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 0x2C, 15, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES.
// ORIGINAL: 0x0044EF30 ??__Fg_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES@@YAXXZ 0x0044EF30-0x0044EF44
// symbol    ?destroy_ter1wreck_unity_wreckage_alt_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1wreck_unity_wreckage_alt_sprites() {
    VectorDtorIterator(g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FOSSIL_FIELD_RIDGE_SPRITES.
// ORIGINAL: 0x0044EF80 ??__Fg_FOSSIL_FIELD_RIDGE_SPRITES@@YAXXZ 0x0044EF80-0x0044EF94
// symbol    ?destroy_fossil_field_ridge_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fossil_field_ridge_sprites() {
    VectorDtorIterator(g_FOSSIL_FIELD_RIDGE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNUSED_SPRITES1.
// ORIGINAL: 0x0044EFD0 ??__Fg_TER1_UNUSED_SPRITES1@@YAXXZ 0x0044EFD0-0x0044EFE4
// symbol    ?destroy_ter1_unused_sprites1@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unused_sprites1() {
    VectorDtorIterator(g_TER1_UNUSED_SPRITES1, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_FARM_SPRITES.
// ORIGINAL: 0x0044F020 ??__Fg_TER1_FARM_SPRITES@@YAXXZ 0x0044F020-0x0044F034
// symbol    ?destroy_ter1_farm_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_farm_sprites() {
    VectorDtorIterator(g_TER1_FARM_SPRITES, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_SOIL_ENRICHER_SPRITES.
// ORIGINAL: 0x0044F070 ??__Fg_TER1_SOIL_ENRICHER_SPRITES@@YAXXZ 0x0044F070-0x0044F084
// symbol    ?destroy_ter1_soil_enricher_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_soil_enricher_sprites() {
    VectorDtorIterator(g_TER1_SOIL_ENRICHER_SPRITES, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_SEA_LAND_RESOURCE_SPRITES.
// ORIGINAL: 0x0044F0C0 ??__Fg_TER1_SEA_LAND_RESOURCE_SPRITES@@YAXXZ 0x0044F0C0-0x0044F0D4
// symbol    ?destroy_ter1_sea_land_resource_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_sea_land_resource_sprites() {
    VectorDtorIterator(g_TER1_SEA_LAND_RESOURCE_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_LANDMARK_RESOURCE_SPRITES.
// ORIGINAL: 0x0044F110 ??__Fg_TER1_LANDMARK_RESOURCE_SPRITES@@YAXXZ 0x0044F110-0x0044F124
// symbol    ?destroy_ter1_landmark_resource_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_landmark_resource_sprites() {
    VectorDtorIterator(g_TER1_LANDMARK_RESOURCE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_GLOW_SPRITES.
// ORIGINAL: 0x0044F160 ??__Fg_GLOW_SPRITES@@YAXXZ 0x0044F160-0x0044F174
// symbol    ?destroy_glow_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_glow_sprites() {
    VectorDtorIterator(g_GLOW_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNITY_POD_SPRITES.
// ORIGINAL: 0x0044F1B0 ??__Fg_TER1_UNITY_POD_SPRITES@@YAXXZ 0x0044F1B0-0x0044F1C4
// symbol    ?destroy_ter1_unity_pod_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unity_pod_sprites() {
    VectorDtorIterator(g_TER1_UNITY_POD_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_MONOLITH_SPRITE.
// ORIGINAL: 0x0044F1F0 ??__Fg_TER1_MONOLITH_SPRITE@@YAXXZ 0x0044F1F0-0x0044F1FA BYTE_EXACT
// symbol    ?destroy_ter1_monolith_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_monolith_sprite() {
    g_TER1_MONOLITH_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BUNKER_SPRITE.
// ORIGINAL: 0x0044F220 ??__Fg_TER1_BUNKER_SPRITE@@YAXXZ 0x0044F220-0x0044F22A BYTE_EXACT
// symbol    ?destroy_ter1_bunker_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_bunker_sprite() {
    g_TER1_BUNKER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_AIRBASE_SPRITE.
// ORIGINAL: 0x0044F250 ??__Fg_TER1_AIRBASE_SPRITE@@YAXXZ 0x0044F250-0x0044F25A BYTE_EXACT
// symbol    ?destroy_ter1_airbase_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_airbase_sprite() {
    g_TER1_AIRBASE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_SENSOR_ARRAY_SPRITE.
// ORIGINAL: 0x0044F280 ??__Fg_TER1_SENSOR_ARRAY_SPRITE@@YAXXZ 0x0044F280-0x0044F28A BYTE_EXACT
// symbol    ?destroy_ter1_sensor_array_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_sensor_array_sprite() {
    g_TER1_SENSOR_ARRAY_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_DOUBLE_TILE_SPRITES.
// ORIGINAL: 0x0044F2C0 ??__Fg_RAINFALL_DOUBLE_TILE_SPRITES@@YAXXZ 0x0044F2C0-0x0044F2D4
// symbol    ?destroy_rainfall_double_tile_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_double_tile_sprites() {
    VectorDtorIterator(g_RAINFALL_DOUBLE_TILE_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VEH_SPRITES.
// ORIGINAL: 0x0044F310 ??__Fg_VEH_SPRITES@@YAXXZ 0x0044F310-0x0044F327
// symbol    ?destroy_veh_sprites@@YAXXZ
// size      23 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_veh_sprites() {
    VectorDtorIterator(g_VEH_SPRITES, 0x2C, 152, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FLAGS_VEH_SPRITES.
// ORIGINAL: 0x0044F360 ??__Fg_FLAGS_VEH_SPRITES@@YAXXZ 0x0044F360-0x0044F374
// symbol    ?destroy_flags_veh_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_flags_veh_sprites() {
    VectorDtorIterator(g_FLAGS_VEH_SPRITES, 0x2C, 112, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ICONS_GENERAL_SPRITES.
// ORIGINAL: 0x0044F3B0 ??__Fg_ICONS_GENERAL_SPRITES@@YAXXZ 0x0044F3B0-0x0044F3C4
// symbol    ?destroy_icons_general_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_icons_general_sprites() {
    VectorDtorIterator(g_ICONS_GENERAL_SPRITES, 0x2C, 16, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RESOURCE_ICON_SPRITES.
// ORIGINAL: 0x0044F400 ??__Fg_RESOURCE_ICON_SPRITES@@YAXXZ 0x0044F400-0x0044F414
// symbol    ?destroy_resource_icon_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_resource_icon_sprites() {
    VectorDtorIterator(g_RESOURCE_ICON_SPRITES, 0x2C, 32, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CITIZEN_LG_CURSOR_SPRITES.
// ORIGINAL: 0x0044F450 ??__Fg_CITIZEN_LG_CURSOR_SPRITES@@YAXXZ 0x0044F450-0x0044F464
// symbol    ?destroy_citizen_lg_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_citizen_lg_cursor_sprites() {
    VectorDtorIterator(g_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SPECIALIST_LG_CURSOR_SPRITES.
// ORIGINAL: 0x0044F4A0 ??__Fg_SPECIALIST_LG_CURSOR_SPRITES@@YAXXZ 0x0044F4A0-0x0044F4B4
// symbol    ?destroy_specialist_lg_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_specialist_lg_cursor_sprites() {
    VectorDtorIterator(g_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CITIZEN_SM_CURSOR_SPRITES.
// ORIGINAL: 0x0044F4F0 ??__Fg_CITIZEN_SM_CURSOR_SPRITES@@YAXXZ 0x0044F4F0-0x0044F504
// symbol    ?destroy_citizen_sm_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_citizen_sm_cursor_sprites() {
    VectorDtorIterator(g_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SPECIALIST_SM_CURSOR_SPRITES.
// ORIGINAL: 0x0044F540 ??__Fg_SPECIALIST_SM_CURSOR_SPRITES@@YAXXZ 0x0044F540-0x0044F554
// symbol    ?destroy_specialist_sm_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_specialist_sm_cursor_sprites() {
    VectorDtorIterator(g_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_CITIZEN_LG_CURSOR_SPRITES.
// ORIGINAL: 0x0044F590 ??__Fg_AL_CITIZEN_LG_CURSOR_SPRITES@@YAXXZ 0x0044F590-0x0044F5A4
// symbol    ?destroy_al_citizen_lg_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_citizen_lg_cursor_sprites() {
    VectorDtorIterator(g_AL_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_SPECIALIST_LG_CURSOR_SPRITES.
// ORIGINAL: 0x0044F5E0 ??__Fg_AL_SPECIALIST_LG_CURSOR_SPRITES@@YAXXZ 0x0044F5E0-0x0044F5F4
// symbol    ?destroy_al_specialist_lg_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_specialist_lg_cursor_sprites() {
    VectorDtorIterator(g_AL_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_CITIZEN_SM_CURSOR_SPRITES.
// ORIGINAL: 0x0044F630 ??__Fg_AL_CITIZEN_SM_CURSOR_SPRITES@@YAXXZ 0x0044F630-0x0044F644
// symbol    ?destroy_al_citizen_sm_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_citizen_sm_cursor_sprites() {
    VectorDtorIterator(g_AL_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_SPECIALIST_SM_CURSOR_SPRITES.
// ORIGINAL: 0x0044F680 ??__Fg_AL_SPECIALIST_SM_CURSOR_SPRITES@@YAXXZ 0x0044F680-0x0044F694
// symbol    ?destroy_al_specialist_sm_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_specialist_sm_cursor_sprites() {
    VectorDtorIterator(g_AL_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RED_ALIEN_HEAD_ICON_SPRITE.
// ORIGINAL: 0x0044F6C0 ??__Fg_RED_ALIEN_HEAD_ICON_SPRITE@@YAXXZ 0x0044F6C0-0x0044F6CA BYTE_EXACT
// symbol    ?destroy_red_alien_head_icon_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_red_alien_head_icon_sprite() {
    g_RED_ALIEN_HEAD_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_SILVER_MENU_ICON_SPRITES.
// ORIGINAL: 0x0044F700 ??__Fg_SILVER_MENU_ICON_SPRITES@@YAXXZ 0x0044F700-0x0044F714
// symbol    ?destroy_silver_menu_icon_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_silver_menu_icon_sprites() {
    VectorDtorIterator(g_SILVER_MENU_ICON_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SILVER_CHECKBOX_ICON_SPRITES.
// ORIGINAL: 0x0044F750 ??__Fg_SILVER_CHECKBOX_ICON_SPRITES@@YAXXZ 0x0044F750-0x0044F764
// symbol    ?destroy_silver_checkbox_icon_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_silver_checkbox_icon_sprites() {
    VectorDtorIterator(g_SILVER_CHECKBOX_ICON_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RED_MALE_HEAD_ICON_SPRITE.
// ORIGINAL: 0x0044F790 ??__Fg_RED_MALE_HEAD_ICON_SPRITE@@YAXXZ 0x0044F790-0x0044F79A BYTE_EXACT
// symbol    ?destroy_red_male_head_icon_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_red_male_head_icon_sprite() {
    g_RED_MALE_HEAD_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_NULL_RESOURCE_ICON_SPRITE.
// ORIGINAL: 0x0044F7C0 ??__Fg_NULL_RESOURCE_ICON_SPRITE@@YAXXZ 0x0044F7C0-0x0044F7CA BYTE_EXACT
// symbol    ?destroy_null_resource_icon_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_null_resource_icon_sprite() {
    g_NULL_RESOURCE_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_PEACE_SIGN_SPRITES.
// ORIGINAL: 0x0044F800 ??__Fg_PEACE_SIGN_SPRITES@@YAXXZ 0x0044F800-0x0044F814
// symbol    ?destroy_peace_sign_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_peace_sign_sprites() {
    VectorDtorIterator(g_PEACE_SIGN_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ICON_TILE_SQUARE_SPRITE.
// ORIGINAL: 0x0044F840 ??__Fg_ICON_TILE_SQUARE_SPRITE@@YAXXZ 0x0044F840-0x0044F84A BYTE_EXACT
// symbol    ?destroy_icon_tile_square_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_icon_tile_square_sprite() {
    g_ICON_TILE_SQUARE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_XI_BOOM_VEH_SPRITES.
// ORIGINAL: 0x0044F880 ??__Fg_XI_BOOM_VEH_SPRITES@@YAXXZ 0x0044F880-0x0044F897
// symbol    ?destroy_xi_boom_veh_sprites@@YAXXZ
// size      23 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_xi_boom_veh_sprites() {
    VectorDtorIterator(g_XI_BOOM_VEH_SPRITES, 0x2C, 144, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_XF_BOOM_VEH_SPRITES.
// ORIGINAL: 0x0044F8D0 ??__Fg_XF_BOOM_VEH_SPRITES@@YAXXZ 0x0044F8D0-0x0044F8E4
// symbol    ?destroy_xf_boom_veh_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_xf_boom_veh_sprites() {
    VectorDtorIterator(g_XF_BOOM_VEH_SPRITES, 0x2C, 64, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_MIND_WORM_SPRITE.
// ORIGINAL: 0x0044F910 ??__Fg_BATTLE_MIND_WORM_SPRITE@@YAXXZ 0x0044F910-0x0044F91A BYTE_EXACT
// symbol    ?destroy_battle_mind_worm_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_mind_worm_sprite() {
    g_BATTLE_MIND_WORM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_ISLE_DEEP_SPRITE.
// ORIGINAL: 0x0044F940 ??__Fg_BATTLE_ISLE_DEEP_SPRITE@@YAXXZ 0x0044F940-0x0044F94A BYTE_EXACT
// symbol    ?destroy_battle_isle_deep_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_isle_deep_sprite() {
    g_BATTLE_ISLE_DEEP_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_LOCUSTS_CHIRON_SPRITE.
// ORIGINAL: 0x0044F970 ??__Fg_BATTLE_LOCUSTS_CHIRON_SPRITE@@YAXXZ 0x0044F970-0x0044F97A BYTE_EXACT
// symbol    ?destroy_battle_locusts_chiron_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_locusts_chiron_sprite() {
    g_BATTLE_LOCUSTS_CHIRON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_FUNGAL_TOWER_SPRITE.
// ORIGINAL: 0x0044F9A0 ??__Fg_BATTLE_FUNGAL_TOWER_SPRITE@@YAXXZ 0x0044F9A0-0x0044F9AA BYTE_EXACT
// symbol    ?destroy_battle_fungal_tower_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_fungal_tower_sprite() {
    g_BATTLE_FUNGAL_TOWER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_SPORE_LAUNCHER_SPRITE.
// ORIGINAL: 0x0044F9D0 ??__Fg_BATTLE_SPORE_LAUNCHER_SPRITE@@YAXXZ 0x0044F9D0-0x0044F9DA BYTE_EXACT
// symbol    ?destroy_battle_spore_launcher_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_spore_launcher_sprite() {
    g_BATTLE_SPORE_LAUNCHER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_SEALURK_SPRITE.
// ORIGINAL: 0x0044FA00 ??__Fg_BATTLE_SEALURK_SPRITE@@YAXXZ 0x0044FA00-0x0044FA0A BYTE_EXACT
// symbol    ?destroy_battle_sealurk_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_sealurk_sprite() {
    g_BATTLE_SEALURK_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TECH_ICON_SPRITES.
// ORIGINAL: 0x0044FA40 ??__Fg_TECH_ICON_SPRITES@@YAXXZ 0x0044FA40-0x0044FA54
// symbol    ?destroy_tech_icon_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tech_icon_sprites() {
    VectorDtorIterator(g_TECH_ICON_SPRITES, 0x2C, 89, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FACILITY_ICON_SPRITES.
// ORIGINAL: 0x0044FA90 ??__Fg_FACILITY_ICON_SPRITES@@YAXXZ 0x0044FA90-0x0044FAA4
// symbol    ?destroy_facility_icon_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_facility_icon_sprites() {
    VectorDtorIterator(g_FACILITY_ICON_SPRITES, 0x2C, 70, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SECRET_PROJECT_ICON_SPRITES.
// ORIGINAL: 0x0044FAE0 ??__Fg_SECRET_PROJECT_ICON_SPRITES@@YAXXZ 0x0044FAE0-0x0044FAF4
// symbol    ?destroy_secret_project_icon_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_secret_project_icon_sprites() {
    VectorDtorIterator(g_SECRET_PROJECT_ICON_SPRITES, 0x2C, 64, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_MP_COMBO_ARROW_SPRITES.
// ORIGINAL: 0x0044FB30 ??__Fg_IFACE_MP_COMBO_ARROW_SPRITES@@YAXXZ 0x0044FB30-0x0044FB44
// symbol    ?destroy_iface_mp_combo_arrow_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_mp_combo_arrow_sprites() {
    VectorDtorIterator(g_IFACE_MP_COMBO_ARROW_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_ARROW_ICON_SPRITES.
// ORIGINAL: 0x0044FB80 ??__Fg_SCROLL_BAR_ARROW_ICON_SPRITES@@YAXXZ 0x0044FB80-0x0044FB94
// symbol    ?destroy_scroll_bar_arrow_icon_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_arrow_icon_sprites() {
    VectorDtorIterator(g_SCROLL_BAR_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES.
// ORIGINAL: 0x0044FBD0 ??__Fg_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES@@YAXXZ 0x0044FBD0-0x0044FBE4
// symbol    ?destroy_scroll_bar_small_arrow_icon_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_small_arrow_icon_sprites() {
    VectorDtorIterator(g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_FILLER_ICON_SPRITES.
// ORIGINAL: 0x0044FC10 ??__Fg_SCROLL_BAR_FILLER_ICON_SPRITES@@YAXXZ 0x0044FC10-0x0044FC1A BYTE_EXACT
// symbol    ?destroy_scroll_bar_filler_icon_sprites@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_filler_icon_sprites() {
    g_SCROLL_BAR_FILLER_ICON_SPRITES->close();
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE.
// ORIGINAL: 0x0044FC40 ??__Fg_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE@@YAXXZ 0x0044FC40-0x0044FC4A BYTE_EXACT
// symbol    ?destroy_scroll_bar_small_filler_icon_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_small_filler_icon_sprite() {
    g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_LOCK_SPRITES.
// ORIGINAL: 0x0044FC80 ??__Fg_IFACE_LOCK_SPRITES@@YAXXZ 0x0044FC80-0x0044FC94
// symbol    ?destroy_iface_lock_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_lock_sprites() {
    VectorDtorIterator(g_IFACE_LOCK_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR01.
// ORIGINAL: 0x0044FCD0 ??__Fg_UNUSED_SPRITES_VAR01@@YAXXZ 0x0044FCD0-0x0044FCE4
// symbol    ?destroy_unused_sprites_var01@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var01() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR01, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE.
// ORIGINAL: 0x0044FD10 ??__Fg_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE@@YAXXZ 0x0044FD10-0x0044FD1A BYTE_EXACT
// symbol    ?destroy_iface_general_windows_top_left_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE.
// ORIGINAL: 0x0044FD40 ??__Fg_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE@@YAXXZ 0x0044FD40-0x0044FD4A BYTE_EXACT
// symbol    ?destroy_iface_general_windows_top_right_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE.
// ORIGINAL: 0x0044FD70 ??__Fg_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE@@YAXXZ 0x0044FD70-0x0044FD7A BYTE_EXACT
// symbol    ?destroy_iface_general_windows_bot_left_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE.
// ORIGINAL: 0x0044FDA0 ??__Fg_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE@@YAXXZ 0x0044FDA0-0x0044FDAA BYTE_EXACT
// symbol    ?destroy_iface_general_windows_bot_right_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE.
// ORIGINAL: 0x0044FDD0 ??__Fg_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE@@YAXXZ 0x0044FDD0-0x0044FDDA BYTE_EXACT
// symbol    ?destroy_iface_general_windows_mid_left_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_mid_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE.
// ORIGINAL: 0x0044FE00 ??__Fg_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE@@YAXXZ 0x0044FE00-0x0044FE0A BYTE_EXACT
// symbol    ?destroy_iface_general_windows_mid_right_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_mid_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE.
// ORIGINAL: 0x0044FE30 ??__Fg_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE@@YAXXZ 0x0044FE30-0x0044FE3A BYTE_EXACT
// symbol    ?destroy_iface_general_windows_top_mid_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE.
// ORIGINAL: 0x0044FE60 ??__Fg_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE@@YAXXZ 0x0044FE60-0x0044FE6A BYTE_EXACT
// symbol    ?destroy_iface_general_windows_bot_mid_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE.
// ORIGINAL: 0x0044FE90 ??__Fg_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE@@YAXXZ 0x0044FE90-0x0044FE9A BYTE_EXACT
// symbol    ?destroy_iface_general_windows_noncap_mid_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE.
// ORIGINAL: 0x0044FEC0 ??__Fg_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE@@YAXXZ 0x0044FEC0-0x0044FECA BYTE_EXACT
// symbol    ?destroy_iface_general_windows_noncap_left_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE.
// ORIGINAL: 0x0044FEF0 ??__Fg_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE@@YAXXZ 0x0044FEF0-0x0044FEFA BYTE_EXACT
// symbol    ?destroy_iface_general_windows_noncap_right_sprite@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR23.
// ORIGINAL: 0x0044FF20 ??__Fg_UNUSED_SPRITE_VAR23@@YAXXZ 0x0044FF20-0x0044FF2A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var23@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var23() {
    g_UNUSED_SPRITE_VAR23->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR24.
// ORIGINAL: 0x0044FF50 ??__Fg_UNUSED_SPRITE_VAR24@@YAXXZ 0x0044FF50-0x0044FF5A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var24@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var24() {
    g_UNUSED_SPRITE_VAR24->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR25.
// ORIGINAL: 0x0044FF80 ??__Fg_UNUSED_SPRITE_VAR25@@YAXXZ 0x0044FF80-0x0044FF8A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var25@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var25() {
    g_UNUSED_SPRITE_VAR25->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR26.
// ORIGINAL: 0x0044FFB0 ??__Fg_UNUSED_SPRITE_VAR26@@YAXXZ 0x0044FFB0-0x0044FFBA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var26@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var26() {
    g_UNUSED_SPRITE_VAR26->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR27.
// ORIGINAL: 0x0044FFE0 ??__Fg_UNUSED_SPRITE_VAR27@@YAXXZ 0x0044FFE0-0x0044FFEA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var27@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var27() {
    g_UNUSED_SPRITE_VAR27->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR28.
// ORIGINAL: 0x00450010 ??__Fg_UNUSED_SPRITE_VAR28@@YAXXZ 0x00450010-0x0045001A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var28@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var28() {
    g_UNUSED_SPRITE_VAR28->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR29.
// ORIGINAL: 0x00450040 ??__Fg_UNUSED_SPRITE_VAR29@@YAXXZ 0x00450040-0x0045004A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var29@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var29() {
    g_UNUSED_SPRITE_VAR29->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR30.
// ORIGINAL: 0x00450070 ??__Fg_UNUSED_SPRITE_VAR30@@YAXXZ 0x00450070-0x0045007A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var30@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var30() {
    g_UNUSED_SPRITE_VAR30->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR31.
// ORIGINAL: 0x004500A0 ??__Fg_UNUSED_SPRITE_VAR31@@YAXXZ 0x004500A0-0x004500AA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var31@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var31() {
    g_UNUSED_SPRITE_VAR31->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR32.
// ORIGINAL: 0x004500D0 ??__Fg_UNUSED_SPRITE_VAR32@@YAXXZ 0x004500D0-0x004500DA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var32@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var32() {
    g_UNUSED_SPRITE_VAR32->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR33.
// ORIGINAL: 0x00450100 ??__Fg_UNUSED_SPRITE_VAR33@@YAXXZ 0x00450100-0x0045010A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var33@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var33() {
    g_UNUSED_SPRITE_VAR33->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR34.
// ORIGINAL: 0x00450130 ??__Fg_UNUSED_SPRITE_VAR34@@YAXXZ 0x00450130-0x0045013A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var34@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var34() {
    g_UNUSED_SPRITE_VAR34->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR35.
// ORIGINAL: 0x00450160 ??__Fg_UNUSED_SPRITE_VAR35@@YAXXZ 0x00450160-0x0045016A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var35@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var35() {
    g_UNUSED_SPRITE_VAR35->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR36.
// ORIGINAL: 0x00450190 ??__Fg_UNUSED_SPRITE_VAR36@@YAXXZ 0x00450190-0x0045019A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var36@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var36() {
    g_UNUSED_SPRITE_VAR36->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR37.
// ORIGINAL: 0x004501C0 ??__Fg_UNUSED_SPRITE_VAR37@@YAXXZ 0x004501C0-0x004501CA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var37@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var37() {
    g_UNUSED_SPRITE_VAR37->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR38.
// ORIGINAL: 0x004501F0 ??__Fg_UNUSED_SPRITE_VAR38@@YAXXZ 0x004501F0-0x004501FA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var38@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var38() {
    g_UNUSED_SPRITE_VAR38->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR39.
// ORIGINAL: 0x00450220 ??__Fg_UNUSED_SPRITE_VAR39@@YAXXZ 0x00450220-0x0045022A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var39@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var39() {
    g_UNUSED_SPRITE_VAR39->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR40.
// ORIGINAL: 0x00450250 ??__Fg_UNUSED_SPRITE_VAR40@@YAXXZ 0x00450250-0x0045025A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var40@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var40() {
    g_UNUSED_SPRITE_VAR40->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR41.
// ORIGINAL: 0x00450280 ??__Fg_UNUSED_SPRITE_VAR41@@YAXXZ 0x00450280-0x0045028A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var41@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var41() {
    g_UNUSED_SPRITE_VAR41->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR42.
// ORIGINAL: 0x004502B0 ??__Fg_UNUSED_SPRITE_VAR42@@YAXXZ 0x004502B0-0x004502BA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var42@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var42() {
    g_UNUSED_SPRITE_VAR42->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR43.
// ORIGINAL: 0x004502E0 ??__Fg_UNUSED_SPRITE_VAR43@@YAXXZ 0x004502E0-0x004502EA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var43@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var43() {
    g_UNUSED_SPRITE_VAR43->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR44.
// ORIGINAL: 0x00450310 ??__Fg_UNUSED_SPRITE_VAR44@@YAXXZ 0x00450310-0x0045031A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var44@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var44() {
    g_UNUSED_SPRITE_VAR44->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR45.
// ORIGINAL: 0x00450340 ??__Fg_UNUSED_SPRITE_VAR45@@YAXXZ 0x00450340-0x0045034A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var45@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var45() {
    g_UNUSED_SPRITE_VAR45->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR46.
// ORIGINAL: 0x00450370 ??__Fg_UNUSED_SPRITE_VAR46@@YAXXZ 0x00450370-0x0045037A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var46@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var46() {
    g_UNUSED_SPRITE_VAR46->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_TECH_TREE_ARROW_SPRITES.
// ORIGINAL: 0x004503B0 ??__Fg_IFACE_TECH_TREE_ARROW_SPRITES@@YAXXZ 0x004503B0-0x004503C4
// symbol    ?destroy_iface_tech_tree_arrow_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_tech_tree_arrow_sprites() {
    VectorDtorIterator(g_IFACE_TECH_TREE_ARROW_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR47.
// ORIGINAL: 0x004503F0 ??__Fg_UNUSED_SPRITE_VAR47@@YAXXZ 0x004503F0-0x004503FA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var47@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var47() {
    g_UNUSED_SPRITE_VAR47->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR48.
// ORIGINAL: 0x00450420 ??__Fg_UNUSED_SPRITE_VAR48@@YAXXZ 0x00450420-0x0045042A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var48@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var48() {
    g_UNUSED_SPRITE_VAR48->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR49.
// ORIGINAL: 0x00450450 ??__Fg_UNUSED_SPRITE_VAR49@@YAXXZ 0x00450450-0x0045045A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var49@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var49() {
    g_UNUSED_SPRITE_VAR49->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR50.
// ORIGINAL: 0x00450480 ??__Fg_UNUSED_SPRITE_VAR50@@YAXXZ 0x00450480-0x0045048A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var50@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var50() {
    g_UNUSED_SPRITE_VAR50->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR51.
// ORIGINAL: 0x004504B0 ??__Fg_UNUSED_SPRITE_VAR51@@YAXXZ 0x004504B0-0x004504BA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var51@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var51() {
    g_UNUSED_SPRITE_VAR51->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR52.
// ORIGINAL: 0x004504E0 ??__Fg_UNUSED_SPRITE_VAR52@@YAXXZ 0x004504E0-0x004504EA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var52@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var52() {
    g_UNUSED_SPRITE_VAR52->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR53.
// ORIGINAL: 0x00450510 ??__Fg_UNUSED_SPRITE_VAR53@@YAXXZ 0x00450510-0x0045051A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var53@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var53() {
    g_UNUSED_SPRITE_VAR53->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR54.
// ORIGINAL: 0x00450540 ??__Fg_UNUSED_SPRITE_VAR54@@YAXXZ 0x00450540-0x0045054A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var54@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var54() {
    g_UNUSED_SPRITE_VAR54->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR55.
// ORIGINAL: 0x00450570 ??__Fg_UNUSED_SPRITE_VAR55@@YAXXZ 0x00450570-0x0045057A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var55@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var55() {
    g_UNUSED_SPRITE_VAR55->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR56.
// ORIGINAL: 0x004505A0 ??__Fg_UNUSED_SPRITE_VAR56@@YAXXZ 0x004505A0-0x004505AA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var56@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var56() {
    g_UNUSED_SPRITE_VAR56->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR57.
// ORIGINAL: 0x004505D0 ??__Fg_UNUSED_SPRITE_VAR57@@YAXXZ 0x004505D0-0x004505DA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var57@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var57() {
    g_UNUSED_SPRITE_VAR57->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR58.
// ORIGINAL: 0x00450600 ??__Fg_UNUSED_SPRITE_VAR58@@YAXXZ 0x00450600-0x0045060A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var58@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var58() {
    g_UNUSED_SPRITE_VAR58->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR59.
// ORIGINAL: 0x00450630 ??__Fg_UNUSED_SPRITE_VAR59@@YAXXZ 0x00450630-0x0045063A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var59@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var59() {
    g_UNUSED_SPRITE_VAR59->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR60.
// ORIGINAL: 0x00450660 ??__Fg_UNUSED_SPRITE_VAR60@@YAXXZ 0x00450660-0x0045066A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var60@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var60() {
    g_UNUSED_SPRITE_VAR60->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR61.
// ORIGINAL: 0x00450690 ??__Fg_UNUSED_SPRITE_VAR61@@YAXXZ 0x00450690-0x0045069A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var61@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var61() {
    g_UNUSED_SPRITE_VAR61->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR62.
// ORIGINAL: 0x004506C0 ??__Fg_UNUSED_SPRITE_VAR62@@YAXXZ 0x004506C0-0x004506CA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var62@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var62() {
    g_UNUSED_SPRITE_VAR62->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR63.
// ORIGINAL: 0x004506F0 ??__Fg_UNUSED_SPRITE_VAR63@@YAXXZ 0x004506F0-0x004506FA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var63@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var63() {
    g_UNUSED_SPRITE_VAR63->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR64.
// ORIGINAL: 0x00450720 ??__Fg_UNUSED_SPRITE_VAR64@@YAXXZ 0x00450720-0x0045072A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var64@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var64() {
    g_UNUSED_SPRITE_VAR64->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR65.
// ORIGINAL: 0x00450750 ??__Fg_UNUSED_SPRITE_VAR65@@YAXXZ 0x00450750-0x0045075A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var65@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var65() {
    g_UNUSED_SPRITE_VAR65->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR66.
// ORIGINAL: 0x00450780 ??__Fg_UNUSED_SPRITE_VAR66@@YAXXZ 0x00450780-0x0045078A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var66@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var66() {
    g_UNUSED_SPRITE_VAR66->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR67.
// ORIGINAL: 0x004507B0 ??__Fg_UNUSED_SPRITE_VAR67@@YAXXZ 0x004507B0-0x004507BA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var67@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var67() {
    g_UNUSED_SPRITE_VAR67->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR68.
// ORIGINAL: 0x004507E0 ??__Fg_UNUSED_SPRITE_VAR68@@YAXXZ 0x004507E0-0x004507EA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var68@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var68() {
    g_UNUSED_SPRITE_VAR68->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR69.
// ORIGINAL: 0x00450810 ??__Fg_UNUSED_SPRITE_VAR69@@YAXXZ 0x00450810-0x0045081A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var69@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var69() {
    g_UNUSED_SPRITE_VAR69->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR70.
// ORIGINAL: 0x00450840 ??__Fg_UNUSED_SPRITE_VAR70@@YAXXZ 0x00450840-0x0045084A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var70@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var70() {
    g_UNUSED_SPRITE_VAR70->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR71.
// ORIGINAL: 0x00450870 ??__Fg_UNUSED_SPRITE_VAR71@@YAXXZ 0x00450870-0x0045087A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var71@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var71() {
    g_UNUSED_SPRITE_VAR71->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR72.
// ORIGINAL: 0x004508A0 ??__Fg_UNUSED_SPRITE_VAR72@@YAXXZ 0x004508A0-0x004508AA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var72@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var72() {
    g_UNUSED_SPRITE_VAR72->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR73.
// ORIGINAL: 0x004508D0 ??__Fg_UNUSED_SPRITE_VAR73@@YAXXZ 0x004508D0-0x004508DA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var73@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var73() {
    g_UNUSED_SPRITE_VAR73->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR74.
// ORIGINAL: 0x00450900 ??__Fg_UNUSED_SPRITE_VAR74@@YAXXZ 0x00450900-0x0045090A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var74@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var74() {
    g_UNUSED_SPRITE_VAR74->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR02.
// ORIGINAL: 0x00450940 ??__Fg_UNUSED_SPRITES_VAR02@@YAXXZ 0x00450940-0x00450954
// symbol    ?destroy_unused_sprites_var02@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var02() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR02, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR03.
// ORIGINAL: 0x00450990 ??__Fg_UNUSED_SPRITES_VAR03@@YAXXZ 0x00450990-0x004509A4
// symbol    ?destroy_unused_sprites_var03@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var03() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR03, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR04.
// ORIGINAL: 0x004509E0 ??__Fg_UNUSED_SPRITES_VAR04@@YAXXZ 0x004509E0-0x004509F4
// symbol    ?destroy_unused_sprites_var04@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var04() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR04, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR05.
// ORIGINAL: 0x00450A30 ??__Fg_UNUSED_SPRITES_VAR05@@YAXXZ 0x00450A30-0x00450A44
// symbol    ?destroy_unused_sprites_var05@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var05() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR05, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR06.
// ORIGINAL: 0x00450A80 ??__Fg_UNUSED_SPRITES_VAR06@@YAXXZ 0x00450A80-0x00450A94
// symbol    ?destroy_unused_sprites_var06@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var06() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR06, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR07.
// ORIGINAL: 0x00450AD0 ??__Fg_UNUSED_SPRITES_VAR07@@YAXXZ 0x00450AD0-0x00450AE4
// symbol    ?destroy_unused_sprites_var07@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var07() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR07, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR08.
// ORIGINAL: 0x00450B20 ??__Fg_UNUSED_SPRITES_VAR08@@YAXXZ 0x00450B20-0x00450B34
// symbol    ?destroy_unused_sprites_var08@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var08() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR08, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR09.
// ORIGINAL: 0x00450B70 ??__Fg_UNUSED_SPRITES_VAR09@@YAXXZ 0x00450B70-0x00450B84
// symbol    ?destroy_unused_sprites_var09@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var09() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR09, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR10.
// ORIGINAL: 0x00450BC0 ??__Fg_UNUSED_SPRITES_VAR10@@YAXXZ 0x00450BC0-0x00450BD4
// symbol    ?destroy_unused_sprites_var10@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var10() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR10, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR75.
// ORIGINAL: 0x00450C00 ??__Fg_UNUSED_SPRITE_VAR75@@YAXXZ 0x00450C00-0x00450C0A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var75@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var75() {
    g_UNUSED_SPRITE_VAR75->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR76.
// ORIGINAL: 0x00450C30 ??__Fg_UNUSED_SPRITE_VAR76@@YAXXZ 0x00450C30-0x00450C3A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var76@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var76() {
    g_UNUSED_SPRITE_VAR76->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR77.
// ORIGINAL: 0x00450C60 ??__Fg_UNUSED_SPRITE_VAR77@@YAXXZ 0x00450C60-0x00450C6A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var77@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var77() {
    g_UNUSED_SPRITE_VAR77->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR78.
// ORIGINAL: 0x00450C90 ??__Fg_UNUSED_SPRITE_VAR78@@YAXXZ 0x00450C90-0x00450C9A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var78@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var78() {
    g_UNUSED_SPRITE_VAR78->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR79.
// ORIGINAL: 0x00450CC0 ??__Fg_UNUSED_SPRITE_VAR79@@YAXXZ 0x00450CC0-0x00450CCA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var79@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var79() {
    g_UNUSED_SPRITE_VAR79->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR80.
// ORIGINAL: 0x00450CF0 ??__Fg_UNUSED_SPRITE_VAR80@@YAXXZ 0x00450CF0-0x00450CFA BYTE_EXACT
// symbol    ?destroy_unused_sprite_var80@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var80() {
    g_UNUSED_SPRITE_VAR80->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR81.
// ORIGINAL: 0x00450D20 ??__Fg_UNUSED_SPRITE_VAR81@@YAXXZ 0x00450D20-0x00450D2A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var81@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var81() {
    g_UNUSED_SPRITE_VAR81->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR82.
// ORIGINAL: 0x00450D50 ??__Fg_UNUSED_SPRITE_VAR82@@YAXXZ 0x00450D50-0x00450D5A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var82@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var82() {
    g_UNUSED_SPRITE_VAR82->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR83.
// ORIGINAL: 0x00450D80 ??__Fg_UNUSED_SPRITE_VAR83@@YAXXZ 0x00450D80-0x00450D8A BYTE_EXACT
// symbol    ?destroy_unused_sprite_var83@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var83() {
    g_UNUSED_SPRITE_VAR83->close();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN_SPRITES.
// ORIGINAL: 0x00450DC0 ??__Fg_BASEWIN_SPRITES@@YAXXZ 0x00450DC0-0x00450DD4
// symbol    ?destroy_basewin_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin_sprites() {
    VectorDtorIterator(g_BASEWIN_SPRITES, 0x2C, 27, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_INFOWIN.
// ORIGINAL: 0x004562E0 ??__Fg_INFOWIN@@YAXXZ 0x004562E0-0x004562EA
// symbol    ?destroy_infowin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_infowin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007AD2A0))->*InfoWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MAININTERFACE.
// ORIGINAL: 0x00459580 ??__Fg_MAININTERFACE@@YAXXZ 0x00459580-0x0045958A BYTE_EXACT
// symbol    ?destroy_maininterface@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_maininterface() {
    static_cast<MainInterface *>(reinterpret_cast<void *>(0x007AE820))->MainInterface::~MainInterface();
}

/*
Purpose: Atexit teardown thunk for g_MAININTERFACE_WAVE.
// ORIGINAL: 0x004595B0 ??__Fg_MAININTERFACE_WAVE@@YAXXZ 0x004595B0-0x004595BA
// symbol    ?destroy_maininterface_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_maininterface_wave() {
    (ORIGINAL(g_MAININTERFACE_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_JACKAL_FONT.
// ORIGINAL: 0x0045F940 ??__Fg_JACKAL_FONT@@YAXXZ 0x0045F940-0x0045F94A BYTE_EXACT
// symbol    ?destroy_jackal_font@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_jackal_font() {
    g_JACKAL_FONT.~Font();
}

/*
Purpose: Atexit teardown thunk for g_MESSAGEWIN.
// ORIGINAL: 0x00471340 ??__Fg_MESSAGEWIN@@YAXXZ 0x00471340-0x0047134A
// symbol    ?destroy_messagewin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_messagewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007F67F8))->*MessageWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GREEN_RIGHT_ARROW_SPRITE.
// ORIGINAL: 0x00471380 ??__Fg_IFACE_GREEN_RIGHT_ARROW_SPRITE@@YAXXZ 0x00471380-0x00471394
// symbol    ?destroy_iface_green_right_arrow_sprite@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_green_right_arrow_sprite() {
    VectorDtorIterator(g_IFACE_GREEN_RIGHT_ARROW_SPRITE, 0x2C, 1, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MONUWIN.
// ORIGINAL: 0x00472220 ??__Fg_MONUWIN@@YAXXZ 0x00472220-0x0047222A BYTE_EXACT
// symbol    ?destroy_monuwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_monuwin() {
    static_cast<MonuWin *>(reinterpret_cast<void *>(0x007F9F58))->MonuWin::~MonuWin();
}

/*
Purpose: Atexit teardown thunk for g_MULTIWIN.
// ORIGINAL: 0x00477E30 ??__Fg_MULTIWIN@@YAXXZ 0x00477E30-0x00477E3A BYTE_EXACT
// symbol    ?destroy_multiwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multiwin() {
    static_cast<MultiWin *>(reinterpret_cast<void *>(0x007FD648))->MultiWin::~MultiWin();
}

/*
Purpose: Atexit teardown thunk for g_MULTIWIN_WAVE.
// ORIGINAL: 0x00477E60 ??__Fg_MULTIWIN_WAVE@@YAXXZ 0x00477E60-0x00477E6A
// symbol    ?destroy_multiwin_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multiwin_wave() {
    (ORIGINAL(g_MULTIWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_NETMSG1.
// ORIGINAL: 0x0047A790 ??__Fg_NETMSG1@@YAXXZ 0x0047A790-0x0047A79A BYTE_EXACT
// symbol    ?destroy_netmsg1@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netmsg1() {
    static_cast<NetMsg *>(reinterpret_cast<void *>(0x00805338))->NetMsg::~NetMsg();
}

/*
Purpose: Atexit teardown thunk for g_NETMSG2.
// ORIGINAL: 0x0047A7C0 ??__Fg_NETMSG2@@YAXXZ 0x0047A7C0-0x0047A7CA BYTE_EXACT
// symbol    ?destroy_netmsg2@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netmsg2() {
    static_cast<NetMsg *>(reinterpret_cast<void *>(0x007FFF80))->NetMsg::~NetMsg();
}

/*
Purpose: Atexit teardown thunk for g_NETWIN.
// ORIGINAL: 0x0047B010 ??__Fg_NETWIN@@YAXXZ 0x0047B010-0x0047B01A BYTE_EXACT
// symbol    ?destroy_netwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netwin() {
    static_cast<NetWin *>(reinterpret_cast<void *>(0x0080A6F8))->NetWin::~NetWin();
}

/*
Purpose: Atexit teardown thunk for g_NEWTECHWIN.
// ORIGINAL: 0x00483870 ??__Fg_NEWTECHWIN@@YAXXZ 0x00483870-0x0048387A BYTE_EXACT
// symbol    ?destroy_newtechwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_newtechwin() {
    static_cast<NewTechWin *>(reinterpret_cast<void *>(0x00811E40))->NewTechWin::~NewTechWin();
}

/*
Purpose: Atexit teardown thunk for g_PICKWIN.
// ORIGINAL: 0x00488770 ??__Fg_PICKWIN@@YAXXZ 0x00488770-0x0048877A BYTE_EXACT
// symbol    ?destroy_pickwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_pickwin() {
    static_cast<PickWin *>(reinterpret_cast<void *>(0x00822718))->PickWin::~PickWin();
}

/*
Purpose: Atexit teardown thunk for g_PREFWIN.
// ORIGINAL: 0x0048D510 ??__Fg_PREFWIN@@YAXXZ 0x0048D510-0x0048D51A BYTE_EXACT
// symbol    ?destroy_prefwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_prefwin() {
    static_cast<PrefWin *>(reinterpret_cast<void *>(0x008578D8))->PrefWin::~PrefWin();
}

/*
Purpose: Atexit teardown thunk for g_PREFWIN_BUTTONGROUP.
// ORIGINAL: 0x0048D540 ??__Fg_PREFWIN_BUTTONGROUP@@YAXXZ 0x0048D540-0x0048D54A BYTE_EXACT
// symbol    ?destroy_prefwin_buttongroup@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_prefwin_buttongroup() {
    g_PREFWIN_BUTTONGROUP->close();
}

/*
Purpose: Atexit teardown thunk for g_QUAYLEWIN.
// ORIGINAL: 0x00495190 ??__Fg_QUAYLEWIN@@YAXXZ 0x00495190-0x0049519A BYTE_EXACT
// symbol    ?destroy_quaylewin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_quaylewin() {
    static_cast<QuayleWin *>(reinterpret_cast<void *>(0x00872CB0))->QuayleWin::~QuayleWin();
}

/*
Purpose: Atexit teardown thunk for g_REPORTIF.
// ORIGINAL: 0x00496920 ??__Fg_REPORTIF@@YAXXZ 0x00496920-0x0049692A
// symbol    ?destroy_reportif@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reportif() {
    (ORIGINAL(reinterpret_cast<void *>(0x00885F38))->*ReportIfDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_REPORTWIN.
// ORIGINAL: 0x00496950 ??__Fg_REPORTWIN@@YAXXZ 0x00496950-0x0049695A BYTE_EXACT
// symbol    ?destroy_reportwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reportwin() {
    static_cast<ReportWin *>(reinterpret_cast<void *>(0x00876478))->ReportWin::~ReportWin();
}

/*
Purpose: Atexit teardown thunk for SocialWinParent.
// ORIGINAL: 0x004AE9D0 ??__FSocialWinParent@@YAXXZ 0x004AE9D0-0x004AE9DA BYTE_EXACT
// symbol    ?destroy_socialwinparent@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_socialwinparent() {
    static_cast<SocialWin *>(reinterpret_cast<void *>(0x008A6270))->SocialWin::~SocialWin();
}

/*
Purpose: Atexit teardown thunk for g_STATUSWIN.
// ORIGINAL: 0x004B3FC0 ??__Fg_STATUSWIN@@YAXXZ 0x004B3FC0-0x004B3FCA
// symbol    ?destroy_statuswin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_statuswin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008C5568))->*StatusWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_TUTWIN.
// ORIGINAL: 0x004BA5E0 ??__Fg_TUTWIN@@YAXXZ 0x004BA5E0-0x004BA5EA BYTE_EXACT
// symbol    ?destroy_tutwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tutwin() {
    static_cast<TutWin *>(reinterpret_cast<void *>(0x008C6E68))->TutWin::~TutWin();
}

/*
Purpose: Atexit teardown thunk for g_VEHDRAW_CAVIAR.
// ORIGINAL: 0x004BF700 ??__Fg_VEHDRAW_CAVIAR@@YAXXZ 0x004BF700-0x004BF70A
// symbol    ?destroy_vehdraw_caviar@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vehdraw_caviar() {
    (ORIGINAL(reinterpret_cast<void *>(0x008CC828))->*CaviarCloseTarget)();
}

/*
Purpose: Atexit teardown thunk for g_VEHDRAW_BUFFER.
// ORIGINAL: 0x004BF730 ??__Fg_VEHDRAW_BUFFER@@YAXXZ 0x004BF730-0x004BF73A BYTE_EXACT
// symbol    ?destroy_vehdraw_buffer@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vehdraw_buffer() {
    g_VEHDRAW_BUFFER->Buffer::~Buffer();
}

/*
Purpose: Atexit teardown thunk for g_WORLDWIN.
// ORIGINAL: 0x004C38D0 ??__Fg_WORLDWIN@@YAXXZ 0x004C38D0-0x004C38DA BYTE_EXACT
// symbol    ?destroy_worldwin@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_worldwin() {
    static_cast<WorldWin *>(reinterpret_cast<void *>(0x008E9F60))->WorldWin::~WorldWin();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_DEVICE.
// ORIGINAL: 0x004C5C70 ??__Fg_WAVE_DEVICE@@YAXXZ 0x004C5C70-0x004C5C7A
// symbol    ?destroy_wave_device@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090D978))->*Wave_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MIDI_DEVICE.
// ORIGINAL: 0x004C5CA0 ??__Fg_MIDI_DEVICE@@YAXXZ 0x004C5CA0-0x004C5CAA
// symbol    ?destroy_midi_device@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_midi_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090D950))->*Midi_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_IN_DEVICE.
// ORIGINAL: 0x004C5CD0 ??__Fg_WAVE_IN_DEVICE@@YAXXZ 0x004C5CD0-0x004C5CDA
// symbol    ?destroy_wave_in_device@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_in_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090DB50))->*Wave_In_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CONSOLE_TIMER.
// ORIGINAL: 0x0050E9A0 ??__Fg_CONSOLE_TIMER@@YAXXZ 0x0050E9A0-0x0050E9AA
// symbol    ?destroy_console_timer@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_console_timer() {
    (ORIGINAL(reinterpret_cast<void *>(0x00939E88))->*TimeDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CURSOR_SPRITES.
// ORIGINAL: 0x0052DAF0 ??__Fg_CURSOR_SPRITES@@YAXXZ 0x0052DAF0-0x0052DB04
// symbol    ?destroy_cursor_sprites@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_cursor_sprites() {
    VectorDtorIterator(g_CURSOR_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_NETDAEMON.
// ORIGINAL: 0x0052DB30 ??__Fg_NETDAEMON@@YAXXZ 0x0052DB30-0x0052DB3A
// symbol    ?destroy_netdaemon@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netdaemon() {
    (ORIGINAL(reinterpret_cast<void *>(0x0093CD90))->*NetDaemonDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_FONTQUEUE_VAL2.
// ORIGINAL: 0x00559250 ??__Fg_FONTQUEUE_VAL2@@YAXXZ 0x00559250-0x0055925A BYTE_EXACT
// symbol    ?destroy_fontqueue_val2@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fontqueue_val2() {
    g_FONTQUEUE_VAL2->~FontQueue();
}

/*
Purpose: Atexit teardown thunk for g_FONTQUEUE_VAL1.
// ORIGINAL: 0x00559280 ??__Fg_FONTQUEUE_VAL1@@YAXXZ 0x00559280-0x0055928A BYTE_EXACT
// symbol    ?destroy_fontqueue_val1@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fontqueue_val1() {
    g_FONTQUEUE_VAL1->~FontQueue();
}

/*
Purpose: Atexit teardown thunk for g_TOP_MENU_WAVE.
// ORIGINAL: 0x00584D20 ??__Fg_TOP_MENU_WAVE@@YAXXZ 0x00584D20-0x00584D2A
// symbol    ?destroy_top_menu_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_top_menu_wave() {
    (ORIGINAL(g_TOP_MENU_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_FONTS.
// ORIGINAL: 0x005882D0 ??__Fg_FONTS@@YAXXZ 0x005882D0-0x005882E4
// symbol    ?destroy_fonts@@YAXXZ
// size      20 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fonts() {
    VectorDtorIterator(g_FONTS, 0x28, 48, FontElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CRASH_LANDING_WAVE.
// ORIGINAL: 0x005AE110 ??__Fg_CRASH_LANDING_WAVE@@YAXXZ 0x005AE110-0x005AE11A
// symbol    ?destroy_crash_landing_wave@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_crash_landing_wave() {
    (ORIGINAL(g_CRASH_LANDING_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_GENERAL.
// ORIGINAL: 0x005B9C30 ??__Fg_WAVE_GENERAL@@YAXXZ 0x005B9C30-0x005B9C3A
// symbol    ?destroy_wave_general@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_general() {
    (ORIGINAL(g_WAVE_GENERAL)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_PALETTE1.
// ORIGINAL: 0x005BEC60 ??__Fg_PALETTE1@@YAXXZ 0x005BEC60-0x005BEC6A
// symbol    ?destroy_palette1@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_palette1() {
    (ORIGINAL(reinterpret_cast<void *>(0x0094C590))->*PaletteDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PALETTE2.
// ORIGINAL: 0x005BEC90 ??__Fg_PALETTE2@@YAXXZ 0x005BEC90-0x005BEC9A
// symbol    ?destroy_palette2@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_palette2() {
    (ORIGINAL(reinterpret_cast<void *>(0x009523A0))->*PaletteDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MULTIDEBUG.
// ORIGINAL: 0x005C97E0 ??__Fg_MULTIDEBUG@@YAXXZ 0x005C97E0-0x005C97EA BYTE_EXACT
// symbol    ?destroy_multidebug@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multidebug() {
    static_cast<MultiDebug *>(reinterpret_cast<void *>(0x009B22F0))->MultiDebug::~MultiDebug();
}

/*
Purpose: Atexit teardown thunk for g_BUFFER.
// ORIGINAL: 0x005E37D0 ??__Fg_BUFFER@@YAXXZ 0x005E37D0-0x005E37DA BYTE_EXACT
// symbol    ?destroy_buffer@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_buffer() {
    g_BUFFER->Buffer::~Buffer();
}

/*
Purpose: Atexit teardown thunk for TxtIndex.
// ORIGINAL: 0x005FD510 ??__FTxtIndex@@YAXXZ 0x005FD510-0x005FD527
// symbol    ?destroy_txtindex@@YAXXZ
// size      23 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_txtindex() {
    VectorDtorIterator(TxtIndexGlobal, 0x118, 4, TextIndexElementTeardown);
}

/*
Purpose: Atexit teardown thunk for StringTable.
// ORIGINAL: 0x006168C0 ??__FStringTable@@YAXXZ 0x006168C0-0x006168CA
// symbol    ?destroy_stringtable@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_stringtable() {
    (ORIGINAL(reinterpret_cast<void *>(0x009B90D8))->*StringsDtorTarget)();
}
