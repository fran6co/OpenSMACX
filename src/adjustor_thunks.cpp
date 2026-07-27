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
func_adjustor_i_piii *CheckBoxAttachTarget =
    (func_adjustor_i_piii *)0x0060E800;
func_adjustor_v_i *CheckBoxOnDialogFocusTarget =
    (func_adjustor_v_i *)0x0060FB90;
func_adjustor_i_i *CheckBoxOnKeyDownTarget =
    (func_adjustor_i_i *)0x0060F8B0;
func_adjustor_v_ii *CheckBoxOnLeftDoubleClickTarget =
    (func_adjustor_v_ii *)0x0060FA80;
func_adjustor_v_ii *CheckBoxOnLeftDownTarget =
    (func_adjustor_v_ii *)0x0060F700;
func_adjustor_v_ii *CheckBoxOnMouseLeaveTarget =
    (func_adjustor_v_ii *)0x0060FC30;
func_adjustor_v_ii *CheckBoxOnMouseMoveTarget =
    (func_adjustor_v_ii *)0x0060FBB0;
func_adjustor_v *CheckBoxOnRedrawTarget =
    (func_adjustor_v *)0x0060FAB0;
func_adjustor_i_ii *ConsoleOnKeyClickTarget =
    (func_adjustor_i_ii *)0x005178C0;
func_adjustor_v_ii *ConsoleOnNcHittestTarget =
    (func_adjustor_v_ii *)0x0050F680;
func_adjustor_v_ii *ConsoleOnNcLeftDownTarget =
    (func_adjustor_v_ii *)0x0050FB10;
func_adjustor_v_ii *ConsoleOnNcLeftUpTarget =
    (func_adjustor_v_ii *)0x0050FE70;
func_adjustor_v_ii *ConsoleOnNcMouseMoveTarget =
    (func_adjustor_v_ii *)0x00510110;
func_adjustor_v_pi *ConsoleOnPostRedrawNcBufferTarget =
    (func_adjustor_v_pi *)0x0050F960;
func_adjustor_v *ConsoleOnSysCloseTarget =
    (func_adjustor_v *)0x0051D7C0;
func_adjustor_p_u *ConsoleScalarDeleteTarget =
    (func_adjustor_p_u *)0x0051DDF0;
func_adjustor_i_piii *DialogsAttachTarget =
    (func_adjustor_i_piii *)0x00612FE0;
func_adjustor_v_i *DialogsOnDialogFocusTarget =
    (func_adjustor_v_i *)0x006130E0;
func_adjustor_i_i *DialogsOnKeyDownTarget =
    (func_adjustor_i_i *)0x00612CC0;
func_adjustor_v_ii *DialogsOnLeftClickTarget =
    (func_adjustor_v_ii *)0x00612E80;
func_adjustor_v_ii *DialogsOnLeftDoubleClickTarget =
    (func_adjustor_v_ii *)0x00612D60;
func_adjustor_v_ii *DialogsOnLeftDownTarget =
    (func_adjustor_v_ii *)0x00612C20;
func_adjustor_v_ii *DialogsOnLeftUpTarget =
    (func_adjustor_v_ii *)0x00612F10;
func_adjustor_v_ii *DialogsOnMouseLeaveTarget =
    (func_adjustor_v_ii *)0x00612AE0;
func_adjustor_v_ii *DialogsOnMouseMoveTarget =
    (func_adjustor_v_ii *)0x00612B80;
func_adjustor_v_i *DialogsOnMousewheelTarget =
    (func_adjustor_v_i *)0x00612FC0;
func_adjustor_v *DialogsOnRedrawTarget =
    (func_adjustor_v *)0x00612E00;
func_adjustor_v_ii *DialogsOnRightClickTarget =
    (func_adjustor_v_ii *)0x00612F60;
func_adjustor_v_ii *DialogsOnRightDoubleClickTarget =
    (func_adjustor_v_ii *)0x00612EF0;
func_adjustor_v_ii *DialogsOnRightDownTarget =
    (func_adjustor_v_ii *)0x00612ED0;
func_adjustor_v_ii *DialogsOnRightUpTarget =
    (func_adjustor_v_ii *)0x00612F40;
func_adjustor_v *DialogsOnScrollCreateTarget =
    (func_adjustor_v *)0x00613220;
func_adjustor_i_p *DialogsOnScrollDeleteTarget =
    (func_adjustor_i_p *)0x00613260;
func_adjustor_v_ii *DialogsOnScrolledTarget =
    (func_adjustor_v_ii *)0x00612F80;
func_adjustor_v_ii *DialogsOnScrollingTarget =
    (func_adjustor_v_ii *)0x00612FA0;
func_adjustor_v *DialogsPassDialogFocusTarget =
    (func_adjustor_v *)0x00613180;
func_adjustor_p_u *DialogsScalarDeleteTarget =
    (func_adjustor_p_u *)0x00407100;
func_adjustor_i_piii *EditGroupAttachTarget =
    (func_adjustor_i_piii *)0x00611AF0;
func_adjustor_v_i *EditGroupOnDialogFocusTarget =
    (func_adjustor_v_i *)0x00612670;
func_adjustor_v *EditGroupOnRedrawTarget =
    (func_adjustor_v *)0x00612450;
func_adjustor_v *EditGroupPassDialogFocusTarget =
    (func_adjustor_v *)0x006126C0;
func_adjustor_i_piii *ListBoxAttachTarget =
    (func_adjustor_i_piii *)0x0060A670;
func_adjustor_v_i *ListBoxOnDialogFocusTarget =
    (func_adjustor_v_i *)0x0060C7C0;
func_adjustor_v_i *ListBoxOnKeyDownTarget =
    (func_adjustor_v_i *)0x0060AF90;
func_adjustor_v_ii *ListBoxOnLeftDoubleClickTarget =
    (func_adjustor_v_ii *)0x0060C710;
func_adjustor_v_ii *ListBoxOnLeftDownTarget =
    (func_adjustor_v_ii *)0x0060AA60;
func_adjustor_v_ii *ListBoxOnMouseLeaveTarget =
    (func_adjustor_v_ii *)0x0060CE10;
func_adjustor_v_ii *ListBoxOnMouseMoveTarget =
    (func_adjustor_v_ii *)0x0060CBC0;
func_adjustor_v_i *ListBoxOnMousewheelTarget =
    (func_adjustor_v_i *)0x0060CB70;
func_adjustor_v *ListBoxOnRedrawTarget =
    (func_adjustor_v *)0x0060C350;
func_adjustor_v_ii *ListBoxOnRightDoubleClickTarget =
    (func_adjustor_v_ii *)0x0060C6D0;
func_adjustor_v_ii *ListBoxOnRightDownTarget =
    (func_adjustor_v_ii *)0x0060AA20;
func_adjustor_v_ii *ListBoxOnScrolledTarget =
    (func_adjustor_v_ii *)0x0060C6A0;
func_adjustor_v_ii *ListBoxOnScrollingTarget =
    (func_adjustor_v_ii *)0x0060C5D0;
func_adjustor_v_i *MapWinOnButtonClickedTarget =
    (func_adjustor_v_i *)0x0046F8C0;
func_adjustor_v_ii *MapWinOnLeftClickTarget =
    (func_adjustor_v_ii *)0x0046EBA0;
func_adjustor_v_ii *MapWinOnLeftDoubleClickTarget =
    (func_adjustor_v_ii *)0x0046EBD0;
func_adjustor_v_ii *MapWinOnLeftDownTarget =
    (func_adjustor_v_ii *)0x0046ED30;
func_adjustor_v_ii *MapWinOnLeftUpTarget =
    (func_adjustor_v_ii *)0x0046EFF0;
func_adjustor_v *MapWinOnLoseMouseCaptureTarget =
    (func_adjustor_v *)0x0046EB90;
func_adjustor_v_ii *MapWinOnMouseMoveTarget =
    (func_adjustor_v_ii *)0x0046F000;
func_adjustor_v_ii *MapWinOnNcHittestTarget =
    (func_adjustor_v_ii *)0x0046F660;
func_adjustor_v_ii *MapWinOnNcLeftDownTarget =
    (func_adjustor_v_ii *)0x0046F700;
func_adjustor_v *MapWinOnRedrawTarget =
    (func_adjustor_v *)0x0046B1D0;
func_adjustor_v_ii *MapWinOnResizeTarget =
    (func_adjustor_v_ii *)0x0046FA00;
func_adjustor_v_ii *MapWinOnRightClickTarget =
    (func_adjustor_v_ii *)0x0046EBE0;
func_adjustor_v_ii *MapWinOnRightDownTarget =
    (func_adjustor_v_ii *)0x0046EC10;
func_adjustor_v *MapWinOnSysCloseTarget =
    (func_adjustor_v *)0x0046F880;
func_adjustor_p_u *MapWinScalarDeleteTarget =
    (func_adjustor_p_u *)0x00421830;
func_adjustor_v *PlanWinOnRedrawTarget =
    (func_adjustor_v *)0x0048AF30;
func_adjustor_p_u *PlanWinScalarDeleteTarget =
    (func_adjustor_p_u *)0x0048BF20;
func_adjustor_i_piii *RadioButtonAttachTarget =
    (func_adjustor_i_piii *)0x0060D1F0;
func_adjustor_v_i *RadioButtonDtorTarget =
    (func_adjustor_v_i *)0x00406F60;
func_adjustor_v_i *RadioButtonOnDialogFocusTarget =
    (func_adjustor_v_i *)0x0060E300;
func_adjustor_i_i *RadioButtonOnKeyDownTarget =
    (func_adjustor_i_i *)0x0060E020;
func_adjustor_v_ii *RadioButtonOnLeftDoubleClickTarget =
    (func_adjustor_v_ii *)0x0060E1E0;
func_adjustor_v_ii *RadioButtonOnLeftDownTarget =
    (func_adjustor_v_ii *)0x0060DE10;
func_adjustor_v_ii *RadioButtonOnMouseLeaveTarget =
    (func_adjustor_v_ii *)0x0060E4A0;
func_adjustor_v_ii *RadioButtonOnMouseMoveTarget =
    (func_adjustor_v_ii *)0x0060E320;
func_adjustor_v *RadioButtonOnRedrawTarget =
    (func_adjustor_v *)0x0060E220;
func_adjustor_i_piii *SpriteBoxAttachTarget =
    (func_adjustor_i_piii *)0x00610540;
func_adjustor_v_i *SpriteBoxOnDialogFocusTarget =
    (func_adjustor_v_i *)0x006115E0;
func_adjustor_i_i *SpriteBoxOnKeyDownTarget =
    (func_adjustor_i_i *)0x00611400;
func_adjustor_v_ii *SpriteBoxOnLeftClickTarget =
    (func_adjustor_v_ii *)0x00611060;
func_adjustor_v_ii *SpriteBoxOnLeftDoubleClickTarget =
    (func_adjustor_v_ii *)0x006112E0;
func_adjustor_v_ii *SpriteBoxOnLeftDownTarget =
    (func_adjustor_v_ii *)0x00611150;
func_adjustor_v_ii *SpriteBoxOnLeftUpTarget =
    (func_adjustor_v_ii *)0x006111A0;
func_adjustor_v_ii *SpriteBoxOnMouseLeaveTarget =
    (func_adjustor_v_ii *)0x00611050;
func_adjustor_v_ii *SpriteBoxOnMouseMoveTarget =
    (func_adjustor_v_ii *)0x00611040;
func_adjustor_v *SpriteBoxOnRedrawTarget =
    (func_adjustor_v *)0x00611380;
func_adjustor_v_ii *SpriteBoxOnRightClickTarget =
    (func_adjustor_v_ii *)0x006111F0;
func_adjustor_v_ii *SpriteBoxOnRightDoubleClickTarget =
    (func_adjustor_v_ii *)0x00611330;
func_adjustor_v_ii *SpriteBoxOnRightDownTarget =
    (func_adjustor_v_ii *)0x00611240;
func_adjustor_v_ii *SpriteBoxOnRightUpTarget =
    (func_adjustor_v_ii *)0x00611290;

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::dtor expects, then forward unchanged.
Original Offset: 00406F50
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_dtor(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonDtorTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_redraw expects, then forward unchanged.
Original Offset: 00406FF0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonOnRedrawTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_mouse_move expects, then forward unchanged.
Original Offset: 00407000
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_mouse_move(void *self, void *,
                                                   int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonOnMouseMoveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_mouse_leave expects, then forward unchanged.
Original Offset: 00407010
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_mouse_leave(void *self, void *,
                                                    int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonOnMouseLeaveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_left_down expects, then forward unchanged.
Original Offset: 00407020
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_left_down(void *self, void *,
                                                  int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonOnLeftDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_key_down expects, then forward unchanged.
Original Offset: 00407030
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_radio_button1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return RadioButtonOnKeyDownTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_left_double_click expects, then forward unchanged.
Original Offset: 00407040
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_left_double_click(void *self, void *,
                                                          int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonOnLeftDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_dialog_focus expects, then forward unchanged.
Original Offset: 00407050
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button1_on_dialog_focus(void *self, void *,
                                                     int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonOnDialogFocusTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk2 subobject to the one
         RadioButton::dtor expects, then forward unchanged.
Original Offset: 00407060
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button2_dtor(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    RadioButtonDtorTarget(object - vtordisp - 1092, arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk3 subobject to the one
         RadioButton::dtor expects, then forward unchanged.
Original Offset: 00407080
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button3_dtor(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonDtorTarget(object - vtordisp - 2584, arg0);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::attach expects, then forward unchanged.
Original Offset: 00407090
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_radio_button1_attach(void *self, void *, void *arg0,
                                           int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return RadioButtonAttachTarget(object - vtordisp, arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from RadioButton's thunk3 subobject to the one
         RadioButton::on_redraw expects, then forward unchanged.
Original Offset: 004070A0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_radio_button3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    RadioButtonOnRedrawTarget(object - vtordisp - 2584);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
Original Offset: 004070F0
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs1_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return DialogsScalarDeleteTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scroll_create expects, then forward unchanged.
Original Offset: 00407150
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_scroll_create(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnScrollCreateTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scroll_delete expects, then forward unchanged.
Original Offset: 00407160
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_dialogs1_on_scroll_delete(void *self, void *,
                                                void *arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return DialogsOnScrollDeleteTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_redraw expects, then forward unchanged.
Original Offset: 00407170
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnRedrawTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mousewheel expects, then forward unchanged.
Original Offset: 00407180
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_mousewheel(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnMousewheelTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mouse_move expects, then forward unchanged.
Original Offset: 00407190
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_mouse_move(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnMouseMoveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mouse_leave expects, then forward unchanged.
Original Offset: 004071A0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_mouse_leave(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnMouseLeaveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_click expects, then forward unchanged.
Original Offset: 004071B0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_left_click(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnLeftClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_down expects, then forward unchanged.
Original Offset: 004071C0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_left_down(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnLeftDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_up expects, then forward unchanged.
Original Offset: 004071D0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_left_up(void *self, void *, int arg0,
                                           int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnLeftUpTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_click expects, then forward unchanged.
Original Offset: 004071E0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_right_click(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnRightClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_down expects, then forward unchanged.
Original Offset: 004071F0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_right_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnRightDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_up expects, then forward unchanged.
Original Offset: 00407200
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_right_up(void *self, void *, int arg0,
                                            int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnRightUpTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_key_down expects, then forward unchanged.
Original Offset: 00407210
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_dialogs1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return DialogsOnKeyDownTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_double_click expects, then forward unchanged.
Original Offset: 00407220
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_left_double_click(void *self, void *,
                                                     int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnLeftDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_double_click expects, then forward unchanged.
Original Offset: 00407230
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_right_double_click(void *self, void *,
                                                      int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnRightDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scrolled expects, then forward unchanged.
Original Offset: 00407240
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_scrolled(void *self, void *, int arg0,
                                            int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnScrolledTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_scrolling expects, then forward unchanged.
Original Offset: 00407250
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_scrolling(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnScrollingTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_dialog_focus expects, then forward unchanged.
Original Offset: 00407260
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_on_dialog_focus(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnDialogFocusTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::pass_dialog_focus expects, then forward unchanged.
Original Offset: 00407270
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs1_pass_dialog_focus(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsPassDialogFocusTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk2 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
Original Offset: 00407280
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs2_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return DialogsScalarDeleteTarget(object - vtordisp - 1092, arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk3 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
Original Offset: 004072A0
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs3_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return DialogsScalarDeleteTarget(object - vtordisp - 2584, arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::attach expects, then forward unchanged.
Original Offset: 004072B0
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_dialogs1_attach(void *self, void *, void *arg0,
                                      int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return DialogsAttachTarget(object - vtordisp, arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk3 subobject to the one
         Dialogs::on_redraw expects, then forward unchanged.
Original Offset: 004072C0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_dialogs3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    DialogsOnRedrawTarget(object - vtordisp - 2584);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::scalar_delete expects, then forward unchanged.
Original Offset: 00421820
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_map_win1_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return MapWinScalarDeleteTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
Original Offset: 00421870
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_lose_mouse_capture(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLoseMouseCaptureTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_redraw expects, then forward unchanged.
Original Offset: 00421880
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnRedrawTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
Original Offset: 00421890
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_resize(void *self, void *, int arg0,
                                          int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnResizeTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_sys_close expects, then forward unchanged.
Original Offset: 004218A0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_sys_close(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnSysCloseTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
Original Offset: 004218B0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_mouse_move(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnMouseMoveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
Original Offset: 004218C0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_left_click(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
Original Offset: 004218D0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_left_down(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
Original Offset: 004218E0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_left_up(void *self, void *, int arg0,
                                           int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftUpTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
Original Offset: 004218F0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_right_click(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnRightClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
Original Offset: 00421900
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_right_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnRightDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
Original Offset: 00421910
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_left_double_click(void *self, void *,
                                                     int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_nc_left_down expects, then forward unchanged.
Original Offset: 00421920
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_nc_left_down(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnNcLeftDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
Original Offset: 00421930
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_button_clicked(void *self, void *,
                                                  int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnButtonClickedTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_nc_hittest expects, then forward unchanged.
Original Offset: 00421940
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win1_on_nc_hittest(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnNcHittestTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk2 subobject to the one
         MapWin::scalar_delete expects, then forward unchanged.
Original Offset: 00421950
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_map_win2_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return MapWinScalarDeleteTarget(object - vtordisp - 1092, arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
Original Offset: 0048BF60
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_lose_mouse_capture(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLoseMouseCaptureTarget(object - vtordisp - 1508);
}

/*
Purpose: Adjust the receiver from PlanWin's thunk1 subobject to the one
         PlanWin::on_redraw expects, then forward unchanged.
Original Offset: 0048BF70
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_plan_win1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    PlanWinOnRedrawTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
Original Offset: 0048BF80
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_resize(void *self, void *, int arg0,
                                          int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnResizeTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_sys_close expects, then forward unchanged.
Original Offset: 0048BF90
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_sys_close(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnSysCloseTarget(object - vtordisp - 1508);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
Original Offset: 0048BFA0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_mouse_move(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnMouseMoveTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
Original Offset: 0048BFB0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_left_click(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftClickTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
Original Offset: 0048BFC0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_left_down(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftDownTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
Original Offset: 0048BFD0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_left_up(void *self, void *, int arg0,
                                           int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftUpTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
Original Offset: 0048BFE0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_right_click(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnRightClickTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
Original Offset: 0048BFF0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_right_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnRightDownTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
Original Offset: 0048C000
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_left_double_click(void *self, void *,
                                                     int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftDoubleClickTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_nc_left_down expects, then forward unchanged.
Original Offset: 0048C010
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_nc_left_down(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnNcLeftDownTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
Original Offset: 0048C020
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_button_clicked(void *self, void *,
                                                  int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnButtonClickedTarget(object - vtordisp - 1508, arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_nc_hittest expects, then forward unchanged.
Original Offset: 0048C030
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win5_on_nc_hittest(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnNcHittestTarget(object - vtordisp - 1508, arg0, arg1);
}

/*
Purpose: Adjust the receiver from PlanWin's thunk2 subobject to the one
         PlanWin::scalar_delete expects, then forward unchanged.
Original Offset: 0048C040
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_plan_win2_scalar_delete(void *self, void *,
                                                 unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return PlanWinScalarDeleteTarget(object - vtordisp - 1092, arg0);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::scalar_delete expects, then forward unchanged.
Original Offset: 0051DDE0
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_console1_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return ConsoleScalarDeleteTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
Original Offset: 0051DE30
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_lose_mouse_capture(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLoseMouseCaptureTarget(object - vtordisp - 9000);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_redraw expects, then forward unchanged.
Original Offset: 0051DE40
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnRedrawTarget(object - vtordisp - 9000);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
Original Offset: 0051DE50
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_resize(void *self, void *, int arg0,
                                          int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnResizeTarget(object - vtordisp - 9000, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_sys_close expects, then forward unchanged.
Original Offset: 0051DE60
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_sys_close(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ConsoleOnSysCloseTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
Original Offset: 0051DE70
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_mouse_move(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnMouseMoveTarget(object - vtordisp - 9000, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_click expects, then forward unchanged.
Original Offset: 0051DE80
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_left_click(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftClickTarget(object - vtordisp - 9000, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_down expects, then forward unchanged.
Original Offset: 0051DE90
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_left_down(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftDownTarget(object - vtordisp - 9000, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_up expects, then forward unchanged.
Original Offset: 0051DEA0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_left_up(void *self, void *, int arg0,
                                           int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftUpTarget(object - vtordisp - 9000, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_right_click expects, then forward unchanged.
Original Offset: 0051DEB0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_right_click(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnRightClickTarget(object - vtordisp - 9000, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_right_down expects, then forward unchanged.
Original Offset: 0051DEC0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_right_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnRightDownTarget(object - vtordisp - 9000, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_key_click expects, then forward unchanged.
Original Offset: 0051DED0
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_console1_on_key_click(void *self, void *, int arg0,
                                            int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return ConsoleOnKeyClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_left_double_click expects, then forward unchanged.
Original Offset: 0051DEE0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_left_double_click(void *self, void *,
                                                     int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnLeftDoubleClickTarget(object - vtordisp - 9000, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_mouse_move expects, then forward unchanged.
Original Offset: 0051DEF0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_nc_mouse_move(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ConsoleOnNcMouseMoveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_left_down expects, then forward unchanged.
Original Offset: 0051DF00
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_nc_left_down(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ConsoleOnNcLeftDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_left_up expects, then forward unchanged.
Original Offset: 0051DF10
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_nc_left_up(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ConsoleOnNcLeftUpTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from MapWin's thunk4 subobject to the one
         MapWin::on_button_clicked expects, then forward unchanged.
Original Offset: 0051DF20
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_map_win4_on_button_clicked(void *self, void *,
                                                  int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    MapWinOnButtonClickedTarget(object - vtordisp - 9000, arg0);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_post_redraw_nc_buffer expects, then forward unchanged.
Original Offset: 0051DF30
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_post_redraw_nc_buffer(void *self, void *,
                                                         void *arg0,
                                                         int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ConsoleOnPostRedrawNcBufferTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk1 subobject to the one
         Console::on_nc_hittest expects, then forward unchanged.
Original Offset: 0051DF40
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_console1_on_nc_hittest(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ConsoleOnNcHittestTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from Console's thunk2 subobject to the one
         Console::scalar_delete expects, then forward unchanged.
Original Offset: 0051DF50
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_console2_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return ConsoleScalarDeleteTarget(object - vtordisp - 1092, arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_redraw expects, then forward unchanged.
Original Offset: 0060CF50
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnRedrawTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mousewheel expects, then forward unchanged.
Original Offset: 0060CF60
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_mousewheel(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnMousewheelTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mouse_move expects, then forward unchanged.
Original Offset: 0060CF70
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_mouse_move(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnMouseMoveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mouse_leave expects, then forward unchanged.
Original Offset: 0060CF80
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_mouse_leave(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnMouseLeaveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_left_down expects, then forward unchanged.
Original Offset: 0060CF90
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_left_down(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnLeftDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_right_down expects, then forward unchanged.
Original Offset: 0060CFA0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_right_down(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnRightDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_key_down expects, then forward unchanged.
Original Offset: 0060CFB0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnKeyDownTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_left_double_click expects, then forward unchanged.
Original Offset: 0060CFC0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_left_double_click(void *self, void *,
                                                      int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnLeftDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_right_double_click expects, then forward unchanged.
Original Offset: 0060CFD0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_right_double_click(void *self, void *,
                                                       int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnRightDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_scrolled expects, then forward unchanged.
Original Offset: 0060CFE0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_scrolled(void *self, void *, int arg0,
                                             int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnScrolledTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_scrolling expects, then forward unchanged.
Original Offset: 0060CFF0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_scrolling(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnScrollingTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_dialog_focus expects, then forward unchanged.
Original Offset: 0060D000
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box1_on_dialog_focus(void *self, void *,
                                                 int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnDialogFocusTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::attach expects, then forward unchanged.
Original Offset: 0060D040
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_list_box1_attach(void *self, void *, void *arg0,
                                       int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return ListBoxAttachTarget(object - vtordisp, arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from ListBox's thunk3 subobject to the one
         ListBox::on_redraw expects, then forward unchanged.
Original Offset: 0060D050
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_list_box3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    ListBoxOnRedrawTarget(object - vtordisp - 2584);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_redraw expects, then forward unchanged.
Original Offset: 0060FE40
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    CheckBoxOnRedrawTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_mouse_move expects, then forward unchanged.
Original Offset: 0060FE50
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_mouse_move(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    CheckBoxOnMouseMoveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_mouse_leave expects, then forward unchanged.
Original Offset: 0060FE60
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_mouse_leave(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    CheckBoxOnMouseLeaveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_left_down expects, then forward unchanged.
Original Offset: 0060FE70
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_left_down(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    CheckBoxOnLeftDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_key_down expects, then forward unchanged.
Original Offset: 0060FE80
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_check_box1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return CheckBoxOnKeyDownTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_left_double_click expects, then forward unchanged.
Original Offset: 0060FE90
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_left_double_click(void *self, void *,
                                                       int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    CheckBoxOnLeftDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_dialog_focus expects, then forward unchanged.
Original Offset: 0060FEA0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box1_on_dialog_focus(void *self, void *,
                                                  int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    CheckBoxOnDialogFocusTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::attach expects, then forward unchanged.
Original Offset: 0060FEE0
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_check_box1_attach(void *self, void *, void *arg0,
                                        int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return CheckBoxAttachTarget(object - vtordisp, arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from CheckBox's thunk3 subobject to the one
         CheckBox::on_redraw expects, then forward unchanged.
Original Offset: 0060FEF0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_check_box3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    CheckBoxOnRedrawTarget(object - vtordisp - 2584);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_redraw expects, then forward unchanged.
Original Offset: 00611820
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnRedrawTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_mouse_move expects, then forward unchanged.
Original Offset: 00611830
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_mouse_move(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnMouseMoveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_mouse_leave expects, then forward unchanged.
Original Offset: 00611840
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_mouse_leave(void *self, void *,
                                                  int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnMouseLeaveTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_click expects, then forward unchanged.
Original Offset: 00611850
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_left_click(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnLeftClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_down expects, then forward unchanged.
Original Offset: 00611860
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_left_down(void *self, void *, int arg0,
                                                int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnLeftDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_up expects, then forward unchanged.
Original Offset: 00611870
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_left_up(void *self, void *, int arg0,
                                              int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnLeftUpTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_click expects, then forward unchanged.
Original Offset: 00611880
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_right_click(void *self, void *,
                                                  int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnRightClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_down expects, then forward unchanged.
Original Offset: 00611890
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_right_down(void *self, void *, int arg0,
                                                 int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnRightDownTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_up expects, then forward unchanged.
Original Offset: 006118A0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_right_up(void *self, void *, int arg0,
                                               int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnRightUpTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_key_down expects, then forward unchanged.
Original Offset: 006118B0
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_sprite_box1_on_key_down(void *self, void *, int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return SpriteBoxOnKeyDownTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_double_click expects, then forward unchanged.
Original Offset: 006118C0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_left_double_click(void *self, void *,
                                                        int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnLeftDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_double_click expects, then forward unchanged.
Original Offset: 006118D0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_right_double_click(void *self, void *,
                                                         int arg0, int arg1) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnRightDoubleClickTarget(object - vtordisp, arg0, arg1);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_dialog_focus expects, then forward unchanged.
Original Offset: 006118E0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box1_on_dialog_focus(void *self, void *,
                                                   int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnDialogFocusTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::attach expects, then forward unchanged.
Original Offset: 00611920
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_sprite_box1_attach(void *self, void *, void *arg0,
                                         int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return SpriteBoxAttachTarget(object - vtordisp, arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from SpriteBox's thunk3 subobject to the one
         SpriteBox::on_redraw expects, then forward unchanged.
Original Offset: 00611930
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_sprite_box3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    SpriteBoxOnRedrawTarget(object - vtordisp - 2584);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::on_redraw expects, then forward unchanged.
Original Offset: 006127B0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_edit_group1_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    EditGroupOnRedrawTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::on_dialog_focus expects, then forward unchanged.
Original Offset: 006127C0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_edit_group1_on_dialog_focus(void *self, void *,
                                                   int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    EditGroupOnDialogFocusTarget(object - vtordisp, arg0);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::pass_dialog_focus expects, then forward unchanged.
Original Offset: 006127D0
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_edit_group1_pass_dialog_focus(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    EditGroupPassDialogFocusTarget(object - vtordisp);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk1 subobject to the one
         EditGroup::attach expects, then forward unchanged.
Original Offset: 00612810
Return Value: the forwarded call's
Status: Complete
*/
int __fastcall adjust_edit_group1_attach(void *self, void *, void *arg0,
                                         int arg1, int arg2, int arg3) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return EditGroupAttachTarget(object - vtordisp, arg0, arg1, arg2, arg3);
}

/*
Purpose: Adjust the receiver from EditGroup's thunk3 subobject to the one
         EditGroup::on_redraw expects, then forward unchanged.
Original Offset: 00612820
Return Value: n/a
Status: Complete
*/
void __fastcall adjust_edit_group3_on_redraw(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    EditGroupOnRedrawTarget(object - vtordisp - 2584);
}
