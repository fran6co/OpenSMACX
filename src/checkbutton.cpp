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

const uint32_t CheckButtonPrimaryVtable = 0x0066DC14;
const uint32_t CheckButtonBufferVtable = 0x0066DC0C;
// The two defaults the constructor reads once and copies into the object:
// unnamed globals the image reads by fixed address and never writes back.
static int *const g_00697f20 = (int *)0x00697F20;
static int *const g_00697f24 = (int *)0x00697F24;

/*
Purpose: Construct the GraphicWin base, then install CheckButton's own
         vtables and the trailing toggle-state fields.
// ORIGINAL: 0x00633750 ??0CheckButton@@QAE@XZ 0x00633750-0x0063379D BYTE_EXACT
// symbol    ??0CheckButton@@QAE@XZ
// size      77 bytes
// prototype void (__thiscall ??0CheckButton@@QAE@XZ)(CheckButton* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4CF0
Return Value: n/a
Status: Complete
*/
CheckButton::CheckButton() {
    field_A14_ = 0;
    isToggled_ = 0;
    field_A1C_ = 0;
    field_A20_ = 0;
    field_A28_ = *g_00697f24;
    field_A24_ = *g_00697f20;
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x006339B0 ?close_class@CheckButton@@QAAXXZ 0x006339B0-0x006339B1 BYTE_EXACT
// symbol    ?close_class@CheckButton@@SAXXZ
// size      1 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CheckButton::close_class() {
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x006339A0 ?init_class@CheckButton@@QAAHXZ 0x006339A0-0x006339A3 BYTE_EXACT
// symbol    ?init_class@CheckButton@@SAHXZ
// size      3 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int CheckButton::init_class() {
    return 0;
}


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
