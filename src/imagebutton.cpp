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
