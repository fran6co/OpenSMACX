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
// Generic Jackal control initialized by jackal_init_real.
#include "graphicwin.h"

 /*
  * CheckButton class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor
  * calls GraphicWin's on an unadjusted `this`, so it needs no ABI workaround
  * despite sitting beside the virtually-derived buttons. Its own fields begin
  * at 0xA14; where the object ends is not established.
  *
  * The complete object is now source-owned. The two Sprite members and two
  * callbacks have distinct roles; the constructor and close preserve their
  * original layout and default-pointer bindings.
  */
class CheckButton : public GraphicWin {
 public:
  typedef void (__cdecl *ToggleCallback)(int);

  CheckButton();
  // NOT a constructor: the image carries no SEH frame at 0x00633750 (no
  // `frame` flag), whereas a REAL derived-class constructor calling
  // `GraphicWin::construct()` picks one up under /GX - see FlatButton's
  // and PullDown's own notes (flatbutton.cpp, pulldown.cpp) for the full
  // measurement. An ordinary method drops it, same as Win/GraphicWin/Log.
  // Out-of-line: the deleting destructor needs a real complete-object body.
  ~CheckButton();
  void close();
  static void close_class();
  static int init_class();
  int init(int x, int y, GraphicWin *parent, int toggle_argument);
  void on_left_click(int x, int y) override;

  // Constructor 0x63375A installs the vtable at 0x66DC14. Its words at
  // 0x66DC40 and 0x66DC84 select 0x633850 and 0x633980 respectively, so
  // these handlers are genuine overrides despite the catalogue's QAE labels.
  void on_redraw();
  void on_left_double_click(int x, int y);

  // Storage the image proves is here: its own methods reach 0xA2C.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 6 member(s) from the IDA database, 1 named; it starts a member at 0xA14, which is where src/ ends.

  // RECOVERY_MEMBER: v1 CheckButton checked_ 0xA14 0x00633850
  uint32_t checked_;
  // The callback argument is distinct from the checked-state word.
  // RECOVERY_MEMBER: v1 CheckButton toggle_argument_ 0xA18 0x00633980
  uint32_t toggle_argument_;
  // RECOVERY_MEMBER: v1 CheckButton click_callback_ 0xA1C 0x00633920
  ToggleCallback click_callback_;
  // RECOVERY_MEMBER: v1 CheckButton double_click_callback_ 0xA20 0x00633980
  ToggleCallback double_click_callback_;
  // RECOVERY_MEMBER: v3 CheckButton unchecked_sprite_ Sprite* 0xA24 0x00633850
  Sprite *unchecked_sprite_;
  // RECOVERY_MEMBER: v3 CheckButton checked_sprite_ Sprite* 0xA28 0x00633850
  Sprite *checked_sprite_;
};

static_assert(sizeof(CheckButton) == 0xA2C,
              "CheckButton layout must match the original executable");
