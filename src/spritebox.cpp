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
#include "spritebox.h"
#include "dialog.h"
#include "vtable_shim.h"

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 00611040
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 00611050
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_mouse_leave(int, int) {
}

void __fastcall sprite_box_on_mouse_move_redirect(SpriteBox *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

void __fastcall sprite_box_on_mouse_leave_redirect(SpriteBox *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}

/*
Purpose: Walk the entry list for `id` and leave the cursor and position on the
         match, or on the last entry examined when there is none.

         Four fields, all raw because SpriteBox models none of them:
             0x2C  head        0x30  cursor
             0x34  count       0x38  position
         and each entry carries its id at +4 and its successor at +0xC.

         THE EMPTY-LIST PATH WRITES NOTHING. `cmp eax,edx / je done` leaves on
         a null head before the two stores, so position keeps whatever it held;
         it is only zeroed once a head exists. A body that reset position first
         and then checked would agree on every non-empty list and differ on the
         one case the branch exists for.

         Both stores DO happen when the head is non-null, even for a
         non-positive count: `mov [ecx+0x38],edx` and `mov [ecx+0x30],eax`
         precede the `jle`, so an empty-but-headed list still clears position
         and parks the cursor.

         Declared uint32_t although the mangled name says void, for the reason
         Dialogs::destroy is: the body loads [this+0x38] into EAX before
         returning, and a void body cannot promise that. The return type does
         not change the thiscall cleanup, so this is the residue and not a
         different function.
Original Offset: 00611600
Return Value: the resulting position, which is also left at 0x38
Status: Complete
*/
uint32_t SpriteBox::id_to_pos(int id) {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    uint32_t *const head = reinterpret_cast<uint32_t *>(self + 0x2C);
    uint32_t *const cursor = reinterpret_cast<uint32_t *>(self + 0x30);
    int32_t *const count = reinterpret_cast<int32_t *>(self + 0x34);
    uint32_t *const position = reinterpret_cast<uint32_t *>(self + 0x38);

    if (*head == 0) {
        return *position;
    }
    *position = 0;
    *cursor = *head;
    if (*count <= 0) {
        return *position;
    }
    int32_t examined = 0;
    for (;;) {
        const uint8_t *const entry = reinterpret_cast<const uint8_t *>(*cursor);
        if (*reinterpret_cast<const int32_t *>(entry + 4) == id) {
            break;
        }
        *position = *position + 1;
        ++examined;
        *cursor = *reinterpret_cast<const uint32_t *>(entry + 0xC);
        if (examined >= *count) {
            break;
        }
    }
    return *position;
}

uint32_t __fastcall sprite_box_id_to_pos_redirect(SpriteBox *self, void *, int id) {
    return self->id_to_pos(id);
}

/*
Original Offset: 00610480
Status: Complete
*/
int SpriteBox::init(RECT* a1, int a2) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1, a2);
}

/*
Purpose: Repaint on dialog focus, through the enclosing object.
Original Offset: 006115E0
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_dialog_focus(int a1) {
    // `this - 0x8c` walks OUT of this subobject to the enclosing one, whose
    // own +4 holds a further this-adjustment delta - the MSVC virtual-base
    // shape. One expression: naming an intermediate changes the register
    // choice. `a1` is dead; `ret 4` still pops it.
    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) - 0x8c
        + *reinterpret_cast<int *>(
            *reinterpret_cast<int *>(
                reinterpret_cast<char *>(this) - 0x8c) + 4))->slot062();
}

/*
Original Offset: 00610420
Status: Complete
*/
int SpriteBox::init(int a1) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1);
}

/*
Original Offset: 006104B0
Status: Complete
*/
int SpriteBox::init(Heap *a1) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1);
}
