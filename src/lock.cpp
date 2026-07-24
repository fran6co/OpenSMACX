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
#include "lock.h"

int32_t *LockMapCount = reinterpret_cast<int32_t *>(0x00949884);
uint8_t *LockMapTable = reinterpret_cast<uint8_t *>(0x0094A30C);

/*
Purpose: Drop the movement bits (0x38) from the flag byte at offset 5 of every
         map-tile record, at the record stride of 0x2C. Operates entirely on
         the global map table, ignoring the Lock instance.
Original Offset: 00590140
Return Value: n/a
Status: Complete
*/
void Lock::reset_map() {
    const int32_t count = *LockMapCount;
    for (int32_t index = 0; index < count; ++index) {
        uint8_t *const flag = LockMapTable + index * 0x2C + 5;
        *flag &= 0xC7;
    }
}

void __fastcall lock_reset_map_redirect(Lock *self, void *) {
    self->reset_map();
}
