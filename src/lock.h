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
  * reset_map ignores its instance entirely - it works on two tables the game
  * keeps at fixed addresses - so this needs no layout for it. The storage is a
  * placeholder for an object the method never reads; the class's other methods
  * are not recovered here.
  */
class DLLEXPORT Lock {
 public:
  Lock() { ; }
  ~Lock() { ; }
  void reset_map();

 private:
  uint8_t unmapped_[4];
};

// The map table and its count live at fixed addresses; rebindable for tests.
extern int32_t *LockMapCount;
extern uint8_t *LockMapTable;

void __fastcall lock_reset_map_redirect(Lock *self, void *);
