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
  * PlayerLock class
  *
  * A small standalone record with no surviving constructor. clear() is the
  * evidence for the layout: a byte flag at 0, then two three-dword entries at
  * 0x4 and 0x10, each reset to two -1 sentinels and a zero. Nothing pins the
  * size beyond the 0x1C those writes reach.
  */
class DLLEXPORT PlayerLock {
 public:
  PlayerLock() { ; }
  ~PlayerLock() { ; }
  void clear();
  int active();

 private:
  struct Entry {
    int32_t first;
    int32_t second;
    int32_t flag;
  };
  uint8_t active_;
  uint8_t pad_1_[3];
  Entry entries_[2];
};

void __fastcall player_lock_clear_redirect(PlayerLock *self, void *);
int __fastcall player_lock_active_redirect(PlayerLock *self, void *);
