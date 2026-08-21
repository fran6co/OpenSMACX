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

uint32_t RadioButtonDefault1;  // 0x006970F0
uint32_t RadioButtonDefault2;  // 0x006970F4

/*
Purpose: Compose a RadioButton from its two base-shaped subobjects
         (GraphicWin, Dialog - both constructed only when most-derived),
         install this class's own vtable/vtordisp values into the object's
         vbtable-named slots, and reset the two state fields.

         `a1` is the compiler's most-derived flag in the image
         (??0RadioButton@@QAE@H@Z): nonzero means construct the two bases
         AND set the object's own vbtable pointer, zero means skip both and
         go straight to installing this class's own tables - the same "am I
         the most-derived object" guard ListBox's constructor uses. This
         class cannot let VC6 synthesise that flag (see the class comment in
         radiobutton.h), so it is modelled as an explicit parameter instead.
// ORIGINAL: 0x0060D0E0 ??0RadioButton@@QAE@H@Z 0x0060D0E0-0x0060D1A2;0x00662F20-0x00662F44
// RULED-OUT: MEASURED 3/52 agreeing, same family as CheckBox/EditGroup - no
//            SEH frame at all in this compiled body (`mov eax,0; call $+5`
//            in place of `push -1; push handler`). `dialog_.construct()`
//            is an ORDINARY METHOD CALL, not a placement-new constructor
//            call, so VC6 has no partially-constructed subobject to
//            protect and emits no frame - the same distinction the
//            coordinator's CheckButton::construct() finding names from the
//            other direction. Getting the frame back needs Dialog's OWN
//            constructor to be non-trivial (a real placement-new target)
//            AND GraphicWin's OWN destructor to be non-trivial (what the
//            catch clause would actually unwind to) - both currently kept
//            trivial by design (see the class comment above), and both are
//            real typed members here (dialog_, virtual_base_), so making
//            either non-trivial changes what EVERY CheckBox/EditGroup/
//            RadioButton/SpriteBox object implicitly does at construction,
//            not just this one function. Not attempted: the coupled edit
//            this blocks on is explicitly flagged elsewhere in this file as
//            OPEN and cross-cutting, well beyond one constructor.
// size      230 bytes
// prototype void (__thiscall ??0RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x00608C10
Return Value: n/a
Status: Complete
*/
RadioButton::RadioButton(int a1) {
    char *const self = reinterpret_cast<char *>(this);

    if (a1 != 0) {
        vbtable_pointer_ = 0x00670590;
        virtual_base_.construct();
        try {
            dialog_.construct();
        } catch (...) {
            virtual_base_.~GraphicWin();
            throw;
        }
    }

    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1) = 0x00669A6C;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 + 0x444) = 0x00669A64;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self + off2) = 0x00669A58;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 - 4) = off1 - 0x18;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self + off2 - 4) = off2 - 0xA30;
    }

    field_C_ = 0;
    field_10_ = 0;
    field_8_ = RadioButtonDefault2;
    field_4_ = RadioButtonDefault1;
}

/*
Purpose: The complete-object destructor entry, which also conditionally
         frees the allocation: reinstall the base subobjects' own
         vtable/vtordisp values, close the RadioButton, destroy the Dialog
         and GraphicWin subobjects, then free through the executable's
         operator delete only when bit 0 of mode asks. See the note in
         radiobutton.h for why this is a free function rather than a
         destructor.
// ORIGINAL: 0x00406F60 ??1RadioButton@@QAE@H@Z 0x00406F60-0x00406FE2
// symbol    ?radio_button_teardown_redirect@@YIPAVRadioButton@@PAX0I@Z
// RULED-OUT: MEASURED 3/39 agreeing, first divergence at instruction 2
//            (matches through `push ebp; mov ebp,esp`, then this body loads
//            the vbtable pointer a step earlier than the image's `push
//            esi/edi` pair). No SEH family issue here - the image's own
//            "frame" flag (not an EH one) is an ordinary ebp frame, which
//            this body reproduces the opening of; the mismatch is plain
//            register-allocation order in the vtable-store block.
// size      130 bytes
// prototype void (__thiscall ??1RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4DD0 0x00608E10 0x0060D1B0 0x0064557F
Return Value: the RadioButton pointer (this - 0x18), the original's EAX
Status: Complete
*/
RadioButton *__fastcall radio_button_teardown_redirect(void *adjusted, void *,
                                                        unsigned int mode) {
    char *const self = static_cast<char *>(adjusted) - 0x18;

    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        *reinterpret_cast<int32_t *>(self + vbtable[1]) = 0x00669A6C;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        *reinterpret_cast<int32_t *>(self + vbtable[1] + 0x444) = 0x00669A64;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        *reinterpret_cast<int32_t *>(self + vbtable[2]) = 0x00669A58;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 - 4) = off1 - 0x18;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self + off2 - 4) = off2 - 0xA30;
    }

    reinterpret_cast<RadioButton *>(self)->close();
    reinterpret_cast<Dialog *>(self + 0xA30)->destroy();
    graphic_win_destructor_redirect(reinterpret_cast<GraphicWin *>(self + 0x18), nullptr);

    if (mode & 1) {
        operator delete(self);
    }
    return reinterpret_cast<RadioButton *>(self);
}

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
    field_8_ = RadioButtonDefault2;
    field_4_ = RadioButtonDefault1;
    reinterpret_cast<Dialog *>(self + vbtable[2])->close();
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
