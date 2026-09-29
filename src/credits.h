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
class Credits : public GraphicWin {
 public:
  Credits() { ; }
  // 0x00428E90 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~Credits();
  void on_left_down(int a1, int a2);
  void on_right_down(int a, int b);
  void on_mouse_move(int a1, int a2);
  void on_button_clicked(int a1);

  // Lifecycle and timer methods recovered from the Credits-owned image
  // functions. The callback's no-argument overload advances the text frame;
  // the integer overload is the timer-dispatch entry point.
  void unload();
  void load();
  void close();
  void draw_credits();
  void timer_callback_daemon();
  void __cdecl timer_callback_daemon(int a1);

  // Storage the image proves is here: its own methods reach 0xA18.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 1 member(s) from the IDA database, 0 named; it starts a member at 0xA14, which is where src/ ends.

  // RECOVERY_MEMBER: v1 Credits frame_count_ 0xA14 0x00428DC0
  uint32_t frame_count_;  // 0xA14

  // Storage the image proves is here: its own methods reach 0x2640.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 12 member(s) from the IDA database, 4 named; it starts a member at 0xA18, which is where src/ ends.

  // RECOVERY_MEMBER: v1 Credits line_count_ 0xA18 0x00428C00
  uint32_t line_count_;  // 0xA18
  // draw_credits indexes this owned string table; unload frees all 512
  // slots, establishing the pointer element width and complete extent.
  char *lines_[512];  // 0xA1C
  uint32_t field_121C_;  // 0x121C
  uint32_t field_1220_;  // 0x1220
  uint32_t field_1224_;  // 0x1224
  uint32_t field_1228_;  // 0x1228
  // Source draw_credits divides the vertical position by this row step.
  // RECOVERY_MEMBER: v1 Credits line_height_ 0x122C 0x00428C00
  uint32_t line_height_;  // 0x122C
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
