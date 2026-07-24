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
uint32_t *LockEnableMask = reinterpret_cast<uint32_t *>(0x009A64E8);
func_square_lock_unlock *LockSquareUnlock =
    (func_square_lock_unlock *)0x0058FD90;
func_square_lock_lock *LockSquareLock = (func_square_lock_lock *)0x0058FE80;
func_current_server *LockCurrentServer = (func_current_server *)0x0052DBA0;
func_message_data *LockMessageData = (func_message_data *)0x00592EE0;

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

/*
Purpose: Reset the lock to empty - clear its three trailing dwords, reset all
         eight records to their unset sentinels, and run the global map reset.
Original Offset: 005900D0
Return Value: n/a
Status: Complete
*/
void Lock::clear() {
    field_E0_ = 0;
    field_E4_ = 0;
    field_E8_ = 0;
    for (int record = 0; record < 8; ++record) {
        records_[record].flag = 0;
        for (int entry = 0; entry < 2; ++entry) {
            records_[record].entries[entry].first = -1;
            records_[record].entries[entry].second = -1;
            records_[record].entries[entry].flag = 0;
        }
    }
    reset_map();
}

void __fastcall lock_clear_redirect(Lock *self, void *) {
    self->clear();
}

/*
Purpose: Report whether any lock is engaged. The trailing dword at 0xE0 forces
         a yes on its own; otherwise records 1 through 7 are scanned, but only
         those whose slot bit is set in both the low and second bytes of the
         enable mask, and a record counts when either of its two entries has
         its flag's low bit set. Record 0 is never scanned.
Original Offset: 005904A0
Return Value: 1 when a lock is engaged, 0 otherwise
Status: Complete
*/
int Lock::any_locks() {
    if (field_E0_ != 0) {
        return 1;
    }
    const uint32_t mask = *LockEnableMask;
    const uint32_t low_byte = mask & 0xFF;
    const uint32_t second_byte = (mask >> 8) & 0xFF;
    for (int index = 1; index < 8; ++index) {
        const uint32_t bit = 1u << index;
        if (!(bit & low_byte) || !(bit & second_byte)) {
            continue;
        }
        for (int entry = 0; entry < 2; ++entry) {
            if (records_[index].entries[entry].flag & 1) {
                return 1;
            }
        }
    }
    return 0;
}

int __fastcall lock_any_locks_redirect(Lock *self, void *) {
    return self->any_locks();
}

/*
Purpose: Release the lock on one slot - forget it as the active lock if it was,
         unlock both of the slot record's square entries, and clear the
         record's flag byte.
Original Offset: 00590170
Return Value: n/a
Status: Complete
*/
void Lock::unlock(int slot) {
    if (field_E0_ == static_cast<uint32_t>(slot)) {
        field_E0_ = 0;
        field_E4_ = 0;
    }
    Record &record = records_[slot];
    for (int entry = 0; entry < 2; ++entry) {
        LockSquareUnlock(&record.entries[entry], slot);
    }
    record.flag = 0;
}

void __fastcall lock_unlock_redirect(Lock *self, void *, int slot) {
    self->unlock(slot);
}

/*
Purpose: Take the global lock for an owner. Succeeds when the lock is free or
         already held by that owner, recording the owner and marking it held;
         fails without change when another owner holds it.
Original Offset: 005902C0
Return Value: 0 on success, 1 when another owner holds the lock
Status: Complete
*/
int Lock::global_lock(int owner) {
    if (field_E0_ != 0 && field_E0_ != static_cast<uint32_t>(owner)) {
        return 1;
    }
    field_E0_ = static_cast<uint32_t>(owner);
    field_E4_ = 1;
    return 0;
}

int __fastcall lock_global_lock_redirect(Lock *self, void *, int owner) {
    return self->global_lock(owner);
}

/*
Purpose: Report whether the global lock this owner holds can now be released -
         true only on the server, when the lock is marked held by this owner
         and none of its slot records still has an active square. On success it
         also drops the held flag. Any other case reports no.
Original Offset: 00590240
Return Value: 1 when the lock may be released, 0 otherwise
Status: Complete
*/
int Lock::check_global_2(int owner) {
    if (LockCurrentServer() == 0) {
        return 0;
    }
    if (field_E4_ == 0) {
        return 0;
    }
    if (field_E0_ != static_cast<uint32_t>(owner)) {
        return 0;
    }
    for (int index = 1; index < 8; ++index) {
        for (int entry = 0; entry < 2; ++entry) {
            if (records_[index].entries[entry].flag & 1) {
                return 0;
            }
        }
    }
    field_E4_ = 0;
    return 1;
}

int __fastcall lock_check_global_2_redirect(Lock *self, void *, int owner) {
    return self->check_global_2(owner);
}

/*
Purpose: On the server, when a held lock has no square still active, broadcast
         its release and drop the held flag. Does nothing off the server, when
         no lock is held, or while any slot record still has an active square.
Original Offset: 005901D0
Return Value: n/a
Status: Complete
*/
void Lock::check_global() {
    if (LockCurrentServer() == 0) {
        return;
    }
    if (field_E4_ == 0) {
        return;
    }
    for (int index = 1; index < 8; ++index) {
        for (int entry = 0; entry < 2; ++entry) {
            if (records_[index].entries[entry].flag & 1) {
                return;
            }
        }
    }
    LockMessageData(0x1205, static_cast<int>(field_E0_), 0, 0, 0, 0);
    field_E4_ = 0;
}

void __fastcall lock_check_global_redirect(Lock *self, void *) {
    self->check_global();
}

/*
Purpose: Add a lock on one slot - forward to SquareLock::lock on the slot
         record's second square entry, with the mask bit 0x10 forced into the
         flags argument.
Original Offset: 00590470
Return Value: whatever SquareLock::lock returns
Status: Complete
*/
int Lock::add_lock(int slot, int flags, int a3, int a4) {
    return LockSquareLock(&records_[slot].entries[1], slot, flags | 0x10, a3, a4);
}

int __fastcall lock_add_lock_redirect(Lock *self, void *, int slot, int flags,
                                      int a3, int a4) {
    return self->add_lock(slot, flags, a3, a4);
}
