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
#pragma once

#include <stddef.h>
#include <string.h>

struct DeletionRecord {
  uint8_t acknowledgements_;  // 0x00: completed faction bits; 0xFF marks an empty slot.
  uint8_t reserved_[3];
  int owner_;                  // 0x04: owner/turn associated with this run.
  int16_t count_;              // 0x08: number of queued vehicle ids.
  int16_t vehicle_ids_[25];    // 0x0A: remaining space in the 0x3C-byte record.
};

static_assert(sizeof(DeletionRecord) == 0x3C,
              "Deletion record stride must match the original executable");
static_assert(offsetof(DeletionRecord, owner_) == 4,
              "Deletion record owner offset must match the original executable");
static_assert(offsetof(DeletionRecord, count_) == 8,
              "Deletion record count offset must match the original executable");

class DeletionList {
 public:
  DeletionList();
  ~DeletionList() { ; }

  void clear(int index);
  int find_free_slot();
  int find_owner_slot(int owner);
  void ack(int owner, int faction);
  int add_deletion(int veh_id);
  int convert(int original_id, int new_owner);

  // The network sequence and deletion owner are the same native dword.
  // Preserve its bits for unsigned wrapping network arithmetic without
  // converting an out-of-range unsigned value to int or adding alias storage.
  uint32_t sequence_bits() const {
    // Signed-to-unsigned conversion is defined modulo2^32; native int is
    // four-byte two's-complement. The reverse store uses the representation.
    return static_cast<uint32_t>(current_owner_);
  }
  void set_sequence_bits(uint32_t bits) {
    memcpy(&current_owner_, &bits, sizeof bits);
  }
  uint32_t advance_sequence() {
    const uint32_t next = sequence_bits() + 1;
    set_sequence_bits(next);
    return next;
  }

  // RECOVERY_MEMBER: v1 DeletionList records_ 0x0 0x0052DCF0
  DeletionRecord records_[24];
  // RECOVERY_MEMBER: v1 DeletionList current_owner_ 0x5A0 0x0052DDC0
  int current_owner_;

 private:
  friend class NetDaemon;
  // NetDaemon's native constructor initializes this member after clearing
  // its locks. This path starts its lifetime without duplicating those writes.
  struct Uninitialized {};
  explicit DeletionList(Uninitialized) {}
};

static_assert(offsetof(DeletionList, current_owner_) == 0x5A0,
              "Deletion list owner offset must match the original executable");
static_assert(sizeof(DeletionList) == 0x5A4,
              "DeletionList layout must match the original executable");
