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
#include "caviar.h"

 /*
  * StatusWin class
  *
  * Derives from Caviar by ordinary single inheritance. Caviar is pinned at
  * 0x13D0, so the base region is exact; StatusWin's own extent is not
  * established.
  *
  * The methods recovered here are bare returns or a bare constant, touching
  * no field, which is why they can be replaced ahead of that mapping.
  */
class DLLEXPORT StatusWin : Caviar {
 public:
  StatusWin() { ; }
  ~StatusWin() { ; }
  void close();
  void set_loc(int x, int y);

 private:
  // Caviar occupies 0x0 to 0x13D0. Everything between there and the fields
  // set_loc writes is still unmapped; the array holds the offsets in place so
  // the four below land where the original puts them, and the test confirms
  // that by checking which bytes of a seeded object actually move.
  uint8_t unmapped_13D0_[0x15B4 - 0x13D0];
  int32_t field_15B4_;
  int32_t field_15B8_;
  int32_t field_15BC_;
  int32_t field_15C0_;
};

void __fastcall status_win_close_redirect(StatusWin *self, void *);
void __fastcall status_win_set_loc_redirect(StatusWin *self, void *, int x, int y);
