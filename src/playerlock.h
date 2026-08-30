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

#include "squarelock.h"

 /*
  * PlayerLock class
  *
  * A small standalone record with no surviving constructor. clear() is the
  * evidence for the layout: a byte flag at 0, then two three-dword entries at
  * 0x4 and 0x10, each reset to two -1 sentinels and a zero. Nothing pins the
  * size beyond the 0x1C those writes reach.
  */
class PlayerLock {
 public:
  PlayerLock() { ; }
  ~PlayerLock() { ; }
  void clear();
  int add_lock(int factionID, int flags, int x, int y);
  void unlock(int factionID);
  int active();

 private:
  typedef SquareLock Entry;
  uint8_t active_;
  uint8_t pad_1_[3];
  Entry entries_[2];
};


static_assert(sizeof(PlayerLock) == 0x1C, "PlayerLock stride must match NetDaemon's 0x14A0 table");
