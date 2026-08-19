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
// ORIGINAL: 0x0060D1B0 ?close@RadioButton@@QAEXXZ 0x0060D1B0-0x0060D1E6
// size      54 bytes
// prototype void (__thiscall ?close@RadioButton@@QAEXXZ)(RadioButton* this)
// callers   21   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
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
// ORIGINAL: 0x0060E300 ?on_dialog_focus@RadioButton@@QAEXH@Z 0x0060E300-0x0060E317 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?on_dialog_focus@RadioButton@@QAEXH@Z)(RadioButton* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060E30E
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
// ORIGINAL: 0x0060E4A0 ?on_mouse_leave@RadioButton@@QAEXHH@Z 0x0060E4A0-0x0060E4C8 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?on_mouse_leave@RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060E4BF
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

/*
// ORIGINAL: 0x0060E4D0 ?init_class@RadioButton@@QAAHXZ 0x0060E4D0-0x0060E5C2;0x00662F44-0x00662F59
// symbol    ?init_class@RadioButton@@SAHXZ
// size      263 bytes
// prototype int (__cdecl ?init_class@RadioButton@@QAAHXZ)()
// callers   1   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005E39A0
//
// Promoted 2026-08-15 from src/unrecovered/0060e4d0.cpp to retire its
// pending_bodies forwarder. Sibling of CheckBox::init_class; same
// placement-new Buffer shape and the same SEH-funclet gap (see that body's
// RULED-OUT).
Status: Complete
*/
static int *const g_006970f8 = (int *)0x006970F8;
static int *const g_009b8ef8 = (int *)0x009B8EF8;
static int *const g_009b8f28 = (int *)0x009B8F28;

int __cdecl RadioButton::init_class() {
    char bufMem[sizeof(Buffer)];
    Buffer *buf = new (bufMem) Buffer();
    buf->init(0x20, 0x20, 0, 0);
    int result = buf->load_pcx(reinterpret_cast<const char *>(g_006970f8), 0, 10, 0xec);
    if (result != 0) {
        buf->~Buffer();
        return result;
    }
    reinterpret_cast<Sprite *>(g_009b8ef8)->extract(buf, 0x109, 1, 0x23, 0x20, 0x20, 0);
    reinterpret_cast<Sprite *>(g_009b8f28)->extract(buf, 0x109, 0x22, 0x23, 0x20, 0x20, 0);
    buf->~Buffer();
    return 0;
}
