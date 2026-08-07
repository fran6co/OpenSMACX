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
#include "radiobutton.h"
#include "vtable_shim.h"

func_dialog_close RadioButtonOriginalDialogClose = original_method<func_dialog_close>(0x00608F50);
uint32_t *RadioButtonDefault1 = (uint32_t *)0x006970F0;
uint32_t *RadioButtonDefault2 = (uint32_t *)0x006970F4;

/*
Purpose: Reset the radio button to its defaults, then close its dialog and
         graphic base. Both calls resolve through the vbtable, so they reach
         the Dialog and the virtual base rather than the object itself.
Original Offset: 0060D1B0
Return Value: n/a
Status: Complete
*/
void RadioButton::close() {
    // The base offsets come from the object's own vbtable, read at run time,
    // not from where they sit in a most-derived RadioButton. When one is
    // embedded in a larger class - Dialogs holds a RadioButton at 0x44 - that
    // object's vbtable names different offsets, and hardcoding this class's
    // own sends both calls to the wrong subobject. Doing exactly that passed
    // every unit test here and crashed the game on a null vtable pointer.
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    field_C_ = 0;
    field_10_ = 0;
    field_8_ = *RadioButtonDefault2;
    field_4_ = *RadioButtonDefault1;
    (ORIGINAL(reinterpret_cast<Dialog *>(self + vbtable[2]))->*RadioButtonOriginalDialogClose)();
    reinterpret_cast<GraphicWin *>(self + vbtable[1])->close();
}

void __fastcall radio_button_close_redirect(RadioButton *self, void *) {
    self->close();
}

/*
Original Offset: 0060E300
Status: Complete
*/
void RadioButton::on_dialog_focus(int a1) {
    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x18 +
        *reinterpret_cast<int *>(
            *reinterpret_cast<char **>(reinterpret_cast<char *>(this) - 0x18) + 4)
    )->slot062();
}

/*
Purpose: Clear the hover index and repaint, through the enclosing object.
Original Offset: 0060E4A0
Return Value: n/a
Status: Complete
*/
void RadioButton::on_mouse_leave(int a1, int a2) {
    // `this - 0x18` reaches a vbtable-shaped descriptor with two deltas:
    // entry +8 locates the field, entry +4 the enclosing object's vtable.
    // Note 0x18 + 0xD8 == 0xF0 across this whole family, so the
    // field is at a FIXED +0xF0 in the enclosing object and these classes are
    // subobjects at differing offsets inside it.
    //
    // The arithmetic must go through `char *`. Through `int *` the compiler
    // scales the +8 and emits [eax+0x20].
    //
    // Both parameters are dead; `ret 8` still pops them.
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(this) + 0xD8 +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this) - 0x18)) + 8)) = -1;

    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x18 +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this) - 0x18)) + 4))->slot062();
}
