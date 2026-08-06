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
  * AlphaMenu class
  *
  * Eleven methods are catalogued - init, compute, adjust, on_redraw,
  * on_mouse_move and requested_height among them - and none of the ten
  * besides requested_height is recovered, so NOTHING here pins the layout.
  * There are deliberately no fields: requested_height reads none, and
  * inventing storage to make the class look complete would be a guess the
  * rest of the tree would then inherit. Nothing may be appended without
  * deriving it first.
  */
class DLLEXPORT AlphaMenu {
 public:
  AlphaMenu() { ; }
  ~AlphaMenu() { ; }
  int requested_height();

  // Storage the image proves is here: its own methods reach 0xA30.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 1 member(s) from the IDA database, 1 named; it starts a member at 0x0, which is where src/ ends.

  uint8_t field_0_[0xA30];  // 0x0

  // Storage the image proves is here: its own methods reach 0x1138.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_A30_[0x708];
};

int __fastcall alpha_menu_requested_height_redirect(AlphaMenu *self, void *);
