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
#include "graphicwin.h"

 /*
  * CheckButton class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor
  * calls GraphicWin's on an unadjusted `this`, so it needs no ABI workaround
  * despite sitting beside the virtually-derived buttons. Its own fields begin
  * at 0xA14; where the object ends is not established.
  *
  * The methods recovered here are bare returns or a bare constant, touching
  * no field, which is why they can be replaced ahead of that mapping.
  */
class CheckButton : public GraphicWin {
 public:
  CheckButton();
  // NOT a constructor: the image carries no SEH frame at 0x00633750 (no
  // `frame` flag), whereas a REAL derived-class constructor calling
  // `GraphicWin::construct()` picks one up under /GX - see FlatButton's
  // and PullDown's own notes (flatbutton.cpp, pulldown.cpp) for the full
  // measurement. An ordinary method drops it, same as Win/GraphicWin/Log.
  // 0x004B3EC0 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~CheckButton();
  static void close_class();
  static int init_class();

  // Storage the image proves is here: its own methods reach 0xA2C.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 6 member(s) from the IDA database, 1 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t field_A14_;  // 0xA14
  uint32_t isToggled_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
};

static_assert(sizeof(CheckButton) == 0xA2C,
              "CheckButton layout must match the original executable");

void __cdecl check_button_close_class_redirect();
int __cdecl check_button_init_class_redirect();
