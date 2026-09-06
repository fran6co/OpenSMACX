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
    field_AB8_ = 0;
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
Purpose: Initialise the button from its definition record. The record goes
         into field_AB8_ - the same slot the nine-argument init overload fills
         with its own first argument - and the width and height the base
         initialiser wants are read back off that record at 0x18 and 0x1c,
         which is why this form takes no explicit pair.
// ORIGINAL: 0x00625380 sub_625380 0x00625380-0x006253C6 BYTE_EXACT
// symbol    ?init@ImageButton@@QAEHPBHPBDHHHPAVWin@@H@Z
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
int ImageButton::init(const int *imageDef, LPCSTR name, int id, int x, int y,
                      Win *parent, int style_flag) {
    if (imageDef == 0) {
        return 3;
    }
    field_AB8_ = reinterpret_cast<uint32_t>(imageDef);
    // The record's own width and height, at 0x18 and 0x1c, read straight off
    // it once and then handed to the base.
    int height = imageDef[7];
    int width = imageDef[6];
    return BaseButton::init(name, id, x, y, width, height, parent,
                            style_flag);
}
