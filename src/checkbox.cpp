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
#include "vtable_shim.h"

func_dialog_close CheckBoxOriginalDialogClose = original_method<func_dialog_close>(0x00608F50);
uint32_t CheckBoxDefault1;  // 0x00697104
uint32_t CheckBoxDefault2;  // 0x00697108

/*
Purpose: Reset the check box to its defaults, then close its dialog and
         graphic base. Both calls resolve through the vbtable, so they reach
         the Dialog and the virtual base rather than the object itself.
// ORIGINAL: 0x0060E7C0 ?close@CheckBox@@QAEXXZ 0x0060E7C0-0x0060E7F9
// size      57 bytes
// prototype void (__thiscall ?close@CheckBox@@QAEXXZ)(CheckBox* this)
// callers   13   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
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
    field_14_ = CheckBoxDefault2;
    field_10_ = CheckBoxDefault1;
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
// ORIGINAL: 0x0060EB50 ?UNK1@CheckBox@@QAEXH@Z 0x0060EB50-0x0060EB7D
// size      45 bytes
// prototype void (__thiscall ?UNK1@CheckBox@@QAEXH@Z)(CheckBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x0060ECC0 ?UNK2@CheckBox@@QAEHH@Z 0x0060ECC0-0x0060ECE0
// size      32 bytes
// prototype int (__thiscall ?UNK2@CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x0060EC80 ?set_state_pos@CheckBox@@QAEXHH@Z 0x0060EC80-0x0060ECB1
// size      49 bytes
// prototype void (__thiscall ?set_state_pos@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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

/*
Purpose: Repaint on dialog focus, dispatching through the enclosing object.
// ORIGINAL: 0x0060FB90 ?on_dialog_focus@CheckBox@@QAEXH@Z 0x0060FB90-0x0060FBA7 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?on_dialog_focus@CheckBox@@QAEXH@Z)(CheckBox* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060FB9E
Return Value: n/a
Status: Complete
*/
void CheckBox::on_dialog_focus(int a1) {
    // `this - 0x1c` is NOT arithmetic on a member: CheckBox is a SUBOBJECT of
    // a larger object, and this walks back to the enclosing one, then applies
    // a further delta read from its own +4. Written as one expression on
    // purpose - naming either intermediate makes VC6 pick eax for the delta
    // and edx for the vtable, which is mnemonic-identical and byte-different.
    // a1 is never read; `ret 4` still pops it.
    VCall *const target = reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x1c +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this) - 0x1c)) + 4));
    target->slot062();
}

/*
Purpose: Clear the hover index and repaint, through the enclosing object.
// ORIGINAL: 0x0060FC30 ?on_mouse_leave@CheckBox@@QAEXHH@Z 0x0060FC30-0x0060FC58 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?on_mouse_leave@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060FC4F
Return Value: n/a
Status: Complete
*/
void CheckBox::on_mouse_leave(int a1, int a2) {
    // `this - 0x1C` reaches a vbtable-shaped descriptor with two deltas:
    // entry +8 locates the field, entry +4 the enclosing object's vtable.
    // Note 0x1C + 0xD4 == 0xF0 across this whole family, so the
    // field is at a FIXED +0xF0 in the enclosing object and these classes are
    // subobjects at differing offsets inside it.
    //
    // The arithmetic must go through `char *`. Through `int *` the compiler
    // scales the +8 and emits [eax+0x20].
    //
    // Both parameters are dead; `ret 8` still pops them.
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(this) + 0xD4 +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this) - 0x1C)) + 8)) = -1;

    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x1C +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this) - 0x1C)) + 4))->slot062();
}

/*
// ORIGINAL: 0x0060FC60 ?init_class@CheckBox@@QAAHXZ 0x0060FC60-0x0060FD52;0x00662F84-0x00662F99
// symbol    ?init_class@CheckBox@@SAHXZ
// RULED-OUT: a real local `Buffer buf;` double-destructs (the explicit ~Buffer() call plus the automatic scope-exit one). A raw `char[sizeof(Buffer)]` + placement `new` + explicit `->~Buffer()` is the shape the original uses. SEH prologue/unwind funclet not reproduced (same gap as the RadioButton sibling).
// size      263 bytes
// prototype int (__cdecl ?init_class@CheckBox@@QAAHXZ)()
// callers   1   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005E39A0
//
// Promoted 2026-08-15 from src/unrecovered/0060fc60.cpp to retire its
// pending_bodies forwarder.
Status: Complete
*/
static int *const g_0069710c = (int *)0x0069710C;
static int *const g_009b8f60 = (int *)0x009B8F60;
static int *const g_009b8f90 = (int *)0x009B8F90;

int __cdecl CheckBox::init_class() {
    char bufMem[sizeof(Buffer)];
    Buffer *buf = new (bufMem) Buffer();
    buf->init(0x20, 0x20, 0, 0);
    int result = buf->load_pcx(reinterpret_cast<const char *>(g_0069710c), 0, 10, 0xec);
    if (result != 0) {
        buf->~Buffer();
        return result;
    }
    reinterpret_cast<Sprite *>(g_009b8f60)->extract(buf, 0x109, 1, 0x44, 0x20, 0x20, 0);
    reinterpret_cast<Sprite *>(g_009b8f90)->extract(buf, 0x109, 0x22, 0x44, 0x20, 0x20, 0);
    buf->~Buffer();
    return 0;
}
