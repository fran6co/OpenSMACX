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
#include <new>
#include "init_thunks.h"
#include "atexit_thunks.h"
#include "ambience.h"
#include "buffer.h"
#include "buttongroup.h"
#include "caviar.h"
#include "font.h"
#include "fx.h"
#include "sprite.h"
#include "strings.h"
#include "texture.h"
#include "time.h"
#include "wave.h"
#include "wave_device.h"

// The game CRT's atexit: the exit list the original
// registrations live in, so recovered initializers must
// register there too or teardown order would split across
// two CRTs. Rebindable so tests can observe registration.

// Per-element constructors the array walks dispatch
// to, bound at the original addresses the ??_L call
// sites pass; the hybrid redirects each to its
// recovered body at run time.
func_thiscall_teardown BufferElementCtor =
    original_method<func_thiscall_teardown>(0x005D7210);
func_thiscall_teardown CaviarDataElementCtor =
    original_method<func_thiscall_teardown>(0x00616BC0);
func_thiscall_teardown FontElementCtor =
    original_method<func_thiscall_teardown>(0x00618EA0);
func_thiscall_teardown SpriteElementCtor =
    original_method<func_thiscall_teardown>(0x005E37E0);
func_thiscall_teardown TextIndexElementCtor =
    original_method<func_thiscall_teardown>(0x005FDF40);
func_thiscall_teardown TextureElementCtor =
    original_method<func_thiscall_teardown>(0x00619650);
func_thiscall_teardown WaveElementCtor =
    original_method<func_thiscall_teardown>(0x004C66E0);

namespace {

// Source-owned trampolines the scalar seams default to:
// each runs the recovered construction on the object the
// thunk names. The hybrid executes these; leaf tests
// rebind the seams to recorders instead.
void __fastcall ambienceinitctor_default(void *object) {
    static_cast<Ambience *>(object)->construct();
}
void __fastcall bufferinitctor_default(void *object) {
    new (object) Buffer();
}
void __fastcall buttongroupinitctor_default(void *object) {
    new (object) ButtonGroup();
}
void __fastcall caviardatainitctor_default(void *object) {
    new (object) CaviarData();
}
void __fastcall caviarinitctor_default(void *object) {
    new (object) Caviar();
}
void __fastcall fxinitctor_default(void *object) {
    new (object) FX();
}
void __fastcall fontinitctor_default(void *object) {
    new (object) Font();
}
void __fastcall spriteinitctor_default(void *object) {
    new (object) Sprite();
}
void __fastcall stringsinitctor_default(void *object) {
    new (object) Strings();
}
void __fastcall textureinitctor_default(void *object) {
    new (object) Texture();
}
void __fastcall timeinitctor_default(void *object) {
    new (object) Time();
}
void __fastcall wavedeviceinitctor_default(void *object) {
    new (object) Wave_Device();
}
void __fastcall waveinitctor_default(void *object) {
    new (object) Wave();
}

}  // namespace

func_thiscall_teardown AmbienceInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&ambienceinitctor_default));
func_thiscall_teardown BufferInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&bufferinitctor_default));
func_thiscall_teardown ButtonGroupInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&buttongroupinitctor_default));
func_thiscall_teardown CaviarDataInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&caviardatainitctor_default));
func_thiscall_teardown CaviarInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&caviarinitctor_default));
func_thiscall_teardown FXInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&fxinitctor_default));
func_thiscall_teardown FontInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&fontinitctor_default));
func_thiscall_teardown SpriteInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&spriteinitctor_default));
func_thiscall_teardown StringsInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&stringsinitctor_default));
func_thiscall_teardown TextureInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&textureinitctor_default));
func_thiscall_teardown TimeInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&timeinitctor_default));
func_thiscall_teardown WaveDeviceInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&wavedeviceinitctor_default));
func_thiscall_teardown WaveInitCtor = original_method<func_thiscall_teardown>(
    reinterpret_cast<unsigned long>(&waveinitctor_default));

// The opaque-storage constructors, each defaulting to
// the original the thunk calls; the hybrid redirects
// it to a recovered body at run time.
func_thiscall_teardown BaseWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00408490);
func_thiscall_teardown BattleWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00422EE0);
func_thiscall_teardown CouncWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00428620);
func_thiscall_teardown DatalinkCtorTarget =
    original_method<func_thiscall_teardown>(0x00428FC0);
func_thiscall_teardown DesignWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00434480);
func_thiscall_teardown DiploPopCtorTarget =
    original_method<func_thiscall_teardown>(0x0043EFF0);
func_thiscall_teardown DiploWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00444FC0);
func_thiscall_teardown FameWinCtorTarget =
    original_method<func_thiscall_teardown>(0x0044B200);
func_thiscall_teardown FactionArtCtorTarget =
    original_method<func_thiscall_teardown>(0x00455F10);
func_thiscall_teardown InfoWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00459500);
func_thiscall_teardown MainInterfaceCtorTarget =
    original_method<func_thiscall_teardown>(0x0045EF10);
func_opaque_ctor_i MapWinCtorTarget =
    original_method<func_opaque_ctor_i>(0x004626E0);
func_thiscall_teardown MessageWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00472190);
func_thiscall_teardown MonuWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00477C80);
func_thiscall_teardown MultiWinCtorTarget =
    original_method<func_thiscall_teardown>(0x0047A590);
func_opaque_ctor_iii NetMsgCtorTarget =
    original_method<func_opaque_ctor_iii>(0x0047ACF0);
func_thiscall_teardown NetWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00481C50);
func_thiscall_teardown NewTechWinCtorTarget =
    original_method<func_thiscall_teardown>(0x004849D0);
func_thiscall_teardown PickWinCtorTarget =
    original_method<func_thiscall_teardown>(0x0048AC10);
func_opaque_ctor_i PlanWinCtorTarget =
    original_method<func_opaque_ctor_i>(0x0048BCD0);
func_thiscall_teardown PrefWinCtorTarget =
    original_method<func_thiscall_teardown>(0x004921E0);
func_thiscall_teardown QuayleWinCtorTarget =
    original_method<func_thiscall_teardown>(0x00496810);
func_thiscall_teardown ReportIfCtorTarget =
    original_method<func_thiscall_teardown>(0x004AD170);
func_thiscall_teardown ReportWinCtorTarget =
    original_method<func_thiscall_teardown>(0x004AD6B0);
func_thiscall_teardown SocialWinCtorTarget =
    original_method<func_thiscall_teardown>(0x004AE9E0);
func_thiscall_teardown StatusWinCtorTarget =
    original_method<func_thiscall_teardown>(0x004BA1A0);
func_thiscall_teardown TutWinCtorTarget =
    original_method<func_thiscall_teardown>(0x004BA6B0);
func_thiscall_teardown WorldWinCtorTarget =
    original_method<func_thiscall_teardown>(0x004C4BF0);
func_thiscall_teardown Midi_DeviceCtorTarget =
    original_method<func_thiscall_teardown>(0x004C5740);
func_thiscall_teardown Wave_In_DeviceCtorTarget =
    original_method<func_thiscall_teardown>(0x004C5940);
func_opaque_ctor_i ConsoleCtorTarget =
    original_method<func_opaque_ctor_i>(0x0050F460);
func_thiscall_teardown NetDaemonCtorTarget =
    original_method<func_thiscall_teardown>(0x005389F0);
func_thiscall_teardown FontQueueCtorTarget =
    original_method<func_thiscall_teardown>(0x00559290);
func_thiscall_teardown MultiDebugCtorTarget =
    original_method<func_thiscall_teardown>(0x005C97F0);
func_thiscall_teardown PaletteCtorTarget =
    original_method<func_thiscall_teardown>(0x005FE2A0);


/*
Purpose: Dynamic initializer for g_ALPHAMENU_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00402F10 ??__Eg_ALPHAMENU_WAVE@@YAXXZ 0x00402F10-0x00402F26 BYTE_EXACT
// symbol    ?construct_alphamenu_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_alphamenu_wave() {
    g_ALPHAMENU_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00402F30));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR02; registers its ??__F teardown.
// ORIGINAL: 0x00404440 ??__Eg_UNUSED_SPRITE_VAR02@@YAXXZ 0x00404440-0x00404456 BYTE_EXACT
// symbol    ?construct_unused_sprite_var02@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var02() {
    g_UNUSED_SPRITE_VAR02->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404460));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR11; registers its ??__F teardown.
// ORIGINAL: 0x00404470 ??__Eg_UNUSED_SPRITE_VAR11@@YAXXZ 0x00404470-0x00404486 BYTE_EXACT
// symbol    ?construct_unused_sprite_var11@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var11() {
    g_UNUSED_SPRITE_VAR11->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404490));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR06; registers its ??__F teardown.
// ORIGINAL: 0x004044A0 ??__Eg_UNUSED_SPRITE_VAR06@@YAXXZ 0x004044A0-0x004044B6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var06@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var06() {
    g_UNUSED_SPRITE_VAR06->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004044C0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR09; registers its ??__F teardown.
// ORIGINAL: 0x004044D0 ??__Eg_UNUSED_SPRITE_VAR09@@YAXXZ 0x004044D0-0x004044E6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var09@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var09() {
    g_UNUSED_SPRITE_VAR09->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004044F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR21; registers its ??__F teardown.
// ORIGINAL: 0x00404500 ??__Eg_UNUSED_SPRITE_VAR21@@YAXXZ 0x00404500-0x00404516 BYTE_EXACT
// symbol    ?construct_unused_sprite_var21@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var21() {
    g_UNUSED_SPRITE_VAR21->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404520));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR05; registers its ??__F teardown.
// ORIGINAL: 0x00404530 ??__Eg_UNUSED_SPRITE_VAR05@@YAXXZ 0x00404530-0x00404546 BYTE_EXACT
// symbol    ?construct_unused_sprite_var05@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var05() {
    g_UNUSED_SPRITE_VAR05->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404550));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR08; registers its ??__F teardown.
// ORIGINAL: 0x00404560 ??__Eg_UNUSED_SPRITE_VAR08@@YAXXZ 0x00404560-0x00404576 BYTE_EXACT
// symbol    ?construct_unused_sprite_var08@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var08() {
    g_UNUSED_SPRITE_VAR08->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404580));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR04; registers its ??__F teardown.
// ORIGINAL: 0x00404590 ??__Eg_UNUSED_SPRITE_VAR04@@YAXXZ 0x00404590-0x004045A6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var04@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var04() {
    g_UNUSED_SPRITE_VAR04->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004045B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR01; registers its ??__F teardown.
// ORIGINAL: 0x004045C0 ??__Eg_UNUSED_SPRITE_VAR01@@YAXXZ 0x004045C0-0x004045D6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var01@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var01() {
    g_UNUSED_SPRITE_VAR01->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004045E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR18; registers its ??__F teardown.
// ORIGINAL: 0x004045F0 ??__Eg_UNUSED_SPRITE_VAR18@@YAXXZ 0x004045F0-0x00404606 BYTE_EXACT
// symbol    ?construct_unused_sprite_var18@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var18() {
    g_UNUSED_SPRITE_VAR18->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404610));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR03; registers its ??__F teardown.
// ORIGINAL: 0x00404620 ??__Eg_UNUSED_SPRITE_VAR03@@YAXXZ 0x00404620-0x00404636 BYTE_EXACT
// symbol    ?construct_unused_sprite_var03@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var03() {
    g_UNUSED_SPRITE_VAR03->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404640));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR20; registers its ??__F teardown.
// ORIGINAL: 0x00404650 ??__Eg_UNUSED_SPRITE_VAR20@@YAXXZ 0x00404650-0x00404666 BYTE_EXACT
// symbol    ?construct_unused_sprite_var20@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var20() {
    g_UNUSED_SPRITE_VAR20->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404670));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR16; registers its ??__F teardown.
// ORIGINAL: 0x00404680 ??__Eg_UNUSED_SPRITE_VAR16@@YAXXZ 0x00404680-0x00404696 BYTE_EXACT
// symbol    ?construct_unused_sprite_var16@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var16() {
    g_UNUSED_SPRITE_VAR16->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004046A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR14; registers its ??__F teardown.
// ORIGINAL: 0x004046B0 ??__Eg_UNUSED_SPRITE_VAR14@@YAXXZ 0x004046B0-0x004046C6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var14@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var14() {
    g_UNUSED_SPRITE_VAR14->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004046D0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR22; registers its ??__F teardown.
// ORIGINAL: 0x004046E0 ??__Eg_UNUSED_SPRITE_VAR22@@YAXXZ 0x004046E0-0x004046F6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var22@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var22() {
    g_UNUSED_SPRITE_VAR22->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404700));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR10; registers its ??__F teardown.
// ORIGINAL: 0x00404710 ??__Eg_UNUSED_SPRITE_VAR10@@YAXXZ 0x00404710-0x00404726 BYTE_EXACT
// symbol    ?construct_unused_sprite_var10@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var10() {
    g_UNUSED_SPRITE_VAR10->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404730));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR15; registers its ??__F teardown.
// ORIGINAL: 0x00404740 ??__Eg_UNUSED_SPRITE_VAR15@@YAXXZ 0x00404740-0x00404756 BYTE_EXACT
// symbol    ?construct_unused_sprite_var15@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var15() {
    g_UNUSED_SPRITE_VAR15->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404760));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR13; registers its ??__F teardown.
// ORIGINAL: 0x00404770 ??__Eg_UNUSED_SPRITE_VAR13@@YAXXZ 0x00404770-0x00404786 BYTE_EXACT
// symbol    ?construct_unused_sprite_var13@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var13() {
    g_UNUSED_SPRITE_VAR13->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404790));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR17; registers its ??__F teardown.
// ORIGINAL: 0x004047A0 ??__Eg_UNUSED_SPRITE_VAR17@@YAXXZ 0x004047A0-0x004047B6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var17@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var17() {
    g_UNUSED_SPRITE_VAR17->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004047C0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR19; registers its ??__F teardown.
// ORIGINAL: 0x004047D0 ??__Eg_UNUSED_SPRITE_VAR19@@YAXXZ 0x004047D0-0x004047E6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var19@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var19() {
    g_UNUSED_SPRITE_VAR19->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004047F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR12; registers its ??__F teardown.
// ORIGINAL: 0x00404800 ??__Eg_UNUSED_SPRITE_VAR12@@YAXXZ 0x00404800-0x00404816 BYTE_EXACT
// symbol    ?construct_unused_sprite_var12@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var12() {
    g_UNUSED_SPRITE_VAR12->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404820));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR07; registers its ??__F teardown.
// ORIGINAL: 0x00404830 ??__Eg_UNUSED_SPRITE_VAR07@@YAXXZ 0x00404830-0x00404846 BYTE_EXACT
// symbol    ?construct_unused_sprite_var07@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var07() {
    g_UNUSED_SPRITE_VAR07->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00404850));
}

/*
Purpose: Dynamic initializer for g_BASEWIN; registers its ??__F teardown.
// ORIGINAL: 0x004083B0 ??__Eg_BASEWIN@@YAXXZ 0x004083B0-0x004083C6
// symbol    ?construct_basewin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00408490 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_basewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x006A7628))->*BaseWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004083D0));
}

/*
Purpose: Dynamic initializer for g_BASEWIN_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x004083E0 ??__Eg_BASEWIN_WAVE@@YAXXZ 0x004083E0-0x004083F6 BYTE_EXACT
// symbol    ?construct_basewin_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_basewin_wave() {
    g_BASEWIN_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00408400));
}

/*
Purpose: Dynamic initializer for g_BattleWin; registers its ??__F teardown.
// ORIGINAL: 0x004219B0 ??__Eg_BattleWin@@YAXXZ 0x004219B0-0x004219C6
// symbol    ?construct_battlewin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00422EE0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battlewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x006EEED8))->*BattleWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004219D0));
}

/*
Purpose: Dynamic initializer for g_COUNCWIN; registers its ??__F teardown.
// ORIGINAL: 0x004243A0 ??__Eg_COUNCWIN@@YAXXZ 0x004243A0-0x004243B6
// symbol    ?construct_councwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00428620 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_councwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x006FEC80))->*CouncWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004243C0));
}

/*
Purpose: Dynamic initializer for g_CREDITS_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00428750 ??__Eg_CREDITS_WAVE@@YAXXZ 0x00428750-0x00428766 BYTE_EXACT
// symbol    ?construct_credits_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_credits_wave() {
    g_CREDITS_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00428770));
}

/*
Purpose: Dynamic initializer for g_DATALINK; registers its ??__F teardown.
// ORIGINAL: 0x00428F90 ??__Eg_DATALINK@@YAXXZ 0x00428F90-0x00428FA6
// symbol    ?construct_datalink@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00428FC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_datalink() {
    (ORIGINAL(reinterpret_cast<void *>(0x00703EA0))->*DatalinkCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00428FB0));
}

/*
Purpose: Dynamic initializer for g_DESIGNWIN; registers its ??__F teardown.
// ORIGINAL: 0x00432850 ??__Eg_DESIGNWIN@@YAXXZ 0x00432850-0x00432866
// symbol    ?construct_designwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00434480 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_designwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0071F2B0))->*DesignWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00432870));
}

/*
Purpose: Dynamic initializer for g_DESIGNWIN_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00432880 ??__Eg_DESIGNWIN_WAVE@@YAXXZ 0x00432880-0x00432896 BYTE_EXACT
// symbol    ?construct_designwin_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_designwin_wave() {
    g_DESIGNWIN_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004328A0));
}

/*
Purpose: Dynamic initializer for g_DIPLOPOP; registers its ??__F teardown.
// ORIGINAL: 0x0043EFC0 ??__Eg_DIPLOPOP@@YAXXZ 0x0043EFC0-0x0043EFD6
// symbol    ?construct_diplopop@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0043EFF0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_diplopop() {
    (ORIGINAL(reinterpret_cast<void *>(0x00733990))->*DiploPopCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0043EFE0));
}

/*
Purpose: Dynamic initializer for g_DIPLOWIN; registers its ??__F teardown.
// ORIGINAL: 0x00440F00 ??__Eg_DIPLOWIN@@YAXXZ 0x00440F00-0x00440F16
// symbol    ?construct_diplowin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00444FC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_diplowin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0073ACD8))->*DiploWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00440F20));
}

/*
Purpose: Dynamic initializer for g_CPU_WAVES; registers its ??__F teardown.
// ORIGINAL: 0x00445450 ??__Eg_CPU_WAVES@@YAXXZ 0x00445450-0x00445474
// symbol    ?construct_cpu_waves@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_cpu_waves() {
    VectorCtorIterator(g_CPU_WAVES, 0x6C, 45, WaveElementCtor, WaveElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00445480));
}

/*
Purpose: Dynamic initializer for g_MENU_UP_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x004454A0 ??__Eg_MENU_UP_WAVE@@YAXXZ 0x004454A0-0x004454B6 BYTE_EXACT
// symbol    ?construct_menu_up_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_menu_up_wave() {
    g_MENU_UP_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004454C0));
}

/*
Purpose: Dynamic initializer for g_MENU_DOWN_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x004454D0 ??__Eg_MENU_DOWN_WAVE@@YAXXZ 0x004454D0-0x004454E6 BYTE_EXACT
// symbol    ?construct_menu_down_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_menu_down_wave() {
    g_MENU_DOWN_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004454F0));
}

/*
Purpose: Dynamic initializer for g_SCOOT_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00445500 ??__Eg_SCOOT_WAVE@@YAXXZ 0x00445500-0x00445516 BYTE_EXACT
// symbol    ?construct_scoot_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scoot_wave() {
    g_SCOOT_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00445520));
}

/*
Purpose: Dynamic initializer for g_OK_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00445530 ??__Eg_OK_WAVE@@YAXXZ 0x00445530-0x00445546 BYTE_EXACT
// symbol    ?construct_ok_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ok_wave() {
    g_OK_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00445550));
}

/*
Purpose: Dynamic initializer for g_PASSOVER_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00445560 ??__Eg_PASSOVER_WAVE@@YAXXZ 0x00445560-0x00445576 BYTE_EXACT
// symbol    ?construct_passover_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_passover_wave() {
    g_PASSOVER_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00445580));
}

/*
Purpose: Dynamic initializer for g_FX; registers its ??__F teardown.
// ORIGINAL: 0x00445590 ??__Eg_FX@@YAXXZ 0x00445590-0x004455A6 BYTE_EXACT
// symbol    ?construct_fx@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004482A0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fx() {
    g_FX->FX::FX();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004455B0));
}

/*
Purpose: Dynamic initializer for g_AMBIENCE; registers its ??__F teardown.
// ORIGINAL: 0x004455C0 ??__Eg_AMBIENCE@@YAXXZ 0x004455C0-0x004455D6
// symbol    ?construct_ambience@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C8460 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ambience() {
    g_AMBIENCE->Ambience::Ambience();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004455E0));
}

/*
Purpose: Dynamic initializer for g_FAMEWIN; registers its ??__F teardown.
// ORIGINAL: 0x004483D0 ??__Eg_FAMEWIN@@YAXXZ 0x004483D0-0x004483E6
// symbol    ?construct_famewin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0044B200 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_famewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0074DAF8))->*FameWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004483F0));
}

/*
Purpose: Dynamic initializer for g_PCX_PARSE_TEMP_BUFFER1; registers its ??__F teardown.
// ORIGINAL: 0x0044CC20 ??__Eg_PCX_PARSE_TEMP_BUFFER1@@YAXXZ 0x0044CC20-0x0044CC36 BYTE_EXACT
// symbol    ?construct_pcx_parse_temp_buffer1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_pcx_parse_temp_buffer1() {
    g_PCX_PARSE_TEMP_BUFFER1->Buffer::Buffer();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CC40));
}

/*
Purpose: Dynamic initializer for FactionArt; registers its ??__F teardown.
// ORIGINAL: 0x0044CC50 ??__EFactionArt@@YAXXZ 0x0044CC50-0x0044CC77
// symbol    ?construct_factionart@@YAXXZ
// size      39 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_factionart() {
    VectorCtorIterator(reinterpret_cast<void *>(0x0078E978), 0x65C, 8,
                       FactionArtCtorTarget, FactionArtElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CC80));
}

/*
Purpose: Dynamic initializer for g_IFACE_CLOSE_X_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044CCA0 ??__Eg_IFACE_CLOSE_X_SPRITES@@YAXXZ 0x0044CCA0-0x0044CCC4
// symbol    ?construct_iface_close_x_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_close_x_sprites() {
    VectorCtorIterator(g_IFACE_CLOSE_X_SPRITES, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CCD0));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES1; registers its ??__F teardown.
// ORIGINAL: 0x0044CCF0 ??__Eg_IFACE_BOX_SPRITES1@@YAXXZ 0x0044CCF0-0x0044CD14
// symbol    ?construct_iface_box_sprites1@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites1() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES1, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CD20));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES2; registers its ??__F teardown.
// ORIGINAL: 0x0044CD40 ??__Eg_IFACE_BOX_SPRITES2@@YAXXZ 0x0044CD40-0x0044CD64
// symbol    ?construct_iface_box_sprites2@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites2() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES2, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CD70));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES3; registers its ??__F teardown.
// ORIGINAL: 0x0044CD90 ??__Eg_IFACE_BOX_SPRITES3@@YAXXZ 0x0044CD90-0x0044CDB4
// symbol    ?construct_iface_box_sprites3@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites3() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES3, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CDC0));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES4; registers its ??__F teardown.
// ORIGINAL: 0x0044CDE0 ??__Eg_IFACE_BOX_SPRITES4@@YAXXZ 0x0044CDE0-0x0044CE04
// symbol    ?construct_iface_box_sprites4@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites4() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES4, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CE10));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES5; registers its ??__F teardown.
// ORIGINAL: 0x0044CE30 ??__Eg_IFACE_BOX_SPRITES5@@YAXXZ 0x0044CE30-0x0044CE54
// symbol    ?construct_iface_box_sprites5@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites5() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES5, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CE60));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES6; registers its ??__F teardown.
// ORIGINAL: 0x0044CE80 ??__Eg_IFACE_BOX_SPRITES6@@YAXXZ 0x0044CE80-0x0044CEA4
// symbol    ?construct_iface_box_sprites6@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites6() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES6, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CEB0));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES7; registers its ??__F teardown.
// ORIGINAL: 0x0044CED0 ??__Eg_IFACE_BOX_SPRITES7@@YAXXZ 0x0044CED0-0x0044CEF4
// symbol    ?construct_iface_box_sprites7@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites7() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES7, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CF00));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES8; registers its ??__F teardown.
// ORIGINAL: 0x0044CF20 ??__Eg_IFACE_BOX_SPRITES8@@YAXXZ 0x0044CF20-0x0044CF44
// symbol    ?construct_iface_box_sprites8@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites8() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES8, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CF50));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITE_BUFFERS; registers its ??__F teardown.
// ORIGINAL: 0x0044CF70 ??__Eg_IFACE_BOX_SPRITE_BUFFERS@@YAXXZ 0x0044CF70-0x0044CF97
// symbol    ?construct_iface_box_sprite_buffers@@YAXXZ
// size      39 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprite_buffers() {
    VectorCtorIterator(g_IFACE_BOX_SPRITE_BUFFERS, 0x588, 51, BufferElementCtor, BufferElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CFA0));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044CFC0 ??__Eg_IFACE_STD_POPUPS_TOP_LEFT_SPRITE@@YAXXZ 0x0044CFC0-0x0044CFD6 BYTE_EXACT
// symbol    ?construct_iface_std_popups_top_left_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_top_left_sprite() {
    g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044CFE0));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044CFF0 ??__Eg_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE@@YAXXZ 0x0044CFF0-0x0044D006 BYTE_EXACT
// symbol    ?construct_iface_std_popups_top_right_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_top_right_sprite() {
    g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D010));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044D020 ??__Eg_IFACE_STD_POPUPS_BOT_LEFT_SPRITE@@YAXXZ 0x0044D020-0x0044D036 BYTE_EXACT
// symbol    ?construct_iface_std_popups_bot_left_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_bot_left_sprite() {
    g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D040));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044D050 ??__Eg_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE@@YAXXZ 0x0044D050-0x0044D066 BYTE_EXACT
// symbol    ?construct_iface_std_popups_bot_right_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_bot_right_sprite() {
    g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D070));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_TOP_MID_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044D080 ??__Eg_IFACE_STD_POPUPS_TOP_MID_SPRITE@@YAXXZ 0x0044D080-0x0044D096 BYTE_EXACT
// symbol    ?construct_iface_std_popups_top_mid_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_top_mid_sprite() {
    g_IFACE_STD_POPUPS_TOP_MID_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D0A0));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_BOT_MID_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044D0B0 ??__Eg_IFACE_STD_POPUPS_BOT_MID_SPRITE@@YAXXZ 0x0044D0B0-0x0044D0C6 BYTE_EXACT
// symbol    ?construct_iface_std_popups_bot_mid_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_bot_mid_sprite() {
    g_IFACE_STD_POPUPS_BOT_MID_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D0D0));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_MID_LEFT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044D0E0 ??__Eg_IFACE_STD_POPUPS_MID_LEFT_SPRITE@@YAXXZ 0x0044D0E0-0x0044D0F6 BYTE_EXACT
// symbol    ?construct_iface_std_popups_mid_left_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_mid_left_sprite() {
    g_IFACE_STD_POPUPS_MID_LEFT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D100));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044D110 ??__Eg_IFACE_STD_POPUPS_MID_RIGHT_SPRITE@@YAXXZ 0x0044D110-0x0044D126 BYTE_EXACT
// symbol    ?construct_iface_std_popups_mid_right_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_mid_right_sprite() {
    g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D130));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_MIDDLE_BUFFER; registers its ??__F teardown.
// ORIGINAL: 0x0044D140 ??__Eg_IFACE_STD_POPUPS_MIDDLE_BUFFER@@YAXXZ 0x0044D140-0x0044D156 BYTE_EXACT
// symbol    ?construct_iface_std_popups_middle_buffer@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_middle_buffer() {
    g_IFACE_STD_POPUPS_MIDDLE_BUFFER->Buffer::Buffer();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D160));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR1; registers its ??__F teardown.
// ORIGINAL: 0x0044D170 ??__Eg_UNUSED_CAVIARDATA_VAR1@@YAXXZ 0x0044D170-0x0044D186 BYTE_EXACT
// symbol    ?construct_unused_caviardata_var1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var1() {
    g_UNUSED_CAVIARDATA_VAR1->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D190));
}

/*
Purpose: Dynamic initializer for g_SSF_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D1A0 ??__Eg_SSF_CAVIARDATA@@YAXXZ 0x0044D1A0-0x0044D1B6 BYTE_EXACT
// symbol    ?construct_ssf_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ssf_caviardata() {
    g_SSF_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D1C0));
}

/*
Purpose: Dynamic initializer for g_SDP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D1D0 ??__Eg_SDP_CAVIARDATA@@YAXXZ 0x0044D1D0-0x0044D1E6 BYTE_EXACT
// symbol    ?construct_sdp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sdp_caviardata() {
    g_SDP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D1F0));
}

/*
Purpose: Dynamic initializer for g_SAS_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D200 ??__Eg_SAS_CAVIARDATA@@YAXXZ 0x0044D200-0x0044D216 BYTE_EXACT
// symbol    ?construct_sas_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sas_caviardata() {
    g_SAS_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D220));
}

/*
Purpose: Dynamic initializer for g_SCD_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D230 ??__Eg_SCD_CAVIARDATA@@YAXXZ 0x0044D230-0x0044D246 BYTE_EXACT
// symbol    ?construct_scd_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scd_caviardata() {
    g_SCD_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D250));
}

/*
Purpose: Dynamic initializer for g_SCJ_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D260 ??__Eg_SCJ_CAVIARDATA@@YAXXZ 0x0044D260-0x0044D276 BYTE_EXACT
// symbol    ?construct_scj_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scj_caviardata() {
    g_SCJ_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D280));
}

/*
Purpose: Dynamic initializer for g_SAGS_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D290 ??__Eg_SAGS_CAVIARDATA@@YAXXZ 0x0044D290-0x0044D2A6 BYTE_EXACT
// symbol    ?construct_sags_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sags_caviardata() {
    g_SAGS_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D2B0));
}

/*
Purpose: Dynamic initializer for g_SFT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D2C0 ??__Eg_SFT_CAVIARDATA@@YAXXZ 0x0044D2C0-0x0044D2D6 BYTE_EXACT
// symbol    ?construct_sft_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sft_caviardata() {
    g_SFT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D2E0));
}

/*
Purpose: Dynamic initializer for g_VHR_CAVIARDATA1; registers its ??__F teardown.
// ORIGINAL: 0x0044D2F0 ??__Eg_VHR_CAVIARDATA1@@YAXXZ 0x0044D2F0-0x0044D306 BYTE_EXACT
// symbol    ?construct_vhr_caviardata1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhr_caviardata1() {
    g_VHR_CAVIARDATA1->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D310));
}

/*
Purpose: Dynamic initializer for g_SHT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D320 ??__Eg_SHT_CAVIARDATA@@YAXXZ 0x0044D320-0x0044D336 BYTE_EXACT
// symbol    ?construct_sht_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sht_caviardata() {
    g_SHT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D340));
}

/*
Purpose: Dynamic initializer for g_SRB_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D350 ??__Eg_SRB_CAVIARDATA@@YAXXZ 0x0044D350-0x0044D366 BYTE_EXACT
// symbol    ?construct_srb_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_srb_caviardata() {
    g_SRB_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D370));
}

/*
Purpose: Dynamic initializer for g_ASAS_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D380 ??__Eg_ASAS_CAVIARDATA@@YAXXZ 0x0044D380-0x0044D396 BYTE_EXACT
// symbol    ?construct_asas_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_asas_caviardata() {
    g_ASAS_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D3A0));
}

/*
Purpose: Dynamic initializer for g_RESLASER_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D3B0 ??__Eg_RESLASER_CAVIARDATA@@YAXXZ 0x0044D3B0-0x0044D3C6 BYTE_EXACT
// symbol    ?construct_reslaser_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_reslaser_caviardata() {
    g_RESLASER_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D3D0));
}

/*
Purpose: Dynamic initializer for g_RESBOLT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D3E0 ??__Eg_RESBOLT_CAVIARDATA@@YAXXZ 0x0044D3E0-0x0044D3F6 BYTE_EXACT
// symbol    ?construct_resbolt_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_resbolt_caviardata() {
    g_RESBOLT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D400));
}

/*
Purpose: Dynamic initializer for g_FUNLOAD_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D410 ??__Eg_FUNLOAD_CAVIARDATA@@YAXXZ 0x0044D410-0x0044D426 BYTE_EXACT
// symbol    ?construct_funload_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_funload_caviardata() {
    g_FUNLOAD_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D430));
}

/*
Purpose: Dynamic initializer for g_TECLOAD_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D440 ??__Eg_TECLOAD_CAVIARDATA@@YAXXZ 0x0044D440-0x0044D456 BYTE_EXACT
// symbol    ?construct_tecload_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_tecload_caviardata() {
    g_TECLOAD_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D460));
}

/*
Purpose: Dynamic initializer for g_SP_DISSWAVE_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D470 ??__Eg_SP_DISSWAVE_CAVIARDATA@@YAXXZ 0x0044D470-0x0044D486 BYTE_EXACT
// symbol    ?construct_sp_disswave_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sp_disswave_caviardata() {
    g_SP_DISSWAVE_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D490));
}

/*
Purpose: Dynamic initializer for g_SP_MARINED_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D4A0 ??__Eg_SP_MARINED_CAVIARDATA@@YAXXZ 0x0044D4A0-0x0044D4B6 BYTE_EXACT
// symbol    ?construct_sp_marined_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sp_marined_caviardata() {
    g_SP_MARINED_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D4C0));
}

/*
Purpose: Dynamic initializer for g_SP_NANOO_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D4D0 ??__Eg_SP_NANOO_CAVIARDATA@@YAXXZ 0x0044D4D0-0x0044D4E6 BYTE_EXACT
// symbol    ?construct_sp_nanoo_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sp_nanoo_caviardata() {
    g_SP_NANOO_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D4F0));
}

/*
Purpose: Dynamic initializer for g_SP_SOPORIFIC_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D500 ??__Eg_SP_SOPORIFIC_CAVIARDATA@@YAXXZ 0x0044D500-0x0044D516 BYTE_EXACT
// symbol    ?construct_sp_soporific_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sp_soporific_caviardata() {
    g_SP_SOPORIFIC_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D520));
}

/*
Purpose: Dynamic initializer for g_AA01_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D530 ??__Eg_AA01_CAVIARDATA@@YAXXZ 0x0044D530-0x0044D546 BYTE_EXACT
// symbol    ?construct_aa01_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_aa01_caviardata() {
    g_AA01_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D550));
}

/*
Purpose: Dynamic initializer for g_AA_ROVER_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D560 ??__Eg_AA_ROVER_CAVIARDATA@@YAXXZ 0x0044D560-0x0044D576 BYTE_EXACT
// symbol    ?construct_aa_rover_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_aa_rover_caviardata() {
    g_AA_ROVER_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D580));
}

/*
Purpose: Dynamic initializer for g_AA_WING_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D590 ??__Eg_AA_WING_CAVIARDATA@@YAXXZ 0x0044D590-0x0044D5B4
// symbol    ?construct_aa_wing_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_aa_wing_caviardata() {
    VectorCtorIterator(g_AA_WING_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D5C0));
}

/*
Purpose: Dynamic initializer for g_ACP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D5E0 ??__Eg_ACP_CAVIARDATA@@YAXXZ 0x0044D5E0-0x0044D604
// symbol    ?construct_acp_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_acp_caviardata() {
    VectorCtorIterator(g_ACP_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D610));
}

/*
Purpose: Dynamic initializer for g_AX_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D630 ??__Eg_AX_CAVIARDATA@@YAXXZ 0x0044D630-0x0044D646 BYTE_EXACT
// symbol    ?construct_ax_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ax_caviardata() {
    g_AX_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D650));
}

/*
Purpose: Dynamic initializer for g_AA_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D660 ??__Eg_AA_CAVIARDATA@@YAXXZ 0x0044D660-0x0044D676 BYTE_EXACT
// symbol    ?construct_aa_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_aa_caviardata() {
    g_AA_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D680));
}

/*
Purpose: Dynamic initializer for g_ACOLPOD_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D690 ??__Eg_ACOLPOD_CAVIARDATA@@YAXXZ 0x0044D690-0x0044D6A6 BYTE_EXACT
// symbol    ?construct_acolpod_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_acolpod_caviardata() {
    g_ACOLPOD_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D6B0));
}

/*
Purpose: Dynamic initializer for g_AT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D6C0 ??__Eg_AT_CAVIARDATA@@YAXXZ 0x0044D6C0-0x0044D6D6 BYTE_EXACT
// symbol    ?construct_at_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_at_caviardata() {
    g_AT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D6E0));
}

/*
Purpose: Dynamic initializer for g_VTA_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D6F0 ??__Eg_VTA_CAVIARDATA@@YAXXZ 0x0044D6F0-0x0044D706 BYTE_EXACT
// symbol    ?construct_vta_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vta_caviardata() {
    g_VTA_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D710));
}

/*
Purpose: Dynamic initializer for g_ATP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D720 ??__Eg_ATP_CAVIARDATA@@YAXXZ 0x0044D720-0x0044D736 BYTE_EXACT
// symbol    ?construct_atp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_atp_caviardata() {
    g_ATP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D740));
}

/*
Purpose: Dynamic initializer for g_SSFA_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D750 ??__Eg_SSFA_CAVIARDATA@@YAXXZ 0x0044D750-0x0044D766 BYTE_EXACT
// symbol    ?construct_ssfa_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ssfa_caviardata() {
    g_SSFA_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D770));
}

/*
Purpose: Dynamic initializer for g_SFTA_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D780 ??__Eg_SFTA_CAVIARDATA@@YAXXZ 0x0044D780-0x0044D796 BYTE_EXACT
// symbol    ?construct_sfta_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sfta_caviardata() {
    g_SFTA_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D7A0));
}

/*
Purpose: Dynamic initializer for g_VW00_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D7B0 ??__Eg_VW00_CAVIARDATA@@YAXXZ 0x0044D7B0-0x0044D7C6 BYTE_EXACT
// symbol    ?construct_vw00_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vw00_caviardata() {
    g_VW00_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D7D0));
}

/*
Purpose: Dynamic initializer for g_VIPTAWL_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D7E0 ??__Eg_VIPTAWL_CAVIARDATA@@YAXXZ 0x0044D7E0-0x0044D7F6 BYTE_EXACT
// symbol    ?construct_viptawl_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_viptawl_caviardata() {
    g_VIPTAWL_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D800));
}

/*
Purpose: Dynamic initializer for g_VIPTASGN_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D810 ??__Eg_VIPTASGN_CAVIARDATA@@YAXXZ 0x0044D810-0x0044D826 BYTE_EXACT
// symbol    ?construct_viptasgn_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_viptasgn_caviardata() {
    g_VIPTASGN_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D830));
}

/*
Purpose: Dynamic initializer for g_VIPTAPSI_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D840 ??__Eg_VIPTAPSI_CAVIARDATA@@YAXXZ 0x0044D840-0x0044D856 BYTE_EXACT
// symbol    ?construct_viptapsi_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_viptapsi_caviardata() {
    g_VIPTAPSI_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D860));
}

/*
Purpose: Dynamic initializer for g_VIPTR_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D870 ??__Eg_VIPTR_CAVIARDATA@@YAXXZ 0x0044D870-0x0044D894
// symbol    ?construct_viptr_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_viptr_caviardata() {
    VectorCtorIterator(g_VIPTR_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D8A0));
}

/*
Purpose: Dynamic initializer for g_VIPTA_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D8C0 ??__Eg_VIPTA_CAVIARDATA@@YAXXZ 0x0044D8C0-0x0044D8E4
// symbol    ?construct_vipta_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vipta_caviardata() {
    VectorCtorIterator(g_VIPTA_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D8F0));
}

/*
Purpose: Dynamic initializer for g_PTMOD_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D910 ??__Eg_PTMOD_CAVIARDATA@@YAXXZ 0x0044D910-0x0044D926 BYTE_EXACT
// symbol    ?construct_ptmod_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ptmod_caviardata() {
    g_PTMOD_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D930));
}

/*
Purpose: Dynamic initializer for g_VB_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D940 ??__Eg_VB_CAVIARDATA@@YAXXZ 0x0044D940-0x0044D956 BYTE_EXACT
// symbol    ?construct_vb_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vb_caviardata() {
    g_VB_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D960));
}

/*
Purpose: Dynamic initializer for g_VBP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D970 ??__Eg_VBP_CAVIARDATA@@YAXXZ 0x0044D970-0x0044D986 BYTE_EXACT
// symbol    ?construct_vbp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vbp_caviardata() {
    g_VBP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D990));
}

/*
Purpose: Dynamic initializer for g_VGMC_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D9A0 ??__Eg_VGMC_CAVIARDATA@@YAXXZ 0x0044D9A0-0x0044D9B6 BYTE_EXACT
// symbol    ?construct_vgmc_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgmc_caviardata() {
    g_VGMC_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D9C0));
}

/*
Purpose: Dynamic initializer for g_VGMCP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044D9D0 ??__Eg_VGMCP_CAVIARDATA@@YAXXZ 0x0044D9D0-0x0044D9E6 BYTE_EXACT
// symbol    ?construct_vgmcp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgmcp_caviardata() {
    g_VGMCP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044D9F0));
}

/*
Purpose: Dynamic initializer for g_VLIGHTS_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DA00 ??__Eg_VLIGHTS_CAVIARDATA@@YAXXZ 0x0044DA00-0x0044DA16 BYTE_EXACT
// symbol    ?construct_vlights_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vlights_caviardata() {
    g_VLIGHTS_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DA20));
}

/*
Purpose: Dynamic initializer for g_VPT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DA30 ??__Eg_VPT_CAVIARDATA@@YAXXZ 0x0044DA30-0x0044DA46 BYTE_EXACT
// symbol    ?construct_vpt_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vpt_caviardata() {
    g_VPT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DA50));
}

/*
Purpose: Dynamic initializer for g_A_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DA60 ??__Eg_A_CAVIARDATA@@YAXXZ 0x0044DA60-0x0044DA76 BYTE_EXACT
// symbol    ?construct_a_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_a_caviardata() {
    g_A_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DA80));
}

/*
Purpose: Dynamic initializer for g_APWALL_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DA90 ??__Eg_APWALL_CAVIARDATA@@YAXXZ 0x0044DA90-0x0044DAA6 BYTE_EXACT
// symbol    ?construct_apwall_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_apwall_caviardata() {
    g_APWALL_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DAB0));
}

/*
Purpose: Dynamic initializer for g_ASGEN_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DAC0 ??__Eg_ASGEN_CAVIARDATA@@YAXXZ 0x0044DAC0-0x0044DAD6 BYTE_EXACT
// symbol    ?construct_asgen_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_asgen_caviardata() {
    g_ASGEN_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DAE0));
}

/*
Purpose: Dynamic initializer for g_APSID_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DAF0 ??__Eg_APSID_CAVIARDATA@@YAXXZ 0x0044DAF0-0x0044DB06 BYTE_EXACT
// symbol    ?construct_apsid_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_apsid_caviardata() {
    g_APSID_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DB10));
}

/*
Purpose: Dynamic initializer for g_VA01_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DB20 ??__Eg_VA01_CAVIARDATA@@YAXXZ 0x0044DB20-0x0044DB36 BYTE_EXACT
// symbol    ?construct_va01_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_va01_caviardata() {
    g_VA01_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DB40));
}

/*
Purpose: Dynamic initializer for g_VR_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DB50 ??__Eg_VR_CAVIARDATA@@YAXXZ 0x0044DB50-0x0044DB74
// symbol    ?construct_vr_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vr_caviardata() {
    VectorCtorIterator(g_VR_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DB80));
}

/*
Purpose: Dynamic initializer for g_VHR_CAVIARDATA2; registers its ??__F teardown.
// ORIGINAL: 0x0044DBA0 ??__Eg_VHR_CAVIARDATA2@@YAXXZ 0x0044DBA0-0x0044DBB6 BYTE_EXACT
// symbol    ?construct_vhr_caviardata2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhr_caviardata2() {
    g_VHR_CAVIARDATA2->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DBC0));
}

/*
Purpose: Dynamic initializer for g_VRC_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DBD0 ??__Eg_VRC_CAVIARDATA@@YAXXZ 0x0044DBD0-0x0044DBF4
// symbol    ?construct_vrc_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vrc_caviardata() {
    VectorCtorIterator(g_VRC_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DC00));
}

/*
Purpose: Dynamic initializer for g_VI_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DC20 ??__Eg_VI_CAVIARDATA@@YAXXZ 0x0044DC20-0x0044DC36 BYTE_EXACT
// symbol    ?construct_vi_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vi_caviardata() {
    g_VI_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DC40));
}

/*
Purpose: Dynamic initializer for g_VGMT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DC50 ??__Eg_VGMT_CAVIARDATA@@YAXXZ 0x0044DC50-0x0044DC66 BYTE_EXACT
// symbol    ?construct_vgmt_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgmt_caviardata() {
    g_VGMT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DC70));
}

/*
Purpose: Dynamic initializer for g_VGMTP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DC80 ??__Eg_VGMTP_CAVIARDATA@@YAXXZ 0x0044DC80-0x0044DC96 BYTE_EXACT
// symbol    ?construct_vgmtp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgmtp_caviardata() {
    g_VGMTP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DCA0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR2; registers its ??__F teardown.
// ORIGINAL: 0x0044DCB0 ??__Eg_UNUSED_CAVIARDATA_VAR2@@YAXXZ 0x0044DCB0-0x0044DCC6 BYTE_EXACT
// symbol    ?construct_unused_caviardata_var2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var2() {
    g_UNUSED_CAVIARDATA_VAR2->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DCD0));
}

/*
Purpose: Dynamic initializer for g_VWNTU_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DCE0 ??__Eg_VWNTU_CAVIARDATA@@YAXXZ 0x0044DCE0-0x0044DCF6 BYTE_EXACT
// symbol    ?construct_vwntu_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vwntu_caviardata() {
    g_VWNTU_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DD00));
}

/*
Purpose: Dynamic initializer for g_VT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DD10 ??__Eg_VT_CAVIARDATA@@YAXXZ 0x0044DD10-0x0044DD26 BYTE_EXACT
// symbol    ?construct_vt_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vt_caviardata() {
    g_VT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DD30));
}

/*
Purpose: Dynamic initializer for g_DROP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DD40 ??__Eg_DROP_CAVIARDATA@@YAXXZ 0x0044DD40-0x0044DD56 BYTE_EXACT
// symbol    ?construct_drop_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_drop_caviardata() {
    g_DROP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DD60));
}

/*
Purpose: Dynamic initializer for g_DROPLET_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DD70 ??__Eg_DROPLET_CAVIARDATA@@YAXXZ 0x0044DD70-0x0044DD86 BYTE_EXACT
// symbol    ?construct_droplet_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_droplet_caviardata() {
    g_DROPLET_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DD90));
}

/*
Purpose: Dynamic initializer for g_VCL_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DDA0 ??__Eg_VCL_CAVIARDATA@@YAXXZ 0x0044DDA0-0x0044DDB6 BYTE_EXACT
// symbol    ?construct_vcl_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcl_caviardata() {
    g_VCL_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DDC0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR3; registers its ??__F teardown.
// ORIGINAL: 0x0044DDD0 ??__Eg_UNUSED_CAVIARDATA_VAR3@@YAXXZ 0x0044DDD0-0x0044DDF4
// symbol    ?construct_unused_caviardata_var3@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var3() {
    VectorCtorIterator(g_UNUSED_CAVIARDATA_VAR3, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DE00));
}

/*
Purpose: Dynamic initializer for g_VCLT00_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DE20 ??__Eg_VCLT00_CAVIARDATA@@YAXXZ 0x0044DE20-0x0044DE36 BYTE_EXACT
// symbol    ?construct_vclt00_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vclt00_caviardata() {
    g_VCLT00_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DE40));
}

/*
Purpose: Dynamic initializer for g_VHT_VBP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DE50 ??__Eg_VHT_VBP_CAVIARDATA@@YAXXZ 0x0044DE50-0x0044DE66 BYTE_EXACT
// symbol    ?construct_vht_vbp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vht_vbp_caviardata() {
    g_VHT_VBP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DE70));
}

/*
Purpose: Dynamic initializer for g_VHTP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DE80 ??__Eg_VHTP_CAVIARDATA@@YAXXZ 0x0044DE80-0x0044DE96 BYTE_EXACT
// symbol    ?construct_vhtp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhtp_caviardata() {
    g_VHTP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DEA0));
}

/*
Purpose: Dynamic initializer for g_VHTA0_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DEB0 ??__Eg_VHTA0_CAVIARDATA@@YAXXZ 0x0044DEB0-0x0044DED4
// symbol    ?construct_vhta0_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhta0_caviardata() {
    VectorCtorIterator(g_VHTA0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DEE0));
}

/*
Purpose: Dynamic initializer for g_VHTTP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DF00 ??__Eg_VHTTP_CAVIARDATA@@YAXXZ 0x0044DF00-0x0044DF16 BYTE_EXACT
// symbol    ?construct_vhttp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhttp_caviardata() {
    g_VHTTP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DF20));
}

/*
Purpose: Dynamic initializer for g_VHTTPA0_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DF30 ??__Eg_VHTTPA0_CAVIARDATA@@YAXXZ 0x0044DF30-0x0044DF54
// symbol    ?construct_vhttpa0_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhttpa0_caviardata() {
    VectorCtorIterator(g_VHTTPA0_CAVIARDATA, 0xC, 3, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DF60));
}

/*
Purpose: Dynamic initializer for g_VSP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DF80 ??__Eg_VSP_CAVIARDATA@@YAXXZ 0x0044DF80-0x0044DF96 BYTE_EXACT
// symbol    ?construct_vsp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vsp_caviardata() {
    g_VSP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DFA0));
}

/*
Purpose: Dynamic initializer for g_VSPA0_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044DFB0 ??__Eg_VSPA0_CAVIARDATA@@YAXXZ 0x0044DFB0-0x0044DFD4
// symbol    ?construct_vspa0_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vspa0_caviardata() {
    VectorCtorIterator(g_VSPA0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044DFE0));
}

/*
Purpose: Dynamic initializer for g_VSPTF_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E000 ??__Eg_VSPTF_CAVIARDATA@@YAXXZ 0x0044E000-0x0044E016 BYTE_EXACT
// symbol    ?construct_vsptf_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vsptf_caviardata() {
    g_VSPTF_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E020));
}

/*
Purpose: Dynamic initializer for g_VSPTB_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E030 ??__Eg_VSPTB_CAVIARDATA@@YAXXZ 0x0044E030-0x0044E046 BYTE_EXACT
// symbol    ?construct_vsptb_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vsptb_caviardata() {
    g_VSPTB_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E050));
}

/*
Purpose: Dynamic initializer for g_VFL_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E060 ??__Eg_VFL_CAVIARDATA@@YAXXZ 0x0044E060-0x0044E076 BYTE_EXACT
// symbol    ?construct_vfl_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vfl_caviardata() {
    g_VFL_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E080));
}

/*
Purpose: Dynamic initializer for g_VGS_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E090 ??__Eg_VGS_CAVIARDATA@@YAXXZ 0x0044E090-0x0044E0A6 BYTE_EXACT
// symbol    ?construct_vgs_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgs_caviardata() {
    g_VGS_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E0B0));
}

/*
Purpose: Dynamic initializer for g_VGSP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E0C0 ??__Eg_VGSP_CAVIARDATA@@YAXXZ 0x0044E0C0-0x0044E0D6 BYTE_EXACT
// symbol    ?construct_vgsp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgsp_caviardata() {
    g_VGSP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E0E0));
}

/*
Purpose: Dynamic initializer for g_VJTP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E0F0 ??__Eg_VJTP_CAVIARDATA@@YAXXZ 0x0044E0F0-0x0044E106 BYTE_EXACT
// symbol    ?construct_vjtp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vjtp_caviardata() {
    g_VJTP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E110));
}

/*
Purpose: Dynamic initializer for g_VJT0_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E120 ??__Eg_VJT0_CAVIARDATA@@YAXXZ 0x0044E120-0x0044E144
// symbol    ?construct_vjt0_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vjt0_caviardata() {
    VectorCtorIterator(g_VJT0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E150));
}

/*
Purpose: Dynamic initializer for g_VCU_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E170 ??__Eg_VCU_CAVIARDATA@@YAXXZ 0x0044E170-0x0044E186 BYTE_EXACT
// symbol    ?construct_vcu_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcu_caviardata() {
    g_VCU_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E190));
}

/*
Purpose: Dynamic initializer for g_VCUP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E1A0 ??__Eg_VCUP_CAVIARDATA@@YAXXZ 0x0044E1A0-0x0044E1B6 BYTE_EXACT
// symbol    ?construct_vcup_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcup_caviardata() {
    g_VCUP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E1C0));
}

/*
Purpose: Dynamic initializer for g_VCUA0_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E1D0 ??__Eg_VCUA0_CAVIARDATA@@YAXXZ 0x0044E1D0-0x0044E1F4
// symbol    ?construct_vcua0_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcua0_caviardata() {
    VectorCtorIterator(g_VCUA0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E200));
}

/*
Purpose: Dynamic initializer for g_VCUW_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E220 ??__Eg_VCUW_CAVIARDATA@@YAXXZ 0x0044E220-0x0044E236 BYTE_EXACT
// symbol    ?construct_vcuw_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcuw_caviardata() {
    g_VCUW_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E240));
}

/*
Purpose: Dynamic initializer for g_VCT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E250 ??__Eg_VCT_CAVIARDATA@@YAXXZ 0x0044E250-0x0044E266 BYTE_EXACT
// symbol    ?construct_vct_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vct_caviardata() {
    g_VCT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E270));
}

/*
Purpose: Dynamic initializer for g_VCTP_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E280 ??__Eg_VCTP_CAVIARDATA@@YAXXZ 0x0044E280-0x0044E296 BYTE_EXACT
// symbol    ?construct_vctp_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vctp_caviardata() {
    g_VCTP_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E2A0));
}

/*
Purpose: Dynamic initializer for g_VCTB_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E2B0 ??__Eg_VCTB_CAVIARDATA@@YAXXZ 0x0044E2B0-0x0044E2C6 BYTE_EXACT
// symbol    ?construct_vctb_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vctb_caviardata() {
    g_VCTB_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E2D0));
}

/*
Purpose: Dynamic initializer for g_VCT0_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E2E0 ??__Eg_VCT0_CAVIARDATA@@YAXXZ 0x0044E2E0-0x0044E304
// symbol    ?construct_vct0_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vct0_caviardata() {
    VectorCtorIterator(g_VCT0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E310));
}

/*
Purpose: Dynamic initializer for g_VWNTT_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E330 ??__Eg_VWNTT_CAVIARDATA@@YAXXZ 0x0044E330-0x0044E346 BYTE_EXACT
// symbol    ?construct_vwntt_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vwntt_caviardata() {
    g_VWNTT_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E350));
}

/*
Purpose: Dynamic initializer for g_VWNST_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E360 ??__Eg_VWNST_CAVIARDATA@@YAXXZ 0x0044E360-0x0044E376 BYTE_EXACT
// symbol    ?construct_vwnst_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vwnst_caviardata() {
    g_VWNST_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E380));
}

/*
Purpose: Dynamic initializer for g_VWNAA_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E390 ??__Eg_VWNAA_CAVIARDATA@@YAXXZ 0x0044E390-0x0044E3A6 BYTE_EXACT
// symbol    ?construct_vwnaa_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vwnaa_caviardata() {
    g_VWNAA_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E3B0));
}

/*
Purpose: Dynamic initializer for g_VW_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E3C0 ??__Eg_VW_CAVIARDATA@@YAXXZ 0x0044E3C0-0x0044E3E4
// symbol    ?construct_vw_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vw_caviardata() {
    VectorCtorIterator(g_VW_CAVIARDATA, 0xC, 16, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E3F0));
}

/*
Purpose: Dynamic initializer for g_VM_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E410 ??__Eg_VM_CAVIARDATA@@YAXXZ 0x0044E410-0x0044E426 BYTE_EXACT
// symbol    ?construct_vm_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vm_caviardata() {
    g_VM_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E430));
}

/*
Purpose: Dynamic initializer for g_VM13_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E440 ??__Eg_VM13_CAVIARDATA@@YAXXZ 0x0044E440-0x0044E456 BYTE_EXACT
// symbol    ?construct_vm13_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vm13_caviardata() {
    g_VM13_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E460));
}

/*
Purpose: Dynamic initializer for g_VPBR0_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E470 ??__Eg_VPBR0_CAVIARDATA@@YAXXZ 0x0044E470-0x0044E494
// symbol    ?construct_vpbr0_caviardata@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vpbr0_caviardata() {
    VectorCtorIterator(g_VPBR0_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E4A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR4; registers its ??__F teardown.
// ORIGINAL: 0x0044E4C0 ??__Eg_UNUSED_CAVIARDATA_VAR4@@YAXXZ 0x0044E4C0-0x0044E4E4
// symbol    ?construct_unused_caviardata_var4@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var4() {
    VectorCtorIterator(g_UNUSED_CAVIARDATA_VAR4, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E4F0));
}

/*
Purpose: Dynamic initializer for g_NW_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E510 ??__Eg_NW_CAVIARDATA@@YAXXZ 0x0044E510-0x0044E526 BYTE_EXACT
// symbol    ?construct_nw_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_nw_caviardata() {
    g_NW_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E530));
}

/*
Purpose: Dynamic initializer for g_NI_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E540 ??__Eg_NI_CAVIARDATA@@YAXXZ 0x0044E540-0x0044E556 BYTE_EXACT
// symbol    ?construct_ni_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ni_caviardata() {
    g_NI_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E560));
}

/*
Purpose: Dynamic initializer for g_NLC_CAVIARDATA; registers its ??__F teardown.
// ORIGINAL: 0x0044E570 ??__Eg_NLC_CAVIARDATA@@YAXXZ 0x0044E570-0x0044E586 BYTE_EXACT
// symbol    ?construct_nlc_caviardata@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616BC0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_nlc_caviardata() {
    g_NLC_CAVIARDATA->CaviarData::CaviarData();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E590));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR5; registers its ??__F teardown.
// ORIGINAL: 0x0044E5A0 ??__Eg_UNUSED_CAVIARDATA_VAR5@@YAXXZ 0x0044E5A0-0x0044E5C4
// symbol    ?construct_unused_caviardata_var5@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var5() {
    VectorCtorIterator(g_UNUSED_CAVIARDATA_VAR5, 0xC, 5, CaviarDataElementCtor, CaviarDataElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E5D0));
}

/*
Purpose: Dynamic initializer for g_RADIUS1_TEXTURE; registers its ??__F teardown.
// ORIGINAL: 0x0044E5F0 ??__Eg_RADIUS1_TEXTURE@@YAXXZ 0x0044E5F0-0x0044E606 BYTE_EXACT
// symbol    ?construct_radius1_texture@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619650 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radius1_texture() {
    g_RADIUS1_TEXTURE->Texture::Texture();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E610));
}

/*
Purpose: Dynamic initializer for g_RADIUS2_TEXTURE; registers its ??__F teardown.
// ORIGINAL: 0x0044E620 ??__Eg_RADIUS2_TEXTURE@@YAXXZ 0x0044E620-0x0044E636 BYTE_EXACT
// symbol    ?construct_radius2_texture@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619650 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radius2_texture() {
    g_RADIUS2_TEXTURE->Texture::Texture();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E640));
}

/*
Purpose: Dynamic initializer for g_ROCKY_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E650 ??__Eg_ROCKY_TEXTURES@@YAXXZ 0x0044E650-0x0044E674
// symbol    ?construct_rocky_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rocky_textures() {
    VectorCtorIterator(g_ROCKY_TEXTURES, 0x70, 4, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E680));
}

/*
Purpose: Dynamic initializer for g_OCEAN_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E6A0 ??__Eg_OCEAN_TEXTURES@@YAXXZ 0x0044E6A0-0x0044E6C4
// symbol    ?construct_ocean_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ocean_textures() {
    VectorCtorIterator(g_OCEAN_TEXTURES, 0x70, 2, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E6D0));
}

/*
Purpose: Dynamic initializer for g_FLAT_ARID_LAND_TEXTURE; registers its ??__F teardown.
// ORIGINAL: 0x0044E6F0 ??__Eg_FLAT_ARID_LAND_TEXTURE@@YAXXZ 0x0044E6F0-0x0044E706 BYTE_EXACT
// symbol    ?construct_flat_arid_land_texture@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619650 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_flat_arid_land_texture() {
    g_FLAT_ARID_LAND_TEXTURE->Texture::Texture();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E710));
}

/*
Purpose: Dynamic initializer for g_MOIST_LAND_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E720 ??__Eg_MOIST_LAND_TEXTURES@@YAXXZ 0x0044E720-0x0044E744
// symbol    ?construct_moist_land_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_moist_land_textures() {
    VectorCtorIterator(g_MOIST_LAND_TEXTURES, 0x70, 16, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E750));
}

/*
Purpose: Dynamic initializer for g_RAINY_LAND_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E770 ??__Eg_RAINY_LAND_TEXTURES@@YAXXZ 0x0044E770-0x0044E794
// symbol    ?construct_rainy_land_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rainy_land_textures() {
    VectorCtorIterator(g_RAINY_LAND_TEXTURES, 0x70, 16, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E7A0));
}

/*
Purpose: Dynamic initializer for g_JUNGLE_LAND_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E7C0 ??__Eg_JUNGLE_LAND_TEXTURES@@YAXXZ 0x0044E7C0-0x0044E7E4
// symbol    ?construct_jungle_land_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_jungle_land_textures() {
    VectorCtorIterator(g_JUNGLE_LAND_TEXTURES, 0x70, 15, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E7F0));
}

/*
Purpose: Dynamic initializer for g_DUNE_LAND_TEXTURE; registers its ??__F teardown.
// ORIGINAL: 0x0044E810 ??__Eg_DUNE_LAND_TEXTURE@@YAXXZ 0x0044E810-0x0044E826 BYTE_EXACT
// symbol    ?construct_dune_land_texture@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619650 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_dune_land_texture() {
    g_DUNE_LAND_TEXTURE->Texture::Texture();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E830));
}

/*
Purpose: Dynamic initializer for g_SUNNY_MESA_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E840 ??__Eg_SUNNY_MESA_TEXTURES@@YAXXZ 0x0044E840-0x0044E864
// symbol    ?construct_sunny_mesa_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sunny_mesa_textures() {
    VectorCtorIterator(g_SUNNY_MESA_TEXTURES, 0x70, 8, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E870));
}

/*
Purpose: Dynamic initializer for g_RAINFALL_SINGLE_TILE_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E890 ??__Eg_RAINFALL_SINGLE_TILE_TEXTURES@@YAXXZ 0x0044E890-0x0044E8B4
// symbol    ?construct_rainfall_single_tile_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rainfall_single_tile_textures() {
    VectorCtorIterator(g_RAINFALL_SINGLE_TILE_TEXTURES, 0x70, 2, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E8C0));
}

/*
Purpose: Dynamic initializer for g_ROAD_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E8E0 ??__Eg_ROAD_TEXTURES@@YAXXZ 0x0044E8E0-0x0044E904
// symbol    ?construct_road_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_road_textures() {
    VectorCtorIterator(g_ROAD_TEXTURES, 0x70, 9, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E910));
}

/*
Purpose: Dynamic initializer for g_MAGTUBE_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E930 ??__Eg_MAGTUBE_TEXTURES@@YAXXZ 0x0044E930-0x0044E954
// symbol    ?construct_magtube_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_magtube_textures() {
    VectorCtorIterator(g_MAGTUBE_TEXTURES, 0x70, 9, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E960));
}

/*
Purpose: Dynamic initializer for g_RIVER_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E980 ??__Eg_RIVER_TEXTURES@@YAXXZ 0x0044E980-0x0044E9A4
// symbol    ?construct_river_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_river_textures() {
    VectorCtorIterator(g_RIVER_TEXTURES, 0x70, 16, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044E9B0));
}

/*
Purpose: Dynamic initializer for g_MOUNT_PLANET_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044E9D0 ??__Eg_MOUNT_PLANET_TEXTURES@@YAXXZ 0x0044E9D0-0x0044E9F4
// symbol    ?construct_mount_planet_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_mount_planet_textures() {
    VectorCtorIterator(g_MOUNT_PLANET_TEXTURES, 0x70, 3, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EA00));
}

/*
Purpose: Dynamic initializer for g_GARLAND_CRATER_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044EA20 ??__Eg_GARLAND_CRATER_TEXTURES@@YAXXZ 0x0044EA20-0x0044EA44
// symbol    ?construct_garland_crater_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_garland_crater_textures() {
    VectorCtorIterator(g_GARLAND_CRATER_TEXTURES, 0x70, 3, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EA50));
}

/*
Purpose: Dynamic initializer for g_FUNGUS_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044EA70 ??__Eg_FUNGUS_TEXTURES@@YAXXZ 0x0044EA70-0x0044EA94
// symbol    ?construct_fungus_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fungus_textures() {
    VectorCtorIterator(g_FUNGUS_TEXTURES, 0x70, 30, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EAA0));
}

/*
Purpose: Dynamic initializer for g_FARM_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044EAC0 ??__Eg_FARM_TEXTURES@@YAXXZ 0x0044EAC0-0x0044EAE4
// symbol    ?construct_farm_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_farm_textures() {
    VectorCtorIterator(g_FARM_TEXTURES, 0x70, 9, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EAF0));
}

/*
Purpose: Dynamic initializer for g_FOREST_TEXTURES; registers its ??__F teardown.
// ORIGINAL: 0x0044EB10 ??__Eg_FOREST_TEXTURES@@YAXXZ 0x0044EB10-0x0044EB34
// symbol    ?construct_forest_textures@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_forest_textures() {
    VectorCtorIterator(g_FOREST_TEXTURES, 0x70, 16, TextureElementCtor, TextureElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EB40));
}

/*
Purpose: Dynamic initializer for g_RAINFALL_SINGLE_TILE_TEXTURE; registers its ??__F teardown.
// ORIGINAL: 0x0044EB60 ??__Eg_RAINFALL_SINGLE_TILE_TEXTURE@@YAXXZ 0x0044EB60-0x0044EB76 BYTE_EXACT
// symbol    ?construct_rainfall_single_tile_texture@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619650 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rainfall_single_tile_texture() {
    g_RAINFALL_SINGLE_TILE_TEXTURE->Texture::Texture();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EB80));
}

/*
Purpose: Dynamic initializer for g_TER1_WHITE_ORG_YEL_TILE_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044EB90 ??__Eg_TER1_WHITE_ORG_YEL_TILE_SPRITES@@YAXXZ 0x0044EB90-0x0044EBB4
// symbol    ?construct_ter1_white_org_yel_tile_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_white_org_yel_tile_sprites() {
    VectorCtorIterator(g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EBC0));
}

/*
Purpose: Dynamic initializer for g_TER1_BOTTOM_LEFT_TILE_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044EBE0 ??__Eg_TER1_BOTTOM_LEFT_TILE_SPRITES@@YAXXZ 0x0044EBE0-0x0044EC04
// symbol    ?construct_ter1_bottom_left_tile_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_bottom_left_tile_sprites() {
    VectorCtorIterator(g_TER1_BOTTOM_LEFT_TILE_SPRITES, 0x2C, 9, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EC10));
}

/*
Purpose: Dynamic initializer for g_TER1_UNUSED_SPRITES2; registers its ??__F teardown.
// ORIGINAL: 0x0044EC30 ??__Eg_TER1_UNUSED_SPRITES2@@YAXXZ 0x0044EC30-0x0044EC54
// symbol    ?construct_ter1_unused_sprites2@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_unused_sprites2() {
    VectorCtorIterator(g_TER1_UNUSED_SPRITES2, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EC60));
}

/*
Purpose: Dynamic initializer for g_TER1_MINE_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044EC80 ??__Eg_TER1_MINE_SPRITE@@YAXXZ 0x0044EC80-0x0044EC96 BYTE_EXACT
// symbol    ?construct_ter1_mine_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_mine_sprite() {
    g_TER1_MINE_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044ECA0));
}

/*
Purpose: Dynamic initializer for g_TER1_SOLAR_COLLECTOR_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044ECB0 ??__Eg_TER1_SOLAR_COLLECTOR_SPRITE@@YAXXZ 0x0044ECB0-0x0044ECC6 BYTE_EXACT
// symbol    ?construct_ter1_solar_collector_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_solar_collector_sprite() {
    g_TER1_SOLAR_COLLECTOR_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044ECD0));
}

/*
Purpose: Dynamic initializer for g_TER1_TIDAL_HARNESS_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044ECE0 ??__Eg_TER1_TIDAL_HARNESS_SPRITE@@YAXXZ 0x0044ECE0-0x0044ECF6 BYTE_EXACT
// symbol    ?construct_ter1_tidal_harness_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_tidal_harness_sprite() {
    g_TER1_TIDAL_HARNESS_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044ED00));
}

/*
Purpose: Dynamic initializer for g_TER1_MINING_PLATFORM_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044ED10 ??__Eg_TER1_MINING_PLATFORM_SPRITE@@YAXXZ 0x0044ED10-0x0044ED26 BYTE_EXACT
// symbol    ?construct_ter1_mining_platform_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_mining_platform_sprite() {
    g_TER1_MINING_PLATFORM_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044ED30));
}

/*
Purpose: Dynamic initializer for g_TER1_TUT_BLANK_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044ED40 ??__Eg_TER1_TUT_BLANK_SPRITE@@YAXXZ 0x0044ED40-0x0044ED56 BYTE_EXACT
// symbol    ?construct_ter1_tut_blank_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_tut_blank_sprite() {
    g_TER1_TUT_BLANK_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044ED60));
}

/*
Purpose: Dynamic initializer for g_TER1_KELP_FARM_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044ED70 ??__Eg_TER1_KELP_FARM_SPRITE@@YAXXZ 0x0044ED70-0x0044ED86 BYTE_EXACT
// symbol    ?construct_ter1_kelp_farm_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_kelp_farm_sprite() {
    g_TER1_KELP_FARM_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044ED90));
}

/*
Purpose: Dynamic initializer for g_TER1_CONDENSER_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044EDA0 ??__Eg_TER1_CONDENSER_SPRITE@@YAXXZ 0x0044EDA0-0x0044EDB6 BYTE_EXACT
// symbol    ?construct_ter1_condenser_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_condenser_sprite() {
    g_TER1_CONDENSER_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EDC0));
}

/*
Purpose: Dynamic initializer for g_TER1_ECHELON_MIRROR_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044EDD0 ??__Eg_TER1_ECHELON_MIRROR_SPRITE@@YAXXZ 0x0044EDD0-0x0044EDE6 BYTE_EXACT
// symbol    ?construct_ter1_echelon_mirror_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_echelon_mirror_sprite() {
    g_TER1_ECHELON_MIRROR_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EDF0));
}

/*
Purpose: Dynamic initializer for g_TER1_BOREHOLE_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044EE00 ??__Eg_TER1_BOREHOLE_SPRITE@@YAXXZ 0x0044EE00-0x0044EE16 BYTE_EXACT
// symbol    ?construct_ter1_borehole_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_borehole_sprite() {
    g_TER1_BOREHOLE_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EE20));
}

/*
Purpose: Dynamic initializer for g_TER1_BOREHOLE_CLUSTER_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044EE30 ??__Eg_TER1_BOREHOLE_CLUSTER_SPRITE@@YAXXZ 0x0044EE30-0x0044EE46 BYTE_EXACT
// symbol    ?construct_ter1_borehole_cluster_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_borehole_cluster_sprite() {
    g_TER1_BOREHOLE_CLUSTER_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EE50));
}

/*
Purpose: Dynamic initializer for g_TER1_MANIFOLD_NEXUS_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044EE60 ??__Eg_TER1_MANIFOLD_NEXUS_SPRITES@@YAXXZ 0x0044EE60-0x0044EE84
// symbol    ?construct_ter1_manifold_nexus_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_manifold_nexus_sprites() {
    VectorCtorIterator(g_TER1_MANIFOLD_NEXUS_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EE90));
}

/*
Purpose: Dynamic initializer for g_TER1WRECK_UNITY_WRECKAGE_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044EEB0 ??__Eg_TER1WRECK_UNITY_WRECKAGE_SPRITES@@YAXXZ 0x0044EEB0-0x0044EED4
// symbol    ?construct_ter1wreck_unity_wreckage_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1wreck_unity_wreckage_sprites() {
    VectorCtorIterator(g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 0x2C, 15, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EEE0));
}

/*
Purpose: Dynamic initializer for g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044EF00 ??__Eg_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES@@YAXXZ 0x0044EF00-0x0044EF24
// symbol    ?construct_ter1wreck_unity_wreckage_alt_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1wreck_unity_wreckage_alt_sprites() {
    VectorCtorIterator(g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 0x2C, 4, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EF30));
}

/*
Purpose: Dynamic initializer for g_FOSSIL_FIELD_RIDGE_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044EF50 ??__Eg_FOSSIL_FIELD_RIDGE_SPRITES@@YAXXZ 0x0044EF50-0x0044EF74
// symbol    ?construct_fossil_field_ridge_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fossil_field_ridge_sprites() {
    VectorCtorIterator(g_FOSSIL_FIELD_RIDGE_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EF80));
}

/*
Purpose: Dynamic initializer for g_TER1_UNUSED_SPRITES1; registers its ??__F teardown.
// ORIGINAL: 0x0044EFA0 ??__Eg_TER1_UNUSED_SPRITES1@@YAXXZ 0x0044EFA0-0x0044EFC4
// symbol    ?construct_ter1_unused_sprites1@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_unused_sprites1() {
    VectorCtorIterator(g_TER1_UNUSED_SPRITES1, 0x2C, 5, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044EFD0));
}

/*
Purpose: Dynamic initializer for g_TER1_FARM_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044EFF0 ??__Eg_TER1_FARM_SPRITES@@YAXXZ 0x0044EFF0-0x0044F014
// symbol    ?construct_ter1_farm_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_farm_sprites() {
    VectorCtorIterator(g_TER1_FARM_SPRITES, 0x2C, 5, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F020));
}

/*
Purpose: Dynamic initializer for g_TER1_SOIL_ENRICHER_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F040 ??__Eg_TER1_SOIL_ENRICHER_SPRITES@@YAXXZ 0x0044F040-0x0044F064
// symbol    ?construct_ter1_soil_enricher_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_soil_enricher_sprites() {
    VectorCtorIterator(g_TER1_SOIL_ENRICHER_SPRITES, 0x2C, 5, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F070));
}

/*
Purpose: Dynamic initializer for g_TER1_SEA_LAND_RESOURCE_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F090 ??__Eg_TER1_SEA_LAND_RESOURCE_SPRITES@@YAXXZ 0x0044F090-0x0044F0B4
// symbol    ?construct_ter1_sea_land_resource_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_sea_land_resource_sprites() {
    VectorCtorIterator(g_TER1_SEA_LAND_RESOURCE_SPRITES, 0x2C, 12, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F0C0));
}

/*
Purpose: Dynamic initializer for g_TER1_LANDMARK_RESOURCE_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F0E0 ??__Eg_TER1_LANDMARK_RESOURCE_SPRITES@@YAXXZ 0x0044F0E0-0x0044F104
// symbol    ?construct_ter1_landmark_resource_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_landmark_resource_sprites() {
    VectorCtorIterator(g_TER1_LANDMARK_RESOURCE_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F110));
}

/*
Purpose: Dynamic initializer for g_GLOW_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F130 ??__Eg_GLOW_SPRITES@@YAXXZ 0x0044F130-0x0044F154
// symbol    ?construct_glow_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_glow_sprites() {
    VectorCtorIterator(g_GLOW_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F160));
}

/*
Purpose: Dynamic initializer for g_TER1_UNITY_POD_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F180 ??__Eg_TER1_UNITY_POD_SPRITES@@YAXXZ 0x0044F180-0x0044F1A4
// symbol    ?construct_ter1_unity_pod_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_unity_pod_sprites() {
    VectorCtorIterator(g_TER1_UNITY_POD_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F1B0));
}

/*
Purpose: Dynamic initializer for g_TER1_MONOLITH_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F1D0 ??__Eg_TER1_MONOLITH_SPRITE@@YAXXZ 0x0044F1D0-0x0044F1E6 BYTE_EXACT
// symbol    ?construct_ter1_monolith_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_monolith_sprite() {
    g_TER1_MONOLITH_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F1F0));
}

/*
Purpose: Dynamic initializer for g_TER1_BUNKER_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F200 ??__Eg_TER1_BUNKER_SPRITE@@YAXXZ 0x0044F200-0x0044F216 BYTE_EXACT
// symbol    ?construct_ter1_bunker_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_bunker_sprite() {
    g_TER1_BUNKER_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F220));
}

/*
Purpose: Dynamic initializer for g_TER1_AIRBASE_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F230 ??__Eg_TER1_AIRBASE_SPRITE@@YAXXZ 0x0044F230-0x0044F246 BYTE_EXACT
// symbol    ?construct_ter1_airbase_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_airbase_sprite() {
    g_TER1_AIRBASE_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F250));
}

/*
Purpose: Dynamic initializer for g_TER1_SENSOR_ARRAY_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F260 ??__Eg_TER1_SENSOR_ARRAY_SPRITE@@YAXXZ 0x0044F260-0x0044F276 BYTE_EXACT
// symbol    ?construct_ter1_sensor_array_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_sensor_array_sprite() {
    g_TER1_SENSOR_ARRAY_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F280));
}

/*
Purpose: Dynamic initializer for g_RAINFALL_DOUBLE_TILE_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F290 ??__Eg_RAINFALL_DOUBLE_TILE_SPRITES@@YAXXZ 0x0044F290-0x0044F2B4
// symbol    ?construct_rainfall_double_tile_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rainfall_double_tile_sprites() {
    VectorCtorIterator(g_RAINFALL_DOUBLE_TILE_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F2C0));
}

/*
Purpose: Dynamic initializer for g_VEH_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F2E0 ??__Eg_VEH_SPRITES@@YAXXZ 0x0044F2E0-0x0044F307
// symbol    ?construct_veh_sprites@@YAXXZ
// size      39 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_veh_sprites() {
    VectorCtorIterator(g_VEH_SPRITES, 0x2C, 152, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F310));
}

/*
Purpose: Dynamic initializer for g_FLAGS_VEH_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F330 ??__Eg_FLAGS_VEH_SPRITES@@YAXXZ 0x0044F330-0x0044F354
// symbol    ?construct_flags_veh_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_flags_veh_sprites() {
    VectorCtorIterator(g_FLAGS_VEH_SPRITES, 0x2C, 112, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F360));
}

/*
Purpose: Dynamic initializer for g_ICONS_GENERAL_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F380 ??__Eg_ICONS_GENERAL_SPRITES@@YAXXZ 0x0044F380-0x0044F3A4
// symbol    ?construct_icons_general_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_icons_general_sprites() {
    VectorCtorIterator(g_ICONS_GENERAL_SPRITES, 0x2C, 16, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F3B0));
}

/*
Purpose: Dynamic initializer for g_RESOURCE_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F3D0 ??__Eg_RESOURCE_ICON_SPRITES@@YAXXZ 0x0044F3D0-0x0044F3F4
// symbol    ?construct_resource_icon_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_resource_icon_sprites() {
    VectorCtorIterator(g_RESOURCE_ICON_SPRITES, 0x2C, 32, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F400));
}

/*
Purpose: Dynamic initializer for g_CITIZEN_LG_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F420 ??__Eg_CITIZEN_LG_CURSOR_SPRITES@@YAXXZ 0x0044F420-0x0044F444
// symbol    ?construct_citizen_lg_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_citizen_lg_cursor_sprites() {
    VectorCtorIterator(g_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 8, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F450));
}

/*
Purpose: Dynamic initializer for g_SPECIALIST_LG_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F470 ??__Eg_SPECIALIST_LG_CURSOR_SPRITES@@YAXXZ 0x0044F470-0x0044F494
// symbol    ?construct_specialist_lg_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_specialist_lg_cursor_sprites() {
    VectorCtorIterator(g_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F4A0));
}

/*
Purpose: Dynamic initializer for g_CITIZEN_SM_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F4C0 ??__Eg_CITIZEN_SM_CURSOR_SPRITES@@YAXXZ 0x0044F4C0-0x0044F4E4
// symbol    ?construct_citizen_sm_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_citizen_sm_cursor_sprites() {
    VectorCtorIterator(g_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 8, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F4F0));
}

/*
Purpose: Dynamic initializer for g_SPECIALIST_SM_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F510 ??__Eg_SPECIALIST_SM_CURSOR_SPRITES@@YAXXZ 0x0044F510-0x0044F534
// symbol    ?construct_specialist_sm_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_specialist_sm_cursor_sprites() {
    VectorCtorIterator(g_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F540));
}

/*
Purpose: Dynamic initializer for g_AL_CITIZEN_LG_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F560 ??__Eg_AL_CITIZEN_LG_CURSOR_SPRITES@@YAXXZ 0x0044F560-0x0044F584
// symbol    ?construct_al_citizen_lg_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_al_citizen_lg_cursor_sprites() {
    VectorCtorIterator(g_AL_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 4, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F590));
}

/*
Purpose: Dynamic initializer for g_AL_SPECIALIST_LG_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F5B0 ??__Eg_AL_SPECIALIST_LG_CURSOR_SPRITES@@YAXXZ 0x0044F5B0-0x0044F5D4
// symbol    ?construct_al_specialist_lg_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_al_specialist_lg_cursor_sprites() {
    VectorCtorIterator(g_AL_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F5E0));
}

/*
Purpose: Dynamic initializer for g_AL_CITIZEN_SM_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F600 ??__Eg_AL_CITIZEN_SM_CURSOR_SPRITES@@YAXXZ 0x0044F600-0x0044F624
// symbol    ?construct_al_citizen_sm_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_al_citizen_sm_cursor_sprites() {
    VectorCtorIterator(g_AL_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 4, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F630));
}

/*
Purpose: Dynamic initializer for g_AL_SPECIALIST_SM_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F650 ??__Eg_AL_SPECIALIST_SM_CURSOR_SPRITES@@YAXXZ 0x0044F650-0x0044F674
// symbol    ?construct_al_specialist_sm_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_al_specialist_sm_cursor_sprites() {
    VectorCtorIterator(g_AL_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F680));
}

/*
Purpose: Dynamic initializer for g_RED_ALIEN_HEAD_ICON_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F6A0 ??__Eg_RED_ALIEN_HEAD_ICON_SPRITE@@YAXXZ 0x0044F6A0-0x0044F6B6 BYTE_EXACT
// symbol    ?construct_red_alien_head_icon_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_red_alien_head_icon_sprite() {
    g_RED_ALIEN_HEAD_ICON_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F6C0));
}

/*
Purpose: Dynamic initializer for g_SILVER_MENU_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F6D0 ??__Eg_SILVER_MENU_ICON_SPRITES@@YAXXZ 0x0044F6D0-0x0044F6F4
// symbol    ?construct_silver_menu_icon_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_silver_menu_icon_sprites() {
    VectorCtorIterator(g_SILVER_MENU_ICON_SPRITES, 0x2C, 4, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F700));
}

/*
Purpose: Dynamic initializer for g_SILVER_CHECKBOX_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F720 ??__Eg_SILVER_CHECKBOX_ICON_SPRITES@@YAXXZ 0x0044F720-0x0044F744
// symbol    ?construct_silver_checkbox_icon_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_silver_checkbox_icon_sprites() {
    VectorCtorIterator(g_SILVER_CHECKBOX_ICON_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F750));
}

/*
Purpose: Dynamic initializer for g_RED_MALE_HEAD_ICON_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F770 ??__Eg_RED_MALE_HEAD_ICON_SPRITE@@YAXXZ 0x0044F770-0x0044F786 BYTE_EXACT
// symbol    ?construct_red_male_head_icon_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_red_male_head_icon_sprite() {
    g_RED_MALE_HEAD_ICON_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F790));
}

/*
Purpose: Dynamic initializer for g_NULL_RESOURCE_ICON_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F7A0 ??__Eg_NULL_RESOURCE_ICON_SPRITE@@YAXXZ 0x0044F7A0-0x0044F7B6 BYTE_EXACT
// symbol    ?construct_null_resource_icon_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_null_resource_icon_sprite() {
    g_NULL_RESOURCE_ICON_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F7C0));
}

/*
Purpose: Dynamic initializer for g_PEACE_SIGN_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F7D0 ??__Eg_PEACE_SIGN_SPRITES@@YAXXZ 0x0044F7D0-0x0044F7F4
// symbol    ?construct_peace_sign_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_peace_sign_sprites() {
    VectorCtorIterator(g_PEACE_SIGN_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F800));
}

/*
Purpose: Dynamic initializer for g_ICON_TILE_SQUARE_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F820 ??__Eg_ICON_TILE_SQUARE_SPRITE@@YAXXZ 0x0044F820-0x0044F836 BYTE_EXACT
// symbol    ?construct_icon_tile_square_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_icon_tile_square_sprite() {
    g_ICON_TILE_SQUARE_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F840));
}

/*
Purpose: Dynamic initializer for g_XI_BOOM_VEH_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F850 ??__Eg_XI_BOOM_VEH_SPRITES@@YAXXZ 0x0044F850-0x0044F877
// symbol    ?construct_xi_boom_veh_sprites@@YAXXZ
// size      39 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_xi_boom_veh_sprites() {
    VectorCtorIterator(g_XI_BOOM_VEH_SPRITES, 0x2C, 144, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F880));
}

/*
Purpose: Dynamic initializer for g_XF_BOOM_VEH_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044F8A0 ??__Eg_XF_BOOM_VEH_SPRITES@@YAXXZ 0x0044F8A0-0x0044F8C4
// symbol    ?construct_xf_boom_veh_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_xf_boom_veh_sprites() {
    VectorCtorIterator(g_XF_BOOM_VEH_SPRITES, 0x2C, 64, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F8D0));
}

/*
Purpose: Dynamic initializer for g_BATTLE_MIND_WORM_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F8F0 ??__Eg_BATTLE_MIND_WORM_SPRITE@@YAXXZ 0x0044F8F0-0x0044F906 BYTE_EXACT
// symbol    ?construct_battle_mind_worm_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_mind_worm_sprite() {
    g_BATTLE_MIND_WORM_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F910));
}

/*
Purpose: Dynamic initializer for g_BATTLE_ISLE_DEEP_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F920 ??__Eg_BATTLE_ISLE_DEEP_SPRITE@@YAXXZ 0x0044F920-0x0044F936 BYTE_EXACT
// symbol    ?construct_battle_isle_deep_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_isle_deep_sprite() {
    g_BATTLE_ISLE_DEEP_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F940));
}

/*
Purpose: Dynamic initializer for g_BATTLE_LOCUSTS_CHIRON_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F950 ??__Eg_BATTLE_LOCUSTS_CHIRON_SPRITE@@YAXXZ 0x0044F950-0x0044F966 BYTE_EXACT
// symbol    ?construct_battle_locusts_chiron_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_locusts_chiron_sprite() {
    g_BATTLE_LOCUSTS_CHIRON_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F970));
}

/*
Purpose: Dynamic initializer for g_BATTLE_FUNGAL_TOWER_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F980 ??__Eg_BATTLE_FUNGAL_TOWER_SPRITE@@YAXXZ 0x0044F980-0x0044F996 BYTE_EXACT
// symbol    ?construct_battle_fungal_tower_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_fungal_tower_sprite() {
    g_BATTLE_FUNGAL_TOWER_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F9A0));
}

/*
Purpose: Dynamic initializer for g_BATTLE_SPORE_LAUNCHER_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F9B0 ??__Eg_BATTLE_SPORE_LAUNCHER_SPRITE@@YAXXZ 0x0044F9B0-0x0044F9C6 BYTE_EXACT
// symbol    ?construct_battle_spore_launcher_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_spore_launcher_sprite() {
    g_BATTLE_SPORE_LAUNCHER_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044F9D0));
}

/*
Purpose: Dynamic initializer for g_BATTLE_SEALURK_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044F9E0 ??__Eg_BATTLE_SEALURK_SPRITE@@YAXXZ 0x0044F9E0-0x0044F9F6 BYTE_EXACT
// symbol    ?construct_battle_sealurk_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_sealurk_sprite() {
    g_BATTLE_SEALURK_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FA00));
}

/*
Purpose: Dynamic initializer for g_TECH_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044FA10 ??__Eg_TECH_ICON_SPRITES@@YAXXZ 0x0044FA10-0x0044FA34
// symbol    ?construct_tech_icon_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_tech_icon_sprites() {
    VectorCtorIterator(g_TECH_ICON_SPRITES, 0x2C, 89, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FA40));
}

/*
Purpose: Dynamic initializer for g_FACILITY_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044FA60 ??__Eg_FACILITY_ICON_SPRITES@@YAXXZ 0x0044FA60-0x0044FA84
// symbol    ?construct_facility_icon_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_facility_icon_sprites() {
    VectorCtorIterator(g_FACILITY_ICON_SPRITES, 0x2C, 70, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FA90));
}

/*
Purpose: Dynamic initializer for g_SECRET_PROJECT_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044FAB0 ??__Eg_SECRET_PROJECT_ICON_SPRITES@@YAXXZ 0x0044FAB0-0x0044FAD4
// symbol    ?construct_secret_project_icon_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_secret_project_icon_sprites() {
    VectorCtorIterator(g_SECRET_PROJECT_ICON_SPRITES, 0x2C, 64, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FAE0));
}

/*
Purpose: Dynamic initializer for g_IFACE_MP_COMBO_ARROW_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044FB00 ??__Eg_IFACE_MP_COMBO_ARROW_SPRITES@@YAXXZ 0x0044FB00-0x0044FB24
// symbol    ?construct_iface_mp_combo_arrow_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_mp_combo_arrow_sprites() {
    VectorCtorIterator(g_IFACE_MP_COMBO_ARROW_SPRITES, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FB30));
}

/*
Purpose: Dynamic initializer for g_SCROLL_BAR_ARROW_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044FB50 ??__Eg_SCROLL_BAR_ARROW_ICON_SPRITES@@YAXXZ 0x0044FB50-0x0044FB74
// symbol    ?construct_scroll_bar_arrow_icon_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scroll_bar_arrow_icon_sprites() {
    VectorCtorIterator(g_SCROLL_BAR_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FB80));
}

/*
Purpose: Dynamic initializer for g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044FBA0 ??__Eg_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES@@YAXXZ 0x0044FBA0-0x0044FBC4
// symbol    ?construct_scroll_bar_small_arrow_icon_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scroll_bar_small_arrow_icon_sprites() {
    VectorCtorIterator(g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FBD0));
}

/*
Purpose: Dynamic initializer for g_SCROLL_BAR_FILLER_ICON_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044FBF0 ??__Eg_SCROLL_BAR_FILLER_ICON_SPRITES@@YAXXZ 0x0044FBF0-0x0044FC06 BYTE_EXACT
// symbol    ?construct_scroll_bar_filler_icon_sprites@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scroll_bar_filler_icon_sprites() {
    g_SCROLL_BAR_FILLER_ICON_SPRITES->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FC10));
}

/*
Purpose: Dynamic initializer for g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FC20 ??__Eg_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE@@YAXXZ 0x0044FC20-0x0044FC36 BYTE_EXACT
// symbol    ?construct_scroll_bar_small_filler_icon_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scroll_bar_small_filler_icon_sprite() {
    g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FC40));
}

/*
Purpose: Dynamic initializer for g_IFACE_LOCK_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0044FC50 ??__Eg_IFACE_LOCK_SPRITES@@YAXXZ 0x0044FC50-0x0044FC74
// symbol    ?construct_iface_lock_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_lock_sprites() {
    VectorCtorIterator(g_IFACE_LOCK_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FC80));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR01; registers its ??__F teardown.
// ORIGINAL: 0x0044FCA0 ??__Eg_UNUSED_SPRITES_VAR01@@YAXXZ 0x0044FCA0-0x0044FCC4
// symbol    ?construct_unused_sprites_var01@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var01() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR01, 0x2C, 8, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FCD0));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FCF0 ??__Eg_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE@@YAXXZ 0x0044FCF0-0x0044FD06 BYTE_EXACT
// symbol    ?construct_iface_general_windows_top_left_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_top_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FD10));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FD20 ??__Eg_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE@@YAXXZ 0x0044FD20-0x0044FD36 BYTE_EXACT
// symbol    ?construct_iface_general_windows_top_right_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_top_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FD40));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FD50 ??__Eg_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE@@YAXXZ 0x0044FD50-0x0044FD66 BYTE_EXACT
// symbol    ?construct_iface_general_windows_bot_left_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_bot_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FD70));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FD80 ??__Eg_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE@@YAXXZ 0x0044FD80-0x0044FD96 BYTE_EXACT
// symbol    ?construct_iface_general_windows_bot_right_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_bot_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FDA0));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FDB0 ??__Eg_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE@@YAXXZ 0x0044FDB0-0x0044FDC6 BYTE_EXACT
// symbol    ?construct_iface_general_windows_mid_left_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_mid_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FDD0));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FDE0 ??__Eg_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE@@YAXXZ 0x0044FDE0-0x0044FDF6 BYTE_EXACT
// symbol    ?construct_iface_general_windows_mid_right_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_mid_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FE00));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FE10 ??__Eg_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE@@YAXXZ 0x0044FE10-0x0044FE26 BYTE_EXACT
// symbol    ?construct_iface_general_windows_top_mid_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_top_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FE30));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FE40 ??__Eg_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE@@YAXXZ 0x0044FE40-0x0044FE56 BYTE_EXACT
// symbol    ?construct_iface_general_windows_bot_mid_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_bot_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FE60));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FE70 ??__Eg_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE@@YAXXZ 0x0044FE70-0x0044FE86 BYTE_EXACT
// symbol    ?construct_iface_general_windows_noncap_mid_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_noncap_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FE90));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FEA0 ??__Eg_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE@@YAXXZ 0x0044FEA0-0x0044FEB6 BYTE_EXACT
// symbol    ?construct_iface_general_windows_noncap_left_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_noncap_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FEC0));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x0044FED0 ??__Eg_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE@@YAXXZ 0x0044FED0-0x0044FEE6 BYTE_EXACT
// symbol    ?construct_iface_general_windows_noncap_right_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_noncap_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FEF0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR23; registers its ??__F teardown.
// ORIGINAL: 0x0044FF00 ??__Eg_UNUSED_SPRITE_VAR23@@YAXXZ 0x0044FF00-0x0044FF16 BYTE_EXACT
// symbol    ?construct_unused_sprite_var23@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var23() {
    g_UNUSED_SPRITE_VAR23->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FF20));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR24; registers its ??__F teardown.
// ORIGINAL: 0x0044FF30 ??__Eg_UNUSED_SPRITE_VAR24@@YAXXZ 0x0044FF30-0x0044FF46 BYTE_EXACT
// symbol    ?construct_unused_sprite_var24@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var24() {
    g_UNUSED_SPRITE_VAR24->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FF50));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR25; registers its ??__F teardown.
// ORIGINAL: 0x0044FF60 ??__Eg_UNUSED_SPRITE_VAR25@@YAXXZ 0x0044FF60-0x0044FF76 BYTE_EXACT
// symbol    ?construct_unused_sprite_var25@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var25() {
    g_UNUSED_SPRITE_VAR25->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FF80));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR26; registers its ??__F teardown.
// ORIGINAL: 0x0044FF90 ??__Eg_UNUSED_SPRITE_VAR26@@YAXXZ 0x0044FF90-0x0044FFA6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var26@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var26() {
    g_UNUSED_SPRITE_VAR26->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FFB0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR27; registers its ??__F teardown.
// ORIGINAL: 0x0044FFC0 ??__Eg_UNUSED_SPRITE_VAR27@@YAXXZ 0x0044FFC0-0x0044FFD6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var27@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var27() {
    g_UNUSED_SPRITE_VAR27->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0044FFE0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR28; registers its ??__F teardown.
// ORIGINAL: 0x0044FFF0 ??__Eg_UNUSED_SPRITE_VAR28@@YAXXZ 0x0044FFF0-0x00450006 BYTE_EXACT
// symbol    ?construct_unused_sprite_var28@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var28() {
    g_UNUSED_SPRITE_VAR28->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450010));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR29; registers its ??__F teardown.
// ORIGINAL: 0x00450020 ??__Eg_UNUSED_SPRITE_VAR29@@YAXXZ 0x00450020-0x00450036 BYTE_EXACT
// symbol    ?construct_unused_sprite_var29@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var29() {
    g_UNUSED_SPRITE_VAR29->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450040));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR30; registers its ??__F teardown.
// ORIGINAL: 0x00450050 ??__Eg_UNUSED_SPRITE_VAR30@@YAXXZ 0x00450050-0x00450066 BYTE_EXACT
// symbol    ?construct_unused_sprite_var30@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var30() {
    g_UNUSED_SPRITE_VAR30->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450070));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR31; registers its ??__F teardown.
// ORIGINAL: 0x00450080 ??__Eg_UNUSED_SPRITE_VAR31@@YAXXZ 0x00450080-0x00450096 BYTE_EXACT
// symbol    ?construct_unused_sprite_var31@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var31() {
    g_UNUSED_SPRITE_VAR31->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004500A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR32; registers its ??__F teardown.
// ORIGINAL: 0x004500B0 ??__Eg_UNUSED_SPRITE_VAR32@@YAXXZ 0x004500B0-0x004500C6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var32@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var32() {
    g_UNUSED_SPRITE_VAR32->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004500D0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR33; registers its ??__F teardown.
// ORIGINAL: 0x004500E0 ??__Eg_UNUSED_SPRITE_VAR33@@YAXXZ 0x004500E0-0x004500F6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var33@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var33() {
    g_UNUSED_SPRITE_VAR33->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450100));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR34; registers its ??__F teardown.
// ORIGINAL: 0x00450110 ??__Eg_UNUSED_SPRITE_VAR34@@YAXXZ 0x00450110-0x00450126 BYTE_EXACT
// symbol    ?construct_unused_sprite_var34@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var34() {
    g_UNUSED_SPRITE_VAR34->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450130));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR35; registers its ??__F teardown.
// ORIGINAL: 0x00450140 ??__Eg_UNUSED_SPRITE_VAR35@@YAXXZ 0x00450140-0x00450156 BYTE_EXACT
// symbol    ?construct_unused_sprite_var35@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var35() {
    g_UNUSED_SPRITE_VAR35->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450160));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR36; registers its ??__F teardown.
// ORIGINAL: 0x00450170 ??__Eg_UNUSED_SPRITE_VAR36@@YAXXZ 0x00450170-0x00450186 BYTE_EXACT
// symbol    ?construct_unused_sprite_var36@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var36() {
    g_UNUSED_SPRITE_VAR36->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450190));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR37; registers its ??__F teardown.
// ORIGINAL: 0x004501A0 ??__Eg_UNUSED_SPRITE_VAR37@@YAXXZ 0x004501A0-0x004501B6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var37@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var37() {
    g_UNUSED_SPRITE_VAR37->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004501C0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR38; registers its ??__F teardown.
// ORIGINAL: 0x004501D0 ??__Eg_UNUSED_SPRITE_VAR38@@YAXXZ 0x004501D0-0x004501E6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var38@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var38() {
    g_UNUSED_SPRITE_VAR38->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004501F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR39; registers its ??__F teardown.
// ORIGINAL: 0x00450200 ??__Eg_UNUSED_SPRITE_VAR39@@YAXXZ 0x00450200-0x00450216 BYTE_EXACT
// symbol    ?construct_unused_sprite_var39@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var39() {
    g_UNUSED_SPRITE_VAR39->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450220));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR40; registers its ??__F teardown.
// ORIGINAL: 0x00450230 ??__Eg_UNUSED_SPRITE_VAR40@@YAXXZ 0x00450230-0x00450246 BYTE_EXACT
// symbol    ?construct_unused_sprite_var40@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var40() {
    g_UNUSED_SPRITE_VAR40->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450250));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR41; registers its ??__F teardown.
// ORIGINAL: 0x00450260 ??__Eg_UNUSED_SPRITE_VAR41@@YAXXZ 0x00450260-0x00450276 BYTE_EXACT
// symbol    ?construct_unused_sprite_var41@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var41() {
    g_UNUSED_SPRITE_VAR41->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450280));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR42; registers its ??__F teardown.
// ORIGINAL: 0x00450290 ??__Eg_UNUSED_SPRITE_VAR42@@YAXXZ 0x00450290-0x004502A6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var42@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var42() {
    g_UNUSED_SPRITE_VAR42->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004502B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR43; registers its ??__F teardown.
// ORIGINAL: 0x004502C0 ??__Eg_UNUSED_SPRITE_VAR43@@YAXXZ 0x004502C0-0x004502D6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var43@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var43() {
    g_UNUSED_SPRITE_VAR43->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004502E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR44; registers its ??__F teardown.
// ORIGINAL: 0x004502F0 ??__Eg_UNUSED_SPRITE_VAR44@@YAXXZ 0x004502F0-0x00450306 BYTE_EXACT
// symbol    ?construct_unused_sprite_var44@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var44() {
    g_UNUSED_SPRITE_VAR44->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450310));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR45; registers its ??__F teardown.
// ORIGINAL: 0x00450320 ??__Eg_UNUSED_SPRITE_VAR45@@YAXXZ 0x00450320-0x00450336 BYTE_EXACT
// symbol    ?construct_unused_sprite_var45@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var45() {
    g_UNUSED_SPRITE_VAR45->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450340));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR46; registers its ??__F teardown.
// ORIGINAL: 0x00450350 ??__Eg_UNUSED_SPRITE_VAR46@@YAXXZ 0x00450350-0x00450366 BYTE_EXACT
// symbol    ?construct_unused_sprite_var46@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var46() {
    g_UNUSED_SPRITE_VAR46->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450370));
}

/*
Purpose: Dynamic initializer for g_IFACE_TECH_TREE_ARROW_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x00450380 ??__Eg_IFACE_TECH_TREE_ARROW_SPRITES@@YAXXZ 0x00450380-0x004503A4
// symbol    ?construct_iface_tech_tree_arrow_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_tech_tree_arrow_sprites() {
    VectorCtorIterator(g_IFACE_TECH_TREE_ARROW_SPRITES, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x004503B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR47; registers its ??__F teardown.
// ORIGINAL: 0x004503D0 ??__Eg_UNUSED_SPRITE_VAR47@@YAXXZ 0x004503D0-0x004503E6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var47@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var47() {
    g_UNUSED_SPRITE_VAR47->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004503F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR48; registers its ??__F teardown.
// ORIGINAL: 0x00450400 ??__Eg_UNUSED_SPRITE_VAR48@@YAXXZ 0x00450400-0x00450416 BYTE_EXACT
// symbol    ?construct_unused_sprite_var48@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var48() {
    g_UNUSED_SPRITE_VAR48->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450420));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR49; registers its ??__F teardown.
// ORIGINAL: 0x00450430 ??__Eg_UNUSED_SPRITE_VAR49@@YAXXZ 0x00450430-0x00450446 BYTE_EXACT
// symbol    ?construct_unused_sprite_var49@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var49() {
    g_UNUSED_SPRITE_VAR49->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450450));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR50; registers its ??__F teardown.
// ORIGINAL: 0x00450460 ??__Eg_UNUSED_SPRITE_VAR50@@YAXXZ 0x00450460-0x00450476 BYTE_EXACT
// symbol    ?construct_unused_sprite_var50@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var50() {
    g_UNUSED_SPRITE_VAR50->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450480));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR51; registers its ??__F teardown.
// ORIGINAL: 0x00450490 ??__Eg_UNUSED_SPRITE_VAR51@@YAXXZ 0x00450490-0x004504A6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var51@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var51() {
    g_UNUSED_SPRITE_VAR51->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004504B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR52; registers its ??__F teardown.
// ORIGINAL: 0x004504C0 ??__Eg_UNUSED_SPRITE_VAR52@@YAXXZ 0x004504C0-0x004504D6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var52@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var52() {
    g_UNUSED_SPRITE_VAR52->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004504E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR53; registers its ??__F teardown.
// ORIGINAL: 0x004504F0 ??__Eg_UNUSED_SPRITE_VAR53@@YAXXZ 0x004504F0-0x00450506 BYTE_EXACT
// symbol    ?construct_unused_sprite_var53@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var53() {
    g_UNUSED_SPRITE_VAR53->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450510));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR54; registers its ??__F teardown.
// ORIGINAL: 0x00450520 ??__Eg_UNUSED_SPRITE_VAR54@@YAXXZ 0x00450520-0x00450536 BYTE_EXACT
// symbol    ?construct_unused_sprite_var54@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var54() {
    g_UNUSED_SPRITE_VAR54->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450540));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR55; registers its ??__F teardown.
// ORIGINAL: 0x00450550 ??__Eg_UNUSED_SPRITE_VAR55@@YAXXZ 0x00450550-0x00450566 BYTE_EXACT
// symbol    ?construct_unused_sprite_var55@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var55() {
    g_UNUSED_SPRITE_VAR55->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450570));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR56; registers its ??__F teardown.
// ORIGINAL: 0x00450580 ??__Eg_UNUSED_SPRITE_VAR56@@YAXXZ 0x00450580-0x00450596 BYTE_EXACT
// symbol    ?construct_unused_sprite_var56@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var56() {
    g_UNUSED_SPRITE_VAR56->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004505A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR57; registers its ??__F teardown.
// ORIGINAL: 0x004505B0 ??__Eg_UNUSED_SPRITE_VAR57@@YAXXZ 0x004505B0-0x004505C6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var57@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var57() {
    g_UNUSED_SPRITE_VAR57->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004505D0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR58; registers its ??__F teardown.
// ORIGINAL: 0x004505E0 ??__Eg_UNUSED_SPRITE_VAR58@@YAXXZ 0x004505E0-0x004505F6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var58@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var58() {
    g_UNUSED_SPRITE_VAR58->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450600));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR59; registers its ??__F teardown.
// ORIGINAL: 0x00450610 ??__Eg_UNUSED_SPRITE_VAR59@@YAXXZ 0x00450610-0x00450626 BYTE_EXACT
// symbol    ?construct_unused_sprite_var59@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var59() {
    g_UNUSED_SPRITE_VAR59->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450630));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR60; registers its ??__F teardown.
// ORIGINAL: 0x00450640 ??__Eg_UNUSED_SPRITE_VAR60@@YAXXZ 0x00450640-0x00450656 BYTE_EXACT
// symbol    ?construct_unused_sprite_var60@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var60() {
    g_UNUSED_SPRITE_VAR60->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450660));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR61; registers its ??__F teardown.
// ORIGINAL: 0x00450670 ??__Eg_UNUSED_SPRITE_VAR61@@YAXXZ 0x00450670-0x00450686 BYTE_EXACT
// symbol    ?construct_unused_sprite_var61@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var61() {
    g_UNUSED_SPRITE_VAR61->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450690));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR62; registers its ??__F teardown.
// ORIGINAL: 0x004506A0 ??__Eg_UNUSED_SPRITE_VAR62@@YAXXZ 0x004506A0-0x004506B6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var62@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var62() {
    g_UNUSED_SPRITE_VAR62->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004506C0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR63; registers its ??__F teardown.
// ORIGINAL: 0x004506D0 ??__Eg_UNUSED_SPRITE_VAR63@@YAXXZ 0x004506D0-0x004506E6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var63@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var63() {
    g_UNUSED_SPRITE_VAR63->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004506F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR64; registers its ??__F teardown.
// ORIGINAL: 0x00450700 ??__Eg_UNUSED_SPRITE_VAR64@@YAXXZ 0x00450700-0x00450716 BYTE_EXACT
// symbol    ?construct_unused_sprite_var64@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var64() {
    g_UNUSED_SPRITE_VAR64->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450720));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR65; registers its ??__F teardown.
// ORIGINAL: 0x00450730 ??__Eg_UNUSED_SPRITE_VAR65@@YAXXZ 0x00450730-0x00450746 BYTE_EXACT
// symbol    ?construct_unused_sprite_var65@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var65() {
    g_UNUSED_SPRITE_VAR65->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450750));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR66; registers its ??__F teardown.
// ORIGINAL: 0x00450760 ??__Eg_UNUSED_SPRITE_VAR66@@YAXXZ 0x00450760-0x00450776 BYTE_EXACT
// symbol    ?construct_unused_sprite_var66@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var66() {
    g_UNUSED_SPRITE_VAR66->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450780));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR67; registers its ??__F teardown.
// ORIGINAL: 0x00450790 ??__Eg_UNUSED_SPRITE_VAR67@@YAXXZ 0x00450790-0x004507A6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var67@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var67() {
    g_UNUSED_SPRITE_VAR67->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004507B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR68; registers its ??__F teardown.
// ORIGINAL: 0x004507C0 ??__Eg_UNUSED_SPRITE_VAR68@@YAXXZ 0x004507C0-0x004507D6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var68@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var68() {
    g_UNUSED_SPRITE_VAR68->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004507E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR69; registers its ??__F teardown.
// ORIGINAL: 0x004507F0 ??__Eg_UNUSED_SPRITE_VAR69@@YAXXZ 0x004507F0-0x00450806 BYTE_EXACT
// symbol    ?construct_unused_sprite_var69@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var69() {
    g_UNUSED_SPRITE_VAR69->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450810));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR70; registers its ??__F teardown.
// ORIGINAL: 0x00450820 ??__Eg_UNUSED_SPRITE_VAR70@@YAXXZ 0x00450820-0x00450836 BYTE_EXACT
// symbol    ?construct_unused_sprite_var70@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var70() {
    g_UNUSED_SPRITE_VAR70->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450840));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR71; registers its ??__F teardown.
// ORIGINAL: 0x00450850 ??__Eg_UNUSED_SPRITE_VAR71@@YAXXZ 0x00450850-0x00450866 BYTE_EXACT
// symbol    ?construct_unused_sprite_var71@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var71() {
    g_UNUSED_SPRITE_VAR71->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450870));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR72; registers its ??__F teardown.
// ORIGINAL: 0x00450880 ??__Eg_UNUSED_SPRITE_VAR72@@YAXXZ 0x00450880-0x00450896 BYTE_EXACT
// symbol    ?construct_unused_sprite_var72@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var72() {
    g_UNUSED_SPRITE_VAR72->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004508A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR73; registers its ??__F teardown.
// ORIGINAL: 0x004508B0 ??__Eg_UNUSED_SPRITE_VAR73@@YAXXZ 0x004508B0-0x004508C6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var73@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var73() {
    g_UNUSED_SPRITE_VAR73->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004508D0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR74; registers its ??__F teardown.
// ORIGINAL: 0x004508E0 ??__Eg_UNUSED_SPRITE_VAR74@@YAXXZ 0x004508E0-0x004508F6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var74@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var74() {
    g_UNUSED_SPRITE_VAR74->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450900));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR02; registers its ??__F teardown.
// ORIGINAL: 0x00450910 ??__Eg_UNUSED_SPRITES_VAR02@@YAXXZ 0x00450910-0x00450934
// symbol    ?construct_unused_sprites_var02@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var02() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR02, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450940));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR03; registers its ??__F teardown.
// ORIGINAL: 0x00450960 ??__Eg_UNUSED_SPRITES_VAR03@@YAXXZ 0x00450960-0x00450984
// symbol    ?construct_unused_sprites_var03@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var03() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR03, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450990));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR04; registers its ??__F teardown.
// ORIGINAL: 0x004509B0 ??__Eg_UNUSED_SPRITES_VAR04@@YAXXZ 0x004509B0-0x004509D4
// symbol    ?construct_unused_sprites_var04@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var04() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR04, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x004509E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR05; registers its ??__F teardown.
// ORIGINAL: 0x00450A00 ??__Eg_UNUSED_SPRITES_VAR05@@YAXXZ 0x00450A00-0x00450A24
// symbol    ?construct_unused_sprites_var05@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var05() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR05, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450A30));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR06; registers its ??__F teardown.
// ORIGINAL: 0x00450A50 ??__Eg_UNUSED_SPRITES_VAR06@@YAXXZ 0x00450A50-0x00450A74
// symbol    ?construct_unused_sprites_var06@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var06() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR06, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450A80));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR07; registers its ??__F teardown.
// ORIGINAL: 0x00450AA0 ??__Eg_UNUSED_SPRITES_VAR07@@YAXXZ 0x00450AA0-0x00450AC4
// symbol    ?construct_unused_sprites_var07@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var07() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR07, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450AD0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR08; registers its ??__F teardown.
// ORIGINAL: 0x00450AF0 ??__Eg_UNUSED_SPRITES_VAR08@@YAXXZ 0x00450AF0-0x00450B14
// symbol    ?construct_unused_sprites_var08@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var08() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR08, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450B20));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR09; registers its ??__F teardown.
// ORIGINAL: 0x00450B40 ??__Eg_UNUSED_SPRITES_VAR09@@YAXXZ 0x00450B40-0x00450B64
// symbol    ?construct_unused_sprites_var09@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var09() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR09, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450B70));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR10; registers its ??__F teardown.
// ORIGINAL: 0x00450B90 ??__Eg_UNUSED_SPRITES_VAR10@@YAXXZ 0x00450B90-0x00450BB4
// symbol    ?construct_unused_sprites_var10@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var10() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR10, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450BC0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR75; registers its ??__F teardown.
// ORIGINAL: 0x00450BE0 ??__Eg_UNUSED_SPRITE_VAR75@@YAXXZ 0x00450BE0-0x00450BF6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var75@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var75() {
    g_UNUSED_SPRITE_VAR75->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450C00));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR76; registers its ??__F teardown.
// ORIGINAL: 0x00450C10 ??__Eg_UNUSED_SPRITE_VAR76@@YAXXZ 0x00450C10-0x00450C26 BYTE_EXACT
// symbol    ?construct_unused_sprite_var76@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var76() {
    g_UNUSED_SPRITE_VAR76->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450C30));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR77; registers its ??__F teardown.
// ORIGINAL: 0x00450C40 ??__Eg_UNUSED_SPRITE_VAR77@@YAXXZ 0x00450C40-0x00450C56 BYTE_EXACT
// symbol    ?construct_unused_sprite_var77@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var77() {
    g_UNUSED_SPRITE_VAR77->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450C60));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR78; registers its ??__F teardown.
// ORIGINAL: 0x00450C70 ??__Eg_UNUSED_SPRITE_VAR78@@YAXXZ 0x00450C70-0x00450C86 BYTE_EXACT
// symbol    ?construct_unused_sprite_var78@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var78() {
    g_UNUSED_SPRITE_VAR78->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450C90));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR79; registers its ??__F teardown.
// ORIGINAL: 0x00450CA0 ??__Eg_UNUSED_SPRITE_VAR79@@YAXXZ 0x00450CA0-0x00450CB6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var79@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var79() {
    g_UNUSED_SPRITE_VAR79->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450CC0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR80; registers its ??__F teardown.
// ORIGINAL: 0x00450CD0 ??__Eg_UNUSED_SPRITE_VAR80@@YAXXZ 0x00450CD0-0x00450CE6 BYTE_EXACT
// symbol    ?construct_unused_sprite_var80@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var80() {
    g_UNUSED_SPRITE_VAR80->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450CF0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR81; registers its ??__F teardown.
// ORIGINAL: 0x00450D00 ??__Eg_UNUSED_SPRITE_VAR81@@YAXXZ 0x00450D00-0x00450D16 BYTE_EXACT
// symbol    ?construct_unused_sprite_var81@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var81() {
    g_UNUSED_SPRITE_VAR81->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450D20));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR82; registers its ??__F teardown.
// ORIGINAL: 0x00450D30 ??__Eg_UNUSED_SPRITE_VAR82@@YAXXZ 0x00450D30-0x00450D46 BYTE_EXACT
// symbol    ?construct_unused_sprite_var82@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var82() {
    g_UNUSED_SPRITE_VAR82->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450D50));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR83; registers its ??__F teardown.
// ORIGINAL: 0x00450D60 ??__Eg_UNUSED_SPRITE_VAR83@@YAXXZ 0x00450D60-0x00450D76 BYTE_EXACT
// symbol    ?construct_unused_sprite_var83@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var83() {
    g_UNUSED_SPRITE_VAR83->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450D80));
}

/*
Purpose: Dynamic initializer for g_BASEWIN_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x00450D90 ??__Eg_BASEWIN_SPRITES@@YAXXZ 0x00450D90-0x00450DB4
// symbol    ?construct_basewin_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_basewin_sprites() {
    VectorCtorIterator(g_BASEWIN_SPRITES, 0x2C, 27, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00450DC0));
}

/*
Purpose: Dynamic initializer for g_INFOWIN; registers its ??__F teardown.
// ORIGINAL: 0x004562C0 ??__Eg_INFOWIN@@YAXXZ 0x004562C0-0x004562D6
// symbol    ?construct_infowin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00459500 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_infowin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007AD2A0))->*InfoWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004562E0));
}

/*
Purpose: Dynamic initializer for g_MAININTERFACE; registers its ??__F teardown.
// ORIGINAL: 0x00459560 ??__Eg_MAININTERFACE@@YAXXZ 0x00459560-0x00459576
// symbol    ?construct_maininterface@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045EF10 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_maininterface() {
    (ORIGINAL(reinterpret_cast<void *>(0x007AE820))->*MainInterfaceCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00459580));
}

/*
Purpose: Dynamic initializer for g_MAININTERFACE_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00459590 ??__Eg_MAININTERFACE_WAVE@@YAXXZ 0x00459590-0x004595A6 BYTE_EXACT
// symbol    ?construct_maininterface_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_maininterface_wave() {
    g_MAININTERFACE_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004595B0));
}

/*
Purpose: Dynamic initializer for g_JACKAL_FONT; registers its ??__F teardown.
// ORIGINAL: 0x0045F920 ??__Eg_JACKAL_FONT@@YAXXZ 0x0045F920-0x0045F936
// symbol    ?construct_jackal_font@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00618EA0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_jackal_font() {
    (ORIGINAL(&g_JACKAL_FONT)->*FontInitCtor)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0045F940));
}

/*
Purpose: Dynamic initializer for g_MAPWIN; registers its ??__F teardown.
// ORIGINAL: 0x00462080 ??__Eg_MAPWIN@@YAXXZ 0x00462080-0x00462098
// symbol    ?construct_mapwin@@YAXXZ
// size      24 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004626E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_mapwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007D4060))->*MapWinCtorTarget)(1);
    atexit(reinterpret_cast<func_atexit_callback *>(0x004620A0));
}

/*
Purpose: Dynamic initializer for g_MESSAGEWIN; registers its ??__F teardown.
// ORIGINAL: 0x00471320 ??__Eg_MESSAGEWIN@@YAXXZ 0x00471320-0x00471336
// symbol    ?construct_messagewin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00472190 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_messagewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007F67F8))->*MessageWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00471340));
}

/*
Purpose: Dynamic initializer for g_IFACE_GREEN_RIGHT_ARROW_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x00471350 ??__Eg_IFACE_GREEN_RIGHT_ARROW_SPRITE@@YAXXZ 0x00471350-0x00471374
// symbol    ?construct_iface_green_right_arrow_sprite@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_green_right_arrow_sprite() {
    VectorCtorIterator(g_IFACE_GREEN_RIGHT_ARROW_SPRITE, 0x2C, 1, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x00471380));
}

/*
Purpose: Dynamic initializer for g_MONUWIN; registers its ??__F teardown.
// ORIGINAL: 0x00472200 ??__Eg_MONUWIN@@YAXXZ 0x00472200-0x00472216
// symbol    ?construct_monuwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00477C80 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_monuwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007F9F58))->*MonuWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00472220));
}

/*
Purpose: Dynamic initializer for g_MULTIWIN; registers its ??__F teardown.
// ORIGINAL: 0x00477E10 ??__Eg_MULTIWIN@@YAXXZ 0x00477E10-0x00477E26
// symbol    ?construct_multiwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047A590 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_multiwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007FD648))->*MultiWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00477E30));
}

/*
Purpose: Dynamic initializer for g_MULTIWIN_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00477E40 ??__Eg_MULTIWIN_WAVE@@YAXXZ 0x00477E40-0x00477E56 BYTE_EXACT
// symbol    ?construct_multiwin_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_multiwin_wave() {
    g_MULTIWIN_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00477E60));
}

/*
Purpose: Dynamic initializer for g_NETMSG1; registers its ??__F teardown.
// ORIGINAL: 0x0047A770 ??__Eg_NETMSG1@@YAXXZ 0x0047A770-0x0047A78F
// symbol    ?construct_netmsg1@@YAXXZ
// size      31 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047ACF0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_netmsg1() {
    (ORIGINAL(reinterpret_cast<void *>(0x00805338))->*NetMsgCtorTarget)(4096, 8, 0);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0047A790));
}

/*
Purpose: Dynamic initializer for g_NETMSG2; registers its ??__F teardown.
// ORIGINAL: 0x0047A7A0 ??__Eg_NETMSG2@@YAXXZ 0x0047A7A0-0x0047A7BC
// symbol    ?construct_netmsg2@@YAXXZ
// size      28 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0047ACF0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_netmsg2() {
    (ORIGINAL(reinterpret_cast<void *>(0x007FFF80))->*NetMsgCtorTarget)(-5, 40, 1);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0047A7C0));
}

/*
Purpose: Dynamic initializer for g_NETWIN; registers its ??__F teardown.
// ORIGINAL: 0x0047AFF0 ??__Eg_NETWIN@@YAXXZ 0x0047AFF0-0x0047B006
// symbol    ?construct_netwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00481C50 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_netwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0080A6F8))->*NetWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0047B010));
}

/*
Purpose: Dynamic initializer for g_NEWTECHWIN; registers its ??__F teardown.
// ORIGINAL: 0x00483850 ??__Eg_NEWTECHWIN@@YAXXZ 0x00483850-0x00483866
// symbol    ?construct_newtechwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004849D0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_newtechwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00811E40))->*NewTechWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00483870));
}

/*
Purpose: Dynamic initializer for g_NEWTECHWIN_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x00483880 ??__Eg_NEWTECHWIN_SPRITES@@YAXXZ 0x00483880-0x004838A4
// symbol    ?construct_newtechwin_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_newtechwin_sprites() {
    VectorCtorIterator(g_NEWTECHWIN_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x004838B0));
}

/*
Purpose: Dynamic initializer for g_PICKWIN; registers its ??__F teardown.
// ORIGINAL: 0x00488750 ??__Eg_PICKWIN@@YAXXZ 0x00488750-0x00488766
// symbol    ?construct_pickwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048AC10 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_pickwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00822718))->*PickWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00488770));
}

/*
Purpose: Dynamic initializer for g_PLANWIN; registers its ??__F teardown.
// ORIGINAL: 0x0048AE00 ??__Eg_PLANWIN@@YAXXZ 0x0048AE00-0x0048AE18
// symbol    ?construct_planwin@@YAXXZ
// size      24 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0048BCD0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_planwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00834D70))->*PlanWinCtorTarget)(1);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0048AE20));
}

/*
Purpose: Dynamic initializer for g_PREFWIN; registers its ??__F teardown.
// ORIGINAL: 0x0048D4F0 ??__Eg_PREFWIN@@YAXXZ 0x0048D4F0-0x0048D506
// symbol    ?construct_prefwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004921E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_prefwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008578D8))->*PrefWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0048D510));
}

/*
Purpose: Dynamic initializer for g_PREFWIN_BUTTONGROUP; registers its ??__F teardown.
// ORIGINAL: 0x0048D520 ??__Eg_PREFWIN_BUTTONGROUP@@YAXXZ 0x0048D520-0x0048D536 BYTE_EXACT
// symbol    ?construct_prefwin_buttongroup@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062B7C0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_prefwin_buttongroup() {
    g_PREFWIN_BUTTONGROUP->ButtonGroup::ButtonGroup();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0048D540));
}

/*
Purpose: Dynamic initializer for g_QUAYLEWIN; registers its ??__F teardown.
// ORIGINAL: 0x00495170 ??__Eg_QUAYLEWIN@@YAXXZ 0x00495170-0x00495186
// symbol    ?construct_quaylewin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00496810 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_quaylewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00872CB0))->*QuayleWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00495190));
}

/*
Purpose: Dynamic initializer for g_REPORTIF; registers its ??__F teardown.
// ORIGINAL: 0x00496900 ??__Eg_REPORTIF@@YAXXZ 0x00496900-0x00496916
// symbol    ?construct_reportif@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004AD170 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_reportif() {
    (ORIGINAL(reinterpret_cast<void *>(0x00885F38))->*ReportIfCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00496920));
}

/*
Purpose: Dynamic initializer for g_REPORTWIN; registers its ??__F teardown.
// ORIGINAL: 0x00496930 ??__Eg_REPORTWIN@@YAXXZ 0x00496930-0x00496946
// symbol    ?construct_reportwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004AD6B0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_reportwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00876478))->*ReportWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00496950));
}

/*
Purpose: Dynamic initializer for SocialWinParent; registers its ??__F teardown.
// ORIGINAL: 0x004AE9B0 ??__ESocialWinParent@@YAXXZ 0x004AE9B0-0x004AE9C6
// symbol    ?construct_socialwinparent@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004AE9E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_socialwinparent() {
    (ORIGINAL(reinterpret_cast<void *>(0x008A6270))->*SocialWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004AE9D0));
}

/*
Purpose: Dynamic initializer for g_STATUSWIN; registers its ??__F teardown.
// ORIGINAL: 0x004B3FA0 ??__Eg_STATUSWIN@@YAXXZ 0x004B3FA0-0x004B3FB6
// symbol    ?construct_statuswin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004BA1A0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_statuswin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008C5568))->*StatusWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004B3FC0));
}

/*
Purpose: Dynamic initializer for g_TUTWIN; registers its ??__F teardown.
// ORIGINAL: 0x004BA5C0 ??__Eg_TUTWIN@@YAXXZ 0x004BA5C0-0x004BA5D6
// symbol    ?construct_tutwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004BA6B0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_tutwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008C6E68))->*TutWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004BA5E0));
}

/*
Purpose: Dynamic initializer for g_VEHDRAW_CAVIAR; registers its ??__F teardown.
// ORIGINAL: 0x004BF6E0 ??__Eg_VEHDRAW_CAVIAR@@YAXXZ 0x004BF6E0-0x004BF6F6 BYTE_EXACT
// symbol    ?construct_vehdraw_caviar@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616DA0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vehdraw_caviar() {
    g_VEHDRAW_CAVIAR->Caviar::Caviar();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004BF700));
}

/*
Purpose: Dynamic initializer for g_VEHDRAW_BUFFER; registers its ??__F teardown.
// ORIGINAL: 0x004BF710 ??__Eg_VEHDRAW_BUFFER@@YAXXZ 0x004BF710-0x004BF726 BYTE_EXACT
// symbol    ?construct_vehdraw_buffer@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vehdraw_buffer() {
    g_VEHDRAW_BUFFER->Buffer::Buffer();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004BF730));
}

/*
Purpose: Dynamic initializer for g_WORLDWIN; registers its ??__F teardown.
// ORIGINAL: 0x004C38B0 ??__Eg_WORLDWIN@@YAXXZ 0x004C38B0-0x004C38C6
// symbol    ?construct_worldwin@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C4BF0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_worldwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008E9F60))->*WorldWinCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004C38D0));
}

/*
Purpose: Dynamic initializer for g_WAVE_DEVICE; registers its ??__F teardown.
// ORIGINAL: 0x004C5C50 ??__Eg_WAVE_DEVICE@@YAXXZ 0x004C5C50-0x004C5C66 BYTE_EXACT
// symbol    ?construct_wave_device@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C4DD0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_wave_device() {
    g_WAVE_DEVICE->Wave_Device::Wave_Device();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004C5C70));
}

/*
Purpose: Dynamic initializer for g_MIDI_DEVICE; registers its ??__F teardown.
// ORIGINAL: 0x004C5C80 ??__Eg_MIDI_DEVICE@@YAXXZ 0x004C5C80-0x004C5C96
// symbol    ?construct_midi_device@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C5740 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_midi_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090D950))->*Midi_DeviceCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004C5CA0));
}

/*
Purpose: Dynamic initializer for g_WAVE_IN_DEVICE; registers its ??__F teardown.
// ORIGINAL: 0x004C5CB0 ??__Eg_WAVE_IN_DEVICE@@YAXXZ 0x004C5CB0-0x004C5CC6
// symbol    ?construct_wave_in_device@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C5940 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_wave_in_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090DB50))->*Wave_In_DeviceCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x004C5CD0));
}

/*
Purpose: Dynamic initializer for g_CONSOLE; registers its ??__F teardown.
// ORIGINAL: 0x0050E850 ??__Eg_CONSOLE@@YAXXZ 0x0050E850-0x0050E868
// symbol    ?construct_console@@YAXXZ
// size      24 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0050F460 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_console() {
    (ORIGINAL(reinterpret_cast<void *>(0x009156B0))->*ConsoleCtorTarget)(1);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0050E870));
}

/*
Purpose: Dynamic initializer for g_CONSOLE_TIMER; registers its ??__F teardown.
// ORIGINAL: 0x0050E980 ??__Eg_CONSOLE_TIMER@@YAXXZ 0x0050E980-0x0050E996 BYTE_EXACT
// symbol    ?construct_console_timer@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006161D0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_console_timer() {
    g_CONSOLE_TIMER->Time::Time();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0050E9A0));
}

/*
Purpose: Dynamic initializer for g_CURSOR_SPRITES; registers its ??__F teardown.
// ORIGINAL: 0x0052DAC0 ??__Eg_CURSOR_SPRITES@@YAXXZ 0x0052DAC0-0x0052DAE4
// symbol    ?construct_cursor_sprites@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_cursor_sprites() {
    VectorCtorIterator(g_CURSOR_SPRITES, 0x2C, 12, SpriteElementCtor, SpriteElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x0052DAF0));
}

/*
Purpose: Dynamic initializer for g_NETDAEMON; registers its ??__F teardown.
// ORIGINAL: 0x0052DB10 ??__Eg_NETDAEMON@@YAXXZ 0x0052DB10-0x0052DB26
// symbol    ?construct_netdaemon@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005389F0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_netdaemon() {
    (ORIGINAL(reinterpret_cast<void *>(0x0093CD90))->*NetDaemonCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0052DB30));
}

/*
Purpose: Dynamic initializer for g_FONTQUEUE_VAL2; registers its ??__F teardown.
// ORIGINAL: 0x00559230 ??__Eg_FONTQUEUE_VAL2@@YAXXZ 0x00559230-0x00559246
// symbol    ?construct_fontqueue_val2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00559290 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fontqueue_val2() {
    (ORIGINAL(reinterpret_cast<void *>(0x0093FB88))->*FontQueueCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00559250));
}

/*
Purpose: Dynamic initializer for g_FONTQUEUE_VAL1; registers its ??__F teardown.
// ORIGINAL: 0x00559260 ??__Eg_FONTQUEUE_VAL1@@YAXXZ 0x00559260-0x00559276
// symbol    ?construct_fontqueue_val1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00559290 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fontqueue_val1() {
    (ORIGINAL(reinterpret_cast<void *>(0x0093FAE8))->*FontQueueCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00559280));
}

/*
Purpose: Dynamic initializer for g_TOP_MENU_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x00584D00 ??__Eg_TOP_MENU_WAVE@@YAXXZ 0x00584D00-0x00584D16 BYTE_EXACT
// symbol    ?construct_top_menu_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_top_menu_wave() {
    g_TOP_MENU_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00584D20));
}

/*
Purpose: Dynamic initializer for g_FONTS; registers its ??__F teardown.
// ORIGINAL: 0x005882A0 ??__Eg_FONTS@@YAXXZ 0x005882A0-0x005882C4
// symbol    ?construct_fonts@@YAXXZ
// size      36 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fonts() {
    VectorCtorIterator(g_FONTS, 0x28, 48, FontElementCtor, FontElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x005882D0));
}

/*
Purpose: Dynamic initializer for g_CRASH_LANDING_WAVE; registers its ??__F teardown.
// ORIGINAL: 0x005AE0F0 ??__Eg_CRASH_LANDING_WAVE@@YAXXZ 0x005AE0F0-0x005AE106 BYTE_EXACT
// symbol    ?construct_crash_landing_wave@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_crash_landing_wave() {
    g_CRASH_LANDING_WAVE->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x005AE110));
}

/*
Purpose: Dynamic initializer for g_WAVE_GENERAL; registers its ??__F teardown.
// ORIGINAL: 0x005B9C10 ??__Eg_WAVE_GENERAL@@YAXXZ 0x005B9C10-0x005B9C26 BYTE_EXACT
// symbol    ?construct_wave_general@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C66E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_wave_general() {
    g_WAVE_GENERAL->Wave::Wave();
    atexit(reinterpret_cast<func_atexit_callback *>(0x005B9C30));
}

/*
Purpose: Dynamic initializer for g_PALETTE1; registers its ??__F teardown.
// ORIGINAL: 0x005BEC40 ??__Eg_PALETTE1@@YAXXZ 0x005BEC40-0x005BEC56
// symbol    ?construct_palette1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE2A0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_palette1() {
    (ORIGINAL(reinterpret_cast<void *>(0x0094C590))->*PaletteCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x005BEC60));
}

/*
Purpose: Dynamic initializer for g_PALETTE2; registers its ??__F teardown.
// ORIGINAL: 0x005BEC70 ??__Eg_PALETTE2@@YAXXZ 0x005BEC70-0x005BEC86
// symbol    ?construct_palette2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE2A0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_palette2() {
    (ORIGINAL(reinterpret_cast<void *>(0x009523A0))->*PaletteCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x005BEC90));
}

/*
Purpose: Dynamic initializer for g_MULTIDEBUG; registers its ??__F teardown.
// ORIGINAL: 0x005C97C0 ??__Eg_MULTIDEBUG@@YAXXZ 0x005C97C0-0x005C97D6
// symbol    ?construct_multidebug@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005C97F0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_multidebug() {
    (ORIGINAL(reinterpret_cast<void *>(0x009B22F0))->*MultiDebugCtorTarget)();
    atexit(reinterpret_cast<func_atexit_callback *>(0x005C97E0));
}

/*
Purpose: Dynamic initializer for g_BUFFER_SPRITE; registers its ??__F teardown.
// ORIGINAL: 0x005D71D0 ??__Eg_BUFFER_SPRITE@@YAXXZ 0x005D71D0-0x005D71E6 BYTE_EXACT
// symbol    ?construct_buffer_sprite@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_buffer_sprite() {
    g_BUFFER_SPRITE->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x005D71F0));
}

/*
Purpose: Dynamic initializer for g_BUFFER; registers its ??__F teardown.
// ORIGINAL: 0x005E37B0 ??__Eg_BUFFER@@YAXXZ 0x005E37B0-0x005E37C6 BYTE_EXACT
// symbol    ?construct_buffer@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_buffer() {
    g_BUFFER->Buffer::Buffer();
    atexit(reinterpret_cast<func_atexit_callback *>(0x005E37D0));
}

/*
Purpose: Dynamic initializer for g_WIN_BUFFER; registers its ??__F teardown.
// ORIGINAL: 0x005EB350 ??__Eg_WIN_BUFFER@@YAXXZ 0x005EB350-0x005EB366 BYTE_EXACT
// symbol    ?construct_win_buffer@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_win_buffer() {
    g_WIN_BUFFER->Buffer::Buffer();
    atexit(reinterpret_cast<func_atexit_callback *>(0x005EB370));
}

/*
Purpose: Dynamic initializer for TxtIndex; registers its ??__F teardown.
// ORIGINAL: 0x005FD4E0 ??__ETxtIndex@@YAXXZ 0x005FD4E0-0x005FD507
// symbol    ?construct_txtindex@@YAXXZ
// size      39 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645398 0x006457C2
Return Value: n/a
Status: Complete
*/
void __cdecl construct_txtindex() {
    VectorCtorIterator(TxtIndexGlobal, 0x118, 4, TextIndexElementCtor, TextIndexElementTeardown);
    atexit(reinterpret_cast<func_atexit_callback *>(0x005FD510));
}

/*
Purpose: Dynamic initializer for g_RADIOBUTTON_SPRITE_1; registers its ??__F teardown.
// ORIGINAL: 0x0060D060 ??__Eg_RADIOBUTTON_SPRITE_1@@YAXXZ 0x0060D060-0x0060D076 BYTE_EXACT
// symbol    ?construct_radiobutton_sprite_1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radiobutton_sprite_1() {
    g_RADIOBUTTON_SPRITE_1->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0060D080));
}

/*
Purpose: Dynamic initializer for g_RADIOBUTTON_SPRITE_2; registers its ??__F teardown.
// ORIGINAL: 0x0060D0A0 ??__Eg_RADIOBUTTON_SPRITE_2@@YAXXZ 0x0060D0A0-0x0060D0B6 BYTE_EXACT
// symbol    ?construct_radiobutton_sprite_2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radiobutton_sprite_2() {
    g_RADIOBUTTON_SPRITE_2->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0060D0C0));
}

/*
Purpose: Dynamic initializer for g_CHECKBOX_SPRITE_1; registers its ??__F teardown.
// ORIGINAL: 0x0060E5F0 ??__Eg_CHECKBOX_SPRITE_1@@YAXXZ 0x0060E5F0-0x0060E606 BYTE_EXACT
// symbol    ?construct_checkbox_sprite_1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_checkbox_sprite_1() {
    g_CHECKBOX_SPRITE_1->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0060E610));
}

/*
Purpose: Dynamic initializer for g_CHECKBOX_SPRITE_2; registers its ??__F teardown.
// ORIGINAL: 0x0060E630 ??__Eg_CHECKBOX_SPRITE_2@@YAXXZ 0x0060E630-0x0060E646 BYTE_EXACT
// symbol    ?construct_checkbox_sprite_2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_checkbox_sprite_2() {
    g_CHECKBOX_SPRITE_2->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x0060E650));
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_1; registers its ??__F teardown.
// ORIGINAL: 0x00613790 ??__Eg_FILEWIN_SPRITE_1@@YAXXZ 0x00613790-0x006137A6 BYTE_EXACT
// symbol    ?construct_filewin_sprite_1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_1() {
    g_FILEWIN_SPRITE_1->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x006137B0));
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_2; registers its ??__F teardown.
// ORIGINAL: 0x006137D0 ??__Eg_FILEWIN_SPRITE_2@@YAXXZ 0x006137D0-0x006137E6 BYTE_EXACT
// symbol    ?construct_filewin_sprite_2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_2() {
    g_FILEWIN_SPRITE_2->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x006137F0));
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_3; registers its ??__F teardown.
// ORIGINAL: 0x00613810 ??__Eg_FILEWIN_SPRITE_3@@YAXXZ 0x00613810-0x00613826 BYTE_EXACT
// symbol    ?construct_filewin_sprite_3@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E37E0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_3() {
    g_FILEWIN_SPRITE_3->Sprite::Sprite();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00613830));
}

/*
Purpose: Dynamic initializer for StringTable; registers its ??__F teardown.
// ORIGINAL: 0x006168A0 ??__EStringTable@@YAXXZ 0x006168A0-0x006168B6 BYTE_EXACT
// symbol    ?construct_stringtable@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006168D0 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_stringtable() {
    StringTable->Strings::Strings();
    atexit(reinterpret_cast<func_atexit_callback *>(0x006168C0));
}

/*
Purpose: Dynamic initializer for g_CAVIAR_BUFFER_1; registers its ??__F teardown.
// ORIGINAL: 0x00616AA0 ??__Eg_CAVIAR_BUFFER_1@@YAXXZ 0x00616AA0-0x00616AB6 BYTE_EXACT
// symbol    ?construct_caviar_buffer_1@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_caviar_buffer_1() {
    g_CAVIAR_BUFFER_1->Buffer::Buffer();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00616AC0));
}

/*
Purpose: Dynamic initializer for g_CAVIAR_BUFFER_2; registers its ??__F teardown.
// ORIGINAL: 0x00616AE0 ??__Eg_CAVIAR_BUFFER_2@@YAXXZ 0x00616AE0-0x00616AF6 BYTE_EXACT
// symbol    ?construct_caviar_buffer_2@@YAXXZ
// size      22 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x00645398
Return Value: n/a
Status: Complete
*/
void __cdecl construct_caviar_buffer_2() {
    g_CAVIAR_BUFFER_2->Buffer::Buffer();
    atexit(reinterpret_cast<func_atexit_callback *>(0x00616B00));
}
