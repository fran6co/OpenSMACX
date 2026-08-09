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
#include "flatbutton.h"
#include "time.h"

 /*
  * Credits class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; Credits's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class DLLEXPORT Credits : GraphicWin {
 public:
  Credits() { ; }
  ~Credits() { ; }
  void on_left_down(int a1, int a2);
  void on_right_down(int a1, int a2);
  void on_mouse_move(int a1, int a2);
  void on_button_clicked(int a1);

  // Storage the image proves is here: its own methods reach 0xA18.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 1 member(s) from the IDA database, 0 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t field_A14_;  // 0xA14

  // Storage the image proves is here: its own methods reach 0x2640.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 12 member(s) from the IDA database, 4 named; it starts a member at 0xA18, which is where src/ ends.

  uint32_t field_A18_;  // 0xA18
  uint8_t field_A1C_[0x800];  // 0xA1C
  uint32_t field_121C_;  // 0x121C
  uint32_t field_1220_;  // 0x1220
  uint32_t field_1224_;  // 0x1224
  uint32_t field_1228_;  // 0x1228
  uint32_t field_122C_;  // 0x122C
  uint32_t field_1230_;  // 0x1230
  // Typed from the sizes the tree already asserts: GraphicWin 0xA14,
  // Font 0x28, Buffer 0x588, each verified against the real class by
  // tools/verify_class_layouts.py. Size-neutral by construction, and
  // the byte comparison is what confirms it.
  GraphicWin graphicWin2_;  // 0x1234
  Font font_;  // 0x1C48
  Buffer buffer_;  // 0x1C70
  FlatButton flatButton_;  // 0x21F8, IDB `flatButton`, size == sizeof(FlatButton)
  Time time_;  // 0x2D44, IDB `time`, size == sizeof(Time); ends 0x2D6C
};

void __fastcall credits_on_left_down_redirect(Credits *self, void *, int a1, int a2);
void __fastcall credits_on_right_down_redirect(Credits *self, void *, int a1, int a2);
void __fastcall credits_on_mouse_move_redirect(Credits *self, void *, int a1, int a2);
void __fastcall credits_on_button_clicked_redirect(Credits *self, void *, int a1);
