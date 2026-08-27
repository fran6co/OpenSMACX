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
#include "editbox.h"
#include "graphicwin.h"
#include "time.h"

/*
 * THE SEVEN GLOBALS EditBox::close REPUBLISHES, and why they are named after
 * their DESTINATIONS rather than after anything they mean.
 *
 * Every member they land in is `field_BXX_` - the class carries no recovered
 * name for any of them - so a name claiming a purpose would be invented. What
 * IS known is which field each one feeds, and that all seven are read at close
 * time to republish per-instance defaults, the same role EditGroupDefault
 * plays for EditGroup.
 *
 * 0x006971DC-0x006971F8 is one contiguous run in .data; 0x009B90B8 and
 * 0x009B90BC sit apart from it, in the zero-filled range.
 *
 * Naming them costs nothing in bytes: the operand is a relocation, and the
 * comparison masks relocated bytes on both sides.
 */
uint32_t EditBoxDefaultB1C = 255;  // 0x006971DC
uint32_t EditBoxDefaultB20 = -1;  // 0x006971F8
uint32_t EditBoxDefaultB24 = -1;  // 0x006971F4
uint32_t EditBoxDefaultB28;  // 0x009B90B8
uint32_t EditBoxDefaultB2C;  // 0x009B90BC
uint32_t EditBoxDefaultB30 = 1;  // 0x006971E0
uint32_t EditBoxDefaultB34 = 4;  // 0x006971E4

/*
Purpose: Reset the edit box - clear its state, republish the seven defaults,
         then close the trailing Time and the GraphicWin prefix.
// ORIGINAL: 0x00614F30 ?close@EditBox@@QAEXXZ 0x00614F30-0x00614FD1 BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/00614f30.cpp, whose claim proved the ARTIFACT reproduced while the program held a pending_bodies forwarder. src/editbox.cpp did not exist - EditBox was header-only - so this file and its CMakeLists entry are new.
// symbol    ?close@EditBox@@QAEXXZ
// size      161 bytes
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00616780
Return Value: n/a
Status: Complete
*/
void EditBox::close() {
    char *self = reinterpret_cast<char *>(this);

    // A BYTE STORE, not a dword one. The image writes `mov byte ptr
    // [esi + 0xa14], al` while field_A14_ is declared uint32_t, so naming
    // the member is right but dropping the width is not.
    *reinterpret_cast<unsigned char *>(&field_A14_) = 0;
    field_B14_ = 0xa;
    field_B28_ = EditBoxDefaultB28;
    field_B2C_ = EditBoxDefaultB2C;
    field_B30_ = EditBoxDefaultB30;
    field_B34_ = EditBoxDefaultB34;
    field_B1C_ = EditBoxDefaultB1C;
    field_B20_ = EditBoxDefaultB20;
    field_B24_ = EditBoxDefaultB24;
    field_B38_ = 0;
    field_B40_ = 0;
    field_B44_ = 0;
    field_B48_ = 0;
    field_B18_ = 0;
    field_B3C_ = 0;

    reinterpret_cast<Time *>(self + 0xb4c)->close();
    reinterpret_cast<GraphicWin *>(self)->close();
}
/*
Purpose: Does this window hold the dialog focus, and its partner too?

             mov esi,ecx / call Win::is_dialog_focus / test eax,eax / je no
             mov ecx,[esi+0xc4] / test ecx,ecx / je yes
             call Win::is_dialog_focus / test eax,eax / je no
             yes: mov eax,1 / ret        no: xor eax,eax / ret

         The second call is made on the window at field 0xc4 - which
         src/win.h names `win_parent_` - not on `this` again; ECX is reloaded
         from it just before. A NULL parent is not a failure: the answer is
         yes.

// ORIGINAL: 0x006161A0 ?UNK2@EditBox@@QAEXXZ 0x006161A0-0x006161CA
// symbol    ?leaf_006161a0_redirect@@YIHPAX0@Z
// TRIED (2026-08-26, deleting win_is_dialog_focus_redirect and calling
// Win::is_dialog_focus directly): 2/17 -> 10/17 agreeing, 20 -> 18 compiled
// instructions. What remains is the wrapper's own tail: ours ends
// `push 1 / pop eax` where the image has `mov eax,1`, and the two early
// `je` distances differ by that one instruction. Restructuring to the
// image's exact arm order (null-parent arm returning 1 as its own `if`)
// REGRESSED to 4/17 with 19 instructions under /O2 - reverted. Plateau.
// size      42 bytes
// prototype void (__thiscall ?UNK2@EditBox@@QAEXXZ)(EditBox* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F2CA0
Return Value: 1 when both hold focus, 0 otherwise
Status: Complete
*/
int __fastcall leaf_006161a0_redirect(void *self, void *) {
    if (reinterpret_cast<Win *>(self)->is_dialog_focus() == 0) {
        return 0;
    }
    Win *const parent = *reinterpret_cast<Win *const *>(
        static_cast<uint8_t *>(self) + 0xC4);
    if (parent != nullptr && parent->is_dialog_focus() == 0) {
        return 0;
    }
    return 1;
}
