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
#include "lock.h"


/*
Purpose: Drop the movement bits (0x38) from the flag byte at offset 5 of every
         map-tile record, at the record stride of 0x2C. Operates entirely on
         the global map table, ignoring the Lock instance.
// ORIGINAL: 0x00590140 ?reset_map@Lock@@QAEXXZ 0x00590140-0x0059016B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?reset_map@Lock@@QAEXXZ)(Lock* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Lock::reset_map() {
    // NOT CACHED: the image RE-READS *LockMapCount at every iteration's
    // test rather than hoisting it into a local once.
    // SEE lock.h: the extra cast is what keeps LockMapTable's fold.
    uint8_t *const table = *reinterpret_cast<uint8_t *const *>(LockMapTable);
    for (int32_t index = 0; index < *LockMapCount; ++index) {
        table[index * 0x2C + 5] &= 0xC7;
    }
}

void __fastcall lock_reset_map_redirect(Lock *self, void *) {
    self->reset_map();
}

/*
Purpose: Reset the lock to empty - clear its three trailing dwords, reset all
         eight records to their unset sentinels, and run the global map reset.
// ORIGINAL: 0x005900D0 ?clear@Lock@@QAEXXZ 0x005900D0-0x0059013D BYTE_EXACT
// size      109 bytes
// prototype void (__thiscall ?clear@Lock@@QAEXXZ)(Lock* this)
// callers   12   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Lock::clear() {
    field_E0_ = 0;
    field_E4_ = 0;
    field_E8_ = 0;
    for (int record = 0; record < 8; ++record) {
        // IMAGE ORDER: both entries' flag/second/first, THEN the record's
        // own flag byte last - not the record's flag first.
        for (int entry = 0; entry < 2; ++entry) {
            records_[record].entries[entry].flag = 0;
            records_[record].entries[entry].second = -1;
            records_[record].entries[entry].first = -1;
        }
        records_[record].flag = 0;
    }
    // reset_map()'S BODY, INLINED: `osmx calls` names zero call targets.
    // NOT CACHED: see reset_map() above.
    uint8_t *const table = *reinterpret_cast<uint8_t *const *>(LockMapTable);
    for (int32_t index = 0; index < *LockMapCount; ++index) {
        table[index * 0x2C + 5] &= 0xC7;
    }
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
// ORIGINAL: 0x005904A0 ?any_locks@Lock@@QAEHXZ 0x005904A0-0x005904FE SEMANTIC
// LEVER: `second_byte` read via a byte pointer into `mask` (not `(mask >> 8) & 0xFF`) is what lets VC6 test it as `bh` with no shift, matching the image's `test al, bh` exactly.
// TRIED: the one remaining byte - image's inner-loop test is `test eax, edi` (85 F8), this tree compiles the same bits as `test edi, eax` (85 C7), same flags, different ModRM. Tried swapping `&` operand order both ways (`bit & low_byte` and `low_byte & bit`) and naming the AND result in its own local first - all three collapse to the same encoding, so it is not a source-order question; MNEMONIC_ONLY at 39/40 (only that one byte) is the plateau reached here.
// size      94 bytes
// prototype int (__thiscall ?any_locks@Lock@@QAEHXZ)(Lock* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when a lock is engaged, 0 otherwise
Status: Complete
*/
int Lock::any_locks() {
    if (field_E0_ != 0) {
        return 1;
    }
    const uint32_t mask = *LockEnableMask;
    const uint32_t low_byte = mask & 0xFF;
    const uint8_t second_byte = reinterpret_cast<const uint8_t *>(&mask)[1];
    for (int index = 1; index < 8; ++index) {
        const uint32_t bit = 1u << index;
        if (!(bit & low_byte) || !(second_byte & bit)) {
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
// ORIGINAL: 0x00590170 ?unlock@Lock@@QAEXH@Z 0x00590170-0x005901C3 BYTE_EXACT
// size      83 bytes
// prototype void (__thiscall ?unlock@Lock@@QAEXH@Z)(Lock* this, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0058FD90
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
        record.entries[entry].unlock(slot);
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
// ORIGINAL: 0x005902C0 ?global_lock@Lock@@QAEHH@Z 0x005902C0-0x005902F3 BYTE_EXACT
// size      51 bytes
// prototype int (__thiscall ?global_lock@Lock@@QAEHH@Z)(Lock* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00590240 ?check_global_2@Lock@@QAEHH@Z 0x00590240-0x005902BE BYTE_EXACT
// size      126 bytes
// prototype int (__thiscall ?check_global_2@Lock@@QAEHH@Z)(Lock* this, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052DBA0
Return Value: 1 when the lock may be released, 0 otherwise
Status: Complete
*/
int Lock::check_global_2(int owner) {
    if (current_server() == 0) {
        return 0;
    }
    if (field_E4_ == 0) {
        return 0;
    }
    if (static_cast<uint32_t>(owner) != field_E0_) {
        return 0;
    }
    for (int index = 1; index < 8; ++index) {
        // Skip the record whose slot IS the global lock's own owner.
        if (index == static_cast<int>(field_E0_)) {
            continue;
        }
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
// ORIGINAL: 0x005901D0 ?check_global@Lock@@QAEXXZ 0x005901D0-0x00590239 BYTE_EXACT
// size      105 bytes
// prototype void (__thiscall ?check_global@Lock@@QAEXXZ)(Lock* this)
// callers   15   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0052DBA0 0x00592EE0
Return Value: n/a
Status: Complete
*/
void Lock::check_global() {
    if (current_server() == 0) {
        return;
    }
    if (field_E4_ == 0) {
        return;
    }
    for (int index = 1; index < 8; ++index) {
        // Skip the record whose slot IS the global lock's own owner.
        if (index == static_cast<int>(field_E0_)) {
            continue;
        }
        for (int entry = 0; entry < 2; ++entry) {
            if (records_[index].entries[entry].flag & 1) {
                return;
            }
        }
    }
    // IMAGE ORDER: field_E4_ cleared BEFORE the call, though its arguments
    // are already pushed by then.
    field_E4_ = 0;
    message_data(0x1205, static_cast<int>(field_E0_), 0, 0, 0, 0);
}

void __fastcall lock_check_global_redirect(Lock *self, void *) {
    self->check_global();
}

/*
Purpose: Add a lock on one slot - forward to SquareLock::lock on the slot
         record's second square entry, with the mask bit 0x10 forced into the
         flags argument.
// ORIGINAL: 0x00590470 ?add_lock@Lock@@QAEHHHHH@Z 0x00590470-0x0059049B BYTE_EXACT
// size      43 bytes
// prototype int (__thiscall ?add_lock@Lock@@QAEHHHHH@Z)(Lock* this, int, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0058FE80
Return Value: whatever SquareLock::lock returns
Status: Complete
*/
int Lock::add_lock(int slot, int flags, int a3, int a4) {
    return records_[slot].entries[1].lock(slot, flags | 0x10, a3, a4);
}

int __fastcall lock_add_lock_redirect(Lock *self, void *, int slot, int flags,
                                      int a3, int a4) {
    return self->add_lock(slot, flags, a3, a4);
}

/*
Purpose: Lock both of a slot's square entries in one call. Refuses when another
         owner holds the global lock. The 0x2 flag bit takes the global lock
         for this slot as owner. Each entry's square is reset to its unset
         sentinels then locked - the first with (flags, a3, a4), the second with
         (a5, a6, a7); if either fails, both are unlocked, the record cleared,
         and the global lock this slot held is dropped. On full success, when
         the 0x2 bit was set and this is the server, the same release check
         check_global runs (but skipping this slot's own record) fires the
         release broadcast if no other slot still holds an active square.
// ORIGINAL: 0x00590300 ?lock@Lock@@QAEHHHHHHHH@Z 0x00590300-0x00590469
// size      361 bytes
// prototype int (__thiscall ?lock@Lock@@QAEHHHHHHHH@Z)(Lock* this, int, int, int, int, int, int, int)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052DBA0 0x0058FD90 0x0058FE80 0x00592EE0
// TRIED: the whole-function shape - this catalogue's `frame` flag is
//        real (image opens `push ebp; mov ebp, esp` and addresses every
//        one of its 7 stack parameters `[ebp+N]`, reusing the dead `flags`
//        parameter slot at `[ebp+8]` for TWO further unrelated locals -
//        `take_global` then the unlock loop's counter). This tree's body,
//        logically equivalent (skip-owner loop and current_server() fix
//        both already applied, matching check_global/check_global_2's
//        siblings), compiles frame-omitted at every flag set in FLAG_SETS
//        including /Oy-, best 4/132 instructions, 0.749 similar. Not
//        chased past that in this pass - a bigger source restructuring
//        (matching the parameter-slot reuse explicitly) is the likely
//        lever, left for a pass that can afford the search.
Return Value: 1 on refusal or lock failure, 0 on success
Status: Complete
*/
int Lock::lock(int slot, int flags, int a3, int a4, int a5, int a6, int a7) {
    // Refuse when another owner already holds the global lock.
    if (field_E0_ != 0 && field_E0_ != static_cast<uint32_t>(slot)) {
        return 1;
    }
    const bool take_global = (flags & 2) != 0;
    if (take_global) {
        // The entry guard above already narrowed field_E0_ to 0 or slot, so
        // this mirrors the binary's second, redundant owner check.
        if (field_E0_ != 0 && field_E0_ != static_cast<uint32_t>(slot)) {
            return 1;
        }
        field_E0_ = static_cast<uint32_t>(slot);
        field_E4_ = 1;
    }

    Record &record = records_[slot];
    for (int entry = 0; entry < 2; ++entry) {
        record.entries[entry].first = -1;
        record.entries[entry].second = -1;
        record.entries[entry].flag = 0;
    }
    record.flag = 0;

    if (record.entries[0].lock(slot, flags, a3, a4) == 0 &&
        record.entries[1].lock(slot, a5, a6, a7) == 0) {
        if (take_global && current_server() != 0 && field_E4_ != 0) {
            const uint32_t owner = field_E0_;
            for (int index = 1; index < 8; ++index) {
                if (static_cast<uint32_t>(index) == owner) {
                    continue;
                }
                for (int entry = 0; entry < 2; ++entry) {
                    if (records_[index].entries[entry].flag & 1) {
                        return 0;
                    }
                }
            }
            message_data(0x1205, static_cast<int>(owner), 0, 0, 0, 0);
            field_E4_ = 0;
        }
        return 0;
    }

    // A square failed: unlock both entries, clear the record, and drop the
    // global lock if this slot holds it.
    for (entry = 0; entry < 2; ++entry) {
        record.entries[entry].unlock(slot);
    }
    record.flag = 0;
    if (field_E0_ == static_cast<uint32_t>(slot)) {
        field_E4_ = 0;
        field_E0_ = 0;
    }
    return 1;
}

int __fastcall lock_lock_redirect(Lock *self, void *, int slot, int flags,
                                  int a3, int a4, int a5, int a6, int a7) {
    return self->lock(slot, flags, a3, a4, a5, a6, a7);
}
