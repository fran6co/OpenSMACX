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
  * CouncWin class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; CouncWin's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class DLLEXPORT CouncWin : GraphicWin {
 public:
  CouncWin() { ; }
  ~CouncWin() { ; }
  void on_group_clicked(int a1, int a2);
  void on_button_passover(int a1, int a2);
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_down(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_toggled(int a1, int a2);
  void on_iface_group_clicked(int a1, int a2, int a3);
  int on_button_text_draw(int, int, int);

  // Storage the image proves is here: its own methods reach 0xA38.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 8 member(s) from the IDA database, 1 named; it starts a member at 0xA14, which is where src/ ends.

  uint8_t subIFace_[0x8];  // 0xA14
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint32_t field_A2C_;  // 0xA2C
  uint32_t field_A30_;  // 0xA30
  uint32_t field_A34_;  // 0xA34
};

void __fastcall counc_win_on_group_clicked_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_button_passover_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_left_click_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_right_click_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_left_down_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_right_down_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_mouse_move_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_mouse_leave_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_selected_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_left_double_click_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_right_double_click_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_button_toggled_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_group_clicked_redirect(CouncWin *self, void *, int a1, int a2, int a3);
int __fastcall counc_win_on_button_text_draw_redirect(
    CouncWin *self, void *, int a1, int a2, int a3);
