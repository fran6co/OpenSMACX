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
#include "caviar.h"

 /*
  * StatusWin class
  *
  * The original does not derive this from Caviar: the constructor builds
  * one at +0x30 on an offset `this`, making it a member. That distinction
  * was missed when this header was first written, because the check looked
  * only at which constructor ran first and not at whether `this` had been
  * adjusted before it.
  *
  * The layout is not established and nothing pins its sizeof.
*/
class DLLEXPORT StatusWin {
 public:
  StatusWin() { ; }
  ~StatusWin() { ; }
  void close();
  void set_loc(int x, int y);
  void reset();


 private:
  // Not a base class: the constructor builds a Caviar at +0x30, on an
  // offset `this`, so Caviar is a member and the bytes before it are
  // something else. What matters for set_loc is only that the four fields
  // below sit where the original puts them, which the unmapped span holds in
  // place and the test checks directly.
  uint8_t unmapped_0_[0x15B4];
  int32_t iTileX_;
  int32_t iTileY_;
  int32_t iUnitIndex_;
  int32_t iCityIndex_;
  uint8_t unmapped_15C4_[0x15D4 - 0x15C4];
  int32_t field_15D4_;

  // Storage the image proves is here: its own methods reach 0x15E0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 2 member(s) from the IDA database, 0 named; it starts a member at 0x15D8, which is where src/ ends.

  uint32_t fInOnClick_;  // 0x15D8
  uint32_t field_15DC_;  // 0x15DC
};

void __fastcall status_win_close_redirect(StatusWin *self, void *);
void __fastcall status_win_set_loc_redirect(StatusWin *self, void *, int x, int y);

// SubInterface::release_iface_mode is not recovered, and the interface it
// acts on is a global the original reaches at a fixed address. Both are
// rebindable so the reset can be observed without either being present.
typedef void (OriginalObject::*func_release_iface_mode)();
extern func_release_iface_mode SubInterfaceOriginalReleaseIfaceMode;
extern void *SubInterfaceGlobal;

void __fastcall status_win_reset_redirect(StatusWin *self, void *);
