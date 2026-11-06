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
#include "deletionlist.h"
#include "lock.h"
#include "veh.h"
#include "log.h"

namespace {
// The .rdata strings the deletion paths hand to log_say, read out of the
// image beside the bindings the same family already carries in veh.cpp.
// Original starts identify the read-only strings, not pointer cells or
// enclosing allocations. log_say takes LPCSTR and borrows these bytes.
// RECOVERY_OBJECT: v1 0x0068C134 _StrDeletingVehicle
static const char StrDeletingVehicle[] = "Deleting vehicle";
// RECOVERY_OBJECT: v1 0x0068C120 _StrDelConvertedIndex
static const char StrDelConvertedIndex[] = "DelConverted index";
}

/*
Purpose: Initialize the 24 deletion records and the current owner. The image
         leaves each record's owner, padding and vehicle-id storage untouched.
// ORIGINAL: 0x0052DCA0 ??0DeletionList@@QAE@XZ 0x0052DCA0-0x0052DCC1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The original body initializes exactly the DeletionList receiver's 24 record headers and trailing owner field, so the source constructor must own it rather than an unused leaf redirect.","evidence":["Original 0x0052DCA0-0x0052DCC1 writes the dword at +0x5A0, then byte +0 and word +8 across 24 records at 0x3C stride.","The historical exact redirect and its width analysis remain at git:dd29f605:src/leaf_recoveries.cpp; Historical src/veh.cpp defined a separate DeletionList global at 0x0093E31C; pinned veh_kill5C08C0 and NetDaemon::fixup_message534330 establish the current NetDaemonNet.deletion_list_ receiver at that address."]}
*/
DeletionList::DeletionList() {
    current_owner_ = 0;
    for (int slot = 0; slot < 24; ++slot) {
        records_[slot].acknowledgements_ = 0xFF;
        records_[slot].count_ = 0;
    }
}

/*
Purpose: Reset one deletion-list entry to its empty state - an 0xFF marker byte
         and a zeroed word - at the record stride of 0x3C.
// ORIGINAL: 0x0052DCD0 ?clear@DeletionList@@QAEXH@Z 0x0052DCD0-0x0052DCEC BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?clear@DeletionList@@QAEXH@Z)(DeletionList* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DeletionList::clear(int index) {
    records_[index].acknowledgements_ = 0xFF;
    records_[index].count_ = 0;
}

/*
Purpose: Find a reusable deletion record. The image returns slot zero if all
         24 slots are occupied, rather than reporting an error.
// ORIGINAL: 0x0052DCF0 ?UNK1@DeletionList@@QAEHXZ 0x0052DCF0-0x0052DD10 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The seed's unnamed member scans the deletion-record acknowledgement bytes for a slot whose active-faction bits are all set.","evidence":["Original 0x0052DCF0-0x0052DD10 reads 0x009A64E8, advances the receiver by 0x3C and tests exactly 24 entries; git:bdab0bfb:src/recovered/0052dcf0.cpp preserves the historical exact spelling.","src/lock.cpp binds 0x009A64E8 as LockEnableMask; DeletionList's neighboring methods use the same record stride and mask."]}
// symbol    ?find_free_slot@DeletionList@@QAEHXZ
*/
int DeletionList::find_free_slot() {
    int slot = 0;
    DeletionRecord *record = records_;
    const unsigned char mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    for (; slot < 24; ) {
        if ((record->acknowledgements_ & mask) == mask) {
            return slot;
        }
        ++slot;
        ++record;
    }
    return 0;
}

/*
Purpose: Find the occupied deletion record for an owner, or return -1.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0052DD10 ?UNK2@DeletionList@@QAEHH@Z 0x0052DD10-0x0052DD4B FILE
// RECOVERY_BINDING: v1 {"reason":"The seed's unnamed member scans occupied 0x3C-byte records and compares the owner word at +4 with its argument.","evidence":["Original 0x0052DD10-0x0052DD4B scans 24 entries, tests the 0x009A64E8 acknowledgement mask, compares +4 with its argument and returns -1 on exhaustion.","src/recovered/units/0052dd10.cpp retains the earlier candidate and its failed standalone compilation evidence."]}
// symbol    ?find_owner_slot@DeletionList@@QAEHH@Z
*/
int DeletionList::find_owner_slot(int owner) {
    const unsigned char mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    for (int slot = 0; slot < 24; ++slot) {
        const DeletionRecord &record = records_[slot];
        if ((record.acknowledgements_ & mask) != mask && record.owner_ == owner) {
            return slot;
        }
    }
    return -1;
}

/*
Purpose: Mark a faction's acknowledgement for an owner's pending deletions;
         free the record once every active faction has acknowledged it.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0052DD50 ?ack@DeletionList@@QAEXHH@Z 0x0052DD50-0x0052DDB5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The owner/mask search and flag update in the seed are one acknowledgement operation over the typed deletion records.","evidence":["Original 0x0052DD50-0x0052DDB5 scans 24 records, compares owner at +4, sets a faction bit in byte zero and writes 0xFF when active bits are complete.","src/recovered/units/0052dd50.cpp preserves the earlier candidate; src/lock.cpp owns the active-faction mask at 0x009A64E8."]}
*/
void DeletionList::ack(int owner, int faction) {
    const int slot = find_owner_slot(owner);
    if (slot < 0) {
        return;
    }
    DeletionRecord &record = records_[slot];
    // Preserve the pinned VC6 byte shift by CL, including its original count behavior.
    // The selected compiler output, rather than portable out-of-range C++ shifts, is the evidence.
    record.acknowledgements_ |= static_cast<unsigned char>(1u << faction);
    const unsigned char mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    if ((record.acknowledgements_ & mask) == mask) {
        record.acknowledgements_ = 0xFF;
    }
}

// DeletionList::convert, 0x0052DDC0. Multiplayer id conversion: remap a
// vehicle id across the deletion list's occupied records, walking every
// record between the old and new owner and subtracting each queued id below
// the value; both paths report through log_say.
/*
Purpose: Convert a vehicle id across the deletion list's queued deletions.
// ORIGINAL: 0x0052DDC0 ?convert@DeletionList@@QAEHHH@Z 0x0052DDC0-0x0052DE65 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original52DDC0..52DE65; retired seed git:117282ee:src/unrecovered/0052ddc0.cpp records the same identity and spans.", "The records are deletionlist.h's own 0x3C-stride entries with the occupancy mask lock.h's LockEnableMask (0x009A64E8), the owner count at +0x5A0 and the per-record id run at +0xA; the log_say format is the image's DelConverted string bound beside this body."], "reason": "Coverage transcription of the id conversion on the typed DeletionList receiver with the subtract-below walk exactly as the seed orders it."}
Status: Coverage transcription - unclaimed body
*/
int DeletionList::convert(int veh_id, int new_owner) {
    const int original_id = veh_id;
    for (int search = new_owner + 1; search <= current_owner_; search++) {
        const int slot = find_owner_slot(search);
        if (slot >= 0) {
            const DeletionRecord &record = records_[slot];
            int count = record.count_;
            for (int index = 0; index < count; ++index) {
                const int deleted_id = record.vehicle_ids_[index];
                if (deleted_id == veh_id) {
                    veh_id = -1;
                } else if (deleted_id < veh_id) {
                    veh_id--;
                }
            }
        }
    }
    if (original_id != veh_id) {
        log_say(StrDelConvertedIndex, original_id, veh_id, 0);
    }
    return veh_id;
}

// DeletionList::add_deletion, 0x0052DE70. Enqueue one multiplayer deletion:
// report the vehicle's position, find its record by owner (falling back to
// the first free record), and append the id to that record's run.
/*
Purpose: Queue one vehicle deletion against its owner's deletion record.
// ORIGINAL: 0x0052DE70 ?add_deletion@DeletionList@@QAEHH@Z 0x0052DE70-0x0052DF2E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original52DE70..52DF2E; retired seed git:117282ee:src/unrecovered/0052de70.cpp records the same identity and spans with its raw-offset TRIED note kept below.", "The vehicles are veh.h's own Vehs (0x00952828, 0x34 stride) whose x/y words open the record; the occupancy mask is lock.h's LockEnableMask and the owner count sits at +0x5A0. The original uses 0x1E shorts as each 0x3C-byte record stride; 25 shorts fit after its 0xA-byte header."], "reason": "Coverage transcription of the deletion enqueue on the typed DeletionList receiver with Vehs and LockEnableMask named from their owners."}
Status: Coverage transcription - unclaimed body
*/
int DeletionList::add_deletion(int veh_id) {
    log_say(StrDeletingVehicle, veh_id, Vehs[veh_id].x, Vehs[veh_id].y);

    int slot = find_owner_slot(current_owner_);
    if (slot < 0) {
        slot = find_free_slot();
        records_[slot].acknowledgements_ = 0xFF;
        records_[slot].count_ = 0;
        records_[slot].owner_ = current_owner_;
        records_[slot].acknowledgements_ = 0;
    }

    const int old_count = records_[slot].count_;
    records_[slot].vehicle_ids_[old_count] = static_cast<short>(veh_id);
    ++records_[slot].count_;
    return static_cast<int>(records_[slot].count_) - 1;
}
