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

#include "original_seam.h"
#include "squarelock.h"

 /*
  * Lock class
  *
  * Opens with eight records of 0x1C bytes each - a flag byte and two entries
  * of the same {first, second, flag} shape PlayerLock uses - followed by three
  * dwords at 0xE0. clear() is the evidence for that layout: it zeroes the three
  * dwords and resets every record to two -1 sentinels and a zero. reset_map
  * ignores the instance and works only on the global map table.
  */
class Lock {
 public:
  Lock() { ; }
  ~Lock() { ; }
  void reset_map();
  void clear();
  int any_locks();
  void unlock(int slot);
  int global_lock(int owner);
  int check_global_2(int owner);
  void check_global();
  int add_lock(int slot, int flags, int x, int y);
  int lock(int slot, int flags, int a3, int a4, int a5, int a6, int a7);

 private:
  // A SquareLock, and not a look-alike. Both are {first, second, flag} at
  // 12 bytes, and every `Entry` this class holds is handed to
  // `SquareLock::lock` and `SquareLock::unlock` - which the tree reached
  // through a pointer-to-member because the types did not admit the call.
  typedef SquareLock Entry;
  struct Record {
    uint8_t flag;
    uint8_t pad[3];
    Entry entries[2];
  };
  Record records_[8];
  uint32_t field_E0_;  // 0x00e0
  uint32_t field_E4_;
  uint32_t field_E8_;
};

static_assert(sizeof(Lock) == 0xEC,
              "Lock layout must match the original executable");

// The map table's COUNT lives at a fixed address holding the int directly;
// the table ITSELF lives at a fixed address holding a POINTER to it (the
// image dereferences the address `mov eax, dword ptr [0x94a30c]` to load the
// table pointer's VALUE before indexing it, it does not use 0x94A30C as the
// table's own base). Both rebindable for tests.
//
// LockMapTable stays a SINGLE star, not `uint8_t **const`: a namespace-scope
// `T *const` folds to the bare immediate at every use (same as
// LockMapCount), but VC6 does NOT fold a `T **const` the same way - it
// materialises it as a real CONST-segment slot and loads it before the
// dereference, costing a whole extra `mov`. Casting the folded immediate to
// a pointer-to-pointer AT THE USE SITE keeps the fold and reaches the same
// address.
int32_t *const LockMapCount = (int32_t *)0x00949884;
uint8_t *const LockMapTable = (uint8_t *)0x0094A30C;

// A per-slot enable mask any_locks gates its record scan on, at a fixed
// address; rebindable for tests.
uint32_t *const LockEnableMask = (uint32_t *)0x009A64E8;

// SquareLock::unlock is 231 bytes of coordinate wrapping over several
// globals and is not recovered; unlock forwards each record entry to it.

// SquareLock::lock owns the same map-coordinate logic as unlock and is not
// recovered; add_lock forwards one record entry to it.

// current_server reports whether this machine is the game server; not
// recovered. Declared as an ordinary function with a pending_bodies.cpp
// forwarder to 0x0052DBA0 - LEVER: bound as a `func_current_server *`
// pointer, this compiled `call dword ptr [...]` where the image's callers
// emit `call rel32`; an ordinary declaration gets the E8 back.
int __cdecl current_server();

// message_data broadcasts a game event; not recovered, so check_global
// reaches it through a rebindable seam.
#include "netdaemon.h"  // message_data, the same 0x00592EE0

