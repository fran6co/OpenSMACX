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
#include "sprite.h"   // the Sprite ~Cursor owns

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0063B930 ?close_cursor_class@Cursor@@QAAXXZ 0x0063B930-0x0063B931 BYTE_EXACT
// symbol    ?close_cursor_class@Cursor@@SAXXZ
// size      1 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Cursor::close_cursor_class() {
}

void __cdecl cursor_close_cursor_class_redirect() {
    Cursor::close_cursor_class();
}

int CursorWidth;   // 0x009BE6D4
int CursorHeight;  // 0x009BE6D8

/*
Purpose: Cache the system cursor's width and height.
// ORIGINAL: 0x0063B910 ?init_cursor_class@Cursor@@QAAHXZ 0x0063B910-0x0063B92D BYTE_EXACT
// symbol    ?init_cursor_class@Cursor@@SAHXZ
// size      29 bytes
// prototype int (__cdecl ?init_cursor_class@Cursor@@QAAHXZ)()
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int Cursor::init_cursor_class() {
    CursorWidth = GetSystemMetrics(SM_CXCURSOR);
    CursorHeight = GetSystemMetrics(SM_CYCURSOR);
    return 0;
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
// ORIGINAL: 0x0063B2D0 ??0Cursor@@QAE@XZ 0x0063B2D0-0x0063B2E0
// size      16 bytes
// prototype Cursor* (__thiscall ??0Cursor@@QAE@XZ)(Cursor* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x0063B8D0 ??1Cursor@@QAE@XZ 0x0063B8D0-0x0063B90A BYTE_EXACT
// body      src/cursor.h
// size      58 bytes
// prototype void (__thiscall ??1Cursor@@QAE@XZ)(Cursor* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3820 0x0064557F
// indirect  0x0063B8DE
*/

Cursor::~Cursor() {
    // `field_0_` is the HCURSOR and `field_4_` a Sprite the object owns;
    // the image destroys both and then clears the two words after them.
    HCURSOR handle = reinterpret_cast<HCURSOR>(field_0_);
    if (handle) {
        DestroyCursor(handle);
        field_0_ = 0;
    }
    Sprite *sprite = reinterpret_cast<Sprite *>(field_4_);
    if (sprite) {
        sprite->close();
        operator delete(sprite);
        field_4_ = 0;
    }
    field_8_ = 0;
    field_C_ = 0;
}

