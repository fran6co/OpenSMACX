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
#include "basepop.h"

// Application-region pieces of JACKAL.LIB's BasePop:
// - BasePop::add_asterisk_item (0x0059D230) lies between the game's Path and
//   PopMenu objects, far from BasePop's library member
//   (src/jackal/basepop.cpp, 0x00600860..). The original was a header-inline
//   COMDAT; it stays out of line in this application unit until the header
//   form is restored.
// - Its "*" marker lives in the game's .data (0x006900C4), not in Jackal's.

// Pinned 0x006900C4 bytes are writable .data `2A 00`; only the observed two bytes are asserted. Old symbol/capacity/whole-use identity remain unknown pending native COFF object binding.
// RECOVERY_OBJECT: v1 0x006900C4 ?BasePopItemMarkerText@@3PADA
char BasePopItemMarkerText[2] = {'*', '\0'};

/*
Purpose: Add one fixed item to the Dialogs at 0x21d0.

             push -2 / push 0x6900c4 / add ecx,0x21d0
             call ?item@Dialogs@@QAEHPADH@Z / ret

         A tail call in everything but name: the callee cleans its own two
         arguments and this function cleans none, so its `ret` carries the
         callee's result straight out.

         -2 is the index. It is not a count or a flag, and it is negative,
         which is the sort of thing that gets "tidied" to 2 or 0 by someone
         reading quickly.


// ORIGINAL: 0x0059D230 sub_59d230 0x0059D230-0x0059D243 BYTE_EXACT
// symbol    ?add_asterisk_item@BasePop@@QAEHXZ
// size      19 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00612A70
Return Value: whatever Dialogs::item returns
Status: Complete
*/
int BasePop::add_asterisk_item() {
    return dialogs_.Dialogs::item(BasePopItemMarkerText, -2);
}
