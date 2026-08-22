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
#include "adjustor_thunks.h"
#include "checkbox.h"
#include "console.h"
#include "dialogs.h"
#include "editgroup.h"
#include "listbox.h"
#include "mapwin.h"
#include "planwin.h"
#include "radiobutton.h"
#include "spritebox.h"

/*
 * WHY 106 OF THESE SIT TWO OR THREE INSTRUCTIONS SHORT, and what would
 * actually close it.
 *
 * The image's thunk is three instructions - `sub ecx, [ecx-4]`,
 * `sub ecx, <fixed>`, then a TAIL JUMP to the target. Ours is seven: the
 * vtordisp load does not fold into the `sub`, and VC6 emits `push arg; call;
 * ret 4` where the image jumps and lets the target reuse the caller's pushed
 * argument.
 *
 * MEASURED, so this is not a guess. The target is ALREADY a named method -
 * `reinterpret_cast<MapWin *>(...)->MapWin::on_button_clicked(arg0)` - so
 * "call through a seam blocks the tail call" is not the explanation, and
 * declaring the destructor for real and forwarding it changes nothing at all
 * (0 of 2 either way on `??1thunk1_RadioButton`). VC6 1998 does not do
 * sibling-call optimisation for a `__fastcall` free function calling a
 * `__thiscall` member with a stack argument, and no spelling asks it to.
 *
 * THE PATH THROUGH IS NOT TO WRITE THESE AT ALL. MSVC GENERATES adjustor
 * thunks itself, byte-exact by construction, when a class really declares
 * virtual inheritance and overrides through it. These are hand-written because
 * the window hierarchy models its vtable as an opaque dword the original
 * installs by hand, so no class here declares a virtual - see the long note in
 * `radiobutton.h`, which measures what that costs and states that giving
 * GraphicWin and Dialog real virtual destructors is a coupled edit rather than
 * an impossible one.
 *
 * So: do not grind these individually. The whole family falls out of that one
 * structural change, or it does not fall out at all.
 */

/*
Verification note: the generated leaf suite proves that each body reads its
vtordisp from the displacement recorded for it, subtracts the adjustment
recorded for it, and forwards its arguments and its result untouched. It does
NOT independently confirm that `this - vtordisp - adjust` is what the original
computed, because the suite's expectation is derived from the same table the
bodies are. What pins the model is the acceptance check in
tools/generate_adjustor_thunks.py: a thunk is emitted only when its bytes are
exactly `sub ecx, [ecx - D]` / optional `sub ecx, A` / `jmp target`, and only
when the target's own callee-pop count agrees with the emitted parameter list.
A three-instruction body matched instruction for instruction leaves nothing
between the bytes and the transcription for a leaf fixture to catch.

The check that would close the remaining gap is a differential oracle running
each original thunk against its recovered twin with the target patched to
record the receiver. That needs the hybrid process, not the leaf suite, and it
is not built here.
*/

// Each seam defaults to the original method the thunk tail
// jumps to; the hybrid redirects it to the recovered body at
// run time, and the leaf suite rebinds it to a recorder.
func_adjustor_p_u ConsoleScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x0051DDF0);
func_adjustor_p_u DialogsScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x00407100);
func_adjustor_p_u MapWinScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x00421830);
func_adjustor_p_u PlanWinScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x0048BF20);
// TRIED for the whole vtordisp family, and the blocker is NOT this seam.
// The image's thunk is two instructions - `sub ecx, [ecx-4]` then a TAIL JUMP
// to 0x00406F60 - and VC6 does generate the folded `sub ecx, [ecx-4]` from
// `object - *(int32_t *)(object - 4)`. What it will not do is reuse the
// caller's pushed argument across the jump: ours emits `push [esp+4]` first,
// four instructions against two.
//
// That argument is MSVC's hidden vbase-destructor flag - 0x00406F60 ends
// `ret 4` - and it only appears when the class really declares virtual
// inheritance. `radiobutton.h` explains why it cannot yet: GraphicWin and
// Dialog model their vtable as an opaque dword the original installs by hand,
// so neither declares a virtual, and giving them real virtual destructors is a
// coupled edit. So this family is capped there, not here.
//
// MEASURED: declaring `~RadioButton()` for real and forwarding it to
// 0x00406F60 changes NOTHING - 0 of 2 either way, four compiled instructions
// either way. The seam is not what costs the match.
func_adjustor_v_i RadioButtonDtorTarget =
    original_method<func_adjustor_v_i>(0x00406F60);

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::dtor expects, then forward unchanged.
// ORIGINAL: 0x00406F50 ??1thunk1_RadioButton@@QAE@H@Z 0x00406F50-0x00406F58
// symbol    ?adjust_radio_button1_dtor@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ??1thunk1_RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_dtor(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    (ORIGINAL(object - vtordisp)->*RadioButtonDtorTarget)(arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00406FF0 ?on_redraw@thunk1_RadioButton@@QAEXXZ 0x00406FF0-0x00406FF8 BYTE_EXACT
// symbol    ?adjust_radio_button1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_RadioButton@@QAEXXZ)(RadioButton* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<RadioButton *>(object - vtordisp)->RadioButton::on_redraw();
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x00407000 ?on_mouse_move@thunk1_RadioButton@@QAEXHH@Z 0x00407000-0x00407008
// symbol    ?adjust_radio_button1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_mouse_move(void *self, void *,
                                                   int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<RadioButton *>(object - vtordisp)->RadioButton::on_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x00407010 ?on_mouse_leave@thunk1_RadioButton@@QAEXHH@Z 0x00407010-0x00407018
// symbol    ?adjust_radio_button1_on_mouse_leave@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_leave@thunk1_RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_mouse_leave(void *self, void *,
                                                    int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<RadioButton *>(object - vtordisp)->RadioButton::on_mouse_leave(arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x00407020 ?on_left_down@thunk1_RadioButton@@QAEXHH@Z 0x00407020-0x00407028
// symbol    ?adjust_radio_button1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_left_down(void *self, void *,
                                                  int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<RadioButton *>(object - vtordisp)->RadioButton::on_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x00407030 ?on_key_down@thunk1_RadioButton@@QAEHH@Z 0x00407030-0x00407038
// symbol    ?adjust_radio_button1_on_key_down@@YIHPAX0H@Z
// size      8 bytes
// prototype int (__thiscall ?on_key_down@thunk1_RadioButton@@QAEHH@Z)(RadioButton* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_radio_button1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<RadioButton *>(object - vtordisp)->RadioButton::on_key_down(arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x00407040 ?on_left_double_click@thunk1_RadioButton@@QAEXHH@Z 0x00407040-0x00407048
// symbol    ?adjust_radio_button1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_left_double_click(void *self, void *,
                                                          int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<RadioButton *>(object - vtordisp)->RadioButton::on_left_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x00407050 ?on_dialog_focus@thunk1_RadioButton@@QAEXH@Z 0x00407050-0x00407058
// symbol    ?adjust_radio_button1_on_dialog_focus@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_dialog_focus@thunk1_RadioButton@@QAEXH@Z)(RadioButton* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_dialog_focus(void *self, void *,
                                                     int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<RadioButton *>(object - vtordisp)->RadioButton::on_dialog_focus(arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk2 subobject to the one
         RadioButton::dtor expects, then forward unchanged.
// ORIGINAL: 0x00407060 ??1thunk2_RadioButton@@QAE@H@Z 0x00407060-0x00407071
// symbol    ?adjust_radio_button2_dtor@@YIXPAX0H@Z
// size      17 bytes
// prototype void (__thiscall ??1thunk2_RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button2_dtor(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    (ORIGINAL(object - vtordisp - 1092)->*RadioButtonDtorTarget)(arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk3 subobject to the one
         RadioButton::dtor expects, then forward unchanged.
// ORIGINAL: 0x00407080 ??1thunk3_RadioButton@@QAE@H@Z 0x00407080-0x0040708E
// symbol    ?adjust_radio_button3_dtor@@YIXPAX0H@Z
// size      14 bytes
// prototype void (__thiscall ??1thunk3_RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button3_dtor(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    (ORIGINAL(object - vtordisp - 2584)->*RadioButtonDtorTarget)(arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::attach expects, then forward unchanged.
// ORIGINAL: 0x00407090 ?attach@thunk1_RadioButton@@QAEHPAUGraphicWin@@HHH@Z 0x00407090-0x00407098
// symbol    ?adjust_radio_button1_attach@@YIHPAX00HHH@Z
// size      8 bytes
// prototype int (__thiscall ?attach@thunk1_RadioButton@@QAEHPAUGraphicWin@@HHH@Z)(RadioButton* this, GraphicWin*, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_radio_button1_attach(void *self, void *, void *arg0,
                                           int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<RadioButton *>(object - vtordisp)->RadioButton::attach(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk3 subobject to the one
         RadioButton::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x004070A0 ?on_redraw@thunk3_RadioButton@@QAEXXZ 0x004070A0-0x004070AE BYTE_EXACT
// symbol    ?adjust_radio_button3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_RadioButton@@QAEXXZ)(RadioButton* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<RadioButton *>(object - vtordisp - 2584)->RadioButton::on_redraw();
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x004070F0 ??_Gthunk1_Dialogs@@UAEPAXI@Z 0x004070F0-0x004070F8
// symbol    ?adjust_dialogs1_scalar_delete@@YIPAXPAX0I@Z
// size      8 bytes
// prototype void* (__thiscall ??_Gthunk1_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs1_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return (ORIGINAL(object - vtordisp)->*DialogsScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scroll_create expects, then forward unchanged.
// ORIGINAL: 0x00407150 ?on_scroll_create@thunk1_Dialogs@@QAEXXZ 0x00407150-0x00407158 BYTE_EXACT
// symbol    ?adjust_dialogs1_on_scroll_create@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_scroll_create@thunk1_Dialogs@@QAEXXZ)(Dialogs* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_scroll_create(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_scroll_create();
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scroll_delete expects, then forward unchanged.
// ORIGINAL: 0x00407160 ?on_scroll_delete@thunk1_Dialogs@@QAEHPAUScroll@@@Z 0x00407160-0x00407168
// symbol    ?adjust_dialogs1_on_scroll_delete@@YIHPAX00@Z
// size      8 bytes
// prototype int (__thiscall ?on_scroll_delete@thunk1_Dialogs@@QAEHPAUScroll@@@Z)(Dialogs* this, Scroll*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_dialogs1_on_scroll_delete(void *self, void *,
                                                void *arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_scroll_delete(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00407170 ?on_redraw@thunk1_Dialogs@@QAEXXZ 0x00407170-0x00407178 BYTE_EXACT
// symbol    ?adjust_dialogs1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_Dialogs@@QAEXXZ)(Dialogs* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_redraw();
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mousewheel expects, then forward unchanged.
// ORIGINAL: 0x00407180 ?on_mousewheel@thunk1_Dialogs@@QAEXH@Z 0x00407180-0x00407188
// symbol    ?adjust_dialogs1_on_mousewheel@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_mousewheel@thunk1_Dialogs@@QAEXH@Z)(Dialogs* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_mousewheel(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_mousewheel(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x00407190 ?on_mouse_move@thunk1_Dialogs@@QAEXHH@Z 0x00407190-0x00407198
// symbol    ?adjust_dialogs1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_mouse_move(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x004071A0 ?on_mouse_leave@thunk1_Dialogs@@QAEXHH@Z 0x004071A0-0x004071A8
// symbol    ?adjust_dialogs1_on_mouse_leave@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_leave@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_mouse_leave(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_mouse_leave(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x004071B0 ?on_left_click@thunk1_Dialogs@@QAEXHH@Z 0x004071B0-0x004071B8
// symbol    ?adjust_dialogs1_on_left_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_click@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_left_click(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_left_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x004071C0 ?on_left_down@thunk1_Dialogs@@QAEXHH@Z 0x004071C0-0x004071C8
// symbol    ?adjust_dialogs1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_left_down(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x004071D0 ?on_left_up@thunk1_Dialogs@@QAEXHH@Z 0x004071D0-0x004071D8
// symbol    ?adjust_dialogs1_on_left_up@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_up@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_left_up(void *self, void *, int arg0,
                                           int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_left_up(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x004071E0 ?on_right_click@thunk1_Dialogs@@QAEXHH@Z 0x004071E0-0x004071E8
// symbol    ?adjust_dialogs1_on_right_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_click@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_right_click(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_right_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x004071F0 ?on_right_down@thunk1_Dialogs@@QAEXHH@Z 0x004071F0-0x004071F8
// symbol    ?adjust_dialogs1_on_right_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_down@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_right_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_right_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_up expects, then forward unchanged.
// ORIGINAL: 0x00407200 ?on_right_up@thunk1_Dialogs@@QAEXHH@Z 0x00407200-0x00407208
// symbol    ?adjust_dialogs1_on_right_up@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_up@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_right_up(void *self, void *, int arg0,
                                            int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_right_up(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x00407210 ?on_key_down@thunk1_Dialogs@@QAEHH@Z 0x00407210-0x00407218
// symbol    ?adjust_dialogs1_on_key_down@@YIHPAX0H@Z
// size      8 bytes
// prototype int (__thiscall ?on_key_down@thunk1_Dialogs@@QAEHH@Z)(Dialogs* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_dialogs1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_key_down(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x00407220 ?on_left_double_click@thunk1_Dialogs@@QAEXHH@Z 0x00407220-0x00407228
// symbol    ?adjust_dialogs1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_left_double_click(void *self, void *,
                                                     int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_left_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_double_click expects, then forward unchanged.
// ORIGINAL: 0x00407230 ?on_right_double_click@thunk1_Dialogs@@QAEXHH@Z 0x00407230-0x00407238
// symbol    ?adjust_dialogs1_on_right_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_double_click@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_right_double_click(void *self, void *,
                                                      int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_right_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scrolled expects, then forward unchanged.
// ORIGINAL: 0x00407240 ?on_scrolled@thunk1_Dialogs@@QAEXHH@Z 0x00407240-0x00407248
// symbol    ?adjust_dialogs1_on_scrolled@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_scrolled@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_scrolled(void *self, void *, int arg0,
                                            int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_scrolled(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scrolling expects, then forward unchanged.
// ORIGINAL: 0x00407250 ?on_scrolling@thunk1_Dialogs@@QAEXHH@Z 0x00407250-0x00407258
// symbol    ?adjust_dialogs1_on_scrolling@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_scrolling@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_scrolling(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_scrolling(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x00407260 ?on_dialog_focus@thunk1_Dialogs@@QAEXH@Z 0x00407260-0x00407268
// symbol    ?adjust_dialogs1_on_dialog_focus@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_dialog_focus@thunk1_Dialogs@@QAEXH@Z)(Dialogs* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_dialog_focus(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::on_dialog_focus(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::pass_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x00407270 ?pass_dialog_focus@thunk1_Dialogs@@QAEXXZ 0x00407270-0x00407278 BYTE_EXACT
// symbol    ?adjust_dialogs1_pass_dialog_focus@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?pass_dialog_focus@thunk1_Dialogs@@QAEXXZ)(Dialogs* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_pass_dialog_focus(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::pass_dialog_focus();
}

/*
Purpose: Adjust the receiver from Dialogs's thunk2 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x00407280 ??_Gthunk2_Dialogs@@UAEPAXI@Z 0x00407280-0x00407291
// symbol    ?adjust_dialogs2_scalar_delete@@YIPAXPAX0I@Z
// size      17 bytes
// prototype void* (__thiscall ??_Gthunk2_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs2_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return (ORIGINAL(object - vtordisp - 1092)->*DialogsScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk3 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x004072A0 ??_Gthunk3_Dialogs@@UAEPAXI@Z 0x004072A0-0x004072AE
// symbol    ?adjust_dialogs3_scalar_delete@@YIPAXPAX0I@Z
// size      14 bytes
// prototype void* (__thiscall ??_Gthunk3_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs3_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return (ORIGINAL(object - vtordisp - 2584)->*DialogsScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::attach expects, then forward unchanged.
// ORIGINAL: 0x004072B0 ?attach@thunk1_Dialogs@@QAEHPAUGraphicWin@@HHH@Z 0x004072B0-0x004072B8
// symbol    ?adjust_dialogs1_attach@@YIHPAX00HHH@Z
// size      8 bytes
// prototype int (__thiscall ?attach@thunk1_Dialogs@@QAEHPAUGraphicWin@@HHH@Z)(Dialogs* this, GraphicWin*, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_dialogs1_attach(void *self, void *, void *arg0,
                                      int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<Dialogs *>(object - vtordisp)->Dialogs::attach(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk3 subobject to the one
         Dialogs::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x004072C0 ?on_redraw@thunk3_Dialogs@@QAEXXZ 0x004072C0-0x004072CE BYTE_EXACT
// symbol    ?adjust_dialogs3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_Dialogs@@QAEXXZ)(Dialogs* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Dialogs *>(object - vtordisp - 2584)->Dialogs::on_redraw();
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x00421820 ??_Gthunk1_MapWin@@UAEPAXI@Z 0x00421820-0x00421828
// symbol    ?adjust_map_win1_scalar_delete@@YIPAXPAX0I@Z
// size      8 bytes
// prototype void* (__thiscall ??_Gthunk1_MapWin@@UAEPAXI@Z)(MapWin* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_map_win1_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return (ORIGINAL(object - vtordisp)->*MapWinScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
// ORIGINAL: 0x00421870 ?on_lose_mouse_capture@thunk1_MapWin@@QAEXXZ 0x00421870-0x00421878 BYTE_EXACT
// symbol    ?adjust_map_win1_on_lose_mouse_capture@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_lose_mouse_capture@thunk1_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_lose_mouse_capture(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_lose_mouse_capture();
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00421880 ?on_redraw@thunk1_MapWin@@QAEXXZ 0x00421880-0x00421888 BYTE_EXACT
// symbol    ?adjust_map_win1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_redraw();
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x00421890 ?on_resize@thunk1_MapWin@@QAEXHH@Z 0x00421890-0x00421898
// symbol    ?adjust_map_win1_on_resize@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_resize@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_resize(void *self, void *, int arg0,
                                          int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_resize(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_sys_close expects, then forward unchanged.
// ORIGINAL: 0x004218A0 ?on_sys_close@thunk1_MapWin@@QAEXXZ 0x004218A0-0x004218A8 BYTE_EXACT
// symbol    ?adjust_map_win1_on_sys_close@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_sys_close@thunk1_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_sys_close(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_sys_close();
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x004218B0 ?on_mouse_move@thunk1_MapWin@@QAEXHH@Z 0x004218B0-0x004218B8
// symbol    ?adjust_map_win1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_mouse_move(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x004218C0 ?on_left_click@thunk1_MapWin@@QAEXHH@Z 0x004218C0-0x004218C8
// symbol    ?adjust_map_win1_on_left_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_click@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_left_click(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_left_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x004218D0 ?on_left_down@thunk1_MapWin@@QAEXHH@Z 0x004218D0-0x004218D8
// symbol    ?adjust_map_win1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_left_down(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x004218E0 ?on_left_up@thunk1_MapWin@@QAEXHH@Z 0x004218E0-0x004218E8
// symbol    ?adjust_map_win1_on_left_up@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_up@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_left_up(void *self, void *, int arg0,
                                           int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_left_up(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x004218F0 ?on_right_click@thunk1_MapWin@@QAEXHH@Z 0x004218F0-0x004218F8
// symbol    ?adjust_map_win1_on_right_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_click@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_right_click(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_right_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x00421900 ?on_right_down@thunk1_MapWin@@QAEXHH@Z 0x00421900-0x00421908
// symbol    ?adjust_map_win1_on_right_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_down@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_right_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_right_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x00421910 ?on_left_double_click@thunk1_MapWin@@QAEXHH@Z 0x00421910-0x00421918
// symbol    ?adjust_map_win1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_left_double_click(void *self, void *,
                                                     int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_left_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_nc_left_down expects, then forward unchanged.
// ORIGINAL: 0x00421920 ?on_nc_left_down@thunk1_MapWin@@QAEXHH@Z 0x00421920-0x00421928
// symbol    ?adjust_map_win1_on_nc_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_nc_left_down@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_nc_left_down(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_nc_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
// ORIGINAL: 0x00421930 ?on_button_clicked@thunk1_MapWin@@QAEXH@Z 0x00421930-0x00421938
// symbol    ?adjust_map_win1_on_button_clicked@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_button_clicked@thunk1_MapWin@@QAEXH@Z)(MapWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_button_clicked(void *self, void *,
                                                  int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_button_clicked(arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_nc_hittest expects, then forward unchanged.
// ORIGINAL: 0x00421940 ?on_nc_hittest@thunk1_MapWin@@QAEXHH@Z 0x00421940-0x00421948
// symbol    ?adjust_map_win1_on_nc_hittest@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_nc_hittest@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_nc_hittest(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp)->MapWin::on_nc_hittest(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk2 subobject to the one
         MapWin::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x00421950 ??_Gthunk2_MapWin@@UAEPAXI@Z 0x00421950-0x00421961
// symbol    ?adjust_map_win2_scalar_delete@@YIPAXPAX0I@Z
// size      17 bytes
// prototype void* (__thiscall ??_Gthunk2_MapWin@@UAEPAXI@Z)(MapWin* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_map_win2_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return (ORIGINAL(object - vtordisp - 1092)->*MapWinScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
// ORIGINAL: 0x0048BF60 ?on_lose_mouse_capture@thunk5_MapWin@@QAEXXZ 0x0048BF60-0x0048BF6E BYTE_EXACT
// symbol    ?adjust_map_win5_on_lose_mouse_capture@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_lose_mouse_capture@thunk5_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_lose_mouse_capture(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_lose_mouse_capture();
}

/*
Purpose: Adjust the receiver from PlanWin's thunk1 subobject to the one
         PlanWin::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0048BF70 ?on_redraw@thunk1_PlanWin@@QAEXXZ 0x0048BF70-0x0048BF78 BYTE_EXACT
// symbol    ?adjust_plan_win1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_PlanWin@@QAEXXZ)(PlanWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_plan_win1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<PlanWin *>(object - vtordisp)->PlanWin::on_redraw();
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x0048BF80 ?on_resize@thunk5_MapWin@@QAEXHH@Z 0x0048BF80-0x0048BF8E
// symbol    ?adjust_map_win5_on_resize@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_resize@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_resize(void *self, void *, int arg0,
                                          int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_resize(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_sys_close expects, then forward unchanged.
// ORIGINAL: 0x0048BF90 ?on_sys_close@thunk5_MapWin@@QAEXXZ 0x0048BF90-0x0048BF9E BYTE_EXACT
// symbol    ?adjust_map_win5_on_sys_close@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_sys_close@thunk5_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_sys_close(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_sys_close();
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0048BFA0 ?on_mouse_move@thunk5_MapWin@@QAEXHH@Z 0x0048BFA0-0x0048BFAE
// symbol    ?adjust_map_win5_on_mouse_move@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_mouse_move@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_mouse_move(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x0048BFB0 ?on_left_click@thunk5_MapWin@@QAEXHH@Z 0x0048BFB0-0x0048BFBE
// symbol    ?adjust_map_win5_on_left_click@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_left_click@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_left_click(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_left_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x0048BFC0 ?on_left_down@thunk5_MapWin@@QAEXHH@Z 0x0048BFC0-0x0048BFCE
// symbol    ?adjust_map_win5_on_left_down@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_left_down@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_left_down(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x0048BFD0 ?on_left_up@thunk5_MapWin@@QAEXHH@Z 0x0048BFD0-0x0048BFDE
// symbol    ?adjust_map_win5_on_left_up@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_left_up@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_left_up(void *self, void *, int arg0,
                                           int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_left_up(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x0048BFE0 ?on_right_click@thunk5_MapWin@@QAEXHH@Z 0x0048BFE0-0x0048BFEE
// symbol    ?adjust_map_win5_on_right_click@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_right_click@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_right_click(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_right_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x0048BFF0 ?on_right_down@thunk5_MapWin@@QAEXHH@Z 0x0048BFF0-0x0048BFFE
// symbol    ?adjust_map_win5_on_right_down@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_right_down@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_right_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_right_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x0048C000 ?on_left_double_click@thunk5_MapWin@@QAEXHH@Z 0x0048C000-0x0048C00E
// symbol    ?adjust_map_win5_on_left_double_click@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_left_double_click@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_left_double_click(void *self, void *,
                                                     int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_left_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_nc_left_down expects, then forward unchanged.
// ORIGINAL: 0x0048C010 ?on_nc_left_down@thunk5_MapWin@@QAEXHH@Z 0x0048C010-0x0048C01E
// symbol    ?adjust_map_win5_on_nc_left_down@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_nc_left_down@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_nc_left_down(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_nc_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
// ORIGINAL: 0x0048C020 ?on_button_clicked@thunk5_MapWin@@QAEXH@Z 0x0048C020-0x0048C02E
// symbol    ?adjust_map_win5_on_button_clicked@@YIXPAX0H@Z
// size      14 bytes
// prototype void (__thiscall ?on_button_clicked@thunk5_MapWin@@QAEXH@Z)(MapWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_button_clicked(void *self, void *,
                                                  int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_button_clicked(arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_nc_hittest expects, then forward unchanged.
// ORIGINAL: 0x0048C030 ?on_nc_hittest@thunk5_MapWin@@QAEXHH@Z 0x0048C030-0x0048C03E
// symbol    ?adjust_map_win5_on_nc_hittest@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_nc_hittest@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_nc_hittest(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 1508)->MapWin::on_nc_hittest(arg0, arg1);
}

/*
Purpose: Adjust the receiver from PlanWin's thunk2 subobject to the one
         PlanWin::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x0048C040 ??_Gthunk2_PlanWin@@UAEPAXI@Z 0x0048C040-0x0048C051
// symbol    ?adjust_plan_win2_scalar_delete@@YIPAXPAX0I@Z
// size      17 bytes
// prototype void* (__thiscall ??_Gthunk2_PlanWin@@UAEPAXI@Z)(PlanWin* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_plan_win2_scalar_delete(void *self, void *,
                                                 unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return (ORIGINAL(object - vtordisp - 1092)->*PlanWinScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x0051DDE0 ??_Gthunk1_Console@@UAEPAXI@Z 0x0051DDE0-0x0051DDE8
// symbol    ?adjust_console1_scalar_delete@@YIPAXPAX0I@Z
// size      8 bytes
// prototype void* (__thiscall ??_Gthunk1_Console@@UAEPAXI@Z)(Console* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_console1_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return (ORIGINAL(object - vtordisp)->*ConsoleScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
// ORIGINAL: 0x0051DE30 ?on_lose_mouse_capture@thunk4_MapWin@@QAEXXZ 0x0051DE30-0x0051DE3E BYTE_EXACT
// symbol    ?adjust_map_win4_on_lose_mouse_capture@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_lose_mouse_capture@thunk4_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_lose_mouse_capture(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_lose_mouse_capture();
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0051DE40 ?on_redraw@thunk4_MapWin@@QAEXXZ 0x0051DE40-0x0051DE4E BYTE_EXACT
// symbol    ?adjust_map_win4_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk4_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_redraw();
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x0051DE50 ?on_resize@thunk4_MapWin@@QAEXHH@Z 0x0051DE50-0x0051DE5E
// symbol    ?adjust_map_win4_on_resize@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_resize@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_resize(void *self, void *, int arg0,
                                          int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_resize(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_sys_close expects, then forward unchanged.
// ORIGINAL: 0x0051DE60 ?on_sys_close@thunk1_Console@@QAEXXZ 0x0051DE60-0x0051DE68 BYTE_EXACT
// symbol    ?adjust_console1_on_sys_close@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_sys_close@thunk1_Console@@QAEXXZ)(Console* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_sys_close(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Console *>(object - vtordisp)->Console::on_sys_close();
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0051DE70 ?on_mouse_move@thunk4_MapWin@@QAEXHH@Z 0x0051DE70-0x0051DE7E
// symbol    ?adjust_map_win4_on_mouse_move@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_mouse_move@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_mouse_move(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x0051DE80 ?on_left_click@thunk4_MapWin@@QAEXHH@Z 0x0051DE80-0x0051DE8E
// symbol    ?adjust_map_win4_on_left_click@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_left_click@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_left_click(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_left_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x0051DE90 ?on_left_down@thunk4_MapWin@@QAEXHH@Z 0x0051DE90-0x0051DE9E
// symbol    ?adjust_map_win4_on_left_down@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_left_down@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_left_down(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x0051DEA0 ?on_left_up@thunk4_MapWin@@QAEXHH@Z 0x0051DEA0-0x0051DEAE
// symbol    ?adjust_map_win4_on_left_up@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_left_up@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_left_up(void *self, void *, int arg0,
                                           int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_left_up(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x0051DEB0 ?on_right_click@thunk4_MapWin@@QAEXHH@Z 0x0051DEB0-0x0051DEBE
// symbol    ?adjust_map_win4_on_right_click@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_right_click@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_right_click(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_right_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x0051DEC0 ?on_right_down@thunk4_MapWin@@QAEXHH@Z 0x0051DEC0-0x0051DECE
// symbol    ?adjust_map_win4_on_right_down@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_right_down@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_right_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_right_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_key_click expects, then forward unchanged.
// ORIGINAL: 0x0051DED0 ?on_key_click@thunk1_Console@@QAEHHH@Z 0x0051DED0-0x0051DED8
// symbol    ?adjust_console1_on_key_click@@YIHPAX0HH@Z
// size      8 bytes
// prototype int (__thiscall ?on_key_click@thunk1_Console@@QAEHHH@Z)(Console* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_console1_on_key_click(void *self, void *, int arg0,
                                            int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<Console *>(object - vtordisp)->Console::on_key_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x0051DEE0 ?on_left_double_click@thunk4_MapWin@@QAEXHH@Z 0x0051DEE0-0x0051DEEE
// symbol    ?adjust_map_win4_on_left_double_click@@YIXPAX0HH@Z
// size      14 bytes
// prototype void (__thiscall ?on_left_double_click@thunk4_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_left_double_click(void *self, void *,
                                                     int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_left_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0051DEF0 ?on_nc_mouse_move@thunk1_Console@@QAEXHH@Z 0x0051DEF0-0x0051DEF8
// symbol    ?adjust_console1_on_nc_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_nc_mouse_move@thunk1_Console@@QAEXHH@Z)(Console* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_nc_mouse_move(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Console *>(object - vtordisp)->Console::on_nc_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_left_down expects, then forward unchanged.
// ORIGINAL: 0x0051DF00 ?on_nc_left_down@thunk1_Console@@QAEXHH@Z 0x0051DF00-0x0051DF08
// symbol    ?adjust_console1_on_nc_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_nc_left_down@thunk1_Console@@QAEXHH@Z)(Console* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_nc_left_down(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Console *>(object - vtordisp)->Console::on_nc_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_left_up expects, then forward unchanged.
// ORIGINAL: 0x0051DF10 ?on_nc_left_up@thunk1_Console@@QAEXHH@Z 0x0051DF10-0x0051DF18
// symbol    ?adjust_console1_on_nc_left_up@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_nc_left_up@thunk1_Console@@QAEXHH@Z)(Console* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_nc_left_up(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Console *>(object - vtordisp)->Console::on_nc_left_up(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
// ORIGINAL: 0x0051DF20 ?on_button_clicked@thunk4_MapWin@@QAEXH@Z 0x0051DF20-0x0051DF2E
// symbol    ?adjust_map_win4_on_button_clicked@@YIXPAX0H@Z
// size      14 bytes
// prototype void (__thiscall ?on_button_clicked@thunk4_MapWin@@QAEXH@Z)(MapWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_button_clicked(void *self, void *,
                                                  int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<MapWin *>(object - vtordisp - 9000)->MapWin::on_button_clicked(arg0);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_post_redraw_nc_buffer expects, then forward unchanged.
// ORIGINAL: 0x0051DF30 ?on_post_redraw_nc_buffer@thunk1_Console@@QAEXPAUBuffer@@H@Z 0x0051DF30-0x0051DF38
// symbol    ?adjust_console1_on_post_redraw_nc_buffer@@YIXPAX00H@Z
// size      8 bytes
// prototype void (__thiscall ?on_post_redraw_nc_buffer@thunk1_Console@@QAEXPAUBuffer@@H@Z)(Console* this, Buffer*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_post_redraw_nc_buffer(void *self, void *,
                                                         void *arg0,
                                                         int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Console *>(object - vtordisp)->Console::on_post_redraw_nc_buffer(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_hittest expects, then forward unchanged.
// ORIGINAL: 0x0051DF40 ?on_nc_hittest@thunk1_Console@@QAEXHH@Z 0x0051DF40-0x0051DF48
// symbol    ?adjust_console1_on_nc_hittest@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_nc_hittest@thunk1_Console@@QAEXHH@Z)(Console* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_nc_hittest(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<Console *>(object - vtordisp)->Console::on_nc_hittest(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk2 subobject to the one
         Console::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x0051DF50 ??_Gthunk2_Console@@UAEPAXI@Z 0x0051DF50-0x0051DF61
// symbol    ?adjust_console2_scalar_delete@@YIPAXPAX0I@Z
// size      17 bytes
// prototype void* (__thiscall ??_Gthunk2_Console@@UAEPAXI@Z)(Console* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_console2_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return (ORIGINAL(object - vtordisp - 1092)->*ConsoleScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0060CF50 ?on_redraw@thunk1_ListBox@@QAEXXZ 0x0060CF50-0x0060CF58 BYTE_EXACT
// symbol    ?adjust_list_box1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_ListBox@@QAEXXZ)(ListBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_redraw();
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mousewheel expects, then forward unchanged.
// ORIGINAL: 0x0060CF60 ?on_mousewheel@thunk1_ListBox@@QAEXH@Z 0x0060CF60-0x0060CF68
// symbol    ?adjust_list_box1_on_mousewheel@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_mousewheel@thunk1_ListBox@@QAEXH@Z)(ListBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_mousewheel(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_mousewheel(arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0060CF70 ?on_mouse_move@thunk1_ListBox@@QAEXHH@Z 0x0060CF70-0x0060CF78
// symbol    ?adjust_list_box1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_mouse_move(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x0060CF80 ?on_mouse_leave@thunk1_ListBox@@QAEXHH@Z 0x0060CF80-0x0060CF88
// symbol    ?adjust_list_box1_on_mouse_leave@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_leave@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_mouse_leave(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_mouse_leave(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x0060CF90 ?on_left_down@thunk1_ListBox@@QAEXHH@Z 0x0060CF90-0x0060CF98
// symbol    ?adjust_list_box1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_left_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x0060CFA0 ?on_right_down@thunk1_ListBox@@QAEXHH@Z 0x0060CFA0-0x0060CFA8
// symbol    ?adjust_list_box1_on_right_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_down@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_right_down(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_right_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x0060CFB0 ?on_key_down@thunk1_ListBox@@QAEXH@Z 0x0060CFB0-0x0060CFB8
// symbol    ?adjust_list_box1_on_key_down@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_key_down@thunk1_ListBox@@QAEXH@Z)(ListBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_key_down(arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x0060CFC0 ?on_left_double_click@thunk1_ListBox@@QAEXHH@Z 0x0060CFC0-0x0060CFC8
// symbol    ?adjust_list_box1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_left_double_click(void *self, void *,
                                                      int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_left_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_right_double_click expects, then forward unchanged.
// ORIGINAL: 0x0060CFD0 ?on_right_double_click@thunk1_ListBox@@QAEXHH@Z 0x0060CFD0-0x0060CFD8
// symbol    ?adjust_list_box1_on_right_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_double_click@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_right_double_click(void *self, void *,
                                                       int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_right_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_scrolled expects, then forward unchanged.
// ORIGINAL: 0x0060CFE0 ?on_scrolled@thunk1_ListBox@@QAEXHH@Z 0x0060CFE0-0x0060CFE8
// symbol    ?adjust_list_box1_on_scrolled@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_scrolled@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_scrolled(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_scrolled(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_scrolling expects, then forward unchanged.
// ORIGINAL: 0x0060CFF0 ?on_scrolling@thunk1_ListBox@@QAEXHH@Z 0x0060CFF0-0x0060CFF8
// symbol    ?adjust_list_box1_on_scrolling@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_scrolling@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_scrolling(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_scrolling(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x0060D000 ?on_dialog_focus@thunk1_ListBox@@QAEXH@Z 0x0060D000-0x0060D008
// symbol    ?adjust_list_box1_on_dialog_focus@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_dialog_focus@thunk1_ListBox@@QAEXH@Z)(ListBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_dialog_focus(void *self, void *,
                                                 int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::on_dialog_focus(arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::attach expects, then forward unchanged.
// ORIGINAL: 0x0060D040 ?attach@thunk1_ListBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060D040-0x0060D048
// symbol    ?adjust_list_box1_attach@@YIHPAX00HHH@Z
// size      8 bytes
// prototype int (__thiscall ?attach@thunk1_ListBox@@QAEHPAUGraphicWin@@HHH@Z)(ListBox* this, GraphicWin*, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_list_box1_attach(void *self, void *, void *arg0,
                                       int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<ListBox *>(object - vtordisp)->ListBox::attach(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from ListBox's thunk3 subobject to the one
         ListBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0060D050 ?on_redraw@thunk3_ListBox@@QAEXXZ 0x0060D050-0x0060D05E BYTE_EXACT
// symbol    ?adjust_list_box3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_ListBox@@QAEXXZ)(ListBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<ListBox *>(object - vtordisp - 2584)->ListBox::on_redraw();
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0060FE40 ?on_redraw@thunk1_CheckBox@@QAEXXZ 0x0060FE40-0x0060FE48 BYTE_EXACT
// symbol    ?adjust_check_box1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_CheckBox@@QAEXXZ)(CheckBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<CheckBox *>(object - vtordisp)->CheckBox::on_redraw();
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0060FE50 ?on_mouse_move@thunk1_CheckBox@@QAEXHH@Z 0x0060FE50-0x0060FE58
// symbol    ?adjust_check_box1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_mouse_move(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<CheckBox *>(object - vtordisp)->CheckBox::on_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x0060FE60 ?on_mouse_leave@thunk1_CheckBox@@QAEXHH@Z 0x0060FE60-0x0060FE68
// symbol    ?adjust_check_box1_on_mouse_leave@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_leave@thunk1_CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_mouse_leave(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<CheckBox *>(object - vtordisp)->CheckBox::on_mouse_leave(arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x0060FE70 ?on_left_down@thunk1_CheckBox@@QAEXHH@Z 0x0060FE70-0x0060FE78
// symbol    ?adjust_check_box1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_left_down(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<CheckBox *>(object - vtordisp)->CheckBox::on_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x0060FE80 ?on_key_down@thunk1_CheckBox@@QAEHH@Z 0x0060FE80-0x0060FE88
// symbol    ?adjust_check_box1_on_key_down@@YIHPAX0H@Z
// size      8 bytes
// prototype int (__thiscall ?on_key_down@thunk1_CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_check_box1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<CheckBox *>(object - vtordisp)->CheckBox::on_key_down(arg0);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x0060FE90 ?on_left_double_click@thunk1_CheckBox@@QAEXHH@Z 0x0060FE90-0x0060FE98
// symbol    ?adjust_check_box1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_left_double_click(void *self, void *,
                                                       int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<CheckBox *>(object - vtordisp)->CheckBox::on_left_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x0060FEA0 ?on_dialog_focus@thunk1_CheckBox@@QAEXH@Z 0x0060FEA0-0x0060FEA8
// symbol    ?adjust_check_box1_on_dialog_focus@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_dialog_focus@thunk1_CheckBox@@QAEXH@Z)(CheckBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_dialog_focus(void *self, void *,
                                                  int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<CheckBox *>(object - vtordisp)->CheckBox::on_dialog_focus(arg0);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::attach expects, then forward unchanged.
// ORIGINAL: 0x0060FEE0 ?attach@thunk1_CheckBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060FEE0-0x0060FEE8
// symbol    ?adjust_check_box1_attach@@YIHPAX00HHH@Z
// size      8 bytes
// prototype int (__thiscall ?attach@thunk1_CheckBox@@QAEHPAUGraphicWin@@HHH@Z)(CheckBox* this, GraphicWin*, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_check_box1_attach(void *self, void *, void *arg0,
                                        int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<CheckBox *>(object - vtordisp)->CheckBox::attach(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk3 subobject to the one
         CheckBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0060FEF0 ?on_redraw@thunk3_CheckBox@@QAEXXZ 0x0060FEF0-0x0060FEFE BYTE_EXACT
// symbol    ?adjust_check_box3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_CheckBox@@QAEXXZ)(CheckBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<CheckBox *>(object - vtordisp - 2584)->CheckBox::on_redraw();
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00611820 ?on_redraw@thunk1_SpriteBox@@QAEXXZ 0x00611820-0x00611828 BYTE_EXACT
// symbol    ?adjust_sprite_box1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_SpriteBox@@QAEXXZ)(SpriteBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_redraw();
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x00611830 ?on_mouse_move@thunk1_SpriteBox@@QAEXHH@Z 0x00611830-0x00611838
// symbol    ?adjust_sprite_box1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_mouse_move(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_mouse_move(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x00611840 ?on_mouse_leave@thunk1_SpriteBox@@QAEXHH@Z 0x00611840-0x00611848
// symbol    ?adjust_sprite_box1_on_mouse_leave@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_leave@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_mouse_leave(void *self, void *,
                                                  int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_mouse_leave(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x00611850 ?on_left_click@thunk1_SpriteBox@@QAEXHH@Z 0x00611850-0x00611858
// symbol    ?adjust_sprite_box1_on_left_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_click@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_left_click(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_left_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x00611860 ?on_left_down@thunk1_SpriteBox@@QAEXHH@Z 0x00611860-0x00611868
// symbol    ?adjust_sprite_box1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_left_down(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_left_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x00611870 ?on_left_up@thunk1_SpriteBox@@QAEXHH@Z 0x00611870-0x00611878
// symbol    ?adjust_sprite_box1_on_left_up@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_up@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_left_up(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_left_up(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x00611880 ?on_right_click@thunk1_SpriteBox@@QAEXHH@Z 0x00611880-0x00611888
// symbol    ?adjust_sprite_box1_on_right_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_click@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_right_click(void *self, void *,
                                                  int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_right_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x00611890 ?on_right_down@thunk1_SpriteBox@@QAEXHH@Z 0x00611890-0x00611898
// symbol    ?adjust_sprite_box1_on_right_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_down@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_right_down(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_right_down(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_up expects, then forward unchanged.
// ORIGINAL: 0x006118A0 ?on_right_up@thunk1_SpriteBox@@QAEXHH@Z 0x006118A0-0x006118A8
// symbol    ?adjust_sprite_box1_on_right_up@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_up@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_right_up(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_right_up(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x006118B0 ?on_key_down@thunk1_SpriteBox@@QAEHH@Z 0x006118B0-0x006118B8
// symbol    ?adjust_sprite_box1_on_key_down@@YIHPAX0H@Z
// size      8 bytes
// prototype int (__thiscall ?on_key_down@thunk1_SpriteBox@@QAEHH@Z)(SpriteBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_sprite_box1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_key_down(arg0);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x006118C0 ?on_left_double_click@thunk1_SpriteBox@@QAEXHH@Z 0x006118C0-0x006118C8
// symbol    ?adjust_sprite_box1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_left_double_click(void *self, void *,
                                                        int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_left_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_double_click expects, then forward unchanged.
// ORIGINAL: 0x006118D0 ?on_right_double_click@thunk1_SpriteBox@@QAEXHH@Z 0x006118D0-0x006118D8
// symbol    ?adjust_sprite_box1_on_right_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_double_click@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_right_double_click(void *self, void *,
                                                         int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_right_double_click(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x006118E0 ?on_dialog_focus@thunk1_SpriteBox@@QAEXH@Z 0x006118E0-0x006118E8
// symbol    ?adjust_sprite_box1_on_dialog_focus@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_dialog_focus@thunk1_SpriteBox@@QAEXH@Z)(SpriteBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_dialog_focus(void *self, void *,
                                                   int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::on_dialog_focus(arg0);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::attach expects, then forward unchanged.
// ORIGINAL: 0x00611920 ?attach@thunk1_SpriteBox@@QAEHPAUGraphicWin@@HHH@Z 0x00611920-0x00611928
// symbol    ?adjust_sprite_box1_attach@@YIHPAX00HHH@Z
// size      8 bytes
// prototype int (__thiscall ?attach@thunk1_SpriteBox@@QAEHPAUGraphicWin@@HHH@Z)(SpriteBox* this, GraphicWin*, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_sprite_box1_attach(void *self, void *, void *arg0,
                                         int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<SpriteBox *>(object - vtordisp)->SpriteBox::attach(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk3 subobject to the one
         SpriteBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00611930 ?on_redraw@thunk3_SpriteBox@@QAEXXZ 0x00611930-0x0061193E BYTE_EXACT
// symbol    ?adjust_sprite_box3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_SpriteBox@@QAEXXZ)(SpriteBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<SpriteBox *>(object - vtordisp - 2584)->SpriteBox::on_redraw();
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x006127B0 ?on_redraw@thunk1_EditGroup@@QAEXXZ 0x006127B0-0x006127B8 BYTE_EXACT
// symbol    ?adjust_edit_group1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_EditGroup@@QAEXXZ)(EditGroup* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_edit_group1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<EditGroup *>(object - vtordisp)->EditGroup::on_redraw();
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x006127C0 ?on_dialog_focus@thunk1_EditGroup@@QAEXH@Z 0x006127C0-0x006127C8
// symbol    ?adjust_edit_group1_on_dialog_focus@@YIXPAX0H@Z
// size      8 bytes
// prototype void (__thiscall ?on_dialog_focus@thunk1_EditGroup@@QAEXH@Z)(EditGroup* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_edit_group1_on_dialog_focus(void *self, void *,
                                                   int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<EditGroup *>(object - vtordisp)->EditGroup::on_dialog_focus(arg0);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::pass_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x006127D0 ?pass_dialog_focus@thunk1_EditGroup@@QAEXXZ 0x006127D0-0x006127D8 BYTE_EXACT
// symbol    ?adjust_edit_group1_pass_dialog_focus@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?pass_dialog_focus@thunk1_EditGroup@@QAEXXZ)(EditGroup* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_edit_group1_pass_dialog_focus(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<EditGroup *>(object - vtordisp)->EditGroup::pass_dialog_focus();
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::attach expects, then forward unchanged.
// ORIGINAL: 0x00612810 ?attach@thunk1_EditGroup@@QAEHPAUGraphicWin@@HHH@Z 0x00612810-0x00612818
// symbol    ?adjust_edit_group1_attach@@YIHPAX00HHH@Z
// size      8 bytes
// prototype int (__thiscall ?attach@thunk1_EditGroup@@QAEHPAUGraphicWin@@HHH@Z)(EditGroup* this, GraphicWin*, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_edit_group1_attach(void *self, void *, void *arg0,
                                         int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return reinterpret_cast<EditGroup *>(object - vtordisp)->EditGroup::attach(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk3 subobject to the one
         EditGroup::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00612820 ?on_redraw@thunk3_EditGroup@@QAEXXZ 0x00612820-0x0061282E BYTE_EXACT
// symbol    ?adjust_edit_group3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_EditGroup@@QAEXXZ)(EditGroup* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_edit_group3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    reinterpret_cast<EditGroup *>(object - vtordisp - 2584)->EditGroup::on_redraw();
}
