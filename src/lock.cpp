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
#include "game.h"  // shared state-window owner
#include "lock.h"
#include "squarelock.h"
#include "map.h"
#include "playerlock.h"

/*
Purpose: Reset the square lock to its unset sentinels.
// ORIGINAL: 0x0058FD70 ?clear@SquareLock@@QAEXXZ 0x0058FD70-0x0058FD85 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?clear@SquareLock@@QAEXXZ)(SquareLock* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SquareLock::clear() {
    flag = 0;
    second = -1;
    first = -1;
}


/*
Purpose: Release the square this lock holds and every tile in its footprint.
         Does nothing when the lock bit (0x1) is clear. When the stored
         coordinate is off the map the record is simply cleared; otherwise
         unlock_map is called on each tile of the footprint - a single tile,
         or, when the 0x4 flag is set without the 0x10 flag, a radius of 25
         tiles (81 when the 0x8 flag is also set) walked through the shared
         RadiusOffset tables and wrapped in x by xrange. The record is reset to
         its unset sentinels on the way out.
// ORIGINAL: 0x0058FD90 ?unlock@SquareLock@@QAEXH@Z 0x0058FD90-0x0058FE77 BYTE_EXACT
// size      231 bytes
// prototype void (__thiscall ?unlock@SquareLock@@QAEXH@Z)(SquareLock* this, int)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591CF0
Return Value: n/a
Status: Complete
*/
void SquareLock::unlock(int factionID) {
    if (!(flag & 1)) {
        return;
    }
    const int x = first;
    const int y = second;
    if (y >= 0 && y < MapLatitudeBounds && x >= 0 && x < MapLongitudeBounds) {
        int count;
        if ((flag & 4) && !(flag & 0x10)) {
            count = (flag & 8) ? 81 : 25;   // RadiusRange[4] or RadiusRange[2]
        } else {
            count = 1;
        }
        for (int i = 0; i < count; ++i) {
            const int nx = wrap_map_x(RadiusOffsetX[i] + first);
            const int ny = RadiusOffsetY[i] + second;
            if (ny >= 0 && ny < MapLatitudeBounds &&
                nx >= 0 && nx < MapLongitudeBounds) {
                unlock_map(nx, ny, factionID);
            }
        }
    }
    flag = 0;
    second = -1;
    first = -1;
}


/*
Purpose: Take a square and every tile in its footprint for a faction. The
         coordinate and the flags (with the lock bit 0x1 forced on) are stored
         in the record first, unconditionally. An off-map coordinate then stops
         there. Otherwise lock_map is called on each footprint tile - a single
         tile, or a radius of 25 (0x4 set, 0x10 clear) or 81 (also 0x8 set)
         tiles walked through the shared RadiusOffset tables and wrapped in x by
         xrange; if any tile is already held by another faction the attempt is
         abandoned at that tile.
// ORIGINAL: 0x0058FE80 ?lock@SquareLock@@QAEHHHHH@Z 0x0058FE80-0x0058FF68 BYTE_EXACT
// size      232 bytes
// prototype BOOL (__thiscall ?lock@SquareLock@@QAEHHHHH@Z)(SquareLock* this, int factionID, int flags, int xCoord, int yCoord)
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591C90
Return Value: 1 when a footprint tile is already locked by another faction,
              0 otherwise (including an off-map coordinate)
Status: Complete
// LEVER: /Oy- is the winning flag set - the doc's own `frame` tag says the
//   image keeps a real ebp frame. Store order is flag/first/second (not
//   declaration order first/second/flag - `store_order.py` shows it), and
//   the loop-count tests read `flag & 4`/`flag & 8` but `flags & 0x10`.
*/
int SquareLock::lock(int factionID, int flags, int x, int y) {
    // STORE ORDER: flag, then first, then second - the image writes
    // `[esi+8]` (flag) before `[esi]`/`[esi+4]` (first/second), the
    // opposite of the field's declaration order.
    flag = flags | 1;
    first = x;
    second = y;
    if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
        return 0;
    }
    // Bits 4 and 8 are read from the stored `flag`; bit 0x10 from the
    // untouched parameter (`test bl, 0x10`).
    int count;
    if ((flag & 4) && !(flags & 0x10)) {
        count = (flag & 8) ? 81 : 25;   // RadiusRange[4] or RadiusRange[2]
    } else {
        count = 1;
    }
    for (int i = 0; i < count; ++i) {
        const int nx = wrap_map_x(RadiusOffsetX[i] + first);
        const int ny = RadiusOffsetY[i] + second;
        if (ny >= 0 && ny < MapLatitudeBounds &&
            nx >= 0 && nx < MapLongitudeBounds) {
            if (lock_map(nx, ny, factionID)) {
                return 1;
            }
        }
    }
    return 0;
}


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


// ORIGINAL: 0x00590000 ?lock@PlayerLock@@QAEHHHHHHHH@Z 0x00590000-0x00590093 BYTE_EXACT
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

// 0x009A64E8, the faction human/alive status dword




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
            records_[record].entries_[entry].flag = 0;
            records_[record].entries_[entry].second = -1;
            records_[record].entries_[entry].first = -1;
        }
        records_[record].active_ = 0;
    }
    // reset_map()'S BODY, INLINED: `osmx calls` names zero call targets.
    // NOT CACHED: see reset_map() above.
    Map *const table = map_tiles();
    for (int32_t index = 0; index < MapArea; ++index) {
        table[index].val3 &= 0xC7;
    }
}

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
    Map *const table = map_tiles();
    for (int32_t index = 0; index < MapArea; ++index) {
        table[index].val3 &= 0xC7;
    }
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
    PlayerLock &record = records_[slot];
    for (int entry = 0; entry < 2; ++entry) {
        record.entries_[entry].unlock(slot);
    }
    record.active_ = 0;
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
            if (records_[index].entries_[entry].flag & 1) {
                return;
            }
        }
    }
    // IMAGE ORDER: field_E4_ cleared BEFORE the call, though its arguments
    // are already pushed by then.
    field_E4_ = 0;
    message_data(0x1205, static_cast<int>(field_E0_), 0, 0, 0, 0);
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
            if (records_[index].entries_[entry].flag & 1) {
                return 0;
            }
        }
    }
    field_E4_ = 0;
    return 1;
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
// ORIGINAL: 0x00590300 ?lock@Lock@@QAEHHHHHHHH@Z 0x00590300-0x00590469 BYTE_EXACT
// size      361 bytes
// prototype int (__thiscall ?lock@Lock@@QAEHHHHHHHH@Z)(Lock* this, int, int, int, int, int, int, int)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052DBA0 0x0058FD90 0x0058FE80 0x00592EE0
// The second owner check is the inlined global_lock(); the first square's
// failure jumps into the second's failure block.
Return Value: 1 on refusal or lock failure, 0 on success
Status: Complete
*/
int Lock::lock(int slot, int flags, int a3, int a4, int a5, int a6, int a7) {
    // Refuse when another owner already holds the global lock.
    if (field_E0_ != 0 && field_E0_ != static_cast<uint32_t>(slot)) {
        return 1;
    }
    const int take_global = flags & 2;
    if (take_global && global_lock(slot) != 0) {
        return 1;
    }

    PlayerLock &record = records_[slot];
    for (int entry = 0; entry < 2; ++entry) {
        record.entries_[entry].flag = 0;
        record.entries_[entry].second = -1;
        record.entries_[entry].first = -1;
    }
    record.active_ = 0;

    if (record.entries_[0].lock(slot, flags, a3, a4) != 0) {
        goto failed;
    }
    if (record.entries_[1].lock(slot, a5, a6, a7) != 0) {
failed:
        // A square failed: unlock both entries, clear the record, and drop
        // the global lock if this slot holds it.
        for (entry = 0; entry < 2; ++entry) {
            record.entries_[entry].unlock(slot);
        }
        record.active_ = 0;
        if (field_E0_ == static_cast<uint32_t>(slot)) {
            field_E4_ = 0;
            field_E0_ = 0;
        }
        return 1;
    }

    if (take_global && current_server() != 0 && field_E4_ != 0) {
        const uint32_t owner = field_E0_;
        for (int index = 1; index < 8; ++index) {
            if (static_cast<uint32_t>(index) == owner) {
                continue;
            }
            for (int entry = 0; entry < 2; ++entry) {
                if (records_[index].entries_[entry].flag & 1) {
                    return 0;
                }
            }
        }
        field_E4_ = 0;
        message_data(0x1205, static_cast<int>(owner), 0, 0, 0, 0);
    }
    return 0;
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
int Lock::add_lock(int slot, int flags, int x, int y) {
    return records_[slot].entries_[1].lock(slot, flags | 0x10, x, y);
}


/*
Purpose: Report whether any lock is engaged. The trailing dword at 0xE0 forces
         a yes on its own; otherwise records 1 through 7 are scanned, but only
         those whose slot bit is set in both the low and second bytes of the
         enable mask, and a record counts when either of its two entries has
         its flag's low bit set. Record 0 is never scanned.
// ORIGINAL: 0x005904A0 ?any_locks@Lock@@QAEHXZ 0x005904A0-0x005904FE BYTE_EXACT
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
    const GameDataLockMaskView mask = {
        GameDataStateStorage.fields_.lock_masks_.enable_mask_
    };
    const uint8_t second_byte = mask.bytes_.alive_mask_;
    for (int index = 1; index < 8; ++index) {
        // Byte-cast mask on the right: the image tests `test eax, edi`.
        if (((1 << index) & static_cast<uint8_t>(mask.enable_mask_)) &&
            (second_byte & (1 << index))) {
            for (int entry = 0; entry < 2; ++entry) {
                if (records_[index].entries_[entry].flag & 1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
