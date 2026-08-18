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
#include "playerlock.h"
#include "lock.h"

/*
Purpose: Reset both lock entries to their unset sentinels and mark the lock
         inactive.
// ORIGINAL: 0x0058FF70 ?clear@PlayerLock@@QAEXXZ 0x0058FF70-0x0058FF96
// size      38 bytes
// prototype void (__thiscall ?clear@PlayerLock@@QAEXXZ)(PlayerLock* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void PlayerLock::clear() {
    for (size_t index = 0; index < 2; ++index) {
        entries_[index].first = -1;
        entries_[index].second = -1;
        entries_[index].flag = 0;
    }
    active_ = 0;
}

void __fastcall player_lock_clear_redirect(PlayerLock *self, void *) {
    self->clear();
}

/*
Purpose: Report whether either lock entry is engaged - the low bit of its flag
         set. The original is typed as returning void but computes this boolean
         in EAX, so the value is returned here to match its behaviour rather
         than its declared type.
// ORIGINAL: 0x0058FFA0 ?active@PlayerLock@@QAEXXZ 0x0058FFA0-0x0058FFBC BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?active@PlayerLock@@QAEXXZ)(PlayerLock* this)
// callers   4   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 1 when an entry is engaged, 0 when neither is
Status: Complete
*/
int PlayerLock::active() {
    for (int index = 0; index < 2; ++index) {
        if (entries_[index].flag & 1) {
            return 1;
        }
    }
    return 0;
}

int __fastcall player_lock_active_redirect(PlayerLock *self, void *) {
    return self->active();
}

/*
Purpose: Lock the second square entry for a faction, forcing the 0x10 flag on.

             mov edx,[ebp+8] / or al,0x10 / add ecx,0x10
             push .. x4 / call SquareLock::lock / ret 0x10

         `add ecx, 0x10` selects entries_[1] - the entries start at 4 and are
         twelve bytes each - and `or al, 0x10` sets bit 4 of the flags. AL is
         the low byte of the flags register, and bit 4 lives there, so setting
         it on the whole word is the same edit.

         Called through the LockSquareLock seam, exactly as Lock::add_lock does
         for the identical shape: PlayerLock::Entry and SquareLock are the same
         three-int triple, and routing through the seam is what lets a fixture
         observe the call rather than the map it would otherwise walk.
// ORIGINAL: 0x005900A0 ?add_lock@PlayerLock@@QAEHHHHH@Z 0x005900A0-0x005900C1
// size      33 bytes
// prototype int (__thiscall ?add_lock@PlayerLock@@QAEHHHHH@Z)(PlayerLock* this, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0058FE80
Return Value: whatever SquareLock::lock returns
Status: Complete
*/
int PlayerLock::add_lock(int factionID, int flags, int x, int y) {
    return (ORIGINAL(&entries_[1])->*LockSquareLock)(factionID, flags | 0x10, x, y);
}

int __fastcall player_lock_add_lock_redirect(PlayerLock *self, void *,
                                             int factionID, int flags,
                                             int x, int y) {
    return self->add_lock(factionID, flags, x, y);
}

/*
Purpose: Release both square entries for a faction and clear the active byte.

         The original walks the two entries with a counted loop from `this+4`
         in steps of twelve, then stores zero at `this+0` AFTER the loop - the
         byte write is the last thing it does, which is the ordering a fixture
         has to hold it to.
// ORIGINAL: 0x0058FFC0 ?unlock@PlayerLock@@QAEXH@Z 0x0058FFC0-0x0058FFF2
// size      50 bytes
// prototype void (__thiscall ?unlock@PlayerLock@@QAEXH@Z)(PlayerLock* this, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0058FD90
Return Value: n/a
Status: Complete
*/
void PlayerLock::unlock(int factionID) {
    for (int entry = 0; entry < 2; ++entry) {
        (ORIGINAL(&entries_[entry])->*LockSquareUnlock)(factionID);
    }
    active_ = 0;
}

void __fastcall player_lock_unlock_redirect(PlayerLock *self, void *,
                                            int factionID) {
    self->unlock(factionID);
}
