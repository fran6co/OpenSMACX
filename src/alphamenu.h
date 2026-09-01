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
  * besides requested_height is recovered. The layout is derived, not
  * guessed: the IDB member table starts a member called `menu` at 0x0
  * whose size, 2916, is exactly the sizeof(Menu) this tree pins, with
  * `buffer` and `sprite1` following at 0xB64 and 0x10EC - the same
  * evidence route the typing campaign uses everywhere else. Console
  * pins the rest: its own carve found the three sprites this header
  * declares at +0x1118, and sizeof(Console) leaves this object exactly
  * 0x119C bytes - where Console's next field starts.
  */
class AlphaMenu : public Menu {
 public:
  AlphaMenu() { ; }
  // 0x0051DC10 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~AlphaMenu();
  // The two slots this class overrides in the Win-family surface, at the
  // positions the image's AlphaMenu vtable carries them: Win::hide's slot 2
  // and the unnamed slot 12 (its image entry is on_redraw@AlphaMenu,
  // 0x00403340). Both bodies are unrecovered; the stubs keep the slots true.
  void hide();
  void vslot_12();
  // The image's slot 17 is the 2-argument on_mouse_move overload
  // (on_mouse_move@AlphaMenu, 0x00403550, unrecovered) - the same
  // slot-true stub.
  void vslot_17(int x, int y);
  int requested_height();

  Buffer buffer_;      // 0xB64, IDB `buffer`, size == sizeof(Buffer)

  Sprite sprite1_;     // 0x10EC, IDB `sprite1`, size == sizeof(Sprite)

  // The three-sprite run Console's constructor walks with the vector
  // constructor iterator at 0x1118 (0x2c each) - the image's ??0Console
  // builds these three as part of building this object.
  Sprite sprites_[3];  // 0x1118
};

