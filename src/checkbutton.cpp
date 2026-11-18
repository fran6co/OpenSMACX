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
#include "checkbutton.h"
#include "checkbox.h"

// Application-region copies of JACKAL.LIB's CheckButton. The image places the
// destructor, its deleting entry and the Buffer-side adjustor at
// 0x004B3EC0..0x004B3F8B, inside the game's SocialWin object, not in
// CheckButton's library member (src/jackal/checkbutton.cpp, 0x00633750..).
// The originals were header-inline COMDATs. The destructor stays out of line
// in this application unit, which therefore also emits the class's deleting
// entries, until the header form is restored.

// ORIGINAL: 0x004B3F20 ??_GCheckButton@@UAEPAXI@Z 0x004B3F20-0x004B3F41 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004B3F20-0x004B3F41", "Pinned class constructor ??0CheckButton@@QAE@XZ at 0x00633750 stores original class/view table VA 0x66dc14 at 0x0063375A (mov dword ptr [esi], 0x66dc14); the current genuine owner vftable ??_7CheckButton@@6BWin@@@ selects generated ??_GCheckButton@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_ECheckButton@@UAEPAXI@Z (mode 2).", "Pinned ??_GCheckButton@@UAEPAXI@Z at 0x004B3F20 calls the source-mapped ??1CheckButton@@QAE@XZ at 0x004B3EC0, then tests byte ptr [ebp + 8], 1; je 0x4b3f3a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/jackal/checkbutton.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original CheckButton scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GCheckButton@@UAEPAXI@Z
// kind      game

/*
Purpose: Tear down CheckButton and its GraphicWin base through ordinary C++
         destruction, preserving the compiler's exception-cleanup path.
// ORIGINAL: 0x004B3EC0 ??1CheckButton@@QAE@XZ 0x004B3EC0-0x004B3F18;0x00659880-0x00659892 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["source:src/recovered/units/004b3ec0.cpp","original:0x004B3EC0-0x004B3F18","source:src/jackal/checkbutton.h:CheckButton"],"reason":"Let C++ emit the base teardown and exception cleanup instead of patching vtable addresses and manually invoking a base destructor."}
// symbol    ??1CheckButton@@UAE@XZ
*/
#pragma inline_depth(0)
CheckButton::~CheckButton() {
    close();
}
#pragma inline_depth(8)

/*
Purpose: Step the receiver back to the subobject ??_GCheckButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004B3F80 ??_GCheckButton@@WEEE@AEPAXI@Z 0x004B3F80-0x004B3F8B BYTE_EXACT
// symbol    ??_ECheckButton@@WEEE@AEPAXI@Z
// CORRECTED from ??3CheckButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004B3F20` into
//   ??_GCheckButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
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
