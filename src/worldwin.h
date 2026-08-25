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
#include "mapwin.h"
#include "buffer.h"

#include "original_seam.h"
#include "graphicwin.h"

 /*
  * WorldWin class
  *
  * Derives from GraphicWin in the original by ordinary single inheritance -
  * its constructor calls GraphicWin's on an unadjusted `this` - so unlike the
  * virtually-derived windows this one is written as the original wrote it and
  * needs no ABI workaround.
  *
  * The layout is not established. The constructor's own fields begin at 0xA14,
  * immediately after the base, and where the object ends is unknown, so
  * nothing pins its sizeof and the fields between are not modelled. The
  * methods recovered here are bare returns that touch no field, which is why
  * they can be replaced ahead of that mapping.
  */
class WorldWin : public GraphicWin {
 public:
  // 0x004C3D40, a pending_bodies forwarder.
  void click(int a1, int a2, int button, int is_double);

 public:
  void follow_center(int xCoord, int yCoord);
  void focus(int a1, int a2);
  void set_center(int a1, int a2);
  WorldWin() { ; }
  // NOT a constructor: mapWin_'s own construction is `MapWin::construct(1)`
  // (see the note in mapwin.h - a real `MapWin(int)` pushes the argument
  // TWICE, VC6's hidden most-derived flag), so this cannot rely on
  // automatic member construction either. An ordinary method matches the
  // same idiom as Win/GraphicWin/MapWin.
  WorldWin *construct();
  // 0x004C4A70 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~WorldWin();
  void clear_terrain();
  void on_left_double_click(int a1, int a2);
  void on_left_click(int a1, int a2);
  void on_right_click(int a1, int a2);

  // Storage the image proves is here: its own methods reach 0x239D0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 5 member(s) from the IDA database, 3 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  Buffer buffer1_;  // 0xA1C
  Buffer buffer2_;  // 0xFA4
  MapWin mapWin_;  // 0x152C
  uint8_t field_239AC_[0x10];  // 0x239AC
  uint32_t field_239BC_;  // 0x239BC
  uint32_t field_239C0_;  // 0x239C0
  uint32_t field_239C4_;  // 0x239C4
  uint32_t field_239C8_;  // 0x239C8
  uint32_t field_239CC_;  // 0x239CC

  // Storage the image proves is here: its own methods reach 0x239D4.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_239D0_[0x4];
};


// WorldWin's shared click handler is not recovered.

