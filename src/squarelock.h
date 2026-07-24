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
  * SquareLock class
  *
  * A small standalone record. clear() is the layout evidence: two -1 sentinels
  * at 0x0 and 0x4 and a zero at 0x8, the same shape as PlayerLock's entries.
  */
class DLLEXPORT SquareLock {
 public:
  SquareLock() { ; }
  ~SquareLock() { ; }
  void clear();
  void unlock(int factionID);
  int lock(int factionID, int flags, int x, int y);

 private:
  int32_t first_;
  int32_t second_;
  int32_t flag_;
};

void __fastcall square_lock_clear_redirect(SquareLock *self, void *);
void __fastcall square_lock_unlock_redirect(SquareLock *self, void *, int factionID);
int __fastcall square_lock_lock_redirect(SquareLock *self, void *, int factionID,
                                         int flags, int x, int y);
