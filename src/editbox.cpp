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
uint32_t EditBoxDefaultB1C;  // 0x006971DC
uint32_t EditBoxDefaultB20;  // 0x006971F8
uint32_t EditBoxDefaultB24;  // 0x006971F4
uint32_t EditBoxDefaultB28;  // 0x009B90B8
uint32_t EditBoxDefaultB2C;  // 0x009B90BC
uint32_t EditBoxDefaultB30;  // 0x006971E0
uint32_t EditBoxDefaultB34;  // 0x006971E4

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

    *reinterpret_cast<unsigned char *>(self + 0xa14) = 0;
    *reinterpret_cast<int *>(self + 0xb14) = 0xa;
    *reinterpret_cast<int *>(self + 0xb28) = EditBoxDefaultB28;
    *reinterpret_cast<int *>(self + 0xb2c) = EditBoxDefaultB2C;
    *reinterpret_cast<int *>(self + 0xb30) = EditBoxDefaultB30;
    *reinterpret_cast<int *>(self + 0xb34) = EditBoxDefaultB34;
    *reinterpret_cast<int *>(self + 0xb1c) = EditBoxDefaultB1C;
    *reinterpret_cast<int *>(self + 0xb20) = EditBoxDefaultB20;
    *reinterpret_cast<int *>(self + 0xb24) = EditBoxDefaultB24;
    *reinterpret_cast<int *>(self + 0xb38) = 0;
    *reinterpret_cast<int *>(self + 0xb40) = 0;
    *reinterpret_cast<int *>(self + 0xb44) = 0;
    field_B48_ = 0;
    *reinterpret_cast<int *>(self + 0xb18) = 0;
    *reinterpret_cast<int *>(self + 0xb3c) = 0;

    reinterpret_cast<Time *>(self + 0xb4c)->close();
    reinterpret_cast<GraphicWin *>(self)->close();
}
