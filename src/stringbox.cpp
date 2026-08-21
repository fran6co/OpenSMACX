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
#include "stringbox.h"
#include <cstring>
#include "vtable_shim.h"

func_string_struct_add StringBoxStructAdd = original_method<func_string_struct_add>(0x00401100);

/*
Purpose: Add a string to the box - stage the text, index and a cleared flag
         into the string struct at 0x2B70, add it, and run the fixup pass when
         the struct reports it did not take.
// ORIGINAL: 0x00629710 ?add@StringBox@@QAEXPADHH@Z 0x00629710-0x00629749
// size      57 bytes
// prototype void (__thiscall ?add@StringBox@@QAEXPADHH@Z)(StringBox* this, int8*, int, int)
// callers   6   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100 0x00629490
Return Value: n/a
Status: Complete
*/
void StringBox::add(char *text, int index, int flag) {
    std::memcpy(&field_2B8C_, &text, sizeof(text));
    field_2B90_ = static_cast<uint32_t>(flag);
    field_2B94_ = 0;
    if ((ORIGINAL(&field_2B70_)->*StringBoxStructAdd)(index) == 0) {
        StringBox::add_fixup();
    }
}

void __fastcall string_box_add_redirect(StringBox *self, void *, char *text,
                                        int index, int flag) {
    self->add(text, index, flag);
}

/*
Purpose: Record the scroll position and repaint through virtual slot 62.
// ORIGINAL: 0x00629D20 ?on_scrolled@StringBox@@QAEXHH@Z 0x00629D20-0x00629D35 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?on_scrolled@StringBox@@QAEXHH@Z)(StringBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00629D2C
Return Value: n/a
Status: Complete
*/
void StringBox::on_scrolled(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0xa1c) = a2;
    reinterpret_cast<VCall *>(this)->slot062();
}
