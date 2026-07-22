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
  * DiploPop class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; DiploPop's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class DLLEXPORT DiploPop : GraphicWin {
 public:
  DiploPop() { ; }
  ~DiploPop() { ; }
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
};

void __fastcall diplo_pop_on_iface_left_click_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_right_click_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_left_down_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_right_down_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_mouse_move_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_mouse_leave_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_selected_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_left_double_click_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_right_double_click_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_button_toggled_redirect(DiploPop *self, void *, int a1, int a2);
