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
// ORIGINAL: 0x0058FF70 ?clear@PlayerLock@@QAEXXZ 0x0058FF70-0x0058FF96 BYTE_EXACT
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
    // IN THE IMAGE'S ORDER, which is the reverse of the declaration order:
    // `[eax+4]=0` then `[eax]=-1` then `[eax-4]=-1` at 0x0058FF78. VC6 emits
    // assignments where they are written, so the loop body has to be written
    // upside down to match.
    for (size_t index = 0; index < 2; ++index) {
        entries_[index].flag = 0;
        entries_[index].second = -1;
        entries_[index].first = -1;
    }
    active_ = 0;
}


// ORIGINAL: 0x00590000 ?lock@PlayerLock@@QAEHHHHHHHH@Z 0x00590000-0x00590093
// RECOVERY_BINDING: v1 {"evidence": ["Explicit seed_bindings delegation permits binding already-scoped seeds to worker-written bodies in existing editable product files; this identity and src/playerlock.cpp are already in task scope.", "Fresh installed-framework inspection verified the pinned image and decoded all 147 bytes at 0x00590000–0x00590093: two entry resets, ordered SquareLock::lock calls at 0x00590041 and 0x0059005A, and a two-entry unlock loop on either failure.", "src/playerlock.cpp:50–70 implements those operations through entries_[0] and entries_[1], clears active_ before acquisition and after rollback, and returns 0 on success or 1 on failure. The current definition line and file SHA-256 were read directly.", "The original identity remains seed-only in src/unrecovered/00590000.cpp. Binding preserves its spans/history and leaves the discovery pending for fresh measurement and review. Supplied checking remains exit 3, with Lock::lock byte/reference failures and unknown layout/build/runtime evidence; no source acceptance is proposed."], "reason": "The unannotated seven-argument PlayerLock::lock definition uses real SquareLock members and expresses the original reset, ordered acquisition, failure rollback and return behavior. Bind the existing scoped seed while retaining its original spans and history."}
// TRIED: real SquareLock member calls (first_/second_/flag_, lock/unlock) already declared by the scaffold; store order flag_,second_, first_ (not declaration order) matches the disassembly's init loop. MISMATCH #3 'push' vs 'mov' remains open.
int PlayerLock::lock(int factionID, int flags, int x1, int y1, int flags2,
                     int x2, int y2) {
    for (int index = 0; index < 2; ++index) {
        entries_[index].flag = 0;
        entries_[index].second = -1;
        entries_[index].first = -1;
    }
    active_ = 0;

    if (entries_[0].lock(factionID, flags, x1, y1) == 0) {
        if (entries_[1].lock(factionID, flags2, x2, y2) == 0) {
            return 0;
        }
    }

    for (int entry = 0; entry < 2; ++entry) {
        entries_[entry].unlock(factionID);
    }
    active_ = 0;
    return 1;
}


/*
Purpose: Report whether either lock entry is engaged - the low bit of its flag
         set. The original is typed as returning void but computes this boolean
         in EAX, so the value is returned here to match its behaviour rather
         than its declared type.
// ORIGINAL: 0x0058FFA0 ?active@PlayerLock@@QAEXXZ 0x0058FFA0-0x0058FFBC BYTE_EXACT
// symbol    ?active@PlayerLock@@QAEHXZ
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
// ORIGINAL: 0x005900A0 ?add_lock@PlayerLock@@QAEHHHHH@Z 0x005900A0-0x005900C1 BYTE_EXACT
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
    return entries_[1].lock(factionID, flags | 0x10, x, y);
}


/*
Purpose: Release both square entries for a faction and clear the active byte.

         The original walks the two entries with a counted loop from `this+4`
         in steps of twelve, then stores zero at `this+0` AFTER the loop - the
         byte write is the last thing it does, which is the ordering a fixture
         has to hold it to.
// ORIGINAL: 0x0058FFC0 ?unlock@PlayerLock@@QAEXH@Z 0x0058FFC0-0x0058FFF2 BYTE_EXACT
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
        entries_[entry].unlock(factionID);
    }
    active_ = 0;
}
