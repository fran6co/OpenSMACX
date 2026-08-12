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
#include "cursor.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x0063B930 BYTE_EXACT
// name      ?close_cursor_class@Cursor@@QAAXXZ
// size      1 bytes
// spans     0x0063B930-0x0063B931
// prototype 
// callers   1   call targets   0
Return Value: n/a
Status: Complete
*/
void Cursor::close_cursor_class() {
}

void __cdecl cursor_close_cursor_class_redirect() {
    Cursor::close_cursor_class();
}

/*
Purpose: Clear all four fields.

             mov eax,ecx / xor ecx,ecx
             mov [eax],ecx / mov [eax+8],ecx / mov [eax+0xC],ecx
             mov [eax+4],ecx / ret

         `mov eax,ecx` first is the legacy EAX = this return every recovered
         constructor here preserves; the redirect carries it.

         The original stores 0, 8, 0xC then 4 - not in address order - and that
         ordering is NOT reproduced deliberately, because it cannot be
         observed: four independent stores of the same constant to distinct
         fields leave the same object whatever order they run in, and nothing
         interleaves. Saying so beats implying the sequence was matched.
ORIGINAL: 0x0063B2D0
// name      ??0Cursor@@QAE@XZ
// size      16 bytes
// spans     0x0063B2D0-0x0063B2E0
// prototype Cursor* (__thiscall ??0Cursor@@QAE@XZ)(Cursor* this)
// callers   0   call targets   0
Return Value: this
Status: Complete
*/
void Cursor::construct() {
    field_0_ = 0;
    field_4_ = 0;
    field_8_ = 0;
    field_C_ = 0;
}

Cursor *__fastcall cursor_construct_redirect(Cursor *self, void *) {
    self->construct();
    return self;
}
