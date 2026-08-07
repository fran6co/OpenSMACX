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
#include "checkbox.h"

func_dialog_close CheckBoxOriginalDialogClose = original_method<func_dialog_close>(0x00608F50);
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
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    field_4_ = 0;
    field_8_ = 0;
    field_C_ = 0;
    field_14_ = *CheckBoxDefault2;
    field_10_ = *CheckBoxDefault1;
    (ORIGINAL(reinterpret_cast<Dialog *>(self + vbtable[2]))->*CheckBoxOriginalDialogClose)();
    reinterpret_cast<GraphicWin *>(self + vbtable[1])->close();
}

void __fastcall check_box_close_redirect(CheckBox *self, void *) {
    self->close();
}

// The state word these three share. Every one of them reaches it the same way
// the original does - `mov eax,[ecx]` then `[eax+8]` - so the Dialog offset
// comes from the object's OWN vbtable at run time. Hardcoding this class's own
// 0xA34 would work for a most-derived CheckBox and aim at the wrong subobject
// for an embedded one, which is the defect that passed every unit test here
// and crashed the game on a null vtable pointer. Repeated inline rather than
// factored out, matching close() above, so each body stands alone.

/*
Purpose: Toggle one bit of the state word - clear it when set, set it when
         clear.

             mov eax,[ecx] / mov edx,[eax+8] / mov eax,1
             lea edx,[edx+ecx+0xEC] / mov ecx,[esp+4] / shl eax,cl
             mov ecx,[edx] / test eax,ecx / je set
             not eax / and eax,ecx / mov [edx],eax / ret 4
             set: or ecx,eax / mov [edx],ecx / ret 4

         The shift count is whatever CL holds, so the position is masked to
         five bits by the hardware; `pos & 31` states that rather than relying
         on C++ shift-overflow, which is undefined rather than truncating.
Original Offset: 0060EB50
Return Value: n/a
Status: Complete
*/
void CheckBox::UNK1(int pos) {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    uint32_t *const flags =
        reinterpret_cast<uint32_t *>(self + vbtable[2] + 0xEC);
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    if (*flags & mask) {
        *flags &= ~mask;
    } else {
        *flags |= mask;
    }
}

void __fastcall check_box_unk1_redirect(CheckBox *self, void *, int pos) {
    self->UNK1(pos);
}

/*
Purpose: Report whether one bit of the state word is set. Returns the MASKED
         WORD, not a normalised 0 or 1 - the body ends `and eax, esi` with no
         further narrowing, so a caller testing it against 1 would read a
         different answer than the original gives.
Original Offset: 0060ECC0
Return Value: the state word masked to the requested bit
Status: Complete
*/
int CheckBox::UNK2(int pos) {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    const uint32_t *const flags =
        reinterpret_cast<const uint32_t *>(self + vbtable[2] + 0xEC);
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    return static_cast<int>(*flags & mask);
}

int __fastcall check_box_unk2_redirect(CheckBox *self, void *, int pos) {
    return self->UNK2(pos);
}

/*
Purpose: Set or clear one bit of the state word, chosen by the second argument.
         Unlike UNK1 this does not depend on the bit's current value.
Original Offset: 0060EC80
Return Value: n/a
Status: Complete
*/
void CheckBox::set_state_pos(int pos, int state) {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    uint32_t *const flags =
        reinterpret_cast<uint32_t *>(self + vbtable[2] + 0xEC);
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    if (state) {
        *flags |= mask;
    } else {
        *flags &= ~mask;
    }
}

void __fastcall check_box_set_state_pos_redirect(CheckBox *self, void *,
                                                 int pos, int state) {
    self->set_state_pos(pos, state);
}
