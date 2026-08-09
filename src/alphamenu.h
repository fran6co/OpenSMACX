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
class DLLEXPORT AlphaMenu {
 public:
  AlphaMenu() { ; }
  ~AlphaMenu() { ; }
  int requested_height();

  Menu menu_;  // 0x0, IDB `menu`, size == sizeof(Menu)

  Buffer buffer_;  // 0xB64, IDB `buffer`, size == sizeof(Buffer)

  // Storage the image proves is here: its own methods reach 0x1138.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_10EC_[0x4C];  // 0x10EC
};

int __fastcall alpha_menu_requested_height_redirect(AlphaMenu *self, void *);
