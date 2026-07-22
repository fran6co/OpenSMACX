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
#include "checkbox.h"

func_dialog_close *CheckBoxOriginalDialogClose = (func_dialog_close *)0x00608F50;
uint32_t *CheckBoxDefault1 = (uint32_t *)0x00697104;
uint32_t *CheckBoxDefault2 = (uint32_t *)0x00697108;

/*
Purpose: Reset the check box to its defaults, then close its dialog and
         graphic base. Both calls resolve through the vbtable, so they reach
         the Dialog and the virtual base rather than the object itself.
Original Offset: 0060E7C0
Return Value: n/a
Status: Complete
*/
void CheckBox::close() {
    // The base offsets come from the object's own vbtable, read at run time,
    // not from where they sit in a most-derived CheckBox. When one is
    // embedded in a larger class - Dialogs holds a RadioButton at 0x44 - that
    // object's vbtable names different offsets, and hardcoding this class's
    // own sends both calls to the wrong subobject. Doing exactly that passed
    // every unit test here and crashed the game on a null vtable pointer.
    auto *const self = reinterpret_cast<uint8_t *>(this);
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    field_4_ = 0;
    field_8_ = 0;
    field_C_ = 0;
    field_14_ = *CheckBoxDefault2;
    field_10_ = *CheckBoxDefault1;
    CheckBoxOriginalDialogClose(
        reinterpret_cast<Dialog *>(self + vbtable[2]));
    reinterpret_cast<GraphicWin *>(self + vbtable[1])->close();
}

void __fastcall check_box_close_redirect(CheckBox *self, void *) {
    self->close();
}
