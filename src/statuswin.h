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
#include "font.h"
#include "spot.h"

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
  StatusWin();
  ~StatusWin();
  void close();
  void set_loc(int x, int y);
  void reset();


 private:
  // Not a base class: the constructor builds a Caviar at +0x30, on an
  // offset `this`, so Caviar is a member and the bytes before it are
  // something else. What matters for set_loc is only that the four fields
  // below sit where the original puts them, which the unmapped span holds in
  // place and the test checks directly.
  uint8_t field_0_[0x30];  // 0x0

  // Caviar, Font x3 and Spot are ordinary typed members, in declaration
  // order matching the image's own construction order exactly (Caviar,
  // then the three Fonts, then Spot) and matching the DESTRUCTION order in
  // reverse (Spot, font3, font2, font1, then Caviar's own destructor, which
  // forwards to close() - see the note on Caviar's destructor). A real
  // member built implicitly gets no null-pointer guard around it; a
  // placement-new expression does (VC6 guards every `new (p) T()` with a
  // `cmp/je` on the pointer, since `operator new` may return null), which
  // is what an earlier raw-storage version of this class paid for no
  // reason - construction here never actually uses `operator new`.
  Caviar caviar_;  // 0x30, sizeof(Caviar)
  uint8_t field_1400_[0x100];  // 0x1400
  uint32_t field_1500_;  // 0x1500
  uint32_t field_1504_;  // 0x1504
  uint32_t field_1508_;  // 0x1508
  uint8_t field_150C_[0x18];  // 0x150C
  float field_1524_;  // 0x1524
  float field_1528_;  // 0x1528
  float field_152C_;  // 0x152C
  Font font1_;  // 0x1530
  Font font2_;  // 0x1558
  Font font3_;  // 0x1580
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

  uint32_t fInOnClick_;  // 0x15D8
  uint32_t field_15DC_;  // 0x15DC

  // The image proves this is here: the constructor builds a Spot at exactly
  // this offset, right where the previously-mapped span ended.
  Spot spot_;  // 0x15E0
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
