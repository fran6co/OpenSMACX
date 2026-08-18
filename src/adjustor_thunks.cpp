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
func_adjustor_i_piii CheckBoxAttachTarget =
    original_method<func_adjustor_i_piii>(0x0060E800);
func_adjustor_v_i CheckBoxOnDialogFocusTarget =
    original_method<func_adjustor_v_i>(0x0060FB90);
func_adjustor_i_i CheckBoxOnKeyDownTarget =
    original_method<func_adjustor_i_i>(0x0060F8B0);
func_adjustor_v_ii CheckBoxOnLeftDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x0060FA80);
func_adjustor_v_ii CheckBoxOnLeftDownTarget =
    original_method<func_adjustor_v_ii>(0x0060F700);
func_adjustor_v_ii CheckBoxOnMouseLeaveTarget =
    original_method<func_adjustor_v_ii>(0x0060FC30);
func_adjustor_v_ii CheckBoxOnMouseMoveTarget =
    original_method<func_adjustor_v_ii>(0x0060FBB0);
func_adjustor_v CheckBoxOnRedrawTarget =
    original_method<func_adjustor_v>(0x0060FAB0);
func_adjustor_i_ii ConsoleOnKeyClickTarget =
    original_method<func_adjustor_i_ii>(0x005178C0);
func_adjustor_v_ii ConsoleOnNcHittestTarget =
    original_method<func_adjustor_v_ii>(0x0050F680);
func_adjustor_v_ii ConsoleOnNcLeftDownTarget =
    original_method<func_adjustor_v_ii>(0x0050FB10);
func_adjustor_v_ii ConsoleOnNcLeftUpTarget =
    original_method<func_adjustor_v_ii>(0x0050FE70);
func_adjustor_v_ii ConsoleOnNcMouseMoveTarget =
    original_method<func_adjustor_v_ii>(0x00510110);
func_adjustor_v_pi ConsoleOnPostRedrawNcBufferTarget =
    original_method<func_adjustor_v_pi>(0x0050F960);
func_adjustor_v ConsoleOnSysCloseTarget =
    original_method<func_adjustor_v>(0x0051D7C0);
func_adjustor_p_u ConsoleScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x0051DDF0);
func_adjustor_i_piii DialogsAttachTarget =
    original_method<func_adjustor_i_piii>(0x00612FE0);
func_adjustor_v_i DialogsOnDialogFocusTarget =
    original_method<func_adjustor_v_i>(0x006130E0);
func_adjustor_i_i DialogsOnKeyDownTarget =
    original_method<func_adjustor_i_i>(0x00612CC0);
func_adjustor_v_ii DialogsOnLeftClickTarget =
    original_method<func_adjustor_v_ii>(0x00612E80);
func_adjustor_v_ii DialogsOnLeftDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x00612D60);
func_adjustor_v_ii DialogsOnLeftDownTarget =
    original_method<func_adjustor_v_ii>(0x00612C20);
func_adjustor_v_ii DialogsOnLeftUpTarget =
    original_method<func_adjustor_v_ii>(0x00612F10);
func_adjustor_v_ii DialogsOnMouseLeaveTarget =
    original_method<func_adjustor_v_ii>(0x00612AE0);
func_adjustor_v_ii DialogsOnMouseMoveTarget =
    original_method<func_adjustor_v_ii>(0x00612B80);
func_adjustor_v_i DialogsOnMousewheelTarget =
    original_method<func_adjustor_v_i>(0x00612FC0);
func_adjustor_v DialogsOnRedrawTarget =
    original_method<func_adjustor_v>(0x00612E00);
func_adjustor_v_ii DialogsOnRightClickTarget =
    original_method<func_adjustor_v_ii>(0x00612F60);
func_adjustor_v_ii DialogsOnRightDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x00612EF0);
func_adjustor_v_ii DialogsOnRightDownTarget =
    original_method<func_adjustor_v_ii>(0x00612ED0);
func_adjustor_v_ii DialogsOnRightUpTarget =
    original_method<func_adjustor_v_ii>(0x00612F40);
func_adjustor_v DialogsOnScrollCreateTarget =
    original_method<func_adjustor_v>(0x00613220);
func_adjustor_i_p DialogsOnScrollDeleteTarget =
    original_method<func_adjustor_i_p>(0x00613260);
func_adjustor_v_ii DialogsOnScrolledTarget =
    original_method<func_adjustor_v_ii>(0x00612F80);
func_adjustor_v_ii DialogsOnScrollingTarget =
    original_method<func_adjustor_v_ii>(0x00612FA0);
func_adjustor_v DialogsPassDialogFocusTarget =
    original_method<func_adjustor_v>(0x00613180);
func_adjustor_p_u DialogsScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x00407100);
func_adjustor_i_piii EditGroupAttachTarget =
    original_method<func_adjustor_i_piii>(0x00611AF0);
func_adjustor_v_i EditGroupOnDialogFocusTarget =
    original_method<func_adjustor_v_i>(0x00612670);
func_adjustor_v EditGroupOnRedrawTarget =
    original_method<func_adjustor_v>(0x00612450);
func_adjustor_v EditGroupPassDialogFocusTarget =
    original_method<func_adjustor_v>(0x006126C0);
func_adjustor_i_piii ListBoxAttachTarget =
    original_method<func_adjustor_i_piii>(0x0060A670);
func_adjustor_v_i ListBoxOnDialogFocusTarget =
    original_method<func_adjustor_v_i>(0x0060C7C0);
func_adjustor_v_i ListBoxOnKeyDownTarget =
    original_method<func_adjustor_v_i>(0x0060AF90);
func_adjustor_v_ii ListBoxOnLeftDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x0060C710);
func_adjustor_v_ii ListBoxOnLeftDownTarget =
    original_method<func_adjustor_v_ii>(0x0060AA60);
func_adjustor_v_ii ListBoxOnMouseLeaveTarget =
    original_method<func_adjustor_v_ii>(0x0060CE10);
func_adjustor_v_ii ListBoxOnMouseMoveTarget =
    original_method<func_adjustor_v_ii>(0x0060CBC0);
func_adjustor_v_i ListBoxOnMousewheelTarget =
    original_method<func_adjustor_v_i>(0x0060CB70);
func_adjustor_v ListBoxOnRedrawTarget =
    original_method<func_adjustor_v>(0x0060C350);
func_adjustor_v_ii ListBoxOnRightDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x0060C6D0);
func_adjustor_v_ii ListBoxOnRightDownTarget =
    original_method<func_adjustor_v_ii>(0x0060AA20);
func_adjustor_v_ii ListBoxOnScrolledTarget =
    original_method<func_adjustor_v_ii>(0x0060C6A0);
func_adjustor_v_ii ListBoxOnScrollingTarget =
    original_method<func_adjustor_v_ii>(0x0060C5D0);
func_adjustor_v_i MapWinOnButtonClickedTarget =
    original_method<func_adjustor_v_i>(0x0046F8C0);
func_adjustor_v_ii MapWinOnLeftClickTarget =
    original_method<func_adjustor_v_ii>(0x0046EBA0);
func_adjustor_v_ii MapWinOnLeftDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x0046EBD0);
func_adjustor_v_ii MapWinOnLeftDownTarget =
    original_method<func_adjustor_v_ii>(0x0046ED30);
func_adjustor_v_ii MapWinOnLeftUpTarget =
    original_method<func_adjustor_v_ii>(0x0046EFF0);
func_adjustor_v MapWinOnLoseMouseCaptureTarget =
    original_method<func_adjustor_v>(0x0046EB90);
func_adjustor_v_ii MapWinOnMouseMoveTarget =
    original_method<func_adjustor_v_ii>(0x0046F000);
func_adjustor_v_ii MapWinOnNcHittestTarget =
    original_method<func_adjustor_v_ii>(0x0046F660);
func_adjustor_v_ii MapWinOnNcLeftDownTarget =
    original_method<func_adjustor_v_ii>(0x0046F700);
func_adjustor_v MapWinOnRedrawTarget =
    original_method<func_adjustor_v>(0x0046B1D0);
func_adjustor_v_ii MapWinOnResizeTarget =
    original_method<func_adjustor_v_ii>(0x0046FA00);
func_adjustor_v_ii MapWinOnRightClickTarget =
    original_method<func_adjustor_v_ii>(0x0046EBE0);
func_adjustor_v_ii MapWinOnRightDownTarget =
    original_method<func_adjustor_v_ii>(0x0046EC10);
func_adjustor_v MapWinOnSysCloseTarget =
    original_method<func_adjustor_v>(0x0046F880);
func_adjustor_p_u MapWinScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x00421830);
func_adjustor_v PlanWinOnRedrawTarget =
    original_method<func_adjustor_v>(0x0048AF30);
func_adjustor_p_u PlanWinScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x0048BF20);
func_adjustor_i_piii RadioButtonAttachTarget =
    original_method<func_adjustor_i_piii>(0x0060D1F0);
func_adjustor_v_i RadioButtonDtorTarget =
    original_method<func_adjustor_v_i>(0x00406F60);
func_adjustor_v_i RadioButtonOnDialogFocusTarget =
    original_method<func_adjustor_v_i>(0x0060E300);
func_adjustor_i_i RadioButtonOnKeyDownTarget =
    original_method<func_adjustor_i_i>(0x0060E020);
func_adjustor_v_ii RadioButtonOnLeftDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x0060E1E0);
func_adjustor_v_ii RadioButtonOnLeftDownTarget =
    original_method<func_adjustor_v_ii>(0x0060DE10);
func_adjustor_v_ii RadioButtonOnMouseLeaveTarget =
    original_method<func_adjustor_v_ii>(0x0060E4A0);
func_adjustor_v_ii RadioButtonOnMouseMoveTarget =
    original_method<func_adjustor_v_ii>(0x0060E320);
func_adjustor_v RadioButtonOnRedrawTarget =
    original_method<func_adjustor_v>(0x0060E220);
func_adjustor_i_piii SpriteBoxAttachTarget =
    original_method<func_adjustor_i_piii>(0x00610540);
func_adjustor_v_i SpriteBoxOnDialogFocusTarget =
    original_method<func_adjustor_v_i>(0x006115E0);
func_adjustor_i_i SpriteBoxOnKeyDownTarget =
    original_method<func_adjustor_i_i>(0x00611400);
func_adjustor_v_ii SpriteBoxOnLeftClickTarget =
    original_method<func_adjustor_v_ii>(0x00611060);
func_adjustor_v_ii SpriteBoxOnLeftDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x006112E0);
func_adjustor_v_ii SpriteBoxOnLeftDownTarget =
    original_method<func_adjustor_v_ii>(0x00611150);
func_adjustor_v_ii SpriteBoxOnLeftUpTarget =
    original_method<func_adjustor_v_ii>(0x006111A0);
func_adjustor_v_ii SpriteBoxOnMouseLeaveTarget =
    original_method<func_adjustor_v_ii>(0x00611050);
func_adjustor_v_ii SpriteBoxOnMouseMoveTarget =
    original_method<func_adjustor_v_ii>(0x00611040);
func_adjustor_v SpriteBoxOnRedrawTarget =
    original_method<func_adjustor_v>(0x00611380);
func_adjustor_v_ii SpriteBoxOnRightClickTarget =
    original_method<func_adjustor_v_ii>(0x006111F0);
func_adjustor_v_ii SpriteBoxOnRightDoubleClickTarget =
    original_method<func_adjustor_v_ii>(0x00611330);
func_adjustor_v_ii SpriteBoxOnRightDownTarget =
    original_method<func_adjustor_v_ii>(0x00611240);
func_adjustor_v_ii SpriteBoxOnRightUpTarget =
    original_method<func_adjustor_v_ii>(0x00611290);

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::dtor expects, then forward unchanged.
// ORIGINAL: 0x00406F50 ??1thunk1_RadioButton@@QAE@H@Z 0x00406F50-0x00406F58
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
// ORIGINAL: 0x00406FF0 ?on_redraw@thunk1_RadioButton@@QAEXXZ 0x00406FF0-0x00406FF8
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
    (ORIGINAL(object - vtordisp)->*RadioButtonOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x00407000 ?on_mouse_move@thunk1_RadioButton@@QAEXHH@Z 0x00407000-0x00407008
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
    (ORIGINAL(object - vtordisp)->*RadioButtonOnMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x00407010 ?on_mouse_leave@thunk1_RadioButton@@QAEXHH@Z 0x00407010-0x00407018
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
    (ORIGINAL(object - vtordisp)->*RadioButtonOnMouseLeaveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x00407020 ?on_left_down@thunk1_RadioButton@@QAEXHH@Z 0x00407020-0x00407028
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
    (ORIGINAL(object - vtordisp)->*RadioButtonOnLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x00407030 ?on_key_down@thunk1_RadioButton@@QAEHH@Z 0x00407030-0x00407038
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
    return (ORIGINAL(object - vtordisp)->*RadioButtonOnKeyDownTarget)(arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x00407040 ?on_left_double_click@thunk1_RadioButton@@QAEXHH@Z 0x00407040-0x00407048
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
    (ORIGINAL(object - vtordisp)->*RadioButtonOnLeftDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x00407050 ?on_dialog_focus@thunk1_RadioButton@@QAEXH@Z 0x00407050-0x00407058
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
    (ORIGINAL(object - vtordisp)->*RadioButtonOnDialogFocusTarget)(arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk2 subobject to the one
         RadioButton::dtor expects, then forward unchanged.
// ORIGINAL: 0x00407060 ??1thunk2_RadioButton@@QAE@H@Z 0x00407060-0x00407071
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
    return (ORIGINAL(object - vtordisp)->*RadioButtonAttachTarget)(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk3 subobject to the one
         RadioButton::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x004070A0 ?on_redraw@thunk3_RadioButton@@QAEXXZ 0x004070A0-0x004070AE
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
    (ORIGINAL(object - vtordisp - 2584)->*RadioButtonOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x004070F0 ??_Gthunk1_Dialogs@@UAEPAXI@Z 0x004070F0-0x004070F8
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
// ORIGINAL: 0x00407150 ?on_scroll_create@thunk1_Dialogs@@QAEXXZ 0x00407150-0x00407158
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
    (ORIGINAL(object - vtordisp)->*DialogsOnScrollCreateTarget)();
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scroll_delete expects, then forward unchanged.
// ORIGINAL: 0x00407160 ?on_scroll_delete@thunk1_Dialogs@@QAEHPAUScroll@@@Z 0x00407160-0x00407168
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
    return (ORIGINAL(object - vtordisp)->*DialogsOnScrollDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00407170 ?on_redraw@thunk1_Dialogs@@QAEXXZ 0x00407170-0x00407178
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
    (ORIGINAL(object - vtordisp)->*DialogsOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mousewheel expects, then forward unchanged.
// ORIGINAL: 0x00407180 ?on_mousewheel@thunk1_Dialogs@@QAEXH@Z 0x00407180-0x00407188
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
    (ORIGINAL(object - vtordisp)->*DialogsOnMousewheelTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x00407190 ?on_mouse_move@thunk1_Dialogs@@QAEXHH@Z 0x00407190-0x00407198
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
    (ORIGINAL(object - vtordisp)->*DialogsOnMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x004071A0 ?on_mouse_leave@thunk1_Dialogs@@QAEXHH@Z 0x004071A0-0x004071A8
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
    (ORIGINAL(object - vtordisp)->*DialogsOnMouseLeaveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x004071B0 ?on_left_click@thunk1_Dialogs@@QAEXHH@Z 0x004071B0-0x004071B8
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
    (ORIGINAL(object - vtordisp)->*DialogsOnLeftClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x004071C0 ?on_left_down@thunk1_Dialogs@@QAEXHH@Z 0x004071C0-0x004071C8
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
    (ORIGINAL(object - vtordisp)->*DialogsOnLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x004071D0 ?on_left_up@thunk1_Dialogs@@QAEXHH@Z 0x004071D0-0x004071D8
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
    (ORIGINAL(object - vtordisp)->*DialogsOnLeftUpTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x004071E0 ?on_right_click@thunk1_Dialogs@@QAEXHH@Z 0x004071E0-0x004071E8
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
    (ORIGINAL(object - vtordisp)->*DialogsOnRightClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x004071F0 ?on_right_down@thunk1_Dialogs@@QAEXHH@Z 0x004071F0-0x004071F8
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
    (ORIGINAL(object - vtordisp)->*DialogsOnRightDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_up expects, then forward unchanged.
// ORIGINAL: 0x00407200 ?on_right_up@thunk1_Dialogs@@QAEXHH@Z 0x00407200-0x00407208
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
    (ORIGINAL(object - vtordisp)->*DialogsOnRightUpTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x00407210 ?on_key_down@thunk1_Dialogs@@QAEHH@Z 0x00407210-0x00407218
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
    return (ORIGINAL(object - vtordisp)->*DialogsOnKeyDownTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x00407220 ?on_left_double_click@thunk1_Dialogs@@QAEXHH@Z 0x00407220-0x00407228
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
    (ORIGINAL(object - vtordisp)->*DialogsOnLeftDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_double_click expects, then forward unchanged.
// ORIGINAL: 0x00407230 ?on_right_double_click@thunk1_Dialogs@@QAEXHH@Z 0x00407230-0x00407238
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
    (ORIGINAL(object - vtordisp)->*DialogsOnRightDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scrolled expects, then forward unchanged.
// ORIGINAL: 0x00407240 ?on_scrolled@thunk1_Dialogs@@QAEXHH@Z 0x00407240-0x00407248
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
    (ORIGINAL(object - vtordisp)->*DialogsOnScrolledTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scrolling expects, then forward unchanged.
// ORIGINAL: 0x00407250 ?on_scrolling@thunk1_Dialogs@@QAEXHH@Z 0x00407250-0x00407258
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
    (ORIGINAL(object - vtordisp)->*DialogsOnScrollingTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x00407260 ?on_dialog_focus@thunk1_Dialogs@@QAEXH@Z 0x00407260-0x00407268
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
    (ORIGINAL(object - vtordisp)->*DialogsOnDialogFocusTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::pass_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x00407270 ?pass_dialog_focus@thunk1_Dialogs@@QAEXXZ 0x00407270-0x00407278
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
    (ORIGINAL(object - vtordisp)->*DialogsPassDialogFocusTarget)();
}

/*
Purpose: Adjust the receiver from Dialogs's thunk2 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x00407280 ??_Gthunk2_Dialogs@@UAEPAXI@Z 0x00407280-0x00407291
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
    return (ORIGINAL(object - vtordisp)->*DialogsAttachTarget)(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk3 subobject to the one
         Dialogs::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x004072C0 ?on_redraw@thunk3_Dialogs@@QAEXXZ 0x004072C0-0x004072CE
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
    (ORIGINAL(object - vtordisp - 2584)->*DialogsOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x00421820 ??_Gthunk1_MapWin@@UAEPAXI@Z 0x00421820-0x00421828
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
// ORIGINAL: 0x00421870 ?on_lose_mouse_capture@thunk1_MapWin@@QAEXXZ 0x00421870-0x00421878
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
    (ORIGINAL(object - vtordisp)->*MapWinOnLoseMouseCaptureTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00421880 ?on_redraw@thunk1_MapWin@@QAEXXZ 0x00421880-0x00421888
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
    (ORIGINAL(object - vtordisp)->*MapWinOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x00421890 ?on_resize@thunk1_MapWin@@QAEXHH@Z 0x00421890-0x00421898
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
    (ORIGINAL(object - vtordisp)->*MapWinOnResizeTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_sys_close expects, then forward unchanged.
// ORIGINAL: 0x004218A0 ?on_sys_close@thunk1_MapWin@@QAEXXZ 0x004218A0-0x004218A8
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
    (ORIGINAL(object - vtordisp)->*MapWinOnSysCloseTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x004218B0 ?on_mouse_move@thunk1_MapWin@@QAEXHH@Z 0x004218B0-0x004218B8
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
    (ORIGINAL(object - vtordisp)->*MapWinOnMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x004218C0 ?on_left_click@thunk1_MapWin@@QAEXHH@Z 0x004218C0-0x004218C8
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
    (ORIGINAL(object - vtordisp)->*MapWinOnLeftClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x004218D0 ?on_left_down@thunk1_MapWin@@QAEXHH@Z 0x004218D0-0x004218D8
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
    (ORIGINAL(object - vtordisp)->*MapWinOnLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x004218E0 ?on_left_up@thunk1_MapWin@@QAEXHH@Z 0x004218E0-0x004218E8
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
    (ORIGINAL(object - vtordisp)->*MapWinOnLeftUpTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x004218F0 ?on_right_click@thunk1_MapWin@@QAEXHH@Z 0x004218F0-0x004218F8
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
    (ORIGINAL(object - vtordisp)->*MapWinOnRightClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x00421900 ?on_right_down@thunk1_MapWin@@QAEXHH@Z 0x00421900-0x00421908
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
    (ORIGINAL(object - vtordisp)->*MapWinOnRightDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x00421910 ?on_left_double_click@thunk1_MapWin@@QAEXHH@Z 0x00421910-0x00421918
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
    (ORIGINAL(object - vtordisp)->*MapWinOnLeftDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_nc_left_down expects, then forward unchanged.
// ORIGINAL: 0x00421920 ?on_nc_left_down@thunk1_MapWin@@QAEXHH@Z 0x00421920-0x00421928
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
    (ORIGINAL(object - vtordisp)->*MapWinOnNcLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
// ORIGINAL: 0x00421930 ?on_button_clicked@thunk1_MapWin@@QAEXH@Z 0x00421930-0x00421938
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
    (ORIGINAL(object - vtordisp)->*MapWinOnButtonClickedTarget)(arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_nc_hittest expects, then forward unchanged.
// ORIGINAL: 0x00421940 ?on_nc_hittest@thunk1_MapWin@@QAEXHH@Z 0x00421940-0x00421948
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
    (ORIGINAL(object - vtordisp)->*MapWinOnNcHittestTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk2 subobject to the one
         MapWin::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x00421950 ??_Gthunk2_MapWin@@UAEPAXI@Z 0x00421950-0x00421961
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
// ORIGINAL: 0x0048BF60 ?on_lose_mouse_capture@thunk5_MapWin@@QAEXXZ 0x0048BF60-0x0048BF6E
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnLoseMouseCaptureTarget)();
}

/*
Purpose: Adjust the receiver from PlanWin's thunk1 subobject to the one
         PlanWin::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0048BF70 ?on_redraw@thunk1_PlanWin@@QAEXXZ 0x0048BF70-0x0048BF78
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
    (ORIGINAL(object - vtordisp)->*PlanWinOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x0048BF80 ?on_resize@thunk5_MapWin@@QAEXHH@Z 0x0048BF80-0x0048BF8E
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnResizeTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_sys_close expects, then forward unchanged.
// ORIGINAL: 0x0048BF90 ?on_sys_close@thunk5_MapWin@@QAEXXZ 0x0048BF90-0x0048BF9E
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnSysCloseTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0048BFA0 ?on_mouse_move@thunk5_MapWin@@QAEXHH@Z 0x0048BFA0-0x0048BFAE
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x0048BFB0 ?on_left_click@thunk5_MapWin@@QAEXHH@Z 0x0048BFB0-0x0048BFBE
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnLeftClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x0048BFC0 ?on_left_down@thunk5_MapWin@@QAEXHH@Z 0x0048BFC0-0x0048BFCE
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x0048BFD0 ?on_left_up@thunk5_MapWin@@QAEXHH@Z 0x0048BFD0-0x0048BFDE
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnLeftUpTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x0048BFE0 ?on_right_click@thunk5_MapWin@@QAEXHH@Z 0x0048BFE0-0x0048BFEE
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnRightClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x0048BFF0 ?on_right_down@thunk5_MapWin@@QAEXHH@Z 0x0048BFF0-0x0048BFFE
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnRightDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x0048C000 ?on_left_double_click@thunk5_MapWin@@QAEXHH@Z 0x0048C000-0x0048C00E
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnLeftDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_nc_left_down expects, then forward unchanged.
// ORIGINAL: 0x0048C010 ?on_nc_left_down@thunk5_MapWin@@QAEXHH@Z 0x0048C010-0x0048C01E
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnNcLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
// ORIGINAL: 0x0048C020 ?on_button_clicked@thunk5_MapWin@@QAEXH@Z 0x0048C020-0x0048C02E
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnButtonClickedTarget)(arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_nc_hittest expects, then forward unchanged.
// ORIGINAL: 0x0048C030 ?on_nc_hittest@thunk5_MapWin@@QAEXHH@Z 0x0048C030-0x0048C03E
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
    (ORIGINAL(object - vtordisp - 1508)->*MapWinOnNcHittestTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from PlanWin's thunk2 subobject to the one
         PlanWin::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x0048C040 ??_Gthunk2_PlanWin@@UAEPAXI@Z 0x0048C040-0x0048C051
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
// ORIGINAL: 0x0051DE30 ?on_lose_mouse_capture@thunk4_MapWin@@QAEXXZ 0x0051DE30-0x0051DE3E
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnLoseMouseCaptureTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0051DE40 ?on_redraw@thunk4_MapWin@@QAEXXZ 0x0051DE40-0x0051DE4E
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x0051DE50 ?on_resize@thunk4_MapWin@@QAEXHH@Z 0x0051DE50-0x0051DE5E
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnResizeTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_sys_close expects, then forward unchanged.
// ORIGINAL: 0x0051DE60 ?on_sys_close@thunk1_Console@@QAEXXZ 0x0051DE60-0x0051DE68
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
    (ORIGINAL(object - vtordisp)->*ConsoleOnSysCloseTarget)();
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0051DE70 ?on_mouse_move@thunk4_MapWin@@QAEXHH@Z 0x0051DE70-0x0051DE7E
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x0051DE80 ?on_left_click@thunk4_MapWin@@QAEXHH@Z 0x0051DE80-0x0051DE8E
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnLeftClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x0051DE90 ?on_left_down@thunk4_MapWin@@QAEXHH@Z 0x0051DE90-0x0051DE9E
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x0051DEA0 ?on_left_up@thunk4_MapWin@@QAEXHH@Z 0x0051DEA0-0x0051DEAE
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnLeftUpTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x0051DEB0 ?on_right_click@thunk4_MapWin@@QAEXHH@Z 0x0051DEB0-0x0051DEBE
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnRightClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x0051DEC0 ?on_right_down@thunk4_MapWin@@QAEXHH@Z 0x0051DEC0-0x0051DECE
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnRightDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_key_click expects, then forward unchanged.
// ORIGINAL: 0x0051DED0 ?on_key_click@thunk1_Console@@QAEHHH@Z 0x0051DED0-0x0051DED8
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
    return (ORIGINAL(object - vtordisp)->*ConsoleOnKeyClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x0051DEE0 ?on_left_double_click@thunk4_MapWin@@QAEXHH@Z 0x0051DEE0-0x0051DEEE
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnLeftDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0051DEF0 ?on_nc_mouse_move@thunk1_Console@@QAEXHH@Z 0x0051DEF0-0x0051DEF8
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
    (ORIGINAL(object - vtordisp)->*ConsoleOnNcMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_left_down expects, then forward unchanged.
// ORIGINAL: 0x0051DF00 ?on_nc_left_down@thunk1_Console@@QAEXHH@Z 0x0051DF00-0x0051DF08
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
    (ORIGINAL(object - vtordisp)->*ConsoleOnNcLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_left_up expects, then forward unchanged.
// ORIGINAL: 0x0051DF10 ?on_nc_left_up@thunk1_Console@@QAEXHH@Z 0x0051DF10-0x0051DF18
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
    (ORIGINAL(object - vtordisp)->*ConsoleOnNcLeftUpTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
// ORIGINAL: 0x0051DF20 ?on_button_clicked@thunk4_MapWin@@QAEXH@Z 0x0051DF20-0x0051DF2E
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
    (ORIGINAL(object - vtordisp - 9000)->*MapWinOnButtonClickedTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_post_redraw_nc_buffer expects, then forward unchanged.
// ORIGINAL: 0x0051DF30 ?on_post_redraw_nc_buffer@thunk1_Console@@QAEXPAUBuffer@@H@Z 0x0051DF30-0x0051DF38
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
    (ORIGINAL(object - vtordisp)->*ConsoleOnPostRedrawNcBufferTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_hittest expects, then forward unchanged.
// ORIGINAL: 0x0051DF40 ?on_nc_hittest@thunk1_Console@@QAEXHH@Z 0x0051DF40-0x0051DF48
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
    (ORIGINAL(object - vtordisp)->*ConsoleOnNcHittestTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk2 subobject to the one
         Console::scalar_delete expects, then forward unchanged.
// ORIGINAL: 0x0051DF50 ??_Gthunk2_Console@@UAEPAXI@Z 0x0051DF50-0x0051DF61
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
// ORIGINAL: 0x0060CF50 ?on_redraw@thunk1_ListBox@@QAEXXZ 0x0060CF50-0x0060CF58
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mousewheel expects, then forward unchanged.
// ORIGINAL: 0x0060CF60 ?on_mousewheel@thunk1_ListBox@@QAEXH@Z 0x0060CF60-0x0060CF68
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnMousewheelTarget)(arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0060CF70 ?on_mouse_move@thunk1_ListBox@@QAEXHH@Z 0x0060CF70-0x0060CF78
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x0060CF80 ?on_mouse_leave@thunk1_ListBox@@QAEXHH@Z 0x0060CF80-0x0060CF88
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnMouseLeaveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x0060CF90 ?on_left_down@thunk1_ListBox@@QAEXHH@Z 0x0060CF90-0x0060CF98
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x0060CFA0 ?on_right_down@thunk1_ListBox@@QAEXHH@Z 0x0060CFA0-0x0060CFA8
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnRightDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x0060CFB0 ?on_key_down@thunk1_ListBox@@QAEXH@Z 0x0060CFB0-0x0060CFB8
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnKeyDownTarget)(arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x0060CFC0 ?on_left_double_click@thunk1_ListBox@@QAEXHH@Z 0x0060CFC0-0x0060CFC8
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnLeftDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_right_double_click expects, then forward unchanged.
// ORIGINAL: 0x0060CFD0 ?on_right_double_click@thunk1_ListBox@@QAEXHH@Z 0x0060CFD0-0x0060CFD8
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnRightDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_scrolled expects, then forward unchanged.
// ORIGINAL: 0x0060CFE0 ?on_scrolled@thunk1_ListBox@@QAEXHH@Z 0x0060CFE0-0x0060CFE8
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnScrolledTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_scrolling expects, then forward unchanged.
// ORIGINAL: 0x0060CFF0 ?on_scrolling@thunk1_ListBox@@QAEXHH@Z 0x0060CFF0-0x0060CFF8
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnScrollingTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x0060D000 ?on_dialog_focus@thunk1_ListBox@@QAEXH@Z 0x0060D000-0x0060D008
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
    (ORIGINAL(object - vtordisp)->*ListBoxOnDialogFocusTarget)(arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::attach expects, then forward unchanged.
// ORIGINAL: 0x0060D040 ?attach@thunk1_ListBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060D040-0x0060D048
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
    return (ORIGINAL(object - vtordisp)->*ListBoxAttachTarget)(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from ListBox's thunk3 subobject to the one
         ListBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0060D050 ?on_redraw@thunk3_ListBox@@QAEXXZ 0x0060D050-0x0060D05E
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
    (ORIGINAL(object - vtordisp - 2584)->*ListBoxOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0060FE40 ?on_redraw@thunk1_CheckBox@@QAEXXZ 0x0060FE40-0x0060FE48
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
    (ORIGINAL(object - vtordisp)->*CheckBoxOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0060FE50 ?on_mouse_move@thunk1_CheckBox@@QAEXHH@Z 0x0060FE50-0x0060FE58
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
    (ORIGINAL(object - vtordisp)->*CheckBoxOnMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x0060FE60 ?on_mouse_leave@thunk1_CheckBox@@QAEXHH@Z 0x0060FE60-0x0060FE68
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
    (ORIGINAL(object - vtordisp)->*CheckBoxOnMouseLeaveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x0060FE70 ?on_left_down@thunk1_CheckBox@@QAEXHH@Z 0x0060FE70-0x0060FE78
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
    (ORIGINAL(object - vtordisp)->*CheckBoxOnLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x0060FE80 ?on_key_down@thunk1_CheckBox@@QAEHH@Z 0x0060FE80-0x0060FE88
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
    return (ORIGINAL(object - vtordisp)->*CheckBoxOnKeyDownTarget)(arg0);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x0060FE90 ?on_left_double_click@thunk1_CheckBox@@QAEXHH@Z 0x0060FE90-0x0060FE98
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
    (ORIGINAL(object - vtordisp)->*CheckBoxOnLeftDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x0060FEA0 ?on_dialog_focus@thunk1_CheckBox@@QAEXH@Z 0x0060FEA0-0x0060FEA8
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
    (ORIGINAL(object - vtordisp)->*CheckBoxOnDialogFocusTarget)(arg0);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::attach expects, then forward unchanged.
// ORIGINAL: 0x0060FEE0 ?attach@thunk1_CheckBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060FEE0-0x0060FEE8
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
    return (ORIGINAL(object - vtordisp)->*CheckBoxAttachTarget)(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk3 subobject to the one
         CheckBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x0060FEF0 ?on_redraw@thunk3_CheckBox@@QAEXXZ 0x0060FEF0-0x0060FEFE
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
    (ORIGINAL(object - vtordisp - 2584)->*CheckBoxOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00611820 ?on_redraw@thunk1_SpriteBox@@QAEXXZ 0x00611820-0x00611828
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x00611830 ?on_mouse_move@thunk1_SpriteBox@@QAEXHH@Z 0x00611830-0x00611838
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnMouseMoveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x00611840 ?on_mouse_leave@thunk1_SpriteBox@@QAEXHH@Z 0x00611840-0x00611848
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnMouseLeaveTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_click expects, then forward unchanged.
// ORIGINAL: 0x00611850 ?on_left_click@thunk1_SpriteBox@@QAEXHH@Z 0x00611850-0x00611858
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnLeftClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x00611860 ?on_left_down@thunk1_SpriteBox@@QAEXHH@Z 0x00611860-0x00611868
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnLeftDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x00611870 ?on_left_up@thunk1_SpriteBox@@QAEXHH@Z 0x00611870-0x00611878
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnLeftUpTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_click expects, then forward unchanged.
// ORIGINAL: 0x00611880 ?on_right_click@thunk1_SpriteBox@@QAEXHH@Z 0x00611880-0x00611888
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnRightClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x00611890 ?on_right_down@thunk1_SpriteBox@@QAEXHH@Z 0x00611890-0x00611898
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnRightDownTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_up expects, then forward unchanged.
// ORIGINAL: 0x006118A0 ?on_right_up@thunk1_SpriteBox@@QAEXHH@Z 0x006118A0-0x006118A8
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnRightUpTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_key_down expects, then forward unchanged.
// ORIGINAL: 0x006118B0 ?on_key_down@thunk1_SpriteBox@@QAEHH@Z 0x006118B0-0x006118B8
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
    return (ORIGINAL(object - vtordisp)->*SpriteBoxOnKeyDownTarget)(arg0);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x006118C0 ?on_left_double_click@thunk1_SpriteBox@@QAEXHH@Z 0x006118C0-0x006118C8
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnLeftDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_double_click expects, then forward unchanged.
// ORIGINAL: 0x006118D0 ?on_right_double_click@thunk1_SpriteBox@@QAEXHH@Z 0x006118D0-0x006118D8
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnRightDoubleClickTarget)(arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x006118E0 ?on_dialog_focus@thunk1_SpriteBox@@QAEXH@Z 0x006118E0-0x006118E8
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
    (ORIGINAL(object - vtordisp)->*SpriteBoxOnDialogFocusTarget)(arg0);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::attach expects, then forward unchanged.
// ORIGINAL: 0x00611920 ?attach@thunk1_SpriteBox@@QAEHPAUGraphicWin@@HHH@Z 0x00611920-0x00611928
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
    return (ORIGINAL(object - vtordisp)->*SpriteBoxAttachTarget)(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk3 subobject to the one
         SpriteBox::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00611930 ?on_redraw@thunk3_SpriteBox@@QAEXXZ 0x00611930-0x0061193E
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
    (ORIGINAL(object - vtordisp - 2584)->*SpriteBoxOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x006127B0 ?on_redraw@thunk1_EditGroup@@QAEXXZ 0x006127B0-0x006127B8
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
    (ORIGINAL(object - vtordisp)->*EditGroupOnRedrawTarget)();
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x006127C0 ?on_dialog_focus@thunk1_EditGroup@@QAEXH@Z 0x006127C0-0x006127C8
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
    (ORIGINAL(object - vtordisp)->*EditGroupOnDialogFocusTarget)(arg0);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::pass_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x006127D0 ?pass_dialog_focus@thunk1_EditGroup@@QAEXXZ 0x006127D0-0x006127D8
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
    (ORIGINAL(object - vtordisp)->*EditGroupPassDialogFocusTarget)();
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::attach expects, then forward unchanged.
// ORIGINAL: 0x00612810 ?attach@thunk1_EditGroup@@QAEHPAUGraphicWin@@HHH@Z 0x00612810-0x00612818
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
    return (ORIGINAL(object - vtordisp)->*EditGroupAttachTarget)(arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk3 subobject to the one
         EditGroup::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00612820 ?on_redraw@thunk3_EditGroup@@QAEXXZ 0x00612820-0x0061282E
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
    (ORIGINAL(object - vtordisp - 2584)->*EditGroupOnRedrawTarget)();
}
