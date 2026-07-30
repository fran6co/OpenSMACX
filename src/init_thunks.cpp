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
func_game_atexit *GameAtexit =
    (func_game_atexit *)0x00645398;

// Per-element constructors the array walks dispatch
// to, bound at the original addresses the ??_L call
// sites pass; the hybrid redirects each to its
// recovered body at run time.
func_thiscall_teardown *BufferElementCtor =
    (func_thiscall_teardown *)0x005D7210;
func_thiscall_teardown *CaviarDataElementCtor =
    (func_thiscall_teardown *)0x00616BC0;
func_thiscall_teardown *FontElementCtor =
    (func_thiscall_teardown *)0x00618EA0;
func_thiscall_teardown *SpriteElementCtor =
    (func_thiscall_teardown *)0x005E37E0;
func_thiscall_teardown *TextIndexElementCtor =
    (func_thiscall_teardown *)0x005FDF40;
func_thiscall_teardown *TextureElementCtor =
    (func_thiscall_teardown *)0x00619650;
func_thiscall_teardown *WaveElementCtor =
    (func_thiscall_teardown *)0x004C66E0;

namespace {

// Source-owned trampolines the scalar seams default to:
// each runs the recovered construction on the object the
// thunk names. The hybrid executes these; leaf tests
// rebind the seams to recorders instead.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall ambienceinitctor_default(void *object) {
    static_cast<Ambience *>(object)->construct();
}
void __thiscall bufferinitctor_default(void *object) {
    static_cast<Buffer *>(object)->construct();
}
void __thiscall buttongroupinitctor_default(void *object) {
    new (object) ButtonGroup();
}
void __thiscall caviardatainitctor_default(void *object) {
    new (object) CaviarData();
}
void __thiscall caviarinitctor_default(void *object) {
    new (object) Caviar();
}
void __thiscall fxinitctor_default(void *object) {
    new (object) FX();
}
void __thiscall fontinitctor_default(void *object) {
    new (object) Font();
}
void __thiscall spriteinitctor_default(void *object) {
    new (object) Sprite();
}
void __thiscall stringsinitctor_default(void *object) {
    new (object) Strings();
}
void __thiscall textureinitctor_default(void *object) {
    new (object) Texture();
}
void __thiscall timeinitctor_default(void *object) {
    new (object) Time();
}
void __thiscall wavedeviceinitctor_default(void *object) {
    new (object) Wave_Device();
}
void __thiscall waveinitctor_default(void *object) {
    new (object) Wave();
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

}  // namespace

func_thiscall_teardown *AmbienceInitCtor = &ambienceinitctor_default;
func_thiscall_teardown *BufferInitCtor = &bufferinitctor_default;
func_thiscall_teardown *ButtonGroupInitCtor = &buttongroupinitctor_default;
func_thiscall_teardown *CaviarDataInitCtor = &caviardatainitctor_default;
func_thiscall_teardown *CaviarInitCtor = &caviarinitctor_default;
func_thiscall_teardown *FXInitCtor = &fxinitctor_default;
func_thiscall_teardown *FontInitCtor = &fontinitctor_default;
func_thiscall_teardown *SpriteInitCtor = &spriteinitctor_default;
func_thiscall_teardown *StringsInitCtor = &stringsinitctor_default;
func_thiscall_teardown *TextureInitCtor = &textureinitctor_default;
func_thiscall_teardown *TimeInitCtor = &timeinitctor_default;
func_thiscall_teardown *WaveDeviceInitCtor = &wavedeviceinitctor_default;
func_thiscall_teardown *WaveInitCtor = &waveinitctor_default;

// The opaque-storage constructors, each defaulting to
// the original the thunk calls; the hybrid redirects
// it to a recovered body at run time.
func_thiscall_teardown *BaseWinCtorTarget =
    (func_thiscall_teardown *)0x00408490;
func_thiscall_teardown *BattleWinCtorTarget =
    (func_thiscall_teardown *)0x00422EE0;
func_thiscall_teardown *CouncWinCtorTarget =
    (func_thiscall_teardown *)0x00428620;
func_thiscall_teardown *DatalinkCtorTarget =
    (func_thiscall_teardown *)0x00428FC0;
func_thiscall_teardown *DesignWinCtorTarget =
    (func_thiscall_teardown *)0x00434480;
func_thiscall_teardown *DiploPopCtorTarget =
    (func_thiscall_teardown *)0x0043EFF0;
func_thiscall_teardown *DiploWinCtorTarget =
    (func_thiscall_teardown *)0x00444FC0;
func_thiscall_teardown *FameWinCtorTarget =
    (func_thiscall_teardown *)0x0044B200;
func_thiscall_teardown *FactionArtCtorTarget =
    (func_thiscall_teardown *)0x00455F10;
func_thiscall_teardown *InfoWinCtorTarget =
    (func_thiscall_teardown *)0x00459500;
func_thiscall_teardown *MainInterfaceCtorTarget =
    (func_thiscall_teardown *)0x0045EF10;
func_opaque_ctor_i *MapWinCtorTarget =
    (func_opaque_ctor_i *)0x004626E0;
func_thiscall_teardown *MessageWinCtorTarget =
    (func_thiscall_teardown *)0x00472190;
func_thiscall_teardown *MonuWinCtorTarget =
    (func_thiscall_teardown *)0x00477C80;
func_thiscall_teardown *MultiWinCtorTarget =
    (func_thiscall_teardown *)0x0047A590;
func_opaque_ctor_iii *NetMsgCtorTarget =
    (func_opaque_ctor_iii *)0x0047ACF0;
func_thiscall_teardown *NetWinCtorTarget =
    (func_thiscall_teardown *)0x00481C50;
func_thiscall_teardown *NewTechWinCtorTarget =
    (func_thiscall_teardown *)0x004849D0;
func_thiscall_teardown *PickWinCtorTarget =
    (func_thiscall_teardown *)0x0048AC10;
func_opaque_ctor_i *PlanWinCtorTarget =
    (func_opaque_ctor_i *)0x0048BCD0;
func_thiscall_teardown *PrefWinCtorTarget =
    (func_thiscall_teardown *)0x004921E0;
func_thiscall_teardown *QuayleWinCtorTarget =
    (func_thiscall_teardown *)0x00496810;
func_thiscall_teardown *ReportIfCtorTarget =
    (func_thiscall_teardown *)0x004AD170;
func_thiscall_teardown *ReportWinCtorTarget =
    (func_thiscall_teardown *)0x004AD6B0;
func_thiscall_teardown *SocialWinCtorTarget =
    (func_thiscall_teardown *)0x004AE9E0;
func_thiscall_teardown *StatusWinCtorTarget =
    (func_thiscall_teardown *)0x004BA1A0;
func_thiscall_teardown *TutWinCtorTarget =
    (func_thiscall_teardown *)0x004BA6B0;
func_thiscall_teardown *WorldWinCtorTarget =
    (func_thiscall_teardown *)0x004C4BF0;
func_thiscall_teardown *Midi_DeviceCtorTarget =
    (func_thiscall_teardown *)0x004C5740;
func_thiscall_teardown *Wave_In_DeviceCtorTarget =
    (func_thiscall_teardown *)0x004C5940;
func_opaque_ctor_i *ConsoleCtorTarget =
    (func_opaque_ctor_i *)0x0050F460;
func_thiscall_teardown *NetDaemonCtorTarget =
    (func_thiscall_teardown *)0x005389F0;
func_thiscall_teardown *FontQueueCtorTarget =
    (func_thiscall_teardown *)0x00559290;
func_thiscall_teardown *MultiDebugCtorTarget =
    (func_thiscall_teardown *)0x005C97F0;
func_thiscall_teardown *PaletteCtorTarget =
    (func_thiscall_teardown *)0x005FE2A0;

Sprite *g_NEWTECHWIN_SPRITES = (Sprite *)0x00822610;
Caviar *g_VEHDRAW_CAVIAR = (Caviar *)0x008CC828;
Wave_Device *g_WAVE_DEVICE = (Wave_Device *)0x0090D978;
Time *g_CONSOLE_TIMER = (Time *)0x00939E88;
Sprite *g_BUFFER_SPRITE = (Sprite *)0x009B3A50;
Buffer *g_WIN_BUFFER = (Buffer *)0x009B6F08;
Sprite *g_RADIOBUTTON_SPRITE_1 = (Sprite *)0x009B8EF8;
Sprite *g_RADIOBUTTON_SPRITE_2 = (Sprite *)0x009B8F28;
Sprite *g_CHECKBOX_SPRITE_1 = (Sprite *)0x009B8F60;
Sprite *g_CHECKBOX_SPRITE_2 = (Sprite *)0x009B8F90;
Sprite *g_FILEWIN_SPRITE_1 = (Sprite *)0x009B9048;
Sprite *g_FILEWIN_SPRITE_2 = (Sprite *)0x009B9018;
Sprite *g_FILEWIN_SPRITE_3 = (Sprite *)0x009B9078;
Buffer *g_CAVIAR_BUFFER_1 = (Buffer *)0x009B9108;
Buffer *g_CAVIAR_BUFFER_2 = (Buffer *)0x009B96B0;

/*
Purpose: Dynamic initializer for g_ALPHAMENU_WAVE; registers its ??__F teardown.
Original Offset: 00402F10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_alphamenu_wave() {
    WaveInitCtor(g_ALPHAMENU_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00402F30));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR02; registers its ??__F teardown.
Original Offset: 00404440
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var02() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR02);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404460));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR11; registers its ??__F teardown.
Original Offset: 00404470
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var11() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR11);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404490));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR06; registers its ??__F teardown.
Original Offset: 004044A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var06() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR06);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004044C0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR09; registers its ??__F teardown.
Original Offset: 004044D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var09() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR09);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004044F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR21; registers its ??__F teardown.
Original Offset: 00404500
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var21() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR21);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404520));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR05; registers its ??__F teardown.
Original Offset: 00404530
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var05() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR05);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404550));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR08; registers its ??__F teardown.
Original Offset: 00404560
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var08() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR08);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404580));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR04; registers its ??__F teardown.
Original Offset: 00404590
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var04() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR04);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004045B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR01; registers its ??__F teardown.
Original Offset: 004045C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var01() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR01);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004045E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR18; registers its ??__F teardown.
Original Offset: 004045F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var18() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR18);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404610));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR03; registers its ??__F teardown.
Original Offset: 00404620
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var03() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR03);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404640));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR20; registers its ??__F teardown.
Original Offset: 00404650
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var20() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR20);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404670));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR16; registers its ??__F teardown.
Original Offset: 00404680
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var16() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR16);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004046A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR14; registers its ??__F teardown.
Original Offset: 004046B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var14() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR14);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004046D0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR22; registers its ??__F teardown.
Original Offset: 004046E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var22() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR22);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404700));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR10; registers its ??__F teardown.
Original Offset: 00404710
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var10() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR10);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404730));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR15; registers its ??__F teardown.
Original Offset: 00404740
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var15() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR15);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404760));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR13; registers its ??__F teardown.
Original Offset: 00404770
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var13() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR13);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404790));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR17; registers its ??__F teardown.
Original Offset: 004047A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var17() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR17);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004047C0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR19; registers its ??__F teardown.
Original Offset: 004047D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var19() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR19);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004047F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR12; registers its ??__F teardown.
Original Offset: 00404800
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var12() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR12);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404820));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR07; registers its ??__F teardown.
Original Offset: 00404830
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var07() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR07);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00404850));
}

/*
Purpose: Dynamic initializer for g_BASEWIN; registers its ??__F teardown.
Original Offset: 004083B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_basewin() {
    BaseWinCtorTarget(reinterpret_cast<void *>(0x006A7628));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004083D0));
}

/*
Purpose: Dynamic initializer for g_BASEWIN_WAVE; registers its ??__F teardown.
Original Offset: 004083E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_basewin_wave() {
    WaveInitCtor(g_BASEWIN_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00408400));
}

/*
Purpose: Dynamic initializer for g_BattleWin; registers its ??__F teardown.
Original Offset: 004219B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battlewin() {
    BattleWinCtorTarget(reinterpret_cast<void *>(0x006EEED8));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004219D0));
}

/*
Purpose: Dynamic initializer for g_COUNCWIN; registers its ??__F teardown.
Original Offset: 004243A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_councwin() {
    CouncWinCtorTarget(reinterpret_cast<void *>(0x006FEC80));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004243C0));
}

/*
Purpose: Dynamic initializer for g_CREDITS_WAVE; registers its ??__F teardown.
Original Offset: 00428750
Return Value: n/a
Status: Complete
*/
void __cdecl construct_credits_wave() {
    WaveInitCtor(g_CREDITS_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00428770));
}

/*
Purpose: Dynamic initializer for g_DATALINK; registers its ??__F teardown.
Original Offset: 00428F90
Return Value: n/a
Status: Complete
*/
void __cdecl construct_datalink() {
    DatalinkCtorTarget(reinterpret_cast<void *>(0x00703EA0));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00428FB0));
}

/*
Purpose: Dynamic initializer for g_DESIGNWIN; registers its ??__F teardown.
Original Offset: 00432850
Return Value: n/a
Status: Complete
*/
void __cdecl construct_designwin() {
    DesignWinCtorTarget(reinterpret_cast<void *>(0x0071F2B0));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00432870));
}

/*
Purpose: Dynamic initializer for g_DESIGNWIN_WAVE; registers its ??__F teardown.
Original Offset: 00432880
Return Value: n/a
Status: Complete
*/
void __cdecl construct_designwin_wave() {
    WaveInitCtor(g_DESIGNWIN_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004328A0));
}

/*
Purpose: Dynamic initializer for g_DIPLOPOP; registers its ??__F teardown.
Original Offset: 0043EFC0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_diplopop() {
    DiploPopCtorTarget(reinterpret_cast<void *>(0x00733990));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0043EFE0));
}

/*
Purpose: Dynamic initializer for g_DIPLOWIN; registers its ??__F teardown.
Original Offset: 00440F00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_diplowin() {
    DiploWinCtorTarget(reinterpret_cast<void *>(0x0073ACD8));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00440F20));
}

/*
Purpose: Dynamic initializer for g_CPU_WAVES; registers its ??__F teardown.
Original Offset: 00445450
Return Value: n/a
Status: Complete
*/
void __cdecl construct_cpu_waves() {
    VectorCtorIterator(g_CPU_WAVES, 0x6C, 45, WaveElementCtor, WaveElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00445480));
}

/*
Purpose: Dynamic initializer for g_MENU_UP_WAVE; registers its ??__F teardown.
Original Offset: 004454A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_menu_up_wave() {
    WaveInitCtor(g_MENU_UP_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004454C0));
}

/*
Purpose: Dynamic initializer for g_MENU_DOWN_WAVE; registers its ??__F teardown.
Original Offset: 004454D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_menu_down_wave() {
    WaveInitCtor(g_MENU_DOWN_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004454F0));
}

/*
Purpose: Dynamic initializer for g_SCOOT_WAVE; registers its ??__F teardown.
Original Offset: 00445500
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scoot_wave() {
    WaveInitCtor(g_SCOOT_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00445520));
}

/*
Purpose: Dynamic initializer for g_OK_WAVE; registers its ??__F teardown.
Original Offset: 00445530
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ok_wave() {
    WaveInitCtor(g_OK_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00445550));
}

/*
Purpose: Dynamic initializer for g_PASSOVER_WAVE; registers its ??__F teardown.
Original Offset: 00445560
Return Value: n/a
Status: Complete
*/
void __cdecl construct_passover_wave() {
    WaveInitCtor(g_PASSOVER_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00445580));
}

/*
Purpose: Dynamic initializer for g_FX; registers its ??__F teardown.
Original Offset: 00445590
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fx() {
    FXInitCtor(g_FX);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004455B0));
}

/*
Purpose: Dynamic initializer for g_AMBIENCE; registers its ??__F teardown.
Original Offset: 004455C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ambience() {
    AmbienceInitCtor(g_AMBIENCE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004455E0));
}

/*
Purpose: Dynamic initializer for g_FAMEWIN; registers its ??__F teardown.
Original Offset: 004483D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_famewin() {
    FameWinCtorTarget(reinterpret_cast<void *>(0x0074DAF8));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004483F0));
}

/*
Purpose: Dynamic initializer for g_PCX_PARSE_TEMP_BUFFER1; registers its ??__F teardown.
Original Offset: 0044CC20
Return Value: n/a
Status: Complete
*/
void __cdecl construct_pcx_parse_temp_buffer1() {
    BufferInitCtor(g_PCX_PARSE_TEMP_BUFFER1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CC40));
}

/*
Purpose: Dynamic initializer for FactionArt; registers its ??__F teardown.
Original Offset: 0044CC50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_factionart() {
    VectorCtorIterator(reinterpret_cast<void *>(0x0078E978), 0x65C, 8,
                       FactionArtCtorTarget, FactionArtElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CC80));
}

/*
Purpose: Dynamic initializer for g_IFACE_CLOSE_X_SPRITES; registers its ??__F teardown.
Original Offset: 0044CCA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_close_x_sprites() {
    VectorCtorIterator(g_IFACE_CLOSE_X_SPRITES, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CCD0));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES1; registers its ??__F teardown.
Original Offset: 0044CCF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites1() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES1, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CD20));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES2; registers its ??__F teardown.
Original Offset: 0044CD40
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites2() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES2, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CD70));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES3; registers its ??__F teardown.
Original Offset: 0044CD90
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites3() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES3, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CDC0));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES4; registers its ??__F teardown.
Original Offset: 0044CDE0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites4() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES4, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CE10));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES5; registers its ??__F teardown.
Original Offset: 0044CE30
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites5() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES5, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CE60));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES6; registers its ??__F teardown.
Original Offset: 0044CE80
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites6() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES6, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CEB0));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES7; registers its ??__F teardown.
Original Offset: 0044CED0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites7() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES7, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CF00));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITES8; registers its ??__F teardown.
Original Offset: 0044CF20
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprites8() {
    VectorCtorIterator(g_IFACE_BOX_SPRITES8, 0x2C, 51, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CF50));
}

/*
Purpose: Dynamic initializer for g_IFACE_BOX_SPRITE_BUFFERS; registers its ??__F teardown.
Original Offset: 0044CF70
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_box_sprite_buffers() {
    VectorCtorIterator(g_IFACE_BOX_SPRITE_BUFFERS, 0x588, 51, BufferElementCtor, BufferElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CFA0));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE; registers its ??__F teardown.
Original Offset: 0044CFC0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_top_left_sprite() {
    SpriteInitCtor(g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044CFE0));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE; registers its ??__F teardown.
Original Offset: 0044CFF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_top_right_sprite() {
    SpriteInitCtor(g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D010));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE; registers its ??__F teardown.
Original Offset: 0044D020
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_bot_left_sprite() {
    SpriteInitCtor(g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D040));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE; registers its ??__F teardown.
Original Offset: 0044D050
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_bot_right_sprite() {
    SpriteInitCtor(g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D070));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_TOP_MID_SPRITE; registers its ??__F teardown.
Original Offset: 0044D080
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_top_mid_sprite() {
    SpriteInitCtor(g_IFACE_STD_POPUPS_TOP_MID_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D0A0));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_BOT_MID_SPRITE; registers its ??__F teardown.
Original Offset: 0044D0B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_bot_mid_sprite() {
    SpriteInitCtor(g_IFACE_STD_POPUPS_BOT_MID_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D0D0));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_MID_LEFT_SPRITE; registers its ??__F teardown.
Original Offset: 0044D0E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_mid_left_sprite() {
    SpriteInitCtor(g_IFACE_STD_POPUPS_MID_LEFT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D100));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE; registers its ??__F teardown.
Original Offset: 0044D110
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_mid_right_sprite() {
    SpriteInitCtor(g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D130));
}

/*
Purpose: Dynamic initializer for g_IFACE_STD_POPUPS_MIDDLE_BUFFER; registers its ??__F teardown.
Original Offset: 0044D140
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_std_popups_middle_buffer() {
    BufferInitCtor(g_IFACE_STD_POPUPS_MIDDLE_BUFFER);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D160));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR1; registers its ??__F teardown.
Original Offset: 0044D170
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var1() {
    CaviarDataInitCtor(g_UNUSED_CAVIARDATA_VAR1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D190));
}

/*
Purpose: Dynamic initializer for g_SSF_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D1A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ssf_caviardata() {
    CaviarDataInitCtor(g_SSF_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D1C0));
}

/*
Purpose: Dynamic initializer for g_SDP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D1D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sdp_caviardata() {
    CaviarDataInitCtor(g_SDP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D1F0));
}

/*
Purpose: Dynamic initializer for g_SAS_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D200
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sas_caviardata() {
    CaviarDataInitCtor(g_SAS_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D220));
}

/*
Purpose: Dynamic initializer for g_SCD_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D230
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scd_caviardata() {
    CaviarDataInitCtor(g_SCD_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D250));
}

/*
Purpose: Dynamic initializer for g_SCJ_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D260
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scj_caviardata() {
    CaviarDataInitCtor(g_SCJ_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D280));
}

/*
Purpose: Dynamic initializer for g_SAGS_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D290
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sags_caviardata() {
    CaviarDataInitCtor(g_SAGS_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D2B0));
}

/*
Purpose: Dynamic initializer for g_SFT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D2C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sft_caviardata() {
    CaviarDataInitCtor(g_SFT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D2E0));
}

/*
Purpose: Dynamic initializer for g_VHR_CAVIARDATA1; registers its ??__F teardown.
Original Offset: 0044D2F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhr_caviardata1() {
    CaviarDataInitCtor(g_VHR_CAVIARDATA1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D310));
}

/*
Purpose: Dynamic initializer for g_SHT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D320
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sht_caviardata() {
    CaviarDataInitCtor(g_SHT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D340));
}

/*
Purpose: Dynamic initializer for g_SRB_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D350
Return Value: n/a
Status: Complete
*/
void __cdecl construct_srb_caviardata() {
    CaviarDataInitCtor(g_SRB_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D370));
}

/*
Purpose: Dynamic initializer for g_ASAS_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D380
Return Value: n/a
Status: Complete
*/
void __cdecl construct_asas_caviardata() {
    CaviarDataInitCtor(g_ASAS_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D3A0));
}

/*
Purpose: Dynamic initializer for g_RESLASER_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D3B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_reslaser_caviardata() {
    CaviarDataInitCtor(g_RESLASER_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D3D0));
}

/*
Purpose: Dynamic initializer for g_RESBOLT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D3E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_resbolt_caviardata() {
    CaviarDataInitCtor(g_RESBOLT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D400));
}

/*
Purpose: Dynamic initializer for g_FUNLOAD_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D410
Return Value: n/a
Status: Complete
*/
void __cdecl construct_funload_caviardata() {
    CaviarDataInitCtor(g_FUNLOAD_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D430));
}

/*
Purpose: Dynamic initializer for g_TECLOAD_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D440
Return Value: n/a
Status: Complete
*/
void __cdecl construct_tecload_caviardata() {
    CaviarDataInitCtor(g_TECLOAD_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D460));
}

/*
Purpose: Dynamic initializer for g_SP_DISSWAVE_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D470
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sp_disswave_caviardata() {
    CaviarDataInitCtor(g_SP_DISSWAVE_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D490));
}

/*
Purpose: Dynamic initializer for g_SP_MARINED_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D4A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sp_marined_caviardata() {
    CaviarDataInitCtor(g_SP_MARINED_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D4C0));
}

/*
Purpose: Dynamic initializer for g_SP_NANOO_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D4D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sp_nanoo_caviardata() {
    CaviarDataInitCtor(g_SP_NANOO_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D4F0));
}

/*
Purpose: Dynamic initializer for g_SP_SOPORIFIC_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D500
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sp_soporific_caviardata() {
    CaviarDataInitCtor(g_SP_SOPORIFIC_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D520));
}

/*
Purpose: Dynamic initializer for g_AA01_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D530
Return Value: n/a
Status: Complete
*/
void __cdecl construct_aa01_caviardata() {
    CaviarDataInitCtor(g_AA01_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D550));
}

/*
Purpose: Dynamic initializer for g_AA_ROVER_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D560
Return Value: n/a
Status: Complete
*/
void __cdecl construct_aa_rover_caviardata() {
    CaviarDataInitCtor(g_AA_ROVER_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D580));
}

/*
Purpose: Dynamic initializer for g_AA_WING_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D590
Return Value: n/a
Status: Complete
*/
void __cdecl construct_aa_wing_caviardata() {
    VectorCtorIterator(g_AA_WING_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D5C0));
}

/*
Purpose: Dynamic initializer for g_ACP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D5E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_acp_caviardata() {
    VectorCtorIterator(g_ACP_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D610));
}

/*
Purpose: Dynamic initializer for g_AX_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D630
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ax_caviardata() {
    CaviarDataInitCtor(g_AX_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D650));
}

/*
Purpose: Dynamic initializer for g_AA_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D660
Return Value: n/a
Status: Complete
*/
void __cdecl construct_aa_caviardata() {
    CaviarDataInitCtor(g_AA_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D680));
}

/*
Purpose: Dynamic initializer for g_ACOLPOD_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D690
Return Value: n/a
Status: Complete
*/
void __cdecl construct_acolpod_caviardata() {
    CaviarDataInitCtor(g_ACOLPOD_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D6B0));
}

/*
Purpose: Dynamic initializer for g_AT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D6C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_at_caviardata() {
    CaviarDataInitCtor(g_AT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D6E0));
}

/*
Purpose: Dynamic initializer for g_VTA_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D6F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vta_caviardata() {
    CaviarDataInitCtor(g_VTA_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D710));
}

/*
Purpose: Dynamic initializer for g_ATP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D720
Return Value: n/a
Status: Complete
*/
void __cdecl construct_atp_caviardata() {
    CaviarDataInitCtor(g_ATP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D740));
}

/*
Purpose: Dynamic initializer for g_SSFA_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D750
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ssfa_caviardata() {
    CaviarDataInitCtor(g_SSFA_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D770));
}

/*
Purpose: Dynamic initializer for g_SFTA_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D780
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sfta_caviardata() {
    CaviarDataInitCtor(g_SFTA_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D7A0));
}

/*
Purpose: Dynamic initializer for g_VW00_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D7B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vw00_caviardata() {
    CaviarDataInitCtor(g_VW00_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D7D0));
}

/*
Purpose: Dynamic initializer for g_VIPTAWL_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D7E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_viptawl_caviardata() {
    CaviarDataInitCtor(g_VIPTAWL_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D800));
}

/*
Purpose: Dynamic initializer for g_VIPTASGN_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D810
Return Value: n/a
Status: Complete
*/
void __cdecl construct_viptasgn_caviardata() {
    CaviarDataInitCtor(g_VIPTASGN_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D830));
}

/*
Purpose: Dynamic initializer for g_VIPTAPSI_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D840
Return Value: n/a
Status: Complete
*/
void __cdecl construct_viptapsi_caviardata() {
    CaviarDataInitCtor(g_VIPTAPSI_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D860));
}

/*
Purpose: Dynamic initializer for g_VIPTR_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D870
Return Value: n/a
Status: Complete
*/
void __cdecl construct_viptr_caviardata() {
    VectorCtorIterator(g_VIPTR_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D8A0));
}

/*
Purpose: Dynamic initializer for g_VIPTA_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D8C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vipta_caviardata() {
    VectorCtorIterator(g_VIPTA_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D8F0));
}

/*
Purpose: Dynamic initializer for g_PTMOD_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D910
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ptmod_caviardata() {
    CaviarDataInitCtor(g_PTMOD_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D930));
}

/*
Purpose: Dynamic initializer for g_VB_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D940
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vb_caviardata() {
    CaviarDataInitCtor(g_VB_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D960));
}

/*
Purpose: Dynamic initializer for g_VBP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D970
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vbp_caviardata() {
    CaviarDataInitCtor(g_VBP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D990));
}

/*
Purpose: Dynamic initializer for g_VGMC_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D9A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgmc_caviardata() {
    CaviarDataInitCtor(g_VGMC_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D9C0));
}

/*
Purpose: Dynamic initializer for g_VGMCP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044D9D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgmcp_caviardata() {
    CaviarDataInitCtor(g_VGMCP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044D9F0));
}

/*
Purpose: Dynamic initializer for g_VLIGHTS_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DA00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vlights_caviardata() {
    CaviarDataInitCtor(g_VLIGHTS_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DA20));
}

/*
Purpose: Dynamic initializer for g_VPT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DA30
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vpt_caviardata() {
    CaviarDataInitCtor(g_VPT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DA50));
}

/*
Purpose: Dynamic initializer for g_A_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DA60
Return Value: n/a
Status: Complete
*/
void __cdecl construct_a_caviardata() {
    CaviarDataInitCtor(g_A_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DA80));
}

/*
Purpose: Dynamic initializer for g_APWALL_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DA90
Return Value: n/a
Status: Complete
*/
void __cdecl construct_apwall_caviardata() {
    CaviarDataInitCtor(g_APWALL_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DAB0));
}

/*
Purpose: Dynamic initializer for g_ASGEN_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DAC0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_asgen_caviardata() {
    CaviarDataInitCtor(g_ASGEN_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DAE0));
}

/*
Purpose: Dynamic initializer for g_APSID_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DAF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_apsid_caviardata() {
    CaviarDataInitCtor(g_APSID_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DB10));
}

/*
Purpose: Dynamic initializer for g_VA01_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DB20
Return Value: n/a
Status: Complete
*/
void __cdecl construct_va01_caviardata() {
    CaviarDataInitCtor(g_VA01_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DB40));
}

/*
Purpose: Dynamic initializer for g_VR_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DB50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vr_caviardata() {
    VectorCtorIterator(g_VR_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DB80));
}

/*
Purpose: Dynamic initializer for g_VHR_CAVIARDATA2; registers its ??__F teardown.
Original Offset: 0044DBA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhr_caviardata2() {
    CaviarDataInitCtor(g_VHR_CAVIARDATA2);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DBC0));
}

/*
Purpose: Dynamic initializer for g_VRC_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DBD0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vrc_caviardata() {
    VectorCtorIterator(g_VRC_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DC00));
}

/*
Purpose: Dynamic initializer for g_VI_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DC20
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vi_caviardata() {
    CaviarDataInitCtor(g_VI_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DC40));
}

/*
Purpose: Dynamic initializer for g_VGMT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DC50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgmt_caviardata() {
    CaviarDataInitCtor(g_VGMT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DC70));
}

/*
Purpose: Dynamic initializer for g_VGMTP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DC80
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgmtp_caviardata() {
    CaviarDataInitCtor(g_VGMTP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DCA0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR2; registers its ??__F teardown.
Original Offset: 0044DCB0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var2() {
    CaviarDataInitCtor(g_UNUSED_CAVIARDATA_VAR2);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DCD0));
}

/*
Purpose: Dynamic initializer for g_VWNTU_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DCE0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vwntu_caviardata() {
    CaviarDataInitCtor(g_VWNTU_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DD00));
}

/*
Purpose: Dynamic initializer for g_VT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DD10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vt_caviardata() {
    CaviarDataInitCtor(g_VT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DD30));
}

/*
Purpose: Dynamic initializer for g_DROP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DD40
Return Value: n/a
Status: Complete
*/
void __cdecl construct_drop_caviardata() {
    CaviarDataInitCtor(g_DROP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DD60));
}

/*
Purpose: Dynamic initializer for g_DROPLET_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DD70
Return Value: n/a
Status: Complete
*/
void __cdecl construct_droplet_caviardata() {
    CaviarDataInitCtor(g_DROPLET_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DD90));
}

/*
Purpose: Dynamic initializer for g_VCL_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DDA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcl_caviardata() {
    CaviarDataInitCtor(g_VCL_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DDC0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR3; registers its ??__F teardown.
Original Offset: 0044DDD0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var3() {
    VectorCtorIterator(g_UNUSED_CAVIARDATA_VAR3, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DE00));
}

/*
Purpose: Dynamic initializer for g_VCLT00_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DE20
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vclt00_caviardata() {
    CaviarDataInitCtor(g_VCLT00_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DE40));
}

/*
Purpose: Dynamic initializer for g_VHT_VBP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DE50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vht_vbp_caviardata() {
    CaviarDataInitCtor(g_VHT_VBP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DE70));
}

/*
Purpose: Dynamic initializer for g_VHTP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DE80
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhtp_caviardata() {
    CaviarDataInitCtor(g_VHTP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DEA0));
}

/*
Purpose: Dynamic initializer for g_VHTA0_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DEB0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhta0_caviardata() {
    VectorCtorIterator(g_VHTA0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DEE0));
}

/*
Purpose: Dynamic initializer for g_VHTTP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DF00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhttp_caviardata() {
    CaviarDataInitCtor(g_VHTTP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DF20));
}

/*
Purpose: Dynamic initializer for g_VHTTPA0_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DF30
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vhttpa0_caviardata() {
    VectorCtorIterator(g_VHTTPA0_CAVIARDATA, 0xC, 3, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DF60));
}

/*
Purpose: Dynamic initializer for g_VSP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DF80
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vsp_caviardata() {
    CaviarDataInitCtor(g_VSP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DFA0));
}

/*
Purpose: Dynamic initializer for g_VSPA0_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044DFB0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vspa0_caviardata() {
    VectorCtorIterator(g_VSPA0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044DFE0));
}

/*
Purpose: Dynamic initializer for g_VSPTF_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E000
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vsptf_caviardata() {
    CaviarDataInitCtor(g_VSPTF_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E020));
}

/*
Purpose: Dynamic initializer for g_VSPTB_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E030
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vsptb_caviardata() {
    CaviarDataInitCtor(g_VSPTB_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E050));
}

/*
Purpose: Dynamic initializer for g_VFL_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E060
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vfl_caviardata() {
    CaviarDataInitCtor(g_VFL_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E080));
}

/*
Purpose: Dynamic initializer for g_VGS_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E090
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgs_caviardata() {
    CaviarDataInitCtor(g_VGS_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E0B0));
}

/*
Purpose: Dynamic initializer for g_VGSP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E0C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vgsp_caviardata() {
    CaviarDataInitCtor(g_VGSP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E0E0));
}

/*
Purpose: Dynamic initializer for g_VJTP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E0F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vjtp_caviardata() {
    CaviarDataInitCtor(g_VJTP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E110));
}

/*
Purpose: Dynamic initializer for g_VJT0_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E120
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vjt0_caviardata() {
    VectorCtorIterator(g_VJT0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E150));
}

/*
Purpose: Dynamic initializer for g_VCU_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E170
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcu_caviardata() {
    CaviarDataInitCtor(g_VCU_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E190));
}

/*
Purpose: Dynamic initializer for g_VCUP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E1A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcup_caviardata() {
    CaviarDataInitCtor(g_VCUP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E1C0));
}

/*
Purpose: Dynamic initializer for g_VCUA0_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E1D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcua0_caviardata() {
    VectorCtorIterator(g_VCUA0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E200));
}

/*
Purpose: Dynamic initializer for g_VCUW_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E220
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vcuw_caviardata() {
    CaviarDataInitCtor(g_VCUW_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E240));
}

/*
Purpose: Dynamic initializer for g_VCT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E250
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vct_caviardata() {
    CaviarDataInitCtor(g_VCT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E270));
}

/*
Purpose: Dynamic initializer for g_VCTP_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E280
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vctp_caviardata() {
    CaviarDataInitCtor(g_VCTP_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E2A0));
}

/*
Purpose: Dynamic initializer for g_VCTB_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E2B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vctb_caviardata() {
    CaviarDataInitCtor(g_VCTB_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E2D0));
}

/*
Purpose: Dynamic initializer for g_VCT0_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E2E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vct0_caviardata() {
    VectorCtorIterator(g_VCT0_CAVIARDATA, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E310));
}

/*
Purpose: Dynamic initializer for g_VWNTT_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E330
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vwntt_caviardata() {
    CaviarDataInitCtor(g_VWNTT_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E350));
}

/*
Purpose: Dynamic initializer for g_VWNST_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E360
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vwnst_caviardata() {
    CaviarDataInitCtor(g_VWNST_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E380));
}

/*
Purpose: Dynamic initializer for g_VWNAA_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E390
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vwnaa_caviardata() {
    CaviarDataInitCtor(g_VWNAA_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E3B0));
}

/*
Purpose: Dynamic initializer for g_VW_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E3C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vw_caviardata() {
    VectorCtorIterator(g_VW_CAVIARDATA, 0xC, 16, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E3F0));
}

/*
Purpose: Dynamic initializer for g_VM_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E410
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vm_caviardata() {
    CaviarDataInitCtor(g_VM_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E430));
}

/*
Purpose: Dynamic initializer for g_VM13_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E440
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vm13_caviardata() {
    CaviarDataInitCtor(g_VM13_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E460));
}

/*
Purpose: Dynamic initializer for g_VPBR0_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E470
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vpbr0_caviardata() {
    VectorCtorIterator(g_VPBR0_CAVIARDATA, 0xC, 4, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E4A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR4; registers its ??__F teardown.
Original Offset: 0044E4C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var4() {
    VectorCtorIterator(g_UNUSED_CAVIARDATA_VAR4, 0xC, 2, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E4F0));
}

/*
Purpose: Dynamic initializer for g_NW_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E510
Return Value: n/a
Status: Complete
*/
void __cdecl construct_nw_caviardata() {
    CaviarDataInitCtor(g_NW_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E530));
}

/*
Purpose: Dynamic initializer for g_NI_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E540
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ni_caviardata() {
    CaviarDataInitCtor(g_NI_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E560));
}

/*
Purpose: Dynamic initializer for g_NLC_CAVIARDATA; registers its ??__F teardown.
Original Offset: 0044E570
Return Value: n/a
Status: Complete
*/
void __cdecl construct_nlc_caviardata() {
    CaviarDataInitCtor(g_NLC_CAVIARDATA);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E590));
}

/*
Purpose: Dynamic initializer for g_UNUSED_CAVIARDATA_VAR5; registers its ??__F teardown.
Original Offset: 0044E5A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_caviardata_var5() {
    VectorCtorIterator(g_UNUSED_CAVIARDATA_VAR5, 0xC, 5, CaviarDataElementCtor, CaviarDataElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E5D0));
}

/*
Purpose: Dynamic initializer for g_RADIUS1_TEXTURE; registers its ??__F teardown.
Original Offset: 0044E5F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radius1_texture() {
    TextureInitCtor(g_RADIUS1_TEXTURE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E610));
}

/*
Purpose: Dynamic initializer for g_RADIUS2_TEXTURE; registers its ??__F teardown.
Original Offset: 0044E620
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radius2_texture() {
    TextureInitCtor(g_RADIUS2_TEXTURE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E640));
}

/*
Purpose: Dynamic initializer for g_ROCKY_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E650
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rocky_textures() {
    VectorCtorIterator(g_ROCKY_TEXTURES, 0x70, 4, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E680));
}

/*
Purpose: Dynamic initializer for g_OCEAN_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E6A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ocean_textures() {
    VectorCtorIterator(g_OCEAN_TEXTURES, 0x70, 2, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E6D0));
}

/*
Purpose: Dynamic initializer for g_FLAT_ARID_LAND_TEXTURE; registers its ??__F teardown.
Original Offset: 0044E6F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_flat_arid_land_texture() {
    TextureInitCtor(g_FLAT_ARID_LAND_TEXTURE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E710));
}

/*
Purpose: Dynamic initializer for g_MOIST_LAND_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E720
Return Value: n/a
Status: Complete
*/
void __cdecl construct_moist_land_textures() {
    VectorCtorIterator(g_MOIST_LAND_TEXTURES, 0x70, 16, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E750));
}

/*
Purpose: Dynamic initializer for g_RAINY_LAND_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E770
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rainy_land_textures() {
    VectorCtorIterator(g_RAINY_LAND_TEXTURES, 0x70, 16, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E7A0));
}

/*
Purpose: Dynamic initializer for g_JUNGLE_LAND_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E7C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_jungle_land_textures() {
    VectorCtorIterator(g_JUNGLE_LAND_TEXTURES, 0x70, 15, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E7F0));
}

/*
Purpose: Dynamic initializer for g_DUNE_LAND_TEXTURE; registers its ??__F teardown.
Original Offset: 0044E810
Return Value: n/a
Status: Complete
*/
void __cdecl construct_dune_land_texture() {
    TextureInitCtor(g_DUNE_LAND_TEXTURE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E830));
}

/*
Purpose: Dynamic initializer for g_SUNNY_MESA_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E840
Return Value: n/a
Status: Complete
*/
void __cdecl construct_sunny_mesa_textures() {
    VectorCtorIterator(g_SUNNY_MESA_TEXTURES, 0x70, 8, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E870));
}

/*
Purpose: Dynamic initializer for g_RAINFALL_SINGLE_TILE_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E890
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rainfall_single_tile_textures() {
    VectorCtorIterator(g_RAINFALL_SINGLE_TILE_TEXTURES, 0x70, 2, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E8C0));
}

/*
Purpose: Dynamic initializer for g_ROAD_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E8E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_road_textures() {
    VectorCtorIterator(g_ROAD_TEXTURES, 0x70, 9, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E910));
}

/*
Purpose: Dynamic initializer for g_MAGTUBE_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E930
Return Value: n/a
Status: Complete
*/
void __cdecl construct_magtube_textures() {
    VectorCtorIterator(g_MAGTUBE_TEXTURES, 0x70, 9, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E960));
}

/*
Purpose: Dynamic initializer for g_RIVER_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E980
Return Value: n/a
Status: Complete
*/
void __cdecl construct_river_textures() {
    VectorCtorIterator(g_RIVER_TEXTURES, 0x70, 16, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044E9B0));
}

/*
Purpose: Dynamic initializer for g_MOUNT_PLANET_TEXTURES; registers its ??__F teardown.
Original Offset: 0044E9D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_mount_planet_textures() {
    VectorCtorIterator(g_MOUNT_PLANET_TEXTURES, 0x70, 3, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EA00));
}

/*
Purpose: Dynamic initializer for g_GARLAND_CRATER_TEXTURES; registers its ??__F teardown.
Original Offset: 0044EA20
Return Value: n/a
Status: Complete
*/
void __cdecl construct_garland_crater_textures() {
    VectorCtorIterator(g_GARLAND_CRATER_TEXTURES, 0x70, 3, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EA50));
}

/*
Purpose: Dynamic initializer for g_FUNGUS_TEXTURES; registers its ??__F teardown.
Original Offset: 0044EA70
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fungus_textures() {
    VectorCtorIterator(g_FUNGUS_TEXTURES, 0x70, 30, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EAA0));
}

/*
Purpose: Dynamic initializer for g_FARM_TEXTURES; registers its ??__F teardown.
Original Offset: 0044EAC0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_farm_textures() {
    VectorCtorIterator(g_FARM_TEXTURES, 0x70, 9, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EAF0));
}

/*
Purpose: Dynamic initializer for g_FOREST_TEXTURES; registers its ??__F teardown.
Original Offset: 0044EB10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_forest_textures() {
    VectorCtorIterator(g_FOREST_TEXTURES, 0x70, 16, TextureElementCtor, TextureElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EB40));
}

/*
Purpose: Dynamic initializer for g_RAINFALL_SINGLE_TILE_TEXTURE; registers its ??__F teardown.
Original Offset: 0044EB60
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rainfall_single_tile_texture() {
    TextureInitCtor(g_RAINFALL_SINGLE_TILE_TEXTURE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EB80));
}

/*
Purpose: Dynamic initializer for g_TER1_WHITE_ORG_YEL_TILE_SPRITES; registers its ??__F teardown.
Original Offset: 0044EB90
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_white_org_yel_tile_sprites() {
    VectorCtorIterator(g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EBC0));
}

/*
Purpose: Dynamic initializer for g_TER1_BOTTOM_LEFT_TILE_SPRITES; registers its ??__F teardown.
Original Offset: 0044EBE0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_bottom_left_tile_sprites() {
    VectorCtorIterator(g_TER1_BOTTOM_LEFT_TILE_SPRITES, 0x2C, 9, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EC10));
}

/*
Purpose: Dynamic initializer for g_TER1_UNUSED_SPRITES2; registers its ??__F teardown.
Original Offset: 0044EC30
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_unused_sprites2() {
    VectorCtorIterator(g_TER1_UNUSED_SPRITES2, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EC60));
}

/*
Purpose: Dynamic initializer for g_TER1_MINE_SPRITE; registers its ??__F teardown.
Original Offset: 0044EC80
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_mine_sprite() {
    SpriteInitCtor(g_TER1_MINE_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044ECA0));
}

/*
Purpose: Dynamic initializer for g_TER1_SOLAR_COLLECTOR_SPRITE; registers its ??__F teardown.
Original Offset: 0044ECB0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_solar_collector_sprite() {
    SpriteInitCtor(g_TER1_SOLAR_COLLECTOR_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044ECD0));
}

/*
Purpose: Dynamic initializer for g_TER1_TIDAL_HARNESS_SPRITE; registers its ??__F teardown.
Original Offset: 0044ECE0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_tidal_harness_sprite() {
    SpriteInitCtor(g_TER1_TIDAL_HARNESS_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044ED00));
}

/*
Purpose: Dynamic initializer for g_TER1_MINING_PLATFORM_SPRITE; registers its ??__F teardown.
Original Offset: 0044ED10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_mining_platform_sprite() {
    SpriteInitCtor(g_TER1_MINING_PLATFORM_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044ED30));
}

/*
Purpose: Dynamic initializer for g_TER1_TUT_BLANK_SPRITE; registers its ??__F teardown.
Original Offset: 0044ED40
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_tut_blank_sprite() {
    SpriteInitCtor(g_TER1_TUT_BLANK_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044ED60));
}

/*
Purpose: Dynamic initializer for g_TER1_KELP_FARM_SPRITE; registers its ??__F teardown.
Original Offset: 0044ED70
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_kelp_farm_sprite() {
    SpriteInitCtor(g_TER1_KELP_FARM_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044ED90));
}

/*
Purpose: Dynamic initializer for g_TER1_CONDENSER_SPRITE; registers its ??__F teardown.
Original Offset: 0044EDA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_condenser_sprite() {
    SpriteInitCtor(g_TER1_CONDENSER_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EDC0));
}

/*
Purpose: Dynamic initializer for g_TER1_ECHELON_MIRROR_SPRITE; registers its ??__F teardown.
Original Offset: 0044EDD0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_echelon_mirror_sprite() {
    SpriteInitCtor(g_TER1_ECHELON_MIRROR_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EDF0));
}

/*
Purpose: Dynamic initializer for g_TER1_BOREHOLE_SPRITE; registers its ??__F teardown.
Original Offset: 0044EE00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_borehole_sprite() {
    SpriteInitCtor(g_TER1_BOREHOLE_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EE20));
}

/*
Purpose: Dynamic initializer for g_TER1_BOREHOLE_CLUSTER_SPRITE; registers its ??__F teardown.
Original Offset: 0044EE30
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_borehole_cluster_sprite() {
    SpriteInitCtor(g_TER1_BOREHOLE_CLUSTER_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EE50));
}

/*
Purpose: Dynamic initializer for g_TER1_MANIFOLD_NEXUS_SPRITES; registers its ??__F teardown.
Original Offset: 0044EE60
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_manifold_nexus_sprites() {
    VectorCtorIterator(g_TER1_MANIFOLD_NEXUS_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EE90));
}

/*
Purpose: Dynamic initializer for g_TER1WRECK_UNITY_WRECKAGE_SPRITES; registers its ??__F teardown.
Original Offset: 0044EEB0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1wreck_unity_wreckage_sprites() {
    VectorCtorIterator(g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 0x2C, 15, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EEE0));
}

/*
Purpose: Dynamic initializer for g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES; registers its ??__F teardown.
Original Offset: 0044EF00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1wreck_unity_wreckage_alt_sprites() {
    VectorCtorIterator(g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 0x2C, 4, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EF30));
}

/*
Purpose: Dynamic initializer for g_FOSSIL_FIELD_RIDGE_SPRITES; registers its ??__F teardown.
Original Offset: 0044EF50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fossil_field_ridge_sprites() {
    VectorCtorIterator(g_FOSSIL_FIELD_RIDGE_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EF80));
}

/*
Purpose: Dynamic initializer for g_TER1_UNUSED_SPRITES1; registers its ??__F teardown.
Original Offset: 0044EFA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_unused_sprites1() {
    VectorCtorIterator(g_TER1_UNUSED_SPRITES1, 0x2C, 5, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044EFD0));
}

/*
Purpose: Dynamic initializer for g_TER1_FARM_SPRITES; registers its ??__F teardown.
Original Offset: 0044EFF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_farm_sprites() {
    VectorCtorIterator(g_TER1_FARM_SPRITES, 0x2C, 5, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F020));
}

/*
Purpose: Dynamic initializer for g_TER1_SOIL_ENRICHER_SPRITES; registers its ??__F teardown.
Original Offset: 0044F040
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_soil_enricher_sprites() {
    VectorCtorIterator(g_TER1_SOIL_ENRICHER_SPRITES, 0x2C, 5, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F070));
}

/*
Purpose: Dynamic initializer for g_TER1_SEA_LAND_RESOURCE_SPRITES; registers its ??__F teardown.
Original Offset: 0044F090
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_sea_land_resource_sprites() {
    VectorCtorIterator(g_TER1_SEA_LAND_RESOURCE_SPRITES, 0x2C, 12, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F0C0));
}

/*
Purpose: Dynamic initializer for g_TER1_LANDMARK_RESOURCE_SPRITES; registers its ??__F teardown.
Original Offset: 0044F0E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_landmark_resource_sprites() {
    VectorCtorIterator(g_TER1_LANDMARK_RESOURCE_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F110));
}

/*
Purpose: Dynamic initializer for g_GLOW_SPRITES; registers its ??__F teardown.
Original Offset: 0044F130
Return Value: n/a
Status: Complete
*/
void __cdecl construct_glow_sprites() {
    VectorCtorIterator(g_GLOW_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F160));
}

/*
Purpose: Dynamic initializer for g_TER1_UNITY_POD_SPRITES; registers its ??__F teardown.
Original Offset: 0044F180
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_unity_pod_sprites() {
    VectorCtorIterator(g_TER1_UNITY_POD_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F1B0));
}

/*
Purpose: Dynamic initializer for g_TER1_MONOLITH_SPRITE; registers its ??__F teardown.
Original Offset: 0044F1D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_monolith_sprite() {
    SpriteInitCtor(g_TER1_MONOLITH_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F1F0));
}

/*
Purpose: Dynamic initializer for g_TER1_BUNKER_SPRITE; registers its ??__F teardown.
Original Offset: 0044F200
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_bunker_sprite() {
    SpriteInitCtor(g_TER1_BUNKER_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F220));
}

/*
Purpose: Dynamic initializer for g_TER1_AIRBASE_SPRITE; registers its ??__F teardown.
Original Offset: 0044F230
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_airbase_sprite() {
    SpriteInitCtor(g_TER1_AIRBASE_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F250));
}

/*
Purpose: Dynamic initializer for g_TER1_SENSOR_ARRAY_SPRITE; registers its ??__F teardown.
Original Offset: 0044F260
Return Value: n/a
Status: Complete
*/
void __cdecl construct_ter1_sensor_array_sprite() {
    SpriteInitCtor(g_TER1_SENSOR_ARRAY_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F280));
}

/*
Purpose: Dynamic initializer for g_RAINFALL_DOUBLE_TILE_SPRITES; registers its ??__F teardown.
Original Offset: 0044F290
Return Value: n/a
Status: Complete
*/
void __cdecl construct_rainfall_double_tile_sprites() {
    VectorCtorIterator(g_RAINFALL_DOUBLE_TILE_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F2C0));
}

/*
Purpose: Dynamic initializer for g_VEH_SPRITES; registers its ??__F teardown.
Original Offset: 0044F2E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_veh_sprites() {
    VectorCtorIterator(g_VEH_SPRITES, 0x2C, 152, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F310));
}

/*
Purpose: Dynamic initializer for g_FLAGS_VEH_SPRITES; registers its ??__F teardown.
Original Offset: 0044F330
Return Value: n/a
Status: Complete
*/
void __cdecl construct_flags_veh_sprites() {
    VectorCtorIterator(g_FLAGS_VEH_SPRITES, 0x2C, 112, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F360));
}

/*
Purpose: Dynamic initializer for g_ICONS_GENERAL_SPRITES; registers its ??__F teardown.
Original Offset: 0044F380
Return Value: n/a
Status: Complete
*/
void __cdecl construct_icons_general_sprites() {
    VectorCtorIterator(g_ICONS_GENERAL_SPRITES, 0x2C, 16, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F3B0));
}

/*
Purpose: Dynamic initializer for g_RESOURCE_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044F3D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_resource_icon_sprites() {
    VectorCtorIterator(g_RESOURCE_ICON_SPRITES, 0x2C, 32, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F400));
}

/*
Purpose: Dynamic initializer for g_CITIZEN_LG_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0044F420
Return Value: n/a
Status: Complete
*/
void __cdecl construct_citizen_lg_cursor_sprites() {
    VectorCtorIterator(g_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 8, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F450));
}

/*
Purpose: Dynamic initializer for g_SPECIALIST_LG_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0044F470
Return Value: n/a
Status: Complete
*/
void __cdecl construct_specialist_lg_cursor_sprites() {
    VectorCtorIterator(g_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F4A0));
}

/*
Purpose: Dynamic initializer for g_CITIZEN_SM_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0044F4C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_citizen_sm_cursor_sprites() {
    VectorCtorIterator(g_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 8, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F4F0));
}

/*
Purpose: Dynamic initializer for g_SPECIALIST_SM_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0044F510
Return Value: n/a
Status: Complete
*/
void __cdecl construct_specialist_sm_cursor_sprites() {
    VectorCtorIterator(g_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F540));
}

/*
Purpose: Dynamic initializer for g_AL_CITIZEN_LG_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0044F560
Return Value: n/a
Status: Complete
*/
void __cdecl construct_al_citizen_lg_cursor_sprites() {
    VectorCtorIterator(g_AL_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 4, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F590));
}

/*
Purpose: Dynamic initializer for g_AL_SPECIALIST_LG_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0044F5B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_al_specialist_lg_cursor_sprites() {
    VectorCtorIterator(g_AL_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F5E0));
}

/*
Purpose: Dynamic initializer for g_AL_CITIZEN_SM_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0044F600
Return Value: n/a
Status: Complete
*/
void __cdecl construct_al_citizen_sm_cursor_sprites() {
    VectorCtorIterator(g_AL_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 4, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F630));
}

/*
Purpose: Dynamic initializer for g_AL_SPECIALIST_SM_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0044F650
Return Value: n/a
Status: Complete
*/
void __cdecl construct_al_specialist_sm_cursor_sprites() {
    VectorCtorIterator(g_AL_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F680));
}

/*
Purpose: Dynamic initializer for g_RED_ALIEN_HEAD_ICON_SPRITE; registers its ??__F teardown.
Original Offset: 0044F6A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_red_alien_head_icon_sprite() {
    SpriteInitCtor(g_RED_ALIEN_HEAD_ICON_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F6C0));
}

/*
Purpose: Dynamic initializer for g_SILVER_MENU_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044F6D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_silver_menu_icon_sprites() {
    VectorCtorIterator(g_SILVER_MENU_ICON_SPRITES, 0x2C, 4, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F700));
}

/*
Purpose: Dynamic initializer for g_SILVER_CHECKBOX_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044F720
Return Value: n/a
Status: Complete
*/
void __cdecl construct_silver_checkbox_icon_sprites() {
    VectorCtorIterator(g_SILVER_CHECKBOX_ICON_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F750));
}

/*
Purpose: Dynamic initializer for g_RED_MALE_HEAD_ICON_SPRITE; registers its ??__F teardown.
Original Offset: 0044F770
Return Value: n/a
Status: Complete
*/
void __cdecl construct_red_male_head_icon_sprite() {
    SpriteInitCtor(g_RED_MALE_HEAD_ICON_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F790));
}

/*
Purpose: Dynamic initializer for g_NULL_RESOURCE_ICON_SPRITE; registers its ??__F teardown.
Original Offset: 0044F7A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_null_resource_icon_sprite() {
    SpriteInitCtor(g_NULL_RESOURCE_ICON_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F7C0));
}

/*
Purpose: Dynamic initializer for g_PEACE_SIGN_SPRITES; registers its ??__F teardown.
Original Offset: 0044F7D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_peace_sign_sprites() {
    VectorCtorIterator(g_PEACE_SIGN_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F800));
}

/*
Purpose: Dynamic initializer for g_ICON_TILE_SQUARE_SPRITE; registers its ??__F teardown.
Original Offset: 0044F820
Return Value: n/a
Status: Complete
*/
void __cdecl construct_icon_tile_square_sprite() {
    SpriteInitCtor(g_ICON_TILE_SQUARE_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F840));
}

/*
Purpose: Dynamic initializer for g_XI_BOOM_VEH_SPRITES; registers its ??__F teardown.
Original Offset: 0044F850
Return Value: n/a
Status: Complete
*/
void __cdecl construct_xi_boom_veh_sprites() {
    VectorCtorIterator(g_XI_BOOM_VEH_SPRITES, 0x2C, 144, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F880));
}

/*
Purpose: Dynamic initializer for g_XF_BOOM_VEH_SPRITES; registers its ??__F teardown.
Original Offset: 0044F8A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_xf_boom_veh_sprites() {
    VectorCtorIterator(g_XF_BOOM_VEH_SPRITES, 0x2C, 64, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F8D0));
}

/*
Purpose: Dynamic initializer for g_BATTLE_MIND_WORM_SPRITE; registers its ??__F teardown.
Original Offset: 0044F8F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_mind_worm_sprite() {
    SpriteInitCtor(g_BATTLE_MIND_WORM_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F910));
}

/*
Purpose: Dynamic initializer for g_BATTLE_ISLE_DEEP_SPRITE; registers its ??__F teardown.
Original Offset: 0044F920
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_isle_deep_sprite() {
    SpriteInitCtor(g_BATTLE_ISLE_DEEP_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F940));
}

/*
Purpose: Dynamic initializer for g_BATTLE_LOCUSTS_CHIRON_SPRITE; registers its ??__F teardown.
Original Offset: 0044F950
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_locusts_chiron_sprite() {
    SpriteInitCtor(g_BATTLE_LOCUSTS_CHIRON_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F970));
}

/*
Purpose: Dynamic initializer for g_BATTLE_FUNGAL_TOWER_SPRITE; registers its ??__F teardown.
Original Offset: 0044F980
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_fungal_tower_sprite() {
    SpriteInitCtor(g_BATTLE_FUNGAL_TOWER_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F9A0));
}

/*
Purpose: Dynamic initializer for g_BATTLE_SPORE_LAUNCHER_SPRITE; registers its ??__F teardown.
Original Offset: 0044F9B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_spore_launcher_sprite() {
    SpriteInitCtor(g_BATTLE_SPORE_LAUNCHER_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044F9D0));
}

/*
Purpose: Dynamic initializer for g_BATTLE_SEALURK_SPRITE; registers its ??__F teardown.
Original Offset: 0044F9E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_battle_sealurk_sprite() {
    SpriteInitCtor(g_BATTLE_SEALURK_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FA00));
}

/*
Purpose: Dynamic initializer for g_TECH_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044FA10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_tech_icon_sprites() {
    VectorCtorIterator(g_TECH_ICON_SPRITES, 0x2C, 89, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FA40));
}

/*
Purpose: Dynamic initializer for g_FACILITY_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044FA60
Return Value: n/a
Status: Complete
*/
void __cdecl construct_facility_icon_sprites() {
    VectorCtorIterator(g_FACILITY_ICON_SPRITES, 0x2C, 70, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FA90));
}

/*
Purpose: Dynamic initializer for g_SECRET_PROJECT_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044FAB0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_secret_project_icon_sprites() {
    VectorCtorIterator(g_SECRET_PROJECT_ICON_SPRITES, 0x2C, 64, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FAE0));
}

/*
Purpose: Dynamic initializer for g_IFACE_MP_COMBO_ARROW_SPRITES; registers its ??__F teardown.
Original Offset: 0044FB00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_mp_combo_arrow_sprites() {
    VectorCtorIterator(g_IFACE_MP_COMBO_ARROW_SPRITES, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FB30));
}

/*
Purpose: Dynamic initializer for g_SCROLL_BAR_ARROW_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044FB50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scroll_bar_arrow_icon_sprites() {
    VectorCtorIterator(g_SCROLL_BAR_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FB80));
}

/*
Purpose: Dynamic initializer for g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044FBA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scroll_bar_small_arrow_icon_sprites() {
    VectorCtorIterator(g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FBD0));
}

/*
Purpose: Dynamic initializer for g_SCROLL_BAR_FILLER_ICON_SPRITES; registers its ??__F teardown.
Original Offset: 0044FBF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scroll_bar_filler_icon_sprites() {
    SpriteInitCtor(g_SCROLL_BAR_FILLER_ICON_SPRITES);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FC10));
}

/*
Purpose: Dynamic initializer for g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE; registers its ??__F teardown.
Original Offset: 0044FC20
Return Value: n/a
Status: Complete
*/
void __cdecl construct_scroll_bar_small_filler_icon_sprite() {
    SpriteInitCtor(g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FC40));
}

/*
Purpose: Dynamic initializer for g_IFACE_LOCK_SPRITES; registers its ??__F teardown.
Original Offset: 0044FC50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_lock_sprites() {
    VectorCtorIterator(g_IFACE_LOCK_SPRITES, 0x2C, 2, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FC80));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR01; registers its ??__F teardown.
Original Offset: 0044FCA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var01() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR01, 0x2C, 8, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FCD0));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE; registers its ??__F teardown.
Original Offset: 0044FCF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_top_left_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FD10));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE; registers its ??__F teardown.
Original Offset: 0044FD20
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_top_right_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FD40));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE; registers its ??__F teardown.
Original Offset: 0044FD50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_bot_left_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FD70));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE; registers its ??__F teardown.
Original Offset: 0044FD80
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_bot_right_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FDA0));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE; registers its ??__F teardown.
Original Offset: 0044FDB0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_mid_left_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FDD0));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE; registers its ??__F teardown.
Original Offset: 0044FDE0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_mid_right_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FE00));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE; registers its ??__F teardown.
Original Offset: 0044FE10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_top_mid_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FE30));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE; registers its ??__F teardown.
Original Offset: 0044FE40
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_bot_mid_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FE60));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE; registers its ??__F teardown.
Original Offset: 0044FE70
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_noncap_mid_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FE90));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE; registers its ??__F teardown.
Original Offset: 0044FEA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_noncap_left_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FEC0));
}

/*
Purpose: Dynamic initializer for g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE; registers its ??__F teardown.
Original Offset: 0044FED0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_general_windows_noncap_right_sprite() {
    SpriteInitCtor(g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FEF0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR23; registers its ??__F teardown.
Original Offset: 0044FF00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var23() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR23);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FF20));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR24; registers its ??__F teardown.
Original Offset: 0044FF30
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var24() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR24);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FF50));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR25; registers its ??__F teardown.
Original Offset: 0044FF60
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var25() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR25);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FF80));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR26; registers its ??__F teardown.
Original Offset: 0044FF90
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var26() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR26);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FFB0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR27; registers its ??__F teardown.
Original Offset: 0044FFC0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var27() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR27);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0044FFE0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR28; registers its ??__F teardown.
Original Offset: 0044FFF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var28() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR28);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450010));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR29; registers its ??__F teardown.
Original Offset: 00450020
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var29() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR29);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450040));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR30; registers its ??__F teardown.
Original Offset: 00450050
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var30() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR30);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450070));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR31; registers its ??__F teardown.
Original Offset: 00450080
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var31() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR31);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004500A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR32; registers its ??__F teardown.
Original Offset: 004500B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var32() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR32);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004500D0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR33; registers its ??__F teardown.
Original Offset: 004500E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var33() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR33);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450100));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR34; registers its ??__F teardown.
Original Offset: 00450110
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var34() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR34);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450130));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR35; registers its ??__F teardown.
Original Offset: 00450140
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var35() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR35);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450160));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR36; registers its ??__F teardown.
Original Offset: 00450170
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var36() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR36);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450190));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR37; registers its ??__F teardown.
Original Offset: 004501A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var37() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR37);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004501C0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR38; registers its ??__F teardown.
Original Offset: 004501D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var38() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR38);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004501F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR39; registers its ??__F teardown.
Original Offset: 00450200
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var39() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR39);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450220));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR40; registers its ??__F teardown.
Original Offset: 00450230
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var40() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR40);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450250));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR41; registers its ??__F teardown.
Original Offset: 00450260
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var41() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR41);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450280));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR42; registers its ??__F teardown.
Original Offset: 00450290
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var42() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR42);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004502B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR43; registers its ??__F teardown.
Original Offset: 004502C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var43() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR43);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004502E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR44; registers its ??__F teardown.
Original Offset: 004502F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var44() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR44);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450310));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR45; registers its ??__F teardown.
Original Offset: 00450320
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var45() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR45);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450340));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR46; registers its ??__F teardown.
Original Offset: 00450350
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var46() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR46);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450370));
}

/*
Purpose: Dynamic initializer for g_IFACE_TECH_TREE_ARROW_SPRITES; registers its ??__F teardown.
Original Offset: 00450380
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_tech_tree_arrow_sprites() {
    VectorCtorIterator(g_IFACE_TECH_TREE_ARROW_SPRITES, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004503B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR47; registers its ??__F teardown.
Original Offset: 004503D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var47() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR47);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004503F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR48; registers its ??__F teardown.
Original Offset: 00450400
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var48() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR48);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450420));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR49; registers its ??__F teardown.
Original Offset: 00450430
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var49() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR49);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450450));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR50; registers its ??__F teardown.
Original Offset: 00450460
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var50() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR50);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450480));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR51; registers its ??__F teardown.
Original Offset: 00450490
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var51() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR51);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004504B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR52; registers its ??__F teardown.
Original Offset: 004504C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var52() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR52);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004504E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR53; registers its ??__F teardown.
Original Offset: 004504F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var53() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR53);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450510));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR54; registers its ??__F teardown.
Original Offset: 00450520
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var54() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR54);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450540));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR55; registers its ??__F teardown.
Original Offset: 00450550
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var55() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR55);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450570));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR56; registers its ??__F teardown.
Original Offset: 00450580
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var56() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR56);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004505A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR57; registers its ??__F teardown.
Original Offset: 004505B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var57() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR57);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004505D0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR58; registers its ??__F teardown.
Original Offset: 004505E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var58() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR58);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450600));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR59; registers its ??__F teardown.
Original Offset: 00450610
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var59() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR59);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450630));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR60; registers its ??__F teardown.
Original Offset: 00450640
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var60() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR60);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450660));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR61; registers its ??__F teardown.
Original Offset: 00450670
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var61() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR61);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450690));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR62; registers its ??__F teardown.
Original Offset: 004506A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var62() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR62);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004506C0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR63; registers its ??__F teardown.
Original Offset: 004506D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var63() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR63);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004506F0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR64; registers its ??__F teardown.
Original Offset: 00450700
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var64() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR64);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450720));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR65; registers its ??__F teardown.
Original Offset: 00450730
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var65() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR65);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450750));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR66; registers its ??__F teardown.
Original Offset: 00450760
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var66() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR66);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450780));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR67; registers its ??__F teardown.
Original Offset: 00450790
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var67() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR67);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004507B0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR68; registers its ??__F teardown.
Original Offset: 004507C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var68() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR68);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004507E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR69; registers its ??__F teardown.
Original Offset: 004507F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var69() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR69);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450810));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR70; registers its ??__F teardown.
Original Offset: 00450820
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var70() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR70);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450840));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR71; registers its ??__F teardown.
Original Offset: 00450850
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var71() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR71);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450870));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR72; registers its ??__F teardown.
Original Offset: 00450880
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var72() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR72);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004508A0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR73; registers its ??__F teardown.
Original Offset: 004508B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var73() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR73);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004508D0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR74; registers its ??__F teardown.
Original Offset: 004508E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var74() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR74);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450900));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR02; registers its ??__F teardown.
Original Offset: 00450910
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var02() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR02, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450940));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR03; registers its ??__F teardown.
Original Offset: 00450960
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var03() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR03, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450990));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR04; registers its ??__F teardown.
Original Offset: 004509B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var04() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR04, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004509E0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR05; registers its ??__F teardown.
Original Offset: 00450A00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var05() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR05, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450A30));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR06; registers its ??__F teardown.
Original Offset: 00450A50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var06() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR06, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450A80));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR07; registers its ??__F teardown.
Original Offset: 00450AA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var07() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR07, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450AD0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR08; registers its ??__F teardown.
Original Offset: 00450AF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var08() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR08, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450B20));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR09; registers its ??__F teardown.
Original Offset: 00450B40
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var09() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR09, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450B70));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITES_VAR10; registers its ??__F teardown.
Original Offset: 00450B90
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprites_var10() {
    VectorCtorIterator(g_UNUSED_SPRITES_VAR10, 0x2C, 3, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450BC0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR75; registers its ??__F teardown.
Original Offset: 00450BE0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var75() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR75);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450C00));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR76; registers its ??__F teardown.
Original Offset: 00450C10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var76() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR76);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450C30));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR77; registers its ??__F teardown.
Original Offset: 00450C40
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var77() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR77);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450C60));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR78; registers its ??__F teardown.
Original Offset: 00450C70
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var78() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR78);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450C90));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR79; registers its ??__F teardown.
Original Offset: 00450CA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var79() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR79);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450CC0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR80; registers its ??__F teardown.
Original Offset: 00450CD0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var80() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR80);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450CF0));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR81; registers its ??__F teardown.
Original Offset: 00450D00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var81() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR81);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450D20));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR82; registers its ??__F teardown.
Original Offset: 00450D30
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var82() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR82);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450D50));
}

/*
Purpose: Dynamic initializer for g_UNUSED_SPRITE_VAR83; registers its ??__F teardown.
Original Offset: 00450D60
Return Value: n/a
Status: Complete
*/
void __cdecl construct_unused_sprite_var83() {
    SpriteInitCtor(g_UNUSED_SPRITE_VAR83);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450D80));
}

/*
Purpose: Dynamic initializer for g_BASEWIN_SPRITES; registers its ??__F teardown.
Original Offset: 00450D90
Return Value: n/a
Status: Complete
*/
void __cdecl construct_basewin_sprites() {
    VectorCtorIterator(g_BASEWIN_SPRITES, 0x2C, 27, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00450DC0));
}

/*
Purpose: Dynamic initializer for g_INFOWIN; registers its ??__F teardown.
Original Offset: 004562C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_infowin() {
    InfoWinCtorTarget(reinterpret_cast<void *>(0x007AD2A0));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004562E0));
}

/*
Purpose: Dynamic initializer for g_MAININTERFACE; registers its ??__F teardown.
Original Offset: 00459560
Return Value: n/a
Status: Complete
*/
void __cdecl construct_maininterface() {
    MainInterfaceCtorTarget(reinterpret_cast<void *>(0x007AE820));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00459580));
}

/*
Purpose: Dynamic initializer for g_MAININTERFACE_WAVE; registers its ??__F teardown.
Original Offset: 00459590
Return Value: n/a
Status: Complete
*/
void __cdecl construct_maininterface_wave() {
    WaveInitCtor(g_MAININTERFACE_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004595B0));
}

/*
Purpose: Dynamic initializer for g_JACKAL_FONT; registers its ??__F teardown.
Original Offset: 0045F920
Return Value: n/a
Status: Complete
*/
void __cdecl construct_jackal_font() {
    FontInitCtor(g_JACKAL_FONT);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0045F940));
}

/*
Purpose: Dynamic initializer for g_MAPWIN; registers its ??__F teardown.
Original Offset: 00462080
Return Value: n/a
Status: Complete
*/
void __cdecl construct_mapwin() {
    MapWinCtorTarget(reinterpret_cast<void *>(0x007D4060), 1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004620A0));
}

/*
Purpose: Dynamic initializer for g_MESSAGEWIN; registers its ??__F teardown.
Original Offset: 00471320
Return Value: n/a
Status: Complete
*/
void __cdecl construct_messagewin() {
    MessageWinCtorTarget(reinterpret_cast<void *>(0x007F67F8));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00471340));
}

/*
Purpose: Dynamic initializer for g_IFACE_GREEN_RIGHT_ARROW_SPRITE; registers its ??__F teardown.
Original Offset: 00471350
Return Value: n/a
Status: Complete
*/
void __cdecl construct_iface_green_right_arrow_sprite() {
    VectorCtorIterator(g_IFACE_GREEN_RIGHT_ARROW_SPRITE, 0x2C, 1, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00471380));
}

/*
Purpose: Dynamic initializer for g_MONUWIN; registers its ??__F teardown.
Original Offset: 00472200
Return Value: n/a
Status: Complete
*/
void __cdecl construct_monuwin() {
    MonuWinCtorTarget(reinterpret_cast<void *>(0x007F9F58));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00472220));
}

/*
Purpose: Dynamic initializer for g_MULTIWIN; registers its ??__F teardown.
Original Offset: 00477E10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_multiwin() {
    MultiWinCtorTarget(reinterpret_cast<void *>(0x007FD648));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00477E30));
}

/*
Purpose: Dynamic initializer for g_MULTIWIN_WAVE; registers its ??__F teardown.
Original Offset: 00477E40
Return Value: n/a
Status: Complete
*/
void __cdecl construct_multiwin_wave() {
    WaveInitCtor(g_MULTIWIN_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00477E60));
}

/*
Purpose: Dynamic initializer for g_NETMSG1; registers its ??__F teardown.
Original Offset: 0047A770
Return Value: n/a
Status: Complete
*/
void __cdecl construct_netmsg1() {
    NetMsgCtorTarget(reinterpret_cast<void *>(0x00805338), 4096, 8, 0);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0047A790));
}

/*
Purpose: Dynamic initializer for g_NETMSG2; registers its ??__F teardown.
Original Offset: 0047A7A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_netmsg2() {
    NetMsgCtorTarget(reinterpret_cast<void *>(0x007FFF80), -5, 40, 1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0047A7C0));
}

/*
Purpose: Dynamic initializer for g_NETWIN; registers its ??__F teardown.
Original Offset: 0047AFF0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_netwin() {
    NetWinCtorTarget(reinterpret_cast<void *>(0x0080A6F8));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0047B010));
}

/*
Purpose: Dynamic initializer for g_NEWTECHWIN; registers its ??__F teardown.
Original Offset: 00483850
Return Value: n/a
Status: Complete
*/
void __cdecl construct_newtechwin() {
    NewTechWinCtorTarget(reinterpret_cast<void *>(0x00811E40));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00483870));
}

/*
Purpose: Dynamic initializer for g_NEWTECHWIN_SPRITES; registers its ??__F teardown.
Original Offset: 00483880
Return Value: n/a
Status: Complete
*/
void __cdecl construct_newtechwin_sprites() {
    VectorCtorIterator(g_NEWTECHWIN_SPRITES, 0x2C, 6, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004838B0));
}

/*
Purpose: Dynamic initializer for g_PICKWIN; registers its ??__F teardown.
Original Offset: 00488750
Return Value: n/a
Status: Complete
*/
void __cdecl construct_pickwin() {
    PickWinCtorTarget(reinterpret_cast<void *>(0x00822718));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00488770));
}

/*
Purpose: Dynamic initializer for g_PLANWIN; registers its ??__F teardown.
Original Offset: 0048AE00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_planwin() {
    PlanWinCtorTarget(reinterpret_cast<void *>(0x00834D70), 1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0048AE20));
}

/*
Purpose: Dynamic initializer for g_PREFWIN; registers its ??__F teardown.
Original Offset: 0048D4F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_prefwin() {
    PrefWinCtorTarget(reinterpret_cast<void *>(0x008578D8));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0048D510));
}

/*
Purpose: Dynamic initializer for g_PREFWIN_BUTTONGROUP; registers its ??__F teardown.
Original Offset: 0048D520
Return Value: n/a
Status: Complete
*/
void __cdecl construct_prefwin_buttongroup() {
    ButtonGroupInitCtor(g_PREFWIN_BUTTONGROUP);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0048D540));
}

/*
Purpose: Dynamic initializer for g_QUAYLEWIN; registers its ??__F teardown.
Original Offset: 00495170
Return Value: n/a
Status: Complete
*/
void __cdecl construct_quaylewin() {
    QuayleWinCtorTarget(reinterpret_cast<void *>(0x00872CB0));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00495190));
}

/*
Purpose: Dynamic initializer for g_REPORTIF; registers its ??__F teardown.
Original Offset: 00496900
Return Value: n/a
Status: Complete
*/
void __cdecl construct_reportif() {
    ReportIfCtorTarget(reinterpret_cast<void *>(0x00885F38));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00496920));
}

/*
Purpose: Dynamic initializer for g_REPORTWIN; registers its ??__F teardown.
Original Offset: 00496930
Return Value: n/a
Status: Complete
*/
void __cdecl construct_reportwin() {
    ReportWinCtorTarget(reinterpret_cast<void *>(0x00876478));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00496950));
}

/*
Purpose: Dynamic initializer for SocialWinParent; registers its ??__F teardown.
Original Offset: 004AE9B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_socialwinparent() {
    SocialWinCtorTarget(reinterpret_cast<void *>(0x008A6270));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004AE9D0));
}

/*
Purpose: Dynamic initializer for g_STATUSWIN; registers its ??__F teardown.
Original Offset: 004B3FA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_statuswin() {
    StatusWinCtorTarget(reinterpret_cast<void *>(0x008C5568));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004B3FC0));
}

/*
Purpose: Dynamic initializer for g_TUTWIN; registers its ??__F teardown.
Original Offset: 004BA5C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_tutwin() {
    TutWinCtorTarget(reinterpret_cast<void *>(0x008C6E68));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004BA5E0));
}

/*
Purpose: Dynamic initializer for g_VEHDRAW_CAVIAR; registers its ??__F teardown.
Original Offset: 004BF6E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vehdraw_caviar() {
    CaviarInitCtor(g_VEHDRAW_CAVIAR);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004BF700));
}

/*
Purpose: Dynamic initializer for g_VEHDRAW_BUFFER; registers its ??__F teardown.
Original Offset: 004BF710
Return Value: n/a
Status: Complete
*/
void __cdecl construct_vehdraw_buffer() {
    BufferInitCtor(g_VEHDRAW_BUFFER);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004BF730));
}

/*
Purpose: Dynamic initializer for g_WORLDWIN; registers its ??__F teardown.
Original Offset: 004C38B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_worldwin() {
    WorldWinCtorTarget(reinterpret_cast<void *>(0x008E9F60));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004C38D0));
}

/*
Purpose: Dynamic initializer for g_WAVE_DEVICE; registers its ??__F teardown.
Original Offset: 004C5C50
Return Value: n/a
Status: Complete
*/
void __cdecl construct_wave_device() {
    WaveDeviceInitCtor(g_WAVE_DEVICE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004C5C70));
}

/*
Purpose: Dynamic initializer for g_MIDI_DEVICE; registers its ??__F teardown.
Original Offset: 004C5C80
Return Value: n/a
Status: Complete
*/
void __cdecl construct_midi_device() {
    Midi_DeviceCtorTarget(reinterpret_cast<void *>(0x0090D950));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004C5CA0));
}

/*
Purpose: Dynamic initializer for g_WAVE_IN_DEVICE; registers its ??__F teardown.
Original Offset: 004C5CB0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_wave_in_device() {
    Wave_In_DeviceCtorTarget(reinterpret_cast<void *>(0x0090DB50));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x004C5CD0));
}

/*
Purpose: Dynamic initializer for g_CONSOLE; registers its ??__F teardown.
Original Offset: 0050E850
Return Value: n/a
Status: Complete
*/
void __cdecl construct_console() {
    ConsoleCtorTarget(reinterpret_cast<void *>(0x009156B0), 1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0050E870));
}

/*
Purpose: Dynamic initializer for g_CONSOLE_TIMER; registers its ??__F teardown.
Original Offset: 0050E980
Return Value: n/a
Status: Complete
*/
void __cdecl construct_console_timer() {
    TimeInitCtor(g_CONSOLE_TIMER);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0050E9A0));
}

/*
Purpose: Dynamic initializer for g_CURSOR_SPRITES; registers its ??__F teardown.
Original Offset: 0052DAC0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_cursor_sprites() {
    VectorCtorIterator(g_CURSOR_SPRITES, 0x2C, 12, SpriteElementCtor, SpriteElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0052DAF0));
}

/*
Purpose: Dynamic initializer for g_NETDAEMON; registers its ??__F teardown.
Original Offset: 0052DB10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_netdaemon() {
    NetDaemonCtorTarget(reinterpret_cast<void *>(0x0093CD90));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0052DB30));
}

/*
Purpose: Dynamic initializer for g_FONTQUEUE_VAL2; registers its ??__F teardown.
Original Offset: 00559230
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fontqueue_val2() {
    FontQueueCtorTarget(reinterpret_cast<void *>(0x0093FB88));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00559250));
}

/*
Purpose: Dynamic initializer for g_FONTQUEUE_VAL1; registers its ??__F teardown.
Original Offset: 00559260
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fontqueue_val1() {
    FontQueueCtorTarget(reinterpret_cast<void *>(0x0093FAE8));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00559280));
}

/*
Purpose: Dynamic initializer for g_TOP_MENU_WAVE; registers its ??__F teardown.
Original Offset: 00584D00
Return Value: n/a
Status: Complete
*/
void __cdecl construct_top_menu_wave() {
    WaveInitCtor(g_TOP_MENU_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00584D20));
}

/*
Purpose: Dynamic initializer for g_FONTS; registers its ??__F teardown.
Original Offset: 005882A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_fonts() {
    VectorCtorIterator(g_FONTS, 0x28, 48, FontElementCtor, FontElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005882D0));
}

/*
Purpose: Dynamic initializer for g_CRASH_LANDING_WAVE; registers its ??__F teardown.
Original Offset: 005AE0F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_crash_landing_wave() {
    WaveInitCtor(g_CRASH_LANDING_WAVE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005AE110));
}

/*
Purpose: Dynamic initializer for g_WAVE_GENERAL; registers its ??__F teardown.
Original Offset: 005B9C10
Return Value: n/a
Status: Complete
*/
void __cdecl construct_wave_general() {
    WaveInitCtor(g_WAVE_GENERAL);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005B9C30));
}

/*
Purpose: Dynamic initializer for g_PALETTE1; registers its ??__F teardown.
Original Offset: 005BEC40
Return Value: n/a
Status: Complete
*/
void __cdecl construct_palette1() {
    PaletteCtorTarget(reinterpret_cast<void *>(0x0094C590));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005BEC60));
}

/*
Purpose: Dynamic initializer for g_PALETTE2; registers its ??__F teardown.
Original Offset: 005BEC70
Return Value: n/a
Status: Complete
*/
void __cdecl construct_palette2() {
    PaletteCtorTarget(reinterpret_cast<void *>(0x009523A0));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005BEC90));
}

/*
Purpose: Dynamic initializer for g_MULTIDEBUG; registers its ??__F teardown.
Original Offset: 005C97C0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_multidebug() {
    MultiDebugCtorTarget(reinterpret_cast<void *>(0x009B22F0));
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005C97E0));
}

/*
Purpose: Dynamic initializer for g_BUFFER_SPRITE; registers its ??__F teardown.
Original Offset: 005D71D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_buffer_sprite() {
    SpriteInitCtor(g_BUFFER_SPRITE);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005D71F0));
}

/*
Purpose: Dynamic initializer for g_BUFFER; registers its ??__F teardown.
Original Offset: 005E37B0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_buffer() {
    BufferInitCtor(g_BUFFER);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005E37D0));
}

/*
Purpose: Dynamic initializer for g_WIN_BUFFER; registers its ??__F teardown.
Original Offset: 005EB350
Return Value: n/a
Status: Complete
*/
void __cdecl construct_win_buffer() {
    BufferInitCtor(g_WIN_BUFFER);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005EB370));
}

/*
Purpose: Dynamic initializer for TxtIndex; registers its ??__F teardown.
Original Offset: 005FD4E0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_txtindex() {
    VectorCtorIterator(TxtIndexGlobal, 0x118, 4, TextIndexElementCtor, TextIndexElementTeardown);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x005FD510));
}

/*
Purpose: Dynamic initializer for g_RADIOBUTTON_SPRITE_1; registers its ??__F teardown.
Original Offset: 0060D060
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radiobutton_sprite_1() {
    SpriteInitCtor(g_RADIOBUTTON_SPRITE_1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0060D080));
}

/*
Purpose: Dynamic initializer for g_RADIOBUTTON_SPRITE_2; registers its ??__F teardown.
Original Offset: 0060D0A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_radiobutton_sprite_2() {
    SpriteInitCtor(g_RADIOBUTTON_SPRITE_2);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0060D0C0));
}

/*
Purpose: Dynamic initializer for g_CHECKBOX_SPRITE_1; registers its ??__F teardown.
Original Offset: 0060E5F0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_checkbox_sprite_1() {
    SpriteInitCtor(g_CHECKBOX_SPRITE_1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0060E610));
}

/*
Purpose: Dynamic initializer for g_CHECKBOX_SPRITE_2; registers its ??__F teardown.
Original Offset: 0060E630
Return Value: n/a
Status: Complete
*/
void __cdecl construct_checkbox_sprite_2() {
    SpriteInitCtor(g_CHECKBOX_SPRITE_2);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x0060E650));
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_1; registers its ??__F teardown.
Original Offset: 00613790
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_1() {
    SpriteInitCtor(g_FILEWIN_SPRITE_1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x006137B0));
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_2; registers its ??__F teardown.
Original Offset: 006137D0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_2() {
    SpriteInitCtor(g_FILEWIN_SPRITE_2);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x006137F0));
}

/*
Purpose: Dynamic initializer for g_FILEWIN_SPRITE_3; registers its ??__F teardown.
Original Offset: 00613810
Return Value: n/a
Status: Complete
*/
void __cdecl construct_filewin_sprite_3() {
    SpriteInitCtor(g_FILEWIN_SPRITE_3);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00613830));
}

/*
Purpose: Dynamic initializer for StringTable; registers its ??__F teardown.
Original Offset: 006168A0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_stringtable() {
    StringsInitCtor(StringTable);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x006168C0));
}

/*
Purpose: Dynamic initializer for g_CAVIAR_BUFFER_1; registers its ??__F teardown.
Original Offset: 00616AA0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_caviar_buffer_1() {
    BufferInitCtor(g_CAVIAR_BUFFER_1);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00616AC0));
}

/*
Purpose: Dynamic initializer for g_CAVIAR_BUFFER_2; registers its ??__F teardown.
Original Offset: 00616AE0
Return Value: n/a
Status: Complete
*/
void __cdecl construct_caviar_buffer_2() {
    BufferInitCtor(g_CAVIAR_BUFFER_2);
    GameAtexit(reinterpret_cast<func_atexit_callback *>(0x00616B00));
}
