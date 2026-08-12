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
#include "deletionlist.h"
#include <cstring>

/*
Purpose: Reset one deletion-list entry to its empty state - an 0xFF marker byte
         and a zeroed word - at the record stride of 0x3C.
ORIGINAL: 0x0052DCD0 BYTE_EXACT
// name      ?clear@DeletionList@@QAEXH@Z
// size      28 bytes
// spans     0x0052DCD0-0x0052DCEC
// prototype void (__thiscall ?clear@DeletionList@@QAEXH@Z)(DeletionList* this, int)
// callers   1   call targets   0
Return Value: n/a
Status: Complete
*/
void DeletionList::clear(int index) {
    uint8_t *const entry =
        reinterpret_cast<uint8_t *>(this) + static_cast<size_t>(index) * 0x3C;
    const uint8_t marker = 0xFF;
    std::memcpy(entry, &marker, sizeof(marker));
    const uint16_t zero = 0;
    std::memcpy(entry + 8, &zero, sizeof(zero));
}

void __fastcall deletion_list_clear_redirect(DeletionList *self, void *, int index) {
    self->clear(index);
}
