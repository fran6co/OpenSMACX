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

#include "menu.h"
#include "buffer.h"
#include "sprite.h"

 /*
  * AlphaMenu class
  *
  * Eleven methods are catalogued - init, compute, adjust, on_redraw,
  * on_mouse_move and requested_height among them - and none of the ten
  * besides requested_height is recovered, so NOTHING here pins the layout.
  * The one declared member is derived, not guessed: the IDB member table
  * starts a member called `menu` at 0x0 whose size, 2916, is exactly the
  * sizeof(Menu) this tree pins - the same evidence route the typing
  * campaign uses everywhere else. Anything further still needs deriving.
  */
class AlphaMenu {
 public:
  AlphaMenu() { ; }
  // 0x0051DC10 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~AlphaMenu();
  int requested_height();

  Menu menu_;  // 0x0, IDB `menu`, size == sizeof(Menu)

  Buffer buffer_;  // 0xB64, IDB `buffer`, size == sizeof(Buffer)

  Sprite sprite1_;  // 0x10EC, IDB `sprite1`, size == sizeof(Sprite)

  // Storage the image proves is here: its own methods reach 0x1138.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_1118_[0x8];  // 0x1118
  uint8_t field_1120_;  // 0x1120
  uint8_t field_1121_[0xF];  // 0x1121
  uint32_t field_1130_;  // 0x1130
  uint32_t field_1134_;  // 0x1134
};

