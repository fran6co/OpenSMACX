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
#include "imagebutton.h"

/*
Purpose: Construct the button on its BaseButton base and clear its one field.
// ORIGINAL: 0x006252E0 ??0ImageButton@@QAE@XZ 0x006252E0-0x00625306 BYTE_EXACT
// LEVER: real-base the class derives from BaseButton now, so `call 0x00606F30` and both vtable stores are the compiler's. The old free-function redirect made all three by hand out of a `void *`.
// LEVER (2026-09-03, the /Ob0 dependency closed): BaseButton's constructor is
//        OUT OF LINE (basebutton.cpp) instead of the in-class `{ ; }`. In
//        class it was EXPANDED at this implicit base construction, and
//        because that expansion carries the two Time constructions VC6 added
//        the SEH unwind frame the image does not pay - 25 compiled
//        instructions, 0 of 9 agreeing, first divergence at instruction 0.
//        Out of line there is no body to fold, so the site is the image's own
//        9 instructions. Same lever as ~Buffer (buffer.h) and Heap's ctor.
// symbol    ??0ImageButton@@QAE@XZ
// size      38 bytes
// kind      game
Return Value: `this`
Status: Complete
*/
ImageButton::ImageButton() {
    // IN THE BODY, not the member-initialiser list. The image stores this
    // AFTER both vfptrs (0x006252F8, after 0x006252E8 and 0x006252EE), and
    // MSVC runs the list BEFORE the vfptr assignment - which is exactly how
    // ??0Win@@QAE@XZ's six stores had to move the other way.
    sprite_ = 0;
}

/*
Purpose: Restore both vtables and tail-call the base destructor.
// ORIGINAL: 0x00625310 ??1ImageButton@@QAE@XZ 0x00625310-0x00625325 BYTE_EXACT
// LEVER: real-base an empty body is correct here: with BaseButton a real base, VC6 emits both vtable restores and then TAIL-JUMPS to ??1BaseButton@@QAE@XZ, which is the image's `jmp 0x00607040`. Spelled against raw storage the destructor was trivial and compiled to a bare `ret`.
// symbol    ??1ImageButton@@UAE@XZ
// size      21 bytes
// kind      game
Return Value: n/a
Status: Complete
*/
ImageButton::~ImageButton() {
}

/*
Purpose: Step the receiver back to the subobject ??_GImageButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x006256F0 ??_GImageButton@@WEEE@AEPAXI@Z 0x006256F0-0x006256FB BYTE_EXACT
// symbol    ??_EImageButton@@WEEE@AEPAXI@Z
// CORRECTED from ??3ImageButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x006256D0` into
//   ??_GImageButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

/*
Purpose: Initialise the button from a borrowed Sprite. Store it in sprite_,
         as the explicit-size overload does, then pass its displayed width
         and height at 0x18 and 0x1c to the base initialiser.
// ORIGINAL: 0x00625380 sub_625380 0x00625380-0x006253C6 BYTE_EXACT
// symbol    ?init@ImageButton@@QAEHPAVSprite@@PBDHHHPAVWin@@H@Z
// size      70 bytes
// prototype int (__thiscall sub_625380)(ImageButton* this, const int*, LPCSTR, int, int, int, Win*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00607210
// LEVER: the receiver is an ImageButton, not a Texture and not BaseButton.
//   Three facts, and the last one is the one the shortlist missed: the store
//   lands at [this+0xAB8], which is `field_AB8_` - a member BaseButton does
//   not have, because sizeof(BaseButton) is exactly 0xAB8 and 0xAB8 is
//   therefore PAST the end of one; 0x00625380 sits immediately after
//   0x00625340, ?init@ImageButton@@QAEHHPADHHHHHPAUWin@@H@Z, in the image;
//   and the same slot is what the nine-argument overload stores. Spelled as
//   a member, the body needs no `this` pun and no ButtonInit stand-in class.
Return Value: whatever BaseButton::init returns
Status: Complete
*/
int ImageButton::init(Sprite *sprite, LPCSTR name, int id, int x, int y,
                      Win *parent, int style_flag) {
    if (sprite == 0) {
        return 3;
    }
    sprite_ = sprite;
    int height = sprite->height();
    int width = sprite->width();
    return BaseButton::init(name, id, x, y, width, height, parent,
                            style_flag);
}

/*
Purpose: Clear the field, then close as a BaseButton.

             mov dword [ecx+0xab8],0 / jmp ?close@BaseButton@@QAEXXZ

         A TAIL jump, so BaseButton::close's return value is this function's -
         but the mangled name says `X`, void, so nothing reads it. Declared
         void to match the name, and the value is discarded rather than
         quietly re-typed.

// ORIGINAL: 0x00625330 ?close@ImageButton@@QAEXXZ 0x00625330-0x0062533F BYTE_EXACT
// symbol    ?close@ImageButton@@QAEXXZ
// size      15 bytes
// prototype void (__thiscall ?close@ImageButton@@QAEXXZ)(ImageButton* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ImageButton::close() {
    sprite_ = 0;
    BaseButton::close();
}

/*
Purpose: Keep the first argument, hand the other eight to BaseButton::init.

             mov eax,[esp+4] / mov edx,[esp+0x24] / mov [ecx+0xab8],eax
             ...eight interleaved reads and pushes... / call BaseButton::init
             ret 0x24

         The reads and pushes alternate because each `[esp+0x20]` is taken
         AFTER a push has moved ESP - so they walk backwards through the
         arguments and re-push them in order. Nine arguments in (`ret 0x24`),
         the first kept at 0xab8 and the remaining eight passed straight on.

// ORIGINAL: 0x00625340 ?init@ImageButton@@QAEHHPADHHHHHPAUWin@@H@Z 0x00625340-0x0062537A BYTE_EXACT
// symbol    ?init@ImageButton@@QAEHPAVSprite@@PBDHHHHHPAVWin@@H@Z
// size      58 bytes
// prototype int (__thiscall ?init@ImageButton@@QAEHHPADHHHHHPAUWin@@H@Z)(ImageButton* this, int, int8*, int, int, int, int, int, Win*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00607210
Return Value: whatever BaseButton::init returns
Status: Complete
*/
int ImageButton::init(Sprite *sprite, LPCSTR name, int id, int x, int y,
                      int width, int height, Win *parent, int style_flag) {
    sprite_ = sprite;
    return BaseButton::init(name, id, x, y, width, height, parent, style_flag);
}
