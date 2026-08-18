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
// ORIGINAL: 0x00611040 ?on_mouse_move@SpriteBox@@QAEXHH@Z 0x00611040-0x00611043 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x00611050 ?on_mouse_leave@SpriteBox@@QAEXHH@Z 0x00611050-0x00611053 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00611600 ?id_to_pos@SpriteBox@@QAEXH@Z 0x00611600-0x00611640
// size      64 bytes
// prototype void (__thiscall ?id_to_pos@SpriteBox@@QAEXH@Z)(SpriteBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00610480 ?init@SpriteBox@@QAEHPAURECT@@H@Z 0x00610480-0x006104A2 BYTE_EXACT
// size      34 bytes
// prototype int (__thiscall ?init@SpriteBox@@QAEHPAURECT@@H@Z)(SpriteBox* this, RECT*, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609470 0x00610280
Status: Complete
*/
int SpriteBox::init(RECT* a1, int a2) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1, a2);
}

/*
Purpose: Repaint on dialog focus, through the enclosing object.
// ORIGINAL: 0x006115E0 ?on_dialog_focus@SpriteBox@@QAEXH@Z 0x006115E0-0x00611600 BYTE_EXACT
// size      32 bytes
// prototype void (__thiscall ?on_dialog_focus@SpriteBox@@QAEXH@Z)(SpriteBox* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x006115F7
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
// ORIGINAL: 0x00610420 ?init@SpriteBox@@QAEHH@Z 0x00610420-0x0061043D BYTE_EXACT
// size      29 bytes
// prototype int (__thiscall ?init@SpriteBox@@QAEHH@Z)(SpriteBox* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006095F0 0x00610280
Status: Complete
*/
int SpriteBox::init(int a1) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1);
}

/*
// ORIGINAL: 0x006104B0 ?init@SpriteBox@@QAEHPAUHeap@@@Z 0x006104B0-0x006104CD BYTE_EXACT
// size      29 bytes
// prototype int (__thiscall ?init@SpriteBox@@QAEHPAUHeap@@@Z)(SpriteBox* this, Heap*)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006098D0 0x00610280
Status: Complete
*/
int SpriteBox::init(Heap *a1) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1);
}

func_sprite_box_close SpriteBoxClose =
    original_method<func_sprite_box_close>(0x00610280);

/*
Purpose: Tear the sprite box's entry list down and close the dialog.
         Body unrecovered; forwards to the original image.
Forwards To: 00610280
Return Value: n/a
Status: Forwarder
*/
void SpriteBox::close() {
    (ORIGINAL(this)->*SpriteBoxClose)();
}

/*
// ORIGINAL: 0x006104D0 ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z 0x006104D0-0x00610501 BYTE_EXACT
// size      49 bytes
// prototype int (__thiscall ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z)(SpriteBox* this, int, int, int, int, Heap*)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609730 0x00610280
Status: Complete
*/
int SpriteBox::init(int a1, int a2, int a3, int a4, Heap * a5) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))
        ->init(a1, a2, a3, a4, a5);
}
