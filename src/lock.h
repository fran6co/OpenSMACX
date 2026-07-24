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

 /*
  * Lock class
  *
  * Opens with eight records of 0x1C bytes each - a flag byte and two entries
  * of the same {first, second, flag} shape PlayerLock uses - followed by three
  * dwords at 0xE0. clear() is the evidence for that layout: it zeroes the three
  * dwords and resets every record to two -1 sentinels and a zero. reset_map
  * ignores the instance and works only on the global map table.
  */
class DLLEXPORT Lock {
 public:
  Lock() { ; }
  ~Lock() { ; }
  void reset_map();
  void clear();
  int any_locks();
  void unlock(int slot);

 private:
  struct Entry {
    int32_t first;
    int32_t second;
    int32_t flag;
  };
  struct Record {
    uint8_t flag;
    uint8_t pad[3];
    Entry entries[2];
  };
  Record records_[8];
  uint32_t field_E0_;
  uint32_t field_E4_;
  uint32_t field_E8_;
};

// The map table and its count live at fixed addresses; rebindable for tests.
extern int32_t *LockMapCount;
extern uint8_t *LockMapTable;

// A per-slot enable mask any_locks gates its record scan on, at a fixed
// address; rebindable for tests.
extern uint32_t *LockEnableMask;

// SquareLock::unlock is 231 bytes of coordinate wrapping over several
// globals and is not recovered; unlock forwards each record entry to it.
typedef void (__thiscall func_square_lock_unlock)(void *entry, int slot);
extern func_square_lock_unlock *LockSquareUnlock;

void __fastcall lock_reset_map_redirect(Lock *self, void *);
void __fastcall lock_clear_redirect(Lock *self, void *);
int __fastcall lock_any_locks_redirect(Lock *self, void *);
void __fastcall lock_unlock_redirect(Lock *self, void *, int slot);
