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
#include "subinterface.h"

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
class StatusWin {
 public:
  // 0x004B9EA0, a pending_bodies forwarder.
  void redraw();

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
  uint8_t field_0_[0x9C];  // 0x0
  uint32_t field_9C_;  // 0x9C
  uint32_t field_A0_;  // 0xA0
  uint32_t field_A4_;  // 0xA4
  uint8_t field_A8_[0x2C];  // 0xA8
  uint8_t field_D4_;  // 0xD4
  uint8_t field_D5_[0x142B];  // 0xD5
  uint32_t field_1500_;  // 0x1500
  uint32_t field_1504_;  // 0x1504
  uint32_t field_1508_;  // 0x1508
  uint8_t field_150C_[0x18];  // 0x150C
  float field_1524_;  // 0x1524
  float field_1528_;  // 0x1528
  float field_152C_;  // 0x152C
  uint32_t field_1530_;  // 0x1530
  uint8_t field_1534_[0xC];  // 0x1534
  uint32_t field_1540_;  // 0x1540
  uint8_t field_1544_[0x14];  // 0x1544
  uint32_t field_1558_;  // 0x1558
  uint8_t field_155C_[0x24];  // 0x155C
  uint32_t field_1580_;  // 0x1580
  uint8_t field_1584_[0x24];  // 0x1584
  uint32_t field_15A8_;  // 0x15A8
  uint32_t field_15AC_;  // 0x15AC
  uint8_t field_15B0_[0x4];  // 0x15B0
  int32_t iTileX_;
  int32_t iTileY_;
  int32_t iUnitIndex_;
  int32_t iCityIndex_;
  uint32_t field_15C4_;  // 0x15C4
  uint32_t field_15C8_;  // 0x15C8
  uint32_t field_15CC_;  // 0x15CC
  uint32_t field_15D0_;  // 0x15D0
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
// The process-wide SubInterface at 0x006EEED8, as a CONSTANT rather than a
// pointer variable: the image's `mov ecx, 0x6eeed8` is an immediate, and a
// variable costs the load it does not make. Same object as `g_BattleWin`.
inline SubInterface *sub_interface_global() {
  return reinterpret_cast<SubInterface *>(0x006EEED8);
}

void __fastcall status_win_reset_redirect(StatusWin *self, void *);
